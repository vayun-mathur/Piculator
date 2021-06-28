#include <math.h>
#include <complex>

using std::complex;

void fft_ensure_table(int k);
void fft_forward(complex<double>* T, int k);
void fft_inverse(complex<double>* T, int k);
void fft_pointwise(complex<double>* T, const complex<double>* A, int k);
void int_to_fft(complex<double>* T, int k, const uint32_t* A, size_t AL);
void fft_to_int(const complex<double>* T, int k, uint32_t* A, size_t AL);