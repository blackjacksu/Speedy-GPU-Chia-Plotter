// Copyright 2018 Chia Network Inc

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//    http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SRC_CPP_CALCULATE_BUCKET_HPP_
#define SRC_CPP_CALCULATE_BUCKET_HPP_

#include <stdint.h>

#include <algorithm>
#include <array>
#include <bitset>
#include <iostream>
#include <map>
#include <utility>
#include <vector>

// #include "b3/blake3.h"
// #include "bits.hpp"
#include "chacha8.cuh"
#include "pos_constants.h"
#include "util.h"
#define GPU_GROUP_SIZE 4

// ChaCha8 block size
const uint16_t kF1BlockSizeBits = 512;

// Extra bits of output from the f functions. Instead of being a function from k -> k bits,
// it's a function from k -> k + kExtraBits bits. This allows less collisions in matches.
// Refer to the paper for mathematical motivations.
const uint8_t kExtraBits = 6;

// Convenience variable
const uint8_t kExtraBitsPow = 1 << kExtraBits;

// B and C groups which constitute a bucket, or BC group. These groups determine how
// elements match with each other. Two elements must be in adjacent buckets to match.
const uint16_t kB = 119;
const uint16_t kC = 127;
const uint16_t kBC = kB * kC;

// This (times k) is the length of the metadata that must be kept for each entry. For example,
// for a table 4 entry, we must keep 4k additional bits for each entry, which is used to
// compute f5.
static const uint8_t kVectorLens[] = {0, 0, 1, 2, 4, 4, 3, 2};


// Class to evaluate F1
class F1Calculator {
public:
    F1Calculator() = default;

    // F1Calculator(bool _gpu_boost);

    F1Calculator(uint8_t k, const uint8_t* orig_key, bool _gpu_boost);

    // F1(x) values for x in range [first_x, first_x + n) are placed in res[].
    // n must not be more than 1 << kBatchSizes.
    void CalculateBuckets(uint64_t first_x, uint64_t n, uint64_t *res);

    void CalculateBuckets_Boost(uint64_t first_x[GPU_GROUP_SIZE], uint64_t n[GPU_GROUP_SIZE], uint64_t *res);

    void say_hi();

    ~F1Calculator();

private:
    // Size of the plot
    uint8_t k_{};

    // ChaCha8 context
    struct chacha8_ctx enc_ctx_{};

    uint8_t *buf_{};

    bool gpu_boost;

    uint64_t start[GPU_GROUP_SIZE];
    uint64_t end[GPU_GROUP_SIZE];
    uint64_t num_blocks[GPU_GROUP_SIZE];
    uint64_t buf_start_index[GPU_GROUP_SIZE];
    uint32_t start_bit[GPU_GROUP_SIZE];
    uint8_t x_shift[GPU_GROUP_SIZE];
};

#endif