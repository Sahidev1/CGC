#include <stdio.h>

void square(int val, int* ret){
    *ret = val*val;
}

int main_gc(void){
    int val = 3422;
    square(val, &val);
    printf("hello world %d\n", val);

    int i = 0;
    int j;
    while(i < __INT32_MAX__){
        j=i;
        while(j < __INT32_MAX__) j++;
        i++;
    }

    return 0;
}