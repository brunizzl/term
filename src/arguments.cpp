#include "arguments.h"

using namespace bmath;


bmath::Value::Value(Basic_Term* parent_)
	:Basic_Term(parent_), value(std::complex<double>(0, 0))
{
}

bmath::Value::Value(std::string name_, Basic_Term* parent_)
	:Basic_Term(parent_)
{
	LOG_C("baue Wert: " << name_);
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

bmath::Value::Value(const Value& source, Basic_Term* parent_)
	:Basic_Term(parent_), value(source.value)
{
	LOG_C("kopiere Wert: " << source);
}

bmath::Value::Value(std::complex<double> value_, Basic_Term* parent_)
	: Basic_Term(parent_), value(value_)
{
}


bmath::Value::~Value()
{
	LOG_C("loesche Wert: " << *this);
}

void bmath::Value::to_str(std::string& str) const
{
	double re = this->value.real();
	double im = this->value.imag();
	if (re != 0 && im != 0) {
		std::stringstream stream_re;
		stream_re << re;
		std::stringstream stream_im;
		stream_im << std::showpos << im;
		str.append(stream_re.str());
		str.append(stream_im.str());
		str.push_back('i');
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

State bmath::Value::get_state() const
{
	return s_value;
}

Vals_Combinded bmath::Value::combine_values()
{
	return Vals_Combinded{ true, this->value };
}

Vals_Combinded bmath::Value::evaluate(const std::string & name_, std::complex<double> value_) const
{
	return Vals_Combinded{true, this->value};
}

bool bmath::Value::search_and_replace(const std::string& name_, std::complex<double> value_)
{
	return false;
}

bool bmath::Value::valid_state() const
{
	//holds no pointers that could be invalid
	return true;
}

bmath::Variable::Variable(Basic_Term* parent_)
	:Basic_Term(parent_)
{
}

bmath::Variable::Variable(std::string name_, Basic_Term* parent_)
	:Basic_Term(parent_), name(name_)
{
	LOG_C("baue Variable: " << name_);
}

bmath::Variable::Variable(const Variable& source, Basic_Term* parent_)
	: Basic_Term(parent_), name(source.name)
{
	LOG_C("kopiere Variable: " << source);
}

bmath::Variable::~Variable()
{
	LOG_C("loesche Variable: " << *this);
}

void bmath::Variable::to_str(std::string& str) const
{
	str.append(this->name);
}

State bmath::Variable::get_state() const
{
	return s_variable;
}

Vals_Combinded bmath::Variable::combine_values()
{
	return Vals_Combinded{false, 0};
}

Vals_Combinded bmath::Variable::evaluate(const std::string & name_, std::complex<double> value_) const
{
	bool same_var = this->name == name_;
	return Vals_Combinded{ same_var, value_ };
}

bool bmath::Variable::search_and_replace(const std::string& name_, std::complex<double> value_)
{
	return this->name == name_;
}

bool bmath::Variable::valid_state() const
{
	//holds no pointers that could be invalid
	return true;
}
