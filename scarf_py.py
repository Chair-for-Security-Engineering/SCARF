import logging, sys
import random

logging.basicConfig(stream=sys.stderr, level=logging.WARNING)

# Rotate functions
ror = lambda val, r_bits, max_bits: ((val & (2**max_bits-1)) >> r_bits%max_bits) | (val << (max_bits-(r_bits%max_bits)) & (2**max_bits-1))
rol = lambda val, r_bits, max_bits: (val << r_bits%max_bits) & (2**max_bits-1) | ((val & (2**max_bits-1)) >> (max_bits-(r_bits%max_bits)))
NOT = lambda val, bits: val ^ (2**bits - 1)

tweakey = [0, 0, 0, 0]
P = [0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 1, 6, 11, 16, 21, 26, 31, 36, 41, 46, 51, 56, 2, 7, 12, 17, 22, 27, 32, 37, 42, 47, 52, 57, 3, 8, 13, 18, 23, 28, 33, 38, 43, 48, 53, 58, 4, 9, 14, 19, 24, 29, 34, 39, 44, 49, 54, 59]

def round_function_1(input : int, key : int):
    if input >= 1024:
        logging.debug("Input to large! - Round Function")
    k = [0]*6
    
    k[0] = key & 0x1F
    k[1] = (key >> 5) & 0x1f
    k[2] = (key >> 10) & 0x1f
    k[3] = (key >> 15) & 0x1f
    k[4] = (key >> 20) & 0x1f
    k[5] = (key >> 25) & 0x1f
        
    right = input & 0x1F
    left = (input >> 5) & 0x1F

    #print(f"Left {left:0x}, Right {right:0x}")

    l_prime = G(left, k) ^ right
    right = S(left ^ k[5])

    #print(f"Left' {l_prime:0x},Right' {right:0x}")

    return (l_prime << 5) | right

def round_function_2(input : int, key : int):
    if input >= 1024:
        logging.debug("Input to large! - Round Function")
    k = [0]*6
    
    k[0] = key & 0x1F
    k[1] = (key >> 5) & 0x1f
    k[2] = (key >> 10) & 0x1f
    k[3] = (key >> 15) & 0x1f
    k[4] = (key >> 20) & 0x1f
    k[5] = (key >> 25) & 0x1f
        
    right = input & 0x1F
    left = (input >> 5) & 0x1F

    right = G(left, k) ^ right
    left = S(left) ^ k[5]
    return (left << 5) | right

def G(x, k):
    #res = 0 
    #for i in range (0,5):
    res = (x ^ k[0])
    for i in range (1,5):
        res ^= ((rol(x, i, 5) & k[i]))
    res ^=  (rol(x, 1, 5) & rol(x, 2, 5))

    return res

def S(x : int):
    logging.debug(f"Starting SBox, input: {x:0x}.")
    t_0 = (x | rol(x, 1, 5)) & ((NOT(rol(x, 3, 5), 5)) | (NOT(rol(x, 4, 5), 5)))
    t_1 = (x | rol(x, 2, 5)) & ((NOT(rol(x, 2, 5), 5)) | rol(x, 3, 5))
    logging.debug(f"Finished SBox, output: {t_0 ^ t_1:0x}.")
    return t_0 ^ t_1

def tweakey_schedule(key, tweak):
    tweakey[0] = expansion(tweak) ^ (key & 2**60-1)
    logging.debug(f"Tweakey 0: {tweakey[0]:060b}")
    tweakey[1] = Sigma(SL(tweakey[0])) ^ ((key >> 60) & (2**60-1))
    logging.debug(f"Tweakey 1: {tweakey[1]:060b}")
    tweakey[2] = SL(pi(SL(tweakey[1]) ^ ((key >> 120) & (2**60-1))))
    logging.debug(f"Tweakey 2: {tweakey[2]:060b}")
    tweakey[3] = SL(Sigma(tweakey[2]) ^ ((key >> 180) & (2**60-1)))
    logging.debug(f"Tweakey 3: {tweakey[3]:060b}")

def expansion(tweak):
    as_bits = f"{tweak:048b}"
    logging.debug(f"Expansion input : {as_bits}")
    res = 0
    for i in (range(48)):
        if i % 4 == 0 and i != 0:
            res <<= 1
        res <<= 1
        res |= int(as_bits[i])
    logging.debug(f"Expansion output: {res:060b}")
    return res

def Sigma(x):
    return x ^ rol(x, 6, 60) ^ rol(x, 12, 60) ^ rol(x, 19, 60) ^ rol(x, 29, 60) ^ rol(x, 43, 60) ^ rol(x, 51, 60)

def SL(x):
    res = 0
    logging.debug(f"SL input : {x:060b}")
    for i in reversed(range(12)):
        res <<= 5
        res |= S((x >> i*5) & 0x1F)
        #x >>= 5
    logging.debug(f"SL output: {res:060b}")
    return res

def pi(x):
    logging.debug(f"Starting permutation: {x:060b}")
    as_bits = f"{x:060b}"
    res = 0
    for i in range(60):
        res |= int(as_bits[59-i]) << P[i]
        #res |= int(as_bits[P[i]])

    logging.debug(f"  Permutation output: {res:060b}")
    return res

def init(key, tweak):
    tweakey_schedule(key, tweak)
    print(f"Tweakey 0: {tweakey[0]:060b}")
    print(f"Tweakey 1: {tweakey[1]:060b}")
    print(f"Tweakey 2: {tweakey[2]:060b}")
    print(f"Tweakey 3: {tweakey[3]:060b}")

    print(f"RK 0: {tweakey[0] & 0x3FFFFFFF:030b}")
    print(f"RK 1: {(tweakey[0] >> 30) & 0x3FFFFFFF:030b}")
    print(f"RK 2: {(tweakey[1]) & 0x3FFFFFFF:030b}")
    print(f"RK 3: {(tweakey[1] >> 30) & 0x3FFFFFFF:030b}")
    print(f"RK 4: {(tweakey[2]) & 0x3FFFFFFF:030b}")
    print(f"RK 5: {(tweakey[2] >> 30) & 0x3FFFFFFF:030b}")
    print(f"RK 6: {(tweakey[3]) & 0x3FFFFFFF:030b}")
    print(f"RK 7: {(tweakey[3] >> 30) & 0x3FFFFFFF:030b}")

def enc(x):
    r = round_function_1(x, tweakey[0] & 0x3FFFFFFF)
    r = round_function_1(r, (tweakey[0] >> 30) & 0x3FFFFFFF)

    r = round_function_1(r, tweakey[1] & 0x3FFFFFFF)
    r = round_function_1(r, (tweakey[1] >> 30) & 0x3FFFFFFF)

    r = round_function_1(r, tweakey[2] & 0x3FFFFFFF)
    r = round_function_1(r, (tweakey[2] >> 30) & 0x3FFFFFFF)

    r = round_function_1(r, tweakey[3] & 0x3FFFFFFF)
    r = round_function_2(r, (tweakey[3] >> 30) & 0x3FFFFFFF)

    print(f"{r:03x}")


key = 0xEBA347BD715B4AE6E8BAE2BE82C35714014D1726D82676E50618AA168941 #random.getrandbits(240)
tweak = 0x71249C3CAAB0#random.getrandbits(48)


print(f"Key: 0x{key:060x}")
print(f"Tweak: 0x{tweak:012x}")

init(key, tweak)
#
for i in range(1024):
    enc(i)
#expansion(0x71249c3caab0)


#for i in range(32):
#    print(S(i))