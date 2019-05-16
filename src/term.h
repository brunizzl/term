#pragma once

#include <string>
#include <sstream>
#include <list>
#include <vector>
#include <set>
#include <iostream>
#include <cmath>
#include <complex>

#include "structs.h"

namespace bmath {
	namespace intern {
		class Pattern_Variable;

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

			//returns kinda true type of term (sum, product, value, etc.)
			virtual State get_state_intern() const = 0;

			//if one sum/product holds a pointer to another sum/product, both get combined into one.
			//IN PLANUNG: WENN NUR NOCH EIN SUMMAND DA, DIE EBENE RAUSNEHMEN
			virtual void combine_layers(Basic_Term*& storage_key);

			//values are added, multiplied, etc.
			virtual Vals_Combined combine_values() = 0;

			//returns {true, whatever it adds up to} if only variables of names of known_variables is present
			//returns {fale, undefined} if more variables are present
			virtual Vals_Combined evaluate(const std::list<Known_Variable>& known_variables) const = 0;

			//searches an replaces all variables with name "name_" with values of value "value_" 
			//storage_key refers to the pointer to this in the object that owns this
			//this function is meant for permanent changes. else use evaluate()
			virtual void search_and_replace(const std::string& name_, std::complex<double> value_, Basic_Term*& storage_key) = 0;

			//true if no subterm holds nullptr, false if otherwise
			virtual bool valid_state() const = 0;

			//used to bring tree in well defined state bevore combine_variables() can be used
			//returns false if class != value, returns true and modifies value if re(value) < 0
			virtual bool re_smaller_than_0();

			//all subterms of requested type get added to the list
			virtual void list_subterms(std::list<Basic_Term*>& subterms, State listed_state) const = 0;

			//needs to be run befor == makes sense to be used
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

		class  Pattern {	//used to pattern match stuff that can then be simplified
		private:
			class Pattern_Term {	//like term, but holds pattern_variables instead of variables.
			public:
				intern::Basic_Term* term_ptr;

				Pattern_Term();
				void build(std::string name, std::list<Pattern_Variable*>& var_adresses);
				~Pattern_Term();

				Basic_Term* copy(Basic_Term* parent_);

				//patterns should not be copied nor changed
				Pattern_Term(const Pattern_Term& source) = delete;
				Pattern_Term(Pattern_Term&& source) = delete;
				Pattern_Term& operator=(const Pattern_Term& source) = delete;
				Pattern_Term& operator=(Pattern_Term&& source) = delete;
			};

		public:
			Pattern(const char* original_, const char* changed_);
			//members:
			std::list<Pattern_Variable*> var_adresses;
			Pattern_Term original;	//pattern to be compared to term opject
			Pattern_Term changed;	//pattern to replace match in term object

			std::string print();
		};


		//used when combining variables to split recurring terms of other summands/ factors for easier matching
		struct ocurrence {
			std::list<Basic_Term*>::iterator list_pos;
			Basic_Term* list_owner;
		};

		struct recurring_term {
			Basic_Term* term;	//points to one instance of recurring term
			std::list<ocurrence> ocurrences;	//holds positiones of all recurring terms in sums/products
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
		Term(const Term& source);
		Term(Term&& source) noexcept;
		Term& operator=(const Term& source);
		Term& operator=(Term&& source) noexcept;
		~Term();

		//returns the tree converted into a string
		std::string to_str() const;

		//true if no subterm holds nullptr, false if otherwise
		bool valid_state() const;

		//performs equivalent transfomations to combine subterms and simplify
		void combine();

		//sets every number to 0 if it is smaller than 10^(pow_of_10_diff_to_set_0) times the average of all numbers
		void cut_rounding_error(int pow_of_10_diff_to_set_0 = 15);

		//adds all variable names in this to list
		std::set<std::string> get_var_names() const;

		std::complex<double> evaluate(const std::string name_, std::complex<double> value_) const;
		std::complex<double> evaluate(const std::list<Known_Variable>& known_variables) const;
		void search_and_replace(const std::string& name_, std::complex<double> value_);

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

#include "internal_functions.h"
#include "arguments.h"
#include "operations.h"