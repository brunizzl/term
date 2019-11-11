
#include <sstream>
#include <charconv>
#include <cassert>

#include "arguments.h"
#include "internal_functions.h"

using namespace bmath::intern;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Value\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Value::Value(const Value& source)
	:value(source.value)
{
}

Value::Value(std::complex<double> value_)
	:value(value_)
{
}

Value::~Value()
{
}

void Value::to_str(std::string& str, int caller_operator_precedence) const
{
	str.append(to_string(this->value, caller_operator_precedence));
}

void Value::to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const
{
	std::string new_line(dist_root * 5, ' ');	//building string with spaces matching dept of this
	this->to_str(new_line, operator_precedence(Type::undefined));

	tree_lines.push_back(std::move(new_line));
	append_last_line(tree_lines, line_prefix);
}

Type Value::type() const
{
	return Type::value;
}

void bmath::intern::Value::combine_layers(Basic_Term*& storage_key)
{
	//nothing to combine layerwise here
}

Vals_Combined Value::combine_values()
{
	return { true, this->value };
}

std::complex<double> Value::evaluate(const std::list<bmath::Known_Variable>& known_variables) const
{
	return { this->value.real(), this->value.imag() };
}

void Value::search_and_replace(const std::string& name_, const Basic_Term* value_, Basic_Term*& storage_key)
{
	//nothing to be done here
}

void bmath::intern::Value::for_each(std::function<void(Basic_Term* this_ptr, Type this_type)> func)
{
	func(this, Type::value);
}

Basic_Term** Value::match_intern(Basic_Term* pattern, std::list<Pattern_Variable*>& pattern_var_adresses, Basic_Term** storage_key)
{
	reset_all_pattern_vars(pattern_var_adresses);
	this->sort();
	if (this->equal_to_pattern(pattern, nullptr, storage_key)) {
		return storage_key;
	}
	else {
		return nullptr;
	}
}

bool bmath::intern::Value::transform(Basic_Term** storage_key, bool only_shallow)
{
	return false;
}

bool bmath::intern::Value::equal_to_pattern(Basic_Term* pattern, Basic_Term* patterns_parent, Basic_Term** storage_key)
{
	const Type pattern_type = type_of(pattern);
	if (pattern_type == Type::value) {
		const Value* other_val = static_cast<const Value*>(pattern);
		return this->value == other_val->value;
	}
	else if (pattern_type == Type::pattern_variable) {
		Pattern_Variable* pattern_var = static_cast<Pattern_Variable*>(pattern);
		return pattern_var->try_matching(this, patterns_parent, storage_key);
	}
	else {
		return false;
	}
}

void bmath::intern::Value::reset_own_matches(Basic_Term* parent)
{
}

bool Value::operator<(const Basic_Term& other) const
{
	if (Type::value != type_of(other)) {
		return Type::value < type_of(other);
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

bool Value::operator==(const Basic_Term& other) const
{
	if (type_of(other) == Type::value) {
		const Value* other_val = static_cast<const Value*>(&other);
		return this->value == other_val->value;
	}
	return false;
}

std::complex<double>& bmath::intern::Value::val()
{
	return this->value;
}

const std::complex<double>& bmath::intern::Value::val() const
{
	return this->value;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Variable\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Variable::Variable(std::string_view name_)
	:name(name_)
{
}

Variable::Variable(const Variable& source)
	:name(source.name)
{
}

Variable::~Variable()
{
}

void Variable::to_str(std::string& str, int caller_operator_precedence) const
{
	str.append(this->name);
}

void Variable::to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const
{
	std::string new_line(dist_root * 5, ' ');	//building string with spaces matching dept of this
	this->to_str(new_line, operator_precedence(Type::undefined));

	tree_lines.push_back(std::move(new_line));
	append_last_line(tree_lines, line_prefix);
}

Type Variable::type() const
{
	return Type::variable;
}

void bmath::intern::Variable::combine_layers(Basic_Term*& storage_key)
{
	//nothing to combine layerwise.
}

Vals_Combined Variable::combine_values()
{
	return { false, 0 };
}

std::complex<double> Variable::evaluate(const std::list<bmath::Known_Variable>& known_variables) const
{
	for (const auto& it : known_variables) {
		if (it.name == this->name) {
			return it.value;
		}
	}
	throw XTermCouldNotBeEvaluated(name.data());
}

void Variable::search_and_replace(const std::string& name_, const Basic_Term* value_, Basic_Term*& storage_key)
{
	if (this->name == name_) {
		storage_key = copy_subterm(value_);
		delete this;
	}
}

void bmath::intern::Variable::for_each(std::function<void(Basic_Term* this_ptr, Type this_type)> func)
{
	func(this, Type::variable);
}

Basic_Term** Variable::match_intern(Basic_Term* pattern, std::list<Pattern_Variable*>& pattern_var_adresses, Basic_Term** storage_key)
{
	reset_all_pattern_vars(pattern_var_adresses);
	this->sort();
	if (this->equal_to_pattern(pattern, nullptr, storage_key )) {
		return storage_key;
	}
	else {
		return nullptr;
	}
}

bool bmath::intern::Variable::transform(Basic_Term** storage_key, bool only_shallow)
{
	return false;
}

bool bmath::intern::Variable::equal_to_pattern(Basic_Term* pattern, Basic_Term* patterns_parent, Basic_Term** storage_key)
{
	const Type pattern_type = type_of(pattern);
	if (pattern_type == Type::variable) {
		const Variable* other_var = static_cast<const Variable*>(pattern);
		return this->name == other_var->name;
	}
	else if (pattern_type == Type::pattern_variable) {
		Pattern_Variable* pattern_var = static_cast<Pattern_Variable*>(pattern);
		return pattern_var->try_matching(this, patterns_parent, storage_key);
	}
	else {
		return false;
	}
}

void bmath::intern::Variable::reset_own_matches(Basic_Term* parent)
{
	assert(false);	//reset_own_matches should be called exclusively in a pattern. patterns don't hold normal variables
}

bool Variable::operator<(const Basic_Term& other) const
{
	if (Type::variable != type_of(other)) {
		return Type::variable < type_of(other);
	}
	else {
		const Variable* other_var = static_cast<const Variable*>(&other);
		return this->name < other_var->name;
	}
}

bool Variable::operator==(const Basic_Term& other) const
{
	if (type_of(other) == Type::variable) {
		const Variable* other_var = static_cast<const Variable*>(&other);
		return this->name == other_var->name;
	}
	return false;
}

