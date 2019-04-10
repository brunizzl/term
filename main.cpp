#include <iostream>
#include "term.h"

int main()
{
	std::string name("(1*(2^(-2)*3*(4*(a^5))))");
	//std::string name("(5+a)/(7-b)*4-(c*d)^(-2)");
	std::cout << "starting from: " << name << '\n';
	bmath::Term test(name);
	std::cout << "resulting Term: " << test << '\n';
	test.simplify();
	std::cout << "simplified Term: " << test << '\n';

	std::cout << "sizeof term: " << sizeof(bmath::Term) << '\n';
	std::cout << "sizeof list: " << sizeof(std::list<bmath::Term>) << '\n';
}




