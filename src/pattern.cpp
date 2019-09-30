
#include "pattern.h"

using namespace bmath::intern;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Pattern_Variable\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Pattern_Variable::Pattern_Variable(std::string_view name_, Basic_Term* parent_)
	:Basic_Term(parent_), name(name_), pattern_value(nullptr)
{
}

Pattern_Variable::~Pattern_Variable()
{
	//pattern_value is not owner -> nothing has to be deleted
}

void Pattern_Variable::to_str(std::string& str) const
{
	str.push_back('{');
	str.append(this->name);
	if (this->pattern_value != nullptr) {
		str.push_back(',');
		this->pattern_value->to_str(str);
	}
	str.push_back('}');
}

void bmath::intern::Pattern_Variable::to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const
{
	std::string new_line(dist_root * 5, ' ');	//building string with spaces matching dept of this
	this->to_str(new_line);

	tree_lines.push_back(std::move(new_line));
	append_last_line(tree_lines, line_prefix);
}

State Pattern_Variable::get_state() const
{
	return pattern_variable;
}

Vals_Combined Pattern_Variable::combine_values()
{
	std::cout << "Error: pattern_variable used instead of variable!\n";
	return Vals_Combined{ false, 0 };
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

void Pattern_Variable::list_subterms(std::list<Basic_Term*>& subterms, State listed_state) const
{
	if (listed_state == variable) {
		subterms.push_back(const_cast<Pattern_Variable*>(this));
	}
}

void Pattern_Variable::sort()
{
	//nothing to be done here
}

Basic_Term** Pattern_Variable::match_intern(Basic_Term* pattern, std::list<Pattern_Variable*>& pattern_var_adresses, Basic_Term** storage_key)
{
	std::cout << "Error: did not expect Pattern_Variable calling match_intern()\n";
	return nullptr;
}

bool Pattern_Variable::operator<(const Basic_Term& other) const
{
	if (this->get_state() != other.get_state()) {
		return this->get_state() < other.get_state();
	}
	else {
		const Pattern_Variable* other_var = static_cast<const Pattern_Variable*>(&other);
		return this->name < other_var->name;
	}
}

bool Pattern_Variable::operator==(const Basic_Term& other) const
{
	if (this->pattern_value == nullptr) {
		this->pattern_value = const_cast<Basic_Term*>(&other);
		return true;
	}
	else {
		return *(this->pattern_value) == other;
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
	if (term_ptr != nullptr) {
		std::cout << "Error: Pattern_Term has already been build.\n";
		return;
	}
	preprocess_str(name);
	this->term_ptr = build_pattern_subterm({ name.data(), name.length() }, nullptr, var_adresses);
}

Pattern::Pattern_Term::~Pattern_Term()
{
	delete this->term_ptr;
}

Basic_Term* Pattern::Pattern_Term::copy(Basic_Term* parent_)
{
	return copy_subterm(this->term_ptr, parent_);
}

Pattern::Pattern(const char* original_, const char* changed_)
	:var_adresses(), original(), changed()
{
	original.build(original_, this->var_adresses);
	changed.build(changed_, this->var_adresses);
}

std::string Pattern::print()
{
	std::string str;
	this->original.term_ptr->to_str(str);
	str.append(" -> ");
	this->changed.term_ptr->to_str(str);
	return str;
}

//rules to simplify terms (left string -> right string)
const std::vector<Pattern*> Pattern::patterns = {
	new Pattern("sin(x)^2+cos(x)^2", "1"),
	new Pattern("a*c+a*b", "a*(c+b)"),
	new Pattern("ln(a)+ln(b)", "ln(a*b)"),
};


