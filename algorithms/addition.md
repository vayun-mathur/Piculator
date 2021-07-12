Back to [Homepage](../index.md)

Back to [Algorithms and Internals](../algorithms.md)

## Basic Addition

The representation of large numbers [is explained here](./number.md)

Adding and subtracting numbers in that representation is pretty much identical to how you would do it with decimal numbers, but instead
of base 10, it is base 10^9. It's pretty basic so I'll just move on to the implementation now.

### C and C++

One of the nice things about using base 10^9 is the ease of addition and carrying. This is because even with the largest 
one digit addition (10^9 - 1 + 10^9 - 1), the result does not overflow. This means that you should carry if the result of the addition is
greater than 10^9, and if it is, you should subtract 10^9 from the result to get the final digit at that location.

Here is a snippet from the addition code of the Piculator:

```c++
int64_t top = std::max(x.exp + x.L, y.exp + y.L);
int64_t bot = std::min(x.exp, y.exp);

uint32_t carry = 0;
for (size_t c = 0; bot < top; bot++, c++) {
    uint32_t word = x.word_at(bot) + y.word_at(bot) + carry;
    carry = 0;
    if (word >= 1'000'000'000) {
        word -= 1'000'000'000;
        carry = 1;
    }
    z.T[c] = word;
}
```

x and y are the inputs, and z is the output.


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

### Assembly

I am writing this section for the situation in which I switch to using base 2^32.

In this case, I would not be able to use the method described in the C and C++ section for addition due to overflow.

For this, you will need to use assembly language. The `add` instruction in assembly sets the carry flag when the sum overflows.
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

## Subtraction

Subtraction is similar to addition, but involves checking for underflow instead of overflow, and borrowing instead of carrying.

## Parallel Addition

Since bignum addition is computational cheap, it is generally memory-bound. Therefore, parallelizing bignum addition is
currently not worthwhile.

If this does ever become a significant avenue of improvement though, I will implement it and explain it here.