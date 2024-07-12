# Compiler Project

In this project, we create a new programming language similar to C and design its compiler using LLVM.

## How to use?

1. Type your code in the `input.txt` file. For instance:
    ```c
    int x = 0;
    int i, j;
    for (i = 1; i <= 5; i++) {
        for (j = 1; j <= 5; j++) {
            if (j % 3 == 1) {
                x += j;
            }
            else if (j % 3 == 2) {
                x += i;
            }
            else {
                x += i + j;
            }
        }
    }
    print(x);
    ```

2. To build the compiler, use this script.
    ```bash
    ./makeBuild.sh
    ```

3. To view the output of the code, run the script provided:
    ```bash
    ./makeRun.sh
    ```
4. To enable optimizer you should set the variable ```optimize``` to true.
   ```c++
   bool optimize = true;
   ```
## Contributors

- [Mohammad Nakhjiri](https://github.com/mnakhjiri)
- [Arash Rezaali](https://github.com/Arash-ra03)
- [Moein Arabi](https://github.com/ILoveBacteria)
