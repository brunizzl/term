#pragma once

#include "term.h"

namespace bmath {

class Product :
	public Basic_Term
{
private:
	std::list<Basic_Term*> factors;
	std::list<Basic_Term*> quotients;
public:
	Product();
	~Product();
};

} //namespace bmath

