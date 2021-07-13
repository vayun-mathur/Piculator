Back to [Homepage](../index.md)

Back to [Algorithms and Internals](../algorithms.md)

## Large Float Representation

The Piculator program represents floating point numbers in base 2^32 numbers stored in arrays of 32-bit integers in little-endian order

    BigFloat = sign * ( A[0] + A[1]*base^1 + A[2]*base^2 + A[3]*base^3 + ... ) * base^exp

The array "A" is the mantissa of the BigFloat object. It is stored in little-endian order so that:
+ Increasing address leads to increasing "magnitude" of digits. (A[0] is the one's digit. A[1] is the 10^9's digit... etc...)
+ Carry-propagation goes in the direction of increasing address.
+ A 32-bit based number would have the exact same binary representation in memory

The array "A" is an array of unsigned 32-bit integers. This is because the original Piculator was a 32 bit program, and this has not
been updated. Changing this will happen, but it doesn't really matter that much, since the most time consuming operations are not
done on this representation of the BigFloat. The FFT and FFT-like algorithms (NTT) first process the BigFloat into their own internal
representations, which means that the lack of 64-bit representation in the BigFloat is not that important.

## Large Integer Representation
Surprise, Surprise! Piculator does not have a specific Large Integer object.

When needing to use large integers, a BigFloat is used where the exponent is zero.

## Considerations

#### Bases
Numbers could either be represented in base 10^9 or base 2^32

Advantages of Base 10^9
+ Easier to convert to base 10 (decimal)

Advantages of Base 2^32
+ Less memory usage (2^32 is larger than 10^9, so it takes less space to store)
+ Add with carry can use the adc assembly instruction

For the Piculator, I chose base 2^32 for the reasons listed above.

This may end up changing in the future