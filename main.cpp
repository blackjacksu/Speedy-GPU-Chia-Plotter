// Include C++ header files.
#include <iostream>
#include <iostream>
#include <string>
#include <chrono>
#include <ctime>

// Include local CUDA header files.
#include "include/cuda_kernel.cuh"
#include "include/chacha8.cuh"

// Include local source header files.
#include "include/calculate_bucket.h"
#include "include/disk.h"
#include "include/phase1.h"

using namespace std;

#define MAX_THREADS     1024 // Change this for HW assignment

int main(int argc, char *argv[]) {


    long int sample_points;
    int num_threads;
    string plot_file_path;

    Timer start_time;

    // if (argc != 3) 
    // {
	//     printf("Need two integers as input \n"); 
	//     printf("Use: <executable_name> <sample_points> <num_threads>\n"); 
	//     exit(0);
    // }
    // sample_points = atol(argv[argc-2]);
    
    // if ((num_threads = atoi(argv[argc-1])) > MAX_THREADS) 
    // {
	//     printf("Maximum number of threads allowed: %d.\n", MAX_THREADS);
	//     exit(0);
    // }; 

    // Initialize arrays A, B, and C.
    double A[3], B[3], C[3];
    uint32_t key = 1;

    F1Calculator f1;
    
    f1.say_hi();
    uint64_t first_x = 0;
    uint64_t n = 0;
    uint64_t *res;
    f1.CalculateBuckets( first_x, n, res);

    FileDisk f("plot_disk");

    std::cout << "FileDisk name: " << f.GetFileName() << std::endl;

    // write to disk 

    // repeat more times


    // Print out result.


    // std::time_t start_time = std::chrono::system_clock::to_time_t(start);

    // Populate arrays A and B.
    A[0] = 1; A[1] = 2; A[2] = 3;
    B[0] = 1; B[1] = 1; B[2] = 1;

    // Sum array elements across ( C[0] = A[0] + B[0] ) into array C using CUDA.
    kernel(A, B, C, 3);




    start_time.PrintElapsed("F1 complete, Time = ");

    return 0;
}