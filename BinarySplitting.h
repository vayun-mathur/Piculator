#pragma once
#include <map>
#include <list>
#include <thread>
#include <vector>
#include "BigInteger.h"

struct BinarySplitting {
public:
	using StoredMap = std::map<char, std::map<std::pair<int64, int64>, BigInteger>>;
	struct Function {
		bool storeLeft, storeRight;
		BigInteger(* const init)(int64);
		BigInteger(* const combine)(const BigInteger&, const BigInteger&, int64, int64, int64, StoredMap&);
		char c;
		Function(bool storeLeft, bool storeRight, BigInteger(* const init)(int64),
			BigInteger(* const combine)(const BigInteger&, const BigInteger&, int64, int64, int64, StoredMap&), char c)
			: storeLeft(storeLeft), storeRight(storeRight), init(init), combine(combine), c(c) {

		}
	};

	BigFloat calculate(int64 a, int64 b, long long& BS_time, long long& final_time, int threads) {
		auto start = std::chrono::high_resolution_clock::now();
		std::map<char, BigInteger> BS_functions = calculateBS(a, b, threads);
		BS_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
		start = std::chrono::high_resolution_clock::now();
		BigFloat _final = calculateFinal(BS_functions);
		final_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
		return _final;
	}

	virtual BigFloat calculateFinal(std::map<char, BigInteger>& BS_functions) = 0;

public:
	BigInteger MultiThreaded(int64 a, int64 b, Function func, int threads) {
		return MultiThreaded(a, b, func.storeLeft, func.storeRight, func.init, func.combine, func.c, threads);
	}
private:

	std::map<char, BigInteger> calculateBS(int64 a, int64 b, int threads) {
		std::map<char, BigInteger> map;
		for (auto&& [name, func] : functions) {
			map.insert({ name, MultiThreaded(a, b, func, threads) });
		}
		return map;
	}

private:
	int64 cnt;
	int64 calcs;
	BigInteger Calculate(int64 a, int64 b, bool storeThis, bool storeLeft, bool storeRight, char name, BigInteger(* const Initialize)(int64), BigInteger(* const combine)(const BigInteger&, const BigInteger&, int64, int64, int64, StoredMap&)) {
		if (a == b - 1) {
			BigInteger i = Initialize(b);
			if (storeThis) tree[name].insert({ {b - 1, b}, i });
			return i;
		}
		int64 m = (a + b) / 2;
		BigInteger r = combine(Calculate(a, m, storeLeft, storeLeft, storeRight, name, Initialize, combine), Calculate(m, b, storeRight, storeLeft, storeRight, name, Initialize, combine), a, m, b, tree);
		if (storeThis) tree[name].insert({ {a, b}, r });
		cnt++;
		printf("Finished %lli of %lli of %c calculations\r", cnt, calcs, name);
		return r;
	}
	BigInteger MultiThreaded(int64 a, int64 b, bool storeLeft, bool storeRight, BigInteger(* const Initialize)(int64), BigInteger(* const combine)(const BigInteger&, const BigInteger&, int64, int64, int64, StoredMap&), char name, int num_threads) {
		cnt = 0;
		calcs = b - num_threads;
		tree.insert({ name, std::map<std::pair<int64, int64>, BigInteger>() });
		int thread_depth = (int)log2(num_threads);

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

		auto thread = [this, storeLeft, storeRight, name, &Initialize, &combine](int64 a, int64 b, bool stack, BigInteger& res) {
			res = Calculate(a, b, stack, storeLeft, storeRight, name, Initialize, combine);
		};
		int i = 0;
		for (auto&& [_a, _b] : divs) {
			threads[i] = std::thread(thread, _a, _b, (storeLeft && (i & 1)) || (storeRight && !(i & 1)), std::ref(r[i]));
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
				BigInteger am = it_prev->second;
				BigInteger mb = it->second;
				int _a = it_prev->first.first;
				int _b = it->first.second;
				int _m = it->first.first;
				ress.erase(it_prev);
				BigInteger res = combine(am, mb, _a, _m, _b, tree);
				it->second = res;
				it->first = { _a, _b };

				if ((storeRight && (i & 1)) || (storeRight && !(i & 1))) tree[name].insert({ {_a, _b}, res });
				cnt++;
			}
		}
		printf("\n");

		return ress.front().second;
	}
protected:
	std::vector<std::pair<char, Function>> functions;
private:
	StoredMap tree;
};