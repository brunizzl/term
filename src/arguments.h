#pragma once

#include "internal_functions.h"
#include "structs.h"
#include "term.h"

namespace bmath {
	namespace intern {

		class Value : public Basic_Term
		{
		private:

			Value(std::string name_, Basic_Term* parent_);
			Value(const Value& source, Basic_Term* parent_ = nullptr);
			Value(std::complex<double> value_, Basic_Term* parent_);

			//access to constructors:
			friend Basic_Term* build_subterm(std::string& subtermstr, Basic_Term* parent_);
			friend Basic_Term* build_pattern_subterm(std::string& subtermstr, Basic_Term* parent_, std::list<Pattern_Variable*>& variables);
			friend Basic_Term* copy_subterm(const Basic_Term* source, Basic_Term* parent_);
			friend class Product;
			friend class Sum;
			friend class Exponentiation;
			friend class bmath::Term;
		public:

			std::complex<double> value;

			~Value();

			void to_str(std::string& str) const override;
			State get_state_intern() const override;
			Vals_Combined combine_values() override;
			std::complex<double> evaluate(const std::list<Known_Variable>& known_variables) const override;
			void search_and_replace(const std::string& name_, const Basic_Term* value_, Basic_Term*& storage_key) override;
			bool re_smaller_than_0() override;
			void list_subterms(std::list<Basic_Term*>& subterms, State listed_state) const override;
			void sort() override; 
			Basic_Term** match_intern(Basic_Term* pattern, std::list<Pattern_Variable*>& pattern_var_adresses, Basic_Term** storage_key) override;
			bool operator<(const Basic_Term& other) const override;
			bool operator==(const Basic_Term& other) const override;
		};


		class Variable : public Basic_Term
		{
		private:
			Variable(std::string name_, Basic_Term* parent_);
			Variable(const Variable& source, Basic_Term* parent_ = nullptr);

			//access to constructors:
			friend Basic_Term* build_subterm(std::string& subtermstr, Basic_Term* parent_);
			friend Basic_Term* build_pattern_subterm(std::string& subtermstr, Basic_Term* parent_, std::list<Pattern_Variable*>& variables);
			friend Basic_Term* copy_subterm(const Basic_Term* source, Basic_Term* parent_);
		public:
			std::string name;

			~Variable();

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

	} //namespace intern
} //namespace bmath
