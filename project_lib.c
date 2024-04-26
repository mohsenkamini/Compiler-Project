#include <stdio.h>
#include <stdlib.h>

// void print(int v)
// {
//     // check if bool or not
//     if (v == 1)
//         printf("1\n");
//     else if (v == 0)
//         printf("0\n");
//     else
//         printf("%d\n", v);
// }

void print(void *v){
    int val = *(int*)v;
    printf("%d\n", *(int*)v);
}