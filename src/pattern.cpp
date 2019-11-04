
#include <cassert>
#include <iostream>

#include "pattern.h"
#include "operations.h"
#include "internal_functions.h"

using namespace bmath::intern;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Pattern_Variable\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Pattern_Variable::Pattern_Variable(std::string_view name_, Type type_)
	:name(name_), matched_term(nullptr), matched_storage_key(nullptr), type_restriction(type_)
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

Basic_Term** Pattern_Variable::match_intern(Basic_Term* pattern, std::list<Pattern_Variable*>& pattern_var_adresses, Basic_Term** storage_key)
{
	assert(false);	//pattern never tries to match to pattern u dummie
	return nullptr;
}

bool bmath::intern::Pattern_Variable::equal_to_pattern(Basic_Term* pattern, Basic_Term** storage_key)
{
	assert(false);	//u should call try_matching, because otherwise u did something wrong.
	return false;
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

bool bmath::intern::Pattern_Variable::try_matching(Basic_Term* other, Basic_Term** other_storage_key)
{
	if (this->matched_term == nullptr) {
		this->matched_term = other;
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
//Pattern_Term\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Pattern_Term::Pattern_Term()
	:term_ptr(nullptr)
{
}

void Pattern_Term::build(std::string name, std::list<Pattern_Variable*>& var_adresses)
{
	assert(term_ptr == nullptr);
	preprocess_str(name);
	this->term_ptr = build_pattern_subterm({ name.data(), name.length() }, var_adresses);
	this->term_ptr->combine_layers(this->term_ptr);
	this->term_ptr->sort();
}

Pattern_Term::~Pattern_Term()
{
	//due to the fact, that Pattern_Term break the Tree structure (one pattern_variable might be owned by multiple parents) -
	//we can not use the destructor of basic_term
	std::list<Basic_Term*> subterms;
	this->term_ptr->for_each([&](Basic_Term* this_ptr, Type this_type) { 
		if (this_type != Type::pattern_variable) subterms.push_back(this_ptr);	//pattern_variables are not deleted by this destructor

		switch (this_type) {
		case Type::par_operator:
			static_cast<Par_Operator*>(this_ptr)->argument = nullptr;
		case Type::exponentiation:
			static_cast<Exponentiation*>(this_ptr)->base = nullptr;
			static_cast<Exponentiation*>(this_ptr)->expo = nullptr;
		case Type::sum:
			static_cast<Sum*>(this_ptr)->operands.clear();
		case Type::product:
			static_cast<Product*>(this_ptr)->operands.clear();
		}
	});
	for (auto subterm : subterms) {
		delete subterm;
	}
}

Basic_Term* Pattern_Term::copy()
{
	return copy_subterm(this->term_ptr);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Transformation\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Transformation::Transformation(std::string input_, std::string output_)
	:var_adresses(), input(), output()
{
	input.build(std::move(input_), this->var_adresses);
	output.build(std::move(output_), this->var_adresses);
}

bmath::intern::Transformation::~Transformation()
{
	for (auto it : this->var_adresses) {
		delete it;
	}
}

std::string Transformation::print() const
{
	std::string str;
	this->input.term_ptr->to_str(str, operator_precedence(Type::undefined));
	str.append(" -> ");
	this->output.term_ptr->to_str(str, operator_precedence(Type::undefined));
	return str;
}

//rules to simplify terms (left string -> right string)
const std::vector<Transformation*> Transformation::transformations = {
	new Transformation("ln(a)+ln(b)", "ln(a*b)"),
	new Transformation("ln(a)-ln(b)", "ln(a/b)"),
	new Transformation("sin(x)^2+cos(x)^2", "1"),
	new Transformation("(a^b)^c", "a^(b*c)"),

	new Transformation("a*b+a*c", "a*(b+c)"),
	new Transformation("b*a+b*c", "b*(a+c)"),
	new Transformation("a*b+a", "a*(b+1)"),
	new Transformation("a+a", "a*2"),
	new Transformation("a^b*a^c", "a^(b+c)"),
	new Transformation("a^b*a", "a^(b+1)"),
	new Transformation("a*a", "a^2"),
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Pattern\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bmath::intern::Pattern::Pattern(std::string name_)
	:var_adresses(), term()
{
	term.build(std::move(name_), this->var_adresses);
}

bmath::intern::Pattern::~Pattern()
{
	for (auto it : this->var_adresses) {
		delete it;
	}
}

std::string bmath::intern::Pattern::print() const
{
	std::string str;
	this->term.term_ptr->to_str(str, operator_precedence(Type::undefined));
	return str;
}
