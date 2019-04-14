#include "arguments.h"

using namespace bmath;


bmath::Value::Value(std::string name_, Basic_Term* parent_)
	:Basic_Term(parent_), value(0)
{
	LOG_C("baue Wert: " << name_);
	std::stringstream stream;
	stream << name_;
	stream >> this->value;
}


bmath::Value::~Value()
{
	LOG_C("loesche Wert: " << *this);
}

void bmath::Value::to_str(std::string& str) const
{
	std::stringstream stream;
	stream << this->value;
	str.append(stream.str());
}

State bmath::Value::get_state() const
{
	return val;
}

void bmath::Value::sort()
{
}

bool bmath::Value::operator<(const Basic_Term& other) const
{
	return false;
}


bmath::Variable::Variable(std::string name_, Basic_Term* parent_)
	:Basic_Term(parent_), name(name_)
{
	LOG_C("baue Variable: " << name_);
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
	return var;
}

void bmath::Variable::sort()
{
}

bool bmath::Variable::operator<(const Basic_Term& other) const
{
	return false;
}
