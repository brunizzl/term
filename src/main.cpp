
#include <iostream>
#include "test.h"
#include "term.h"

int main()
{
	//test_timing();
	//test_strings();
	//test_strings_2();
	//test_length();
	//test_function("atan(x)");
	test_rechner();
	int* a = new int(3);
	int* b = new int(4);
	int* c = new int(1);
	int* d = new int(2);
	int* e = new int(-5);
	std::list<int*> l1 = { a, b, c, d, e };
	l1.sort([](int*& a, int*& b) -> bool {return *a < *b; });
	for (auto it : l1) {
		std::cout << *it << ' ';
	}
}