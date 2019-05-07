
#define _USE_MATH_DEFINES
#include <cmath>
#include <array>
#include "test.h"

template <int N>
void baue_teststrs(std::array<std::string, N> &teststrs) {
	for (auto str : teststrs) {
		std::cout << '\n';
		std::cout << "String: \t" << str << '\n';
		{
			bmath::Term test(str);
			test.combine();
			std::cout << "print:  \t" << test << '\n';
		}
	}
}

void test_strings() {
	std::array<std::string, 11> teststrs = {
	"(1*(2^(-2)*3*(4*(a^5))))",
	"(5+a)/(7-b)*4-(c*d)^(-2)",
	"(3*x-2*y)/5",
	"5+pi+7/(5-a+ln[2])^3",
	"auto^herbert*3+auto^(-32*a)-4",
	"2.5",
	"((((((-2.6))))))",
	"6/7/8*a/4",
	"-4*q/s^2",
	"(((-a-b)))",
	"(log10((2)))",
	};
	baue_teststrs(teststrs);
}

void test_strings_2() {
	std::array<std::string, 14> teststrs = {
	"(ln((2*3*4)))",
	"(log10((abs(-2))))",
	"(log10((a^b)))",
	"(exp((2)))",
	"(e^sin(2/3*pi))+3",
	"(cos((2)))",
	"(tan((2)))",
	"(asin((2)))",
	"(acos((2)))",
	"(atan((2)))",
	"(sinh((2)))",
	"(cosh((2)))",
	"(tanh((2)))",
	"(atan((2)))",
	};
	baue_teststrs(teststrs);
}

void test_length() {
	std::cout << "Groesse von double:          " << sizeof(double) << " bytes\n";
	std::cout << "Groesse von long double:     " << sizeof(long double) << " bytes\n";
	std::cout << "Groesse von complex<double>: " << sizeof(std::complex<double>) << " bytes\n";
	std::cout << "Groesse von list:            " << sizeof(std::list<bmath::intern::Basic_Term>) << " bytes\n";
	std::cout << "Groesse von Basic_Term:      " << sizeof(bmath::intern::Basic_Term) << " bytes\n";
	std::cout << "Groesse von Basic_Term*:     " << sizeof(bmath::intern::Basic_Term*) << " bytes\n";
	std::cout << "Groesse von Product:         " << sizeof(bmath::intern::Product) << " bytes\n";
	std::cout << "Groesse von Sum:             " << sizeof(bmath::intern::Sum) << " bytes\n";
	std::cout << "Groesse von Exponentiation:  " << sizeof(bmath::intern::Exponentiation) << " bytes\n";
	std::cout << "Groesse von Variable:        " << sizeof(bmath::intern::Variable) << " bytes\n";
	std::cout << "Groesse von Value:           " << sizeof(bmath::intern::Value) << " bytes\n";
	std::cout << "Groesse von Par_Operator:    " << sizeof(bmath::intern::Par_Operator) << " bytes\n";
	std::cout << "Groesse von Term:            " << sizeof(bmath::Term) << " bytes\n";
	std::cout << '\n';
}

void test_function(std::string name) {
	bmath::Term f_von_x(name);
	std::string x_string("x");
	std::string pi("pi");
	std::string e("e");
	f_von_x.search_and_replace(pi, M_PI);
	f_von_x.search_and_replace(e, M_E);
	for (double x = 0; x < 10; x+= 0.1) {
		bmath::Vals_Combinded y = f_von_x.evaluate(x_string, x);
		if (y.known) {
			std::cout << x << '\t' << y.val << '\n';
		}
		else {
			std::cout << x << '\t' << "fehler :(\n";
		}
	}
}

void test_rechner() {
	while (true) {
		std::string name;
		std::cin >> name;
		bmath::Term test(name);
		std::string pi("pi");
		std::string e("e");
		test.search_and_replace(pi, std::complex<double>(M_PI, 0));
		test.search_and_replace(e, std::complex<double>(M_E, 0));
		if (test.valid_state()) {
			test.combine();
			std::cout << "-> " << test << '\n';
		}
		std::cin.get();
	}
}