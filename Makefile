###########################################################

## USER SPECIFIC DIRECTORIES ##

# CUDA directory:
CUDA_ROOT_DIR=/sw/eb/sw/CUDA/11.4.1

##########################################################

## CC COMPILER OPTIONS ##

# CC compiler options:
CC=g++
CC_FLAGS=-std=c++17 -Wall
CC_LIBS= -lpthread

##########################################################

## NVCC COMPILER OPTIONS ##

# NVCC compiler options:
NVCC=nvcc
NVCC_FLAGS=-O3
NVCC_LIBS=

# CUDA library directory:
CUDA_LIB_DIR= -L$(CUDA_ROOT_DIR)/lib64
# CUDA include directory:
CUDA_INC_DIR= -I$(CUDA_ROOT_DIR)/include
# CUDA linking libraries:
CUDA_LINK_LIBS= -lcudadevrt -lcudart

##########################################################

## GEN CODE OPTIONS ##
SM := 35
# Gen code options:
GENCODE_FLAGS = -gencode arch=compute_$(SM),code=sm_$(SM)

##########################################################

## Project file structure ##

# Source file directory:
SRC_DIR = src
B3_DIR = b3

# Object file directory:
OBJ_DIR = bin

# Include header file diretory:
INC_DIR = include

##########################################################

## Make variables ##

# Target executable name:
EXE = ChiaGPUPlotter

# Object files:
OBJS = $(OBJ_DIR)/main.o $(OBJ_DIR)/util.o $(OBJ_DIR)/calculate_bucket.o $(OBJ_DIR)/phase1.o \
		$(OBJ_DIR)/disk.o $(OBJ_DIR)/sort_manager.o $(OBJ_DIR)/entry_sizes.o 

KERNEL_OBJS = $(OBJ_DIR)/cuda_kernel.o $(OBJ_DIR)/chacha8.o 

##########################################################

## Compile ##

# Link c++ and CUDA compiled object files to target executable:
$(EXE) : $(OBJS) $(KERNEL_OBJS)
	$(CC) $(CC_FLAGS) $(OBJS) $(KERNEL_OBJS) -o $@ $(CUDA_LINK_LIBS) $(CC_LIBS)

# Compile main .cpp file to object files:
$(OBJ_DIR)/%.o : %.cpp
	$(CC) $(CC_FLAGS) -c $< -o $@ $(CC_LIBS)

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(CC) $(CC_FLAGS) -c $< -o $@

# Compile C++ source files to object files:
$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp $(INC_DIR)/%.h
	$(CC) $(CC_FLAGS) -c $< -o $@

# Compile CUDA source files to object files:
$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cu $(INC_DIR)/%.cuh
	$(NVCC) $(NVCC_FLAGS) -c $< -o $@ $(NVCC_LIBS)

# Clean objects in object directory.
clean:
	$(RM) bin/* *.o *.tmp $(EXE)




