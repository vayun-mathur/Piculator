# Pi Calculator
This is a program that can compute the mathematical constant Pi to millions of digits.

## Optimizations
Fast Fourier Transform for Multiplication
Vectorization of FFT using SSE3 assembly instructions
Parallelization of computation using multithreading

## Future updates
Multiplication for larger numbers (FFT only works for numbers up to 1,610,612,736 decimal digits)
* NNT (Number Theoretic Transform)
Faster memory allocation
More vectorization of multiplication (AVX, AVX2, AVX256)

## Benchmarks
### Intel i7-10510U
1,000,000 digits - 3 seconds
10,000,000 digits - 35 seconds
100,000,000 digits - 8 minutes, 46 seconds