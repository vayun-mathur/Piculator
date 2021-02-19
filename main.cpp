#include "BigInteger.h"
#include <list>
#include <thread>
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>

BigInteger r, total;

std::map<std::pair<int, int>, BigInteger> q_tree;

int max_b = 10'00;
int threads = 8;

BigInteger Q(int a, int b, bool stack) {
	static int cnt = 0;
	if (a == b - 1) {
		BigInteger i = b;
		if (stack) q_tree.insert({ {b - 1, b}, i });
		return i;
	}
	int m = (a + b) / 2;
	BigInteger r1 = Q(a, m, false);
	BigInteger r2 = Q(m, b, true);
	BigInteger r = r1 * r2;
	//std::cout << std::hex << r1 << " * " << r2 << " = " << r << std::endl;
	if (stack) q_tree.insert({ {a, b}, r });
	cnt++;
	printf("Finished %i of %i of Q calculations\r", cnt, max_b - threads);
	return r;
}

BigInteger QMultiThreaded(int a, int b, bool stack, int thread_depth) {
	std::list<std::pair<int, int>> divs;
	divs.push_back({ a, b });
	for (int i = 0; i < thread_depth; i++) {
		for (auto it = divs.begin(); it != divs.end(); ++it) {
			int m = (it->first + it->second) / 2;
			divs.insert(it, { it->first, m });
			it->first = m;
		}
	}
	BigInteger* r = new BigInteger[divs.size()];
	std::thread* threads = new std::thread[divs.size()];

	auto thread = [](int a, int b, bool stack, BigInteger& res) {
		res = Q(a, b, stack);
	};
	int i = 0;
	for (auto&& [_a, _b] : divs) {
		threads[i] = std::thread(thread, _a, _b, i & 1, std::ref(r[i]));
		i++;
	}
	for (int i = 0; i < divs.size(); i++) threads[i].join();
	std::list<std::pair<std::pair<int, int>, BigInteger>> ress;
	i = 0;
	for (std::pair<int, int> p : divs) {
		ress.push_back({ p, r[i] });
		i++;
	}
	for (int i = 0; i < thread_depth; i++) {
		int cnt = 0;
		for (auto it = ress.begin(); it != ress.end(); ++it) {
			auto it_prev = it++;
			BigInteger l = it_prev->second;
			BigInteger r = it->second;
			int _a = it_prev->first.first;
			int _b = it->first.second;
			ress.erase(it_prev);
			BigInteger res = l * r;
			it->second = res;
			it->first = { _a, _b };

			if ((cnt & 1) == 1) q_tree.insert({ {_a, _b}, res });
			cnt++;
		}
	}
	printf("\n");

	return ress.front().second;
}

BigInteger P(int a, int b) {
	static int cnt = 0;
	if (a == b - 1) {
		BigInteger i = 1;
		return i;
	}
	int m = (a + b) / 2;

	BigInteger r = P(a, m) * q_tree[{m, b}] + P(m, b);
	cnt++;
	printf("Finished %i of %i of P calculations\r", cnt, max_b - threads);
	return r;
}

BigInteger PMultiThreaded(int a, int b, int thread_depth) {
	std::list<std::pair<int, int>> divs;
	divs.push_back({ a, b });
	for (int i = 0; i < thread_depth; i++) {
		for (auto it = divs.begin(); it != divs.end(); ++it) {
			int m = (it->first + it->second) / 2;
			divs.insert(it, { it->first, m });
			it->first = m;
		}
	}
	BigInteger* r = new BigInteger[divs.size()];
	std::thread* threads = new std::thread[divs.size()];

	auto thread = [](int a, int b, BigInteger& res) {
		res = P(a, b);
	};
	int i = 0;
	for (auto&& [_a, _b] : divs) {
		threads[i] = std::thread(thread, _a, _b, std::ref(r[i]));
		i++;
	}
	for (int i = 0; i < divs.size(); i++) threads[i].join();
	std::list<std::pair<std::pair<int, int>, BigInteger>> ress;
	i = 0;
	for (std::pair<int, int> p : divs) {
		ress.push_back({ p, r[i] });
		i++;
	}
	for (int i = 0; i < thread_depth; i++) {
		for (auto it = ress.begin(); it != ress.end(); ++it) {
			auto it_prev = it++;
			BigInteger l = it_prev->second;
			BigInteger r = it->second;
			BigInteger qr = q_tree[{it->first.first, it->first.second}];
			int _a = it_prev->first.first;
			int _b = it->first.second;
			ress.erase(it_prev);
			BigInteger res = l * qr + r;
			it->second = res;
			it->first = { _a, _b };
		}
	}
	printf("\n");

	return ress.front().second;
}

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

uint64_t numBits(word w) {
	uint64_t res = 0;
	while (w != 0) {
		w >>= 1;
		res++;
	}
	return res + 1;
}

int main() {
	float_msb = 4;
	float_lsb = -4;
	sqrt(BigFloat(2));
	/*
	std::ifstream hexEFile("C:\\Users\\Vayun Mathur\\OneDrive\\Documents\\code\\c++\\Math\\Hex Digits.txt");
	std::stringstream buffer;
	buffer << hexEFile.rdbuf();

	threads = get<int>("How many threads to use? (Choose a power of 2)");
	index _msb = get<index>("How many 64 bit words to use in BigInteger? (Choose a power of 2)");
	index _lsb = -get<index>("How many 64 bit decimal words to use in BigInteger? (Choose a power of 2)");
	int_msb = _msb;
	int_lsb = 0;
	float_msb = _msb;
	float_lsb = _lsb;
	max_b = get<int>("How many iterations?");
	std::ofstream outFile(getLine("File to output calculated digits:"));

	timer();
	BigInteger q = QMultiThreaded(0, max_b, true, (int)log2(threads));
	long long lq = timer();
	BigInteger p = PMultiThreaded(0, max_b, (int)log2(threads));
	long long lp = timer();
	printf("\n");
	printf("Calculating Q and P of e took %llu ms\n", lp + lq);

	BigFloat pf = p;
	BigFloat qf = q;
	BigFloat e = pf / qf;
	e += BigFloat(1);
	long long le = timer();
	printf("Final division to calculate e took %llu ms\n", le);
	printf("\n");

	std::stringstream eStringStream;
	eStringStream << std::hex << e;
	std::cout << std::hex << e << std::endl;
	std::string eString = eStringStream.str();
	std::string realE = buffer.str();
	for (int i = 0; i < std::min(eString.size(), realE.size()); i++) {
		if (eString[i] != realE[i]) {
			std::cout << "Matched " << std::dec << i - 2 << " digits past the decimal point" << std::endl;
			outFile << std::hex << eString.substr(0, i) << std::endl;
			std::cout << std::hex << eString.substr(0, i) << std::endl;

			std::cout << "TIP: Increase iterations to calculate more digits" << std::endl;
			goto counted;
		}
	}
	std::cout << "Matched " << std::dec << std::min(eString.size(), realE.size()) - 2 << " digits past the decimal point" << std::endl;
	outFile << std::hex << eString << std::endl;
	std::cout << std::hex << eString << std::endl;
	std::cout << "TIP: Increase size of BigInteger to calculate more digits" << std::endl;
counted:
	return 0;
	*/
}