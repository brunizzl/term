#include "term.h"

using namespace bmath::intern;

void bmath::intern::Basic_Term::combine_layers()
{
	//the base class does not know of the tree structures, the derived classes provide.
	//therefore, no tree can be combined.
	//the classes Value and Variable dont need an override to this function.
	//every derived class not beeing leaf needs an overriding function.
}

bool bmath::intern::Basic_Term::combine_variables()
{
	//again: leaves need no overriding, all others do.
	return false;
}

bool bmath::intern::Basic_Term::re_smaller_than_0()
{
	//only value needs overloading
	return false;
}

bmath::intern::Basic_Term::Basic_Term(Basic_Term* parent_)
	:parent(parent_)
{
}

bmath::intern::Basic_Term::Basic_Term(const Basic_Term& source)
	:parent(source.parent)
{
}

bmath::intern::Basic_Term::~Basic_Term()
{
	//cleaning up the tree is done in derived classes
}

bmath::Term::Term(std::string name_)
	:term_ptr(nullptr)
{
	if (preprocess_str(name_)) {
		this->term_ptr = build_subterm(name_, nullptr);
	}
	LOG_C("baue Term: " << *this);
}

bmath::Term::Term(const Term& source)
	:term_ptr(copy_subterm(source.term_ptr, nullptr))
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

std::complex<double> bmath::Term::evaluate(const std::string & name_, std::complex<double> value_) const
{
	return this->term_ptr->evaluate(name_, value_).val;
}

void bmath::Term::search_and_replace(const std::string& name_, std::complex<double> value_)
{
	if (this->term_ptr->search_and_replace(name_, value_)) {
		delete this->term_ptr;
		this->term_ptr = new Value(value_, nullptr);
	}
	return;
}

bool bmath::Term::valid_state() const
{
	if (this->term_ptr == nullptr) {
		return false;
	}
	return this->term_ptr->valid_state();
}

void bmath::Term::combine()
{
	this->term_ptr->combine_layers();
	Vals_Combined new_subterm = this->term_ptr->combine_values();
	if (new_subterm.known) {
		delete this->term_ptr;
		this->term_ptr = new Value(new_subterm.val, nullptr);
	}
	this->term_ptr->combine_layers();
	while(this->term_ptr->combine_variables());
}

void bmath::Term::cut_rounding_error(int pow_of_10_diff_to_set_0)
{
	std::list<Value*> values;
	this->term_ptr->list_values(values);
	double re_sum = 0;
	for (auto it : values) {
		re_sum += std::abs(it->value.real());
	}
	if (re_sum != 0) {
		double re_average = re_sum / values.size();
		double limit_to_0 = re_average * std::pow(10, -pow_of_10_diff_to_set_0);
		for (auto it : values) {
			if (std::abs(it->value.real()) < limit_to_0) {
				it->value.real(0);
			}
			if (std::abs(it->value.imag()) < limit_to_0) {
				it->value.imag(0);
			}
		}
	}
}

bmath::Term& bmath::Term::operator+=(const Term& summand)
{
	Sum* sum = new Sum(nullptr);
	this->term_ptr->parent = sum;
	sum->summands.push_back(this->term_ptr);
	sum->summands.push_back(copy_subterm(summand.term_ptr, sum));
	this->term_ptr = sum;
	this->combine();
	return *this;
}

bmath::Term& bmath::Term::operator-=(const Term& subtractor)
{
	Sum* sum = new Sum(nullptr);
	this->term_ptr->parent = sum;
	sum->summands.push_back(this->term_ptr);
	sum->subtractors.push_back(copy_subterm(subtractor.term_ptr, sum));
	this->term_ptr = sum;
	this->combine();
	return *this;
}

bmath::Term& bmath::Term::operator*=(const Term& factor)
{
	Product* product = new Product(nullptr);
	this->term_ptr->parent = product;
	product->factors.push_back(this->term_ptr);
	product->factors.push_back(copy_subterm(factor.term_ptr, product));
	this->term_ptr = product;
	this->combine();
	return *this;
}

bmath::Term& bmath::Term::operator/=(const Term& divisor)
{
	Product* product = new Product(nullptr);
	this->term_ptr->parent = product;
	product->factors.push_back(this->term_ptr);
	product->divisors.push_back(copy_subterm(divisor.term_ptr, product));
	this->term_ptr = product;
	this->combine();
	return *this;
}