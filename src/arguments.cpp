
#include "arguments.h"

using namespace bmath::intern;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Value\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Value::Value(std::string_view name_, Basic_Term* parent_)
	:Basic_Term(parent_), value(std::complex<double>(0, 0))
{
	if (name_ == "i") {
		this->value.imag(1);
	}
	else {
		double factor;
		std::from_chars(name_.data(), name_.data() + name_.size(), factor);
		if (name_.find_first_of("i") != std::string::npos) {
			this->value.imag(factor);
		}
		else {
			this->value.real(factor);
		}
	}
}

Value::Value(const Value& source, Basic_Term* parent_)
	:Basic_Term(parent_), value(source.value)
{
}

Value::Value(std::complex<double> value_, Basic_Term* parent_)
	: Basic_Term(parent_), value(value_)
{
}


Value::~Value()
{
}

void Value::to_str(std::string& str) const
{
	const double re = this->value.real();
	const double im = this->value.imag();
	bool pars = false;
	std::stringstream buffer;

	if (re != 0 && im != 0) {
		pars = state(this->parent) > this->get_state(); //stronger binding term above -> parentheses
		buffer << re;

		if (im > -1.00000000001 && im < -0.99999999999) {		//im == -1
			buffer << '-';
		}
		else if (im < 1.00000000001 && im > 0.99999999999) {	//im == 1
			buffer << '+';
		}
		else {
			buffer << std::showpos << im;
		}
		buffer << 'i';
	}
	else if (re != 0 && im == 0) {
		pars = re < 0 && this->parent != nullptr; //leading '-' and term above		
		buffer << re;
	}
	else if (re == 0 && im != 0) {		// yi
		pars = im < 0 && this->parent != nullptr; //leading '-' and term above		
		if (im > -1.00000000001 && im < -0.99999999999) {		//im == -1
			buffer << '-';
		}
		else if (im > 1.00000000001 || im < 0.99999999999) {	//im != 1
			buffer << im;
		}
		buffer << 'i';
	}
	else if (re == 0 && im == 0) {
		str.push_back('0');
		return;
	}

	if (pars) {
		str.push_back('(');
		str.append(buffer.str());
		str.push_back(')');
	}
	else {
		str.append(buffer.str());
	}
}

void Value::to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const
{
	std::string new_line(dist_root * 5, ' ');	//building string with spaces matching dept of this
	this->to_str(new_line);

	tree_lines.push_back(std::move(new_line));
	append_last_line(tree_lines, line_prefix);
}

State Value::get_state() const
{
	return s_value;
}

Vals_Combined Value::combine_values()
{
	return Vals_Combined{ true, this->value };
}

std::complex<double> Value::evaluate(const std::list<bmath::Known_Variable>& known_variables) const
{
	return this->value;
}

void Value::search_and_replace(const std::string& name_, const Basic_Term* value_, Basic_Term*& storage_key)
{
	//nothing to be done here
}

bool Value::re_smaller_than_0()
{
	if (this->value.real() < 0) {
		this->value *= -1.0;
		return true;
	}
	else {
		return false;
	}
}

void Value::list_subterms(std::list<Basic_Term*>& subterms, State listed_state) const
{
	if (listed_state == s_value) {
		subterms.push_back(const_cast<Value*>(this));
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
	if (this->get_state() != other.get_state()) {
		return this->get_state() < other.get_state();
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
	switch (other.get_state()) {
	case s_value:
		break;
	case s_pattern_variable:
		return other == *this;
	default:
		return false;
	}
	const Value* other_val = static_cast<const Value*>(&other);
	return this->value == other_val->value;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Variable\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Variable::Variable(std::string_view name_, Basic_Term* parent_)
	:Basic_Term(parent_), name(name_)
{
}

Variable::Variable(const Variable& source, Basic_Term* parent_)
	: Basic_Term(parent_), name(source.name)
{
}

Variable::~Variable()
{
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

State Variable::get_state() const
{
	return s_variable;
}

Vals_Combined Variable::combine_values()
{
	return Vals_Combined{false, 0};
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
		storage_key = copy_subterm(value_, this->parent);
		delete this;
	}
}

void Variable::list_subterms(std::list<Basic_Term*>& subterms, State listed_state) const
{
	if (listed_state == s_variable) {
		subterms.push_back(const_cast<Variable*>(this));
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
	if (this->get_state() != other.get_state()) {
		return this->get_state() < other.get_state();
	}
	else {
		const Variable* other_var = static_cast<const Variable*>(&other);
		return this->name < other_var->name;
	}
}

bool Variable::operator==(const Basic_Term& other) const
{
	switch (other.get_state()) {
	case s_variable:
		break;
	case s_pattern_variable:
		return other == *this;
	default:
		return false;
	}
	const Variable* other_var = static_cast<const Variable*>(&other);
	return this->name == other_var->name;
}

