#include "BigInteger.h"
#include "BinarySplitting.h"
#include <list>
#include <thread>
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>

BigInteger r, total;

BigInteger _P(const BigInteger& pam, const BigInteger& pmb, int64 a, int64 m, int64 b, BinarySplitting::StoredMap& tree) {
	return pam * tree['Q'][{m, b}] + pmb * tree['R'][{a, m}];
}

BigInteger _Q(const BigInteger& qam, const BigInteger& qmb, int64 a, int64 m, int64 b, BinarySplitting::StoredMap& tree) {
	return qam * qmb;
}

BigInteger _R(const BigInteger& ram, const BigInteger& rmb, int64 a, int64 m, int64 b, BinarySplitting::StoredMap& tree) {
	return ram * rmb;
}

BigInteger _P_init(int64 b) {
	return (BigInteger(b) * 545140134ll + 13591409ll) * (2 * b - 1) * (6 * b - 5) * (6 * b - 1) * (b % 2 == 0 ? 1ll : -1ll);
}

BigInteger _Q_init(int64 b) {
	return BigInteger(10939058860032000ll) * b * b * b;
}

BigInteger _R_init(int64 b) {
	return BigInteger(2 * b - 1) * (6 * b - 5) * (6 * b - 1);
}

struct PiSplitting : BinarySplitting {
public:
	PiSplitting()
	{
		functions.push_back({ 'Q', BinarySplitting::Function(false, true, _Q_init, _Q, 'Q') });
		functions.push_back({ 'R', BinarySplitting::Function(false, true, _R_init, _R, 'R') });
		functions.push_back({ 'P', BinarySplitting::Function(false, true, _P_init, _P, 'P') });
	}

	virtual BigFloat calculateFinal(std::map<char, BigInteger>& BS_functions) override {
		BigFloat q = BS_functions['Q'];
		BigFloat r = BS_functions['R'];
		BigFloat p = BS_functions['P'];
		return (q * 4270934400ll) / (p + q * 13591409ll) * invsqrt(BigFloat(10005));
	}
};
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

	//inputs
	int threads = get<int>("How many threads to use? (Choose a power of 2)");
	index _msb = get<index>("How many 64 bit words to use in BigInteger? (Choose a power of 2)");
	index _lsb = -get<index>("How many 64 bit decimal words to use in BigInteger? (Choose a power of 2)");
	int_msb = _msb, int_lsb = 0, float_msb = _msb, float_lsb = _lsb;
	int64 max_b = get<int64>("How many iterations?");

	std::ofstream outFile(getLine("File to output calculated digits:"));
	std::ifstream inFile(getLine("File to input correct digits:"));
	std::string realE(std::istreambuf_iterator<char>{inFile}, {});


	//calculate binary sums
	PiSplitting binary_splitting;

	long long BS_time, final_time;
	BigFloat e = binary_splitting.calculate(0, max_b, BS_time, final_time, threads);
	printf("\nCalculating Binary Splitting functions took %llu ms\n", BS_time);
	printf("\nFinal calculation took %llu ms\n", final_time);

	std::string eString = e.toHexString();


	//compare with real value
	size_t digits = correctDigits(realE, eString);
	printf("Matched %lli digits past the decimal point\n", digits-2);
	if (digits < std::min(eString.size(), realE.size()))
		printf("TIP: Increase iterations to calculate more digits\n");
	else
		printf("TIP: Increase size of BigInteger to calculate more digits\n");

	//write calculated value to file
	std::cout << std::hex << eString << std::endl;
	outFile << std::hex << eString << std::endl;
}