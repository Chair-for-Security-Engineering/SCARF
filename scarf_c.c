#include "scarf_c.h"
#include <stdio.h>

int main(){
    uint64_t k0 = 0x6E50618AA168941;
    uint64_t k1 = 0x14014D1726D8267;
    uint64_t k2 = 0x6E8BAE2BE82C357;
    uint64_t k3 = 0xEBA347BD715B4AE;
    uint64_t t = 0x71249C3CAAB0;

    for(int i=0; i<64; i++){
        for(int j=0; j<16; j++){
            printf("%03x ", enc(i*16+j, k3, k2, k1, k0, t));
        }
        printf("\n");
    }
}