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


bmath::Variable::Variable(std::string name_, Basic_Term* parent_)
	:Basic_Term(parent_), name(name_)
{
	std::cout << "baue Variable: " << name_ << '\n';
}


bmath::Variable::~Variable()
{
}
