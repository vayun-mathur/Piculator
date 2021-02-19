#include "BigInteger.h"
#include "BinarySplitting.h"
#include <list>
#include <thread>
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>

BigInteger r, total;

BigInteger _P(const BigInteger& pam, const BigInteger& pmb, int a, int m, int b, BinarySplitting::StoredMap& tree) {
	return pam * tree['Q'][{m, b}] + pmb * tree['R'][{a, m}];
}

BigInteger _Q(const BigInteger& qam, const BigInteger& qmb, int a, int m, int b, BinarySplitting::StoredMap& tree) {
	return qam * qmb;
}

BigInteger _R(const BigInteger& ram, const BigInteger& rmb, int a, int m, int b, BinarySplitting::StoredMap& tree) {
	return ram * rmb;
}

struct PiSplitting : BinarySplitting {
public:
	PiSplitting(int max_b, int threads)
		: BinarySplitting(max_b, threads)
	{

	}

	BigInteger PMultiThreaded(int a, int b) {
		auto init = [](int _b)->BigInteger {
			int64 __b = _b;
			return (BigInteger(__b) * 545140134ll + 13591409ll) * (2 * __b - 1) * (6 * __b - 5) * (6 * __b - 1) * (__b % 2 == 0 ? 1 : -1);
		};
		return MultiThreaded(a, b, true, false, init, _P, 'P');
	}

	BigInteger QMultiThreaded(int a, int b) {
		auto init = [](int _b)->BigInteger {
			int64 __b = _b;
			return BigInteger(10939058860032000ll) * __b * __b * __b;
		};
		return MultiThreaded(a, b, true, false, init, _Q, 'Q');
	}

	BigInteger RMultiThreaded(int a, int b) {
		auto init = [](int _b)->BigInteger {
			int64 __b = _b;
			return BigInteger(2 * __b - 1) * (6 * __b - 5) * (6 * __b - 1);
		};
		return MultiThreaded(a, b, true, false, init, _R, 'R');
	}
};

long long timer() {
	static auto start = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
	start = std::chrono::high_resolution_clock::now();
	return duration;
}
template<typename t>
t get(std::string message) {
	std::cout << message << std::endl;
	t x;
	std::cin >> x;
	return x;
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

int main() {
	//read real number
	std::ifstream hexEFile("C:\\Users\\Vayun Mathur\\OneDrive\\Documents\\code\\c++\\Math\\Hex Digits.txt");
	std::stringstream buffer;
	//buffer << hexEFile.rdbuf();

	//inputs
	int threads = get<int>("How many threads to use? (Choose a power of 2)");
	index _msb = get<index>("How many 64 bit words to use in BigInteger? (Choose a power of 2)");
	index _lsb = -get<index>("How many 64 bit decimal words to use in BigInteger? (Choose a power of 2)");
	int_msb = _msb;
	int_lsb = 0;
	float_msb = _msb;
	float_lsb = _lsb;
	int max_b = get<int>("How many iterations?");


	PiSplitting binary_splitting(max_b, threads);

	std::ofstream outFile(getLine("File to output calculated digits:"));

	//calculate binary sums
	timer();
	BigFloat q = binary_splitting.QMultiThreaded(0, max_b);
	long long lq = timer();
	BigFloat r = binary_splitting.RMultiThreaded(0, max_b);
	long long lr = timer();
	BigFloat p = binary_splitting.PMultiThreaded(0, max_b);
	long long lp = timer();
	printf("\nCalculating Q, R, and P of pi took %llu ms\n", lp + lq + lr);

	//calculate final value;
	BigFloat e = (q * 4270934400ll) / (p + q * 13591409ll) * invsqrt(BigFloat(10005));
	long long lpi = timer();
	printf("Final division to calculate pi took %llu ms\n\n", lpi);

	//compare with real value
	std::stringstream eStringStream;
	eStringStream << std::hex << e;
	std::string eString = eStringStream.str();
	std::string realE = buffer.str();

	size_t digits = correctDigits(realE, eString);
	std::cout << "Matched " << std::dec << digits - 2 << " digits past the decimal point" << std::endl;
	if (digits < std::min(eString.size(), realE.size())) {
		std::cout << "TIP: Increase iterations to calculate more digits" << std::endl;
	}
	else {
		std::cout << "TIP: Increase size of BigInteger to calculate more digits" << std::endl;
	}
	std::cout << std::hex << eString << std::endl;

	//write calculated value to file
	outFile << std::hex << eString << std::endl;
}