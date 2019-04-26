
#define _USE_MATH_DEFINES
#include <cmath>
#include "test.h"

void test_strings() {
	std::string teststrs[11];
	teststrs[0] = "(1*(2^(-2)*3*(4*(a^5))))";
	teststrs[1] = "(5+a)/(7-b)*4-(c*d)^(-2)";
	teststrs[2] = "(3*x-2*y)/5";
	teststrs[3] = "5+pi+7/(5-a+ln[2])^3";
	teststrs[4] = "auto^herbert*3+auto^(-32*a)-4";
	teststrs[5] = "2.5";
	teststrs[6] = "((((((-2.6))))))";
	teststrs[7] = "6/7/8*a/4";
	teststrs[8] = "-4*q/s^2";
	teststrs[9] = "(((-a-b)))";
	teststrs[10] = "(log10((2)))";

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

void test_strings_2() {
	std::string teststrs[14];
	teststrs[0] = "(ln((2*3*4)))";
	teststrs[1] = "(log10((abs(-2))))";
	teststrs[2] = "(log10((a^b)))";
	teststrs[3] = "(exp((2)))";
	teststrs[4] = "(e^sin(2/3*pi))+3";
	teststrs[5] = "(cos((2)))";
	teststrs[6] = "(tan((2)))";
	teststrs[7] = "(asin((2)))";
	teststrs[8] = "(acos((2)))";
	teststrs[9] = "(atan((2)))";
	teststrs[10] = "(sinh((2)))";
	teststrs[11] = "(cosh((2)))";
	teststrs[12] = "(tanh((2)))";
	teststrs[13] = "(atan((2)))";

	for (auto str : teststrs) {
		std::cout << '\n';
		std::cout << "String: " << str << '\n';
		{
			bmath::Term test(str);
			test.combine();
			std::cout << " print: " << test << '\n';
		}
	}
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