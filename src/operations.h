
#pragma once

#include <string>
#include <string_view>
#include <list>
#include <vector>
#include <complex>
#include <cassert>
#include <functional>
#include <iostream>
#include <algorithm>

#include "arguments.h"
#include "internal_functions.h"
#include "structs.h"
#include "term.h"

namespace bmath {
	namespace intern {

		//sum and product can have an arbitrary amount of summands / factors and behave nearly identical in structure.
		//this class tries to implement all identic behavior of sum / product
		//operate is add for sum, mul for product, this_type is Type::sum or Type::product, neutral_val is 0 for sum, 1 for product
		template<void(*operate)(std::complex<double>* const first, const std::complex<double> second), Type this_type, int neutral_val>
		class Variadic_Operator : public Basic_Term
		{
		protected:
			std::list<Basic_Term*> operands;	//summands in sum, factors in product 

			Variadic_Operator();
			Variadic_Operator(const Variadic_Operator& source);
			Variadic_Operator(std::list<Basic_Term*>&& operands);
			~Variadic_Operator();

		public:
			Type type() const override;
			void combine_layers(Basic_Term*& storage_key) override;
			Vals_Combined combine_values() override;
			std::complex<double> evaluate(const std::list<Known_Variable>& known_variables) const override;
			void search_and_replace(const std::string& name_, const Basic_Term* value_, Basic_Term*& storage_key) override;
			void for_each(std::function<void(Basic_Term* this_ptr, Type this_type)> func) override;
			Basic_Term** match_intern(Basic_Term* pattern, std::list<Pattern_Variable*>& pattern_var_adresses, Basic_Term** storage_key) override;
			bool equal_to_pattern(Basic_Term* pattern, Basic_Term* patterns_parent, Basic_Term** storage_key) override;
			void reset_own_matches(Basic_Term* parent) override;
			bool operator<(const Basic_Term& other) const override;
			bool operator==(const Basic_Term& other) const override;

			//this function is called in match_intern() of Variadic_Operator<>. 
			//match_intern() always tries to match from the highest level (match intern is called -> caller knows he sees full pattern). 
			//that allows to only match some of operands with the pattern, as long, as all of the pattern is matched.
			//other operands may still call equal_to_pattern() and may not need behaving different if pattern is of type variadic_operator<>.
			//if parts of this operands match pattern, a new variadic_operator<> will be constructed, with the matched -
			//operands moved there. the new variadic_operator<> will be returned. otherwise nullptr is returned.
			Basic_Term** part_equal_to_pattern(Basic_Term* pattern, Basic_Term* patterns_parent, Basic_Term** storage_key);

			//pushes term_ptr back into operands
			void push_back(Basic_Term* const term_ptr);

			//wrapper for std::list sort()
			inline void sort_operands() { this->operands.sort([](Basic_Term*& a, Basic_Term*& b) -> bool {return *a < *b; }); }

			//constructor wrapper
			static Basic_Term* new_instance(std::list<Basic_Term*> && operands);

			//returns, whether test_ops has a match for every element in pattern_ops (pattern is owner of pattern_ops)
			//returns {} if no match was found, returns matched operands of test_ops if match was found
			static std::optional<std::list<Basic_Term*>> operands_contain_pattern(std::list<Basic_Term*>& test_ops, std::list<Basic_Term*>& pattern_ops, Basic_Term* pattern);
		};

		class Sum : public Variadic_Operator<add, Type::sum, 0>
		{
		private:
			friend Pattern_Term::~Pattern_Term();

		public:
			Sum();
			Sum(std::string_view name_, std::size_t op);
			Sum(std::string_view name_, std::size_t op, std::list<Pattern_Variable*>& variables);
			Sum(std::list<Basic_Term*>&& operands);
			Sum(const Sum& source);

			void to_str(std::string& str, int caller_operator_precedence) const override;
			void to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const override;
		};


		class Product : public Variadic_Operator<mul, Type::product, 1>
		{
		private:
			friend Pattern_Term::~Pattern_Term();

		public:
			Product();
			Product(std::string_view name_, std::size_t op);
			Product(Basic_Term* name_, std::complex<double> factor);
			Product(std::string_view name_, std::size_t op, std::list<Pattern_Variable*>& variables);
			Product(std::list<Basic_Term*>&& operands);
			Product(const Product& source);

			void to_str(std::string& str, int caller_operator_precedence) const override;
			void to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const override;
		};


		class Exponentiation : public Basic_Term
		{
		private:
			Basic_Term* expo;
			Basic_Term* base;

			friend class bmath::Term;
			friend Pattern_Term::~Pattern_Term();

		public:
			Exponentiation();
			Exponentiation(std::string_view name_, std::size_t op);
			Exponentiation(Basic_Term* base_, std::complex<double> exponent_);
			Exponentiation(std::string_view name_, std::size_t op, std::list<Pattern_Variable*>& variables);
			Exponentiation(const Exponentiation& source);
			~Exponentiation();

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
			void reset_own_matches(Basic_Term* parent) override;
			bool operator<(const Basic_Term& other) const override;
			bool operator==(const Basic_Term& other) const override;
		};


		class Par_Operator : public Basic_Term
		{
		private:
			Par_Op_Type op_type;
			Basic_Term* argument;

			friend Pattern_Term::~Pattern_Term();

		public:
			Par_Operator();
			Par_Operator(std::string_view name_, Par_Op_Type op_type_);
			Par_Operator(std::string_view name_, Par_Op_Type op_type_, std::list<Pattern_Variable*>& variables);
			Par_Operator(const Par_Operator& source);
			~Par_Operator();

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
			void reset_own_matches(Basic_Term* parent) override;
			bool operator<(const Basic_Term& other) const override;
			bool operator==(const Basic_Term& other) const override;
		};








		template<void(*operate)(std::complex<double>* const first, const std::complex<double>second), Type this_type, int neutral_val>
		inline Variadic_Operator<operate, this_type, neutral_val>::Variadic_Operator()
		{
		}

		template<void(*operate)(std::complex<double>* const first, const std::complex<double>second), Type this_type, int neutral_val>
		inline Variadic_Operator<operate, this_type, neutral_val>::Variadic_Operator(const Variadic_Operator& source)
			//:operands(source.operands)
		{
			for (const auto it : source.operands) {
				this->operands.push_back(copy_subterm(it));
			}
		}

		template<void(*operate)(std::complex<double>* const first, const std::complex<double>second), Type this_type, int neutral_val>
		inline Variadic_Operator<operate, this_type, neutral_val>::Variadic_Operator(std::list<Basic_Term*>&& operands_)
			: operands(std::move(operands_))
		{
		}

		template<void(*operate)(std::complex<double>* const first, const std::complex<double>second), Type this_type, int neutral_val>
		inline Variadic_Operator<operate, this_type, neutral_val>::~Variadic_Operator()
		{
			for (const auto it : this->operands) {
				delete it;
			}
		}

		template<void(*operate)(std::complex<double>* const first, const std::complex<double>second), Type this_type, int neutral_val>
		inline Type Variadic_Operator<operate, this_type, neutral_val>::type() const
		{
			return this_type;
		}

		template<void(*operate)(std::complex<double>* const first, const std::complex<double>second), Type this_type, int neutral_val>
		inline void Variadic_Operator<operate, this_type, neutral_val>::combine_layers(Basic_Term*& storage_key)
		{
			for (auto& it = this->operands.begin(); it != this->operands.end();) {	//reference is needed in next line
				(*it)->combine_layers(*it);
				if (type_of(*it) == this_type) {
					Variadic_Operator<operate, this_type, neutral_val>* redundant = static_cast<Variadic_Operator<operate, this_type, neutral_val>*>(*it);
					this->operands.splice(this->operands.end(), redundant->operands);
					delete redundant;
					it = this->operands.erase(it);
				}
				else {
					++it;
				}
			}
			if (this->operands.size() == 1) {	//this only consists of one operand -> this layer is not needed and removed
				storage_key = *(this->operands.begin());
				this->operands.clear();
				delete this;
			}
		}

		template<void(*operate)(std::complex<double>* const first, const std::complex<double>second), Type this_type, int neutral_val>
		inline Vals_Combined Variadic_Operator<operate, this_type, neutral_val>::combine_values()
		{
			bool only_known = true;
			std::complex<double> value = static_cast<double>(neutral_val);

			for (auto it = this->operands.begin(); it != this->operands.end();) {
				const auto [it_known, it_val] = (*it)->combine_values();
				if (it_known) {
					operate(&value, it_val);
					delete (*it);
					it = this->operands.erase(it);
				}
				else {
					only_known = false;
					++it;
				}
			}
			if (!only_known && value != static_cast<double>(neutral_val)) {	//if only_known, this would be deleted anyway.
				if constexpr (this_type == Type::product)
				{
					if (value == 0.0) {
						return { true, 0 };
					}
				}
				this->operands.push_back(new Value(value));
			}
			return { only_known, value };
		}

		template<void(*operate)(std::complex<double>* const first, const std::complex<double>second), Type this_type, int neutral_val>
		inline std::complex<double> Variadic_Operator<operate, this_type, neutral_val>::evaluate(const std::list<Known_Variable>& known_variables) const
		{
			std::complex<double> result = static_cast<double>(neutral_val);
			for (const auto it : this->operands) {
				operate(&result, it->evaluate(known_variables));
			}
			return result;
		}

		template<void(*operate)(std::complex<double>* const first, const std::complex<double>second), Type this_type, int neutral_val>
		inline void Variadic_Operator<operate, this_type, neutral_val>::search_and_replace(const std::string& name_, const Basic_Term* value_, Basic_Term*& storage_key)
		{
			for (auto& it : this->operands) {	//reference is needed in next line
				it->search_and_replace(name_, value_, it);
			}
		}

		template<void(*operate)(std::complex<double>* const first, const std::complex<double>second), Type this_type, int neutral_val>
		inline void Variadic_Operator<operate, this_type, neutral_val>::for_each(std::function<void(Basic_Term* this_ptr, Type this_type)> func)
		{
			for (auto it : this->operands) {
				it->for_each(func);
			}
			func(this, this_type);
		}

		template<void(*operate)(std::complex<double>* const first, const std::complex<double>second), Type this_type, int neutral_val>
		inline Basic_Term** Variadic_Operator<operate, this_type, neutral_val>::match_intern(Basic_Term* pattern, std::list<Pattern_Variable*>& pattern_var_adresses, Basic_Term** storage_key)
		{
			reset_all_pattern_vars(pattern_var_adresses);
			this->sort();
			Basic_Term** const found_part = this->part_equal_to_pattern(pattern, nullptr, storage_key);
			if (found_part) {
				return found_part;
			}
			else {
				Basic_Term** argument_match;
				for (auto& it : this->operands) {	//references are important, because we want to return the position of it.
					reset_all_pattern_vars(pattern_var_adresses);
					argument_match = it->match_intern(pattern, pattern_var_adresses, &it);
					if (argument_match) {
						return argument_match;
					}
				}
			}
			return nullptr;
		}

		template<void(*operate)(std::complex<double>* const first, const std::complex<double>second), Type this_type, int neutral_val>
		inline bool Variadic_Operator<operate, this_type, neutral_val>::equal_to_pattern(Basic_Term* pattern, Basic_Term* patterns_parent, Basic_Term** storage_key)
		{
			const Type pattern_type = type_of(pattern);
			if (pattern_type == this_type) {
				auto* const pattern_ = static_cast<Variadic_Operator<operate, this_type, neutral_val>*>(pattern);
				if (this->operands.size() != pattern_->operands.size()) {
					return false;
				}
				auto matched_operands = operands_contain_pattern(this->operands, pattern_->operands, pattern);
				if (matched_operands) {
					this->operands.splice(this->operands.end(), *matched_operands);
					return true;
				}
				else {
					return false;
				}
			}
			else if (pattern_type == Type::pattern_variable) {
				Pattern_Variable* pattern_var = static_cast<Pattern_Variable*>(pattern);
				return pattern_var->try_matching(this, patterns_parent, storage_key);
			}
			else {
				return false;
			}
		}

		template<void(*operate)(std::complex<double>* const first, const std::complex<double>second), Type this_type, int neutral_val>
		inline void Variadic_Operator<operate, this_type, neutral_val>::reset_own_matches(Basic_Term* parent)
		{
			for (auto it : this->operands) {
				it->reset_own_matches(this);
			}
		}

		template<void(*operate)(std::complex<double>* const first, const std::complex<double>second), Type this_type, int neutral_val>
		inline bool Variadic_Operator<operate, this_type, neutral_val>::operator<(const Basic_Term& other) const
		{
			if (this_type != type_of(other)) {
				return this_type < type_of(other);
			}
			else {
				const Variadic_Operator<operate, this_type, neutral_val>* other_product = static_cast<const Variadic_Operator<operate, this_type, neutral_val>*>(&other);
				if (this->operands.size() != other_product->operands.size()) {
					return this->operands.size() < other_product->operands.size();
				}
				//the operator assumes from now on to have sorted products to compare
				auto it_this = this->operands.begin();
				auto it_other = other_product->operands.begin();
				for (; it_this != this->operands.end() && it_other != other_product->operands.end(); ++it_this, ++it_other) {
					if ((**it_this) < (**it_other)) {
						return true;
					}
					if ((**it_other) < (**it_this)) {
						return false;
					}
				}
				return false;
			}
		}

		template<void(*operate)(std::complex<double>* const first, const std::complex<double>second), Type this_type, int neutral_val>
		inline bool Variadic_Operator<operate, this_type, neutral_val>::operator==(const Basic_Term& other) const
		{
			if (type_of(other) == this_type) {
				const Variadic_Operator<operate, this_type, neutral_val>* other_variadic = static_cast<const Variadic_Operator<operate, this_type, neutral_val>*>(&other);
				if (this->operands.size() != other_variadic->operands.size()) {
					return false;
				}
				//the operator assumes from now on to have sorted products to compare
				auto it_this = this->operands.begin();
				auto it_other = other_variadic->operands.begin();
				for (; it_this != this->operands.end() && it_other != other_variadic->operands.end(); ++it_this, ++it_other) {
					if ((**it_this) != (**it_other)) {
						return false;
					}
				}
				return true;
			}
			return false;
		}

		template<void(*operate)(std::complex<double>* const first, const std::complex<double>second), Type this_type, int neutral_val>
		inline Basic_Term** Variadic_Operator<operate, this_type, neutral_val>::part_equal_to_pattern(Basic_Term* pattern, Basic_Term* patterns_parent, Basic_Term** storage_key)
		{
			const Type pattern_type = type_of(pattern);
			if (pattern_type == this_type) {
				Variadic_Operator<operate, this_type, neutral_val>* const pattern_ = static_cast<Variadic_Operator<operate, this_type, neutral_val>*>(pattern);
				if (pattern_->operands.size() > this->operands.size()) {
					return nullptr;
				}
				auto matched_operands = operands_contain_pattern(this->operands, pattern_->operands, pattern);
				if (matched_operands) {
					if (this->operands.size()) {
						this->operands.push_back(new_instance(std::move(*matched_operands)));
						return &(this->operands.back());
					}
					else {
						this->operands.splice(this->operands.end(), *matched_operands);
						return storage_key;
					}
				}
				else {
					return false;
				}
			}
			else if (pattern_type == Type::pattern_variable) {
				Pattern_Variable* pattern_var = static_cast<Pattern_Variable*>(pattern);
				if (pattern_var->try_matching(this, patterns_parent, storage_key)) {
					return storage_key;
				}
			}
			return nullptr;
		}

		template<void(*operate)(std::complex<double>* const first, const std::complex<double>second), Type this_type, int neutral_val>
		inline void Variadic_Operator<operate, this_type, neutral_val>::push_back(Basic_Term* const term_ptr)
		{
			this->operands.push_back(term_ptr);
		}

		template<void(*operate)(std::complex<double>* const first, const std::complex<double>second), Type this_type, int neutral_val>
		inline Basic_Term* Variadic_Operator<operate, this_type, neutral_val>::new_instance(std::list<Basic_Term*>&& operands)
		{
			if constexpr (this_type == Type::sum) {
				return new Sum(std::forward<std::list<Basic_Term*>>(operands));
			}
			else if constexpr (this_type == Type::product) {
				return new Product(std::forward<std::list<Basic_Term*>>(operands));
			}
			else {
				assert(false);	//the only (currently implemented) variadic_operands are sum and product.
			}
		}

		template<void(*operate)(std::complex<double>* const first, const std::complex<double>second), Type this_type, int neutral_val>
		inline std::optional<std::list<Basic_Term*>> Variadic_Operator<operate, this_type, neutral_val>::operands_contain_pattern(std::list<Basic_Term*>& test_ops, std::list<Basic_Term*>& pattern_ops, Basic_Term* pattern)
		{
			assert(std::is_sorted(test_ops.begin(), test_ops.end(), [](Basic_Term* a, Basic_Term* b) { return *a < *b; }));
			pattern_ops.sort([](Basic_Term*& a, Basic_Term*& b) -> bool {return *a < *b; }); //due to the possibility of pattern variables beeing matched by other functions called by match_intern, this step is neccessary.

			std::list<Basic_Term*> matched_operands;	//to not be matched multiple times, already matched operands need to be moved here.
			std::vector<std::list<Basic_Term*>::iterator> match_positions;	//remembers position of each matched_operand
			match_positions.reserve(pattern_ops.size());
			bool already_matching_pattern_vars = false;
			auto next_search_begin = test_ops.begin();	//no operand before this will be checked by current pattern_it
			auto pattern_it = pattern_ops.begin();

			while (pattern_it != pattern_ops.end()) {

				//once we start to compare to pattern_variables, we need to make sure, already matched ones are compared first. (hence we sort)
				if (!already_matching_pattern_vars && type_of(*pattern_it) == Type::pattern_variable) {
					already_matching_pattern_vars = true;
					pattern_ops.sort([](Basic_Term* a, Basic_Term* b) {return *a < *b; });
					pattern_it = find_first_of(pattern_ops, Type::pattern_variable);
				}

				//if we reached the last pattern_var and this last pattern_var has not been matched yet, it now matches the whole rest of test_ops, as the rest is packaged into one single operand.
				if (already_matching_pattern_vars && static_cast<Pattern_Variable*>(*pattern_it)->is_unmatched() && std::next(pattern_it) == pattern_ops.end()) {
					test_ops.push_back(new_instance(std::move(test_ops)));
					next_search_begin = test_ops.begin();
				}

				bool found_match = false;
				for (auto test_it = next_search_begin; test_it != test_ops.end(); ++test_it) {
					if ((*test_it)->equal_to_pattern(*pattern_it, pattern, &*test_it)) {
						match_positions.emplace_back(std::next(test_it));
						matched_operands.splice(matched_operands.end(), test_ops, test_it);
						found_match = true;
						break;
					}
					else {
						(*pattern_it)->reset_own_matches(pattern);
					}
				}

				if (!found_match) {
					if (matched_operands.size()) {	//going back one operand in pattern and try to rematch
						test_ops.splice(match_positions.back(), matched_operands, std::prev(matched_operands.end()));
						next_search_begin = match_positions.back();
						match_positions.pop_back();
						--pattern_it;
						--pattern;	//pattern is exclusively used to mark what exactly matched the pattern_variables. to distinguish different pattern_ops, we need to give every match a different "parent"
						(*pattern_it)->reset_own_matches(pattern);
						if (already_matching_pattern_vars && type_of(*pattern_it) != Type::pattern_variable) {
							already_matching_pattern_vars = false;
						}
					}
					else {	//cleanup
						test_ops.splice(test_ops.end(), matched_operands);
						test_ops.sort([](Basic_Term* a, Basic_Term* b) { return *a < *b; });
						return {};
					}
				}
				else {
					++pattern_it;
					++pattern;
					next_search_begin = test_ops.begin();
				}
			}
			return std::move(matched_operands);
		}

} //namespace intern
} //namespace bmath
