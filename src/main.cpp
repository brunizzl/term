#include <iostream>
#include "term.h"

int main()
{
	std::string name("(1*(2^(-2)*3*(4*(a^5))))");
	//std::string name("(5+a)/(7-b)*4-(c*d)^(-2)");

	std::cout << "sizeof double: " << sizeof(double) << '\n';
	std::cout << "sizeof list: " << sizeof(std::list<bmath::Basic_Term>) << '\n';
	std::cout << "sizeof bmath::Basic_Term: " << sizeof(bmath::Basic_Term) << '\n';
	std::cout << "sizeof bmath::Product: " << sizeof(bmath::Product) << '\n';
	std::cout << "sizeof bmath::Term: " << sizeof(bmath::Term) << '\n';
}




