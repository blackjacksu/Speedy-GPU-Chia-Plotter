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

// #ifndef SRC_CPP_CALCULATE_BUCKET_HPP_
// #define SRC_CPP_CALCULATE_BUCKET_HPP_

#include "../include/calculate_bucket.h"

uint16_t L_targets[2][kBC][kExtraBitsPow];
bool initialized = false;
void load_tables()
{
    for (uint8_t parity = 0; parity < 2; parity++) {
        for (uint16_t i = 0; i < kBC; i++) {
            uint16_t indJ = i / kC;
            for (uint16_t m = 0; m < kExtraBitsPow; m++) {
                uint16_t yr =
                    ((indJ + m) % kB) * kC + (((2 * m + parity) * (2 * m + parity) + i) % kC);
                L_targets[parity][i][m] = yr;
            }
        }
    }
}


F1Calculator::F1Calculator()
{
    // Default constructor
    std::cout << "Default F1Calculator" << std::endl;
}

F1Calculator::F1Calculator(uint8_t k, const uint8_t* orig_key)
{
    uint8_t enc_key[32];
    size_t buf_blocks = cdiv(k << kBatchSizes, kF1BlockSizeBits) + 1;
    this->k_ = k;
    this->buf_ = new uint8_t[buf_blocks * kF1BlockSizeBits / 8 + 7];
    // First byte is 1, the index of this table
    enc_key[0] = 1;
    memcpy(enc_key + 1, orig_key, 31);
    // Setup ChaCha8 context with zero-filled IV
    // chacha8_keysetup(&this->enc_ctx_, enc_key, 256, NULL);
}

// F1(x) values for x in range [first_x, first_x + n) are placed in res[].
// n must not be more than 1 << kBatchSizes.
void F1Calculator::CalculateBuckets(uint64_t first_x, uint64_t n, uint64_t *res)
{
    uint64_t start = first_x * k_ / kF1BlockSizeBits;
    // 'end' is one past the last keystream block number to be generated
    uint64_t end = cdiv((first_x + n) * k_, kF1BlockSizeBits);
    uint64_t num_blocks = end - start;
    uint32_t start_bit = first_x * k_ % kF1BlockSizeBits;
    uint8_t x_shift = k_ - kExtraBits;
    assert(n <= (1U << kBatchSizes));
    // chacha8_get_keystream(&this->enc_ctx_, start, num_blocks, buf_);
    for (uint64_t x = first_x; x < first_x + n; x++) {
        uint64_t y = SliceInt64FromBytes(buf_, start_bit, k_);
        res[x - first_x] = (y << kExtraBits) | (x >> x_shift);
        start_bit += k_;
    }
}

void F1Calculator::say_hi()
{
    std::cout << "Hi" << std::endl;
}

// Destructor
F1Calculator::~F1Calculator()
{
    delete[] buf_;
}


/*
    // Disable copying
    F1Calculator(const F1Calculator&) = delete;

    // Reloading the encryption key is a no-op since encryption state is local.
    inline void ReloadKey() {}

    // Performs one evaluation of the F function on input L of k bits.
    inline Bits CalculateF(const Bits& L) const
    {
        uint16_t num_output_bits = k_;
        uint16_t block_size_bits = kF1BlockSizeBits;

        // Calculates the counter that will be used to get ChaCha8 keystream.
        // Since k < block_size_bits, we can fit several k bit blocks into one
        // ChaCha8 block.
        uint128_t counter_bit = L.GetValue() * (uint128_t)num_output_bits;
        uint64_t counter = counter_bit / block_size_bits;

        // How many bits are before L, in the current block
        uint32_t bits_before_L = counter_bit % block_size_bits;

        // How many bits of L are in the current block (the rest are in the next block)
        const uint16_t bits_of_L =
            std::min((uint16_t)(block_size_bits - bits_before_L), num_output_bits);

        // True if L is divided into two blocks, and therefore 2 ChaCha8
        // keystream blocks will be generated.
        const bool spans_two_blocks = bits_of_L < num_output_bits;

        uint8_t ciphertext_bytes[kF1BlockSizeBits / 8];
        Bits output_bits;

        // This counter is used to initialize words 12 and 13 of ChaCha8
        // initial state (4x4 matrix of 32-bit words). This is similar to
        // encrypting plaintext at a given offset, but we have no
        // plaintext, so no XORing at the end.
        chacha8_get_keystream(&this->enc_ctx_, counter, 1, ciphertext_bytes);
        Bits ciphertext0(ciphertext_bytes, block_size_bits / 8, block_size_bits);

        if (spans_two_blocks) {
            // Performs another encryption if necessary
            ++counter;
            chacha8_get_keystream(&this->enc_ctx_, counter, 1, ciphertext_bytes);
            Bits ciphertext1(ciphertext_bytes, block_size_bits / 8, block_size_bits);
            output_bits = ciphertext0.Slice(bits_before_L) +
                          ciphertext1.Slice(0, num_output_bits - bits_of_L);
        } else {
            output_bits = ciphertext0.Slice(bits_before_L, bits_before_L + num_output_bits);
        }

        // Adds the first few bits of L to the end of the output, production k + kExtraBits of
        // output
        Bits extra_data = L.Slice(0, kExtraBits);
        if (extra_data.GetSize() < kExtraBits) {
            extra_data += Bits(0, kExtraBits - extra_data.GetSize());
        }
        return output_bits + extra_data;
    }

    // Returns an evaluation of F1(L), and the metadata (L) that must be stored to evaluate F2.
    inline std::pair<Bits, Bits> CalculateBucket(const Bits& L) const
    {
        return std::make_pair(CalculateF(L), L);
    }
*/
