#pragma once

#include "term.h"

namespace bmath {

class Product : public Basic_Term
{
private:
	std::list<Basic_Term*> factors;
	std::list<Basic_Term*> quotients;
public:
	Product(std::string name_, Basic_Term* parent_, std::size_t op);
	~Product();

	void to_str(std::string& str) const override;
	State get_state() const override;
	void sort() override;
	bool operator<(const Basic_Term& other) const override;
};


class Sum : public Basic_Term
{
private:
	std::list<Basic_Term*> summands;
	std::list<Basic_Term*> subtrahends;
public:
	Sum(std::string name_, Basic_Term* parent_, std::size_t op);
	~Sum();

	void to_str(std::string& str) const override;
	State get_state() const override;
	void sort() override;
	bool operator<(const Basic_Term& other) const override;
};


class Exponentiation : public Basic_Term
{
private:
	Basic_Term* exponent;
	Basic_Term* base;
public:
	Exponentiation(std::string name_, Basic_Term* parent_, std::size_t op);
	~Exponentiation();

	void to_str(std::string& str) const override;
	State get_state() const override;
	void sort() override;
	bool operator<(const Basic_Term& other) const override;
};

} //namespace bmath

