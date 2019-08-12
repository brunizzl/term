#include "arguments.h"

using namespace bmath::intern;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Value\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Value::Value(std::string name_, Basic_Term* parent_)
	:Basic_Term(parent_), value(std::complex<double>(0, 0))
{
	LOG_C("baue Wert: " << name_);
	if (name_ == std::string("i")) {
		this->value.imag(1);
	}
	else {
		double factor;
		std::stringstream stream;
		stream << name_;
		stream >> factor;
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
	LOG_C("kopiere Wert: " << source);
}

Value::Value(std::complex<double> value_, Basic_Term* parent_)
	: Basic_Term(parent_), value(value_)
{
}


Value::~Value()
{
	LOG_C("loesche Wert: " << *this);
}

void Value::to_str(std::string& str) const
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
		if (im != 1) {
			std::stringstream stream_im;
			stream_im << im;
			str.append(stream_im.str());
		}
		str.push_back('i');
	}
	else if (re == 0 && im == 0) {
		str.push_back('0');
	}
}

void Value::to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const
{
	std::string new_line(dist_root * 5, ' ');	//building string with spaces matching dept of this
	this->to_str(new_line);

	tree_lines.push_back(std::move(new_line));
	append_last_line(tree_lines, line_prefix);
}

State Value::get_state_intern() const
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
		LOG_P("nicht matched value: " << *this << " =/= " << *pattern);
		return nullptr;
	}
}

bool Value::operator<(const Basic_Term& other) const
{
	if (this->get_state_intern() != other.get_state_intern()) {
		return this->get_state_intern() < other.get_state_intern();
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
	LOG_P(" vergleiche  " << *this << " und " << other);
	switch (other.get_state_intern()) {
	case s_value:
		break;
	case s_pattern_variable:
		return other == *this;
	default:
		LOG_P("wert ungleich (verschiedener state) " << this->get_state_intern() << " =/= " << other.get_state_intern());
		return false;
	}
	const Value* other_val = static_cast<const Value*>(&other);
	return this->value == other_val->value;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Variable\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Variable::Variable(std::string name_, Basic_Term* parent_)
	:Basic_Term(parent_), name(name_)
{
	LOG_C("baue Variable: " << name_);
}

Variable::Variable(const Variable& source, Basic_Term* parent_)
	: Basic_Term(parent_), name(source.name)
{
	LOG_C("kopiere Variable: " << source);
}

Variable::~Variable()
{
	LOG_C("loesche Variable: " << *this);
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

State Variable::get_state_intern() const
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
	throw XTermCouldNotBeEvaluated("variable " + this->name + " not part of list with matching values");
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
		LOG_P("nicht matched variable: " << *this << " =/= " << *pattern);
		return nullptr;
	}
}

bool Variable::operator<(const Basic_Term& other) const
{
	if (this->get_state_intern() != other.get_state_intern()) {
		return this->get_state_intern() < other.get_state_intern();
	}
	else {
		const Variable* other_var = static_cast<const Variable*>(&other);
		return this->name < other_var->name;
	}
}

bool Variable::operator==(const Basic_Term& other) const
{
	LOG_P(" vergleiche  " << *this << " und " << other);
	switch (other.get_state_intern()) {
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

