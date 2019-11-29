
#include <iostream>
#include "term.h"
#include "test.h"

using namespace bmath::intern;

int main()
{
	//test_strings();
	Value val({ -1.0, 0.0 });
	std::cout << fullfills_restr(&val, Restriction::integer) << std::endl;
	test_rechner();
}
