#include <stdint.h>

#define ROUNDS 8
#define N 10
#define TL 60
#define unit64 (uint64_t)1


int sbox( int input );
int key_add(int input, int round_key);
uint64_t permutation(uint64_t input);
int enc(int input, uint64_t key3, uint64_t key2, uint64_t key1, uint64_t key0, uint64_t tweak);
int R1 (int input, uint64_t key);
int R2 (int input, uint64_t key);
int Gbox (int input);
int Gfunc (int input, uint64_t key);
uint64_t tweakey_schedule (uint64_t key3, uint64_t key2, uint64_t key1, uint64_t key0, uint64_t tweak,  uint64_t *RK);
uint64_t extract_bits(uint64_t input, unsigned int start, unsigned int end);
uint64_t rol(uint64_t num, unsigned int shift, int size);


uint64_t rol(uint64_t num, unsigned int shift, int size)
{
    uint64_t output = (num << shift) ^ (num >> (size - shift));
    output= output & ((unit64<<size)-1);

    return output;
}


int sbox( int input )
{
    int sbox5bit[1<<(N/2)] = {0,2,4,12,8,14,24,21,16,19,28,5,17,20,11,23,1,6,7,26,25,18,10,27,3,13,9,29,22,30,15,31};
    int output = sbox5bit[input];

    return output;
}


uint64_t permutation(uint64_t input)
{
    int P[TL]={ 0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55,
        1, 6, 11, 16, 21, 26, 31, 36, 41, 46, 51, 56,
        2, 7, 12, 17, 22, 27, 32, 37, 42, 47, 52, 57,
        3, 8, 13, 18, 23, 28, 33, 38, 43, 48, 53, 58,
        4, 9, 14, 19, 24, 29, 34, 39, 44, 49, 54, 59};

    uint64_t output=0;

    for ( int i =0; i < TL; i++)
    {
        if ( ((input >>i) &1) ==  1 )
        {
            output^= (unit64<< P[i]);
        }
    }

    return output;
}


int key_add(int input, int round_key)
{
    return input ^ round_key;
}


uint64_t Slayer( uint64_t input )
{
    uint64_t sbox5bit[1<<(N/2)] = {0,2,4,12,8,14,24,21,16,19,28,5,17,20,11,23,1,6,7,26,25,18,10,27,3,13,9,29,22,30,15,31};
    uint64_t temp_in =input;
    uint64_t output=0;

    for (int i=0; i < 12; i++)
    {
        output^=(sbox5bit[temp_in&0x1F]<<((N/2)*i));
        
        temp_in=temp_in>>(N/2);
    }

    return output;
}


int enc(int input, uint64_t key3, uint64_t key2, uint64_t key1, uint64_t key0, uint64_t tweak)
{
    uint64_t RK[ROUNDS];
    tweakey_schedule(key3, key2, key1, key0, tweak, &RK[0]);

    int ct = input;

    for (int i =0; i< ROUNDS-1; i++)
    {
        ct = R1(ct, RK[i]);
    }

    ct= R2(ct, RK[ROUNDS-1]);

    return ct;
}


uint64_t Sigma(uint64_t input)
{
    uint64_t t_0=input;
    uint64_t t_1=rol(input, 6, TL);
    uint64_t t_2=rol(input, 12, TL);
    uint64_t t_3=rol(input, 19, TL);
    uint64_t t_4=rol(input, 29, TL);
    uint64_t t_5=rol(input, 43, TL);
    uint64_t t_6=rol(input, 51, TL);

    return t_0^t_1^t_2^t_3^t_4^t_5^t_6;
}


uint64_t tweakey_schedule (uint64_t key3, uint64_t key2, uint64_t key1, uint64_t key0, uint64_t tweak, uint64_t *RK)
{
    tweak=tweak & 0xFFFFFFFFFFFF;


    for (int i=1 ; i< 12; i++)
    {
        uint64_t temp = (tweak >> (5*i-1)) << (5*i);

        tweak= (tweak & ( (unit64<<(5*i-1) ) - unit64)) ^ temp;
    }

    uint64_t K0=key0 & 0xFFFFFFFFFFFFFFF;
    uint64_t K1=key1 & 0xFFFFFFFFFFFFFFF;
    uint64_t K2=key2 & 0xFFFFFFFFFFFFFFF;
    uint64_t K3=key3 & 0xFFFFFFFFFFFFFFF;

    uint64_t T1 = tweak^K0;

    uint64_t T2 = Slayer(T1);
    uint64_t T3 = Sigma(T2)^K1;

    uint64_t T4 = Slayer(T3)^K2;
    uint64_t T5 = permutation(T4);
    uint64_t T6 = Slayer(T5);

    uint64_t T7 = Sigma(T6)^K3;
    uint64_t T8 = Slayer(T7);

    RK[0]=extract_bits(T1,0,30);
    RK[1]=extract_bits(T1,30,60);

    RK[2]=extract_bits(T3,0,30);
    RK[3]=extract_bits(T3,30,60);

    RK[4]=extract_bits(T6,0,30);
    RK[5]=extract_bits(T6,30,60);

    RK[6]=extract_bits(T8,0,30);
    RK[7]=extract_bits(T8,30,60);

    return T8;
}


uint64_t extract_bits(uint64_t input, unsigned int start, unsigned int end)
{
    uint64_t mask= (unit64<<end) - (unit64<<start);
    uint64_t output= (input & mask) >> start;

    return output;
}


int R1 (int input, uint64_t key)
{
    uint64_t SK0 = key & 0x1FFFFFF;
    int SK1 = (key &  0x3E000000)>>25;

    int xR=extract_bits(input,0,5);
    int xL=extract_bits(input,5,10);

    int temp_xL=xL;

    xL=xR^Gfunc(temp_xL, SK0);

    xR=sbox(temp_xL^SK1);


    return (xL<<5)^xR;
}


int R2 (int input, uint64_t key)
{
    uint64_t SK0 = key & 0x1FFFFFF;
    int SK1 = (key &  0x3E000000)>>25;

    int xR=extract_bits(input,0,5);
    int xL=extract_bits(input,5,10);

    int temp_xL=xL;

    xR=xR^Gfunc(temp_xL, SK0);

    xL=sbox(temp_xL)^SK1;

    return (xL<<5)^xR;
}


int Gfunc(int input, uint64_t key)
{
    int SK[5];

    for (int i=0; i<5; i++)
    {
        SK[i]= extract_bits(key, 5*i, 5*(i+1));
    }

    int x0=input & SK[0]; 
    int x1=rol(input,1, N/2)&SK[1];
    int x2=rol(input,2, N/2)&SK[2];
    int x3=rol(input,3, N/2)&SK[3];
    int x4=rol(input,4, N/2)&SK[4];

    int output= x0^x1^x2^x3^x4^(rol(input,1, N/2) & rol(input,2, N/2));

    return output;
}


int Gbox (int input)
{
    int t_1= rol(input, 1, N/2);
    int t_2= rol(input, 2, N/2);

    int output =input^ (t_1 & (t_2 ));

    return output;
}
