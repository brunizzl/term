
#pragma once

#include <vector>
#include <iostream>
#include <chrono>
#include "term.h"

#include "internal_functions.h"
#include "operations.h"
#include "arguments.h"
#include "pattern.h"

void baue_teststrs(const std::vector<std::string>& teststrs) 
{
	for (auto& str : teststrs) {
		std::cout << "String: \t" << str << '\n';
		bmath::Term test(str);

		auto vars = test.get_var_names();
		std::cout << "variables: ";
		for (auto& name : vars) {
			std::cout << name << ' ';
		}
		std::cout << '\n';

		std::cout << '\n';
		std::cout << "raw print:  \t" << test << '\n';
		std::cout << "raw tree:" << test.to_tree(16).erase(0, 9) << '\n';
		std::cout << '\n';
		test.combine();
		test.cut_rounding_error();
		std::cout << "combined print:\t" << test << '\n';
		std::cout << "combined tree:" << test.to_tree(16).erase(0, 14) << '\n';
		std::cout << '\n';
		std::cout << "__________________________________________________________________________________________________________________\n\n";
	}
}

void test_strings() 
{
	const std::vector<std::string> teststrs = {
		"(3*x-2*y)/5",
		"(1*[2i^(-2)*3*(4*(a^5))])",
		"-i+300*a/(a*b)",
		"1-2*3",
		"(10/5)^3",
		"5+pi+7/(5-a+ln[2])^3",
		"auto^herbert*3+auto^(-32*a)-4",
		"6/7/8*a/4",
		"-4*q/s^2",
		"re(3+4i)*im(2-3i)",
		"a+ln(b^2)+ln(c)+2-b^2-c*a",
		"3*(sin(a+b+c)^2+cos(a+b+c)^2+4)+i",
		"(3^(x^2))^(x)",
		"sin(-a*b)",
		"atanh(3+sqrt(-2i))+1*5-12*cos(2i-3)",
		"sin(x)*b+3*b",
		"sin(x)*3^a+b*3^a",
		"a*d+a*b*c",
	};
	baue_teststrs(teststrs);
}

void print_all_patterns()
{
	for (const auto& pattern : bmath::intern::Transformation::transformations) {
		std::cout << pattern->print() << '\n';
		std::cout << '\n';
		std::cout << "input :" << ptr_to_tree(pattern->input.term_ptr, 12).erase(0, 10) << '\n';
		std::cout << '\n';
		std::cout << "output  :" << ptr_to_tree(pattern->output.term_ptr, 12).erase(0, 10) << '\n';
		std::cout << "__________________________________________________________\n";
		std::cout << '\n';
	}
}

void test_rechner() 
{
	while (true) {
		std::string name;
		std::cin >> name;
		try {
			bmath::Term test(name); 
			std::cout << "baum:" << test.to_tree(7).erase(0, 5) << '\n';
			test.combine();
			test.cut_rounding_error();
			std::cout << "-> " << test << "\n\n";
			//print_all_patterns();
			std::cout << '\n';
		}
		catch (bmath::XTermConstructionError& err) {
			std::cout << "faulty construction string: " << err.what() << std::endl;
		}
		std::cin.get();
	}
}

