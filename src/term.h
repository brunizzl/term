
#pragma once

#include <string>
#include <list>
#include <vector>
#include <complex>
#include <functional>

#include "structs.h"
#include "exceptions.h"

namespace bmath {
	namespace intern {

		class Pattern_Variable;
		class Pattern;

		class Basic_Term
		{
		public:
			virtual ~Basic_Term();

			//appends this to str. caller_operator_precedence tells callee, whether to put parentheses around string or not
			virtual void to_str(std::string& str, int caller_operator_precedence) const = 0;

			//called in function to_tree() of bmath::term
			//tree_lines holds output line by line, dist_root stores distance from this vertex to root, 
			//line prefix is meant to be set in front of content of this eg. '+' if calling term is sum
			virtual void to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const = 0;

			//returns true type of term (sum, product, value, etc.)
			virtual Type type() const = 0;

			//if one sum/product holds a pointer to another sum/product, both get combined into one,
			//redundant layers are removed (exponentiation with exponent == 1, sum with only one summand...)
			//storage key refers to the actual memory holding the pointer to this. 
			//(aka if you wanna remove the current layer, this is where to reconnect the layers above and below you.)
			virtual void combine_layers(Basic_Term*& storage_key) = 0;

			//values are added, multiplied, etc.
			virtual Vals_Combined combine_values() = 0;

			//returns whatever it adds up to if only variables with names of known_variables are present
			//throws XTermCouldNotBeEvaluated if unknown Variables exist
			virtual std::complex<double> evaluate(const std::list<Known_Variable>& known_variables) const = 0;

			//searches an replaces all variables with name "name_" with values of value "value_" 
			//storage_key refers to the pointer to this in the object that owns this
			//this function is meant for permanent changes. else use evaluate()
			virtual void search_and_replace(const std::string& name_, const Basic_Term* value_, Basic_Term*& storage_key) = 0;

			//executes func first on each subterm, then on itself. 
			//func takes this of caller and the type of therm of caller
			virtual void for_each(std::function<void(Basic_Term* this_ptr, Type this_type)> func) = 0;

			//returns storage position of subterm matching pattern or returns nullptr
			//storage_key is pointer to the pointer to "this" in the object that owns "this" (also return value if match was found)
			//first tries to match this to pattern, then tries to match subterms
			virtual Basic_Term** match_intern(Basic_Term* pattern, std::list<Pattern_Variable*>& pattern_var_adresses, Basic_Term** storage_key) = 0;

			//called by match_intern to test if this is equal to pattern.
			//only differs from operator== in its ability to modify the matched pattern_variables
			//patterns_parent is the term holding pattern, used to mark whitch term is responsible for matching.
			//(used to set member responsible_parent in pattern_variable if a match was found)
			//(if equal_to_pattern() descends one layer, pattern will become patterns_parent)
			//storage_key is pointer to the space in the term owning this, that holds the pointer to this (as in the function above)
			virtual bool equal_to_pattern(Basic_Term* pattern, Basic_Term* patterns_parent, Basic_Term** storage_key) = 0;

			//called by terms part of a pattern to reset the pattern subterms held by this paticualar part or its subterms
			//returns, whether reset was successful (true) or whether there was another option to match and this is now set (false)
			virtual void reset_own_matches(Basic_Term* parent) = 0;

			//changes order in sums and products beeing subterms of this
			//behaves like std::next_permutation()
			virtual bool next_permutation() = 0;

			//called by match_intern() to test if this (called on pattern) finds match on comp
			//if comp is already matched to this, next_match_in_term() tries to find the next way to match. if there is no other way to match ordered after -
			//the current match, every pattern_variable owned by this or its subterms gets resetted and false is returned.
			//comp is term matched to this (this is pattern / part of pattern) patterns_parent is term owning this (nullptr if function is called on full pattern)

			//soll equal_to_pattern, reset_own_matches und next_permutation ablösen
			//muss noch inplementiert werden.
			virtual bool next_match_in_term(const Basic_Term* const comp, Basic_Term* patterns_parent) = 0;

			//works as expected only on sorted terms
			virtual bool operator<(const Basic_Term& other) const = 0;

			virtual bool operator==(const Basic_Term& other) const = 0;
			bool operator!=(const Basic_Term& other) const;

			//these last functions all utilize for_each() -
			//to circumvent declaring each of them as virtual anf forcing to overload and implement each function in each term.

			//needs to be run before == makes sense to be used (sorts sum and product lists)
			void sort();

			//all subterms of requested type get added to the list
			std::list<Basic_Term*> list_subterms(Type requested_type);
		};

	} //namespace intern

	//"head" used to acess and manage actual term (only expected user interface)
	class Term
	{
	private:
		intern::Basic_Term* term_ptr;		//start of actual term tree

		//compares pattern_original to term. 
		//if match was found, it is replaced by pattern.changed and (true) is returned.
		//if no match is found, (false) is returned.
		bool match_and_transform(intern::Pattern& pattern);

		void combine_values();
		
	public:
		Term();
		Term(std::string name_);
		Term(std::complex<double> val);
		Term(const Term& source);
		Term(Term&& source) noexcept;
		Term& operator=(const Term& source);
		Term& operator=(Term&& source) noexcept;
		~Term();

		//returns a string similar to the construction input, only now build out of the terms.
		std::string to_str() const;

		//returns visualized tree structure as string
		//offset is amount of spaces seperating tree from left rim of console
		std::string to_tree(std::size_t offset = 0) const;

		//performs equivalent transfomations to combine subterms and simplify
		//tries to match every pattern among other things.
		void combine();

		//sets every number to 0 if it is smaller than 10^(pow_of_10_diff_to_set_0) times the quadratic average of all numbers
		//warning: may not round as expected, as there are some more values 1 and -1, as one sees in the printed string. (use to_tree() to check where)
		void cut_rounding_error(int pow_of_10_diff_to_set_0 = 15);

		//adds all variable names in this to list
		//if one variable appears n times, its name will occur n times in returned list.
		std::list<std::string> get_var_names() const;

		std::complex<double> evaluate(const std::string& name_, std::complex<double> value_) const;
		std::complex<double> evaluate(const std::list<Known_Variable>& known_variables) const;
		void search_and_replace(const std::string& name_, std::complex<double> value_);
		void search_and_replace(const std::string& name_, const bmath::Term& value_);

		//arithmetic operators
		Term& operator+=(const Term& operand2);
		Term& operator-=(const Term& operand2);
		Term& operator*=(const Term& operand2);
		Term& operator/=(const Term& operand2);

		Term  operator+(const Term& operand2) const;
		Term  operator-(const Term& operand2) const;
		Term  operator*(const Term& operand2) const;
		Term  operator/(const Term& operand2) const;
	};

}//namespace bmath

//allows ostream to output Terms
std::ostream& operator<<(std::ostream& stream, const bmath::Term& term);

//allows ostream to output Basic_Terms (maybe only useful for debugging?)
std::ostream& operator<<(std::ostream& stream, const bmath::intern::Basic_Term& term);

