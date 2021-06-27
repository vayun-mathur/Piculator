#include "NTT.h"

void fast_ntt::NTT(uint32* dst, uint32* src, uint32 n)
{
    if (n > 0)
    {
        init(n);
    }
    NTT_fast(dst, src, N, W);
    //  NTT_slow(dst,src,N,W);
}

//---------------------------------------------------------------------------
void fast_ntt::INTT(uint32* dst, uint32* src, uint32 n)
{
    if (n > 0)
    {
        init(n);
    }
    NTT_fast(dst, src, N, iW);
    for (uint32 i = 0; i < N; i++)
    {
        dst[i] = modmul(dst[i], rN);
    }
    //  INTT_slow(dst,src,N,W);
}

//---------------------------------------------------------------------------
bool fast_ntt::init(uint32 n)
{
    // (max(src[])^2)*n < p else NTT overflow can ocur !!!
    r = 2;
    //p = 0xC0000001;
    if ((n < 2) || (n > 0x10000000))
    {
        r = 0; L = 0; W = 0; // p = 0;
        iW = 0; rN = 0; N = 0;
        return false;
    }
    L = 0x30000000 / n; // 32:30 bit best for unsigned 32 bit
    //  r=2; p=0x78000001; if ((n<2)||(n>0x04000000)) { r=0; L=0; p=0; W=0; iW=0; rN=0; N=0; return false; } L=0x3c000000/n; // 31:27 bit best for signed 32 bit
    //  r=2; p=0x00010001; if ((n<2)||(n>0x00000020)) { r=0; L=0; p=0; W=0; iW=0; rN=0; N=0; return false; } L=0x00000020/n; // 17:16 bit best for 16 bit
    //  r=2; p=0x0a000001; if ((n<2)||(n>0x01000000)) { r=0; L=0; p=0; W=0; iW=0; rN=0; N=0; return false; } L=0x01000000/n; // 28:25 bit
    N = n;               // size of vectors [uint32s]
    W = modpow(r, L); // Wn for NTT
    iW = modpow(r, p - 1 - L); // Wn for INTT
    rN = modpow(n, p - 2); // scale for INTT
    return true;
}

//---------------------------------------------------------------------------

void fast_ntt::NTT_fast(uint32* dst, uint32* src, uint32 n, uint32 w)
{
    if (n > 1)
    {
        if (dst != src)
        {
            NTT_calc(dst, src, n, w);
        }
        else
        {
            uint32* temp = new uint32[n];
            NTT_calc(temp, src, n, w);
            memcpy(dst, temp, n * sizeof(uint32));
            delete[] temp;
        }
    }
    else if (n == 1)
    {
        dst[0] = src[0];
    }
}

void fast_ntt::NTT_fast(uint32* dst, const uint32* src, uint32 n, uint32 w)
{
    if (n > 1)
    {
        uint32* temp = new uint32[n];
        memcpy(temp, src, n * sizeof(uint32));
        NTT_calc(dst, temp, n, w);
        delete[] temp;
    }
    else if (n == 1)
    {
        dst[0] = src[0];
    }
}



void fast_ntt::NTT_calc(uint32* dst, uint32* src, uint32 n, uint32 w)
{
    if (n > 1)
    {
        uint32 i, j, a0, a1,
            n2 = n >> 1,
            w2 = modmul(w, w);

        // reorder even,odd
        for (i = 0, j = 0; i < n2; i++, j += 2)
        {
            dst[i] = src[j];
        }
        for (j = 1; i < n; i++, j += 2)
        {
            dst[i] = src[j];
        }
        // recursion
        if (n2 > 1)
        {
            NTT_calc(src, dst, n2, w2);  // even
            NTT_calc(src + n2, dst + n2, n2, w2);  // odd
        }
        else if (n2 == 1)
        {
            src[0] = dst[0];
            src[1] = dst[1];
        }

        // restore results

        w2 = 1, i = 0, j = n2;
        a0 = src[i];
        a1 = src[j];
        dst[i] = modadd(a0, a1);
        dst[j] = modsub(a0, a1);
        while (++i < n2)
        {
            w2 = modmul(w2, w);
            j++;
            a0 = src[i];
            a1 = modmul(src[j], w2);
            dst[i] = modadd(a0, a1);
            dst[j] = modsub(a0, a1);
        }
    }
}

//---------------------------------------------------------------------------
void fast_ntt::NTT_slow(uint32* dst, uint32* src, uint32 n, uint32 w)
{
    uint32 i, j, wj, wi, a,
        n2 = n >> 1;
    for (wj = 1, j = 0; j < n; j++)
    {
        a = 0;
        for (wi = 1, i = 0; i < n; i++)
        {
            a = modadd(a, modmul(wi, src[i]));
            wi = modmul(wi, wj);
        }
        dst[j] = a;
        wj = modmul(wj, w);
    }
}

//---------------------------------------------------------------------------
void fast_ntt::INTT_slow(uint32* dst, uint32* src, uint32 n, uint32 w)
{
    uint32 i, j, wi = 1, wj = 1, a, n2 = n >> 1;

    for (wj = 1, j = 0; j < n; j++)
    {
        a = 0;
        for (wi = 1, i = 0; i < n; i++)
        {
            a = modadd(a, modmul(wi, src[i]));
            wi = modmul(wi, wj);
        }
        dst[j] = modmul(a, rN);
        wj = modmul(wj, iW);
    }
}


//---------------------------------------------------------------------------
uint32 fast_ntt::modadd(uint32 a, uint32 b)
{
    uint32 d;
    d = a + b;

    if (d < a)
    {
        d -= p;
    }
    if (d >= p)
    {
        d -= p;
    }
    return d;
}

//---------------------------------------------------------------------------
uint32 fast_ntt::modsub(uint32 a, uint32 b)
{
    uint32 d;
    d = a - b;
    if (d > a)
    {
        d += p;
    }
    return d;
}

//---------------------------------------------------------------------------
uint32 fast_ntt::modmul(uint32 a, uint32 b)
{
    uint32 _a = a;
    uint32 _b = b;
    return ((uint64)a)*b%p;
}


uint32 fast_ntt::modpow(uint32 a, uint32 b)
{
    //*
    uint32 D, M, A, P;

    P = p; A = a;
    M = 0llu - (b & 1);
    D = (M & A) | ((~M) & 1);

    while ((b >>= 1) != 0)
    {
        A = modmul(A, A);
        //A = (A * A) % P;

        if ((b & 1) == 1)
        {
            //D = (D * A) % P;
            D = modmul(D, A);
        }
    }
    return (uint32)D;
}