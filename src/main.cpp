
#include <iostream>
#include "test.h"
#include "term.h"

int main()
{
	//test_strings();
	//test_strings_2();
	test_length();
	//test_function("atan(x)");
	while (true) {
		std::string name;
		std::cin >> name;
		bmath::Term test(name);
		if (test.valid_state()) {
			test.combine();
			std::cout << test << '\n';
		}
		std::cin.get();
	}

}




