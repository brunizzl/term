#include "arguments.h"

using namespace bmath::intern;


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Value
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bmath::intern::Value::Value(Basic_Term* parent_)
	:Basic_Term(parent_), value(std::complex<double>(0, 0))
{
}

bmath::intern::Value::Value(std::string name_, Basic_Term* parent_)
	:Basic_Term(parent_), value(std::complex<double>(0, 0))
{
	LOG_C("baue Wert: " << name_);
	if (name_ == std::string("i")) {
		this->value.imag(1);
	}
	else {
		double factor;
		std::stringstream stream;
		stream << name_;
		stream >> factor;
		if (name_.find_first_of("i") != std::string::npos) {
			this->value.imag(factor);
		}
		else {
			this->value.real(factor);
		}
	}
}

bmath::intern::Value::Value(const Value& source, Basic_Term* parent_)
	:Basic_Term(parent_), value(source.value)
{
	LOG_C("kopiere Wert: " << source);
}

bmath::intern::Value::Value(std::complex<double> value_, Basic_Term* parent_)
	: Basic_Term(parent_), value(value_)
{
}


bmath::intern::Value::~Value()
{
	LOG_C("loesche Wert: " << *this);
}

void bmath::intern::Value::to_str(std::string& str) const
{
	double re = this->value.real();
	double im = this->value.imag();
	if (re != 0 && im != 0) {
		if (get_state(this->parent) >= this->get_state_intern()) {
			str.push_back('(');
		}
		std::stringstream stream_re;
		stream_re << re;
		std::stringstream stream_im;
		stream_im << std::showpos << im;
		str.append(stream_re.str());
		str.append(stream_im.str());
		str.push_back('i');
		if (get_state(this->parent) >= this->get_state_intern()) {
			str.push_back(')');
		}
	}
	else if (re != 0 && im == 0) {
		std::stringstream stream_re;
		stream_re << re;
		str.append(stream_re.str());
	}
	else if (re == 0 && im != 0) {
		if (im != 1) {
			std::stringstream stream_im;
			stream_im << im;
			str.append(stream_im.str());
		}
		str.push_back('i');
	}
	else if (re == 0 && im == 0) {
		str.push_back('0');
	}
}

State bmath::intern::Value::get_state_intern() const
{
	return s_value;
}

Vals_Combined bmath::intern::Value::combine_values()
{
	return Vals_Combined{ true, this->value };
}

Vals_Combined bmath::intern::Value::evaluate(const std::list<Known_Variable>& known_variables) const
{
	return Vals_Combined{true, this->value};
}

void bmath::intern::Value::search_and_replace(const std::string& name_, std::complex<double> value_, Basic_Term*& storage_key)
{
	//nothing to be done here
}

bool bmath::intern::Value::valid_state() const
{
	//holds no pointers that could be invalid
	return true;
}

bool bmath::intern::Value::re_smaller_than_0()
{
	if (this->value.real() < 0) {
		this->value *= -1.0;
		return true;
	}
	else {
		return false;
	}
}

void bmath::intern::Value::list_subterms(std::list<Basic_Term*>& subterms, State listed_state) const
{
	if (listed_state == s_value) {
		subterms.push_back(const_cast<Value*>(this));
	}
}

void bmath::intern::Value::sort()
{
	//nothing to be done here
}

bool bmath::intern::Value::operator<(const Basic_Term& other) const
{
	if (this->get_state_intern() != other.get_state_intern()) {
		return this->get_state_intern() < other.get_state_intern();
	}
	else {
		const Value* other_val = static_cast<const Value*>(&other);
		if (this->value.real() != other_val->value.real()) {
			return this->value.real() < other_val->value.real();
		}
		else {
			return this->value.imag() < other_val->value.imag();
		}
	}
}

bool bmath::intern::Value::operator==(const Basic_Term& other) const
{
	if (this->get_state_intern() != other.get_state_intern()) {
		return false;
	}
	else {
		const Value* other_val = static_cast<const Value*>(&other);
		return this->value == other_val->value;
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Variable
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bmath::intern::Variable::Variable(Basic_Term* parent_)
	:Basic_Term(parent_)
{
}

bmath::intern::Variable::Variable(std::string name_, Basic_Term* parent_)
	:Basic_Term(parent_), name(name_)
{
	LOG_C("baue Variable: " << name_);
}

bmath::intern::Variable::Variable(const Variable& source, Basic_Term* parent_)
	: Basic_Term(parent_), name(source.name)
{
	LOG_C("kopiere Variable: " << source);
}

bmath::intern::Variable::~Variable()
{
	LOG_C("loesche Variable: " << *this);
}

void bmath::intern::Variable::to_str(std::string& str) const
{
	str.append(this->name);
}

State bmath::intern::Variable::get_state_intern() const
{
	return s_variable;
}

Vals_Combined bmath::intern::Variable::combine_values()
{
	return Vals_Combined{false, 0};
}

Vals_Combined bmath::intern::Variable::evaluate(const std::list<Known_Variable>& known_variables) const
{
	for (auto& it : known_variables) {
		if (it.name == this->name) {
			return Vals_Combined{ true, it.value };
		}
	}
	return Vals_Combined{ false, 0 };
}

void bmath::intern::Variable::search_and_replace(const std::string& name_, std::complex<double> value_, Basic_Term*& storage_key)
{
	if (this->name == name_) {
		storage_key = new Value(value_, this->parent);
		delete this;
	}
}

bool bmath::intern::Variable::valid_state() const
{
	//holds no pointers that could be invalid
	return true;
}

void bmath::intern::Variable::list_subterms(std::list<Basic_Term*>& subterms, State listed_state) const
{
	if (listed_state == s_variable) {
		subterms.push_back(const_cast<Variable*>(this));
	}
}

void bmath::intern::Variable::sort()
{
	//nothing to be done here
}

bool bmath::intern::Variable::operator<(const Basic_Term& other) const
{
	if (this->get_state_intern() != other.get_state_intern()) {
		return this->get_state_intern() < other.get_state_intern();
	}
	else {
		const Variable* other_var = static_cast<const Variable*>(&other);
		return this->name < other_var->name;
	}
}

bool bmath::intern::Variable::operator==(const Basic_Term& other) const
{
	if (this->get_state_intern() != other.get_state_intern()) {
		return false;
	}
	else {
		const Variable* other_var = static_cast<const Variable*>(&other);
		return this->name == other_var->name;
	}
}
