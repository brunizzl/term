#include "operations.h"

using namespace bmath;


bmath::Product::Product(Basic_Term* parent_)
	:Basic_Term(parent_)
{
}

bmath::Product::Product(std::string name_, Basic_Term* parent_, std::size_t op)
	:Basic_Term(parent_)
{
	LOG_C("baue Produkt: " << name_);
	std::vector<Pos_Pars> pars;
	find_pars(name_, pars);
	std::string subterm;
	while (op != std::string::npos) {
		subterm = name_.substr(op + 1);
		switch (name_[op]) {
		case '*':
			this->factors.push_front(build_subterm(subterm, this));
			break;
		case '/':
			this->divisors.push_front(build_subterm(subterm, this));
			break;
		}
		name_.erase(op);
		op = find_last_of_skip_pars(name_, "*/", pars);
	}
	this->factors.push_front(build_subterm(name_, this));
}

bmath::Product::Product(const Product& source, Basic_Term* parent_)
	:Basic_Term(parent_)
{
	LOG_C("kopiere Produkt: " << source);
	for (auto it : source.factors) {
		this->factors.push_back(copy_subterm(it, this));
	}
	for (auto it : source.divisors) {
		this->divisors.push_back(copy_subterm(it, this));
	}
}


bmath::Product::~Product()
{
	LOG_C("loesche Produkt: " << *this);
	for (auto it : factors) {
		delete it;
	}
	for (auto it : divisors) {
		delete it;
	}
}

void bmath::Product::to_str(std::string& str) const
{
	if (this->parent->get_state() >= this->get_state()) {
		str.push_back('(');
	}
	for (auto it : this->factors) {
		it->to_str(str);
		str.push_back('*');
	}
	str.pop_back();
	for (auto it : this->divisors) {
		str.push_back('/');
		it->to_str(str);
	}
	if (this->parent->get_state() >= this->get_state()) {
		str.push_back(')');
	}
}

State bmath::Product::get_state() const
{
	return product;
}

void bmath::Product::combine()
{
	for (std::list<Basic_Term*>::iterator it = this->factors.begin(); it != this->factors.end();) {
		(*it)->combine();
		if ((*it)->get_state() == product) {
			Product* redundant = static_cast<Product*>((*it));
			for (auto it_red : redundant->factors) {
				it_red->parent = this;
				this->factors.push_back(it_red);
			}
			redundant->factors.clear();
			for (auto it_red : redundant->divisors) {
				it_red->parent = this;
				this->divisors.push_back(it_red);
			}
			redundant->divisors.clear();
			delete redundant;
			std::list<Basic_Term*>::iterator it_2 = it;
			++it;
			this->factors.erase(it_2);
		}
		else {
			++it;
		}
	}
	for (std::list<Basic_Term*>::iterator it = this->divisors.begin(); it != this->divisors.end();) {
		(*it)->combine();
		if ((*it)->get_state() == product) {
			Product* redundant = static_cast<Product*>((*it));
			for (auto it_red : redundant->factors) {
				it_red->parent = this;
				this->divisors.push_back(it_red);
			}
			redundant->factors.clear();
			for (auto it_red : redundant->divisors) {
				it_red->parent = this;
				this->factors.push_back(it_red);
			}
			redundant->divisors.clear();
			delete redundant;
			std::list<Basic_Term*>::iterator it_2 = it;
			++it;
			this->divisors.erase(it_2);
		}
		else {
			++it;
		}
	}
}

bmath::Sum::Sum(Basic_Term* parent_)
	:Basic_Term(parent_)
{
}

bmath::Sum::Sum(std::string name_, Basic_Term* parent_, std::size_t op)
	:Basic_Term(parent_)
{
	LOG_C("baue Summe: " << name_);
	std::vector<Pos_Pars> pars;
	find_pars(name_, pars);
	std::string subterm;
	while (op != std::string::npos) {
		subterm = name_.substr(op + 1);
		switch (name_[op]) {
		case '+':
			this->summands.push_front(build_subterm(subterm, this));
			break;
		case '-':
			this->subtractors.push_front(build_subterm(subterm, this));
			break;
		}
		name_.erase(op);
		op = find_last_of_skip_pars(name_, "+-", pars);
	}
	if (name_.size() != 0) {
		this->summands.push_front(build_subterm(name_, this));
	}
}

bmath::Sum::Sum(const Sum& source, Basic_Term* parent_)
	:Basic_Term(parent_)
{
	LOG_C("kopiere Summe: " << source);
	for (auto it : source.summands) {
		this->summands.push_back(copy_subterm(it, this));
	}
	for (auto it : source.subtractors) {
		this->subtractors.push_back(copy_subterm(it, this));
	}
}

bmath::Sum::~Sum()
{
	LOG_C("loesche Summe: " << *this);
	for (auto it : summands) {
		delete it;
	}
	for (auto it : subtractors) {
		delete it;
	}
}

void bmath::Sum::to_str(std::string& str) const
{
	if (this->parent->get_state() >= this->get_state()) {
		str.push_back('(');
	}
	for (auto it : this->summands) {
		it->to_str(str);
		str.push_back('+');
	}
	if (this->summands.size()) {
		str.pop_back();
	}
	for (auto it : this->subtractors) {
		str.push_back('-');
		it->to_str(str);
	}
	if (this->parent->get_state() >= this->get_state()) {
		str.push_back(')');
	}
}

State bmath::Sum::get_state() const
{
	return sum;
}

void bmath::Sum::combine()
{
	for (std::list<Basic_Term*>::iterator it = this->summands.begin(); it != this->summands.end();) {
		(*it)->combine();
		if ((*it)->get_state() == sum) {
			Sum* redundant = static_cast<Sum*>((*it));
			for (auto it_red : redundant->summands) {
				it_red->parent = this;
				this->summands.push_back(it_red);
			}
			redundant->summands.clear();
			for (auto it_red : redundant->subtractors) {
				it_red->parent = this;
				this->subtractors.push_back(it_red);
			}
			redundant->subtractors.clear();
			delete redundant;
			std::list<Basic_Term*>::iterator it_2 = it;
			++it;
			this->summands.erase(it_2);
		}
		else {
			++it;
		}
	}
	for (std::list<Basic_Term*>::iterator it = this->subtractors.begin(); it != this->subtractors.end();) {
		(*it)->combine();
		if ((*it)->get_state() == sum) {
			Sum* redundant = static_cast<Sum*>((*it));
			for (auto it_red : redundant->summands) {
				it_red->parent = this;
				this->subtractors.push_back(it_red);
			}
			redundant->summands.clear();
			for (auto it_red : redundant->subtractors) {
				it_red->parent = this;
				this->summands.push_back(it_red);
			}
			redundant->subtractors.clear();
			delete redundant;
			std::list<Basic_Term*>::iterator it_2 = it;
			++it;
			this->subtractors.erase(it_2);
		}
		else {
			++it;
		}
	}
}

bmath::Exponentiation::Exponentiation(Basic_Term* parent_)
	:Basic_Term(parent_)
{
}

bmath::Exponentiation::Exponentiation(std::string name_, Basic_Term* parent_, std::size_t op)
	:Basic_Term(parent_)
{
	LOG_C("baue Potenz: " << name_);
	std::string subterm;
	subterm = name_.substr(op + 1);
	this->exponent = build_subterm(subterm, this);
	name_.erase(op);
	this->base = build_subterm(name_, this);
}

bmath::Exponentiation::Exponentiation(const Exponentiation& source, Basic_Term* parent_)
	:Basic_Term(parent_), base(copy_subterm(source.base, this)), exponent(copy_subterm(source.exponent, this))
{
	LOG_C("kopiere Potenz: " << source);
}

bmath::Exponentiation::~Exponentiation()
{
	LOG_C("loesche Potenz: " << *this);
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

void bmath::Exponentiation::combine()
{
	this->base->combine();
	this->exponent->combine();
}