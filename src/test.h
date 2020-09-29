
#pragma once

#include <vector>
#include <iostream>
#include <chrono>
#include "term.h"
#include <unordered_map>

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
		test.simplify();
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
		"a+ln(b^2)+ln(c)+2-b^2-c*a",
		"c*d+g*f+g",
		"12*herbert+herbert+4",
		"(3*x-2*y)/5",
		"(1*[2i^(-2)*3*(4*(a^5))])",
		"(10/5)^3",
		"5+pi+7/(5-a+ln[2])^3",
		"auto^herbert*3+auto^(-32*a)-4",
		"3*(sin(a+b+c)^2+cos(a+b+c)^2+4)+i",
		"(3^(x^2))^(x)",
		"sin(-a*b)",
		"sin(x)*b+3*b",
		"a*d+a*b*c",
		"a/(a*b)",
		"4*a+9*(a^2)",
	};
	baue_teststrs(teststrs);
}

void test_rechner() 
{
	while (true) {
		std::cout << "> ";
		std::string name;
		std::cin >> name;
		try {
			bmath::Term test(name); 
			std::cout << "roh:" << test.to_tree(7).erase(0, 4) << '\n';
			test.simplify();
			test.cut_rounding_error();
			std::cout << "erg:" << test.to_tree(7).erase(0, 4) << '\n';
			std::cout << "= " << test << '\n';
			std::cout << '\n';
		}
		catch (bmath::XTermConstructionError& err) {
			std::cout << "faulty construction string: " << err.what() << std::endl;
		}
		std::cin.get();
	}
}

void rechner()
{
	const auto cut_whitespace = [](std::string& str) {
		for (std::size_t i = 0; i < str.length(); i++) {	//deleting whitespace and counting parentheses
			switch (str[i]) {
			case '\t':
			case '\n':
			case ' ':
				str.erase(i--, 1);	//erase this char -> set i one back, as string got shorter
				break;
			}
		}
	};

	std::unordered_map<std::string, bmath::Term> variables;
	while (true) {
		std::string input;
		std::string name;
		std::cout << "> ";
		std::getline(std::cin, input);
		if (std::size_t eq = input.find('='); eq != std::string::npos) {
			name = input.substr(0, eq);
			cut_whitespace(name);
			input.erase(0, eq + 1);
		}
		try {
			bmath::Term term(input);
			for (auto& [name, val] : variables) {
				term.search_and_replace(name, val);
			}
			term.simplify();
			if (name.size()) {
				variables.insert_or_assign(name, std::move(term));
			}
			else {
				std::cout << term << std::endl;
				std::cout << "tree:" << term.to_tree(8).erase(0, 5) << '\n';
			}
		}
		catch (bmath::XTermConstructionError& err) {
			std::cout << "faulty construction string: " << err.what() << std::endl;
		}
	}
}

