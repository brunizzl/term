#include "operations.h"

using namespace bmath;


bmath::Product::Product(std::string name_, Basic_Term* parent_, std::size_t op)
	:Basic_Term(parent_)
{
	std::cout << "baue Produkt: " << name_ << '\n';
	std::vector<Pos_Pars> pars;
	find_pars(name_, pars);
	std::string subterm;
	while (op != std::string::npos) {
		subterm = name_.substr(op + 1);
		switch (name_[op]) {
		case '*':
			this->factors.push_back(build_subterm(subterm, this));
			break;
		case '/':
			this->quotients.push_back(build_subterm(subterm, this));
			break;
		}
		name_.erase(op);
		op = find_last_of_skip_pars(name_, "*/", pars);
	}
	this->factors.push_back(build_subterm(name_, this));
}


bmath::Product::~Product()
{
	for (auto it : factors) {
		delete it;
	}
	for (auto it : quotients) {
		delete it;
	}
}

void bmath::Product::to_str(std::string& str) const
{
	if (this->parent->get_state() > this->get_state()) {
		str.push_back('(');
	}
	for (auto it : this->factors) {
		it->to_str(str);
		str.push_back('*');
	}
	str.pop_back();
	for (auto it : this->quotients) {
		str.push_back('/');
		it->to_str(str);
	}
	if (this->parent->get_state() > this->get_state()) {
		str.push_back(')');
	}
}

State bmath::Product::get_state() const
{
	return product;
}


bmath::Sum::Sum(std::string name_, Basic_Term* parent_, std::size_t op)
	:Basic_Term(parent_)
{
	std::cout << "baue Summe: " << name_ << '\n';
	std::vector<Pos_Pars> pars;
	find_pars(name_, pars);
	std::string subterm;
	while (op != std::string::npos) {
		subterm = name_.substr(op + 1);
		switch (name_[op]) {
		case '+':
			this->summands.push_back(build_subterm(subterm, this));
			break;
		case '-':
			this->subtrahends.push_back(build_subterm(subterm, this));
			break;
		}
		name_.erase(op);
		op = find_last_of_skip_pars(name_, "+-", pars);
	}
	if (name_.size() != 0) {
		this->summands.push_back(build_subterm(name_, this));
	}
}


bmath::Sum::~Sum()
{
	for (auto it : summands) {
		delete it;
	}
	for (auto it : subtrahends) {
		delete it;
	}
}

void bmath::Sum::to_str(std::string& str) const
{
	if (this->parent->get_state() > this->get_state()) {
		str.push_back('(');
	}
	for (auto it : this->summands) {
		it->to_str(str);
		str.push_back('+');
	}
	if (this->summands.size()) {
		str.pop_back();
	}
	for (auto it : this->subtrahends) {
		str.push_back('-');
		it->to_str(str);
	}
	if (this->parent->get_state() > this->get_state()) {
		str.push_back(')');
	}
}

State bmath::Sum::get_state() const
{
	return sum;
}


bmath::Exponentiation::Exponentiation(std::string name_, Basic_Term* parent_, std::size_t op)
	:Basic_Term(parent_)
{
	std::cout << "baue Potenz: " << name_ << '\n';
	std::string subterm;
	subterm = name_.substr(op + 1);
	this->exponent = build_subterm(subterm, this);
	name_.erase(op);
	this->base = build_subterm(name_, this);
}


bmath::Exponentiation::~Exponentiation()
{
	delete exponent;
	delete base;
}

void bmath::Exponentiation::to_str(std::string& str) const
{
	if (this->parent->get_state() > this->get_state()) {
		str.push_back('(');
	}
	this->base->to_str(str);
	str.push_back('^');
	this->exponent->to_str(str);
	if (this->parent->get_state() > this->get_state()) {
		str.push_back(')');
	}
}

State bmath::Exponentiation::get_state() const
{
	return exponentiation;
}
