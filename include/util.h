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

#ifndef SRC_CPP_UTIL_HPP_
#define SRC_CPP_UTIL_HPP_

#include <cassert>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

typedef __uint128_t uint128_t;

template <typename Int, typename Int2>
constexpr inline Int cdiv(Int a, Int2 b) { return (a + b - 1) / b; }

inline uint16_t bswap_16(uint16_t x) { return __builtin_bswap16(x); }
inline uint32_t bswap_32(uint32_t x) { return __builtin_bswap32(x); }
inline uint64_t bswap_64(uint64_t x) { return __builtin_bswap64(x); }


class Timer {
 public:
    Timer();

    void PrintElapsed(std::string name);

    std::string GetCurrentTimeString();

 private:
  std::chrono::time_point<std::chrono::steady_clock> wall_clock_time_start_;
  clock_t cpu_time_start_;
};

// 'bytes' points to a big-endian 64 bit value (possibly truncated, if
// (start_bit % 8 + num_bits > 64)). Returns the integer that starts at
// 'start_bit' that is 'num_bits' long (as a native-endian integer).
//
// Note: requires that 8 bytes after the first sliced byte are addressable
// (regardless of 'num_bits'). In practice it can be ensured by allocating
// extra 7 bytes to all memory buffers passed to this function.
uint64_t SliceInt64FromBytes(
    const uint8_t *bytes,
    uint32_t start_bit,
    const uint32_t num_bits);

/*
 * Converts a byte array to a 64 bit int.
 */
uint64_t EightBytesToInt(const uint8_t *bytes);

void IntTo16Bytes(uint8_t *result, const uint128_t input);

uint64_t ExtractNum(
    const uint8_t *bytes,
    uint32_t len_bytes,
    uint32_t begin_bits,
    uint32_t take_bits);

double RoundPow2(double a);

uint32_t ByteAlign(uint32_t num_bits);

#endif