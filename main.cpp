#include "BigInteger.h"
#include <iostream>

int main() {
	std::cout << std::hex << std::endl;
	BigInteger a = -1;
	BigInteger b = -1;
	std::cout << a << std::endl;
	std::cout << b << std::endl;
	BigInteger c = a * b;
	std::cout << c << std::endl;
}