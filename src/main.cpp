
#include <iostream>
#include <cstdlib>
#include <ctime>
#include "term.h"
#include "test.h"

using namespace bmath::intern;

int main()
{
	//test_strings();
	//test_rechner();
	bmath::Term delta("exp(x)-e^x");
	std::string name("x");
	std::srand(std::time(nullptr));
	int fehler = 0;
	double biggest = 0;
	for (int i = 0; i < 1000; i++) {
		std::complex<double> x = { std::rand() / 100.0, std::rand() / 100.0 };
		double error = abs(delta.evaluate(name, x));
		if (error != 0.0) {
			std::cout << "delta(" << x << ") = " << error << "\trelative: " << error/abs(x) << '\n';
			fehler++;
			if (error > biggest) {
				biggest = error;
			}
		}
	}
	std::cout << "fehlerquote: " << fehler / 100.0 << std::endl;
	std::cout << "groester:    " << biggest << std::endl;
}
