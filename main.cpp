// Include C++ header files.
// #include <math.h>
// #include <stdio.h>
// #include <sys/stat.h>
// #include "include/encoding.hpp"
// #include "include/phases.hpp"
// #include "include/phase2.hpp"
// #include "include/b17phase2.hpp"
// #include "include/phase3.hpp"
// #include "include/b17phase3.hpp"
// #include "include/phase4.hpp"
// #include "include/b17phase4.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include <utility>
#include <vector>
#include <thread>
#include <memory>
#include <mutex>


#define Test_GPU 1

#if Test_GPU
#include "include/chacha8.cuh"
#include <iostream>
#include <array>
#else
// #include <unordered_map>
// #include <unordered_set>
// Include local CUDA header files.
#include "include/cuda_kernel.cuh"
#include "include/chacha8.cuh"

// Include local source header files.
#include "include/calculate_bucket.h"
#include "include/disk.h"
#include "include/phase1.h"
#include "include/util.h"
#include "include/chia_filesystem.h"
#include "include/entry_sizes.h"
#include "include/exceptions.h"
// #include "include/pos_constants.h"
#include "include/sort_manager.h" 


#endif

#define MAX_THREADS 4 // Change this for HW assignment
#define ARGC_NUM 5
#define MAX_K 50
#define MIN_K 0 // Should be 32, but we are testing

using namespace std;

int WritePlotFile(int num_threads_input, uint8_t const k, bool gpu_boost, std::string file_path, std::string start_time);
void HexToBytes(const string &hex, uint8_t *result);
void init_data(struct chacha8_ctx * x, uint64_t *pos, uint64_t *n_blocks);
#if Test_GPU

#else
extern GlobalData globals;
#endif
void init_data(struct chacha8_ctx * x, uint64_t *pos, uint64_t *n_blocks)
{

    x[0].input[0] = 0;
    x[0].input[1] = 1;
    x[0].input[2] = 2;
    x[0].input[3] = 3;
    x[0].input[4] = 4;
    x[0].input[5] = 5;
    x[0].input[6] = 6;
    x[0].input[7] = 7;
    x[0].input[8] = 8;
    x[0].input[9] = 9;
    x[0].input[10] = 10;
    x[0].input[11] = 11;
    x[0].input[12] = 12;
    x[0].input[13] = 13;
    x[0].input[14] = 14;
    x[0].input[15] = 15;

    pos[0] = 1;
    pos[1] = 1;
    pos[2] = 1;
    pos[3] = 1;

    n_blocks[0] = 1;
    n_blocks[1] = 1;
    n_blocks[2] = 2;
    n_blocks[3] = 3;
}

int main(int argc, char *argv[]) {

#if Test_GPU
    // Using GPU for chacha8
    int size = 4;
    struct chacha8_ctx x[1];
    uint64_t pos[4];
    uint64_t n_blocks[4];
    uint8_t *c;
    uint8_t c_start[4];

    uint8_t i;

    uint8_t block_size = 0;
    for (i = 0 ; i < 4 ; i++)
    {   
        c_start[i] = block_size;
        block_size += n_blocks[i];
    }

    init_data(x, pos, n_blocks);

    get_chacha8_key(x, pos, n_blocks, c, c_start, block_size, size);

    std::cout << "Result: " << std::endl;
    for (i = 0 ; i < block_size; i++)
    {
        std::cout << c[i] << std::endl;
    }

#else

    // Execution format: ./ChiaGPUPloter <num of thread> <gpu boost mode> <k> <plot file path> 
    // For example: ./ChiaGPUPlotter 4 0 32 /scratch/user/willytwsu/plot/
    int arg_count = argc;
    long int sample_points;
    int num_threads;
    bool gpu_boost = false;
    int k;
    string plot_file_path;
    uint8_t id[2];
    id[0] = 111;
    Timer start_time;
    string file_timestamp = start_time.GetCurrentTimeString();
    cout << file_timestamp << endl;

    if (argc != ARGC_NUM) {
        cout << "Please select your choice: " << endl;
	    cout << "1. Number of thread for parallelism" << endl; 
        cout << "e.g. 4" << endl;  
        cout << "2. Whether you want to use GPU: no" << endl;  
        cout << "e.g. 0" << endl;  
        cout << "3. Plot constant (k)" << endl; 
        cout << "e.g. 32" << endl; 
        cout << "4. Where do you want to store you plot file?" << endl; 
        cout << "e.g. /scratch/user/willytwsu/plot/" << endl; 
	    exit(0);
    }

    std::cout << "argc: " << arg_count << std::endl;

    // File Path
    // Check if the directory exist
    // if not return error
    argc--;
    plot_file_path = argv[argc];
    

    // k
    argc--;
    if (atoi(argv[argc]) < MIN_K || atoi(argv[argc]) > 50)
    {
        cout << "Notice the range of k (plot constant) is 0~50" << endl;
        exit(0);
    }
    k = atoi(argv[argc]);
    cout << "k = " << k << endl;
    // GPU boost
    argc--;
    if (atoi(argv[argc]) != 1 && atoi(argv[argc]) != 0)
    {
        cout << "Type 0 as false, if you don't want to use GPU for plot" << endl;
        cout << "Type 1 as true, if you want to use GPU for plot" << endl;
        exit(0);
    }

    gpu_boost = atoi(argv[argc]) == 1;
    cout << "gpu boost: " << gpu_boost << endl;

    // Number of thread
    argc--; 
    if (atoi(argv[argc]) < 0 || atoi(argv[argc]) > MAX_THREADS)
    {
        std::cout << "Incorrect thread number" << arg_count << std::endl;
        exit(0);
    }
    num_threads = atoi(argv[argc]);


    // Initialize arrays A, B, and C.
    // double A[3], B[3], C[3];
    // uint32_t key = 1;

    // F1Calculator f1;
    
    // f1.say_hi();
    // uint64_t first_x = 0;
    // uint64_t n = 0;
    // uint64_t *res;
    // f1.CalculateBuckets( first_x, n, res);

    // write to disk 
    WritePlotFile(num_threads, k, gpu_boost, plot_file_path, file_timestamp);

    // repeat more times


    // Print out result.
    

    // std::time_t start_time = std::chrono::system_clock::to_time_t(start);

    // // Populate arrays A and B.
    // A[0] = 1; A[1] = 2; A[2] = 3;
    // B[0] = 1; B[1] = 1; B[2] = 1;

    // // Sum array elements across ( C[0] = A[0] + B[0] ) into array C using CUDA.
    // kernel(A, B, C, 3);


    start_time.PrintElapsed("F1 complete, Time = ");
#endif

    return 0;
}

#if Test_GPU

#else
int WritePlotFile(int num_threads_input, uint8_t const k, bool gpu_boost, std::string file_path, std::string start_time)
{
    uint32_t num_stripes = 0;
    string filename = "plot.dat";
    string tempdir = ".";
    string tempdir2 = ".";
    string finaldir = ".";
    string operation = "help";
    string memo = "0102030405";
    string id = "022fb42c08c12de3a6af053880199806532e79515f94e83461612101f9412f9e";
    uint32_t buf_megabytes_input = 0;
    uint32_t num_buckets_input = 0;
    uint64_t stripe_size_input = 0;


    if (k < kMinPlotSize || k > kMaxPlotSize) {
        throw InvalidValueException("Plot size k= " + std::to_string(k) + " is invalid");
    }
    uint32_t stripe_size, buf_megabytes, num_buckets;
    uint8_t num_threads;
    if (stripe_size_input != 0) {
        stripe_size = stripe_size_input;
    } else {
        stripe_size = 65536;
    }
    if (num_threads_input != 0) {
        num_threads = num_threads_input;
    } else {
        num_threads = 2;
    }
    if (buf_megabytes_input != 0) {
        buf_megabytes = buf_megabytes_input;
    } else {
        buf_megabytes = 4608;
    }
    if (buf_megabytes < 10) {
        throw InsufficientMemoryException("Please provide at least 10MiB of ram");
    }
    // Subtract some ram to account for dynamic allocation through the code
    uint64_t thread_memory = num_threads * (2 * (stripe_size + 5000)) *
                             EntrySizes::GetMaxEntrySize(k, 4, true) / (1024 * 1024);
    uint64_t sub_mbytes = (5 + (int)std::min(buf_megabytes * 0.05, (double)50) + thread_memory);
    if (sub_mbytes > buf_megabytes) {
        throw InsufficientMemoryException(
            "Please provide more memory. At least " + std::to_string(sub_mbytes));
    }
    uint64_t memory_size = ((uint64_t)(buf_megabytes - sub_mbytes)) * 1024 * 1024;
    double max_table_size = 0;
    for (size_t i = 1; i <= 7; i++) {
        double memory_i = 1.3 * ((uint64_t)1 << k) * EntrySizes::GetMaxEntrySize(k, i, true);
        if (memory_i > max_table_size)
            max_table_size = memory_i;
    }
    if (num_buckets_input != 0) {
        num_buckets = RoundPow2(num_buckets_input);
    } else {
        num_buckets = 2 * RoundPow2(ceil(
                              ((double)max_table_size) / (memory_size * kMemSortProportion)));
    }
    if (num_buckets < kMinBuckets) {
        if (num_buckets_input != 0) {
            throw InvalidValueException("Minimum buckets is " + std::to_string(kMinBuckets));
        }
        num_buckets = kMinBuckets;
    } else if (num_buckets > kMaxBuckets) {
        if (num_buckets_input != 0) {
            throw InvalidValueException("Maximum buckets is " + std::to_string(kMaxBuckets));
        }
        double required_mem =
            (max_table_size / kMaxBuckets) / kMemSortProportion / (1024 * 1024) + sub_mbytes;
        throw InsufficientMemoryException(
            "Do not have enough memory. Need " + std::to_string(required_mem) + " MiB");
    }
    uint32_t log_num_buckets = log2(num_buckets);

    std::cout << "Computing table 1" << std::endl;
    std::cout << "Progress update: 0.01" << std::endl;
    std::cout << "ID:" << id << std::endl;
    std::cout << "Num of thread: " << num_threads << std::endl;
    std::cout << "Num of bucket: " << num_buckets << std::endl;
    std::cout << "Enable GPU:" << gpu_boost << std::endl;


    globals.stripe_size = stripe_size;
    globals.num_threads = num_threads;
    uint64_t x = 0;

    std::string tmp_dirname = tempdir;

    uint32_t const t1_entry_size_bytes = EntrySizes::GetMaxEntrySize(k, 1, true);
    globals.L_sort_manager = std::make_unique<SortManager>(
        memory_size,
        num_buckets,
        log_num_buckets,
        t1_entry_size_bytes,
        tmp_dirname,
        filename + ".p1.t1",
        0,
        globals.stripe_size);
    std::array<uint8_t, 32> id_bytes;
    HexToBytes(id, id_bytes.data());

    // These are used for sorting on disk. The sort on disk code needs to know how
    // many elements are in each bucket.
    std::vector<uint64_t> table_sizes = std::vector<uint64_t>(8, 0);
    std::mutex sort_manager_mutex;
    {
        // Start of parallel execution
        std::vector<std::thread> threads;
        for (int i = 0; i < num_threads; i++) {
            threads.emplace_back(F1thread, i, k, id_bytes.data(), &sort_manager_mutex, gpu_boost);
        }

        for (auto& t : threads) {
            t.join();
        }
        // end of parallel execution

        // No parallel execution
        // for (int i = 0; i < num_threads; i++) {
        //     F1thread(i, k, id_bytes.data(), &sort_manager_mutex, gpu_boost);
        // }
    }    
    
    std::cout << "Flush to cache" << std::endl;

    uint64_t prevtableentries = 1ULL << k;
    globals.L_sort_manager->FlushCache();
    table_sizes[1] = x + 1;

    return 0;
}


void HexToBytes(const string &hex, uint8_t *result)
{
    for (uint32_t i = 0; i < hex.length(); i += 2) {
        string byteString = hex.substr(i, 2);
        uint8_t byte = (uint8_t)strtol(byteString.c_str(), NULL, 16);
        result[i / 2] = byte;
    }
}
#endif