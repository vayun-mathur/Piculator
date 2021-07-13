Back to [Homepage](./index.md)

## Expanded Explanations
+ [Number Representation](./algorithms/number.md)
+ [Large Addition](./algorithms/addition.md)
+ [Large Multiplication](./algorithms/multiplication.md)
+ [Inverse Square Root](./algorithms/invsqrt.md)
+ [Division and Reciprocal](./algorithms/division.md)
+ [Radix Conversion](./algorithms/radix.md)
+ [Binary Splitting](./algorithms/binary-splitting.md)

## Implementation (v0.4.1)
#### General
+ Piculator is programmed in C++17
+ Intel SSE3 compiler intrinsics are used

The Piculator has no external dependencies. No Boost, GMP, etc.

The Piculator is compiled with the Microsoft Visual Studio compiler

#### Internal Requirements

+ Little-endian addressing
+ Sign-fill (arithmetic) right-shift
+ 64 bit integers and floating point numbers

#### Limits

| Category               	| Limit                                            	| Comments                          	|
|------------------------	|--------------------------------------------------	|-----------------------------------	|
| RAM Usage              	| ~ 1EiB (10^18 bytes)                             	| Limited by 64 bit address space   	|
| Disk Usage             	| ~ 1EiB                                           	| Limited by 64 bit address space   	|
| Largest Multiplication 	| (1.319 * 10^13) x (1.319 * 10^13) decimal digits 	| 64 bit Number Theoretic Transform 	|

