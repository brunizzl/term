
#include <cassert>
#include <iostream>

#include "pattern.h"
#include "operations.h"
#include "internal_functions.h"

using namespace bmath::intern;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Pattern_Variable\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Pattern_Variable::Pattern_Variable(std::string_view name_, Restriction restriction_)
	:name(name_), matched_term(nullptr), responsible_parent(nullptr), matched_storage_key(nullptr), restriction(restriction_)
{
}

void bmath::intern::Pattern_Variable::set_parent(Basic_Term* new_parent)
{
	this->responsible_parent = new_parent;
}

Pattern_Variable::~Pattern_Variable()
{
	//pattern_value is not owner -> nothing has to be deleted
}

void Pattern_Variable::to_str(std::string& str, int caller_operator_precedence) const
{
	str.push_back('{');
	str.append(this->name);
	if (this->restriction != Restriction::none) {
		str.push_back('!');
		str.append(name_of(this->restriction));
	}
	if (this->matched_term != nullptr) {
		str.push_back(',');
		this->matched_term->to_str(str, operator_precedence(Type::pattern_variable));
	}
	str.push_back('}');
}

void Pattern_Variable::to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const
{
	std::string new_line(dist_root * 5, ' ');	//building string with spaces matching dept of this
	this->to_str(new_line, operator_precedence(Type::undefined));

	tree_lines.push_back(std::move(new_line));
	append_last_line(tree_lines, line_prefix);
}

Type Pattern_Variable::type() const
{
	return Type::pattern_variable;
}

void bmath::intern::Pattern_Variable::combine_layers(Basic_Term*& storage_key)
{
	//nothing to combine layerwise.
}

Vals_Combined Pattern_Variable::combine_values()
{
	assert(false); //transformations can not be evaluated
	return { false, 0 };
}

std::complex<double> Pattern_Variable::evaluate(const std::list<bmath::Known_Variable>& known_variables) const
{
	assert(false); //transformations can not be evaluated
	return 0;
}

void Pattern_Variable::search_and_replace(const std::string& name_, const Basic_Term* value_, Basic_Term*& storage_key)
{
	assert(false);	//patternvariables should never become values. then the pattern would change.
}

void bmath::intern::Pattern_Variable::for_each(std::function<void(Basic_Term* this_ptr, Type this_type)> func)
{
	func(this, Type::pattern_variable);
}

bool bmath::intern::Pattern_Variable::transform(Basic_Term *& storage_key)
{
	assert(false);	//no pattern shall be transformed by other patterns. (yet)
	return false;
}

bool bmath::intern::Pattern_Variable::equal_to_pattern(Basic_Term* pattern, Basic_Term* patterns_parent, Basic_Term *& storage_key)
{
	assert(false);	//patterns should not be compared to patterns.
	return false;
}

void bmath::intern::Pattern_Variable::reset_own_matches(Basic_Term* parent)
{
	if (parent == this->responsible_parent) {
		this->matched_term = nullptr;
		this->responsible_parent = nullptr;
		this->matched_storage_key = nullptr;
	}
}

bool Pattern_Variable::operator<(const Basic_Term& other) const
{
	if (Type::pattern_variable != type_of(other)) {
		return Type::pattern_variable < type_of(other);
	}
	else {
		const Pattern_Variable* other_var = static_cast<const Pattern_Variable*>(&other);
		if (this->matched_term && other_var->matched_term) {
			return *(this->matched_term) < *(other_var->matched_term);
		}
		else if (!this->matched_term && !other_var->matched_term) {
			if ((this->restriction != Restriction::none || other_var->restriction != Restriction::none) 
				&& this->restriction != other_var->restriction) {
				return this->restriction < other_var->restriction;
			}
			else {
				return this->name < other_var->name;
			}
		}
		else {
			return this->matched_term > other_var->matched_term;	//pattern_variables with match are sorted before ones without
		}
	}
}

bool Pattern_Variable::operator==(const Basic_Term& other) const
{
	if (type_of(other) == Type::pattern_variable) {
		const Pattern_Variable* other_var = static_cast<const Pattern_Variable*>(&other);
		if (this->restriction != other_var->restriction) {
			return false;
		}
		else {
			return this->name == other_var->name;
		}
	}
	return false;
}

Basic_Term* bmath::intern::Pattern_Variable::copy_matched_term() const
{
	assert(this->matched_term != nullptr);
	if (*(this->matched_storage_key) != nullptr) {
		*(this->matched_storage_key) = nullptr;	//setting owner of matched_term to nullptr
		return this->matched_term;
	}
	else {
		return copy_subterm(this->matched_term);
	}
}

bool bmath::intern::Pattern_Variable::try_matching(Basic_Term* other, Basic_Term* patterns_parent, Basic_Term *& other_storage_key)
{
	if (this->matched_term == nullptr) {
		if (this->restriction == Restriction::none || fullfills_restr(other, this->restriction)) {
			this->matched_term = other;
			this->responsible_parent = patterns_parent;
			this->matched_storage_key = &other_storage_key;
			return true;
		}
		return false;
	}
	else {
		if (*(this->matched_term) == *other) {
			//if future pattern_variable holds some buffer Basic_Term**, this is where to set it.
			return true;
		}
		return false;
	}
}

bool bmath::intern::Pattern_Variable::is_unmatched() const
{
	return matched_term == nullptr;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Transformation\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Transformation::Transformation(std::string input_, std::string output_)
	:var_adresses(), input(), output()
{
	preprocess_str(input_);
	preprocess_str(output_);
	input = build_pattern_subterm({ input_.data(), input_.length() }, this->var_adresses);
	output = build_pattern_subterm({ output_.data(), output_.length() }, this->var_adresses);
	input->combine_layers(input);
	output->combine_layers(output);
	input->sort();
	output->sort();
}

bmath::intern::Transformation::~Transformation()
{
	delete_pattern(input);
	delete_pattern(output);
	for (auto it : this->var_adresses) {
		delete it;
	}
}

std::string Transformation::print() const
{
	std::string str;
	this->input->to_str(str, operator_precedence(Type::undefined));
	str.append(" -> ");
	this->output->to_str(str, operator_precedence(Type::undefined));
	return str;
}

Basic_Term* bmath::intern::Transformation::copy()
{
	return copy_subterm(this->output);
}

