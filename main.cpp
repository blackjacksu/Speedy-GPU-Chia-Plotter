// Include C++ header files.
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <ctime>
#include <vector>
#include <memory>
#include <mutex>

// Include local CUDA header files.
#include "include/cuda_kernel.cuh"
#include "include/chacha8.cuh"

// Include local source header files.
#include "include/calculate_bucket.h"
#include "include/disk.h"
#include "include/phase1.h"

using namespace std;

#define MAX_THREADS 4 // Change this for HW assignment
#define ARGC_NUM 5
#define MAX_K 50
#define MIN_K 32

int WritePlotFile(int num_threads, uint8_t const k, const uint8_t* id, std::string file_path, std::string start_time);

int main(int argc, char *argv[]) {

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
        cout << "Notice the range of k (plot constant) is 32~50" << endl;
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
    WritePlotFile(num_threads, k, id, plot_file_path, file_timestamp);

    // repeat more times


    // Print out result.
    

    // std::time_t start_time = std::chrono::system_clock::to_time_t(start);

    // // Populate arrays A and B.
    // A[0] = 1; A[1] = 2; A[2] = 3;
    // B[0] = 1; B[1] = 1; B[2] = 1;

    // // Sum array elements across ( C[0] = A[0] + B[0] ) into array C using CUDA.
    // kernel(A, B, C, 3);


    start_time.PrintElapsed("F1 complete, Time = ");

    return 0;
}


int WritePlotFile(int num_threads, uint8_t const k, const uint8_t* id, std::string file_path, std::string start_time)
{
    std::mutex sort_manager_mutex;
    {
        // Start of parallel execution
        std::vector<std::thread> threads;
        for (int i = 0; i < num_threads; i++) {
            threads.emplace_back(F1thread, i, k, id, &sort_manager_mutex, file_path, start_time);
        }

        for (auto& t : threads) {
            t.join();
        }
        // end of parallel execution
    }

    return 0;
}