#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include <array>
#include <iostream>
#include <chrono>
#include "term.h"


template <int N>
void baue_teststrs(std::array<std::string, N>& teststrs) {
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

void replace_constants(bmath::Term& term) {
	std::string pi("pi");
	std::string e("e");
	term.search_and_replace(pi, M_PI);
	term.search_and_replace(e, M_E);
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
	std::cout << "groesse von double:          " << sizeof(double) << " bytes\n";
	std::cout << "groesse von long double:     " << sizeof(long double) << " bytes\n";
	std::cout << "groesse von complex<double>: " << sizeof(std::complex<double>) << " bytes\n";
	std::cout << "groesse von list:            " << sizeof(std::list<bmath::intern::Basic_Term>) << " bytes\n";
	std::cout << "groesse von Basic_Term:      " << sizeof(bmath::intern::Basic_Term) << " bytes\n";
	std::cout << "groesse von Basic_Term*:     " << sizeof(bmath::intern::Basic_Term*) << " bytes\n";
	std::cout << "groesse von Product:         " << sizeof(bmath::intern::Product) << " bytes\n";
	std::cout << "groesse von Sum:             " << sizeof(bmath::intern::Sum) << " bytes\n";
	std::cout << "groesse von Exponentiation:  " << sizeof(bmath::intern::Exponentiation) << " bytes\n";
	std::cout << "groesse von Variable:        " << sizeof(bmath::intern::Variable) << " bytes\n";
	std::cout << "groesse von Value:           " << sizeof(bmath::intern::Value) << " bytes\n";
	std::cout << "groesse von Par_Operator:    " << sizeof(bmath::intern::Par_Operator) << " bytes\n";
	std::cout << "groesse von Term:            " << sizeof(bmath::Term) << " bytes\n";
	std::cout << '\n';
}

void test_function(std::string name) {
	bmath::Term function(name);
	std::string x_string("x");
	replace_constants(function);
	for (double x = 0; x < 10; x += 0.1) {
		std::cout << x << '\t' << function.evaluate(x_string, x) << '\n';
	}
}

void test_rechner() {
	while (true) {
		std::string name;
		std::cin >> name;
		try {
			bmath::Term test(name); 
			replace_constants(test);
			test.combine();
			test.cut_rounding_error();
			std::cout << "-> " << test << '\n';
		}
		catch (bmath::XTermConstructionError& err) {
			std::cout << "faulty construction string: " << err.what() << std::endl;
		}
		std::cin.get();
	}
}

void test_timing() {
	//std::cin.get();
	std::cout << "starting calculations with term..." << std::endl;
	bmath::Term t1("3+4a");
	bmath::Term t2("1-2*b");
	t1.combine();
	t2.combine();
	int repetitions = 100000;
	auto start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < repetitions; i++) {
		bmath::Term t3(t1);
		bmath::Term t4(t2);
		t3 += t4;
	}
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> duration1 = end - start;
	std::cout << "took " << duration1.count() << "s for " << repetitions << " repetitions\n";
	//std::cin.get();
	std::cout << "\nstarting calculations with complex..." << std::endl;
	start = std::chrono::high_resolution_clock::now();
	std::complex<double> c1(3, 4);
	std::complex<double> c2(1, -1);
	for (int i = 0; i < repetitions; i++) {
		std::complex<double> c3(c1);
		std::complex<double> c4(c2);
		c3 += c4;
	}
	end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> duration2 = end - start;
	std::cout << "took " << duration2.count() << "s for " << repetitions << " repetitions\n";
	std::cout << "\nratio durations (term / complex): " << duration1 / duration2 << '\n';
}
