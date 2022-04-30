# Speedy-GPU-Chia-Plotter
This repo is a simple integration of using GPU to plot a chia plot table 1.

Author: TING-WEI (Willy) SU \
Department: Texas A&M University \
            Electrical and Computer Engineering \
Date: April 29 2022 \

Project: ECEN 689 Scalable Distributed Consensus \
Version:

Compile: > make
Execution > ./ChiaGPUPlotter 4 0 32 /scratch/user/willytwsu/plot/ \

Execution format: ./ChiaGPUPloter <num of thread> <gpu boost mode> <k> <plot file path> \
- num of thread: 0-4, currently only support up to 4 thread parallelism \
- gpu boost mode: 0: use cpu to plot, 1: use gpu to plot \
- k: 32~50 \
- plot file path: the folder your want to store your plot file \