#include "operations.h"

using namespace bmath;


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
	LOG_C("loesche Produkt: " << *this);
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

void bmath::Product::sort()
{
}

bool bmath::Product::operator<(const Basic_Term& other) const
{
	return false;
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
	LOG_C("loesche Summe: " << *this);
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

void bmath::Sum::sort()
{
}

bool bmath::Sum::operator<(const Basic_Term& other) const
{
	return false;
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

void bmath::Exponentiation::sort()
{
}

bool bmath::Exponentiation::operator<(const Basic_Term& other) const
{
	return false;
}

bmath::Par_Operator::Par_Operator(std::string name_, Basic_Term* parent_, Par_Op_State op_state_)
	:Basic_Term(parent_), op_state(op_state_), argument(nullptr)
{
	LOG_C("baue Par_Operator: " << name_);
	name_.pop_back(); //closing parenthesis gets cut of
	switch (op_state_) {
	//erase function cuts of operator + opening par -> operator cases sorted by length
	case ln:
		name_.erase(0, 3);
		LOG_C("shortened name: " << name_);
		this->argument = build_subterm(name_, this);
		break;
	case exp:
	case sin:
	case cos:
	case tan:
		name_.erase(0, 4);
		LOG_C("shortened name: " << name_);
		this->argument = build_subterm(name_, this);
		break;
	case log2:
	case asin:
	case acos:
	case atan:
	case sinh:
	case cosh:
	case tanh:
		name_.erase(0, 5);
		LOG_C("shortened name: " << name_);
		this->argument = build_subterm(name_, this);
		break;
	case log10:
	case gamma:
		name_.erase(0, 6);
		LOG_C("shortened name: " << name_);
		this->argument = build_subterm(name_, this);
		break;
	}
}

bmath::Par_Operator::~Par_Operator()
{
	LOG_C("loesche Par_Operator: " << *this);
	delete this->argument;
}

void bmath::Par_Operator::to_str(std::string& str) const
{
	switch (this->op_state) {
	case ln:
		str.append("ln(");
		break;
	case log10:
		str.append("log10(");
		break;
	case log2:
		str.append("log2(");
		break;
	case exp:
		str.append("exp(");
		break;
	case sin:
		str.append("sin(");
		break;
	case cos:
		str.append("cos(");
		break;
	case tan:
		str.append("tan(");
		break;
	case asin:
		str.append("asin(");
		break;
	case acos:
		str.append("acos(");
		break;
	case atan:
		str.append("atan(");
		break;
	case sinh:
		str.append("sinh(");
		break;
	case cosh:
		str.append("cosh(");
		break;
	case tanh:
		str.append("tanh(");
		break;
	case gamma:
		str.append("gamma(");
		break;
	}
	this->argument->to_str(str);
	str.push_back(')');
}

State bmath::Par_Operator::get_state() const
{
	return State();
}

void bmath::Par_Operator::sort()
{
}

bool bmath::Par_Operator::operator<(const Basic_Term& other) const
{
	return false;
}
