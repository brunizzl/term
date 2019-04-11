#pragma once

#include "term.h"

namespace bmath {

class Sum :
	public Basic_Term
{
private:
	std::list<Basic_Term*> summands;
	std::list<Basic_Term*> subtrahends;
public:
	Sum();
	~Sum();
};

} //namespace bmath

