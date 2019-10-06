
#pragma once

#include <string>
#include <array>

#include "structs.h"
#include "term.h"
#include "pattern.h"

namespace bmath {
	namespace intern {

		//deletes spaces and checks parentheses (does not check rules for positioning of operators, aka "+-2" would slip here.)
		void preprocess_str(std::string& str);

		//finds matching open parethesis to the clsd_par in name 
		//(starts search at clsd_par and searches to the left of clsd_par)
		std::size_t find_open_par(std::size_t clsd_par, const std::string_view name);

		//searches characters in name while skipping parts containing parentheses
		std::size_t find_last_of_skip_pars(const std::string_view name, const char* const characters);
		//overload for single character to search
		std::size_t find_last_of_skip_pars(const std::string_view name, const char character);

		//(needs modification if new termtype is added)
		//decides type of next subterm (finds the next operation to split string and create subterm from)
		//can not determine Type::value, as this is assumed to be found by function is_computable()
		Type type_subterm(const std::string_view name, std::size_t& op, Par_Op_Type& type_par_op);

		//(needs modification if new termtype is added)
		//returns pointer to newly build term of right type (u now have ownership of object)
		//if manipulator is != { nullptr, nullptr } and the new subterm is of type value, the return value of build_subterm() will -
		//be nullptr and the value will instead be combined with manipulator.key using function manipulator.func
		//manipulator.func takes *manipulator.key as first argument and the new computed value as second.
		Basic_Term* build_subterm(std::string_view subterm_view, Basic_Term* parent_, Value_Manipulator manipulator = { nullptr, nullptr });

		//(needs modification if new termtype is added)
		//behaves like build_subterm, exept when a variable is build, it checks in variables if this exists already.
		//if so: it will just return the adress of the existing variable. (and variables will become pattern_variables)
		//the behavior regarding manipulator is the same as in build_subterm()
		Basic_Term* build_pattern_subterm(std::string_view subtermstr, Basic_Term* parent_, std::list<Pattern_Variable*>& variables, Value_Manipulator manipulator = { nullptr, nullptr });

		//(needs modification if new termtype is added)
		//returns pointer to newly build term of right type (u now have ownership of object)
		//(needs modification if new termtype is added)
		Basic_Term* copy_subterm(const Basic_Term* source, Basic_Term* parent_);

		//returns visualized tree structure as string
		//offset is amount of spaces seperating tree from left rim of console
		std::string ptr_to_tree(const Basic_Term* term_ptr, std::size_t offset);

		//used in output as tree to visually connect last subterm with rest of tree
		void append_last_line(std::vector<std::string>& tree_lines, char operation);

		//resets all pattern_values to nullptr (needs to be run, before next match can be found)
		void reset_pattern_vars(std::list<Pattern_Variable*>& var_adresses);

		//translates val to a string
		//parent_type is needed, to determine, wheather to put parenteses around the string
		//if inverse == true, -val will be printed
		std::string to_string(std::complex<double> val, Type parent_type, bool inverse = false);

		//returns actual type of obj (sum, product, exponentiation...) but if obj is nullptr returns "undefined"
		Type type(const Basic_Term* obj);

		//returns c string of operator as written in input/output
		constexpr std::string_view name_of(Par_Op_Type op_type);

		//returns operation(argument), operation depends on op_type
		constexpr std::complex<double> value_of(std::complex<double> argument, Par_Op_Type op_type);

		//returns c string of constant as written in input/output
		constexpr std::string_view name_of(Math_Constant constant);

		//returns value of constant
		constexpr std::complex<double> value_of(Math_Constant constant);

		//functions used to complete template arguments of Variadic_Operator<> and to use as (*func) in struct Value_Manipulator
		inline void add(std::complex<double>* first, std::complex<double> second) { *first += second; }
		inline void sub(std::complex<double>* first, std::complex<double> second) { *first -= second; }
		inline void mul(std::complex<double>* first, std::complex<double> second) { *first *= second; }
		inline void div(std::complex<double>* first, std::complex<double> second) { *first /= second; }

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//stack based calculation\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		//determines if name is computable (has no variables, but could contain occurences of Math_Constant) or not
		bool is_computable(std::string_view name);

		//equivalent to type_subterm plus build_subterm bzw. build_pattern_subterm, but computing value directly, not building next subterm
		//expects name to not have any variables (aka to habe attrbute is_computable) and returns evaluation of name
		std::complex<double> compute(std::string_view name);

	} //namespace intern3+121212
} //namespace bmath
