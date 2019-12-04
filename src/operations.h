
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
			bool combine_layers(Basic_Term*& storage_key) override;
			Vals_Combined combine_values() override;
			std::complex<double> evaluate(const std::list<Known_Variable>& known_variables) const override;
			void search_and_replace(const std::string& name_, const Basic_Term* value_, Basic_Term*& storage_key) override;
			void for_each(std::function<void(Basic_Term* this_ptr, Type this_type)> func) override;
			bool equal_to_pattern(Basic_Term* pattern, Basic_Term* patterns_parent, Basic_Term *& storage_key) override;
			void reset_own_matches(Basic_Term* parent) override;
			bool operator<(const Basic_Term& other) const override;
			bool operator==(const Basic_Term& other) const override;

			//this function is called in match_intern() of Variadic_Operator<>. 
			//match_intern() always tries to match from the highest level (match intern is called -> caller knows he sees full pattern). 
			//that allows to only match some of this operands with the pattern, as long, as all of the pattern is matched.
			//other operands may still call equal_to_pattern() and may not need behaving different if pattern is of type variadic_operator<>.
			//if parts of this operands match pattern, a new variadic_operator<> will be constructed, with the matched -
			//operands moved there. the new variadic_operator<> will be returned. otherwise nullptr is returned.
			Basic_Term** part_equal_to_pattern(Basic_Term* pattern, Basic_Term* patterns_parent, Basic_Term *& storage_key);

			//pushes term_ptr back into operands
			void push_back(Basic_Term* const term_ptr);

			//wrapper for std::list sort()
			inline void sort_operands() { this->operands.sort([](Basic_Term*& a, Basic_Term*& b) -> bool {return *a < *b; }); }

			//clears list
			void clear_operands();

			//constructor wrapper
			static Basic_Term* new_instance(std::list<Basic_Term*> && operands);

			//returns, whether test_ops has a match for every element in pattern_ops (pattern is owner of pattern_ops)
			//returns {} if no match was found, returns matched operands of test_ops if match was found
			static std::optional<std::list<Basic_Term*>> 
				operands_contain_pattern(std::list<Basic_Term*>& test_ops, std::list<Basic_Term*>& pattern_ops, Basic_Term* pattern);
		};

		class Sum : public Variadic_Operator<add, Type::sum, 0>
		{
		private:
			const static std::vector<Transformation*> sum_transforms;
			friend void delete_pattern(Basic_Term* pattern);

		public:
			Sum();
			Sum(std::string_view name_, std::size_t op);
			Sum(std::string_view name_, std::size_t op, std::list<Pattern_Variable*>& variables);
			Sum(std::list<Basic_Term*>&& operands);
			Sum(const Sum& source);

			void to_str(std::string& str, int caller_operator_precedence) const override;
			void to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const override;
			bool transform(Basic_Term *& storage_key) override;

			//as finding common factors in summands turns out to be quite hard, this is not done via pattern matching, but via this function
			//something was found and combined -> returns true, else return false
			bool factoring();

			//converts "a-(b+c)" to "a-b-c" returns true if something changed
			bool unpack_minus();

			//tests if this is a polinomial. if so, it will find its roots and transform the polinomial into the product of its roots.
			//returns true if changed, false if no polinomial was found. storage_key is pointer to where the pointer to this is held.
			bool factor_polinomial(Basic_Term** storage_key);
		};


		class Product : public Variadic_Operator<mul, Type::product, 1>
		{
		private:
			const static std::vector<Transformation*> product_transforms;

			friend void delete_pattern(Basic_Term* pattern);
			friend class Sum;

		public:
			Product();
			Product(std::string_view name_, std::size_t op);
			Product(Basic_Term* name_, std::complex<double> factor);
			Product(std::string_view name_, std::size_t op, std::list<Pattern_Variable*>& variables);
			Product(std::list<Basic_Term*>&& operands);
			Product(const Product& source);

			void to_str(std::string& str, int caller_operator_precedence) const override;
			void to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const override;
			bool transform(Basic_Term *& storage_key) override;

			//converts "a/(b*c)" to "a/b/c" returns true if something changed
			bool unpack_division();
		};


		class Power : public Basic_Term
		{
		private:
			Basic_Term* expo;
			Basic_Term* base;
			const static std::vector<Transformation*> pow_transforms;

			friend class bmath::Term;
			friend void delete_pattern(Basic_Term* pattern);
			friend class Product;
			friend class Monom;

		public:
			Power();
			Power(std::string_view name_, std::size_t op);
			Power(Basic_Term* base_, std::complex<double> exponent_);
			Power(Basic_Term* base_, Basic_Term* expo_);
			Power(std::string_view name_, std::size_t op, std::list<Pattern_Variable*>& variables);
			Power(const Power& source);
			~Power();

			void to_str(std::string& str, int caller_operator_precedence) const override;
			void to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const override;
			Type type() const override;
			bool combine_layers(Basic_Term*& storage_key) override;
			Vals_Combined combine_values() override;
			std::complex<double> evaluate(const std::list<Known_Variable>& known_variables) const override;
			void search_and_replace(const std::string& name_, const Basic_Term* value_, Basic_Term*& storage_key) override;
			void for_each(std::function<void(Basic_Term* this_ptr, Type this_type)> func) override;
			bool transform(Basic_Term *& storage_key) override;
			bool equal_to_pattern(Basic_Term* pattern, Basic_Term* patterns_parent, Basic_Term *& storage_key) override;
			void reset_own_matches(Basic_Term* parent) override;
			bool operator<(const Basic_Term& other) const override;
			bool operator==(const Basic_Term& other) const override;
		};


		class Par_Operator : public Basic_Term
		{
		private:
			Par_Op_Type op_type;
			Basic_Term* argument;
			const static std::vector<Transformation*> parop_transforms;

			friend void delete_pattern(Basic_Term* pattern);

		public:
			Par_Operator();
			Par_Operator(std::string_view name_, Par_Op_Type op_type_);
			Par_Operator(std::string_view name_, Par_Op_Type op_type_, std::list<Pattern_Variable*>& variables);
			Par_Operator(const Par_Operator& source);
			~Par_Operator();

			void to_str(std::string& str, int caller_operator_precedence) const override;
			void to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const override;
			Type type() const override;
			bool combine_layers(Basic_Term*& storage_key) override;
			Vals_Combined combine_values() override;
			std::complex<double> evaluate(const std::list<Known_Variable>& known_variables) const override;
			void search_and_replace(const std::string& name_, const Basic_Term* value_, Basic_Term*& storage_key) override;
			void for_each(std::function<void(Basic_Term* this_ptr, Type this_type)> func) override;
			bool transform(Basic_Term *& storage_key) override;
			bool equal_to_pattern(Basic_Term* pattern, Basic_Term* patterns_parent, Basic_Term *& storage_key) override;
			void reset_own_matches(Basic_Term* parent) override;
			bool operator<(const Basic_Term& other) const override;
			bool operator==(const Basic_Term& other) const override;
		};


		class Monom	//pattern to identify monoms (monom beeing summand of polynom (a*x^n)) to test if sum is polynom
		{
		private:
			Pattern_Variable a;	//all suberms of monom are held here directly, as we need to acess them directly anyway.
			Pattern_Variable n;
			Pattern_Variable x;
			Power x_n;		//x^n
			Product monom;	//a*x^n

			Product a_x;	//this is not part of the same tree as the others, but builds (from same a and x) the tree a*x

		public:
			Monom();
			~Monom();

			//determines if thest is a monom or not
			bool matching(Basic_Term* test, Basic_Term*& storage_key);

			void full_reset();					//resets a, n and x
			void partial_reset();				//only resets a and n
			std::complex<double> factor() const;
			int exponent() const;
			Basic_Term* base() const;
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
		inline bool Variadic_Operator<operate, this_type, neutral_val>::combine_layers(Basic_Term*& storage_key)
		{
			bool changed = false;
			for (auto& it = this->operands.begin(); it != this->operands.end();) {	//reference is needed in next line
				changed |= (*it)->combine_layers(*it);
				if (type_of(*it) == this_type) {
					Variadic_Operator<operate, this_type, neutral_val>* redundant = static_cast<Variadic_Operator<operate, this_type, neutral_val>*>(*it);
					this->operands.splice(this->operands.end(), redundant->operands);
					delete redundant;
					it = this->operands.erase(it);
					changed = true;
				}
				else {
					++it;
				}
			}
			auto val = find_first_of(this->operands, Type::value);
			if (val != this->operands.end() && static_cast<Value*>(*val)->val() == static_cast<double>(neutral_val)) {
				this->operands.erase(val);
				changed = true;
			}
			if (this->operands.size() == 1) {	//this only consists of one operand -> this layer is not needed and removed
				storage_key = *(this->operands.begin());
				this->operands.clear();
				delete this;
				changed = true;
			}
			return changed;
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
		inline bool Variadic_Operator<operate, this_type, neutral_val>::equal_to_pattern(Basic_Term* pattern, Basic_Term* patterns_parent, Basic_Term *& storage_key)
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
		inline Basic_Term** Variadic_Operator<operate, this_type, neutral_val>::part_equal_to_pattern(Basic_Term* pattern, Basic_Term* patterns_parent, Basic_Term *& storage_key)
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
						return &storage_key;
					}
				}
				else {
					return nullptr;
				}
			}
			else if (pattern_type == Type::pattern_variable) {
				Pattern_Variable* pattern_var = static_cast<Pattern_Variable*>(pattern);
				if (pattern_var->try_matching(this, patterns_parent, storage_key)) {
					return &storage_key;
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
		inline void Variadic_Operator<operate, this_type, neutral_val>::clear_operands()
		{
			this->operands.clear();
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
			Basic_Term* match_counter = pattern;		//used to differentiate between matches of same parent term.

			while (pattern_it != pattern_ops.end()) {

				//once we start to compare to pattern_variables, we need to make sure, already matched ones are compared first. (hence we sort)
				if (!already_matching_pattern_vars && type_of(*pattern_it) == Type::pattern_variable) {
					already_matching_pattern_vars = true;
					pattern_ops.sort([](Basic_Term* a, Basic_Term* b) {return *a < *b; });
					pattern_it = find_first_of(pattern_ops, Type::pattern_variable);
				}

				//if we reached the last pattern_var and this last pattern_var has not been matched yet, it now matches the whole rest of test_ops, as the rest is packaged into one single operand.
				if (already_matching_pattern_vars && static_cast<Pattern_Variable*>(*pattern_it)->is_unmatched() && std::next(pattern_it) == pattern_ops.end() && test_ops.size() > 1) {
					test_ops.push_back(new_instance(std::move(test_ops)));
					next_search_begin = test_ops.begin();
				}

				bool found_match = false;
				for (auto& test_it = next_search_begin; test_it != test_ops.end(); ++test_it) {
					if ((*test_it)->equal_to_pattern(*pattern_it, match_counter, *test_it)) {
						match_positions.emplace_back(std::next(test_it));
						matched_operands.splice(matched_operands.end(), test_ops, test_it);
						found_match = true;
						break;
					}
					else {
						(*pattern_it)->reset_own_matches(match_counter);
					}
				}

				if (!found_match) {
					if (matched_operands.size()) {	//going back one operand in pattern and try to rematch
						test_ops.splice(match_positions.back(), matched_operands, std::prev(matched_operands.end()));
						next_search_begin = match_positions.back();
						match_positions.pop_back();
						--pattern_it;
						--match_counter;
						(*pattern_it)->reset_own_matches(match_counter);
						if (already_matching_pattern_vars && type_of(*pattern_it) != Type::pattern_variable) {
							already_matching_pattern_vars = false;
						}
					}
					else {	//cleanup
						test_ops.splice(test_ops.end(), matched_operands);
						test_ops.sort([](Basic_Term* a, Basic_Term* b) { return *a < *b; });
						for (auto& operand : test_ops) {
							operand->combine_layers(operand);
						}
						return {};
					}
				}
				else {
					++pattern_it;
					++match_counter;
					//the modification of the match_counter pointer is a hack and only workes, as long as no 
					//two variadic operators appear ony different levels in a pattern (no variadic should be subterm of another, not direct nor indirect)
					//this is because reset_own_matches() relies on the pattern pointer to free a pattern_var, however the pointer is lost if this 
					//function here is called recursively (equal_to_pattern calls this function and vice versa)
					//example: pattern "a*b+a" would not reset the "b" pattern_var correctly, as it was set with next(product's pointer), but product
					//tries to reset with its pointer directly, not with the adresses following it. if the (just reset) "a" now matches and the whole match
					//succedes, a term not equivalent to the original is produced. e.g. matching ("a*b+a" -> "a*(b+1)") with "c*d+g*f+g" would result in 
					//"g*(d+1)+c*d", instead of the expected "c*d+g*(f+1)". this is very bad.
					next_search_begin = test_ops.begin();
				}
			}
			//hacky solution to problem: correct all adresses after the fact.
			for (auto operand : pattern_ops) {
				if (type_of(operand) == Type::pattern_variable) {
					Pattern_Variable* const pattern_var = static_cast<Pattern_Variable*>(operand);
					pattern_var->set_parent(pattern);
				}
			}
			return std::move(matched_operands);
		}

} //namespace intern
} //namespace bmath
