Back to [Homepage](../index.md)

Back to [Algorithms and Internals](../algorithms.md)

Multiplication is the most important part of the Piculator. Operations such as division, square root, and more all reduce to
large multiplication

## Overview

For the purpose of large multiplication, the Piculator currently employs two algorithms: the Fast Fourier Transform,
and the Number-Theoretic Transform

### Size Limits

The largest multiplication that the Piculator is currently theoretically capable of doing is around 
`(13.19 trillion) x (13.19 trillion)` decimal digits. This is achieved with the NTT. However, this operation would require so much memory
that I could never confirm this. For now, it is larger that we need.

## Multiplication Algorithms

### Basecase:

The Basecase algorithm is not currently implemented in the Piculator, though it may be implemented in the future for very small products.

### Floating-Point FFT:

The floating-point FFT currently handles small to medium sized products up to about 400 million digits.

This implementation of FFT has been sped up using the SSE3 SIMD instruction set.

The FFT is the fastest multiplication algorithm that exists by far. It is around 10x faster than the NTT.
As support for more SIMD instruction sets are added, FFT will continue to get faster and faster.

As fast as it is, the FFT has two major drawbacks: memory consumption and rounding error. For large products, the FFT uses way too much
memory and memory bandwith. The memory cost is super-linear because the number of bits that can be placed in each point
decreases for larger FFTs.

Rounding errors are the other big error and the reason why we don't use the FFT for large sized products.

### Number-Theoretic Transform:

The most common alternative to the Floating-Point FFT is the Number-Theoretic Transform (NTT). 
NTTs are about an order of magnitude slower than FFTs, but they require only a fraction of the memory cost. 
So while FFTs are bandwidth constrained, NTTs tend to be compute-bound - which makes them good candidates for parallelization