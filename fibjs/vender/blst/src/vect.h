/*
 * Copyright Supranational LLC
 * Licensed under the Apache License, Version 2.0, see LICENSE for details.
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __BLS12_381_ASM_VECT_H__
#define __BLS12_381_ASM_VECT_H__

#include <stddef.h>

#if defined(__BLST_NO_ASM__) || defined(__wasm64__)
typedef unsigned int limb_t;
# define LIMB_T_BITS    32
# ifndef __BLST_NO_ASM__
#  define __BLST_NO_ASM__
# endif

#elif defined(__x86_64__) || defined(__aarch64__)
/* These are available even in ILP32 flavours, but even then they are
 * capable of performing 64-bit operations as efficiently as in *P64. */
typedef unsigned long long limb_t;
# define LIMB_T_BITS    64

#elif defined(_WIN64)   /* Win64 is P64 */
typedef unsigned __int64 limb_t;
# define LIMB_T_BITS    64

#else                   /* 32 bits on 32-bit platforms, 64 - on 64-bit */
#  ifdef _LP64
typedef unsigned long long limb_t;
#   define LIMB_T_BITS   64
#  else
typedef unsigned long limb_t;
#   define LIMB_T_BITS   32
#   define __BLST_NO_ASM__
#  endif
#endif

/*
 * Why isn't LIMB_T_BITS defined as 8*sizeof(limb_t)? Because pre-processor
 * knows nothing about sizeof(anything)...
 */
#if LIMB_T_BITS == 64
# define TO_LIMB_T(limb64)     limb64
#else
# define TO_LIMB_T(limb64)     (limb_t)limb64,(limb_t)(limb64>>32)
#endif

#define NLIMBS(bits)   (bits/LIMB_T_BITS)

typedef limb_t vec256[NLIMBS(256)];
typedef limb_t vec512[NLIMBS(512)];
typedef limb_t vec384[NLIMBS(384)];
typedef limb_t vec768[NLIMBS(768)];
typedef vec384 vec384x[2];      /* 0 is "real" part, 1 is "imaginary" */

typedef unsigned char byte;
#define TO_BYTES(limb64)    (byte)limb64,(byte)(limb64>>8),\
                            (byte)(limb64>>16),(byte)(limb64>>24),\
                            (byte)(limb64>>32),(byte)(limb64>>40),\
                            (byte)(limb64>>48),(byte)(limb64>>56)
typedef byte pow256[256/8];

/*
 * Internal Boolean type, Bolean by value, hence safe to cast to or
 * reinterpret as 'bool'.
 */
typedef limb_t bool_t;

/*
 * Assembly subroutines...
 */
#if defined(__ADX__) /* e.g. -march=broadwell */ && !defined(__BLST_PORTABLE__)
# define mul_mont_sparse_256 mulx_mont_sparse_256
# define sqr_mont_sparse_256 sqrx_mont_sparse_256
# define from_mont_256 fromx_mont_256
# define redc_mont_256 redcx_mont_256
# define mul_mont_384 mulx_mont_384
# define sqr_mont_384 sqrx_mont_384
# define sqr_n_mul_mont_384 sqrx_n_mul_mont_384
# define sqr_n_mul_mont_383 sqrx_n_mul_mont_383
# define mul_384 mulx_384
# define sqr_384 sqrx_384
# define redc_mont_384 redcx_mont_384
# define from_mont_384 fromx_mont_384
# define sgn0_pty_mont_384 sgn0x_pty_mont_384
# define sgn0_pty_mont_384x sgn0x_pty_mont_384x
# define ct_inverse_mod_383 ctx_inverse_mod_383
#elif defined(__BLST_NO_ASM__)
# define ct_inverse_mod_383 ct_inverse_mod_384
#endif

void mul_mont_sparse_256(vec256 ret, const vec256 a, const vec256 b,
                         const vec256 p, limb_t n0);
void sqr_mont_sparse_256(vec256 ret, const vec256 a, const vec256 p, limb_t n0);
void redc_mont_256(vec256 ret, const vec512 a, const vec256 p, limb_t n0);
void from_mont_256(vec256 ret, const vec256 a, const vec256 p, limb_t n0);

void add_mod_256(vec256 ret, const vec256 a, const vec256 b, const vec256 p);
void sub_mod_256(vec256 ret, const vec256 a, const vec256 b, const vec256 p);
void mul_by_3_mod_256(vec256 ret, const vec256 a, const vec256 p);
void cneg_mod_256(vec256 ret, const vec256 a, bool_t flag, const vec256 p);
void lshift_mod_256(vec256 ret, const vec256 a, size_t count, const vec256 p);
void rshift_mod_256(vec256 ret, const vec256 a, size_t count, const vec256 p);
bool_t eucl_inverse_mod_256(vec256 ret, const vec256 a, const vec256 p,
                            const vec256 one);
limb_t check_mod_256(const pow256 a, const vec256 p);
limb_t add_n_check_mod_256(pow256 ret, const pow256 a, const pow256 b,
                                       const vec256 p);
limb_t sub_n_check_mod_256(pow256 ret, const pow256 a, const pow256 b,
                                       const vec256 p);

void vec_prefetch(const void *ptr, size_t len);

void mul_mont_384(vec384 ret, const vec384 a, const vec384 b,
                  const vec384 p, limb_t n0);
void sqr_mont_384(vec384 ret, const vec384 a, const vec384 p, limb_t n0);
void sqr_n_mul_mont_384(vec384 ret, const vec384 a, size_t count,
                        const vec384 p, limb_t n0, const vec384 b);
void sqr_n_mul_mont_383(vec384 ret, const vec384 a, size_t count,
                        const vec384 p, limb_t n0, const vec384 b);

void mul_384(vec768 ret, const vec384 a, const vec384 b);
void sqr_384(vec768 ret, const vec384 a);
void redc_mont_384(vec384 ret, const vec768 a, const vec384 p, limb_t n0);
void from_mont_384(vec384 ret, const vec384 a, const vec384 p, limb_t n0);
limb_t sgn0_pty_mont_384(const vec384 a, const vec384 p, limb_t n0);
limb_t sgn0_pty_mont_384x(const vec384x a, const vec384 p, limb_t n0);
limb_t sgn0_pty_mod_384(const vec384 a, const vec384 p);
limb_t sgn0_pty_mod_384x(const vec384x a, const vec384 p);

void add_mod_384(vec384 ret, const vec384 a, const vec384 b, const vec384 p);
void sub_mod_384(vec384 ret, const vec384 a, const vec384 b, const vec384 p);
void mul_by_8_mod_384(vec384 ret, const vec384 a, const vec384 p);
void mul_by_3_mod_384(vec384 ret, const vec384 a, const vec384 p);
void cneg_mod_384(vec384 ret, const vec384 a, bool_t flag, const vec384 p);
void lshift_mod_384(vec384 ret, const vec384 a, size_t count, const vec384 p);
void rshift_mod_384(vec384 ret, const vec384 a, size_t count, const vec384 p);
void div_by_2_mod_384(vec384 ret, const vec384 a, const vec384 p);
void ct_inverse_mod_383(vec768 ret, const vec384 inp, const vec384 mod,
                                                      const vec384 modx);
void ct_inverse_mod_256(vec512 ret, const vec256 inp, const vec256 mod,
                                                      const vec256 modx);
bool_t ct_is_square_mod_384(const vec384 inp, const vec384 mod);

#if defined(__ADX__) /* e.g. -march=broadwell */ && !defined(__BLST_PORTABLE__)
# define mul_mont_384x mulx_mont_384x
# define sqr_mont_384x sqrx_mont_384x
# define sqr_mont_382x sqrx_mont_382x
# define sqr_n_mul_mont_384x sqrx_n_mul_mont_384x
# define mul_382x mulx_382x
# define sqr_382x sqrx_382x
#endif

void mul_mont_384x(vec384x ret, const vec384x a, const vec384x b,
                   const vec384 p, limb_t n0);
void sqr_mont_384x(vec384x ret, const vec384x a, const vec384 p, limb_t n0);
void sqr_mont_382x(vec384x ret, const vec384x a, const vec384 p, limb_t n0);
void sqr_n_mul_mont_384x(vec384x ret, const vec384x a, size_t count,
                         const vec384 p, limb_t n0, const vec384x b);
void mul_382x(vec768 ret[2], const vec384x a, const vec384x b, const vec384 p);
void sqr_382x(vec768 ret[2], const vec384x a, const vec384 p);

void add_mod_384x(vec384x ret, const vec384x a, const vec384x b,
                  const vec384 p);
void sub_mod_384x(vec384x ret, const vec384x a, const vec384x b,
                  const vec384 p);
void mul_by_8_mod_384x(vec384x ret, const vec384x a, const vec384 p);
void mul_by_3_mod_384x(vec384x ret, const vec384x a, const vec384 p);
void mul_by_1_plus_i_mod_384x(vec384x ret, const vec384x a, const vec384 p);
void add_mod_384x384(vec768 ret, const vec768 a, const vec768 b,
                     const vec384 p);
void sub_mod_384x384(vec768 ret, const vec768 a, const vec768 b,
                     const vec384 p);

/*
 * C subroutines
 */
static void exp_mont_384(vec384 out, const vec384 inp, const byte *pow,
                         size_t pow_bits, const vec384 p, limb_t n0);
static void exp_mont_384x(vec384x out, const vec384x inp, const byte *pow,
                          size_t pow_bits, const vec384 p, limb_t n0);
static void div_by_zz(limb_t val[]);
static void div_by_z(limb_t val[]);

#ifdef __UINTPTR_TYPE__
typedef __UINTPTR_TYPE__ uptr_t;
#else
typedef const void *uptr_t;
#endif

#if !defined(restrict)
# if !defined(__STDC_VERSION__) || __STDC_VERSION__<199901
#  if defined(__GNUC__) && __GNUC__>=2
#   define restrict __restrict__
#  elif defined(_MSC_VER)
#   define restrict __restrict
#  else
#   define restrict
#  endif
# endif
#endif

#if defined(__CUDA_ARCH__)
# define inline inline __device__
#endif

#if !defined(inline) && !defined(__cplusplus)
# if !defined(__STDC_VERSION__) || __STDC_VERSION__<199901
#  if defined(__GNUC__) && __GNUC__>=2
#   define inline __inline__
#  elif defined(_MSC_VER)
#   define inline __inline
#  else
#   define inline
#  endif
# endif
#endif

static inline bool_t is_bit_set(const byte *v, size_t i)
{   return (v[i/8] >> (i%8)) & 1;   }

static inline bool_t byte_is_zero(unsigned char c)
{   return ((limb_t)(c) - 1) >> (LIMB_T_BITS - 1);   }

static inline bool_t bytes_are_zero(const unsigned char *a, size_t num)
{
    unsigned char acc;
    size_t i;

    for (acc = 0, i = 0; i < num; i++)
        acc |= a[i];

    return byte_is_zero(acc);
}

static inline void bytes_zero(unsigned char *a, size_t num)
{
    size_t i;

    for (i = 0; i < num; i++)
        a[i] = 0;
}

static inline void vec_cswap(void *restrict a, void *restrict b, size_t num,
                             bool_t cbit)
{
    limb_t ai, *ap = (limb_t *)a;
    limb_t bi, *bp = (limb_t *)b;
    limb_t xorm, mask = (limb_t)0 - cbit;
    size_t i;

    num /= sizeof(limb_t);

    for (i = 0; i < num; i++) {
        xorm = ((ai = ap[i]) ^ (bi = bp[i])) & mask;
        ap[i] = ai ^ xorm;
        bp[i] = bi ^ xorm;
    }
}

/* ret = bit ? a : b */
#ifdef __CUDA_ARCH__
extern "C" {
__device__ void vec_select_48(void *ret, const void *a, const void *b,
                                         unsigned int sel_a);
__device__ void vec_select_96(void *ret, const void *a, const void *b,
                                         unsigned int sel_a);
__device__ void vec_select_192(void *ret, const void *a, const void *b,
                                          unsigned int sel_a);
__device__ void vec_select_144(void *ret, const void *a, const void *b,
                                          unsigned int sel_a);
__device__ void vec_select_288(void *ret, const void *a, const void *b,
                                          unsigned int sel_a);
}
#else
void vec_select_48(void *ret, const void *a, const void *b, bool_t sel_a);
void vec_select_96(void *ret, const void *a, const void *b, bool_t sel_a);
void vec_select_144(void *ret, const void *a, const void *b, bool_t sel_a);
void vec_select_192(void *ret, const void *a, const void *b, bool_t sel_a);
void vec_select_288(void *ret, const void *a, const void *b, bool_t sel_a);
#endif
static inline void vec_select(void *ret, const void *a, const void *b,
                              size_t num, bool_t sel_a)
{
#ifndef __BLST_NO_ASM__
    if (num == 48)          vec_select_48(ret, a, b, sel_a);
    else if (num == 96)     vec_select_96(ret, a, b, sel_a);
    else if (num == 144)    vec_select_144(ret, a, b, sel_a);
    else if (num == 192)    vec_select_192(ret, a, b, sel_a);
    else if (num == 288)    vec_select_288(ret, a, b, sel_a);
#else
    if (0) ;
#endif
    else {
        limb_t bi, *rp = (limb_t *)ret;
        const limb_t *ap = (const limb_t *)a;
        const limb_t *bp = (const limb_t *)b;
        limb_t xorm, mask = (limb_t)0 - sel_a;
        size_t i;

        num /= sizeof(limb_t);

        for (i = 0; i < num; i++) {
            xorm = (ap[i] ^ (bi = bp[i])) & mask;
            rp[i] = bi ^ xorm;
        }
    }
}

static inline bool_t is_zero(limb_t l)
{   return (~l & (l - 1)) >> (LIMB_T_BITS - 1);   }

static inline bool_t vec_is_zero(const void *a, size_t num)
{
    const limb_t *ap = (const limb_t *)a;
    limb_t acc;
    size_t i;

    num /= sizeof(limb_t);

    for (acc = 0, i = 0; i < num; i++)
        acc |= ap[i];

    return is_zero(acc);
}

static inline bool_t vec_is_equal(const void *a, const void *b, size_t num)
{
    const limb_t *ap = (const limb_t *)a;
    const limb_t *bp = (const limb_t *)b;
    limb_t acc;
    size_t i;

    num /= sizeof(limb_t);

    for (acc = 0, i = 0; i < num; i++)
        acc |= ap[i] ^ bp[i];

    return is_zero(acc);
}

static inline void cneg_mod_384x(vec384x ret, const vec384x a, bool_t flag,
                                 const vec384 p)
{
    cneg_mod_384(ret[0], a[0], flag, p);
    cneg_mod_384(ret[1], a[1], flag, p);
}

static inline void vec_copy(void *restrict ret, const void *a, size_t num)
{
    limb_t *rp = (limb_t *)ret;
    const limb_t *ap = (const limb_t *)a;
    size_t i;

    num /= sizeof(limb_t);

    for (i = 0; i < num; i++)
        rp[i] = ap[i];
}

static inline void vec_zero(void *ret, size_t num)
{
    volatile limb_t *rp = (volatile limb_t *)ret;
    size_t i;

    num /= sizeof(limb_t);

    for (i = 0; i < num; i++)
        rp[i] = 0;

#if defined(__GNUC__) && !defined(__NVCC__)
    asm volatile("" : : "r"(ret) : "memory");
#endif
}

static inline void limbs_from_be_bytes(limb_t *restrict ret,
                                       const unsigned char *in, size_t n)
{
    limb_t limb = 0;

    while(n--) {
        limb <<= 8;
        limb |= *in++;
        /*
         * 'if (n % sizeof(limb_t) == 0)' is omitted because it's cheaper
         * to perform redundant stores than to pay penalty for
         * mispredicted branch. Besides, some compilers unroll the
         * loop and remove redundant stores to 'restict'-ed storage...
         */
        ret[n / sizeof(limb_t)] = limb;
    }
}

static inline void be_bytes_from_limbs(unsigned char *out, const limb_t *in,
                                       size_t n)
{
    limb_t limb;

    while(n--) {
        limb = in[n / sizeof(limb_t)];
        *out++ = (unsigned char)(limb >> (8 * (n % sizeof(limb_t))));
    }
}

static inline void limbs_from_le_bytes(limb_t *restrict ret,
                                       const unsigned char *in, size_t n)
{
    limb_t limb = 0;

    while(n--) {
        limb <<= 8;
        limb |= in[n];
        /*
         * 'if (n % sizeof(limb_t) == 0)' is omitted because it's cheaper
         * to perform redundant stores than to pay penalty for
         * mispredicted branch. Besides, some compilers unroll the
         * loop and remove redundant stores to 'restict'-ed storage...
         */
        ret[n / sizeof(limb_t)] = limb;
    }
}

static inline void le_bytes_from_limbs(unsigned char *out, const limb_t *in,
                                       size_t n)
{
    const union {
        long one;
        char little;
    } is_endian = { 1 };
    limb_t limb;
    size_t i, j, r;

    if ((uptr_t)out == (uptr_t)in && is_endian.little)
        return;

    r = n % sizeof(limb_t);
    n /= sizeof(limb_t);

    for(i = 0; i < n; i++) {
        for (limb = in[i], j = 0; j < sizeof(limb_t); j++, limb >>= 8)
            *out++ = (unsigned char)limb;
    }
    if (r) {
        for (limb = in[i], j = 0; j < r; j++, limb >>= 8)
            *out++ = (unsigned char)limb;
    }
}

/*
 * Some compilers get arguably overzealous(*) when passing pointer to
 * multi-dimensional array [such as vec384x] as 'const' argument.
 * General direction seems to be to legitimize such constification,
 * so it's argued that suppressing the warning is appropriate.
 *
 * (*)  http://www.open-std.org/jtc1/sc22/wg14/www/docs/n1923.htm
 */
#if defined(__INTEL_COMPILER)
# pragma warning(disable:167)
# pragma warning(disable:556)
#elif defined(__GNUC__) && !defined(__clang__)
# pragma GCC diagnostic ignored "-Wpedantic"
#elif defined(_MSC_VER)
# pragma warning(disable: 4127 4189)
#endif

#if !defined(__wasm__)
# include <stdlib.h>
#endif

#if defined(__GNUC__)
# ifndef alloca
#  define alloca(s) __builtin_alloca(s)
# endif
#elif defined(__sun)
# include <alloca.h>
#elif defined(_WIN32)
# include <malloc.h>
# ifndef alloca
#  define alloca(s) _alloca(s)
# endif
#endif

#endif /* __BLS12_381_ASM_VECT_H__ */