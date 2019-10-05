
#include <sstream>
#include <charconv>

#include "arguments.h"
#include "internal_functions.h"

using namespace bmath::intern;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Value\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Value::Value(const Value& source, Basic_Term* parent_)
	:std::complex<double>(source), parent_ptr(parent_)
{
}

Value::Value(std::complex<double> value_, Basic_Term* parent_)
	:std::complex<double>(value_), parent_ptr(parent_)
{
}

Value::~Value()
{
}

Basic_Term* bmath::intern::Value::parent() const
{
	return this->parent_ptr;
}

void bmath::intern::Value::set_parent(Basic_Term* new_parent)
{
	this->parent_ptr = new_parent;
}

void Value::to_str(std::string& str) const
{
	str.append(to_string(*this, type(parent_ptr)));
}

void Value::to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const
{
	std::string new_line(dist_root * 5, ' ');	//building string with spaces matching dept of this
	this->to_str(new_line);

	tree_lines.push_back(std::move(new_line));
	append_last_line(tree_lines, line_prefix);
}

Type Value::get_type() const
{
	return Type::value;
}

Vals_Combined Value::combine_values()
{
	return { true, *this };
}

std::complex<double> Value::evaluate(const std::list<bmath::Known_Variable>& known_variables) const
{
	return { this->real(), this->imag() };
}

void Value::search_and_replace(const std::string& name_, const Basic_Term* value_, Basic_Term*& storage_key)
{
	//nothing to be done here
}

void Value::list_subterms(std::list<const Basic_Term*>& subterms, Type listed_type) const
{
	if (listed_type == Type::value) {
		subterms.push_back(this);
	}
}

void Value::sort()
{
	//nothing to be done here
}

Basic_Term** Value::match_intern(Basic_Term* pattern, std::list<Pattern_Variable*>& pattern_var_adresses, Basic_Term** storage_key)
{
	if (*this == *pattern) {
		return storage_key;
	}
	else {
		return nullptr;
	}
}

bool Value::operator<(const Basic_Term& other) const
{
	if (this->get_type() != other.get_type()) {
		return this->get_type() < other.get_type();
	}
	else {
		const Value* other_val = static_cast<const Value*>(&other);
		if (this->real() != other_val->real()) {
			return this->real() < other_val->real();
		}
		else {
			return this->imag() < other_val->imag();
		}
	}
}

bool Value::operator==(const Basic_Term& other) const
{
	switch (other.get_type()) {
	case Type::value:
		break;
	case Type::pattern_variable:
		return other == *this;	//other is Basic_Term -> the operator==() of Basic_Term has to be used.
	default:
		return false;
	}
	const Value* other_val = static_cast<const Value*>(&other);
	return static_cast<std::complex<double>>(*this) == static_cast<std::complex<double>>(*other_val);	//making sure to call operator==() for std::complex
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Variable\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Variable::Variable(std::string_view name_, Basic_Term* parent_)
	:parent_ptr(parent_), name(name_)
{
}

Variable::Variable(const Variable& source, Basic_Term* parent_)
	: parent_ptr(parent_), name(source.name)
{
}

Variable::~Variable()
{
}

Basic_Term* bmath::intern::Variable::parent() const
{
	return this->parent_ptr;
}

void bmath::intern::Variable::set_parent(Basic_Term* new_parent)
{
	this->parent_ptr = new_parent;
}

void Variable::to_str(std::string& str) const
{
	str.append(this->name);
}

void Variable::to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const
{
	std::string new_line(dist_root * 5, ' ');	//building string with spaces matching dept of this
	this->to_str(new_line);

	tree_lines.push_back(std::move(new_line));
	append_last_line(tree_lines, line_prefix);
}

Type Variable::get_type() const
{
	return Type::variable;
}

Vals_Combined Variable::combine_values()
{
	return { false, 0 };
}

std::complex<double> Variable::evaluate(const std::list<bmath::Known_Variable>& known_variables) const
{
	for (auto& it : known_variables) {
		if (it.name == this->name) {
			return it.value;
		}
	}
	throw XTermCouldNotBeEvaluated(name.data());
}

void Variable::search_and_replace(const std::string& name_, const Basic_Term* value_, Basic_Term*& storage_key)
{
	if (this->name == name_) {
		storage_key = copy_subterm(value_, this->parent_ptr);
		delete this;
	}
}

void Variable::list_subterms(std::list<const Basic_Term*>& subterms, Type listed_type) const
{
	if (listed_type == Type::variable) {
		subterms.push_back(this);
	}
}

void Variable::sort()
{
	//nothing to be done here
}

Basic_Term** Variable::match_intern(Basic_Term* pattern, std::list<Pattern_Variable*>& pattern_var_adresses, Basic_Term** storage_key)
{
	if (*this == *pattern) {
		return storage_key;
	}
	else {
		return nullptr;
	}
}

bool Variable::operator<(const Basic_Term& other) const
{
	if (this->get_type() != other.get_type()) {
		return this->get_type() < other.get_type();
	}
	else {
		const Variable* other_var = static_cast<const Variable*>(&other);
		return this->name < other_var->name;
	}
}

bool Variable::operator==(const Basic_Term& other) const
{
	switch (other.get_type()) {
	case Type::variable:
		break;
	case Type::pattern_variable:
		return other == *this;
	default:
		return false;
	}
	const Variable* other_var = static_cast<const Variable*>(&other);
	return this->name == other_var->name;
}

