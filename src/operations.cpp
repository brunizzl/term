#include "operations.h"

using namespace bmath;


bmath::Product::Product(std::string name_, Basic_Term* parent_)
	:Basic_Term(parent_)
{
	std::cout << "baue Produkt: " << name_ << '\n';
	std::vector<Pos_Pars> pars;
	find_pars(name_, pars);
	std::size_t op = find_last_of_skip_pars(name_, "*/", pars);
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


bmath::Sum::Sum(std::string name_, Basic_Term* parent_)
	:Basic_Term(parent_)
{
	std::cout << "baue Summe: " << name_ << '\n';
	std::vector<Pos_Pars> pars;
	find_pars(name_, pars);
	std::size_t op = find_last_of_skip_pars(name_, "+-", pars);
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


bmath::Exponentiation::Exponentiation(std::string name_, Basic_Term* parent_)
	:Basic_Term(parent_)
{
	std::cout << "baue Potenz: " << name_ << '\n';
	std::vector<Pos_Pars> pars;
	find_pars(name_, pars);
	std::size_t op = find_last_of_skip_pars(name_, "^", pars);
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
