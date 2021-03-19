#include "BigInteger.h"
#include "BinarySplitting.h"
#include <list>
#include <thread>
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <functional>

struct ESplitting : BinarySplitting {
public:
	ESplitting()
		: BinarySplitting(FINAL_FUNC(BigFloat(1) + p / q))
	{
		functions.push_back({ 'Q', BinarySplitting::Function(false, true,
			INIT_FUNC(BigInteger(b)),
			COMBINE_FUNC(am * mb),
			'Q') });

		functions.push_back({ 'P', BinarySplitting::Function(false, false,
			INIT_FUNC(BigInteger(1)),
			COMBINE_FUNC(am * tree['Q'][mb_int] + mb),
			'P') });
	}
};

struct PiSplitting : BinarySplitting {
public:
	PiSplitting()
		: BinarySplitting(FINAL_FUNC((q * 4270934400ll) / (p + q * 13591409ll) * invsqrt(BigFloat(10005))))
	{
		functions.push_back({ 'Q', BinarySplitting::Function(false, true,
			INIT_FUNC(BigInteger(10939058860032000ll) * b * b * b),
			COMBINE_FUNC(am * mb),
			'Q') });

		functions.push_back({ 'R', BinarySplitting::Function(true, false,
			INIT_FUNC(BigInteger(2 * b - 1) * (6 * b - 5) * (6 * b - 1)),
			COMBINE_FUNC(am * mb),
			'R') });

		functions.push_back({ 'P', BinarySplitting::Function(false, false,
			INIT_FUNC((BigInteger(b) * 545140134ll + 13591409ll) * (2 * b - 1) * (6 * b - 5) * (6 * b - 1) * (b % 2 == 0 ? 1ll : -1ll)),
			COMBINE_FUNC(am * tree['Q'][mb_int] + mb * tree['R'][am_int]),
			'P') });
	}
};

template<typename t>
t get(std::string message) {
	std::cout << message << std::endl;
	t x;
	std::cin >> x;
	return x;
}

int get_option(std::string message, std::string* options, size_t option_count) {
	std::cout << message << '\n';
	for (int i = 1; i <= option_count; i++) {
		std::cout << i << ") " << options[i - 1] << '\n';
	}
	std::cout.flush();
	int choice;
	std::cin >> choice;
	while (choice > option_count) {
		printf("Choose a number between 1 and %llu\n", option_count);
		std::cout.flush();
	}
	return choice-1;
}

std::string getLine(std::string message) {
	std::cout << message << std::endl;
	std::string s;
	std::cin.get();
	std::getline(std::cin, s);
	return s;
}

size_t correctDigits(std::string& real, std::string& our_guess) {
	size_t chars = std::min(real.size(), our_guess.size());
	for (size_t i = 0; i < chars; i++) {
		if (real[i] != our_guess[i]) {
			return i;
		}
	}
	return chars;
}

std::string options[] = {
	"pi", "e"
};

int main() {

	//inputs
	int option = get_option("Choose a constant to calculate", options, sizeof(options)/sizeof(std::string));
	int threads = get<int>("How many threads to use? (Choose a power of 2)");
	index _msb = get<index>("How many 64 bit words to use in BigInteger? (Choose a power of 2)");
	index _lsb = -get<index>("How many 64 bit decimal words to use in BigInteger? (Choose a power of 2)");
	int_msb = _msb, int_lsb = 0, float_msb = _msb, float_lsb = _lsb;
	int64 max_b = get<int64>("How many iterations?");

	std::ofstream outFile(getLine("File to output calculated digits:"));
	std::ifstream inFile(getLine("File to input correct digits:"));
	std::string realConstantString(std::istreambuf_iterator<char>{inFile}, {});

	//calculate binary sums
	BinarySplitting* binary_splitting = nullptr;
	int64 x = 0;
	switch (option) {
	case 0:
		binary_splitting = new PiSplitting();
		break;
	case 1:
		binary_splitting = new ESplitting();
		break;
	}

	long long BS_time, final_time;
	BigFloat constant = binary_splitting->calculate(9, max_b, BS_time, final_time, threads);
	printf("\nCalculating Binary Splitting functions took %llu ms\n", BS_time);
	printf("\nFinal calculation took %llu ms\n", final_time);

	std::string constantString = constant.toHexString();


	//compare with real value
	size_t digits = correctDigits(realConstantString, constantString);
	printf("Matched %lli digits past the decimal point\n", digits-2);
	if (digits < std::min(constantString.size(), realConstantString.size()))
		printf("TIP: Increase iterations to calculate more digits\n");
	else
		printf("TIP: Increase size of BigInteger to calculate more digits\n");

	//write calculated value to file
	std::cout << constantString << std::endl;
	outFile << constantString << std::endl;
}