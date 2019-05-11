#pragma once

#include <map>
#include <array>
#include "term.h"

namespace bmath {
	namespace intern {

		//"original" will be replaced by "changed"
		//var_adresses holds adresses of Pattern_Variables in original (used to reset pattern_values after failed matching attemt)
		struct  Pattern {
			Term original;
			Term changed;
			std::list<Basic_Term*> var_adresses;
		};

		//compares pattern_original to term.
		//if a match is found, function returns the matching subterm in term.
		//if pattern is sum and matching sum in term has more summands then pattern,
		//a new sum is constructed in term were summands used to be. the matched summands become summands of new sum. (same with products)
		//example:	pattern "a^2+b^2" is matched in term "var^2+sin(x)^2+3". term then is changed to "(var^2+sin(x)^2)+3"
		//this is nessesary, as the match will be replaced. 
		Basic_Term* match(const Pattern& pattern, Term& term);

		void reset_pattern_vars(std::list<Basic_Term*>& var_adresses);

	} //intern

	//needs to be run if this file is included, before terms can be constructed properly (abd before pattern matching can be used)
	void pattern_initialize();

} //bmath