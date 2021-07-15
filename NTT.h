#pragma once
#include <stdint.h>

void ntt_ensure_table(int k);
void ntt_forward(uint64_t* T, int k, int threads);
void ntt_inverse(uint64_t* T, int k, int threads);
void ntt_pointwise(uint64_t* T, uint64_t* A, int k);
void int_to_ntt(uint64_t* T, int k, const uint32_t* A, size_t AL);
void ntt_to_int(uint64_t* T, int k, uint32_t* A, size_t AL);