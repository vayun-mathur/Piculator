#include "PI.h"
#include "Console.h"

size_t digits = 10'000'000;
size_t threads = 8;

void calculatePi() {
	printf_color(MAGENTA, "\n\nCalculate Pi:\n\n");

	printf_color(WHITE, "1\tMulti-Threading:  ");
	printf_color(CYAN, "%d\n", threads);

	printf_color(WHITE, "2\tDecimal Digits:   ");
	printf_color(CYAN, "%llu\n\n", digits);

	printf_color(WHITE, "0\tRun Computation\n\n");

	int option = -1;
	while (option < 0 || option > 2) {
		printf_color(WHITE, "Option: ");
		std::cin >> option;
	}

	if (option == 0) {
		printf("\n\n\n\n\n\n");
		Pi(digits, threads);
	}
	else if (option == 1) {
		printf_color(WHITE, "\nSelect the # of threads to use:\nOption: ");
		std::cin >> threads;
		printf("\n\n\n\n\n\n");
		calculatePi();
	}
	else if (option == 2) {
		printf_color(WHITE, "\nSelect the # of decimal digits to compute:\nOption: ");
		std::cin >> digits;
		printf("\n\n\n\n\n\n");
		calculatePi();
	}
}

bool Prime(NNT_WORD w) {
	NNT_WORD s = ((NNT_WORD)sqrt(w) + 1);
	for (NNT_WORD i = 2; i <= s; i++) {
		if (w % i == 0) return false;
	}
	return true;
}

int main() {
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	if (!GetConsoleMode(hOut, &dwMode))
	{
		return GetLastError();
	}

	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	if (!SetConsoleMode(hOut, dwMode))
	{
		return GetLastError();
	}
	Pi(100'000'000, 8);
	/*
	printf_color(GREEN, "Piculator v0.4\n");
	printf_color(BRIGHT_BLUE, "Copyright 2020 Vayun Mathur\n\n\n");

	printf_color(BRIGHT_CYAN, "  0\t\tCalculate Pi\n");
	printf_color(BRIGHT_GREEN, "  1\t\tDigit Viewer\n\n");


	printf_color(WHITE, "Enter your choice:\n");
	int option = -1;
	while (option < 0 || option > 1) {
		printf_color(WHITE, "Option: ");
		std::cin >> option;
	}

	if (option == 0) {
		calculatePi();
	}
	else if (option == 1) {

	}

	*/
#ifdef _WIN32
	system("pause");
#endif
}