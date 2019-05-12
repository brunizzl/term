#pragma once

#include <string>
#include <array>
#include "term.h"
#include "structs.h"

namespace bmath {
	namespace intern {

		//finds matching closed parethesis to the open_par in name 
		std::size_t find_closed_par(std::size_t open_par, const std::string& name);

		//finds the highest level of parentheses in name
		void find_pars(const std::string& name, std::vector<Pos_Pars>& pars);

		//skips parentheses, else finds_last_of characters in name like std::string function
		std::size_t find_last_of_skip_pars(const std::string& name, const char* characters, const std::vector<Pos_Pars>& pars);

		//skips parentheses, else rfind in name like std::string function
		std::size_t rfind_skip_pars(const std::string& name, const char* searchstr, const std::vector<Pos_Pars>& pars);

		//deletes parentheses of parentheses list, which start after the end of the name
		void del_pars_after(std::vector<Pos_Pars>& pars, const std::string& name);

		//returns pointer to newly build term of right type (u now have ownership of object)
		//(needs modification if new termtype is added)
		Basic_Term* build_subterm(std::string& subtermstr, Basic_Term* parent_);

		//behaves like build_subterm, exept when a variable is build, it checks in variables if this exists already.
		//if so: it will just return the adress of the existing variable. (and variables will become pattern_variables)
		//(needs modification if new termtype is added)
		Basic_Term* build_pattern_subterm(std::string& subtermstr, Basic_Term* parent_, std::list<Pattern_Variable*>& variables);

		//returns pointer to newly build term of right type (u now have ownership of object)
		//(needs modification if new termtype is added)
		Basic_Term* copy_subterm(const Basic_Term* source, Basic_Term* parent_);

		//decides type of next subterm (finds the next operation to split)
		//(needs modification if new termtype is added)
		State type_subterm(const std::string& name, const std::vector<Pos_Pars>& pars, std::size_t& op, Par_Op_State& type_par_op);

		//returns c string of operator as written in input/output
		const char* op_name(Par_Op_State op_state);

		//so far: deletes spaces and counts parentheses
		bool preprocess_str(std::string& str);

		//same as get_state_intern(), but returns "s_undefined" if obj == nullptr
		State get_state(const Basic_Term* obj);

		//needs to be run before combine_variables() makes sense to run
		//products with negative factor get wrapped in a sum, exponentiations with negative exponents into a product
		//DERZEIT UNGENUTZT (soll sowieso obszolet werden, weil summe und produkt noch operanden umkehroperationen verlieren)
		Basic_Term* standardize_structure(Basic_Term* obj);

		//compares pattern_original to term.
		//if a match is found, function returns the matching subterm in term.
		//if pattern is sum and matching sum in term has more summands then pattern,
		//a new sum is constructed in term were summands used to be. the matched summands become summands of new sum. (same with products)
		//example:	pattern "a^2+b^2" is matched in term "var^2+sin(x)^2+3". term then is changed to "(var^2+sin(x)^2)+3"
		//this is nessesary, as the match will be replaced. 
		Basic_Term* match(const Pattern& pattern, Term& term);

		void reset_pattern_vars(std::list<Basic_Term*>& var_adresses);

	} //namespace intern
} //namespace bmath

//allows ostream to output terms
std::ostream& operator<<(std::ostream& stream, const bmath::Term& term);

//allows ostream to output Basic_Terms
std::ostream& operator<<(std::ostream& stream, const bmath::intern::Basic_Term& term);
