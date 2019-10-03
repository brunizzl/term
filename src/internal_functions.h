
#pragma once

#include <string>
#include <array>
#include <iostream>

#include "structs.h"
#include "term.h"
#include "pattern.h"

namespace bmath {
	namespace intern {

		//finds matching open parethesis to the clsd_par in name 
		std::size_t find_open_par(std::size_t clsd_par, const std::string_view name);

		//searches characters in name while skipping parts containing parentheses
		std::size_t find_last_of_skip_pars(const std::string_view name, const char* const characters);

		//returns pointer to newly build term of right type (u now have ownership of object)
		Basic_Term* build_subterm(std::string_view subterm_view, Basic_Term* parent_);

		//behaves like build_subterm, exept when a variable is build, it checks in variables if this exists already.
		//if so: it will just return the adress of the existing variable. (and variables will become pattern_variables)
		//(needs modification if new termtype is added)
		Basic_Term* build_pattern_subterm(std::string_view subtermstr, Basic_Term* parent_, std::list<Pattern_Variable*>& variables);

		//returns pointer to newly build term of right type (u now have ownership of object)
		//(needs modification if new termtype is added)
		Basic_Term* copy_subterm(const Basic_Term* source, Basic_Term* parent_);

		//decides type of next subterm (finds the next operation to split string and create subterm from)
		//(needs modification if new termtype is added)
		Type type_subterm(const std::string_view name, std::size_t& op, Par_Op_Type& type_par_op);

		//deletes spaces and checks parentheses
		void preprocess_str(std::string& str);

		//returns actual type of obj (sum, product, exponentiation...) but if obj is nullptr returns "undefined"
		Type type(const Basic_Term* obj);

		//used in output as tree to visually connect new subterm with rest of tree
		void append_last_line(std::vector<std::string>& tree_lines, char operation);

		//resets all pattern_values to nullptr (needs to be run, before next match can be found)
		void reset_pattern_vars(std::list<Pattern_Variable*>& var_adresses);

		//returns c string of operator as written in input/output
		const char* const par_op_name(Par_Op_Type op_type);

		//returns operation(argument), operation depends on op_type
		std::complex<double> evaluate_par_op(std::complex<double> argument, Par_Op_Type op_type);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//stack based calculation\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		//says whether name is computable (has no variables) or not
		bool is_computable(std::string_view name);

		//equivalent to type_subterm plus build_subterm bzw. build_pattern_subterm, but computing value directly, not building next subterm
		//expects name to not have any variables (aka to habe attrbute is_computable) and returns evaluation of name
		std::complex<double> compute(std::string_view name);

	} //namespace intern3+121212
} //namespace bmath
