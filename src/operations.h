
#pragma once

#include <string>
#include <string_view>
#include <list>
#include <vector>
#include <complex>
#include <cassert>
#include <functional>

#include "arguments.h"
#include "internal_functions.h"
#include "structs.h"
#include "term.h"

namespace bmath {
	namespace intern {

		//sum and product can have an arbitrary amount of summands / factors and behave nearly identical in structure.
		//this class tries to implement all identic behavior of sum / product
		//operate is add for sum, mul for product, this_type is Type::sum or Type::product, neutral_val is 0 for sum, 1 for product
		template<void(*operate)(std::complex<double>* first, const std::complex<double> second), Type this_type, int neutral_val>
		class Variadic_Operator : public Basic_Term
		{
		protected:
			std::list<Basic_Term*> operands;	//summands in sum, factors in product
			Value value;				//only one summand / factor is allowed to be of Type::value. this is stored outside of operands here.

			Variadic_Operator();
			Variadic_Operator(const Variadic_Operator& source);
			Variadic_Operator(std::complex<double> value_, std::list<Basic_Term*>&& operands);
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
			//this is the only overload of reset_own_matches() able to fail, as it is the only able to try multiple matches.
			bool reset_own_matches(Basic_Term* parent) override;
			bool operator<(const Basic_Term& other) const override;
			bool operator==(const Basic_Term& other) const override;

			//this function is called in match_intern() of Variadic_Operator<>. 
			//match_intern() always tries to match from the highest level (match intern is called -> caller knows he sees full pattern). 
			//that allows to only match some of operands with the pattern, as long, as all of the pattern is matched.
			//other operands may still call equal_to_pattern() and may not need behaving different if pattern is of type variadic_operator<>.
			//if parts of this operands match pattern, a new variadic_operator<> will be constructed, with the matched -
			//operands moved there. the new variadic_operator<> will be returned. otherwise nullptr is returned.
			Basic_Term** part_equal_to_pattern(Basic_Term* pattern, Basic_Term* patterns_parent, Basic_Term** storage_key);

			//pushes copy of source into this->operands or, if type_of(source) is value, uses operate() to add/ multiply onto this->value
			void copy_into_operands(Basic_Term* source);

			//pushes term_ptr into this->operands or, if (type(term_ptr) is value, uses operate() to add/ multiply onto this->value and deletes term_ptr
			void move_into_operands(Basic_Term* term_ptr);

			//uses operate() on this->value and number (no new allocation, if done via this method.)
			inline void calc_onto_value(std::complex<double> number) { operate(&(this->value.val()), number); }

			//wrapper for std::list sort()
			inline void sort_operands(std::function<bool(Basic_Term*& a, Basic_Term*& b)> compare) { this->operands.sort(compare); }
		};

		class Sum : public Variadic_Operator<add, Type::sum, 0>
		{
		public:
			Sum();
			Sum(std::string_view name_, std::size_t op);
			Sum(std::string_view name_, std::size_t op, std::list<Pattern_Variable*>& variables);
			Sum(const Sum& source);

			void to_str(std::string& str, int caller_operator_precedence) const override;
			void to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const override;
		};


		class Product : public Variadic_Operator<mul, Type::product, 1>
		{
		public:
			Product();
			Product(std::string_view name_, std::size_t op);
			Product(Basic_Term* name_, std::complex<double> factor);
			Product(std::string_view name_, std::size_t op, std::list<Pattern_Variable*>& variables);
			Product(const Product& source);

			void to_str(std::string& str, int caller_operator_precedence) const override;
			void to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const override;
		};


		class Exponentiation : public Basic_Term
		{
		private:
			Basic_Term* exponent;
			Basic_Term* base;

			friend class bmath::Term;

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
			bool reset_own_matches(Basic_Term* parent) override;
			bool operator<(const Basic_Term& other) const override;
			bool operator==(const Basic_Term& other) const override;
		};


		class Par_Operator : public Basic_Term
		{
		private:
			Par_Op_Type op_type;
			Basic_Term* argument;

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
			bool reset_own_matches(Basic_Term* parent) override;
			bool operator<(const Basic_Term& other) const override;
			bool operator==(const Basic_Term& other) const override;
		};









		template<void(*operate)(std::complex<double>* first, const std::complex<double>second), Type this_type, int neutral_val>
		inline Variadic_Operator<operate, this_type, neutral_val>::Variadic_Operator()
			:value({ static_cast<double>(neutral_val), 0 })
		{
		}

		template<void(*operate)(std::complex<double>* first, const std::complex<double>second), Type this_type, int neutral_val>
		inline Variadic_Operator<operate, this_type, neutral_val>::Variadic_Operator(const Variadic_Operator& source)
			:value(source.value.val())
		{
			for (const auto it : source.operands) {
				this->operands.push_back(copy_subterm(it));
			}
		}

		template<void(*operate)(std::complex<double>* first, const std::complex<double>second), Type this_type, int neutral_val>
		inline Variadic_Operator<operate, this_type, neutral_val>::Variadic_Operator(std::complex<double> value_, std::list<Basic_Term*>&& operands_)
			:value(value_), operands(std::move(operands_))
		{
		}

		template<void(*operate)(std::complex<double>* first, const std::complex<double>second), Type this_type, int neutral_val>
		inline Variadic_Operator<operate, this_type, neutral_val>::~Variadic_Operator()
		{
			for (const auto it : this->operands) {
				delete it;
			}
		}

		template<void(*operate)(std::complex<double>* first, const std::complex<double>second), Type this_type, int neutral_val>
		inline Type Variadic_Operator<operate, this_type, neutral_val>::type() const
		{
			return this_type;
		}

		template<void(*operate)(std::complex<double>* first, const std::complex<double>second), Type this_type, int neutral_val>
		inline void Variadic_Operator<operate, this_type, neutral_val>::combine_layers(Basic_Term*& storage_key)
		{
			for (auto& it = this->operands.begin(); it != this->operands.end();) {	//reference is needed in next line
				(*it)->combine_layers(*it);
				if (type_of(*it) == this_type) {
					Variadic_Operator<operate, this_type, neutral_val>* redundant = static_cast<Variadic_Operator<operate, this_type, neutral_val>*>((*it));
					operate(&(this->value.val()), redundant->value.val());
					this->operands.splice(this->operands.end(), redundant->operands);
					delete redundant;
					it = this->operands.erase(it);
				}
				else {
					++it;
				}
			}
			if (this->operands.size() == 1 && this->value.val() == static_cast<double>(neutral_val)) {	//this only consists of one non- neutral value -> layer is not needed and this is deleted
				storage_key = *(this->operands.begin());
				this->operands.clear();
				delete this;
			}
			else if (this->operands.size() == 0) {	//this only consists of value -> layer is not needed and this is deleted
				storage_key = new Value(this->value);
				delete this;
			}
		}

		template<void(*operate)(std::complex<double>* first, const std::complex<double>second), Type this_type, int neutral_val>
		inline Vals_Combined Variadic_Operator<operate, this_type, neutral_val>::combine_values()
		{
			bool only_known = true;

			for (auto it = this->operands.begin(); it != this->operands.end();) {
				const auto [it_known, it_val] = (*it)->combine_values();
				if (it_known) {
					operate(&(this->value.val()), it_val);
					delete (*it);
					it = this->operands.erase(it);
				}
				else {
					only_known = false;
					++it;
				}
			}
			return { only_known, this->value.val() };
		}

		template<void(*operate)(std::complex<double>* first, const std::complex<double>second), Type this_type, int neutral_val>
		inline std::complex<double> Variadic_Operator<operate, this_type, neutral_val>::evaluate(const std::list<Known_Variable>& known_variables) const
		{
			std::complex<double> result = this->value.val();
			for (const auto it : this->operands) {
				operate(&result, it->evaluate(known_variables));
			}
			return result;
		}

		template<void(*operate)(std::complex<double>* first, const std::complex<double>second), Type this_type, int neutral_val>
		inline void Variadic_Operator<operate, this_type, neutral_val>::search_and_replace(const std::string& name_, const Basic_Term* value_, Basic_Term*& storage_key)
		{
			for (auto& it : this->operands) {	//reference is needed in next line
				it->search_and_replace(name_, value_, it);
			}
		}

		template<void(*operate)(std::complex<double>* first, const std::complex<double>second), Type this_type, int neutral_val>
		inline void Variadic_Operator<operate, this_type, neutral_val>::for_each(std::function<void(Basic_Term* this_ptr, Type this_type)> func)
		{
			for (auto it : this->operands) {
				it->for_each(func);
			}
			func(this, this_type);
		}

		template<void(*operate)(std::complex<double>* first, const std::complex<double>second), Type this_type, int neutral_val>
		inline Basic_Term** Variadic_Operator<operate, this_type, neutral_val>::match_intern(Basic_Term* pattern, std::list<Pattern_Variable*>& pattern_var_adresses, Basic_Term** storage_key)
		{
			if (this->equal_to_pattern(pattern, nullptr, storage_key)) {
				return storage_key;
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
			reset_all_pattern_vars(pattern_var_adresses);
			return nullptr;
		}

		template<void(*operate)(std::complex<double>* first, const std::complex<double>second), Type this_type, int neutral_val>
		inline bool Variadic_Operator<operate, this_type, neutral_val>::equal_to_pattern(Basic_Term* pattern, Basic_Term* patterns_parent, Basic_Term** storage_key)
		{
			const Type pattern_type = type_of(pattern);
			if (pattern_type == this_type) {
				const Variadic_Operator<operate, this_type, neutral_val>* variadic_pattern = static_cast<const Variadic_Operator<operate, this_type, neutral_val>*>(pattern);
				//HIER MUSS NOCH TEST HIN, OB PATTERN NEUTRALES ELEMENT ALS VALUE HAT. 
				if (this->operands.size() != variadic_pattern->operands.size()) {
					return false;
				}
				if (this->value.val() != variadic_pattern->value.val()) {
					return false;
				}
				//the operator assumes from now on to have sorted products to compare
				auto& this_it = this->operands.begin();	//reference, because equal_to_pattern() needs the storage position of operands
				auto pattern_it = variadic_pattern->operands.begin();
				for (; this_it != this->operands.end() && pattern_it != variadic_pattern->operands.end(); ++this_it, ++pattern_it) {
					if ( !(*this_it)->equal_to_pattern(*pattern_it, pattern, &*this_it) ) {
						return false;
					}
				}
				return true;
			}
			else if (pattern_type == Type::pattern_variable) {
				Pattern_Variable* pattern_var = static_cast<Pattern_Variable*>(pattern);
				return pattern_var->try_matching(this, patterns_parent, storage_key);
			}
			else {
				return false;
			}
		}

		template<void(*operate)(std::complex<double>* first, const std::complex<double>second), Type this_type, int neutral_val>
		inline bool Variadic_Operator<operate, this_type, neutral_val>::reset_own_matches(Basic_Term* parent)
		{
			//NOCH NICHT FERTIG - bisher ist resettable immer true
			bool resettable = true;
			for (auto it : this->operands) {
				resettable &= it->reset_own_matches(this);
			}
			return resettable;
		}

		template<void(*operate)(std::complex<double>* first, const std::complex<double>second), Type this_type, int neutral_val>
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
				if (this->value.val().real() != other_product->value.val().real()) {
					return this->value.val().real() < other_product->value.val().real();
				}
				if (this->value.val().imag() != other_product->value.val().imag()) {
					return this->value.val().imag() < other_product->value.val().imag();
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

		template<void(*operate)(std::complex<double>* first, const std::complex<double>second), Type this_type, int neutral_val>
		inline bool Variadic_Operator<operate, this_type, neutral_val>::operator==(const Basic_Term& other) const
		{
			if (type_of(other) == this_type) {
				const Variadic_Operator<operate, this_type, neutral_val>* other_variadic = static_cast<const Variadic_Operator<operate, this_type, neutral_val>*>(&other);
				if (this->operands.size() != other_variadic->operands.size()) {
					return false;
				}
				if (this->value.val() != other_variadic->value.val()) {
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

		template<void(*operate)(std::complex<double>* first, const std::complex<double>second), Type this_type, int neutral_val>
		inline Basic_Term** Variadic_Operator<operate, this_type, neutral_val>::part_equal_to_pattern(Basic_Term* pattern, Basic_Term* patterns_parent, Basic_Term** storage_key)
		{
			const Type pattern_type = type_of(pattern);
			if (pattern_type == this_type) {
				Variadic_Operator<operate, this_type, neutral_val>* const pattern_variadic = static_cast<Variadic_Operator<operate, this_type, neutral_val>*>(pattern);
				if (pattern_variadic->operands.size() > this->operands.size()) {
					return nullptr;
				}
				else {
					//HIER MUSS NOCH INHALT HIN!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				}				
			}
			else if (pattern_type == Type::pattern_variable) {
				Pattern_Variable* pattern_var = static_cast<Pattern_Variable*>(pattern);
				if (pattern_var->try_matching(this, patterns_parent, storage_key )) {
					return storage_key;
				}
			}
			return nullptr;
		}

		template<void(*operate)(std::complex<double>* first, const std::complex<double>second), Type this_type, int neutral_val>
		inline void Variadic_Operator<operate, this_type, neutral_val>::copy_into_operands(Basic_Term* source)
		{
			if (type_of(source) == Type::value) {
				const Value* const val_source = static_cast<Value*>(source);
				operate(&(this->value.val()), val_source->val());
			}
			else {
				this->operands.push_back(copy_subterm(source));
			}
		}

		template<void(*operate)(std::complex<double>* first, const std::complex<double>second), Type this_type, int neutral_val>
		inline void Variadic_Operator<operate, this_type, neutral_val>::move_into_operands(Basic_Term* term_ptr)
		{
			if (type_of(term_ptr) == Type::value) {
				Value* const val_ptr = static_cast<Value*>(term_ptr);
				operate(&(this->value.val()), val_ptr->val());
				delete val_ptr;
			}
			else {
				this->operands.push_back(term_ptr);
			}
		}

} //namespace intern
} //namespace bmath
