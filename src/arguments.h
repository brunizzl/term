#pragma once

#include "term.h"

namespace bmath {
	namespace intern {

		class Value : public Basic_Term
		{
		public:
			std::complex<double> value;

			Value(Basic_Term* parent_);
			Value(std::string name_, Basic_Term* parent_);
			Value(const Value& source, Basic_Term* parent_ = nullptr);
			Value(std::complex<double> value_, Basic_Term* parent_);
			~Value();

			void to_str(std::string& str) const override;
			State get_state_intern() const override;
			Vals_Combined combine_values() override;
			Vals_Combined evaluate(const std::list<Known_Variable>& known_variables) const override;
			void search_and_replace(const std::string& name_, std::complex<double> value_, Basic_Term*& storage_key) override;
			bool valid_state() const override;
			bool re_smaller_than_0() override;
			void list_subterms(std::list<Basic_Term*>& subterms, State listed_state) const override;
			void sort() override;
			bool operator<(const Basic_Term& other) const override;
			bool operator==(const Basic_Term& other) const override;
		};


		class Variable : public Basic_Term
		{
		public:
			std::string name;

			Variable(Basic_Term* parent_);
			Variable(std::string name_, Basic_Term* parent_);
			Variable(const Variable& source, Basic_Term* parent_ = nullptr);
			~Variable();

			void to_str(std::string& str) const override;
			State get_state_intern() const override;
			Vals_Combined combine_values() override;
			Vals_Combined evaluate(const std::list<Known_Variable>& known_variables) const override;
			void search_and_replace(const std::string& name_, std::complex<double> value_, Basic_Term*& storage_key) override;
			bool valid_state() const override;
			void list_subterms(std::list<Basic_Term*>& subterms, State listed_state) const override;
			void sort() override;
			bool operator<(const Basic_Term& other) const override;
			bool operator==(const Basic_Term& other) const override;
		};

	} //namespace intern
} //namespace bmath
