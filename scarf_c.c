#include "scarf_c.h"
#include <stdio.h>

int main(){
    for(int i=0; i<1024; i++)
        printf("%3x\n", enc(i, 0xEBA347BD715B4AE, 0x6E8BAE2BE82C357, 0x14014D1726D8267, 0x6E50618AA168941, 0x71249C3CAAB0));
}