
#include "arguments.h"
#include "internal_functions.h"

#include <sstream>
#include <charconv>

using namespace bmath::intern;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Value\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Value::Value(std::string_view name_, Basic_Term* parent_)
	:Basic_Term(parent_), val({ 0.0, 0.0 })
{
	if (name_ == "i") {
		this->val.imag(1);
	}
	else {
		double factor;
		std::from_chars(name_.data(), name_.data() + name_.size(), factor);
		if (name_.find_first_of('i') != std::string::npos) {
			this->val.imag(factor);
		}
		else {
			this->val.real(factor);
		}
	}
}

Value::Value(const Value& source, Basic_Term* parent_)
	:Basic_Term(parent_), val(source.val)
{
}

Value::Value(std::complex<double> value_, Basic_Term* parent_)
	: Basic_Term(parent_), val(value_)
{
}


std::string bmath::intern::Value::val_to_str(bool inverse) const {	//warning: ugliest function ever :(
	const double re = inverse ? -(this->val.real()) : this->val.real();
	const double im = inverse ? -(this->val.imag()) : this->val.imag();
	bool pars = false;	//decides if parentheses are put around number
	std::stringstream buffer;

	if (re != 0 && im != 0) {
		pars = type(this->parent) > this->get_type(); //stronger binding term above -> parentheses
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
		pars = re < 0 && type(this->parent) > value; //leading '-' and term above		
		buffer << re;
	}
	else if (re == 0 && im != 0) {		// yi
		pars = im < 0 && type(this->parent) > value; //leading '-' and term above		
		if (im > -1.00000000001 && im < -0.99999999999) {		//im == -1
			buffer << '-';
		}
		else if (im > 1.00000000001 || im < 0.99999999999) {	//im != 1
			buffer << im;
		}
		buffer << 'i';
	}
	else if (re == 0 && im == 0) {
		buffer << '0';
	}

	if (pars) {
		return '(' + buffer.str() + ')';
	}
	else {
		return buffer.str();
	}
}

Value::~Value()
{
}

void Value::to_str(std::string& str) const
{
	str.append(this->val_to_str(false));
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
	return value;
}

Vals_Combined Value::combine_values()
{
	return Vals_Combined{ true, this->val };
}

std::complex<double> Value::evaluate(const std::list<bmath::Known_Variable>& known_variables) const
{
	return this->val;
}

void Value::search_and_replace(const std::string& name_, const Basic_Term* value_, Basic_Term*& storage_key)
{
	//nothing to be done here
}

void Value::list_subterms(std::list<Basic_Term*>& subterms, Type listed_type) const
{
	if (listed_type == value) {
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
	if (this->get_type() != other.get_type()) {
		return this->get_type() < other.get_type();
	}
	else {
		const Value* other_val = static_cast<const Value*>(&other);
		if (this->val.real() != other_val->val.real()) {
			return this->val.real() < other_val->val.real();
		}
		else {
			return this->val.imag() < other_val->val.imag();
		}
	}
}

bool Value::operator==(const Basic_Term& other) const
{
	switch (other.get_type()) {
	case value:
		break;
	case pattern_variable:
		return other == *this;
	default:
		return false;
	}
	const Value* other_val = static_cast<const Value*>(&other);
	return this->val == other_val->val;
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

Type Variable::get_type() const
{
	return variable;
}

Vals_Combined Variable::combine_values()
{
	return Vals_Combined{ false, 0 };
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

void Variable::list_subterms(std::list<Basic_Term*>& subterms, Type listed_type) const
{
	if (listed_type == variable) {
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
	case variable:
		break;
	case pattern_variable:
		return other == *this;
	default:
		return false;
	}
	const Variable* other_var = static_cast<const Variable*>(&other);
	return this->name == other_var->name;
}

