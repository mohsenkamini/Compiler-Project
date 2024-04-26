#!/bin/bash

# Step 1: Navigate to the build directory and run the compiler
cd build/code/
./compiler "$(cat ../../input.txt)" > compiler.ll

# Step 2: Compile the support library to an object file
clang -c ../../project_lib.c -o lib.o

# Step 3: Compile the LLVM IR to an object file
clang -c compiler.ll -o compiler.o

# Step 4: Link the object files to create the executable
clang compiler.o lib.o -o executable

# Step 5: Execute the program
./executable
