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

#include "../include/util.h"


Timer::Timer() {
    this->wall_clock_time_start_ = std::chrono::steady_clock::now();
    this->cpu_time_start_ = clock();
}

void Timer::PrintElapsed(std::string name) {
    auto end = std::chrono::steady_clock::now();
    auto wall_clock_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                         end - this->wall_clock_time_start_).count();

    double cpu_time_ms =  1000.0 * (static_cast<double>(clock()) - this->cpu_time_start_) / CLOCKS_PER_SEC;

    double cpu_ratio = static_cast<int>(10000 * (cpu_time_ms / wall_clock_ms)) / 100.0;

    std::cout << name << " " << (wall_clock_ms / 1000.0)  << " seconds. CPU (" << cpu_ratio << "%)" << std::endl;
}

std::string Timer::GetCurrentTimeString()
{   
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    time (&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer,80,"%m%d%Y_%H%M%S",timeinfo);
    return buffer;
}


uint64_t SliceInt64FromBytes(
    const uint8_t *bytes,
    uint32_t start_bit,
    const uint32_t num_bits)
{
    uint64_t tmp;
    if (start_bit + num_bits > 64) {
        bytes += start_bit / 8;
        start_bit %= 8;
    }
    tmp = EightBytesToInt(bytes);
    tmp <<= start_bit;
    tmp >>= 64 - num_bits;
    return tmp;
}

uint64_t EightBytesToInt(const uint8_t *bytes)
{
    uint64_t i;
    memcpy(&i, bytes, sizeof(i));
    return bswap_64(i);
}

void IntTo16Bytes(uint8_t *result, const uint128_t input)
{
   uint64_t r = bswap_64(input >> 64);
   memcpy(result, &r, sizeof(r));
   r = bswap_64((uint64_t)input);
   memcpy(result + 8, &r, sizeof(r));
}

uint64_t ExtractNum(
    const uint8_t *bytes,
    uint32_t len_bytes,
    uint32_t begin_bits,
    uint32_t take_bits)
{
    if ((begin_bits + take_bits) / 8 > len_bytes - 1) {
        take_bits = len_bytes * 8 - begin_bits;
    }
    return SliceInt64FromBytes(bytes, begin_bits, take_bits);
}

double RoundPow2(double a)
{
    // https://stackoverflow.com/questions/54611562/truncate-float-to-nearest-power-of-2-in-c-performance
    int exp;
    double frac = frexp(a, &exp);
    if (frac > 0.0)
        frac = 0.5;
    else if (frac < 0.0)
        frac = -0.5;
    double b = ldexp(frac, exp);
    return b;
}

uint32_t ByteAlign(uint32_t num_bits) 
{ 
    return (num_bits + (8 - ((num_bits) % 8)) % 8); 
}