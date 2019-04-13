#include <iostream>
#include "term.h"

void test_strings() {
	std::string teststrs[10];
	teststrs[0] = "(1*(2^(-2)*3*(4*(a^5))))";
	teststrs[1] = "(5+a)/(7-b)*4-(c*d)^(-2)";
	teststrs[2] = "(3*x-2*y)/5";
	teststrs[3] = "5+pi+7/(5-a+ln[2])^3";
	teststrs[4] = "auto^herbert*3+auto^(-32*a)-4";
	teststrs[5] = "2.5";
	teststrs[6] = "((((((2.6))))))";
	teststrs[7] = "6/7/8*a/4";
	teststrs[8] = "-4*q/s^2";
	teststrs[9] = "(((-a-b)))";

	for (auto str : teststrs) {
		std::cout << "String: " << str << '\n';
		bmath::Term test(str);
		std::cout << "  print: " << test << '\n' << '\n';
	}
}

void test_length() {
	std::cout << "sizeof double:                " << sizeof(double) << '\n';
	std::cout << "sizeof std::list:             " << sizeof(std::list<bmath::Basic_Term>) << '\n';
	std::cout << "sizeof bmath::Basic_Term:     " << sizeof(bmath::Basic_Term) << '\n';
	std::cout << "sizeof bmath::Product:        " << sizeof(bmath::Product) << '\n';
	std::cout << "sizeof bmath::Sum:            " << sizeof(bmath::Sum) << '\n';
	std::cout << "sizeof bmath::Exponentiation: " << sizeof(bmath::Exponentiation) << '\n';
	std::cout << "sizeof bmath::Variable:       " << sizeof(bmath::Variable) << '\n';
	std::cout << "sizeof bmath::Value:          " << sizeof(bmath::Value) << '\n';
	std::cout << "sizeof bmath::Term:           " << sizeof(bmath::Term) << '\n';
	std::cout << '\n';	
}

int main()
{
	test_strings();
}




