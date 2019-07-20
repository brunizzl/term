#pragma once

#include <string>
#include <list>

#include "internal_functions.h"
#include "structs.h"
#include "term.h"

namespace bmath {
	namespace intern {

		//intended to be exclusively used by Pattern_Term
		class Pattern_Variable : public Basic_Term
		{
		private:
			Pattern_Variable(std::string name_, Basic_Term* parent_);

			//access to constructor:
			friend Basic_Term* build_pattern_subterm(std::string& subtermstr, Basic_Term* parent_, std::list<Pattern_Variable*>& variables);
		public:
			std::string name;
			mutable Basic_Term* pattern_value;	//mutable to allow operator== to stay const

			~Pattern_Variable();

			void to_str(std::string& str) const override;
			State get_state_intern() const override;
			Vals_Combined combine_values() override;
			std::complex<double> evaluate(const std::list<Known_Variable>& known_variables) const override;
			void search_and_replace(const std::string& name_, const Basic_Term* value_, Basic_Term*& storage_key) override;
			void list_subterms(std::list<Basic_Term*>& subterms, State listed_state) const override;
			void sort() override;
			Basic_Term** match_intern(Basic_Term* pattern, std::list<Pattern_Variable*>& pattern_var_adresses, Basic_Term** storage_key) override;
			bool operator<(const Basic_Term& other) const override;
			bool operator==(const Basic_Term& other) const override;
		};

		


		class  Pattern {	//used to pattern match stuff that can then be simplified
		private:

			class Pattern_Term { //like bmath::Term, but holds pattern_variables instead of variables.
			public:
				Basic_Term* term_ptr;

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

	} //namespace intern
} // namespace bmath