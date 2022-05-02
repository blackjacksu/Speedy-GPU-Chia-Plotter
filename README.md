# Speedy-GPU-Chia-Plotter
This repo is a simple integration of using GPU to plot a chia plot table 1.

Author: TING-WEI (Willy) SU \
Department: Texas A&M University \
            Electrical and Computer Engineering \
Date: April 29 2022 \

Project: ECEN 689 Scalable Distributed Consensus \
Version:


Requirement: 
Compiler: 
- GCC/G++
- Makefile
- NVCC (Nvidia CUDA Compiler)
    ** Installation guide: Either (a) or (b)
    >> Option (a): Follow the guide on this page: https://docs.nvidia.com/cuda/archive/9.2/cuda-installation-guide-mac-os-x/index.html#installation

    >> Option (b)
        b-1. Download the NVCC dmg on mac from my cloud drive: https://drive.google.com/drive/folders/1s-gS7TgvovT37AI1T6qbIdyWgrvgr5JG?usp=sharing
        b-2. Open the dmg then follow the instruction for installation.


Compile: > make
Execution > ./ChiaGPUPlotter 4 0 32 /scratch/user/willytwsu/plot/ \

Execution format: ./ChiaGPUPloter <num of thread> <gpu boost mode> <k> <plot file path>
- num of thread: 0-4, currently only support up to 4 thread parallelism
- gpu boost mode: 0: use cpu to plot, 1: use gpu to plot
    ** Even though you are not running the executable with GPU enable, you are still required to have the nvcc compiler installed.
- k: 32~50
- plot file path: the folder your want to store your plot file

