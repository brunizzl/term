
#pragma once

#include <string>
#include <list>
#include <vector>
#include <complex>

#include "structs.h"
#include "exceptions.h"

namespace bmath {
	namespace intern {

		class Pattern_Variable;
		class Pattern;

		class Basic_Term
		{
		public:
			//pointer to whoever owns this (basic_term does not own parent)
			Basic_Term* parent;

			Basic_Term(Basic_Term* parent_);
			Basic_Term(const Basic_Term& source);
			virtual ~Basic_Term();

			//appends this to str
			virtual void to_str(std::string& str) const = 0;

			//is needed in to_str to allow product to print "/" and sum to print "-"
			//only product and exponentiation implement this function other than basic_term.
			virtual bool expect_inverse_str() const;

			//called in function to_tree() of bmath::term
			// tree_lines holds output line by line, dist_root stores distance from this vertex to root, 
			// line prefix is meant to be set in front of content of this
			virtual void to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const = 0;

			//returns kinda true type of term (sum, product, value, etc.)
			virtual Type get_type() const = 0;

			//if one sum/product holds a pointer to another sum/product, both get combined into one,
			//redundant layers are removed (exponentiation with exponent == 1, sum with only one summand...)
			//storage key refers to the actual memory holding the pointer to this. 
			//(aka if you wanna remove the current layer, this is where to reconnect the layers above and below you.
			virtual void combine_layers(Basic_Term*& storage_key);

			//values are added, multiplied, etc.
			virtual Vals_Combined combine_values() = 0;

			//returns whatever it adds up to if only variables with names of known_variables are present
			//throws XTermCouldNotBeEvaluated if unknown Variables exist
			virtual std::complex<double> evaluate(const std::list<Known_Variable>& known_variables) const = 0;

			//searches an replaces all variables with name "name_" with values of value "value_" 
			//storage_key refers to the pointer to this in the object that owns this
			//this function is meant for permanent changes. else use evaluate()
			virtual void search_and_replace(const std::string& name_, const Basic_Term* value_, Basic_Term*& storage_key) = 0;

			//all subterms of requested type get added to the list
			virtual void list_subterms(std::list<Basic_Term*>& subterms, Type listed_type) const = 0;

			//needs to be run before == makes sense to be used
			virtual void sort() = 0;

			//returns subterm matching pattern or nullptr (basically operator==, but with pattern matching) 
			//storage_key is pointer to the pointer to "this" in the object that owns "this"
			//only differs from operator== on first layer, as it calls operator== itself.
			//first tries to match this to pattern, then tries to match subterms
			virtual Basic_Term** match_intern(Basic_Term* pattern, std::list<Pattern_Variable*>& pattern_var_adresses, Basic_Term** storage_key) = 0;

			//works only on sorted terms
			virtual bool operator<(const Basic_Term& other) const = 0;

			virtual bool operator==(const Basic_Term& other) const = 0;
			virtual bool operator!=(const Basic_Term& other) const;
		};

	} //namespace intern

	//"head" used to acess and manage actual term (only expected user interface)
	class Term {
	private:
		intern::Basic_Term* term_ptr;		//start of actual term tree

		//compares pattern_original to term.
		//if a match is found, function returns the matching subterm in term.
		//if pattern is sum and matching sum in term has more summands then pattern,
		//a new sum is constructed in term were summands used to be. the matched summands become summands of new sum. (same with products)
		//example:	pattern "a^2+b^2" is matched in term "var^2+sin(x)^2+3". term then is changed to "(var^2+sin(x)^2)+3"
		//this is nessesary, as the match will be replaced. 
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

		//returns the tree converted into a string
		std::string to_str() const;

		//returns visualized tree structure as string
		//offset is amount of spaces seperating tree from left rim of console
		std::string to_tree(std::size_t offset = 0) const;

		//performs equivalent transfomations to combine subterms and simplify
		void combine();

		//sets every number to 0 if it is smaller than 10^(pow_of_10_diff_to_set_0) times the quadratic average of all numbers
		void cut_rounding_error(int pow_of_10_diff_to_set_0 = 15);

		//adds all variable names in this to list
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

//allows ostream to output Basic_Terms
std::ostream& operator<<(std::ostream& stream, const bmath::intern::Basic_Term& term);

#include "arguments.h"
#include "operations.h"
#include "pattern.h"
