#pragma once

#include "term.h"

namespace bmath {

class Product : public Basic_Term
{
private:
	std::list<Basic_Term*> factors;
	std::list<Basic_Term*> divisors;
public:
	Product(Basic_Term* parent_);
	Product(std::string name_, Basic_Term* parent_, std::size_t op);
	Product(const Product& source, Basic_Term* parent_ = nullptr);
	~Product();

	void to_str(std::string& str) const override;
	State get_state() const override;
	void combine() override;

	friend class Term;
};


class Sum : public Basic_Term
{
private:
	std::list<Basic_Term*> summands;
	std::list<Basic_Term*> subtractors;
public:
	Sum(Basic_Term* parent_);
	Sum(std::string name_, Basic_Term* parent_, std::size_t op);
	Sum(const Sum& source, Basic_Term* parent_ = nullptr);
	~Sum();

	void to_str(std::string& str) const override;
	State get_state() const override;
	void combine() override;

	friend class Term;
};


class Exponentiation : public Basic_Term
{
private:
	Basic_Term* exponent;
	Basic_Term* base;
public:
	Exponentiation(Basic_Term* parent_);
	Exponentiation(std::string name_, Basic_Term* parent_, std::size_t op);
	Exponentiation(const Exponentiation& source, Basic_Term* parent_ = nullptr);
	~Exponentiation();

	void to_str(std::string& str) const override;
	State get_state() const override;
	void combine() override;

	friend class Term;
};

} //namespace bmath

