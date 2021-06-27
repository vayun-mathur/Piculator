#pragma once
#include <string.h>

typedef unsigned int uint32;

typedef unsigned long long uint64;

class fast_ntt                                   // number theoretic transform
{
public:
    fast_ntt()
    {
        r = 0; L = 0;
        W = 0; iW = 0; rN = 0;
    }
    // main interface
    void  NTT(uint32* dst, uint32* src, uint32 n = 0);             // uint32 dst[n] = fast  NTT(uint32 src[n])
    void INTT(uint32* dst, uint32* src, uint32 n = 0);             // uint32 dst[n] = fast INTT(uint32 src[n])
    // helper functions

    bool init(uint32 n);                                     // init r,L,p,W,iW,rN
    inline uint32 modmul(uint32 a, uint32 b);
private:
    void NTT_calc(uint32* dst, uint32* src, uint32 n, uint32 w);  // uint32 dst[n] = fast  NTT(uint32 src[n])

    void  NTT_fast(uint32* dst, uint32* src, uint32 n, uint32 w);  // uint32 dst[n] = fast  NTT(uint32 src[n])
    void NTT_fast(uint32* dst, const uint32* src, uint32 n, uint32 w);
    // only for testing
    void  NTT_slow(uint32* dst, uint32* src, uint32 n, uint32 w);  // uint32 dst[n] = slow  NTT(uint32 src[n])
    void INTT_slow(uint32* dst, uint32* src, uint32 n, uint32 w);  // uint32 dst[n] = slow INTT(uint32 src[n])
    // uint32 arithmetics


    // modular arithmetics
    inline uint32 modadd(uint32 a, uint32 b);
    inline uint32 modsub(uint32 a, uint32 b);
    inline uint32 modpow(uint32 a, uint32 b);

    uint32 r, L, N;//, p;
    uint32 W, iW, rN;

    const uint32 p = 0xC0000001;
};