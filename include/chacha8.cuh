#ifndef SRC_CHACHA8_H_
#define SRC_CHACHA8_H_

#include <stdint.h>
#include <iostream>
#include <array>

struct chacha8_ctx {
    uint32_t input[16];
};

typedef std::array<struct chacha8_ctx, 10> chacha_x;

#ifdef __cplusplus
extern "C" {
#endif

void chacha8_keysetup(struct chacha8_ctx *x, const uint8_t *k, uint32_t kbits, const uint8_t *iv);
void get_chacha8_key(struct chacha8_ctx * _x, uint64_t *_pos, uint32_t *_n_blocks, uint8_t **_c, int array_size);
void chacha8_get_keystream(
    const struct chacha8_ctx *x,
    uint64_t pos,
    uint32_t n_blocks,
    uint8_t *c);

#ifdef __cplusplus
}
#endif

#endif  // SRC_CHACHA8_H_
