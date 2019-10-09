
#pragma once

#include <string>
#include <string_view>
#include <list>
#include <vector>
#include <complex>

#include "structs.h"
#include "term.h"

namespace bmath {
	namespace intern {

		//intended to be exclusively used by Pattern_Term
		class Pattern_Variable : public Basic_Term
		{
		private:
			Basic_Term* matched_term;
			//parent under whitch the match was found, -
			//and that is responsible for cleaning up the match, if overall no match was made
			Basic_Term* responsible_parent;	

			friend void reset_all_pattern_vars(std::list<Pattern_Variable*>& var_adresses);

		public:
			const std::string name;

			Pattern_Variable(std::string_view name_);
			~Pattern_Variable();

			void to_str(std::string& str, int caller_operator_precedence) const override;
			void to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const override;
			Type type() const override;
			void combine_layers(Basic_Term*& storage_key) override;
			Vals_Combined combine_values() override;
			std::complex<double> evaluate(const std::list<Known_Variable>& known_variables) const override;
			void search_and_replace(const std::string& name_, const Basic_Term* value_, Basic_Term*& storage_key) override;
			void for_each(std::function<void(Basic_Term* this_ptr, Type this_type)> func) override;
			Basic_Term** match_intern(Basic_Term* pattern, std::list<Pattern_Variable*>& pattern_var_adresses, Basic_Term** storage_key) override;
			bool equal_to_pattern(Basic_Term* pattern, Basic_Term* patterns_parent, Basic_Term** storage_key) override;
			bool reset_own_matches(Basic_Term* parent) override;
			bool operator<(const Basic_Term& other) const override;
			bool operator==(const Basic_Term& other) const override;

			//accesses matched_term to copy it, returns copy
			Basic_Term* copy_matched_term() const;

			//does the actual matching attempt. takes same parameters as equal_to_pattern, just with changed roles.
			//to not cause any confusion, the functionality of try_matching() is not implemented in equal_to_pattern()
			bool try_matching(Basic_Term* other, Basic_Term* patterns_parent, Basic_Term** other_storage_key);
		};		


		class  Pattern {	//used to pattern match stuff that can then be simplified
		private:

			class Pattern_Term { //like bmath::Term, but holds pattern_variables instead of variables.
			public:
				Basic_Term* term_ptr;

				Pattern_Term();
				void build(std::string name, std::list<Pattern_Variable*>& var_adresses);
				~Pattern_Term();

				Basic_Term* copy();

				//patterns should not be copied nor changed
				Pattern_Term(const Pattern_Term& source) = delete;
				Pattern_Term(Pattern_Term&& source) = delete;
				Pattern_Term& operator=(const Pattern_Term& source) = delete;
				Pattern_Term& operator=(Pattern_Term&& source) = delete;
			};

			//private constructor -> only member "static const std::vector<Pattern*> patterns" can be build
			Pattern(const char * const original_, const char * const changed_);

		public:
			//members:
			std::list<Pattern_Variable*> var_adresses;
			Pattern_Term original;	//pattern to be compared to term opject
			Pattern_Term changed;	//pattern to replace match in term object

			std::string print() const;

			//only instances of pattern
			static const std::vector<Pattern*> patterns;
		};

	} //namespace intern
} // namespace bmath
