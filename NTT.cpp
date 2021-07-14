#include <intrin.h>
#include <immintrin.h>
#include <memory>
#include <map>
#include <thread>
#include "NTT.h"
#include <future>
#pragma intrinsic(_umul128)
#pragma intrinsic(_udiv128)



void NNT::_free()
{
	NN = 0;
	if (WW) delete[]  WW;  WW = NULL;
	if (iWW) delete[] iWW; iWW = NULL;
}


void NNT::_alloc(NNT_WORD n)
{
	/*
	WW = new NNT_WORD[n];
	WW[0] = 1;
	for (size_t i = 1; i < n; i++) {
		WW[i] = modmul(WW[i - 1], W);
	}

	iWW = new NNT_WORD[n];
	iWW[0] = 1;
	for (size_t i = 1; i < n; i++) {
		iWW[i] = modmul(iWW[i - 1], iW);
	}
	*/
	NN = n;
}


void NNT::NTT(NNT_WORD* dst, NNT_WORD* src, int threads)
{
	NTT_fast(dst, src, N, W, threads);
}


void NNT::iNTT(NNT_WORD* dst, NNT_WORD* src, int threads)
{
	NTT_fast(dst, src, N, iW, threads);
	for (NNT_WORD i = 0; i < N; i++) dst[i] = modmul(dst[i], rN);
}
NNT_WORD modmul(NNT_WORD a, NNT_WORD b, NNT_WORD p)
{
	uint64_t r, s = _umul128(a, b, &r);
	(void)_udiv128(r, s, p, &r);
	return r;
}
NNT_WORD modpow(NNT_WORD a, NNT_WORD b, NNT_WORD p) {
	NNT_WORD result = 1;
	while (b > 0) {
		if (b & 1)
			result = modmul(result, a, p);
		b >>= 1;
		a = modmul(a, a, p);
	}
	return result;
}

#include <vector>

NNT_WORD generator(NNT_WORD p) {
	std::vector<NNT_WORD> fact;
	NNT_WORD phi = p - 1, n = phi;
	for (NNT_WORD i = 2; i * i <= n; ++i)
		if (n % i == 0) {
			fact.push_back(i);
			while (n % i == 0)
				n /= i;
		}
	if (n > 1)
		fact.push_back(n);

	for (NNT_WORD res = 2; res <= p; ++res) {
		bool ok = true;
		for (size_t i = 0; i < fact.size() && ok; ++i)
			ok &= modpow(res, phi / fact[i], p) != 1;
		if (ok)  return res;
	}
	return -1;
}

std::map<int, NNT_WORD> primes = {
	{11, 9223372036854675457},
{12, 9223372036854497281},
{13, 9223372036854497281},
{14, 9223372036854497281},
{15, 9223372036853661697},
{16, 9223372036853661697},
{17, 9223372036844421121},
{18, 9223372036836950017},
{19, 9223372036836950017},
{20, 9223372036836950017},
{21, 9223372036752015361},
{22, 9223372036737335297},
{23, 9223372036737335297},
{24, 9223372036737335297},
{25, 9223372036083023873},
{26, 9223372035915251713},
{27, 9223372035915251713},
{28, 9223372034170421249},
{29, 9223372034170421249},
{30, 9223372006790004737},
{31, 9223372006790004737},
{32, 9223372006790004737},
{33, 9223371564408373249},
{34, 9223371280940531713},
{35, 9223371280940531713},
{36, 9223371280940531713},
{37, 9223369837831520257},
{38, 9223369837831520257},
{39, 9223369837831520257},
{40, 9223369837831520257},
{41, 9223369837831520257},
{42, 9223341250529198081}
};

bool PrimeQ(NNT_WORD w) {
	NNT_WORD s = ((NNT_WORD)sqrt(w) + 1);
	for (NNT_WORD i = 2; i <= s; i++) {
		if (w % i == 0) return false;
	}
	return true;
}


bool NNT::init(NNT_WORD n)
{
	/*
	L = (1ull << (64 - (int)round(log2(n)))) - 1;
	while (PrimeQ(L * n + 1) == false) L--;
	p = L * n + 1;
	r = generator(p);
	printf("Prime: %llu\n", p);
	*/
	p = primes[(int)round(log2(n))];
	r = generator(p);
	L = (p - 1) / n;

	N = n;                // Size of vectors [NNT_WORDs]
	W = modpow(r, L);  // Wn for NTT
	iW = modpow(r, p - 1 - L);  // Wn for INTT
	rN = modpow(n, p - 2);  // Scale for INTT
	_alloc(n >> 1);        // Precompute W,iW powers
	return true;
}


void NNT::NTT_fast(NNT_WORD* dst, NNT_WORD* src, NNT_WORD n, NNT_WORD w, int threads)
{

	if (n <= 1) { if (n == 1) dst[0] = src[0]; return; }
	NNT_WORD i, j, a0, a1, n2 = n >> 1, w2 = modmul(w, w);

	// Reorder even,odd
	for (i = 0, j = 0; i < n2; i++, j += 2) dst[i] = src[j];
	for (j = 1; i < n; i++, j += 2) dst[i] = src[j];

	// Recursion
	if (threads == 1) {
		NTT_fast(src, dst, n2, w2, 1);    // Even
		NTT_fast(src + n2, dst + n2, n2, w2, 1);    // Odd
	}
	else {
		int tds0 = threads / 2;
		int tds1 = threads - tds0;
		void (NNT::*func_ptr)(NNT_WORD *, NNT_WORD *, NNT_WORD, NNT_WORD, int) = &NNT::NTT_fast;
		auto x = std::async(std::launch::async, func_ptr, this, src, dst, n2, w2, tds0);    // Even
		NTT_fast(src + n2, dst + n2, n2, w2, tds1);    // Odd
		x.wait();
	}

	// Restore results
	for (w2 = 1, i = 0, j = n2; i < n2; i++, j++, w2 = modmul(w2, w))
	{
		a0 = src[i];
		a1 = modmul(src[j], w2);
		dst[i] = modadd(a0, a1);
		dst[j] = modsub(a0, a1);
	}
}


void NNT::NTT_fast(NNT_WORD* dst, NNT_WORD* src, NNT_WORD n, NNT_WORD* w2, NNT_WORD i2)
{
	if (n <= 1) { if (n == 1) dst[0] = src[0]; return; }
	NNT_WORD i, j, a0, a1, n2 = n >> 1;

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


NNT_WORD NNT::mod(NNT_WORD a)
{
	if (a > p) a -= p;
	return a;
}


NNT_WORD NNT::modadd(NNT_WORD a, NNT_WORD b)
{
	NNT_WORD d;
#if NTT_64
	bool cy = _addcarry_u64(0, a, b, &d);
#else
	bool cy = _addcarry_u32(0, a, b, &d);
#endif
	if (cy) d -= p;
	if (d >= p) d -= p;
	return d;
}


NNT_WORD NNT::modsub(NNT_WORD a, NNT_WORD b)
{
	NNT_WORD d;
#if NTT_64
	bool cy = _subborrow_u64(0, a, b, &d);
#else
	bool cy = _subborrow_u32(0, a, b, &d);
#endif
	if (cy) d += p;
	return d;
}


NNT_WORD NNT::modmul(NNT_WORD a, NNT_WORD b)
{
	uint64_t r, s = _umul128(a, b, &r);
	(void)_udiv128(r, s, p, &r);
	return r;
}
NNT_WORD NNT::modpow(NNT_WORD a, NNT_WORD b) {
	NNT_WORD result = 1;
	while (b > 0) {
		if (b & 1)
			result = modmul(result, a);
		b >>= 1;
		a = modmul(a, a);
	}
	return result;
}


std::map<int, NNT> ntts;

void ntt_ensure_table(int k)
{
	if (k <= 26) return;
	if (ntts.find(k) != ntts.end()) return;
	ntt_ensure_table(k - 1);
	ntts.insert({ k, NNT() });
	ntts[k].init(1ull << k);
}

void ntt_forward(NNT_WORD* T, int k, int threads)
{
	NNT& nnt = ntts[k];
	NNT_WORD* D = new NNT_WORD[(1ull << k)];
	nnt.NTT(D, T, threads);
	memcpy(T, D, sizeof(NNT_WORD) * (1ull << k));
	delete[] D;
}

void ntt_inverse(NNT_WORD* T, int k, int threads)
{
	NNT& nnt = ntts[k];
	NNT_WORD* D = new NNT_WORD[(1ull << k)];
	nnt.iNTT(D, T, threads);
	memcpy(T, D, sizeof(NNT_WORD) * (1ull << k));
	delete[] D;
}

void ntt_pointwise(NNT_WORD* T, NNT_WORD* A, int k)
{
	NNT& nnt = ntts[k];
	for (int i = 0; i < (1 << k); i++) T[i] = nnt.modmul(T[i], A[i]);
}

void int_to_ntt(NNT_WORD* T, int k, const uint32_t* A, size_t AL)
{
	for (size_t i = 0; i < AL; i++) {
		uint32_t X = A[i];
		T[2 * i] = X % 65536;
		X /= 65536;
		T[2 * i + 1] = X % 65536;
	}
	for (size_t i = 2 * AL; i < (1ull << k); i++) {
		T[i] = 0;
	}
}

void ntt_to_int(NNT_WORD* T, int k, uint32_t* A, size_t AL)
{

	//  Compute Scaling Factor
	size_t fft_length = (size_t)1 << k;

	//  Since there are 9 digits per word and we want to put 3 digits per
	//  point, the length of the transform must be at least 3 times the word
	//  length of the input.
	if (fft_length < 2 * AL)
		throw "FFT length is too small.";

	uint64_t carry = 0;
	for (size_t c = 0; c < AL; c++) {
		uint32_t word;

		carry += T[2 * c];                        //  Add to carry
		word = carry % 65536;                    //  Get 3 digits.
		carry /= 65536;

		carry += T[2 * c + 1];                  //  Add to carry
		word += (carry % 65536) * 65536;          //  Get 3 digits.
		carry /= 65536;

		A[c] = word;
	}
}
