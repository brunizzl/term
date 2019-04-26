#include "operations.h"

using namespace bmath::intern;


bmath::intern::Product::Product(Basic_Term* parent_)
	:Basic_Term(parent_)
{
}

bmath::intern::Product::Product(std::string name_, Basic_Term* parent_, std::size_t op)
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

bmath::intern::Product::Product(const Product& source, Basic_Term* parent_)
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


bmath::intern::Product::~Product()
{
	LOG_C("loesche Produkt: " << *this);
	for (auto it : factors) {
		delete it;
	}
	for (auto it : divisors) {
		delete it;
	}
}

void bmath::intern::Product::to_str(std::string& str) const
{
	if (get_state(this->parent) >= this->get_state_intern()) {
		str.push_back('(');
	}
	bool already_printed_smth = false;
	for (auto it : this->factors) {
		if (std::exchange(already_printed_smth, true)) {
			str.push_back('*');
		}
		it->to_str(str);
	}
	if (!already_printed_smth) {
		str.push_back('1');
	}
	for (auto it : this->divisors) {
		str.push_back('/');
		it->to_str(str);
	}
	if (get_state(this->parent) >= this->get_state_intern()) {
		str.push_back(')');
	}
}

State bmath::intern::Product::get_state_intern() const
{
	return s_product;
}

void bmath::intern::Product::combine_layers()
{
	for (std::list<Basic_Term*>::iterator it = this->factors.begin(); it != this->factors.end();) {
		(*it)->combine_layers();
		if (get_state(*it) == s_product) {
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
		(*it)->combine_layers();
		if (get_state(*it) == s_product) {
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

bmath::Vals_Combinded bmath::intern::Product::combine_values()
{
	std::complex<double> buffer_factor = 1;
	bool only_known = true;
	for (std::list<Basic_Term*>::iterator it = this->factors.begin(); it != this->factors.end();) {
		Vals_Combinded factor = (*it)->combine_values();
		if (factor.known) {
			buffer_factor *= factor.val;
			delete (*it);
			std::list<Basic_Term*>::iterator it_2 = it;
			++it;
			this->factors.erase(it_2);
		}
		else {
			only_known = false;
			++it;
		}
	}
	for (std::list<Basic_Term*>::iterator it = this->divisors.begin(); it != this->divisors.end();) {
		Vals_Combinded divisor = (*it)->combine_values();
		if (divisor.known) {
			buffer_factor /= divisor.val;
			delete (*it);
			std::list<Basic_Term*>::iterator it_2 = it;
			++it;
			this->divisors.erase(it_2);
		}
		else {
			only_known = false;
			++it;
		}
	}
	if (buffer_factor != std::complex<double>(1, 0)) {
		this->factors.push_front(new Value(buffer_factor, this));
	}
	if (only_known) {
		return Vals_Combinded{ true, buffer_factor };
	}
	return Vals_Combinded{ false, 0 };
}

bmath::Vals_Combinded bmath::intern::Product::evaluate(const std::string & name_, std::complex<double> value_) const
{
	Vals_Combinded result{ true, 1 };
	for (auto it : this->factors) {
		Vals_Combinded factor_combined = it->evaluate(name_, value_);
		if (factor_combined.known) {
			result.val *= factor_combined.val;
		}
		else {
			return Vals_Combinded{ false, 0 };
		}
	}
	for (auto it : this->divisors) {
		Vals_Combinded divisor_combined = it->evaluate(name_, value_);
		if (divisor_combined.known) {
			result.val /= divisor_combined.val;
		}
		else {
			return Vals_Combinded{ false, 0 };
		}
	}
	return result;
}

bool bmath::intern::Product::search_and_replace(const std::string& name_, std::complex<double> value_)
{
	for (std::list<Basic_Term*>::iterator it = this->factors.begin(); it != this->factors.end();) {
		if ((*it)->search_and_replace(name_, value_)) {
			delete* it;
			std::list<Basic_Term*>::iterator it_2 = it;
			++it;
			this->factors.erase(it_2);
			this->factors.push_front(new Value(value_, this));
		}
		else {
			++it;
		}
	}
	for (std::list<Basic_Term*>::iterator it = this->divisors.begin(); it != this->divisors.end();) {
		if ((*it)->search_and_replace(name_, value_)) {
			delete* it;
			std::list<Basic_Term*>::iterator it_2 = it;
			++it;
			this->divisors.erase(it_2);
			this->divisors.push_front(new Value(value_, this));
		}
		else {
			++it;
		}
	}
	return false;
}

bool bmath::intern::Product::valid_state() const
{
	for (auto it : this->factors) {

		if (it == nullptr || it->valid_state() == false) {
			return false;
		}
	}
	for (auto it : this->divisors) {
		if (it == nullptr || it->valid_state() == false) {
			return false;
		}
	}
	return true;
}

bmath::intern::Sum::Sum(Basic_Term* parent_)
	:Basic_Term(parent_)
{
}

bmath::intern::Sum::Sum(std::string name_, Basic_Term* parent_, std::size_t op)
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

bmath::intern::Sum::Sum(const Sum& source, Basic_Term* parent_)
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

bmath::intern::Sum::~Sum()
{
	LOG_C("loesche Summe: " << *this);
	for (auto it : summands) {
		delete it;
	}
	for (auto it : subtractors) {
		delete it;
	}
}

void bmath::intern::Sum::to_str(std::string& str) const
{
	if (get_state(this->parent) >= this->get_state_intern()) {
		str.push_back('(');
	}
	bool need_operator = false;
	for (auto it : this->summands) {
		if (std::exchange(need_operator, true)/* && get_state(it) != s_value*/) {
			str.push_back('+');
		}
		it->to_str(str);
	}
	for (auto it : this->subtractors) {
		str.push_back('-');
		it->to_str(str);
	}
	if (get_state(this->parent) >= this->get_state_intern()) {
		str.push_back(')');
	}
}

State bmath::intern::Sum::get_state_intern() const
{
	return s_sum;
}

void bmath::intern::Sum::combine_layers()
{
	for (std::list<Basic_Term*>::iterator it = this->summands.begin(); it != this->summands.end();) {
		(*it)->combine_layers();
		if (get_state(*it) == s_sum) {
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
		(*it)->combine_layers();
		if (get_state(*it) == s_sum) {
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

bmath::Vals_Combinded bmath::intern::Sum::combine_values()
{
	std::complex<double> buffer_summand = 0;
	bool only_known = true;
	for (std::list<Basic_Term*>::iterator it = this->summands.begin(); it != this->summands.end();) {
		Vals_Combinded summand = (*it)->combine_values();
		if (summand.known) {
			buffer_summand += summand.val;
			delete (*it);
			std::list<Basic_Term*>::iterator it_2 = it;
			++it;
			this->summands.erase(it_2);
		}
		else {
			only_known = false;
			++it;
		}
	}
	for (std::list<Basic_Term*>::iterator it = this->subtractors.begin(); it != this->subtractors.end();) {
		Vals_Combinded subtractor = (*it)->combine_values();
		if (subtractor.known) {
			buffer_summand -= subtractor.val;
			delete (*it);
			std::list<Basic_Term*>::iterator it_2 = it;
			++it;
			this->subtractors.erase(it_2);
		}
		else {
			only_known = false;
			++it;
		}
	}
	if (buffer_summand != std::complex<double>(0, 0)) {
		this->summands.push_front(new Value(buffer_summand, this));
	}
	if (only_known) {
		return Vals_Combinded{ true, buffer_summand };
	}	
	return Vals_Combinded{ false, 0 };
}

bmath::Vals_Combinded bmath::intern::Sum::evaluate(const std::string & name_, std::complex<double> value_) const
{
	Vals_Combinded result{ true, 0 };
	for (auto it : this->summands) {
		Vals_Combinded summand_combined = it->evaluate(name_, value_);
		if (summand_combined.known) {
			result.val += summand_combined.val;
		}
		else {
			return Vals_Combinded{ false, 0 };
		}
	}
	for (auto it : this->subtractors) {
		Vals_Combinded subtractor_combined = it->evaluate(name_, value_);
		if (subtractor_combined.known) {
			result.val -= subtractor_combined.val;
		}
		else {
			return Vals_Combinded{ false, 0 };
		}
	}
	return result;
}

bool bmath::intern::Sum::search_and_replace(const std::string& name_, std::complex<double> value_)
{
	for (std::list<Basic_Term*>::iterator it = this->summands.begin(); it != this->summands.end();) {
		if ((*it)->search_and_replace(name_, value_)) {
			delete* it;
			std::list<Basic_Term*>::iterator it_2 = it;
			++it;
			this->summands.erase(it_2);
			this->summands.push_front(new Value(value_, this));
		}
		else {
			++it;
		}
	}
	for (std::list<Basic_Term*>::iterator it = this->subtractors.begin(); it != this->subtractors.end();) {
		if ((*it)->search_and_replace(name_, value_)) {
			delete* it;
			std::list<Basic_Term*>::iterator it_2 = it;
			++it;
			this->subtractors.erase(it_2);
			this->subtractors.push_front(new Value(value_, this));
		}
		else {
			++it;
		}
	}
	return false;
}

bool bmath::intern::Sum::valid_state() const
{
	for (auto it : this->summands) {
		if (it == nullptr || it->valid_state() == false) {
			return false;
		}
	}
	for (auto it : this->subtractors) {
		if (it == nullptr || it->valid_state() == false) {
			return false;
		}
	}
	return true;
}

bmath::intern::Exponentiation::Exponentiation(Basic_Term* parent_)
	:Basic_Term(parent_), base(nullptr), exponent(nullptr)
{
}

bmath::intern::Exponentiation::Exponentiation(std::string name_, Basic_Term* parent_, std::size_t op)
	:Basic_Term(parent_)
{
	LOG_C("baue Potenz: " << name_);
	std::string subterm;
	subterm = name_.substr(op + 1);
	this->exponent = build_subterm(subterm, this);
	name_.erase(op);
	this->base = build_subterm(name_, this);
}

bmath::intern::Exponentiation::Exponentiation(const Exponentiation& source, Basic_Term* parent_)
	:Basic_Term(parent_), base(copy_subterm(source.base, this)), exponent(copy_subterm(source.exponent, this))
{
	LOG_C("kopiere Potenz: " << source);
}

bmath::intern::Exponentiation::~Exponentiation()
{
	LOG_C("loesche Potenz: " << *this);
	delete exponent;
	delete base;
}

void bmath::intern::Exponentiation::to_str(std::string& str) const
{
	if (get_state(this->parent) > this->get_state_intern()) {
		str.push_back('(');
	}
	this->base->to_str(str);
	str.push_back('^');
	this->exponent->to_str(str);
	if (get_state(this->parent) > this->get_state_intern()) {
		str.push_back(')');
	}
}

State bmath::intern::Exponentiation::get_state_intern() const
{
	return s_exponentiation;
}

void bmath::intern::Exponentiation::combine_layers()
{
	this->base->combine_layers();
	this->exponent->combine_layers();
}

bmath::Vals_Combinded bmath::intern::Exponentiation::combine_values()
{
	Vals_Combinded base_ = this->base->combine_values();
	Vals_Combinded exponent_ = this->exponent->combine_values();
	if (base_.known && exponent_.known) {
		std::complex<double> result = std::pow(base_.val, exponent_.val);
		return Vals_Combinded{ true, result };
	}
	else if (base_.known && !exponent_.known) {
		if (get_state(this->base) != s_value) {
			delete this->base;
			this->base = new Value(base_.val, this);
		}
	}
	else if (!base_.known && exponent_.known) {
		if (get_state(this->exponent) != s_value) {
			delete this->exponent;
			this->exponent = new Value(exponent_.val, this);
		}
	}

	return Vals_Combinded{ false, 0 };
}

bmath::Vals_Combinded bmath::intern::Exponentiation::evaluate(const std::string & name_, std::complex<double> value_) const
{
	Vals_Combinded base_ = this->base->evaluate(name_, value_);
	Vals_Combinded exponent_ = this->exponent->evaluate(name_, value_);
	if (base_.known && exponent_.known) {
		std::complex<double> result = std::pow(base_.val, exponent_.val);
		return Vals_Combinded{ true, result };
	}
	return Vals_Combinded{ false, 0 };
}

bool bmath::intern::Exponentiation::search_and_replace(const std::string& name_, std::complex<double> value_)
{
	if (this->base->search_and_replace(name_, value_)) {
		delete this->base;
		this->base = new Value(value_, this);
	}
	if (this->exponent->search_and_replace(name_, value_)) {
		delete this->exponent;
		this->exponent = new Value(value_, this);
	}
	return false;
}

bool bmath::intern::Exponentiation::valid_state() const
{
	if (this->base == nullptr || this->exponent == nullptr) {
		return false;
	}
	return this->base->valid_state() && this->exponent->valid_state();
}
