/*******************************************
 * * Copyright (C) 2022 Intel Corporation
 * * SPDX-License-Identifier: BSD-3-Clause
 * *******************************************/

#include "avx512-16bit-qsort.hpp"
#include "avx512-32bit-qsort.hpp"
#include "avx512-64bit-keyvaluesort.hpp"
#include "avx512-64bit-qsort.hpp"
#include "cpuinfo.h"
#include "rand_array.h"
#include <gtest/gtest.h>
#include <vector>

template <typename T>
class avx512_sort : public ::testing::Test {
};
TYPED_TEST_SUITE_P(avx512_sort);

TYPED_TEST_P(avx512_sort, test_arrsizes)
{
    if (cpu_has_avx512bw()) {
        if ((sizeof(TypeParam) == 2) && (!cpu_has_avx512_vbmi2())) {
            GTEST_SKIP() << "Skipping this test, it requires avx512_vbmi2";
        }
        std::vector<int64_t> arrsizes;
        for (int64_t ii = 0; ii < 1024; ++ii) {
            arrsizes.push_back((TypeParam)ii);
        }
        std::vector<TypeParam> arr;
        std::vector<TypeParam> sortedarr;
        for (size_t ii = 0; ii < arrsizes.size(); ++ii) {
            /* Random array */
            arr = get_uniform_rand_array<TypeParam>(arrsizes[ii]);
            sortedarr = arr;
            /* Sort with std::sort for comparison */
            std::sort(sortedarr.begin(), sortedarr.end());
            avx512_qsort<TypeParam>(arr.data(), arr.size());
            ASSERT_EQ(sortedarr, arr);
            arr.clear();
            sortedarr.clear();
        }
    }
    else {
        GTEST_SKIP() << "Skipping this test, it requires avx512bw";
    }
}

REGISTER_TYPED_TEST_SUITE_P(avx512_sort, test_arrsizes);

using Types = testing::Types<uint16_t,
                             int16_t,
                             float,
                             double,
                             uint32_t,
                             int32_t,
                             uint64_t,
                             int64_t>;
INSTANTIATE_TYPED_TEST_SUITE_P(TestPrefix, avx512_sort, Types);

template <typename K, typename V = uint64_t>
struct sorted_t {
    K key;
    K value;
};
template <typename K, typename V = uint64_t>
bool compare(sorted_t<K, V> a, sorted_t<K, V> b)
{
    return a.key == b.key ? a.value < b.value : a.key < b.key;
}

template <typename K>
class TestKeyValueSort : public ::testing::Test {
};

TYPED_TEST_SUITE_P(TestKeyValueSort);

TYPED_TEST_P(TestKeyValueSort, KeyValueSort)
{
    std::vector<int64_t> keysizes;
    for (int64_t ii = 0; ii < 1024; ++ii) {
        keysizes.push_back((TypeParam)ii);
    }
    std::vector<TypeParam> keys;
    std::vector<uint64_t> values;
    std::vector<sorted_t<TypeParam, uint64_t>> sortedarr;

    for (size_t ii = 0; ii < keysizes.size(); ++ii) {
        /* Random array */
        keys = get_uniform_rand_array_key<TypeParam>(keysizes[ii]);
        values = get_uniform_rand_array<uint64_t>(keysizes[ii]);
        for (size_t i = 0; i < keys.size(); i++) {
            sorted_t<TypeParam, uint64_t> tmp_s;
            tmp_s.key = keys[i];
            tmp_s.value = values[i];
            sortedarr.emplace_back(tmp_s);
        }
        /* Sort with std::sort for comparison */
        std::sort(sortedarr.begin(),
                  sortedarr.end(),
                  compare<TypeParam, uint64_t>);
        avx512_qsort_kv<TypeParam>(keys.data(), values.data(), keys.size());
        for (size_t i = 0; i < keys.size(); i++) {
            ASSERT_EQ(keys[i], sortedarr[i].key);
            ASSERT_EQ(values[i], sortedarr[i].value);
        }
        keys.clear();
        values.clear();
        sortedarr.clear();
    }
}

REGISTER_TYPED_TEST_SUITE_P(TestKeyValueSort, KeyValueSort);

using TypesKv = testing::Types<double, uint64_t, int64_t>;
INSTANTIATE_TYPED_TEST_SUITE_P(TestPrefixKv, TestKeyValueSort, TypesKv);