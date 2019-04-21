#include "term.h"

using namespace bmath;

void bmath::Basic_Term::combine_layers()
{
	//the base class does not know of the tree structures, the derived classes provide.
	//therefore, no tree can be combined.
	//the classes Term, Value and Variable dont need an override to this function.
	//every derived class not beeing root or leaf need an overriding function.
}

Vals_Combinded bmath::Basic_Term::combine_values()
{
	//root does not need an overload of this function. all other derived classes do (including leaves).
	return Vals_Combinded{ false, 0 };
}

void bmath::Basic_Term::combine_variables()
{
	//again: root and leaves need no overriding, all others do.
}

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
	LOG_C("baue Term: " << *this);
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
	return s_undefined;
}

Vals_Combinded bmath::Term::evaluate(const std::string & name_, double value_) const
{
	return this->term_ptr->evaluate(name_, value_);
}

bool bmath::Term::search_and_replace(const std::string& name_, double value_)
{
	return this->term_ptr->search_and_replace(name_, value_);
}

void bmath::Term::combine()
{
	this->term_ptr->combine_layers();

	Vals_Combinded new_val = this->term_ptr->combine_values();
	if (new_val.known) {
		delete this->term_ptr;
		this->term_ptr = new Value(new_val.val, this);
	}

	this->term_ptr->combine_variables();
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