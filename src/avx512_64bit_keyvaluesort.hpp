/*******************************************************************
 * Copyright (C) 2022 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 * Authors: Raghuveer Devulapalli <raghuveer.devulapalli@intel.com>
 * ****************************************************************/

#ifndef AVX512_QSORT_64BIT_KV
#define AVX512_QSORT_64BIT_KV

#include "avx512-common-keyvaluesort.h"

/*
 * Constants used in sorting 8 elements in a ZMM registers. Based on Bitonic
 * sorting network (see
 * https://en.wikipedia.org/wiki/Bitonic_sorter#/media/File:BitonicSort.svg)
 */
// ZMM                  7, 6, 5, 4, 3, 2, 1, 0
#define NETWORK_64BIT_1 4, 5, 6, 7, 0, 1, 2, 3
#define NETWORK_64BIT_2 0, 1, 2, 3, 4, 5, 6, 7
#define NETWORK_64BIT_3 5, 4, 7, 6, 1, 0, 3, 2
#define NETWORK_64BIT_4 3, 2, 1, 0, 7, 6, 5, 4

template <>
struct zmm_kv_vector<int64_t> {
    using type_t = int64_t;
    using zmm_t = __m512i;
    using ymm_t = __m512i;
    using opmask_t = __mmask8;
    static const uint8_t numlanes = 8;

    static type_t type_max()
    {
        return X86_SIMD_SORT_MAX_INT64;
    }
    static type_t type_min()
    {
        return X86_SIMD_SORT_MIN_INT64;
    }
    static zmm_t zmm_max()
    {
        return _mm512_set1_epi64(type_max());
    } // TODO: this should broadcast bits as is?

    static zmm_t set(type_t v1,
                     type_t v2,
                     type_t v3,
                     type_t v4,
                     type_t v5,
                     type_t v6,
                     type_t v7,
                     type_t v8)
    {
        return _mm512_set_epi64(v1, v2, v3, v4, v5, v6, v7, v8);
    }

    static opmask_t knot_opmask(opmask_t x)
    {
        return _knot_mask8(x);
    }
    static opmask_t eq(zmm_t x, zmm_t y)
    {
        return _mm512_cmp_epi64_mask(x, y, _MM_CMPINT_EQ);
    }
    static opmask_t ge(zmm_t x, zmm_t y)
    {
        return _mm512_cmp_epi64_mask(x, y, _MM_CMPINT_NLT);
    }
    template <int scale>
    static zmm_t i64gather(__m512i index, void const *base)
    {
        return _mm512_i64gather_epi64(index, base, scale);
    }
    static zmm_t loadu(void const *mem)
    {
        return _mm512_loadu_si512(mem);
    }
    static zmm_t max(zmm_t x, zmm_t y)
    {
        return _mm512_max_epi64(x, y);
    }
    static void mask_compressstoreu(void *mem, opmask_t mask, zmm_t x)
    {
        return _mm512_mask_compressstoreu_epi64(mem, mask, x);
    }
    static zmm_t mask_loadu(zmm_t x, opmask_t mask, void const *mem)
    {
        return _mm512_mask_loadu_epi64(x, mask, mem);
    }
    static zmm_t mask_mov(zmm_t x, opmask_t mask, zmm_t y)
    {
        return _mm512_mask_mov_epi64(x, mask, y);
    }
    static void mask_storeu(void *mem, opmask_t mask, zmm_t x)
    {
        return _mm512_mask_storeu_epi64(mem, mask, x);
    }
    static zmm_t min(zmm_t x, zmm_t y)
    {
        return _mm512_min_epi64(x, y);
    }
    static zmm_t permutexvar(__m512i idx, zmm_t zmm)
    {
        return _mm512_permutexvar_epi64(idx, zmm);
    }
    static type_t reducemax(zmm_t v)
    {
        return _mm512_reduce_max_epi64(v);
    }
    static type_t reducemin(zmm_t v)
    {
        return _mm512_reduce_min_epi64(v);
    }
    static zmm_t set1(type_t v)
    {
        return _mm512_set1_epi64(v);
    }
    template <uint8_t mask>
    static zmm_t shuffle(zmm_t zmm)
    {
        __m512d temp = _mm512_castsi512_pd(zmm);
        return _mm512_castpd_si512(
                _mm512_shuffle_pd(temp, temp, (_MM_PERM_ENUM)mask));
    }
    static void storeu(void *mem, zmm_t x)
    {
        return _mm512_storeu_si512(mem, x);
    }
};
template <>
struct zmm_kv_vector<uint64_t> {
    using type_t = uint64_t;
    using zmm_t = __m512i;
    using ymm_t = __m512i;
    using opmask_t = __mmask8;
    static const uint8_t numlanes = 8;

    static type_t type_max()
    {
        return X86_SIMD_SORT_MAX_UINT64;
    }
    static type_t type_min()
    {
        return 0;
    }
    static zmm_t zmm_max()
    {
        return _mm512_set1_epi64(type_max());
    }

    static zmm_t set(type_t v1,
                     type_t v2,
                     type_t v3,
                     type_t v4,
                     type_t v5,
                     type_t v6,
                     type_t v7,
                     type_t v8)
    {
        return _mm512_set_epi64(v1, v2, v3, v4, v5, v6, v7, v8);
    }

    template <int scale>
    static zmm_t i64gather(__m512i index, void const *base)
    {
        return _mm512_i64gather_epi64(index, base, scale);
    }
    static opmask_t knot_opmask(opmask_t x)
    {
        return _knot_mask8(x);
    }
    static opmask_t eq(zmm_t x, zmm_t y)
    {
        return _mm512_cmp_epu64_mask(x, y, _MM_CMPINT_EQ);
    }
    static opmask_t ge(zmm_t x, zmm_t y)
    {
        return _mm512_cmp_epu64_mask(x, y, _MM_CMPINT_NLT);
    }
    static zmm_t loadu(void const *mem)
    {
        return _mm512_loadu_si512(mem);
    }
    static zmm_t max(zmm_t x, zmm_t y)
    {
        return _mm512_max_epu64(x, y);
    }
    static void mask_compressstoreu(void *mem, opmask_t mask, zmm_t x)
    {
        return _mm512_mask_compressstoreu_epi64(mem, mask, x);
    }
    static zmm_t mask_loadu(zmm_t x, opmask_t mask, void const *mem)
    {
        return _mm512_mask_loadu_epi64(x, mask, mem);
    }
    static zmm_t mask_mov(zmm_t x, opmask_t mask, zmm_t y)
    {
        return _mm512_mask_mov_epi64(x, mask, y);
    }
    static void mask_storeu(void *mem, opmask_t mask, zmm_t x)
    {
        return _mm512_mask_storeu_epi64(mem, mask, x);
    }
    static zmm_t min(zmm_t x, zmm_t y)
    {
        return _mm512_min_epu64(x, y);
    }
    static zmm_t permutexvar(__m512i idx, zmm_t zmm)
    {
        return _mm512_permutexvar_epi64(idx, zmm);
    }
    static type_t reducemax(zmm_t v)
    {
        return _mm512_reduce_max_epu64(v);
    }
    static type_t reducemin(zmm_t v)
    {
        return _mm512_reduce_min_epu64(v);
    }
    static zmm_t set1(type_t v)
    {
        return _mm512_set1_epi64(v);
    }
    template <uint8_t mask>
    static zmm_t shuffle(zmm_t zmm)
    {
        __m512d temp = _mm512_castsi512_pd(zmm);
        return _mm512_castpd_si512(
                _mm512_shuffle_pd(temp, temp, (_MM_PERM_ENUM)mask));
    }
    static void storeu(void *mem, zmm_t x)
    {
        return _mm512_storeu_si512(mem, x);
    }
};
template <>
struct zmm_kv_vector<double> {
    using type_t = double;
    using zmm_t = __m512d;
    using ymm_t = __m512d;
    using opmask_t = __mmask8;
    static const uint8_t numlanes = 8;

    static type_t type_max()
    {
        return X86_SIMD_SORT_INFINITY;
    }
    static type_t type_min()
    {
        return -X86_SIMD_SORT_INFINITY;
    }
    static zmm_t zmm_max()
    {
        return _mm512_set1_pd(type_max());
    }

    static zmm_t set(type_t v1,
                     type_t v2,
                     type_t v3,
                     type_t v4,
                     type_t v5,
                     type_t v6,
                     type_t v7,
                     type_t v8)
    {
        return _mm512_set_pd(v1, v2, v3, v4, v5, v6, v7, v8);
    }

    static opmask_t knot_opmask(opmask_t x)
    {
        return _knot_mask8(x);
    }
    static opmask_t eq(zmm_t x, zmm_t y)
    {
        return _mm512_cmp_pd_mask(x, y, _CMP_EQ_OS);
    }
    static opmask_t ge(zmm_t x, zmm_t y)
    {
        return _mm512_cmp_pd_mask(x, y, _CMP_GE_OQ);
    }
    template <int scale>
    static zmm_t i64gather(__m512i index, void const *base)
    {
        return _mm512_i64gather_pd(index, base, scale);
    }
    static zmm_t loadu(void const *mem)
    {
        return _mm512_loadu_pd(mem);
    }
    static zmm_t max(zmm_t x, zmm_t y)
    {
        return _mm512_max_pd(x, y);
    }
    static void mask_compressstoreu(void *mem, opmask_t mask, zmm_t x)
    {
        return _mm512_mask_compressstoreu_pd(mem, mask, x);
    }
    static zmm_t mask_loadu(zmm_t x, opmask_t mask, void const *mem)
    {
        return _mm512_mask_loadu_pd(x, mask, mem);
    }
    static zmm_t mask_mov(zmm_t x, opmask_t mask, zmm_t y)
    {
        return _mm512_mask_mov_pd(x, mask, y);
    }
    static void mask_storeu(void *mem, opmask_t mask, zmm_t x)
    {
        return _mm512_mask_storeu_pd(mem, mask, x);
    }
    static zmm_t min(zmm_t x, zmm_t y)
    {
        return _mm512_min_pd(x, y);
    }
    static zmm_t permutexvar(__m512i idx, zmm_t zmm)
    {
        return _mm512_permutexvar_pd(idx, zmm);
    }
    static type_t reducemax(zmm_t v)
    {
        return _mm512_reduce_max_pd(v);
    }
    static type_t reducemin(zmm_t v)
    {
        return _mm512_reduce_min_pd(v);
    }
    static zmm_t set1(type_t v)
    {
        return _mm512_set1_pd(v);
    }
    template <uint8_t mask>
    static zmm_t shuffle(zmm_t zmm)
    {
        return _mm512_shuffle_pd(zmm, zmm, (_MM_PERM_ENUM)mask);
    }
    static void storeu(void *mem, zmm_t x)
    {
        return _mm512_storeu_pd(mem, x);
    }
};

/*
 * Assumes zmm is random and performs a full sorting network defined in
 * https://en.wikipedia.org/wiki/Bitonic_sorter#/media/File:BitonicSort.svg
 */

template <typename vtype,
          typename zmm_t = typename vtype::zmm_t,
          typename index_t = zmm_kv_vector<uint64_t>::zmm_t>
X86_SIMD_SORT_INLINE zmm_t sort_zmm_64bit(zmm_t key_zmm, index_t &index_zmm)
{
    const __m512i rev_index = _mm512_set_epi64(NETWORK_64BIT_2);
    key_zmm = cmp_merge<vtype>(
            key_zmm,
            vtype::template shuffle<SHUFFLE_MASK(1, 1, 1, 1)>(key_zmm),
            index_zmm,
            zmm_kv_vector<uint64_t>::template shuffle<SHUFFLE_MASK(1, 1, 1, 1)>(
                    index_zmm),
            0xAA);
    key_zmm = cmp_merge<vtype>(
            key_zmm,
            vtype::permutexvar(_mm512_set_epi64(NETWORK_64BIT_1), key_zmm),
            index_zmm,
            zmm_kv_vector<uint64_t>::permutexvar(
                    _mm512_set_epi64(NETWORK_64BIT_1), index_zmm),
            0xCC);
    key_zmm = cmp_merge<vtype>(
            key_zmm,
            vtype::template shuffle<SHUFFLE_MASK(1, 1, 1, 1)>(key_zmm),
            index_zmm,
            zmm_kv_vector<uint64_t>::template shuffle<SHUFFLE_MASK(1, 1, 1, 1)>(
                    index_zmm),
            0xAA);
    key_zmm = cmp_merge<vtype>(
            key_zmm,
            vtype::permutexvar(rev_index, key_zmm),
            index_zmm,
            zmm_kv_vector<uint64_t>::permutexvar(rev_index, index_zmm),
            0xF0);
    key_zmm = cmp_merge<vtype>(
            key_zmm,
            vtype::permutexvar(_mm512_set_epi64(NETWORK_64BIT_3), key_zmm),
            index_zmm,
            zmm_kv_vector<uint64_t>::permutexvar(
                    _mm512_set_epi64(NETWORK_64BIT_3), index_zmm),
            0xCC);
    key_zmm = cmp_merge<vtype>(
            key_zmm,
            vtype::template shuffle<SHUFFLE_MASK(1, 1, 1, 1)>(key_zmm),
            index_zmm,
            zmm_kv_vector<uint64_t>::template shuffle<SHUFFLE_MASK(1, 1, 1, 1)>(
                    index_zmm),
            0xAA);
    return key_zmm;
}
// Assumes zmm is bitonic and performs a recursive half cleaner
template <typename vtype,
          typename zmm_t = typename vtype::zmm_t,
          typename index_t = zmm_kv_vector<uint64_t>::zmm_t>
X86_SIMD_SORT_INLINE zmm_t bitonic_merge_zmm_64bit(
        zmm_t key_zmm, zmm_kv_vector<uint64_t>::zmm_t &index_zmm)
{

    // 1) half_cleaner[8]: compare 0-4, 1-5, 2-6, 3-7
    key_zmm = cmp_merge<vtype>(
            key_zmm,
            vtype::permutexvar(_mm512_set_epi64(NETWORK_64BIT_4), key_zmm),
            index_zmm,
            zmm_kv_vector<uint64_t>::permutexvar(
                    _mm512_set_epi64(NETWORK_64BIT_4), index_zmm),
            0xF0);
    // 2) half_cleaner[4]
    key_zmm = cmp_merge<vtype>(
            key_zmm,
            vtype::permutexvar(_mm512_set_epi64(NETWORK_64BIT_3), key_zmm),
            index_zmm,
            zmm_kv_vector<uint64_t>::permutexvar(
                    _mm512_set_epi64(NETWORK_64BIT_3), index_zmm),
            0xCC);
    // 3) half_cleaner[1]
    key_zmm = cmp_merge<vtype>(
            key_zmm,
            vtype::template shuffle<SHUFFLE_MASK(1, 1, 1, 1)>(key_zmm),
            index_zmm,
            zmm_kv_vector<uint64_t>::template shuffle<SHUFFLE_MASK(1, 1, 1, 1)>(
                    index_zmm),
            0xAA);
    return key_zmm;
}
// Assumes zmm1 and zmm2 are sorted and performs a recursive half cleaner
template <typename vtype,
          typename zmm_t = typename vtype::zmm_t,
          typename index_t = zmm_kv_vector<uint64_t>::zmm_t>
X86_SIMD_SORT_INLINE void bitonic_merge_two_zmm_64bit(zmm_t &key_zmm1,
                                                      zmm_t &key_zmm2,
                                                      index_t &index_zmm1,
                                                      index_t &index_zmm2)
{
    const __m512i rev_index = _mm512_set_epi64(NETWORK_64BIT_2);
    // 1) First step of a merging network: coex of zmm1 and zmm2 reversed
    key_zmm2 = vtype::permutexvar(rev_index, key_zmm2);
    index_zmm2 = zmm_kv_vector<uint64_t>::permutexvar(rev_index, index_zmm2);

    zmm_t key_zmm3 = vtype::min(key_zmm1, key_zmm2);
    zmm_t key_zmm4 = vtype::max(key_zmm1, key_zmm2);

    index_t index_zmm3 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm2, vtype::eq(key_zmm3, key_zmm1), index_zmm1);
    index_t index_zmm4 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm1, vtype::eq(key_zmm3, key_zmm1), index_zmm2);

    // 2) Recursive half cleaner for each
    key_zmm1 = bitonic_merge_zmm_64bit<vtype>(key_zmm3, index_zmm3);
    key_zmm2 = bitonic_merge_zmm_64bit<vtype>(key_zmm4, index_zmm4);
    index_zmm1 = index_zmm3;
    index_zmm2 = index_zmm4;
}
// Assumes [zmm0, zmm1] and [zmm2, zmm3] are sorted and performs a recursive
// half cleaner
template <typename vtype,
          typename zmm_t = typename vtype::zmm_t,
          typename index_t = zmm_kv_vector<uint64_t>::zmm_t>
X86_SIMD_SORT_INLINE void bitonic_merge_four_zmm_64bit(zmm_t *key_zmm,
                                                       index_t *index_zmm)
{
    const __m512i rev_index = _mm512_set_epi64(NETWORK_64BIT_2);
    // 1) First step of a merging network
    zmm_t key_zmm2r = vtype::permutexvar(rev_index, key_zmm[2]);
    zmm_t key_zmm3r = vtype::permutexvar(rev_index, key_zmm[3]);
    index_t index_zmm2r
            = zmm_kv_vector<uint64_t>::permutexvar(rev_index, index_zmm[2]);
    index_t index_zmm3r
            = zmm_kv_vector<uint64_t>::permutexvar(rev_index, index_zmm[3]);

    zmm_t key_zmm_t1 = vtype::min(key_zmm[0], key_zmm3r);
    zmm_t key_zmm_t2 = vtype::min(key_zmm[1], key_zmm2r);
    zmm_t key_zmm_m1 = vtype::max(key_zmm[0], key_zmm3r);
    zmm_t key_zmm_m2 = vtype::max(key_zmm[1], key_zmm2r);

    index_t index_zmm_t1 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm3r, vtype::eq(key_zmm_t1, key_zmm[0]), index_zmm[0]);
    index_t index_zmm_m1 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm[0], vtype::eq(key_zmm_t1, key_zmm[0]), index_zmm3r);
    index_t index_zmm_t2 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm2r, vtype::eq(key_zmm_t2, key_zmm[1]), index_zmm[1]);
    index_t index_zmm_m2 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm[1], vtype::eq(key_zmm_t2, key_zmm[1]), index_zmm2r);

    // 2) Recursive half clearer: 16
    zmm_t key_zmm_t3 = vtype::permutexvar(rev_index, key_zmm_m2);
    zmm_t key_zmm_t4 = vtype::permutexvar(rev_index, key_zmm_m1);
    index_t index_zmm_t3
            = zmm_kv_vector<uint64_t>::permutexvar(rev_index, index_zmm_m2);
    index_t index_zmm_t4
            = zmm_kv_vector<uint64_t>::permutexvar(rev_index, index_zmm_m1);

    zmm_t key_zmm0 = vtype::min(key_zmm_t1, key_zmm_t2);
    zmm_t key_zmm1 = vtype::max(key_zmm_t1, key_zmm_t2);
    zmm_t key_zmm2 = vtype::min(key_zmm_t3, key_zmm_t4);
    zmm_t key_zmm3 = vtype::max(key_zmm_t3, key_zmm_t4);

    index_t index_zmm0 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm_t2, vtype::eq(key_zmm0, key_zmm_t1), index_zmm_t1);
    index_t index_zmm1 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm_t1, vtype::eq(key_zmm0, key_zmm_t1), index_zmm_t2);
    index_t index_zmm2 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm_t4, vtype::eq(key_zmm2, key_zmm_t3), index_zmm_t3);
    index_t index_zmm3 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm_t3, vtype::eq(key_zmm2, key_zmm_t3), index_zmm_t4);

    key_zmm[0] = bitonic_merge_zmm_64bit<vtype>(key_zmm0, index_zmm0);
    key_zmm[1] = bitonic_merge_zmm_64bit<vtype>(key_zmm1, index_zmm1);
    key_zmm[2] = bitonic_merge_zmm_64bit<vtype>(key_zmm2, index_zmm2);
    key_zmm[3] = bitonic_merge_zmm_64bit<vtype>(key_zmm3, index_zmm3);

    index_zmm[0] = index_zmm0;
    index_zmm[1] = index_zmm1;
    index_zmm[2] = index_zmm2;
    index_zmm[3] = index_zmm3;
}
template <typename vtype,
          typename zmm_t = typename vtype::zmm_t,
          typename index_t = zmm_kv_vector<uint64_t>::zmm_t>
X86_SIMD_SORT_INLINE void bitonic_merge_eight_zmm_64bit(zmm_t *key_zmm,
                                                        index_t *index_zmm)
{
    const __m512i rev_index = _mm512_set_epi64(NETWORK_64BIT_2);
    zmm_t key_zmm4r = vtype::permutexvar(rev_index, key_zmm[4]);
    zmm_t key_zmm5r = vtype::permutexvar(rev_index, key_zmm[5]);
    zmm_t key_zmm6r = vtype::permutexvar(rev_index, key_zmm[6]);
    zmm_t key_zmm7r = vtype::permutexvar(rev_index, key_zmm[7]);
    index_t index_zmm4r
            = zmm_kv_vector<uint64_t>::permutexvar(rev_index, index_zmm[4]);
    index_t index_zmm5r
            = zmm_kv_vector<uint64_t>::permutexvar(rev_index, index_zmm[5]);
    index_t index_zmm6r
            = zmm_kv_vector<uint64_t>::permutexvar(rev_index, index_zmm[6]);
    index_t index_zmm7r
            = zmm_kv_vector<uint64_t>::permutexvar(rev_index, index_zmm[7]);

    zmm_t key_zmm_t1 = vtype::min(key_zmm[0], key_zmm7r);
    zmm_t key_zmm_t2 = vtype::min(key_zmm[1], key_zmm6r);
    zmm_t key_zmm_t3 = vtype::min(key_zmm[2], key_zmm5r);
    zmm_t key_zmm_t4 = vtype::min(key_zmm[3], key_zmm4r);

    zmm_t key_zmm_m1 = vtype::max(key_zmm[0], key_zmm7r);
    zmm_t key_zmm_m2 = vtype::max(key_zmm[1], key_zmm6r);
    zmm_t key_zmm_m3 = vtype::max(key_zmm[2], key_zmm5r);
    zmm_t key_zmm_m4 = vtype::max(key_zmm[3], key_zmm4r);

    index_t index_zmm_t1 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm7r, vtype::eq(key_zmm_t1, key_zmm[0]), index_zmm[0]);
    index_t index_zmm_m1 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm[0], vtype::eq(key_zmm_t1, key_zmm[0]), index_zmm7r);
    index_t index_zmm_t2 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm6r, vtype::eq(key_zmm_t2, key_zmm[1]), index_zmm[1]);
    index_t index_zmm_m2 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm[1], vtype::eq(key_zmm_t2, key_zmm[1]), index_zmm6r);
    index_t index_zmm_t3 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm5r, vtype::eq(key_zmm_t3, key_zmm[2]), index_zmm[2]);
    index_t index_zmm_m3 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm[2], vtype::eq(key_zmm_t3, key_zmm[2]), index_zmm5r);
    index_t index_zmm_t4 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm4r, vtype::eq(key_zmm_t4, key_zmm[3]), index_zmm[3]);
    index_t index_zmm_m4 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm[3], vtype::eq(key_zmm_t4, key_zmm[3]), index_zmm4r);

    zmm_t key_zmm_t5 = vtype::permutexvar(rev_index, key_zmm_m4);
    zmm_t key_zmm_t6 = vtype::permutexvar(rev_index, key_zmm_m3);
    zmm_t key_zmm_t7 = vtype::permutexvar(rev_index, key_zmm_m2);
    zmm_t key_zmm_t8 = vtype::permutexvar(rev_index, key_zmm_m1);
    index_t index_zmm_t5
            = zmm_kv_vector<uint64_t>::permutexvar(rev_index, index_zmm_m4);
    index_t index_zmm_t6
            = zmm_kv_vector<uint64_t>::permutexvar(rev_index, index_zmm_m3);
    index_t index_zmm_t7
            = zmm_kv_vector<uint64_t>::permutexvar(rev_index, index_zmm_m2);
    index_t index_zmm_t8
            = zmm_kv_vector<uint64_t>::permutexvar(rev_index, index_zmm_m1);

    COEX<vtype>(key_zmm_t1, key_zmm_t3, index_zmm_t1, index_zmm_t3);
    COEX<vtype>(key_zmm_t2, key_zmm_t4, index_zmm_t2, index_zmm_t4);
    COEX<vtype>(key_zmm_t5, key_zmm_t7, index_zmm_t5, index_zmm_t7);
    COEX<vtype>(key_zmm_t6, key_zmm_t8, index_zmm_t6, index_zmm_t8);
    COEX<vtype>(key_zmm_t1, key_zmm_t2, index_zmm_t1, index_zmm_t2);
    COEX<vtype>(key_zmm_t3, key_zmm_t4, index_zmm_t3, index_zmm_t4);
    COEX<vtype>(key_zmm_t5, key_zmm_t6, index_zmm_t5, index_zmm_t6);
    COEX<vtype>(key_zmm_t7, key_zmm_t8, index_zmm_t7, index_zmm_t8);
    key_zmm[0] = bitonic_merge_zmm_64bit<vtype>(key_zmm_t1, index_zmm_t1);
    key_zmm[1] = bitonic_merge_zmm_64bit<vtype>(key_zmm_t2, index_zmm_t2);
    key_zmm[2] = bitonic_merge_zmm_64bit<vtype>(key_zmm_t3, index_zmm_t3);
    key_zmm[3] = bitonic_merge_zmm_64bit<vtype>(key_zmm_t4, index_zmm_t4);
    key_zmm[4] = bitonic_merge_zmm_64bit<vtype>(key_zmm_t5, index_zmm_t5);
    key_zmm[5] = bitonic_merge_zmm_64bit<vtype>(key_zmm_t6, index_zmm_t6);
    key_zmm[6] = bitonic_merge_zmm_64bit<vtype>(key_zmm_t7, index_zmm_t7);
    key_zmm[7] = bitonic_merge_zmm_64bit<vtype>(key_zmm_t8, index_zmm_t8);

    index_zmm[0] = index_zmm_t1;
    index_zmm[1] = index_zmm_t2;
    index_zmm[2] = index_zmm_t3;
    index_zmm[3] = index_zmm_t4;
    index_zmm[4] = index_zmm_t5;
    index_zmm[5] = index_zmm_t6;
    index_zmm[6] = index_zmm_t7;
    index_zmm[7] = index_zmm_t8;
}
template <typename vtype,
          typename zmm_t = typename vtype::zmm_t,
          typename index_t = zmm_kv_vector<uint64_t>::zmm_t>
X86_SIMD_SORT_INLINE void bitonic_merge_sixteen_zmm_64bit(zmm_t *key_zmm,
                                                          index_t *index_zmm)
{
    const __m512i rev_index = _mm512_set_epi64(NETWORK_64BIT_2);
    zmm_t key_zmm8r = vtype::permutexvar(rev_index, key_zmm[8]);
    zmm_t key_zmm9r = vtype::permutexvar(rev_index, key_zmm[9]);
    zmm_t key_zmm10r = vtype::permutexvar(rev_index, key_zmm[10]);
    zmm_t key_zmm11r = vtype::permutexvar(rev_index, key_zmm[11]);
    zmm_t key_zmm12r = vtype::permutexvar(rev_index, key_zmm[12]);
    zmm_t key_zmm13r = vtype::permutexvar(rev_index, key_zmm[13]);
    zmm_t key_zmm14r = vtype::permutexvar(rev_index, key_zmm[14]);
    zmm_t key_zmm15r = vtype::permutexvar(rev_index, key_zmm[15]);

    index_t index_zmm8r
            = zmm_kv_vector<uint64_t>::permutexvar(rev_index, index_zmm[8]);
    index_t index_zmm9r
            = zmm_kv_vector<uint64_t>::permutexvar(rev_index, index_zmm[9]);
    index_t index_zmm10r
            = zmm_kv_vector<uint64_t>::permutexvar(rev_index, index_zmm[10]);
    index_t index_zmm11r
            = zmm_kv_vector<uint64_t>::permutexvar(rev_index, index_zmm[11]);
    index_t index_zmm12r
            = zmm_kv_vector<uint64_t>::permutexvar(rev_index, index_zmm[12]);
    index_t index_zmm13r
            = zmm_kv_vector<uint64_t>::permutexvar(rev_index, index_zmm[13]);
    index_t index_zmm14r
            = zmm_kv_vector<uint64_t>::permutexvar(rev_index, index_zmm[14]);
    index_t index_zmm15r
            = zmm_kv_vector<uint64_t>::permutexvar(rev_index, index_zmm[15]);

    zmm_t key_zmm_t1 = vtype::min(key_zmm[0], key_zmm15r);
    zmm_t key_zmm_t2 = vtype::min(key_zmm[1], key_zmm14r);
    zmm_t key_zmm_t3 = vtype::min(key_zmm[2], key_zmm13r);
    zmm_t key_zmm_t4 = vtype::min(key_zmm[3], key_zmm12r);
    zmm_t key_zmm_t5 = vtype::min(key_zmm[4], key_zmm11r);
    zmm_t key_zmm_t6 = vtype::min(key_zmm[5], key_zmm10r);
    zmm_t key_zmm_t7 = vtype::min(key_zmm[6], key_zmm9r);
    zmm_t key_zmm_t8 = vtype::min(key_zmm[7], key_zmm8r);

    zmm_t key_zmm_m1 = vtype::max(key_zmm[0], key_zmm15r);
    zmm_t key_zmm_m2 = vtype::max(key_zmm[1], key_zmm14r);
    zmm_t key_zmm_m3 = vtype::max(key_zmm[2], key_zmm13r);
    zmm_t key_zmm_m4 = vtype::max(key_zmm[3], key_zmm12r);
    zmm_t key_zmm_m5 = vtype::max(key_zmm[4], key_zmm11r);
    zmm_t key_zmm_m6 = vtype::max(key_zmm[5], key_zmm10r);
    zmm_t key_zmm_m7 = vtype::max(key_zmm[6], key_zmm9r);
    zmm_t key_zmm_m8 = vtype::max(key_zmm[7], key_zmm8r);

    index_t index_zmm_t1 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm15r, vtype::eq(key_zmm_t1, key_zmm[0]), index_zmm[0]);
    index_t index_zmm_m1 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm[0], vtype::eq(key_zmm_t1, key_zmm[0]), index_zmm15r);
    index_t index_zmm_t2 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm14r, vtype::eq(key_zmm_t2, key_zmm[1]), index_zmm[1]);
    index_t index_zmm_m2 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm[1], vtype::eq(key_zmm_t2, key_zmm[1]), index_zmm14r);
    index_t index_zmm_t3 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm13r, vtype::eq(key_zmm_t3, key_zmm[2]), index_zmm[2]);
    index_t index_zmm_m3 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm[2], vtype::eq(key_zmm_t3, key_zmm[2]), index_zmm13r);
    index_t index_zmm_t4 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm12r, vtype::eq(key_zmm_t4, key_zmm[3]), index_zmm[3]);
    index_t index_zmm_m4 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm[3], vtype::eq(key_zmm_t4, key_zmm[3]), index_zmm12r);

    index_t index_zmm_t5 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm11r, vtype::eq(key_zmm_t5, key_zmm[4]), index_zmm[4]);
    index_t index_zmm_m5 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm[4], vtype::eq(key_zmm_t5, key_zmm[4]), index_zmm11r);
    index_t index_zmm_t6 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm10r, vtype::eq(key_zmm_t6, key_zmm[5]), index_zmm[5]);
    index_t index_zmm_m6 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm[5], vtype::eq(key_zmm_t6, key_zmm[5]), index_zmm10r);
    index_t index_zmm_t7 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm9r, vtype::eq(key_zmm_t7, key_zmm[6]), index_zmm[6]);
    index_t index_zmm_m7 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm[6], vtype::eq(key_zmm_t7, key_zmm[6]), index_zmm9r);
    index_t index_zmm_t8 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm8r, vtype::eq(key_zmm_t8, key_zmm[7]), index_zmm[7]);
    index_t index_zmm_m8 = zmm_kv_vector<uint64_t>::mask_mov(
            index_zmm[7], vtype::eq(key_zmm_t8, key_zmm[7]), index_zmm8r);

    zmm_t key_zmm_t9 = vtype::permutexvar(rev_index, key_zmm_m8);
    zmm_t key_zmm_t10 = vtype::permutexvar(rev_index, key_zmm_m7);
    zmm_t key_zmm_t11 = vtype::permutexvar(rev_index, key_zmm_m6);
    zmm_t key_zmm_t12 = vtype::permutexvar(rev_index, key_zmm_m5);
    zmm_t key_zmm_t13 = vtype::permutexvar(rev_index, key_zmm_m4);
    zmm_t key_zmm_t14 = vtype::permutexvar(rev_index, key_zmm_m3);
    zmm_t key_zmm_t15 = vtype::permutexvar(rev_index, key_zmm_m2);
    zmm_t key_zmm_t16 = vtype::permutexvar(rev_index, key_zmm_m1);
    index_t index_zmm_t9
            = zmm_kv_vector<uint64_t>::permutexvar(rev_index, index_zmm_m8);
    index_t index_zmm_t10
            = zmm_kv_vector<uint64_t>::permutexvar(rev_index, index_zmm_m7);
    index_t index_zmm_t11
            = zmm_kv_vector<uint64_t>::permutexvar(rev_index, index_zmm_m6);
    index_t index_zmm_t12
            = zmm_kv_vector<uint64_t>::permutexvar(rev_index, index_zmm_m5);
    index_t index_zmm_t13
            = zmm_kv_vector<uint64_t>::permutexvar(rev_index, index_zmm_m4);
    index_t index_zmm_t14
            = zmm_kv_vector<uint64_t>::permutexvar(rev_index, index_zmm_m3);
    index_t index_zmm_t15
            = zmm_kv_vector<uint64_t>::permutexvar(rev_index, index_zmm_m2);
    index_t index_zmm_t16
            = zmm_kv_vector<uint64_t>::permutexvar(rev_index, index_zmm_m1);

    COEX<vtype>(key_zmm_t1, key_zmm_t5, index_zmm_t1, index_zmm_t5);
    COEX<vtype>(key_zmm_t2, key_zmm_t6, index_zmm_t2, index_zmm_t6);
    COEX<vtype>(key_zmm_t3, key_zmm_t7, index_zmm_t3, index_zmm_t7);
    COEX<vtype>(key_zmm_t4, key_zmm_t8, index_zmm_t4, index_zmm_t8);
    COEX<vtype>(key_zmm_t9, key_zmm_t13, index_zmm_t9, index_zmm_t13);
    COEX<vtype>(key_zmm_t10, key_zmm_t14, index_zmm_t10, index_zmm_t14);
    COEX<vtype>(key_zmm_t11, key_zmm_t15, index_zmm_t11, index_zmm_t15);
    COEX<vtype>(key_zmm_t12, key_zmm_t16, index_zmm_t12, index_zmm_t16);

    COEX<vtype>(key_zmm_t1, key_zmm_t3, index_zmm_t1, index_zmm_t3);
    COEX<vtype>(key_zmm_t2, key_zmm_t4, index_zmm_t2, index_zmm_t4);
    COEX<vtype>(key_zmm_t5, key_zmm_t7, index_zmm_t5, index_zmm_t7);
    COEX<vtype>(key_zmm_t6, key_zmm_t8, index_zmm_t6, index_zmm_t8);
    COEX<vtype>(key_zmm_t9, key_zmm_t11, index_zmm_t9, index_zmm_t11);
    COEX<vtype>(key_zmm_t10, key_zmm_t12, index_zmm_t10, index_zmm_t12);
    COEX<vtype>(key_zmm_t13, key_zmm_t15, index_zmm_t13, index_zmm_t15);
    COEX<vtype>(key_zmm_t14, key_zmm_t16, index_zmm_t14, index_zmm_t16);

    COEX<vtype>(key_zmm_t1, key_zmm_t2, index_zmm_t1, index_zmm_t2);
    COEX<vtype>(key_zmm_t3, key_zmm_t4, index_zmm_t3, index_zmm_t4);
    COEX<vtype>(key_zmm_t5, key_zmm_t6, index_zmm_t5, index_zmm_t6);
    COEX<vtype>(key_zmm_t7, key_zmm_t8, index_zmm_t7, index_zmm_t8);
    COEX<vtype>(key_zmm_t9, key_zmm_t10, index_zmm_t9, index_zmm_t10);
    COEX<vtype>(key_zmm_t11, key_zmm_t12, index_zmm_t11, index_zmm_t12);
    COEX<vtype>(key_zmm_t13, key_zmm_t14, index_zmm_t13, index_zmm_t14);
    COEX<vtype>(key_zmm_t15, key_zmm_t16, index_zmm_t15, index_zmm_t16);
    //
    key_zmm[0] = bitonic_merge_zmm_64bit<vtype>(key_zmm_t1, index_zmm_t1);
    key_zmm[1] = bitonic_merge_zmm_64bit<vtype>(key_zmm_t2, index_zmm_t2);
    key_zmm[2] = bitonic_merge_zmm_64bit<vtype>(key_zmm_t3, index_zmm_t3);
    key_zmm[3] = bitonic_merge_zmm_64bit<vtype>(key_zmm_t4, index_zmm_t4);
    key_zmm[4] = bitonic_merge_zmm_64bit<vtype>(key_zmm_t5, index_zmm_t5);
    key_zmm[5] = bitonic_merge_zmm_64bit<vtype>(key_zmm_t6, index_zmm_t6);
    key_zmm[6] = bitonic_merge_zmm_64bit<vtype>(key_zmm_t7, index_zmm_t7);
    key_zmm[7] = bitonic_merge_zmm_64bit<vtype>(key_zmm_t8, index_zmm_t8);
    key_zmm[8] = bitonic_merge_zmm_64bit<vtype>(key_zmm_t9, index_zmm_t9);
    key_zmm[9] = bitonic_merge_zmm_64bit<vtype>(key_zmm_t10, index_zmm_t10);
    key_zmm[10] = bitonic_merge_zmm_64bit<vtype>(key_zmm_t11, index_zmm_t11);
    key_zmm[11] = bitonic_merge_zmm_64bit<vtype>(key_zmm_t12, index_zmm_t12);
    key_zmm[12] = bitonic_merge_zmm_64bit<vtype>(key_zmm_t13, index_zmm_t13);
    key_zmm[13] = bitonic_merge_zmm_64bit<vtype>(key_zmm_t14, index_zmm_t14);
    key_zmm[14] = bitonic_merge_zmm_64bit<vtype>(key_zmm_t15, index_zmm_t15);
    key_zmm[15] = bitonic_merge_zmm_64bit<vtype>(key_zmm_t16, index_zmm_t16);

    index_zmm[0] = index_zmm_t1;
    index_zmm[1] = index_zmm_t2;
    index_zmm[2] = index_zmm_t3;
    index_zmm[3] = index_zmm_t4;
    index_zmm[4] = index_zmm_t5;
    index_zmm[5] = index_zmm_t6;
    index_zmm[6] = index_zmm_t7;
    index_zmm[7] = index_zmm_t8;
    index_zmm[8] = index_zmm_t9;
    index_zmm[9] = index_zmm_t10;
    index_zmm[10] = index_zmm_t11;
    index_zmm[11] = index_zmm_t12;
    index_zmm[12] = index_zmm_t13;
    index_zmm[13] = index_zmm_t14;
    index_zmm[14] = index_zmm_t15;
    index_zmm[15] = index_zmm_t16;
}
template <typename vtype, typename type_t>
X86_SIMD_SORT_INLINE void
sort_8_64bit(type_t *keys, uint64_t *indexes, int32_t N)
{
    typename vtype::opmask_t load_mask = (0x01 << N) - 0x01;
    typename vtype::zmm_t key_zmm
            = vtype::mask_loadu(vtype::zmm_max(), load_mask, keys);

    zmm_kv_vector<uint64_t>::zmm_t index_zmm
            = zmm_kv_vector<uint64_t>::mask_loadu(
                    zmm_kv_vector<uint64_t>::zmm_max(), load_mask, indexes);
    vtype::mask_storeu(
            keys, load_mask, sort_zmm_64bit<vtype>(key_zmm, index_zmm));
    zmm_kv_vector<uint64_t>::mask_storeu(indexes, load_mask, index_zmm);
}

template <typename vtype, typename type_t>
X86_SIMD_SORT_INLINE void
sort_16_64bit(type_t *keys, uint64_t *indexes, int32_t N)
{
    if (N <= 8) {
        sort_8_64bit<vtype>(keys, indexes, N);
        return;
    }
    using zmm_t = typename vtype::zmm_t;
    using index_t = zmm_kv_vector<uint64_t>::zmm_t;

    typename vtype::opmask_t load_mask = (0x01 << (N - 8)) - 0x01;

    zmm_t key_zmm1 = vtype::loadu(keys);
    zmm_t key_zmm2 = vtype::mask_loadu(vtype::zmm_max(), load_mask, keys + 8);

    index_t index_zmm1 = zmm_kv_vector<uint64_t>::loadu(indexes);
    index_t index_zmm2 = zmm_kv_vector<uint64_t>::mask_loadu(
            zmm_kv_vector<uint64_t>::zmm_max(), load_mask, indexes + 8);

    key_zmm1 = sort_zmm_64bit<vtype>(key_zmm1, index_zmm1);
    key_zmm2 = sort_zmm_64bit<vtype>(key_zmm2, index_zmm2);
    bitonic_merge_two_zmm_64bit<vtype>(
            key_zmm1, key_zmm2, index_zmm1, index_zmm2);

    zmm_kv_vector<uint64_t>::storeu(indexes, index_zmm1);
    zmm_kv_vector<uint64_t>::mask_storeu(indexes + 8, load_mask, index_zmm2);

    vtype::storeu(keys, key_zmm1);
    vtype::mask_storeu(keys + 8, load_mask, key_zmm2);
}

template <typename vtype, typename type_t>
X86_SIMD_SORT_INLINE void
sort_32_64bit(type_t *keys, uint64_t *indexes, int32_t N)
{
    if (N <= 16) {
        sort_16_64bit<vtype>(keys, indexes, N);
        return;
    }
    using zmm_t = typename vtype::zmm_t;
    using opmask_t = typename vtype::opmask_t;
    using index_t = zmm_kv_vector<uint64_t>::zmm_t;
    zmm_t key_zmm[4];
    index_t index_zmm[4];

    key_zmm[0] = vtype::loadu(keys);
    key_zmm[1] = vtype::loadu(keys + 8);

    index_zmm[0] = zmm_kv_vector<uint64_t>::loadu(indexes);
    index_zmm[1] = zmm_kv_vector<uint64_t>::loadu(indexes + 8);

    key_zmm[0] = sort_zmm_64bit<vtype>(key_zmm[0], index_zmm[0]);
    key_zmm[1] = sort_zmm_64bit<vtype>(key_zmm[1], index_zmm[1]);

    opmask_t load_mask1 = 0xFF, load_mask2 = 0xFF;
    uint64_t combined_mask = (0x1ull << (N - 16)) - 0x1ull;
    load_mask1 = (combined_mask)&0xFF;
    load_mask2 = (combined_mask >> 8) & 0xFF;
    key_zmm[2] = vtype::mask_loadu(vtype::zmm_max(), load_mask1, keys + 16);
    key_zmm[3] = vtype::mask_loadu(vtype::zmm_max(), load_mask2, keys + 24);

    index_zmm[2] = zmm_kv_vector<uint64_t>::mask_loadu(
            zmm_kv_vector<uint64_t>::zmm_max(), load_mask1, indexes + 16);
    index_zmm[3] = zmm_kv_vector<uint64_t>::mask_loadu(
            zmm_kv_vector<uint64_t>::zmm_max(), load_mask2, indexes + 24);

    key_zmm[2] = sort_zmm_64bit<vtype>(key_zmm[2], index_zmm[2]);
    key_zmm[3] = sort_zmm_64bit<vtype>(key_zmm[3], index_zmm[3]);

    bitonic_merge_two_zmm_64bit<vtype>(
            key_zmm[0], key_zmm[1], index_zmm[0], index_zmm[1]);
    bitonic_merge_two_zmm_64bit<vtype>(
            key_zmm[2], key_zmm[3], index_zmm[2], index_zmm[3]);
    bitonic_merge_four_zmm_64bit<vtype>(key_zmm, index_zmm);

    zmm_kv_vector<uint64_t>::storeu(indexes, index_zmm[0]);
    zmm_kv_vector<uint64_t>::storeu(indexes + 8, index_zmm[1]);
    zmm_kv_vector<uint64_t>::mask_storeu(
            indexes + 16, load_mask1, index_zmm[2]);
    zmm_kv_vector<uint64_t>::mask_storeu(
            indexes + 24, load_mask2, index_zmm[3]);

    vtype::storeu(keys, key_zmm[0]);
    vtype::storeu(keys + 8, key_zmm[1]);
    vtype::mask_storeu(keys + 16, load_mask1, key_zmm[2]);
    vtype::mask_storeu(keys + 24, load_mask2, key_zmm[3]);
}

template <typename vtype, typename type_t>
X86_SIMD_SORT_INLINE void
sort_64_64bit(type_t *keys, uint64_t *indexes, int32_t N)
{
    if (N <= 32) {
        sort_32_64bit<vtype>(keys, indexes, N);
        return;
    }
    using zmm_t = typename vtype::zmm_t;
    using opmask_t = typename vtype::opmask_t;
    using index_t = zmm_kv_vector<uint64_t>::zmm_t;
    zmm_t key_zmm[8];
    index_t index_zmm[8];

    key_zmm[0] = vtype::loadu(keys);
    key_zmm[1] = vtype::loadu(keys + 8);
    key_zmm[2] = vtype::loadu(keys + 16);
    key_zmm[3] = vtype::loadu(keys + 24);

    index_zmm[0] = zmm_kv_vector<uint64_t>::loadu(indexes);
    index_zmm[1] = zmm_kv_vector<uint64_t>::loadu(indexes + 8);
    index_zmm[2] = zmm_kv_vector<uint64_t>::loadu(indexes + 16);
    index_zmm[3] = zmm_kv_vector<uint64_t>::loadu(indexes + 24);
    key_zmm[0] = sort_zmm_64bit<vtype>(key_zmm[0], index_zmm[0]);
    key_zmm[1] = sort_zmm_64bit<vtype>(key_zmm[1], index_zmm[1]);
    key_zmm[2] = sort_zmm_64bit<vtype>(key_zmm[2], index_zmm[2]);
    key_zmm[3] = sort_zmm_64bit<vtype>(key_zmm[3], index_zmm[3]);

    opmask_t load_mask1 = 0xFF, load_mask2 = 0xFF;
    opmask_t load_mask3 = 0xFF, load_mask4 = 0xFF;
    // N-32 >= 1
    uint64_t combined_mask = (0x1ull << (N - 32)) - 0x1ull;
    load_mask1 = (combined_mask)&0xFF;
    load_mask2 = (combined_mask >> 8) & 0xFF;
    load_mask3 = (combined_mask >> 16) & 0xFF;
    load_mask4 = (combined_mask >> 24) & 0xFF;
    key_zmm[4] = vtype::mask_loadu(vtype::zmm_max(), load_mask1, keys + 32);
    key_zmm[5] = vtype::mask_loadu(vtype::zmm_max(), load_mask2, keys + 40);
    key_zmm[6] = vtype::mask_loadu(vtype::zmm_max(), load_mask3, keys + 48);
    key_zmm[7] = vtype::mask_loadu(vtype::zmm_max(), load_mask4, keys + 56);

    index_zmm[4] = zmm_kv_vector<uint64_t>::mask_loadu(
            zmm_kv_vector<uint64_t>::zmm_max(), load_mask1, indexes + 32);
    index_zmm[5] = zmm_kv_vector<uint64_t>::mask_loadu(
            zmm_kv_vector<uint64_t>::zmm_max(), load_mask2, indexes + 40);
    index_zmm[6] = zmm_kv_vector<uint64_t>::mask_loadu(
            zmm_kv_vector<uint64_t>::zmm_max(), load_mask3, indexes + 48);
    index_zmm[7] = zmm_kv_vector<uint64_t>::mask_loadu(
            zmm_kv_vector<uint64_t>::zmm_max(), load_mask4, indexes + 56);
    key_zmm[4] = sort_zmm_64bit<vtype>(key_zmm[4], index_zmm[4]);
    key_zmm[5] = sort_zmm_64bit<vtype>(key_zmm[5], index_zmm[5]);
    key_zmm[6] = sort_zmm_64bit<vtype>(key_zmm[6], index_zmm[6]);
    key_zmm[7] = sort_zmm_64bit<vtype>(key_zmm[7], index_zmm[7]);

    bitonic_merge_two_zmm_64bit<vtype>(
            key_zmm[0], key_zmm[1], index_zmm[0], index_zmm[1]);
    bitonic_merge_two_zmm_64bit<vtype>(
            key_zmm[2], key_zmm[3], index_zmm[2], index_zmm[3]);
    bitonic_merge_two_zmm_64bit<vtype>(
            key_zmm[4], key_zmm[5], index_zmm[4], index_zmm[5]);
    bitonic_merge_two_zmm_64bit<vtype>(
            key_zmm[6], key_zmm[7], index_zmm[6], index_zmm[7]);
    bitonic_merge_four_zmm_64bit<vtype>(key_zmm, index_zmm);
    bitonic_merge_four_zmm_64bit<vtype>(key_zmm + 4, index_zmm + 4);
    bitonic_merge_eight_zmm_64bit<vtype>(key_zmm, index_zmm);

    zmm_kv_vector<uint64_t>::storeu(indexes, index_zmm[0]);
    zmm_kv_vector<uint64_t>::storeu(indexes + 8, index_zmm[1]);
    zmm_kv_vector<uint64_t>::storeu(indexes + 16, index_zmm[2]);
    zmm_kv_vector<uint64_t>::storeu(indexes + 24, index_zmm[3]);
    zmm_kv_vector<uint64_t>::mask_storeu(
            indexes + 32, load_mask1, index_zmm[4]);
    zmm_kv_vector<uint64_t>::mask_storeu(
            indexes + 40, load_mask2, index_zmm[5]);
    zmm_kv_vector<uint64_t>::mask_storeu(
            indexes + 48, load_mask3, index_zmm[6]);
    zmm_kv_vector<uint64_t>::mask_storeu(
            indexes + 56, load_mask4, index_zmm[7]);

    vtype::storeu(keys, key_zmm[0]);
    vtype::storeu(keys + 8, key_zmm[1]);
    vtype::storeu(keys + 16, key_zmm[2]);
    vtype::storeu(keys + 24, key_zmm[3]);
    vtype::mask_storeu(keys + 32, load_mask1, key_zmm[4]);
    vtype::mask_storeu(keys + 40, load_mask2, key_zmm[5]);
    vtype::mask_storeu(keys + 48, load_mask3, key_zmm[6]);
    vtype::mask_storeu(keys + 56, load_mask4, key_zmm[7]);
}

template <typename vtype, typename type_t>
X86_SIMD_SORT_INLINE void
sort_128_64bit(type_t *keys, uint64_t *indexes, int32_t N)
{
    if (N <= 64) {
        sort_64_64bit<vtype>(keys, indexes, N);
        return;
    }
    using zmm_t = typename vtype::zmm_t;
    using index_t = zmm_kv_vector<uint64_t>::zmm_t;
    using opmask_t = typename vtype::opmask_t;
    zmm_t key_zmm[16];
    index_t index_zmm[16];

    key_zmm[0] = vtype::loadu(keys);
    key_zmm[1] = vtype::loadu(keys + 8);
    key_zmm[2] = vtype::loadu(keys + 16);
    key_zmm[3] = vtype::loadu(keys + 24);
    key_zmm[4] = vtype::loadu(keys + 32);
    key_zmm[5] = vtype::loadu(keys + 40);
    key_zmm[6] = vtype::loadu(keys + 48);
    key_zmm[7] = vtype::loadu(keys + 56);

    index_zmm[0] = zmm_kv_vector<uint64_t>::loadu(indexes);
    index_zmm[1] = zmm_kv_vector<uint64_t>::loadu(indexes + 8);
    index_zmm[2] = zmm_kv_vector<uint64_t>::loadu(indexes + 16);
    index_zmm[3] = zmm_kv_vector<uint64_t>::loadu(indexes + 24);
    index_zmm[4] = zmm_kv_vector<uint64_t>::loadu(indexes + 32);
    index_zmm[5] = zmm_kv_vector<uint64_t>::loadu(indexes + 40);
    index_zmm[6] = zmm_kv_vector<uint64_t>::loadu(indexes + 48);
    index_zmm[7] = zmm_kv_vector<uint64_t>::loadu(indexes + 56);
    key_zmm[0] = sort_zmm_64bit<vtype>(key_zmm[0], index_zmm[0]);
    key_zmm[1] = sort_zmm_64bit<vtype>(key_zmm[1], index_zmm[1]);
    key_zmm[2] = sort_zmm_64bit<vtype>(key_zmm[2], index_zmm[2]);
    key_zmm[3] = sort_zmm_64bit<vtype>(key_zmm[3], index_zmm[3]);
    key_zmm[4] = sort_zmm_64bit<vtype>(key_zmm[4], index_zmm[4]);
    key_zmm[5] = sort_zmm_64bit<vtype>(key_zmm[5], index_zmm[5]);
    key_zmm[6] = sort_zmm_64bit<vtype>(key_zmm[6], index_zmm[6]);
    key_zmm[7] = sort_zmm_64bit<vtype>(key_zmm[7], index_zmm[7]);

    opmask_t load_mask1 = 0xFF, load_mask2 = 0xFF;
    opmask_t load_mask3 = 0xFF, load_mask4 = 0xFF;
    opmask_t load_mask5 = 0xFF, load_mask6 = 0xFF;
    opmask_t load_mask7 = 0xFF, load_mask8 = 0xFF;
    if (N != 128) {
        uint64_t combined_mask = (0x1ull << (N - 64)) - 0x1ull;
        load_mask1 = (combined_mask)&0xFF;
        load_mask2 = (combined_mask >> 8) & 0xFF;
        load_mask3 = (combined_mask >> 16) & 0xFF;
        load_mask4 = (combined_mask >> 24) & 0xFF;
        load_mask5 = (combined_mask >> 32) & 0xFF;
        load_mask6 = (combined_mask >> 40) & 0xFF;
        load_mask7 = (combined_mask >> 48) & 0xFF;
        load_mask8 = (combined_mask >> 56) & 0xFF;
    }
    key_zmm[8] = vtype::mask_loadu(vtype::zmm_max(), load_mask1, keys + 64);
    key_zmm[9] = vtype::mask_loadu(vtype::zmm_max(), load_mask2, keys + 72);
    key_zmm[10] = vtype::mask_loadu(vtype::zmm_max(), load_mask3, keys + 80);
    key_zmm[11] = vtype::mask_loadu(vtype::zmm_max(), load_mask4, keys + 88);
    key_zmm[12] = vtype::mask_loadu(vtype::zmm_max(), load_mask5, keys + 96);
    key_zmm[13] = vtype::mask_loadu(vtype::zmm_max(), load_mask6, keys + 104);
    key_zmm[14] = vtype::mask_loadu(vtype::zmm_max(), load_mask7, keys + 112);
    key_zmm[15] = vtype::mask_loadu(vtype::zmm_max(), load_mask8, keys + 120);

    index_zmm[8] = zmm_kv_vector<uint64_t>::mask_loadu(
            zmm_kv_vector<uint64_t>::zmm_max(), load_mask1, indexes + 64);
    index_zmm[9] = zmm_kv_vector<uint64_t>::mask_loadu(
            zmm_kv_vector<uint64_t>::zmm_max(), load_mask2, indexes + 72);
    index_zmm[10] = zmm_kv_vector<uint64_t>::mask_loadu(
            zmm_kv_vector<uint64_t>::zmm_max(), load_mask3, indexes + 80);
    index_zmm[11] = zmm_kv_vector<uint64_t>::mask_loadu(
            zmm_kv_vector<uint64_t>::zmm_max(), load_mask4, indexes + 88);
    index_zmm[12] = zmm_kv_vector<uint64_t>::mask_loadu(
            zmm_kv_vector<uint64_t>::zmm_max(), load_mask5, indexes + 96);
    index_zmm[13] = zmm_kv_vector<uint64_t>::mask_loadu(
            zmm_kv_vector<uint64_t>::zmm_max(), load_mask6, indexes + 104);
    index_zmm[14] = zmm_kv_vector<uint64_t>::mask_loadu(
            zmm_kv_vector<uint64_t>::zmm_max(), load_mask7, indexes + 112);
    index_zmm[15] = zmm_kv_vector<uint64_t>::mask_loadu(
            zmm_kv_vector<uint64_t>::zmm_max(), load_mask8, indexes + 120);
    key_zmm[8] = sort_zmm_64bit<vtype>(key_zmm[8], index_zmm[8]);
    key_zmm[9] = sort_zmm_64bit<vtype>(key_zmm[9], index_zmm[9]);
    key_zmm[10] = sort_zmm_64bit<vtype>(key_zmm[10], index_zmm[10]);
    key_zmm[11] = sort_zmm_64bit<vtype>(key_zmm[11], index_zmm[11]);
    key_zmm[12] = sort_zmm_64bit<vtype>(key_zmm[12], index_zmm[12]);
    key_zmm[13] = sort_zmm_64bit<vtype>(key_zmm[13], index_zmm[13]);
    key_zmm[14] = sort_zmm_64bit<vtype>(key_zmm[14], index_zmm[14]);
    key_zmm[15] = sort_zmm_64bit<vtype>(key_zmm[15], index_zmm[15]);

    bitonic_merge_two_zmm_64bit<vtype>(
            key_zmm[0], key_zmm[1], index_zmm[0], index_zmm[1]);
    bitonic_merge_two_zmm_64bit<vtype>(
            key_zmm[2], key_zmm[3], index_zmm[2], index_zmm[3]);
    bitonic_merge_two_zmm_64bit<vtype>(
            key_zmm[4], key_zmm[5], index_zmm[4], index_zmm[5]);
    bitonic_merge_two_zmm_64bit<vtype>(
            key_zmm[6], key_zmm[7], index_zmm[6], index_zmm[7]);
    bitonic_merge_two_zmm_64bit<vtype>(
            key_zmm[8], key_zmm[9], index_zmm[8], index_zmm[9]);
    bitonic_merge_two_zmm_64bit<vtype>(
            key_zmm[10], key_zmm[11], index_zmm[10], index_zmm[11]);
    bitonic_merge_two_zmm_64bit<vtype>(
            key_zmm[12], key_zmm[13], index_zmm[12], index_zmm[13]);
    bitonic_merge_two_zmm_64bit<vtype>(
            key_zmm[14], key_zmm[15], index_zmm[14], index_zmm[15]);
    bitonic_merge_four_zmm_64bit<vtype>(key_zmm, index_zmm);
    bitonic_merge_four_zmm_64bit<vtype>(key_zmm + 4, index_zmm + 4);
    bitonic_merge_four_zmm_64bit<vtype>(key_zmm + 8, index_zmm + 8);
    bitonic_merge_four_zmm_64bit<vtype>(key_zmm + 12, index_zmm + 12);
    bitonic_merge_eight_zmm_64bit<vtype>(key_zmm, index_zmm);
    bitonic_merge_eight_zmm_64bit<vtype>(key_zmm + 8, index_zmm + 8);
    bitonic_merge_sixteen_zmm_64bit<vtype>(key_zmm, index_zmm);
    zmm_kv_vector<uint64_t>::storeu(indexes, index_zmm[0]);
    zmm_kv_vector<uint64_t>::storeu(indexes + 8, index_zmm[1]);
    zmm_kv_vector<uint64_t>::storeu(indexes + 16, index_zmm[2]);
    zmm_kv_vector<uint64_t>::storeu(indexes + 24, index_zmm[3]);
    zmm_kv_vector<uint64_t>::storeu(indexes + 32, index_zmm[4]);
    zmm_kv_vector<uint64_t>::storeu(indexes + 40, index_zmm[5]);
    zmm_kv_vector<uint64_t>::storeu(indexes + 48, index_zmm[6]);
    zmm_kv_vector<uint64_t>::storeu(indexes + 56, index_zmm[7]);
    zmm_kv_vector<uint64_t>::mask_storeu(
            indexes + 64, load_mask1, index_zmm[8]);
    zmm_kv_vector<uint64_t>::mask_storeu(
            indexes + 72, load_mask2, index_zmm[9]);
    zmm_kv_vector<uint64_t>::mask_storeu(
            indexes + 80, load_mask3, index_zmm[10]);
    zmm_kv_vector<uint64_t>::mask_storeu(
            indexes + 88, load_mask4, index_zmm[11]);
    zmm_kv_vector<uint64_t>::mask_storeu(
            indexes + 96, load_mask5, index_zmm[12]);
    zmm_kv_vector<uint64_t>::mask_storeu(
            indexes + 104, load_mask6, index_zmm[13]);
    zmm_kv_vector<uint64_t>::mask_storeu(
            indexes + 112, load_mask7, index_zmm[14]);
    zmm_kv_vector<uint64_t>::mask_storeu(
            indexes + 120, load_mask8, index_zmm[15]);

    vtype::storeu(keys, key_zmm[0]);
    vtype::storeu(keys + 8, key_zmm[1]);
    vtype::storeu(keys + 16, key_zmm[2]);
    vtype::storeu(keys + 24, key_zmm[3]);
    vtype::storeu(keys + 32, key_zmm[4]);
    vtype::storeu(keys + 40, key_zmm[5]);
    vtype::storeu(keys + 48, key_zmm[6]);
    vtype::storeu(keys + 56, key_zmm[7]);
    vtype::mask_storeu(keys + 64, load_mask1, key_zmm[8]);
    vtype::mask_storeu(keys + 72, load_mask2, key_zmm[9]);
    vtype::mask_storeu(keys + 80, load_mask3, key_zmm[10]);
    vtype::mask_storeu(keys + 88, load_mask4, key_zmm[11]);
    vtype::mask_storeu(keys + 96, load_mask5, key_zmm[12]);
    vtype::mask_storeu(keys + 104, load_mask6, key_zmm[13]);
    vtype::mask_storeu(keys + 112, load_mask7, key_zmm[14]);
    vtype::mask_storeu(keys + 120, load_mask8, key_zmm[15]);
}

template <typename vtype, typename type_t>
X86_SIMD_SORT_INLINE type_t get_pivot_64bit(type_t *keys,
                                            uint64_t *indexes,
                                            const int64_t left,
                                            const int64_t right)
{
    // median of 8
    int64_t size = (right - left) / 8;
    using zmm_t = typename vtype::zmm_t;
    using index_t = zmm_kv_vector<uint64_t>::zmm_t;
    __m512i rand_index = _mm512_set_epi64(left + size,
                                          left + 2 * size,
                                          left + 3 * size,
                                          left + 4 * size,
                                          left + 5 * size,
                                          left + 6 * size,
                                          left + 7 * size,
                                          left + 8 * size);
    zmm_t key_vec = vtype::template i64gather<sizeof(type_t)>(rand_index, keys);

    index_t index_vec;
    zmm_t sort;
    index_vec = zmm_kv_vector<uint64_t>::template i64gather<sizeof(uint64_t)>(
            rand_index, indexes);
    sort = sort_zmm_64bit<vtype>(key_vec, index_vec);
    // pivot will never be a nan, since there are no nan's!

    return ((type_t *)&sort)[4];
}

template <typename vtype, typename type_t>
inline void heapify(type_t *keys, uint64_t *indexes, int64_t idx, int64_t size)
{
    int64_t i = idx;
    while (true) {
        int64_t j = 2 * i + 1;
        if (j >= size || j < 0) { break; }
        int k = j + 1;
        if (k < size && keys[j] < keys[k]) { j = k; }
        if (keys[j] < keys[i]) { break; }
        std::swap(keys[i], keys[j]);
        std::swap(indexes[i], indexes[j]);
        i = j;
    }
}
template <typename vtype, typename type_t>
inline void heap_sort(type_t *keys, uint64_t *indexes, int64_t size)
{
    for (int64_t i = size / 2 - 1; i >= 0; i--) {
        heapify<vtype>(keys, indexes, i, size);
    }
    for (int64_t i = size - 1; i > 0; i--) {
        std::swap(keys[0], keys[i]);
        std::swap(indexes[0], indexes[i]);
        heapify<vtype>(keys, indexes, 0, i);
    }
}

template <typename vtype, typename type_t>
inline void qsort_64bit_(type_t *keys,
                         uint64_t *indexes,
                         int64_t left,
                         int64_t right,
                         int64_t max_iters)
{
    /*
     * Resort to std::sort if quicksort isnt making any progress
     */
    if (max_iters <= 0) {
        heap_sort<vtype>(keys + left, indexes + left, right - left + 1);
        return;
    }
    /*
     * Base case: use bitonic networks to sort arrays <= 128
     */
    if (right + 1 - left <= 128) {

        sort_128_64bit<vtype>(
                keys + left, indexes + left, (int32_t)(right + 1 - left));
        return;
    }

    type_t pivot = get_pivot_64bit<vtype>(keys, indexes, left, right);
    type_t smallest = vtype::type_max();
    type_t biggest = vtype::type_min();
    int64_t pivot_index = partition_avx512<vtype>(
            keys, indexes, left, right + 1, pivot, &smallest, &biggest);
    if (pivot != smallest)
        qsort_64bit_<vtype>(
                keys, indexes, left, pivot_index - 1, max_iters - 1);
    if (pivot != biggest)
        qsort_64bit_<vtype>(keys, indexes, pivot_index, right, max_iters - 1);
}

X86_SIMD_SORT_INLINE int64_t replace_nan_with_inf_kv(double *arr,
                                                     int64_t arrsize)
{
    int64_t nan_count = 0;
    __mmask8 loadmask = 0xFF;
    while (arrsize > 0) {
        if (arrsize < 8) { loadmask = (0x01 << arrsize) - 0x01; }
        __m512d in_zmm = _mm512_maskz_loadu_pd(loadmask, arr);
        __mmask8 nanmask = _mm512_cmp_pd_mask(in_zmm, in_zmm, _CMP_NEQ_UQ);
        nan_count += _mm_popcnt_u32((int32_t)nanmask);
        _mm512_mask_storeu_pd(arr, nanmask, ZMM_MAX_DOUBLE);
        arr += 8;
        arrsize -= 8;
    }
    return nan_count;
}

X86_SIMD_SORT_INLINE void
replace_inf_with_nan_kv(double *arr, int64_t arrsize, int64_t nan_count)
{
    for (int64_t ii = arrsize - 1; nan_count > 0; --ii) {
        arr[ii] = std::nan("1");
        nan_count -= 1;
    }
}

template <>
inline void
avx512_qsort_kv<int64_t>(int64_t *keys, uint64_t *indexes, int64_t arrsize)
{
    if (arrsize > 1) {
        qsort_64bit_<zmm_kv_vector<int64_t>, int64_t>(
                keys, indexes, 0, arrsize - 1, 2 * (int64_t)log2(arrsize));
    }
}

template <>
inline void
avx512_qsort_kv<uint64_t>(uint64_t *keys, uint64_t *indexes, int64_t arrsize)
{
    if (arrsize > 1) {
        qsort_64bit_<zmm_kv_vector<uint64_t>, uint64_t>(
                keys, indexes, 0, arrsize - 1, 2 * (int64_t)log2(arrsize));
    }
}

template <>
inline void
avx512_qsort_kv<double>(double *keys, uint64_t *indexes, int64_t arrsize)
{
    if (arrsize > 1) {
        int64_t nan_count = replace_nan_with_inf(keys, arrsize);
        qsort_64bit_<zmm_kv_vector<double>, double>(
                keys, indexes, 0, arrsize - 1, 2 * (int64_t)log2(arrsize));
        replace_inf_with_nan_kv(keys, arrsize, nan_count);
    }
}
#endif // AVX512_QSORT_64BIT_KV