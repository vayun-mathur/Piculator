#pragma once
#include <stdint.h>


#define NTT_64 1

#if NTT_64
using NNT_WORD = uint64_t;
#else
using NNT_WORD = uint32_t;
#endif

class NNT // Number theoretic transform
{
public:
    NNT() { r = 0; L = 0; p = 0; W = 0; iW = 0; rN = 0; WW = NULL; iWW = NULL; NN = 0; }
    ~NNT() { _free(); }

    // Main interface
    void  NTT(NNT_WORD* dst, NNT_WORD* src, int threads);                // NNT_WORD dst[n] = fast  NTT(NNT_WORD src[n])
    void iNTT(NNT_WORD* dst, NNT_WORD* src, int threads);               // NNT_WORD dst[n] = fast INTT(NNT_WORD src[n])

    // Modular arithmetics
    NNT_WORD mod(NNT_WORD a);
    NNT_WORD modadd(NNT_WORD a, NNT_WORD b);
    NNT_WORD modsub(NNT_WORD a, NNT_WORD b);
    NNT_WORD modmul(NNT_WORD a, NNT_WORD b);
    NNT_WORD modpow(NNT_WORD a, NNT_WORD b);

    bool init(NNT_WORD n);                                          // init r,L,p,W,iW,rN

private:
    // Internals
    void _free();                                            // Free precomputed W,iW powers tables
    void _alloc(NNT_WORD n);                                    // Allocate and precompute W,iW powers tables

    // Helper functions
    void  NTT_fast(NNT_WORD* dst, NNT_WORD* src, NNT_WORD n, NNT_WORD w, int threads);    // NNT_WORD dst[n] = fast  NTT(NNT_WORD src[n])
    void  NTT_fast(NNT_WORD* dst, NNT_WORD* src, NNT_WORD n, NNT_WORD* w2, NNT_WORD i2);

private:
    NNT_WORD r, L, p, N;
    NNT_WORD W, iW, rN;        // W=(r^L) mod p, iW=inverse W, rN = inverse N
    NNT_WORD* WW, * iWW, NN;    // Precomputed (W,iW)^(0,..,NN-1) powers
};

void ntt_ensure_table(int k);
void ntt_forward(NNT_WORD* T, int k, int threads);
void ntt_inverse(NNT_WORD* T, int k, int threads);
void ntt_pointwise(NNT_WORD* T, NNT_WORD* A, int k);
void int_to_ntt(NNT_WORD* T, int k, const uint32_t* A, size_t AL);
void ntt_to_int(NNT_WORD* T, int k, uint32_t* A, size_t AL);