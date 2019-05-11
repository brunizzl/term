#include "Rule_Patterns.h"

using namespace bmath::intern;

//do not change this variable - call pattern_initialize()
bool constructing_patterns(true);

//define rules here
std::array<Pattern, 1> patterns = {
	{ "b*a + c*a", "(b+c) * a" },
};

void bmath::pattern_initialize()
{
	constructing_patterns = false;
	for (auto& pattern : patterns) {
		pattern.original.term_ptr->list_subterms(pattern.var_adresses, s_pattern_var);
	}
}

Basic_Term* bmath::intern::match(const Pattern& pattern, Term& term)
{
	//MUSS NOCH GEBAUT WERDEN SPACKO
	return nullptr;
}

void bmath::intern::reset_pattern_vars(std::list<Basic_Term*>& var_adresses)
{
	for (auto& pattern_var : var_adresses) {
		static_cast<Pattern_Variable*>(pattern_var)->pattern_value = nullptr;
	}
}
