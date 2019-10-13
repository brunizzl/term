
#include <cassert>
#include <iostream>

#include "pattern.h"
#include "internal_functions.h"

using namespace bmath::intern;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Pattern_Variable\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Pattern_Variable::Pattern_Variable(std::string_view name_)
	:name(name_), matched_term(nullptr), responsible_parent(nullptr), matched_storage_key(nullptr)
{
}

Pattern_Variable::~Pattern_Variable()
{
	//pattern_value is not owner -> nothing has to be deleted
}

void Pattern_Variable::to_str(std::string& str, int caller_operator_precedence) const
{
	str.push_back('{');
	str.append(this->name);
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
	assert(false); //patterns can not be evaluated
	return { false, 0 };
}

std::complex<double> Pattern_Variable::evaluate(const std::list<bmath::Known_Variable>& known_variables) const
{
	assert(false); //patterns can not be evaluated
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

Basic_Term** Pattern_Variable::match_intern(Basic_Term* pattern, std::list<Pattern_Variable*>& pattern_var_adresses, Basic_Term** storage_key)
{
	assert(false);	//pattern never tries to match to pattern u dummie
	return nullptr;
}

bool bmath::intern::Pattern_Variable::equal_to_pattern(Basic_Term* pattern, Basic_Term* patterns_parent, Basic_Term** storage_key)
{
	assert(false);	//u should call try_matching, because otherwise u did something wrong.
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
			return this->name < other_var->name;
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
		return this->name == other_var->name;
	}
	return false;
}

Basic_Term* bmath::intern::Pattern_Variable::copy_matched_term() const
{
	assert(this->matched_term != nullptr);
	if (this->matched_storage_key != nullptr) {
		*(this->matched_storage_key) = nullptr;
		return this->matched_term;
	}
	else {
		return copy_subterm(this->matched_term);
	}
}

bool bmath::intern::Pattern_Variable::try_matching(Basic_Term* other, Basic_Term* patterns_parent, Basic_Term** other_storage_key)
{
	if (this->matched_term == nullptr) {
		this->matched_term = other;
		this->responsible_parent = patterns_parent;
		return true;
	}
	else {
		if (*(this->matched_term) == *other) {
			//if future pattern_variable holds some buffer Basic_Term**, this is where to set it.
			return true;
		}
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Pattern\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Pattern::Pattern_Term::Pattern_Term()
	:term_ptr(nullptr)
{
}

void Pattern::Pattern_Term::build(std::string name, std::list<Pattern_Variable*>& var_adresses)
{
	assert(term_ptr == nullptr);
	preprocess_str(name);
	this->term_ptr = build_pattern_subterm({ name.data(), name.length() }, var_adresses);
	this->term_ptr->combine_layers(this->term_ptr);
	this->term_ptr->sort();
}

Pattern::Pattern_Term::~Pattern_Term()
{
	delete this->term_ptr;
}

Basic_Term* Pattern::Pattern_Term::copy()
{
	return copy_subterm(this->term_ptr);
}

Pattern::Pattern(const char * const original_, const char * const changed_)
	:var_adresses(), original(), changed()
{
	original.build({ original_ }, this->var_adresses);
	changed.build({ changed_ }, this->var_adresses);
}

std::string Pattern::print() const
{
	std::string str;
	this->original.term_ptr->to_str(str, operator_precedence(Type::undefined));
	str.append(" -> ");
	this->changed.term_ptr->to_str(str, operator_precedence(Type::undefined));
	return str;
}

//rules to simplify terms (left string -> right string)
const std::vector<Pattern*> Pattern::patterns = {
	new Pattern("ln(a)+ln(b)", "ln(a*b)"),
	new Pattern("sin(x)^2+cos(x)^2", "1"),
	new Pattern("(a^b)^c", "a^(b*c)"),

	new Pattern("a*b+a*c", "a*(b+c)"),
	new Pattern("a*b+a", "a*(b+1)"),
	new Pattern("a+a", "a*2"),
	new Pattern("a^b*a^c", "a^(b+c)"),
	new Pattern("a^b*a", "a^(b+1)"),
	new Pattern("a*a", "a^2"),
};


