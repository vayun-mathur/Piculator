#pragma once
#include <map>
#include <list>
#include <thread>
#include "BigInteger.h"

struct BinarySplitting {
public:
	using StoredMap = std::map<char, std::map<std::pair<int, int>, BigInteger>>;
	BinarySplitting(int max_b, int threads)
		: max_b(max_b), threads(threads)
	{

	}
	BigInteger Calculate(int a, int b, bool storeThis, bool storeLeft, bool storeRight, char name, BigInteger(* const Initialize)(int), BigInteger(* const combine)(const BigInteger&, const BigInteger&, int, int, int, StoredMap&)) {
		static int cnt = 0;
		if (a == b - 1) {
			BigInteger i = Initialize(b);
			if (storeThis) tree[name].insert({ {b - 1, b}, i });
			return i;
		}
		int m = (a + b) / 2;
		BigInteger r = combine(Calculate(a, m, storeLeft, storeLeft, storeRight, name, Initialize, combine), Calculate(m, b, storeRight, storeLeft, storeRight, name, Initialize, combine), a, m, b, tree);
		if (storeThis) tree[name].insert({ {a, b}, r });
		cnt++;
		printf("Finished %i of %i of %c calculations\r", cnt, max_b - threads, name);
		return r;
	}

	BigInteger MultiThreaded(int a, int b, bool storeLeft, bool storeRight, BigInteger(* const Initialize)(int), BigInteger(* const combine)(const BigInteger&, const BigInteger&, int, int, int, StoredMap&), char name) {
		tree.insert({ name, std::map<std::pair<int, int>, BigInteger>() });
		int thread_depth = (int)log2(threads);

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

		auto thread = [this, storeLeft, storeRight, name, &Initialize, &combine](int a, int b, bool stack, BigInteger& res) {
			res = Calculate(a, b, stack, storeLeft, storeRight, name, Initialize, combine);
		};
		int i = 0;
		for (auto&& [_a, _b] : divs) {
			threads[i] = std::thread(thread, _a, _b, (storeRight && (i & 1)) || (storeRight && !(i & 1)), std::ref(r[i]));
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
	StoredMap tree;
private:
	int max_b;
	int threads;
};