#pragma once

#include "term.h"

namespace bmath {

class Value :
	public Basic_Term
{
private:
	double value;
public:
	Value();
	~Value();
};

} //namespace bmath

