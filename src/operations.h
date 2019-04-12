#pragma once

#include "term.h"

namespace bmath {

class Product : public Basic_Term
{
private:
	std::list<Basic_Term*> factors;
	std::list<Basic_Term*> quotients;
public:
	Product(std::string name_, Basic_Term* parent_);
	~Product();
};


class Sum : public Basic_Term
{
private:
	std::list<Basic_Term*> summands;
	std::list<Basic_Term*> subtrahends;
public:
	Sum(std::string name_, Basic_Term* parent_);
	~Sum();
};


class Exponentiation : public Basic_Term
{
private:
	Basic_Term* exponent;
	Basic_Term* base;
public:
	Exponentiation(std::string name_, Basic_Term* parent_);
	~Exponentiation();
};

} //namespace bmath

