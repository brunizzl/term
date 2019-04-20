#include "arguments.h"

using namespace bmath;


bmath::Value::Value(Basic_Term* parent_)
	:Basic_Term(parent_)
{
}

bmath::Value::Value(std::string name_, Basic_Term* parent_)
	:Basic_Term(parent_), value(0)
{
	LOG_C("baue Wert: " << name_);
	std::stringstream stream;
	stream << name_;
	stream >> this->value;
}

bmath::Value::Value(const Value& source, Basic_Term* parent_)
	:Basic_Term(parent_), value(source.value)
{
	LOG_C("kopiere Wert: " << source);
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

void bmath::Value::combine()
{
	//nothing needs to be done here (value does not hold any pointers to combine with)
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
	return var;
}

void bmath::Variable::combine()
{
	//nothing needs to be done here (variable does not hold any pointers to combine with)
}