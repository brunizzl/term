
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
			Basic_Term* matched_term;			//term this variable gets compared to (if nullptr, current comparison is true and variable is set)
			Basic_Term** matched_storage_key;	//actual storage position of matched_term (place in parent, where matched_term is stored)
			//WARNING: responsible_parent may not point to a Basic_Term. this member is only a marker.
			void* responsible_parent;			//parent unter which matched_term is set, and which is responsible for resetting matched_term
			Type type_restriction;				//if variable != Type::undefined, this can only match terms of same type

			friend void reset_all_pattern_vars(std::list<Pattern_Variable*>& var_adresses);
			friend Basic_Term* build_pattern_subterm(std::string_view subtermstr, std::list<Pattern_Variable*>& variables, Value_Manipulator manipulator);

			Pattern_Variable(std::string_view name_, Type type_);

		public:
			const std::string name;

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
			bool transform(std::vector<Transformation*>& transforms, Basic_Term** storage_key) override;
			bool equal_to_pattern(Basic_Term* pattern, Basic_Term* patterns_parent, Basic_Term** storage_key) override;
			void reset_own_matches(Basic_Term* parent) override;
			bool operator<(const Basic_Term& other) const override;
			bool operator==(const Basic_Term& other) const override;

			//accesses matched_term to copy it, returns copy
			Basic_Term* copy_matched_term() const;

			//does the actual matching attempt. takes same parameters as equal_to_pattern, just with output roles.
			//to not cause any confusion, the functionality of try_matching() is not implemented in equal_to_pattern()
			bool try_matching(Basic_Term* other, Basic_Term* patterns_parent, Basic_Term** other_storage_key);

			bool is_unmatched() const;
		};	


		class Pattern_Term
		{ //like bmath::Term, but holds pattern_variables instead of variables.
		public:
			Basic_Term* term_ptr;

			Pattern_Term();
			void build(std::string name, std::list<Pattern_Variable*>& var_adresses);
			~Pattern_Term();

			Basic_Term* copy();	//returns copy of this, but the Pattern_Variables are replaced by the terms they matched

			//Pattern_Terms should not be copied nor changed. they only help other terms to change.
			Pattern_Term(const Pattern_Term& source) = delete;
			Pattern_Term(Pattern_Term&& source) = delete;
			Pattern_Term& operator=(const Pattern_Term& source) = delete;
			Pattern_Term& operator=(Pattern_Term&& source) = delete;
		};


		class  Transformation {	//"template" to transform input to output
		public:
			std::list<Pattern_Variable*> var_adresses;
			Pattern_Term input;	//pattern to be compared to term opject
			Pattern_Term output;	//pattern to replace match in term object

			Transformation(std::string input_, std::string output_);
			~Transformation();

			std::string print() const;	//debugging

			//only instances of transformation
			static const std::vector<Transformation*> transformations;
			static const std::vector<Transformation*> new_transforms;	//so far only for debugging
		};

		class Pattern
		{
		public:
			std::list<Pattern_Variable*> var_adresses;
			Pattern_Term term;

			Pattern(std::string name_);
			~Pattern();

			std::string print() const;	//debugging
		};

	} //namespace intern
} // namespace bmath
