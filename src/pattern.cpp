
#include <cassert>
#include <iostream>

#include "pattern.h"
#include "internal_functions.h"

using namespace bmath::intern;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Pattern_Variable\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Pattern_Variable::Pattern_Variable(std::string_view name_, Basic_Term* parent_)
	:name(name_), matched_term(nullptr)
{
}

Pattern_Variable::~Pattern_Variable()
{
	//pattern_value is not owner -> nothing has to be deleted
}

Basic_Term* bmath::intern::Pattern_Variable::parent() const
{
	return nullptr;	//pattern_variable has multiple parents, none of them is stored in the variable itself.
}

void bmath::intern::Pattern_Variable::set_parent(Basic_Term* new_parent)
{
	//pattern_variable has multiple parents and does not know them.
}

void Pattern_Variable::to_str(std::string& str) const
{
	str.push_back('{');
	str.append(this->name);
	if (this->matched_term != nullptr) {
		str.push_back(',');
		assert(type(this->matched_term) != Type::pattern_variable);
		this->matched_term->to_str(str);
	}
	str.push_back('}');
}

void Pattern_Variable::to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const
{
	std::string new_line(dist_root * 5, ' ');	//building string with spaces matching dept of this
	this->to_str(new_line);

	tree_lines.push_back(std::move(new_line));
	append_last_line(tree_lines, line_prefix);
}

Type Pattern_Variable::get_type() const
{
	return Type::pattern_variable;
}

Vals_Combined Pattern_Variable::combine_values()
{
	std::cout << "Error: pattern_variable used instead of variable!\n";
	return { false, 0 };
}

std::complex<double> Pattern_Variable::evaluate(const std::list<bmath::Known_Variable>& known_variables) const
{
	std::cout << "Error: pattern_variable used instead of variable!\n";
	return 0;
}

void Pattern_Variable::search_and_replace(const std::string& name_, const Basic_Term* value_, Basic_Term*& storage_key)
{
	std::cout << "Error: pattern_variable used instead of variable!\n";
}

void Pattern_Variable::list_subterms(std::list<const Basic_Term*>& subterms, Type listed_type) const
{
	if (listed_type == Type::variable) {
		subterms.push_back(this);
	}
}

void Pattern_Variable::sort()
{
	//nothing to to here
}

Basic_Term** Pattern_Variable::match_intern(Basic_Term* pattern, std::list<Pattern_Variable*>& pattern_var_adresses, Basic_Term** storage_key)
{
	std::cout << "Error: did not expect Pattern_Variable calling match_intern()\n";
	return nullptr;
}

bool Pattern_Variable::operator<(const Basic_Term& other) const
{
	if (this->get_type() != other.get_type()) {
		return this->get_type() < other.get_type();
	}
	else {
		const Pattern_Variable* other_var = static_cast<const Pattern_Variable*>(&other);
		return this->name < other_var->name;
	}
}

bool Pattern_Variable::operator==(const Basic_Term& other) const
{
	if (this->matched_term == nullptr) {
		this->matched_term = const_cast<Basic_Term*>(&other);
		return true;
	}
	else {
		return *(this->matched_term) == other;
	}
}

Basic_Term* bmath::intern::Pattern_Variable::copy_matched_term(Basic_Term* parent_) const
{
	return copy_subterm(this->matched_term, parent_);
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
	if (term_ptr != nullptr) {
		std::cout << "Error: Pattern_Term has already been build.\n";
		return;
	}
	preprocess_str(name);
	this->term_ptr = build_pattern_subterm({ name.data(), name.length() }, nullptr, var_adresses, { nullptr, nullptr });
	this->term_ptr->combine_layers(this->term_ptr);
	this->term_ptr->sort();
}

Pattern::Pattern_Term::~Pattern_Term()
{
	delete this->term_ptr;
}

Basic_Term* Pattern::Pattern_Term::copy(Basic_Term* parent_)
{
	return copy_subterm(this->term_ptr, parent_);
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
	this->original.term_ptr->to_str(str);
	str.append(" -> ");
	this->changed.term_ptr->to_str(str);
	return str;
}

void Pattern::print_all()
{
	for (const auto& pattern : patterns) {
		std::cout << pattern->print() << '\n';
		std::cout << "original :" << ptr_to_tree(pattern->original.term_ptr, 12).erase(0, 10) << '\n';
		std::cout << "changed  :" << ptr_to_tree(pattern->changed.term_ptr, 12).erase(0, 10) << '\n';
		std::cout << "__________________________________________________________\n";
		std::cout << '\n';
	}
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


