#include "NNT.h"
#include <intrin.h>
#include <immintrin.h>

void NNT::_free()
{
	NN = 0;
	if (WW) delete[]  WW;  WW = NULL;
	if (iWW) delete[] iWW; iWW = NULL;
}


void NNT::_alloc(DWORD n)
{
	if (n <= NN) return;
	DWORD* tmp, i, w;
	tmp = new DWORD[n]; if ((NN) && (WW)) for (i = 0; i < NN; i++) tmp[i] = WW[i]; if (WW) delete[]  WW;  WW = tmp;  WW[0] = 1; for (i = NN ? NN : 1, w = WW[i - 1]; i < n; i++) { w = modmul(w, W);  WW[i] = w; }
	tmp = new DWORD[n]; if ((NN) && (iWW)) for (i = 0; i < NN; i++) tmp[i] = iWW[i]; if (iWW) delete[] iWW; iWW = tmp; iWW[0] = 1; for (i = NN ? NN : 1, w = iWW[i - 1]; i < n; i++) { w = modmul(w, iW); iWW[i] = w; }
	NN = n;
}


void NNT::NTT(DWORD* dst, DWORD* src, DWORD n)
{
	if (n > 0) init(n);
	NTT_fast(dst, src, N, WW, 1);
}


void NNT::iNTT(DWORD* dst, DWORD* src, DWORD n)
{
	if (n > 0) init(n);
	NTT_fast(dst, src, N, iWW, 1);
	for (DWORD i = 0; i < N; i++) dst[i] = modmul(dst[i], rN);
}


bool NNT::init(DWORD n)
{
	// (max(src[])^2)*n < p else NTT overflow can ocur!!!
	r = 2; p = 0xC0000001; if ((n < 2) || (n > 0x10000000)) { r = 0; L = 0; p = 0; W = 0; iW = 0; rN = 0; N = 0; return false; } L = 0x30000000 / n; // 32:30 bit best for unsigned 32 bit
//    r=2; p=0x78000001; if ((n<2)||(n>0x04000000)) { r=0; L=0; p=0; W=0; iW=0; rN=0; N=0; return false; } L=0x3c000000/n; // 31:27 bit best for signed 32 bit
//    r=2; p=0x00010001; if ((n<2)||(n>0x00000020)) { r=0; L=0; p=0; W=0; iW=0; rN=0; N=0; return false; } L=0x00000020/n; // 17:16 bit best for 16 bit
//    r=2; p=0x0a000001; if ((n<2)||(n>0x01000000)) { r=0; L=0; p=0; W=0; iW=0; rN=0; N=0; return false; } L=0x01000000/n; // 28:25 bit
	N = n;                // Size of vectors [DWORDs]
	W = modpow(r, L);  // Wn for NTT
	iW = modpow(r, p - 1 - L);  // Wn for INTT
	rN = modpow(n, p - 2);  // Scale for INTT
	_alloc(n >> 1);        // Precompute W,iW powers
	return true;
}


void NNT::NTT_fast(DWORD* dst, DWORD* src, DWORD n, DWORD w)
{

	if (n <= 1) { if (n == 1) dst[0] = src[0]; return; }
	DWORD i, j, a0, a1, n2 = n >> 1, w2 = modmul(w, w);

	// Reorder even,odd
	for (i = 0, j = 0; i < n2; i++, j += 2) dst[i] = src[j];
	for (j = 1; i < n; i++, j += 2) dst[i] = src[j];

	// Recursion
	NTT_fast(src, dst, n2, w2);    // Even
	NTT_fast(src + n2, dst + n2, n2, w2);    // Odd

	// Restore results
	for (w2 = 1, i = 0, j = n2; i < n2; i++, j++, w2 = modmul(w2, w))
	{
		a0 = src[i];
		a1 = modmul(src[j], w2);
		dst[i] = modadd(a0, a1);
		dst[j] = modsub(a0, a1);
	}
}


void NNT::NTT_fast(DWORD* dst, DWORD* src, DWORD n, DWORD* w2, DWORD i2)
{
	if (n <= 1) { if (n == 1) dst[0] = src[0]; return; }
	DWORD i, j, a0, a1, n2 = n >> 1;

	// Reorder even,odd
	for (i = 0, j = 0; i < n2; i++, j += 2) dst[i] = src[j];
	for (j = 1; i < n; i++, j += 2) dst[i] = src[j];

	// Recursion
	i = i2 << 1;
	NTT_fast(src, dst, n2, w2, i);    // Even
	NTT_fast(src + n2, dst + n2, n2, w2, i);    // Odd
	// Restore results

	for (i = 0, j = n2; i < n2; i++, j++, w2 += i2)
	{
		a0 = src[i];
		a1 = modmul(src[j], *w2);
		dst[i] = modadd(a0, a1);
		dst[j] = modsub(a0, a1);
	}
}


DWORD NNT::mod(DWORD a)
{
	if (a > p) a -= p;
	return a;
}


DWORD NNT::modadd(DWORD a, DWORD b)
{
	DWORD d;
	bool cy = _addcarry_u32(0, a, b, &d);
	if (cy) d -= p;
	if (d > p) d -= p;
	return d;
}


DWORD NNT::modsub(DWORD a, DWORD b)
{
	DWORD d;
	bool cy = _subborrow_u32(0, a, b, &d);
	if (cy) d += p;
	if (d > p) d -= p;
	return d;
}


DWORD NNT::modmul(DWORD a, DWORD b)
{
	return (((uint64_t)a) * b) % p;
}
DWORD NNT::modpow(DWORD a, DWORD b) {
	DWORD result = 1;
	while (b > 0) {
		if (b & 1)
			result = modmul(result, a);
		b >>= 1;
		a = modmul(a, a);
	}
	return result;
}