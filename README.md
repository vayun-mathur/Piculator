# Piculator
This is a program that can compute the mathematical constant Pi to billions of digits.

Other pi calculators exist, but this is the first completely open source fast pi calculator

## Optimizations
Fast Fourier Transform for Multiplication
Vectorization of FFT using SSE3 assembly instructions
Parallelization of computation using multithreading

## Future updates
Better memory management
More vectorization of multiplication algorithms using SIMD (AVX, AVX2, AVX256)
Better thread management

## Benchmarks
### Intel i7-10510U
1,000,000 digits - 3 seconds
10,000,000 digits - 35 seconds
100,000,000 digits - 8 minutes, 46 seconds
1,000,000,000 digits - 18 hours, 8 minutes, 51 seconds (with inefficient threading)