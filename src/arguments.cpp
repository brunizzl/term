#include "arguments.h"

using namespace bmath;


bmath::Value::Value(std::string name_, Basic_Term* parent_)
	:Basic_Term(parent_), value(0)
{
	std::cout << "baue Wert: " << name_ << '\n';
	std::stringstream stream;
	stream << name_;
	stream >> this->value;
}


bmath::Value::~Value()
{
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


bmath::Variable::Variable(std::string name_, Basic_Term* parent_)
	:Basic_Term(parent_), name(name_)
{
	std::cout << "baue Variable: " << name_ << '\n';
}


bmath::Variable::~Variable()
{
}

void bmath::Variable::to_str(std::string& str) const
{
	str.append(this->name);
}

State bmath::Variable::get_state() const
{
	return var;
}
