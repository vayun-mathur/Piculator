#include "PI.h"

int main() {

	size_t digits = 10'000'000;

	//e(digits);
	Pi(digits);

#ifdef _WIN32
	system("pause");
#endif
}