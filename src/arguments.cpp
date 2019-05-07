#include "arguments.h"

using namespace bmath::intern;


bmath::intern::Value::Value(Basic_Term* parent_)
	:Basic_Term(parent_), value(std::complex<double>(0, 0))
{
}

bmath::intern::Value::Value(std::string name_, Basic_Term* parent_)
	:Basic_Term(parent_)
{
	LOG_C("baue Wert: " << name_);
	if (name_ == std::string("i")) {
		this->value = std::complex<double>(0, 1);
	}
	else {
		double factor;
		std::stringstream stream;
		stream << name_;
		stream >> factor;
		if (name_.find_first_of("i") != std::string::npos) {
			this->value = std::complex<double>(0, factor);
		}
		else {
			this->value = std::complex<double>(factor, 0);
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
		std::stringstream stream_im;
		stream_im << im;
		str.append(stream_im.str());
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

bmath::Vals_Combinded bmath::intern::Value::combine_values()
{
	return Vals_Combinded{ true, this->value };
}

bmath::Vals_Combinded bmath::intern::Value::evaluate(const std::string & name_, std::complex<double> value_) const
{
	return Vals_Combinded{true, this->value};
}

bool bmath::intern::Value::search_and_replace(const std::string& name_, std::complex<double> value_)
{
	return false;
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

bmath::Vals_Combinded bmath::intern::Variable::combine_values()
{
	return Vals_Combinded{false, 0};
}

bmath::Vals_Combinded bmath::intern::Variable::evaluate(const std::string & name_, std::complex<double> value_) const
{
	bool same_var = this->name == name_;
	return Vals_Combinded{ same_var, value_ };
}

bool bmath::intern::Variable::search_and_replace(const std::string& name_, std::complex<double> value_)
{
	return this->name == name_;
}

bool bmath::intern::Variable::valid_state() const
{
	//holds no pointers that could be invalid
	return true;
}
