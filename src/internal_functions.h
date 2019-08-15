#pragma once

#include <string>
#include <array>

#include "structs.h"
#include "term.h"
#include "pattern.h"

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
		Basic_Term* build_subterm(std::string& subtermstr, Basic_Term* parent_);

		//behaves like build_subterm, exept when a variable is build, it checks in variables if this exists already.
		//if so: it will just return the adress of the existing variable. (and variables will become pattern_variables)
		//(needs modification if new termtype is added)
		Basic_Term* build_pattern_subterm(std::string& subtermstr, Basic_Term* parent_, std::list<Pattern_Variable*>& variables);

		//returns pointer to newly build term of right type (u now have ownership of object)
		//(needs modification if new termtype is added)
		Basic_Term* copy_subterm(const Basic_Term* source, Basic_Term* parent_);

		//decides type of next subterm (finds the next operation to split string and create subterm from)
		//(needs modification if new termtype is added)
		State type_subterm(const std::string& name, const std::vector<Pos_Pars>& pars, std::size_t& op, Par_Op_State& type_par_op);

		//deletes spaces and checks parentheses
		bool preprocess_str(std::string& str);

		//returns actual type of obj (sum, product, exponentiation...) but if obj is nullptr returns s_undefined
		State state(const Basic_Term* obj);

		//needs to be run before combine_variables() makes sense to run
		//products with negative factor get wrapped in a sum, exponentiations with negative exponents into a product
		//DERZEIT UNGENUTZT (soll sowieso obszolet werden, weil summe und produkt noch operanden umkehroperationen verlieren)
		Basic_Term* standardize_structure(Basic_Term* obj);

		//used in output as tree to visually connect new subterm with rest of tree
		void append_last_line(std::vector<std::string>& tree_lines, char operation);

		void reset_pattern_vars(std::list<Pattern_Variable*>& var_adresses);

	} //namespace intern
} //namespace bmath

//allows ostream to output terms
std::ostream& operator<<(std::ostream& stream, const bmath::Term& term);

//allows ostream to output Basic_Terms
std::ostream& operator<<(std::ostream& stream, const bmath::intern::Basic_Term& term);
