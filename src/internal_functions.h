
#pragma once

#include <string>
#include <array>
#include <optional>

#include "structs.h"
#include "term.h"
#include "pattern.h"

namespace bmath {
	namespace intern {

		//deletes spaces, checks parentheses and brackets and does a rudimentary check if two operators directly follow each other eg. "1-+2" would be detected.
		void preprocess_str(std::string& str);

		//finds matching open parethesis to the clsd_par in name 
		//(starts search at clsd_par and searches to the left of clsd_par until it hits the begin of name)
		std::size_t find_open_par(std::size_t clsd_par, const std::string_view name);

		//searches characters in name while skipping parts containing parentheses
		std::size_t find_last_of_skip_pars(const std::string_view name, const char* const characters);
		//overload for single character to search
		std::size_t find_last_of_skip_pars(const std::string_view name, const char character);

		//decides type of next subterm (finds the next operation to split string and create subterm from)
		//can not determine Type::value, as this is assumed to be found by function is_computable()
		Type type_subterm(const std::string_view name, std::size_t& op, Par_Op_Type& type_par_op);

		//returns pointer to newly build term of right type (u now have ownership of object)
		//if manipulator is != { nullptr, nullptr } and the new subterm is of type value, the return value of build_subterm() will -
		//be nullptr and the value will instead be combined with manipulator.key using function manipulator.func
		//manipulator.func takes *manipulator.key as first argument and the new computed value as second.
		Basic_Term* build_subterm(std::string_view subterm_view, Value_Manipulator manipulator = { nullptr, nullptr });

		//behaves like build_subterm, exept when a variable is build, it checks in variables if this name exists already.
		//if so: it will just return the adress of the existing variable. (and variables will become pattern_variables)
		//the behavior regarding manipulator is the same as in build_subterm()
		Basic_Term* build_pattern_subterm(std::string_view subtermstr, std::list<Pattern_Variable*>& variables, Value_Manipulator manipulator = { nullptr, nullptr });

		//returns pointer to newly build term of right type (u now have ownership of object)
		Basic_Term* copy_subterm(const Basic_Term* source);

		//beautifier of Basic_Term->get_type()
		inline Type type_of(const Basic_Term& obj) { return obj.type(); };
		inline Type type_of(const Basic_Term* const obj) { return obj->type(); };

		//returns score to say whether the operator has high precedence (high score) or low precedence over other operators
		int operator_precedence(Type operator_type);

		//returns visualized tree structure as string
		//offset is amount of spaces seperating tree from left rim of console
		std::string ptr_to_tree(const Basic_Term* term_ptr, std::size_t offset);

		//used in output as tree to visually connect last subterm with rest of tree
		void append_last_line(std::vector<std::string>& tree_lines, char operation);

		//resets all pattern_values to nullptr (needs to be run, before next match can be found)
		void reset_all_pattern_vars(std::list<Pattern_Variable*>& var_adresses);

		//returns first iterator of matching type in search, if none is found returns search.end()
		std::list<Basic_Term*>::iterator find_first_of(std::list<Basic_Term*>& search, Type type);

		//used in variadic_operator. returns, whether test_ops has a match for every element in pattern_ops pattern is owner of pattern_ops
		//returns {} if no match was found, returns matched operands of test_ops if match was found
		std::optional<std::list<Basic_Term*>> operands_contain_pattern(std::list<Basic_Term*>& test_ops, std::list<Basic_Term*>& pattern_ops, Basic_Term* pattern);

		//translates val to a string
		//parent_operator_precedence is needed to determine, wheather to put parenteses around the string
		//if inverse == true, -(val) will be printed
		std::string to_string(std::complex<double> val, int parent_operator_precedence, bool inverse = false);

		//returns string_view of operator as written in input/output
		constexpr std::string_view name_of(Par_Op_Type op_type);

		//returns operation(argument), operation depends on op_type
		constexpr std::complex<double> value_of(std::complex<double> argument, Par_Op_Type op_type);

		//returns c string of constant as written in input/output
		constexpr std::string_view name_of(Math_Constant constant);

		//returns value of constant
		constexpr std::complex<double> value_of(Math_Constant constant);

		//functions used to complete template arguments of Variadic_Operator<> and to use as (*func) in struct Value_Manipulator
		inline void add(std::complex<double>* const first, std::complex<double> second) { *first += second; }
		inline void sub(std::complex<double>* const first, std::complex<double> second) { *first -= second; }
		inline void mul(std::complex<double>* const first, std::complex<double> second) { *first *= second; }
		inline void div(std::complex<double>* const first, std::complex<double> second) { *first /= second; }

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//stack based calculation\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		//determines if name is computable (has no variables, but could contain occurences of Math_Constant) or not
		bool is_computable(std::string_view name);

		//equivalent to type_subterm() plus build_subterm() bzw. build_pattern_subterm(), but computing value directly, not building next subterm
		//expects name to not have any variables (aka to habe attribute is_computable) and returns evaluation of name
		std::complex<double> compute(std::string_view name);

	} //namespace intern3+121212
} //namespace bmath
