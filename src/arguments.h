#pragma once

#include "term.h"

namespace bmath {

class Value : public Basic_Term
{
private:
	double value;
public:
	Value();
	~Value();
};


class Variable : public Basic_Term
{
private:
	std::string name;
	bool negative;
public:
	Variable();
	~Variable();
};

} //namespace bmath

