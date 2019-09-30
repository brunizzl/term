
#pragma once

#include <vector>
#include <iostream>
#include <chrono>
#include "term.h"

void replace_constants(bmath::Term& term) {
	const std::string pi("pi");
	const std::string e("e");
	term.search_and_replace(pi, 3.14159265358979323846);
	term.search_and_replace(e, 2.71828182845904523536);
}

void baue_teststrs(const std::vector<std::string>& teststrs) {
	for (auto& str : teststrs) {
		std::cout << '\n';
		std::cout << "String: \t" << str << '\n';
		{
			bmath::Term test(str);
			std::cout << "raw tree:" << test.to_tree(16).erase(0, 9) << std::endl;
			test.combine();
			test.cut_rounding_error();
			std::cout << "combined tree:" << test.to_tree(16).erase(0, 14) << std::endl;
			std::cout << "print:  \t" << test << '\n';
		}
	}
}

void test_strings() {
	const std::vector<std::string> teststrs = {
		"(1*(2i^(-2)*3*(4*(a^5))))",
		"(5+a)/(7-b)*4-(c*d)^(-2)",
		"(3*x-2*y)/5",
		"5+pi+7/(5-a+ln(2))^3",
		"auto^herbert*3+auto^(-32*a)-4",
		"2.5",
		"((((((-2.6))))))",
		"6/7/8*a/4",
		"-4*q/s^2",
		"(((-a-b)))",
		"(log10((2)))",
		"sin(x+300*cos(pi))^a",
		"re(3+4i)*im(2-3i)",
		"-i+300*a/(a*b)",
		"a+ln(b^2)+ln(c)+2-b^2-c*a",
		"3*(sin(a+b+c)^2+cos(a+b+c)^2)+i",
		"e^(sin(x)*sin(a+b)^2+sin(x)*cos(a+b)^2)+5",
	};
	baue_teststrs(teststrs);
}

void test_function(std::string name) {
	bmath::Term function(name);
	const std::string x_string("x");
	replace_constants(function);
	for (double x = 0; x < 10; x += 0.1) {
		std::cout << x << '\t' << function.evaluate(x_string, x) << '\n';
	}
}

//vergleicht zwei funktionen auf äquivalenz
void test_vergleich() {
	
	const bmath::Term spannungsteiler("z4/(z3+z4)*z2/(z1+z2)");
	const bmath::Term Zaus_durch_Zein("(1/(1/z4+1/(z2+z3))) / (z1+1/(1/z2+1/(z3+z4)))");
	std::cout << "spannungsteiler:\n" << spannungsteiler.to_tree(5) << std::endl << std::endl;
	std::cout << "Zaus_durch_Zein:\n" << Zaus_durch_Zein.to_tree(5) << std::endl << std::endl;

	for (int i = 0; i < 20; i++) {
		std::complex<double> z1 = rand() % 1000;
		std::complex<double> z2 = rand() % 1000;
		std::complex<double> z3 = rand() % 1000;
		std::complex<double> z4 = rand() % 1000;
		std::cout << "z1: " << z1.real() << " z2: " << z2.real() << " z3: " << z3.real() << " z4: " << z4.real() << std::endl;
		double spannungsteiler_erg = spannungsteiler.evaluate({ { "z1", z1 }, { "z2", z2 }, { "z3", z3 }, { "z4", z4 } }).real();
		double Zaus_durch_Zein_erg = Zaus_durch_Zein.evaluate({ { "z1", z1 }, { "z2", z2 }, { "z3", z3 }, { "z4", z4 } }).real();

		std::cout << (spannungsteiler_erg - Zaus_durch_Zein_erg) / spannungsteiler_erg << std::endl << std::endl;
	}	
}

void test_rechner() {
	while (true) {
		std::string name;
		std::cin >> name;
		try {
			bmath::Term test(name); 
			std::cout << test.to_tree() << '\n';
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
	std::cout << "starting calculations with term..." << std::endl;
	bmath::Term t1("3+4a");
	bmath::Term t2("1-2*b");
	t1.combine();
	t2.combine();
	int repetitions = 1000000;
	auto start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < repetitions; i++) {
		bmath::Term t3(t1);
		bmath::Term t4(t2);
		t3 += t4;
		if (i % (repetitions / 10) == 0) {
			std::cout << t3 << std::endl;
		}
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
		if (i % (repetitions / 10) == 0) {
			std::cout << c3 << std::endl;
		}
	}
	end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> duration2 = end - start;
	std::cout << "took " << duration2.count() << "s for " << repetitions << " repetitions\n";
	std::cout << "\nratio durations (term / complex): " << duration1 / duration2 << '\n';
}
