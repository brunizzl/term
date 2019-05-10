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

	class Term;

	namespace intern {

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

			//if one term holds a pointer to a term of same type both are combinded (if possible)
			virtual void combine_layers();

			//values are added, multiplied, etc.
			virtual Vals_Combined combine_values() = 0;

			//NOCH NICHT AUSGEDACHT
			//returns whether something has changed in the tree
			virtual bool combine_variables();

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

			//works only on sorted terms
			virtual bool operator<(const Basic_Term& other) const = 0;
			virtual bool operator>(const Basic_Term& other) const;
			virtual bool operator<=(const Basic_Term& other) const;
			virtual bool operator>=(const Basic_Term& other) const;

			virtual bool operator==(const Basic_Term& other) const = 0;
			virtual bool operator!=(const Basic_Term& other) const;
		};

	} //namespace intern

	//"head" used to acess and manage actual term (only expected user interface)
	class Term {
	private:
		intern::Basic_Term* term_ptr;		//start of actual term tree
		
	public:
		Term(std::string name_);
		Term(const Term& source);
		~Term();

		void to_str(std::string& str) const;
		bool valid_state() const;

		//performs equivalent transfomations to combine subterms and simplify
		void combine();

		//sets every number to 0 if it is smaller than 10^(pow_of_10_diff_to_set_0) times the average of all numbers
		void cut_rounding_error(int pow_of_10_diff_to_set_0 = 15);

		//adds all variable names in this to list
		void get_var_names(std::list<std::string>& names);

		std::complex<double> evaluate(const std::string name_, std::complex<double> value_) const;
		std::complex<double> evaluate(const std::list<Known_Variable>& known_variables) const;
		void search_and_replace(const std::string& name_, std::complex<double> value_);

		//arithmetic operators
		Term& operator+=(const Term& operand2);
		Term& operator-=(const Term& operand2);
		Term& operator*=(const Term& operand2);
		Term& operator/=(const Term& operand2);
	};

}//namespace bmath

#include "internal_functions.h"
#include "arguments.h"
#include "operations.h"