
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

		class Value : public Basic_Term, public std::complex<double>
		{
		private:
			Basic_Term* parent_ptr;

		public:
			Value(const Value& source, Basic_Term* parent_ = nullptr);
			Value(std::complex<double> value_, Basic_Term* parent_);
			~Value();

			Basic_Term* parent() const override;
			void set_parent(Basic_Term* new_parent) override;
			void to_str(std::string& str) const override;
			void to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const override;
			Type get_type() const override;
			void combine_layers(Basic_Term*& storage_key) override;
			Vals_Combined combine_values() override;
			std::complex<double> evaluate(const std::list<Known_Variable>& known_variables) const override;
			void search_and_replace(const std::string& name_, const Basic_Term* value_, Basic_Term*& storage_key) override;
			void list_subterms(std::list<const Basic_Term*>& subterms, Type listed_type) const override;
			void sort() override; 
			Basic_Term** match_intern(Basic_Term* pattern, std::list<Pattern_Variable*>& pattern_var_adresses, Basic_Term** storage_key) override;
			bool equal_to_pattern(Basic_Term* pattern, Basic_Term** storage_key) override;
			bool operator<(const Basic_Term& other) const override;
			bool operator==(const Basic_Term& other) const override;
		};


		class Variable : public Basic_Term
		{
		private:
			Basic_Term* parent_ptr;

		public:
			const std::string name;

			Variable(std::string_view name_, Basic_Term* parent_);
			Variable(const Variable& source, Basic_Term* parent_ = nullptr);
			~Variable();

			Basic_Term* parent() const override;
			void set_parent(Basic_Term* new_parent) override;
			void to_str(std::string& str) const override;
			void to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const override;
			Type get_type() const override;
			void combine_layers(Basic_Term*& storage_key) override;
			Vals_Combined combine_values() override;
			std::complex<double> evaluate(const std::list<Known_Variable>& known_variables) const override;
			void search_and_replace(const std::string& name_, const Basic_Term* value_, Basic_Term*& storage_key) override;
			void list_subterms(std::list<const Basic_Term*>& subterms, Type listed_type) const override;
			void sort() override;
			Basic_Term** match_intern(Basic_Term* pattern, std::list<Pattern_Variable*>& pattern_var_adresses, Basic_Term** storage_key) override;
			bool equal_to_pattern(Basic_Term* pattern, Basic_Term** storage_key) override;
			bool operator<(const Basic_Term& other) const override;
			bool operator==(const Basic_Term& other) const override;
		};

	} //namespace intern
} //namespace bmath
