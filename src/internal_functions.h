
#pragma once

#include <string>
#include <array>

#include "structs.h"
#include "term.h"
#include "pattern.h"

namespace bmath {
	namespace intern {

		//finds matching closed parethesis to the open_par in name 
		std::size_t find_closed_par(std::size_t open_par, const std::string_view name);

		//puts the parts of name not enclosed in parentheses in exposed (including the bordering open (not closed) parentheses)
		//example: name "(2+a)*4/sin(x)" would push the view "*4/sin(" in exposed. 
		//the open parenthesis is needed to detect par_operator (as sin) in exposed parts 
		void find_exposed_parts(std::string_view name, std::vector<std::string_view>& exposed);

		//searches characters in views of name, returns position in name
		std::size_t find_last_of_in_views(const std::string_view name, const std::vector<std::string_view>& views, const char* characters);

		//searches searchstr in views of name, returns position in name (modified rfind of std::string_view)
		std::size_t rfind_in_views(const std::string_view name, const std::vector<std::string_view>& views, const char* searchstr);

		//removes views no longer part of name
		void update_views(const std::string_view name, std::vector<std::string_view>& views);

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
		State type_subterm(const std::string_view name, const std::vector<std::string_view>& exposed_parts, std::size_t& op, Par_Op_State& type_par_op);

		//deletes spaces and checks parentheses
		void preprocess_str(std::string& str);

		//returns actual type of obj (sum, product, exponentiation...) but if obj is nullptr returns s_undefined
		State state(const Basic_Term* obj);

		//used in output as tree to visually connect new subterm with rest of tree
		void append_last_line(std::vector<std::string>& tree_lines, char operation);

		//resets all pattern_values to nullptr (needs to be run, before next match can be found)
		void reset_pattern_vars(std::list<Pattern_Variable*>& var_adresses);

	} //namespace intern
} //namespace bmath

//allows ostream to output Terms
std::ostream& operator<<(std::ostream& stream, const bmath::Term& term);

//allows ostream to output Basic_Terms
std::ostream& operator<<(std::ostream& stream, const bmath::intern::Basic_Term& term);
