
#pragma once

#include <string>
#include <string_view>
#include <list>
#include <vector>
#include <complex>
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
			Basic_Term* parent_ptr;
			std::list<Basic_Term*> operands;	//summands in sum, factors in product
			Value value_operand;				//only one summand / factor is allowed to be of Type::value. value_operand is that.

			Variadic_Operator(Basic_Term* parent_);
			Variadic_Operator(const Variadic_Operator& source, Basic_Term* parent_);
			~Variadic_Operator();
		public:

			Basic_Term* parent() const override;
			void set_parent(Basic_Term* new_parent) override;
			Type get_type() const override;
			void combine_layers(Basic_Term*& storage_key) override;
			Vals_Combined combine_values() override;
			std::complex<double> evaluate(const std::list<Known_Variable>& known_variables) const override;
			void search_and_replace(const std::string& name_, const Basic_Term* value_, Basic_Term*& storage_key) override;
			void list_subterms(std::list<const Basic_Term*>& subterms, Type listed_type) const override;
			void sort() override;
			Basic_Term** match_intern(Basic_Term* pattern, std::list<Pattern_Variable*>& pattern_var_adresses, Basic_Term** storage_key) override;
			bool operator<(const Basic_Term& other) const override;
			bool operator==(const Basic_Term& other) const override;
		};

		class Sum : public Variadic_Operator<add, Type::sum, 0>
		{
		private:

			friend class bmath::Term;
		public:
			Sum(Basic_Term* parent_);
			Sum(std::string_view name_, Basic_Term* parent_, std::size_t op);
			Sum(std::string_view name_, Basic_Term* parent_, std::size_t op, std::list<Pattern_Variable*>& variables);
			Sum(const Sum& source, Basic_Term* parent_ = nullptr);

			void to_str(std::string& str) const override;
			void to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const override;
		};


		class Product : public Variadic_Operator<mul, Type::product, 1>
		{
		private:

			friend class bmath::Term;
		public:
			Product(Basic_Term* parent_);
			Product(std::string_view name_, Basic_Term* parent_, std::size_t op);
			Product(Basic_Term* name_, Basic_Term* parent_, std::complex<double> factor);
			Product(std::string_view name_, Basic_Term* parent_, std::size_t op, std::list<Pattern_Variable*>& variables);
			Product(const Product& source, Basic_Term* parent_ = nullptr);

			void to_str(std::string& str) const override;
			void to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const override;
		};


		class Exponentiation : public Basic_Term
		{
		private:
			Basic_Term* parent_ptr;
			Basic_Term* exponent;
			Basic_Term* base;

			friend class bmath::Term;

		public:
			Exponentiation(Basic_Term* parent_);
			Exponentiation(std::string_view name_, Basic_Term* parent_, std::size_t op);
			Exponentiation(Basic_Term* base_, Basic_Term* parent_, std::complex<double> exponent_);
			Exponentiation(std::string_view name_, Basic_Term* parent_, std::size_t op, std::list<Pattern_Variable*>& variables);
			Exponentiation(const Exponentiation& source, Basic_Term* parent_ = nullptr);
			~Exponentiation();

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
			bool operator<(const Basic_Term& other) const override;
			bool operator==(const Basic_Term& other) const override;
		};


		class Par_Operator : public Basic_Term
		{
		private:
			Basic_Term* parent_ptr;
			Par_Op_Type op_type;
			Basic_Term* argument;

		public:
			Par_Operator(Basic_Term* parent_);
			Par_Operator(std::string_view name_, Basic_Term* parent_, Par_Op_Type op_type_);
			Par_Operator(std::string_view name_, Basic_Term* parent_, Par_Op_Type op_type_, std::list<Pattern_Variable*>& variables);
			Par_Operator(const Par_Operator& source, Basic_Term* parent_ = nullptr);
			~Par_Operator();

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
			bool operator<(const Basic_Term& other) const override;
			bool operator==(const Basic_Term& other) const override;
		};




		template<void(*operate)(std::complex<double>* first, const std::complex<double>second), Type this_type, int neutral_val>
		inline Variadic_Operator<operate, this_type, neutral_val>::Variadic_Operator(Basic_Term* parent_)
			:parent_ptr(parent_), value_operand({ static_cast<double>(neutral_val), 0 }, this)
		{
		}

		template<void(*operate)(std::complex<double>* first, const std::complex<double>second), Type this_type, int neutral_val>
		inline Variadic_Operator<operate, this_type, neutral_val>::Variadic_Operator(const Variadic_Operator& source, Basic_Term* parent_)
			:parent_ptr(parent_), value_operand({ source.value_operand.real(), source.value_operand.imag() }, this)
		{
			for (const auto it : source.operands) {
				this->operands.push_back(copy_subterm(it, this));
			}
		}

		template<void(*operate)(std::complex<double>* first, const std::complex<double>second), Type this_type, int neutral_val>
		inline Variadic_Operator<operate, this_type, neutral_val>::~Variadic_Operator()
		{
			for (const auto it : this->operands) {
				delete it;
			}
		}

		template<void(*operate)(std::complex<double>* first, const std::complex<double>second), Type this_type, int neutral_val>
		inline Basic_Term* Variadic_Operator<operate, this_type, neutral_val>::parent() const
		{
			return this->parent_ptr;
		}

		template<void(*operate)(std::complex<double>* first, const std::complex<double>second), Type this_type, int neutral_val>
		inline void Variadic_Operator<operate, this_type, neutral_val>::set_parent(Basic_Term* new_parent)
		{
			this->parent_ptr = new_parent;
		}

		template<void(*operate)(std::complex<double>* first, const std::complex<double>second), Type this_type, int neutral_val>
		inline Type Variadic_Operator<operate, this_type, neutral_val>::get_type() const
		{
			return this_type;
		}

		template<void(*operate)(std::complex<double>* first, const std::complex<double>second), Type this_type, int neutral_val>
		inline void Variadic_Operator<operate, this_type, neutral_val>::combine_layers(Basic_Term*& storage_key)
		{
			for (auto& it = this->operands.begin(); it != this->operands.end();) {	//reference is needed in next line
				(*it)->combine_layers(*it);
				if (type(*it) == this_type) {
					Variadic_Operator<operate, this_type, neutral_val>* redundant = static_cast<Variadic_Operator<operate, this_type, neutral_val>*>((*it));
					this->value_operand *= redundant->value_operand;
					for (auto it_red : redundant->operands) {
						it_red->set_parent(this);
					}
					this->operands.splice(this->operands.end(), redundant->operands);
					delete redundant;
					it = this->operands.erase(it);
				}
				else {
					++it;
				}
			}
			if (this->operands.size() == 1 && this->value_operand == static_cast<double>(neutral_val)) {	//this only consists of one non- neutral operand -> layer is not needed and this is deleted
				Basic_Term* const only_operand = *(this->operands.begin());
				storage_key = only_operand;
				only_operand->set_parent(this->parent_ptr);
				this->operands.clear();
				delete this;
			}
			else if (this->operands.size() == 0) {	//this only consists of value_operand -> layer is not needed and this is deleted
				Value* const only_operand = new Value(this->value_operand, this->parent_ptr);
				storage_key = only_operand;
				delete this;
			}
		}

		template<void(*operate)(std::complex<double>* first, const std::complex<double>second), Type this_type, int neutral_val>
		inline Vals_Combined Variadic_Operator<operate, this_type, neutral_val>::combine_values()
		{
			bool only_known = true;

			for (auto it = this->operands.begin(); it != this->operands.end();) {
				const auto [known, operand] = (*it)->combine_values();
				if (known) {
					operate(&(this->value_operand), operand);
					delete (*it);
					it = this->operands.erase(it);
				}
				else {
					only_known = false;
					++it;
				}
			}
			return { only_known, this->value_operand };
		}

		template<void(*operate)(std::complex<double>* first, const std::complex<double>second), Type this_type, int neutral_val>
		inline std::complex<double> Variadic_Operator<operate, this_type, neutral_val>::evaluate(const std::list<Known_Variable>& known_variables) const
		{
			std::complex<double> result = this->value_operand;
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
		inline void Variadic_Operator<operate, this_type, neutral_val>::list_subterms(std::list<const Basic_Term*>& subterms, Type listed_type) const
		{
			if (listed_type == this_type) {
				subterms.push_back(this);
			}
			else if (listed_type == Type::value) {
				subterms.push_back(&this->value_operand);
			}
			for (auto it : this->operands) {
				it->list_subterms(subterms, listed_type);
			}
		}

		template<void(*operate)(std::complex<double>* first, const std::complex<double>second), Type this_type, int neutral_val>
		inline void Variadic_Operator<operate, this_type, neutral_val>::sort()
		{
			for (auto it : this->operands) {
				it->sort();
			}
			this->operands.sort([](Basic_Term*& a, Basic_Term*& b) -> bool {return *a < *b; });
		}

		template<void(*operate)(std::complex<double>* first, const std::complex<double>second), Type this_type, int neutral_val>
		inline Basic_Term** Variadic_Operator<operate, this_type, neutral_val>::match_intern(Basic_Term* pattern, std::list<Pattern_Variable*>& pattern_var_adresses, Basic_Term** storage_key)
		{
			if (*this == *pattern) {
				return storage_key;
			}
			else {
				Basic_Term** argument_match;
				for (auto& it : this->operands) {	//references are important, because we want to return the position of it.
					reset_pattern_vars(pattern_var_adresses);
					argument_match = it->match_intern(pattern, pattern_var_adresses, &it);
					if (argument_match) {
						return argument_match;
					}
				}
			}
			reset_pattern_vars(pattern_var_adresses);
			return nullptr;
		}

		template<void(*operate)(std::complex<double>* first, const std::complex<double>second), Type this_type, int neutral_val>
		inline bool Variadic_Operator<operate, this_type, neutral_val>::operator<(const Basic_Term& other) const
		{
			if (this_type != other.get_type()) {
				return this_type < other.get_type();
			}
			else {
				const Variadic_Operator<operate, this_type, neutral_val>* other_product = static_cast<const Variadic_Operator<operate, this_type, neutral_val>*>(&other);
				if (this->operands.size() != other_product->operands.size()) {
					return this->operands.size() < other_product->operands.size();
				}
				if (this->value_operand.real() != other_product->value_operand.real()) {
					return this->value_operand.real() < other_product->value_operand.real();
				}
				if (this->value_operand.imag() != other_product->value_operand.imag()) {
					return this->value_operand.imag() < other_product->value_operand.imag();
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
			switch (other.get_type()) {
			case this_type:
				break;
			case Type::pattern_variable:
				return other == *this;
			default:
				return false;
			}
			const Variadic_Operator<operate, this_type, neutral_val>* other_variadic = static_cast<const Variadic_Operator<operate, this_type, neutral_val>*>(&other);
			if (this->operands.size() != other_variadic->operands.size()) {
				return false;
			}
			if (this->value_operand.real() != other_variadic->value_operand.real()) {
				return false;
			}
			if (this->value_operand.imag() != other_variadic->value_operand.imag()) {
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

} //namespace intern
} //namespace bmath
