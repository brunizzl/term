#include "term.h"

using namespace bmath;

bmath::Basic_Term::Basic_Term(Basic_Term* parent_)
	:parent(parent_)
{
}

bmath::Basic_Term::Basic_Term(const Basic_Term& source)
	:parent(source.parent)
{
}

bmath::Basic_Term::~Basic_Term()
{
	//cleaning up the tree is done in derived classes
}

bmath::Term::Term(std::string name_)
	:Basic_Term(nullptr), term_ptr(build_subterm(name_, this))
{
}

bmath::Term::Term(const Term& source)
	:Basic_Term(nullptr), term_ptr(copy_subterm(source.term_ptr, this))
{
	LOG_C("kopiere Term: " << *this);
}

bmath::Term::~Term()
{
	LOG_C("loesche Term: " << *this);
	delete term_ptr;
}

void bmath::Term::to_str(std::string& str) const
{
	this->term_ptr->to_str(str);
}

State bmath::Term::get_state() const
{
	return undefined;
}

void bmath::Term::combine()
{
	this->term_ptr->combine();
}

Term& bmath::Term::operator+=(const Term& summand)
{
	Sum* sum = new Sum(this);
	this->term_ptr->parent = sum;
	sum->summands.push_back(this->term_ptr);
	sum->summands.push_back(copy_subterm(summand.term_ptr, sum));
	this->term_ptr = sum;
	this->combine();
	return *this;
}

Term& bmath::Term::operator-=(const Term& subtractor)
{
	Sum* sum = new Sum(this);
	this->term_ptr->parent = sum;
	sum->summands.push_back(this->term_ptr);
	sum->subtractors.push_back(copy_subterm(subtractor.term_ptr, sum));
	this->term_ptr = sum;
	this->combine();
	return *this;
}

Term& bmath::Term::operator*=(const Term& factor)
{
	Product* product = new Product(this);
	this->term_ptr->parent = product;
	product->factors.push_back(this->term_ptr);
	product->factors.push_back(copy_subterm(factor.term_ptr, product));
	this->term_ptr = product;
	this->combine();
	return *this;
}

Term& bmath::Term::operator/=(const Term& divisor)
{
	Product* product = new Product(this);
	this->term_ptr->parent = product;
	product->factors.push_back(this->term_ptr);
	product->divisors.push_back(copy_subterm(divisor.term_ptr, product));
	this->term_ptr = product;
	this->combine();
	return *this;
}