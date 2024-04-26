#include <stdio.h>
#include <stdlib.h>

void print(int v){
    printf("%d\n", v);
}

void printBool(int v){
    if(v){
        printf("true\n");
    }else{
        printf("false\n");
    }
}