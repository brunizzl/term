#include <iostream>
#include "term.h"

int main()
{
	std::string plus("(5+a)/(7-b)*4-(c*d)^(-2)");
	bruno::Term test(plus);
	std::cout << test << '\n';
	std::cin.get();
}




