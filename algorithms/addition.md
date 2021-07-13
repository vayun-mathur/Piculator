Back to [Homepage](../index.md)

Back to [Algorithms and Internals](../algorithms.md)

## Basic Addition

The representation of large numbers [is explained here](./number.md)

Adding and subtracting numbers in that representation is pretty much identical to how you would do it with decimal numbers, but instead
of base 10, it is base 2^32. It's pretty basic so I'll just move on to the implementation now.

### C and C++

In C++, it is not easy to tell when an overflow has occured in an addition, especially when you have a carry flag to deal with as well.

Therefore, the addition code in the Piculator makes use of compiler intrinsics to use useful assembly instructions for this purpose.

### Assembly

The `add` instruction in assembly sets the carry flag when the sum overflows.
The `adc` instruction (add with carry) is identical, but it also adds in the carry bit flag to the sum.

Adding a pair of large numbers in base 2^32 would simply require chaining the adc instruction.

It would look something like this: (where r8 and r9 are the locations of the addends, and r10 is the location of the sum)

```assembly
	mov     (%r8), eax;
	add     (%r9), eax;
	mov     eax, (%r10);

	mov     4(%r8), eax;
	adc     4(%r9), eax;
	mov     eax, 4(%r10);

	mov     8(%r8), eax;
	adc     8(%r9), eax;
	mov     eax, 8(%r10);
```

This could easily be converted to 64 bit too.

#### Compiler Intrinsics

Due to the fact that inline assembly is not supported on the MSVC compiler (which I am using).
This code is instead implemented with compiler intrinsics.

Here is a snippet of the addition code from the Piculator:

```c++
int64_t top = std::max(exp + L, x.exp + x.L);
int64_t bot = std::min(exp, x.exp);
uint32_t carry = 0;
for (size_t c = 0; bot < top; bot++, c++) {
    carry = _addcarry_u32(carry, word_at(bot), x.word_at(bot), &z.T[c]);
}
```

The .word_at() function gets the word at a certain magnitude (not index of array),
returning zero if there is no number in the array cooresponding with that magnitude.
This is the equivilant of adding padding zeroes on either side of the numbers so that they line up.

Demonstration:

Original
```
    3.566  
+1235.2
```
In this situation, some of the columns don't have values from both numbers

Padded
```
 0003.566  
+1235.200
```
In this situation, all of the columns have values from both numbers

## Subtraction

Subtraction is similar to addition, but requires subtraction and borrowing instead of addition and carrying.

## Parallel Addition

Since bignum addition is computational cheap, it is generally memory-bound. Therefore, parallelizing bignum addition is
currently not worthwhile.

If this does ever become a significant avenue of improvement though, I will implement it and explain it here.