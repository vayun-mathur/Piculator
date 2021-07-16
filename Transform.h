#pragma once
#include "NTT.h"
#include "FFT.h"

struct Transform {
	void* memory;
	int k;
	bool sign;
	int64_t exp;
	size_t L;
};

inline void mulNTT(Transform& t1, Transform t2) {
	ntt_pointwise((uint64_t*)t1.memory, (uint64_t*)t2.memory, t1.k);
	t1.sign = t1.sign == t2.sign;
	t1.exp += t2.exp;
	t1.L += t2.L;
}

inline void mulFFT(Transform& t1, Transform t2) {
	fft_pointwise((__m128d*)t1.memory, (__m128d*)t2.memory, t1.k);
	t1.sign = t1.sign == t2.sign;
	t1.exp += t2.exp;
	t1.L += t2.L;
}

inline void deleteNTT(Transform t1) {
	free(t1.memory);
}

inline void deleteFFT(Transform t1) {
	_mm_free(t1.memory);
}