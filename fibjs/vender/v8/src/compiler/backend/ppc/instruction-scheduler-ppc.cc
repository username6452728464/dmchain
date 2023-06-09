#include "src/init/v8.h"

#if V8_TARGET_ARCH_PPC

// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/compiler/backend/instruction-scheduler.h"

namespace v8 {
namespace internal {
namespace compiler {

bool InstructionScheduler::SchedulerSupported() { return true; }

int InstructionScheduler::GetTargetInstructionFlags(
    const Instruction* instr) const {
  switch (instr->arch_opcode()) {
    case kPPC_And:
    case kPPC_AndComplement:
    case kPPC_Or:
    case kPPC_OrComplement:
    case kPPC_Xor:
    case kPPC_ShiftLeft32:
    case kPPC_ShiftLeft64:
    case kPPC_ShiftLeftPair:
    case kPPC_ShiftRight32:
    case kPPC_ShiftRight64:
    case kPPC_ShiftRightPair:
    case kPPC_ShiftRightAlg32:
    case kPPC_ShiftRightAlg64:
    case kPPC_ShiftRightAlgPair:
    case kPPC_RotRight32:
    case kPPC_RotRight64:
    case kPPC_Not:
    case kPPC_RotLeftAndMask32:
    case kPPC_RotLeftAndClear64:
    case kPPC_RotLeftAndClearLeft64:
    case kPPC_RotLeftAndClearRight64:
    case kPPC_Add32:
    case kPPC_Add64:
    case kPPC_AddWithOverflow32:
    case kPPC_AddPair:
    case kPPC_AddDouble:
    case kPPC_Sub:
    case kPPC_SubWithOverflow32:
    case kPPC_SubPair:
    case kPPC_SubDouble:
    case kPPC_Mul32:
    case kPPC_Mul32WithHigh32:
    case kPPC_Mul64:
    case kPPC_MulHigh32:
    case kPPC_MulHighU32:
    case kPPC_MulPair:
    case kPPC_MulDouble:
    case kPPC_Div32:
    case kPPC_Div64:
    case kPPC_DivU32:
    case kPPC_DivU64:
    case kPPC_DivDouble:
    case kPPC_Mod32:
    case kPPC_Mod64:
    case kPPC_ModU32:
    case kPPC_ModU64:
    case kPPC_ModDouble:
    case kPPC_Neg:
    case kPPC_NegDouble:
    case kPPC_SqrtDouble:
    case kPPC_FloorDouble:
    case kPPC_CeilDouble:
    case kPPC_TruncateDouble:
    case kPPC_RoundDouble:
    case kPPC_MaxDouble:
    case kPPC_MinDouble:
    case kPPC_AbsDouble:
    case kPPC_Cntlz32:
    case kPPC_Cntlz64:
    case kPPC_Popcnt32:
    case kPPC_Popcnt64:
    case kPPC_Cmp32:
    case kPPC_Cmp64:
    case kPPC_CmpDouble:
    case kPPC_Tst32:
    case kPPC_Tst64:
    case kPPC_ExtendSignWord8:
    case kPPC_ExtendSignWord16:
    case kPPC_ExtendSignWord32:
    case kPPC_Uint32ToUint64:
    case kPPC_Int64ToInt32:
    case kPPC_Int64ToFloat32:
    case kPPC_Int64ToDouble:
    case kPPC_Uint64ToFloat32:
    case kPPC_Uint64ToDouble:
    case kPPC_Int32ToFloat32:
    case kPPC_Int32ToDouble:
    case kPPC_Uint32ToFloat32:
    case kPPC_Uint32ToDouble:
    case kPPC_Float32ToDouble:
    case kPPC_Float64SilenceNaN:
    case kPPC_DoubleToInt32:
    case kPPC_DoubleToUint32:
    case kPPC_DoubleToInt64:
    case kPPC_DoubleToUint64:
    case kPPC_DoubleToFloat32:
    case kPPC_DoubleExtractLowWord32:
    case kPPC_DoubleExtractHighWord32:
    case kPPC_DoubleInsertLowWord32:
    case kPPC_DoubleInsertHighWord32:
    case kPPC_DoubleConstruct:
    case kPPC_BitcastInt32ToFloat32:
    case kPPC_BitcastFloat32ToInt32:
    case kPPC_BitcastInt64ToDouble:
    case kPPC_BitcastDoubleToInt64:
    case kPPC_ByteRev32:
    case kPPC_ByteRev64:
    case kPPC_DecompressSigned:
    case kPPC_DecompressPointer:
    case kPPC_DecompressAny:
    case kPPC_CompressSigned:
    case kPPC_CompressPointer:
    case kPPC_CompressAny:
      return kNoOpcodeFlags;

    case kPPC_LoadWordS8:
    case kPPC_LoadWordU8:
    case kPPC_LoadWordS16:
    case kPPC_LoadWordU16:
    case kPPC_LoadWordS32:
    case kPPC_LoadWordU32:
    case kPPC_LoadWord64:
    case kPPC_LoadFloat32:
    case kPPC_LoadDouble:
    case kPPC_AtomicLoadUint8:
    case kPPC_AtomicLoadUint16:
    case kPPC_AtomicLoadWord32:
    case kPPC_AtomicLoadWord64:
    case kPPC_Peek:
      return kIsLoadOperation;

    case kPPC_StoreWord8:
    case kPPC_StoreWord16:
    case kPPC_StoreWord32:
    case kPPC_StoreWord64:
    case kPPC_StoreFloat32:
    case kPPC_StoreDouble:
    case kPPC_Push:
    case kPPC_PushFrame:
    case kPPC_StoreToStackSlot:
    case kPPC_Sync:
      return kHasSideEffect;

    case kPPC_AtomicStoreUint8:
    case kPPC_AtomicStoreUint16:
    case kPPC_AtomicStoreWord32:
    case kPPC_AtomicStoreWord64:
    case kPPC_AtomicExchangeUint8:
    case kPPC_AtomicExchangeUint16:
    case kPPC_AtomicExchangeWord32:
    case kPPC_AtomicExchangeWord64:
    case kPPC_AtomicCompareExchangeUint8:
    case kPPC_AtomicCompareExchangeUint16:
    case kPPC_AtomicCompareExchangeWord32:
    case kPPC_AtomicCompareExchangeWord64:
    case kPPC_AtomicAddUint8:
    case kPPC_AtomicAddUint16:
    case kPPC_AtomicAddUint32:
    case kPPC_AtomicAddUint64:
    case kPPC_AtomicAddInt8:
    case kPPC_AtomicAddInt16:
    case kPPC_AtomicAddInt32:
    case kPPC_AtomicAddInt64:
    case kPPC_AtomicSubUint8:
    case kPPC_AtomicSubUint16:
    case kPPC_AtomicSubUint32:
    case kPPC_AtomicSubUint64:
    case kPPC_AtomicSubInt8:
    case kPPC_AtomicSubInt16:
    case kPPC_AtomicSubInt32:
    case kPPC_AtomicSubInt64:
    case kPPC_AtomicAndUint8:
    case kPPC_AtomicAndUint16:
    case kPPC_AtomicAndUint32:
    case kPPC_AtomicAndUint64:
    case kPPC_AtomicAndInt8:
    case kPPC_AtomicAndInt16:
    case kPPC_AtomicAndInt32:
    case kPPC_AtomicAndInt64:
    case kPPC_AtomicOrUint8:
    case kPPC_AtomicOrUint16:
    case kPPC_AtomicOrUint32:
    case kPPC_AtomicOrUint64:
    case kPPC_AtomicOrInt8:
    case kPPC_AtomicOrInt16:
    case kPPC_AtomicOrInt32:
    case kPPC_AtomicOrInt64:
    case kPPC_AtomicXorUint8:
    case kPPC_AtomicXorUint16:
    case kPPC_AtomicXorUint32:
    case kPPC_AtomicXorUint64:
    case kPPC_AtomicXorInt8:
    case kPPC_AtomicXorInt16:
    case kPPC_AtomicXorInt32:
    case kPPC_AtomicXorInt64:
      return kHasSideEffect;

#define CASE(Name) case k##Name:
      COMMON_ARCH_OPCODE_LIST(CASE)
#undef CASE
      // Already covered in architecture independent code.
      UNREACHABLE();
  }

  UNREACHABLE();
}

int InstructionScheduler::GetInstructionLatency(const Instruction* instr) {
  // TODO(all): Add instruction cost modeling.
  return 1;
}

}  // namespace compiler
}  // namespace internal
}  // namespace v8


#endif  // V8_TARGET_ARCH_PPC