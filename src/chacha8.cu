// CUDA libraries.
#include <cuda.h>
#include <cuda_runtime.h>

// Include associated header file.
#include "../include/chacha8.cuh"

#define MAX_ARRAY_SIZE 256

// There is a multiply relation between the size of c (output) and n_blocks
// n_blocks = 1, sizeof(c) = uint8_t * 64 * 1
// n_blocks = 2, sizeof(c) = uint8_t * 64 * 2
// n_blocks = 3, sizeof(c) = uint8_t * 64 * 3
#define SIZE_OF_OUTPUT_PER_BLOCK 64

#define U32TO32_LITTLE(v) (v)
#define U8TO32_LITTLE(p) (*(const uint32_t *)(p))
#define U32TO8_LITTLE(p, v) (((uint32_t *)(p))[0] = U32TO32_LITTLE(v))
#define ROTL32(v, n) (((v) << (n)) | ((v) >> (32 - (n))))

#define ROTATE(v, c) (ROTL32(v, c))
#define XOR(v, w) ((v) ^ (w))
#define PLUS(v, w) ((v) + (w))
#define PLUSONE(v) (PLUS((v), 1))

#define QUARTERROUND(a, b, c, d) \
    a = PLUS(a, b);              \
    d = ROTATE(XOR(d, a), 16);   \
    c = PLUS(c, d);              \
    b = ROTATE(XOR(b, c), 12);   \
    a = PLUS(a, b);              \
    d = ROTATE(XOR(d, a), 8);    \
    c = PLUS(c, d);              \
    b = ROTATE(XOR(b, c), 7)

// This is the GPU device code
__global__ void Kernel_Print(int * block_dim, int * thread_id, int * grid_dim)
{
    printf("blockDim x:%d, y:%d, z:%d\n", blockDim.x, blockDim.y, blockDim.z);
    printf("threadIdx x:%d, y:%d, z:%d\n", threadIdx.x, threadIdx.y, threadIdx.z);
    printf("gridDim x:%d, y:%d, z:%d\n", gridDim.x, gridDim.y, gridDim.z);

    block_dim[0] = blockDim.x;
    block_dim[1] = blockDim.y;
    block_dim[2] = blockDim.z;

    thread_id[0] = threadIdx.x;
    thread_id[1] = threadIdx.y;
    thread_id[2] = threadIdx.z;

    grid_dim[0] = gridDim.x;
    grid_dim[1] = gridDim.y;
    grid_dim[2] = gridDim.z;
}


// This is the GPU device code
__global__ void chacha8_get_keystream_cuda( struct chacha8_ctx *x, uint64_t *pos, uint32_t *n_blocks, uint8_t **c)
{
    int idx = threadIdx.x;
    printf("[chacha8_get_keystream_cuda] i = %d\n", idx);

    uint32_t x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15;
    uint32_t j0, j1, j2, j3, j4, j5, j6, j7, j8, j9, j10, j11, j12, j13, j14, j15;
    int i;

    j0 = x[idx].input[0];
    j1 = x[idx].input[1];
    j2 = x[idx].input[2];
    j3 = x[idx].input[3];
    j4 = x[idx].input[4];
    j5 = x[idx].input[5];
    j6 = x[idx].input[6];
    j7 = x[idx].input[7];
    j8 = x[idx].input[8];
    j9 = x[idx].input[9];
    j10 = x[idx].input[10];
    j11 = x[idx].input[11];
    j12 = pos[idx];
    j13 = pos[idx] >> 32;
    j14 = x[idx].input[14];
    j15 = x[idx].input[15];

    while (n_blocks[idx]--) {
        x0 = j0;
        x1 = j1;
        x2 = j2;
        x3 = j3;
        x4 = j4;
        x5 = j5;
        x6 = j6;
        x7 = j7;
        x8 = j8;
        x9 = j9;
        x10 = j10;
        x11 = j11;
        x12 = j12;
        x13 = j13;
        x14 = j14;
        x15 = j15;
        for (i = 8; i > 0; i -= 2) {
            QUARTERROUND(x0, x4, x8, x12);
            QUARTERROUND(x1, x5, x9, x13);
            QUARTERROUND(x2, x6, x10, x14);
            QUARTERROUND(x3, x7, x11, x15);
            QUARTERROUND(x0, x5, x10, x15);
            QUARTERROUND(x1, x6, x11, x12);
            QUARTERROUND(x2, x7, x8, x13);
            QUARTERROUND(x3, x4, x9, x14);
        }
        x0 = PLUS(x0, j0);
        x1 = PLUS(x1, j1);
        x2 = PLUS(x2, j2);
        x3 = PLUS(x3, j3);
        x4 = PLUS(x4, j4);
        x5 = PLUS(x5, j5);
        x6 = PLUS(x6, j6);
        x7 = PLUS(x7, j7);
        x8 = PLUS(x8, j8);
        x9 = PLUS(x9, j9);
        x10 = PLUS(x10, j10);
        x11 = PLUS(x11, j11);
        x12 = PLUS(x12, j12);
        x13 = PLUS(x13, j13);
        x14 = PLUS(x14, j14);
        x15 = PLUS(x15, j15);

        j12 = PLUSONE(j12);
        if (!j12) {
            j13 = PLUSONE(j13);
            /* stopping at 2^70 bytes per nonce is user's responsibility */
        }

        U32TO8_LITTLE(c[idx] + 0, x0);
        U32TO8_LITTLE(c[idx] + 4, x1);
        U32TO8_LITTLE(c[idx] + 8, x2);
        U32TO8_LITTLE(c[idx] + 12, x3);
        U32TO8_LITTLE(c[idx] + 16, x4);
        U32TO8_LITTLE(c[idx] + 20, x5);
        U32TO8_LITTLE(c[idx] + 24, x6);
        U32TO8_LITTLE(c[idx] + 28, x7);
        U32TO8_LITTLE(c[idx] + 32, x8);
        U32TO8_LITTLE(c[idx] + 36, x9);
        U32TO8_LITTLE(c[idx] + 40, x10);
        U32TO8_LITTLE(c[idx] + 44, x11);
        U32TO8_LITTLE(c[idx] + 48, x12);
        U32TO8_LITTLE(c[idx] + 52, x13);
        U32TO8_LITTLE(c[idx] + 56, x14);
        U32TO8_LITTLE(c[idx] + 60, x15);

        c[idx] += 64;
    }
}



void get_chacha8_key(struct chacha8_ctx *_x, uint64_t *_pos, uint32_t *_n_blocks, uint8_t **_c, int array_size)
{
    // 
    std::cout << "Size of uint64_t:" << sizeof(uint64_t) << std::endl;
    std::cout << "Size of * uint64_t:" << sizeof(_pos) << std::endl;
    std::cout << "Size of uint32_t:" << sizeof(uint32_t) << std::endl;
    std::cout << "Size of * uint32_t:" << sizeof(_n_blocks) << std::endl;
    std::cout << "Size of struct chacha8_ctx:" << sizeof(struct chacha8_ctx) << std::endl;

    if (array_size > MAX_ARRAY_SIZE)
    {
        std::cout << "Array size out of bound" << std::endl;
        return;
    }

    struct chacha8_ctx *x = _x;
    uint64_t *pos = _pos;
    uint32_t *n_blocks = _n_blocks;
    uint8_t **c = _c;
    int thread_block = array_size;

    // Has to handle error if memory allocation failed
    cudaError_t error;

    error = cudaMalloc((void**) &pos, array_size * sizeof(uint64_t));
    if (error)
    {
        std::cout << "cudaMalloc fail at pos error: " << error << std::endl; 
        return;
    }

    error = cudaMalloc((void**) &n_blocks, array_size * sizeof(uint32_t));
    if (error)
    {
        std::cout << "cudaMalloc fail at n_blocks error: " << error << std::endl; 
        return;
    }

    error = cudaMalloc((void**) &x, array_size * sizeof(struct chacha8_ctx));
    if (error)
    {
        std::cout << "cudaMalloc fail at x error: " << error << std::endl; 
        return;
    }
    std::cout << "happen error: " << std::endl; 
    error = cudaMalloc((void**) &c, SIZE_OF_OUTPUT_PER_BLOCK * _n_blocks[0]);
    if (error)
    {
        std::cout << "cudaMalloc fail at c error: " << error << std::endl; 
        return;
    }
    std::cout << "happen after error: " << std::endl; 
    error = cudaMemcpy(pos, _pos, array_size * sizeof(uint64_t), cudaMemcpyHostToDevice);
    if (error)
    {
        std::cout << "cudaMemcpy fail at pos error: " << error << std::endl; 
        return;
    }

    error = cudaMemcpy(n_blocks, _n_blocks, array_size * sizeof(uint32_t), cudaMemcpyHostToDevice);
    if (error)
    {
        std::cout << "cudaMemcpy fail at n_blocks error: " << error << std::endl; 
        return;
    }

    error = cudaMemcpy(x, _x, array_size * sizeof(struct chacha8_ctx), cudaMemcpyHostToDevice);
    if (error)
    {
        std::cout << "cudaMemcpy fail at x error: " << error << std::endl; 
        return;
    }



    // Calculate blocksize and gridsize.
    // dim3 blockSize(512, 1, 1);
    // dim3 gridSize(512 / array_size + 1, 1);

    chacha8_get_keystream_cuda<<<1, thread_block>>>(x, pos, n_blocks, c);

    // Copy result to output
    error = cudaMemcpy(_c[0], c[0], SIZE_OF_OUTPUT_PER_BLOCK * n_blocks[0], cudaMemcpyDeviceToHost);
    if (error)
    {
        std::cout << "cudaMemcpy fail at x error: " << error << std::endl; 
        return;
    }
    int block_dim[3] = {1, 2, 3};
    int thread_id[3] = {1, 2, 3};
    int grid_dim[3] = {1, 2, 3};


    Kernel_Print<<<1, thread_block>>>(block_dim, thread_id, grid_dim);
    std::cout << "block_dim.x: " << block_dim[0] << ", block_dim.y: " << block_dim[1] << ", block_dim.z: " << block_dim[2] << std::endl;
    std::cout << "thread_id.x: " << thread_id[0] << ", thread_id.y: " << thread_id[1] << ", thread_id.z: " << thread_id[2] << std::endl;
    std::cout << "grid_dim.x: " << grid_dim[0] << ", grid_dim.y: " << grid_dim[1] << ", grid_dim.z: " << grid_dim[2] << std::endl;

    // free memory
}

// This is host code
void chacha8_keysetup(struct chacha8_ctx *x, const uint8_t *k, uint32_t kbits, const uint8_t *iv)
{
    const char *constants;
    static const char sigma[17] = "expand 32-byte k";
    static const char tau[17] = "expand 16-byte k";

    x->input[4] = U8TO32_LITTLE(k + 0);
    x->input[5] = U8TO32_LITTLE(k + 4);
    x->input[6] = U8TO32_LITTLE(k + 8);
    x->input[7] = U8TO32_LITTLE(k + 12);
    if (kbits == 256) { /* recommended */
        k += 16;
        constants = sigma;
    } else { /* kbits == 128 */
        constants = tau;
    }
    x->input[8] = U8TO32_LITTLE(k + 0);
    x->input[9] = U8TO32_LITTLE(k + 4);
    x->input[10] = U8TO32_LITTLE(k + 8);
    x->input[11] = U8TO32_LITTLE(k + 12);
    x->input[0] = U8TO32_LITTLE(constants + 0);
    x->input[1] = U8TO32_LITTLE(constants + 4);
    x->input[2] = U8TO32_LITTLE(constants + 8);
    x->input[3] = U8TO32_LITTLE(constants + 12);
    if (iv) {
        x->input[14] = U8TO32_LITTLE(iv + 0);
        x->input[15] = U8TO32_LITTLE(iv + 4);
    } else {
        x->input[14] = 0;
        x->input[15] = 0;
    }
}


