#pragma once

#include "term.h"

namespace bmath {

class Variable :
	public Basic_Term
{
private:
	std::string name;
	bool negative;
public:
	Variable();
	~Variable();
};

} //namespace bmath

