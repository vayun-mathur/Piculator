## The Piculator

The best completely open-source Pi calculator

The Piculator is a program that can compute Pi to billions of digits.

## Features

The main features of Piculator are:

+ Able to compute Pi to billions of digits.
+ Fast Large Multiplication  The core of this program is the O(N log N) multiplication algorithms it uses
+ MultiThreaded  Multithreading used to utilize all cores of a processor, still needs some work
+ Vectorized  Able to fully utilize SSE3 instructions, still need to work on AVX, AVX256, etc...

## Download

#### Latest Release

| OS      	| Download Link                                                                                                      	| Size  	|
|---------	|--------------------------------------------------------------------------------------------------------------------	|-------	|
| Windows 	| [Piculator v0.4.1 Windows Edition](https://github.com/vayun-mathur/Piculator/releases/download/v0.4/Piculator.exe) 	| 59 KB 	|

#### System Requirements

+ Windows 10
+ 64 bit OS
+ Processor with SSE3 instruction set

#### Other Downloads

[Source Code](https://github.com/vayun-mathur/Piculator)

## Other Links

+ [Algorithms and Internals](./algorithms.md)
+ [Technology](./technology.md)

## Benchmarks

Computations of Pi to various sizes. The timings do not include the time taken to write the digits to disk.

| Processor       	|       Intel Core i7 10510U      	|         Intel Xeon Platinum 8272CL      |
|-----------------	|:-------------------------------:	|:--------------------------------------:	|
| Generation      	|         Intel Comet Lake        	|            Intel Cascade Lake           |
| Cores/Threads   	|               4/8               	|                   16/32                 |
| Clock Speed     	|             1.8 GHz             	|                  2.6 GHz                |
| Memory          	|              20 GB              	|                   16 GB               	|
| Version         	|              v0.4.1             	|                  v0.4.2               	|
| Instruction Set 	|               SSE3              	|                   SSE3                	|
|       1,000,000 	|            3 seconds            	|                0 seconds              	|
|      10,000,000 	|            35 seconds           	|                11 seconds           	  |
|     100,000,000 	|      8 minutes, 46 seconds      	|          4 minutes, 53 seconds         	|
|   1,000,000,000 	| 18 hours, 8 minutes, 51 seconds 	|                    TBD               	  |
