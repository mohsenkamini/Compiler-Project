#include <stdio.h>
#include <stdlib.h>

void print(int v){
    printf("%d\n", v);
}

void print(bool v){
    printf("%s\n", v ? "true" : "false");
}