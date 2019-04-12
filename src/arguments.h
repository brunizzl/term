#pragma once

#include "term.h"

namespace bmath {

class Value : public Basic_Term
{
private:
	double value;
public:
	Value(std::string name_, Basic_Term* parent_);
	~Value();
};


class Variable : public Basic_Term
{
private:
	std::string name;
public:
	Variable(std::string name_, Basic_Term* parent_);
	~Variable();
};

} //namespace bmath

