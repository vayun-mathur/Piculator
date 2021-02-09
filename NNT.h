#pragma once
#include <stdint.h>

using DWORD = uint32_t;

class NNT // Number theoretic transform
{
public:
    NNT() { r = 0; L = 0; p = 0; W = 0; iW = 0; rN = 0; WW = NULL; iWW = NULL; NN = 0; }
    ~NNT() { _free(); }

    // Main interface
    void  NTT(DWORD* dst, DWORD* src, DWORD n = 0);                // DWORD dst[n] = fast  NTT(DWORD src[n])
    void iNTT(DWORD* dst, DWORD* src, DWORD n = 0);               // DWORD dst[n] = fast INTT(DWORD src[n])

    // Modular arithmetics
    DWORD mod(DWORD a);
    DWORD modadd(DWORD a, DWORD b);
    DWORD modsub(DWORD a, DWORD b);
    DWORD modmul(DWORD a, DWORD b);
    DWORD modpow(DWORD a, DWORD b);

private:
    // Internals
    void _free();                                            // Free precomputed W,iW powers tables
    void _alloc(DWORD n);                                    // Allocate and precompute W,iW powers tables

    // Helper functions
    bool init(DWORD n);                                          // init r,L,p,W,iW,rN
    void  NTT_fast(DWORD* dst, DWORD* src, DWORD n, DWORD w);    // DWORD dst[n] = fast  NTT(DWORD src[n])
    void  NTT_fast(DWORD* dst, DWORD* src, DWORD n, DWORD* w2, DWORD i2);

private:
    DWORD r, L, p, N;
    DWORD W, iW, rN;        // W=(r^L) mod p, iW=inverse W, rN = inverse N
    DWORD* WW, * iWW, NN;    // Precomputed (W,iW)^(0,..,NN-1) powers
};