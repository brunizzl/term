#include "pattern.h"

using namespace bmath::intern;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Pattern_Variable\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Pattern_Variable::Pattern_Variable(std::string name_, Basic_Term* parent_)
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

State Pattern_Variable::get_state_intern() const
{
	return s_pattern_variable;
}

Vals_Combined Pattern_Variable::combine_values()
{
	std::cout << "Error: pattern_variable used instead of variable!\n";
	std::cout << "(try running bmath::pattern_initialize() function first.)\n";
	return Vals_Combined();
}

std::complex<double> Pattern_Variable::evaluate(const std::list<bmath::Known_Variable>& known_variables) const
{
	std::cout << "Error: pattern_variable used instead of variable!\n";
	std::cout << "(try running bmath::pattern_initialize() function first.)\n";
	return 0;
}

void Pattern_Variable::search_and_replace(const std::string& name_, const Basic_Term* value_, Basic_Term*& storage_key)
{
	std::cout << "Error: pattern_variable used instead of variable!\n";
	std::cout << "(try running bmath::pattern_initialize() function first.)\n";
}

void Pattern_Variable::list_subterms(std::list<Basic_Term*>& subterms, State listed_state) const
{
	if (listed_state == s_variable) {
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
	if (this->get_state_intern() != other.get_state_intern()) {
		return this->get_state_intern() < other.get_state_intern();
	}
	else {
		const Pattern_Variable* other_var = static_cast<const Pattern_Variable*>(&other);
		return this->name < other_var->name;
	}
}

bool Pattern_Variable::operator==(const Basic_Term& other) const
{
	LOG_P(" vergleiche  " << *this << " und " << other);
	if (this->pattern_value == nullptr) {
		this->pattern_value = const_cast<Basic_Term*>(&other);
		LOG_P("pattern_var matched mit " << other);
		return true;
	}
	else {
		bool match = *(this->pattern_value) == other;

		LOG_P("pattern_var " << *this << (match ? " === " : " =/= ") << other);
		return match;
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
	if (preprocess_str(name)) {
		this->term_ptr = build_pattern_subterm(name, nullptr, var_adresses);
	}
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