/*
 * traceInfo.cpp
 *
 *  Created on: Oct 1, 2017
 *      Author: lion
 */

#ifdef _WIN32
#pragma warning(disable : 4800)
#pragma warning(disable : 4101)
#pragma warning(disable : 4244)
#endif

#include "v8/src/codegen/bailout-reason.h"
#include "v8/src/objects/compressed-slots.h"
#include "v8/src/objects/function-kind.h"
#include "v8/src/objects/function-syntax-kind.h"
#include "v8/src/objects/objects.h"
#include "v8/src/objects/script.h"
#include "v8/src/objects/slots.h"
#include "v8/src/objects/smi.h"
#include "v8/src/objects/struct.h"
#include "v8/src/objects/object-macros.h"

#define private public
#include "v8/src/objects/shared-function-info.h"
#undef private

#include "v8/src/api/api-inl.h"
#include "v8/src/utils/utils.h"
#include "v8/src/api/api.h"
#include "v8/src/execution/isolate.h"
#include "v8/src/execution/frames.h"
#include "v8/src/execution/frames-inl.h"
#include "v8/src/json/json-stringifier.h"
#include "v8/src/debug/debug-interface.h"
#include "v8/src/execution/microtask-queue.h"

#include "exlib/include/qstring.h"

#include "v8.h"
#include "v8_api.h"

using namespace v8;

namespace fibjs {

intptr_t RunMicrotaskSize(Isolate* isolate)
{
    i::Isolate* _isolate = reinterpret_cast<i::Isolate*>(isolate);
    return _isolate->default_microtask_queue()->size();
}

bool isFrozen(Handle<Object> object)
{
    auto obj = Utils::OpenHandle(*object);
    Maybe<bool> test = i::JSReceiver::TestIntegrityLevel(obj, i::FROZEN);
    return test.ToChecked();
}

void setAsyncFunctoin(Handle<Function> func)
{
    i::Handle<i::Object> obj = Utils::OpenHandle(*func);
    i::Handle<i::JSFunction> _func = i::Handle<i::JSFunction>::cast(obj);
    _func->shared().set_kind(i::kAsyncFunction);
}

template <bool do_callback>
class CallDepthScope {
public:
    CallDepthScope(i::Isolate* isolate, Local<Context> context)
        : isolate_(isolate)
        , context_(context)
        , escaped_(false)
        , safe_for_termination_(isolate->next_v8_call_is_safe_for_termination())
        , interrupts_scope_(isolate_, i::StackGuard::TERMINATE_EXECUTION,
              isolate_->only_terminate_in_safe_scope()
                  ? (safe_for_termination_
                          ? i::InterruptsScope::kRunInterrupts
                          : i::InterruptsScope::kPostponeInterrupts)
                  : i::InterruptsScope::kNoop)
    {
        isolate_->thread_local_top()->IncrementCallDepth(this);
        isolate_->set_next_v8_call_is_safe_for_termination(false);
        if (!context.IsEmpty()) {
            i::Handle<i::Context> env = Utils::OpenHandle(*context);
            i::HandleScopeImplementer* impl = isolate->handle_scope_implementer();
            if (!isolate->context().is_null() && isolate->context().native_context() == env->native_context()) {
                context_ = Local<Context>();
            } else {
                impl->SaveContext(isolate->context());
                isolate->set_context(*env);
            }
        }
        if (do_callback)
            isolate_->FireBeforeCallEnteredCallback();
    }
    ~CallDepthScope()
    {
        i::MicrotaskQueue* microtask_queue = isolate_->default_microtask_queue();
        if (!context_.IsEmpty()) {
            i::HandleScopeImplementer* impl = isolate_->handle_scope_implementer();
            isolate_->set_context(impl->RestoreContext());

            i::Handle<i::Context> env = Utils::OpenHandle(*context_);
            microtask_queue = env->native_context().microtask_queue();
        }
        if (!escaped_)
            isolate_->thread_local_top()->DecrementCallDepth(this);
        if (do_callback)
            isolate_->FireCallCompletedCallback(microtask_queue);
// TODO(jochen): This should be #ifdef DEBUG
#ifdef V8_CHECK_MICROTASKS_SCOPES_CONSISTENCY
        if (do_callback)
            CheckMicrotasksScopesConsistency(microtask_queue);
#endif
        isolate_->set_next_v8_call_is_safe_for_termination(safe_for_termination_);
    }

    void Escape()
    {
        DCHECK(!escaped_);
        escaped_ = true;
        auto thread_local_top = isolate_->thread_local_top();
        thread_local_top->DecrementCallDepth(this);
        bool clear_exception = thread_local_top->CallDepthIsZero() && thread_local_top->try_catch_handler_ == nullptr;
        isolate_->OptionalRescheduleException(clear_exception);
    }

private:
    i::Isolate* const isolate_;
    Local<Context> context_;
    bool escaped_;
    bool do_callback_;
    bool safe_for_termination_;
    i::InterruptsScope interrupts_scope_;
    i::Address previous_stack_height_;

    friend class i::ThreadLocalTop;

    DISALLOW_NEW_AND_DELETE()
    DISALLOW_COPY_AND_ASSIGN(CallDepthScope);
};

bool path_isAbsolute(exlib::string path);

Local<String> JSON_Stringify(Isolate* isolate,
    Local<Value> json_object, Local<Function> json_replacer)
{
    i::Isolate* v8_isolate = reinterpret_cast<i::Isolate*>(isolate);
    CallDepthScope<false> call_depth_scope(v8_isolate, isolate->GetCurrentContext());

    Local<String> result;
    if (*json_object == nullptr || *json_replacer == nullptr)
        return result;

    i::Handle<i::Object> object = Utils::OpenHandle(*json_object);
    i::Handle<i::Object> replacer = Utils::OpenHandle(*json_replacer);
    i::Handle<i::String> gap_string = v8_isolate->factory()->empty_string();
    i::Handle<i::Object> maybe;

    if (i::JsonStringify(v8_isolate, object, replacer, gap_string)
            .ToHandle(&maybe))
        ToLocal<String>(i::Object::ToString(v8_isolate, maybe), &result);

    if (result.IsEmpty())
        call_depth_scope.Escape();

    return result;
}

void InvokeApiInterruptCallbacks(Isolate* isolate)
{
    i::Isolate* v8_isolate = (i::Isolate*)isolate;
    v8_isolate->InvokeApiInterruptCallbacks();
}

V8FrameInfo save_fi(Isolate* isolate)
{
    i::Isolate* v8_isolate = (i::Isolate*)isolate;
    V8FrameInfo fi;

    fi.entry_fp = (void*)*v8_isolate->c_entry_fp_address();
    fi.handle = (void*)*v8_isolate->handler_address();

    return fi;
}

exlib::string traceInfo(Isolate* isolate, int32_t deep, void* entry_fp, void* handle)
{
    i::Isolate* v8_isolate = (i::Isolate*)isolate;
    i::ThreadLocalTop tt;

    tt.c_entry_fp_ = (i::Address)entry_fp;
    tt.handler_ = (i::Address)handle;

    i::JavaScriptFrameIterator it(v8_isolate, &tt);

    exlib::string strBuffer;
    bool bFirst = true;

    for (; !it.done() && deep-- > 0; it.Advance()) {
        i::JavaScriptFrame* frame = it.frame();
        std::vector<i::FrameSummary> frames;

        frame->Summarize(&frames);

        const i::FrameSummary::JavaScriptFrameSummary& summ = frames[0].AsJavaScript();
        i::Handle<i::Script> script = i::Handle<i::Script>::cast(summ.script());
        if (script->type() == i::Script::TYPE_NORMAL) {
            strBuffer.append(bFirst ? "    at " : "\n    at ");
            bFirst = false;

            String::Utf8Value funcname(isolate, Utils::ToLocal(summ.FunctionName()));
            if (*funcname) {
                strBuffer.append(*funcname);
                strBuffer.append(" (", 2);
            }

            int32_t line_number = 0;
            int32_t column_number = 0;
            i::Script::PositionInfo info;
            bool valid_pos = i::Script::GetPositionInfo(script, summ.SourcePosition(),
                &info, i::Script::WITH_OFFSET);

            if (valid_pos) {
                line_number = info.line + 1;
                column_number = info.column + 1;
            }

            i::Handle<i::Object> name(script->name(), v8_isolate);
            String::Utf8Value filename(isolate, Utils::ToLocal(name));

            char numStr[32];

            if (*filename) {
                strBuffer.append(*filename);
                strBuffer.append(1, ':');
            } else
                strBuffer.append("[eval]:", 7);

            sprintf(numStr, "%d", line_number);
            strBuffer.append(numStr);
            strBuffer.append(1, ':');
            sprintf(numStr, "%d", column_number);
            strBuffer.append(numStr);

            if (**funcname)
                strBuffer.append(1, ')');
        }
    }
    return strBuffer;
}

exlib::string traceInfo(Isolate* isolate, int32_t deep)
{
    i::Isolate* v8_isolate = (i::Isolate*)isolate;
    return traceInfo(isolate, deep, (void*)*v8_isolate->c_entry_fp_address(), (void*)*v8_isolate->handler_address());
}

void beginCoverage(Isolate* isolate)
{
    debug::Coverage::SelectMode(isolate, debug::CoverageMode::kBlockCount);
}

void pauseCoverage(Isolate* isolate)
{
    debug::Coverage::SelectMode(isolate, debug::CoverageMode::kBestEffort);
}

inline std::string ToSTLString(Isolate* isolate, Local<String> v8_str)
{
    String::Utf8Value utf8(isolate, v8_str);
    return *utf8;
}

inline int LineFromOffset(Local<debug::Script> script, int offset)
{
    debug::Location location = script->GetSourceLocation(offset);
    return location.GetLineNumber();
}

inline void WriteLcovDataForRange(std::vector<uint32_t>& lines, int start_line,
    int end_line, uint32_t count)
{
    // Ensure space in the array.
    lines.resize(std::max(static_cast<size_t>(end_line + 1), lines.size()), 0);
    // Boundary lines could be shared between two functions with different
    // invocation counts. Take the maximum.
    lines[start_line] = std::max(lines[start_line], count);
    lines[end_line] = std::max(lines[end_line], count);
    // Invocation counts for non-boundary lines are overwritten.
    for (int k = start_line + 1; k < end_line; k++)
        lines[k] = count;
}

inline void WriteLcovDataForNamedRange(FILE* sink,
    std::vector<uint32_t>& lines, std::string name, int start_line, int end_line, uint32_t count)
{
    WriteLcovDataForRange(lines, start_line, end_line, count);
    fprintf(sink, "FN:%d,%s\n", start_line + 1, name.c_str());
    fprintf(sink, "FNDA:%d,%s\n", count, name.c_str());
}

void WriteLcovData(Isolate* isolate, FILE* file)
{
    HandleScope handle_scope(isolate);
    debug::Coverage coverage = debug::Coverage::CollectPrecise(isolate);

    for (size_t i = 0; i < coverage.ScriptCount(); i++) {
        debug::Coverage::ScriptData script_data = coverage.GetScriptData(i);
        Local<debug::Script> script = script_data.GetScript();
        // Skip unnamed scripts.
        Local<String> name;
        if (!script->Name().ToLocal(&name))
            continue;
        std::string file_name = ToSTLString(isolate, name);
        if (!path_isAbsolute(file_name))
            continue;

        fprintf(file, "SF:%s\n", file_name.c_str());
        std::vector<uint32_t> lines;
        for (size_t j = 0; j < script_data.FunctionCount(); j++) {
            debug::Coverage::FunctionData function_data = script_data.GetFunctionData(j);

            // Write function stats.
            {
                debug::Location start = script->GetSourceLocation(function_data.StartOffset());
                debug::Location end = script->GetSourceLocation(function_data.EndOffset());
                int start_line = start.GetLineNumber();
                int end_line = end.GetLineNumber();
                uint32_t count = function_data.Count();

                Local<String> name;
                std::stringstream name_stream;
                if (function_data.Name().ToLocal(&name)) {
                    name_stream << ToSTLString(isolate, name);
                } else {
                    name_stream << "<" << start_line + 1 << "-";
                    name_stream << start.GetColumnNumber() << ">";
                }

                WriteLcovDataForNamedRange(file, lines, name_stream.str(), start_line,
                    end_line, count);
            }

            // Process inner blocks.
            for (size_t k = 0; k < function_data.BlockCount(); k++) {
                debug::Coverage::BlockData block_data = function_data.GetBlockData(k);
                int start_line = LineFromOffset(script, block_data.StartOffset());
                int end_line = LineFromOffset(script, block_data.EndOffset() - 1);
                uint32_t count = block_data.Count();
                WriteLcovDataForRange(lines, start_line, end_line, count);
            }
        }
        // Write per-line coverage. LCOV uses 1-based line numbers.
        for (size_t i = 0; i < lines.size(); i++)
            fprintf(file, "DA:%d,%d\n", (int32_t)(i + 1), lines[i]);

        fprintf(file, "end_of_record\n");
    }

    fclose(file);
}
}