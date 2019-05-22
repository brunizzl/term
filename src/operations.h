#pragma once

#include "term.h"

namespace bmath {
	namespace intern {

		class Product : public Basic_Term
		{
		public:
			std::list<Basic_Term*> factors;
			std::list<Basic_Term*> divisors;

			Product(Basic_Term* parent_);
			Product(std::string name_, Basic_Term* parent_, std::size_t op);
			Product(std::string name_, Basic_Term* parent_, std::size_t op, std::list<Basic_Term*>& variables);
			Product(const Product& source, Basic_Term* parent_ = nullptr);
			~Product();

			void to_str(std::string& str) const override;
			State get_state_intern() const override;
			void combine_layers(Basic_Term*& storage_key) override;
			Vals_Combined combine_values() override; 
			Vals_Combined evaluate(const std::list<Known_Variable>& known_variables) const override;
			void search_and_replace(const std::string& name_, std::complex<double> value_, Basic_Term*& storage_key) override;
			bool valid_state() const override;
			void list_subterms(std::list<Basic_Term*>& subterms, State listed_state) const override;
			void sort() override;
			Basic_Term** match_intern(Basic_Term* pattern, std::list<Basic_Term*>& pattern_var_adresses, Basic_Term** storage_key) override;	//NOCH NICHT GANZ IMPLEMENTIERT!
			bool operator<(const Basic_Term& other) const override;
			bool operator==(const Basic_Term& other) const override;
		};


		class Sum : public Basic_Term
		{
		public:
			std::list<Basic_Term*> summands;
			std::list<Basic_Term*> subtractors;

			Sum(Basic_Term* parent_);
			Sum(std::string name_, Basic_Term* parent_, std::size_t op);
			Sum(std::string name_, Basic_Term* parent_, std::size_t op, std::list<Basic_Term*>& variables);
			Sum(const Sum& source, Basic_Term* parent_ = nullptr);
			~Sum();

			void to_str(std::string& str) const override;
			State get_state_intern() const override;
			void combine_layers(Basic_Term*& storage_key) override;
			Vals_Combined combine_values() override;
			Vals_Combined evaluate(const std::list<Known_Variable>& known_variables) const override;
			void search_and_replace(const std::string& name_, std::complex<double> value_, Basic_Term*& storage_key) override;
			bool valid_state() const override;
			void list_subterms(std::list<Basic_Term*>& subterms, State listed_state) const override;
			void sort() override;
			Basic_Term** match_intern(Basic_Term* pattern, std::list<Basic_Term*>& pattern_var_adresses, Basic_Term** storage_key) override;	//NOCH NICHT GANZ IMPLEMENTIERT!
			bool operator<(const Basic_Term& other) const override;
			bool operator==(const Basic_Term& other) const override;
		};


		class Exponentiation : public Basic_Term
		{
		public:
			Basic_Term* exponent;
			Basic_Term* base;

			Exponentiation(Basic_Term* parent_);
			Exponentiation(std::string name_, Basic_Term* parent_, std::size_t op);
			Exponentiation(std::string name_, Basic_Term* parent_, std::size_t op, std::list<Basic_Term*>& variables);
			Exponentiation(const Exponentiation& source, Basic_Term* parent_ = nullptr);
			~Exponentiation();

			void to_str(std::string& str) const override;
			State get_state_intern() const override;
			void combine_layers(Basic_Term*& storage_key) override;
			Vals_Combined combine_values() override;
			Vals_Combined evaluate(const std::list<Known_Variable>& known_variables) const override;
			void search_and_replace(const std::string& name_, std::complex<double> value_, Basic_Term*& storage_key) override;
			bool valid_state() const override;
			void list_subterms(std::list<Basic_Term*>& subterms, State listed_state) const override;
			void sort() override;
			Basic_Term** match_intern(Basic_Term* pattern, std::list<Basic_Term*>& pattern_var_adresses, Basic_Term** storage_key) override;
			bool operator<(const Basic_Term& other) const override;
			bool operator==(const Basic_Term& other) const override;
		};


		class Par_Operator : public Basic_Term
		{
		private:
			Par_Op_State op_state;
			Basic_Term* argument;

			Vals_Combined internal_combine(Vals_Combined argument_) const;

		public:
			Par_Operator(Basic_Term* parent_);
			Par_Operator(std::string name_, Basic_Term* parent_, Par_Op_State op_state_);
			Par_Operator(std::string name_, Basic_Term* parent_, Par_Op_State op_state_, std::list<Basic_Term*>& variables);
			Par_Operator(const Par_Operator& source, Basic_Term* parent_ = nullptr);
			~Par_Operator();

			void to_str(std::string& str) const override;
			State get_state_intern() const override;
			void combine_layers(Basic_Term*& storage_key) override;
			Vals_Combined combine_values() override;
			Vals_Combined evaluate(const std::list<Known_Variable>& known_variables) const override;
			void search_and_replace(const std::string& name_, std::complex<double> value_, Basic_Term*& storage_key) override;
			bool valid_state() const override;
			void list_subterms(std::list<Basic_Term*>& subterms, State listed_state) const override;
			void sort() override;
			Basic_Term** match_intern(Basic_Term* pattern, std::list<Basic_Term*>& pattern_var_adresses, Basic_Term** storage_key) override;
			bool operator<(const Basic_Term& other) const override;
			bool operator==(const Basic_Term& other) const override;
		};

		//used to store remaining summands / factors marked in pattern with "..." (basically is sum or product)
		class Variadic_Pattern_Operator : public Basic_Term
		{
		public:
			bool is_product;	//true -> is product, false -> is sum
			mutable std::list<Basic_Term*> operands;		//summands or factors		(the "...")
			mutable std::list<Basic_Term*> inv_operands;	//subtractos or divisors

			std::list<Pattern_Variable*> pattern_operands;		//these lists contain explicit written pattern_vaiables 
			std::list<Pattern_Variable*> inv_pattern_operands;	// (everything but "...")

			Variadic_Pattern_Operator(Basic_Term* parent_);
			~Variadic_Pattern_Operator();

			void to_str(std::string& str) const override;
			State get_state_intern() const override;
			Vals_Combined combine_values() override;
			Vals_Combined evaluate(const std::list<Known_Variable>& known_variables) const override;
			void search_and_replace(const std::string& name_, std::complex<double> value_, Basic_Term*& storage_key) override;
			bool valid_state() const override;
			void list_subterms(std::list<Basic_Term*>& subterms, State listed_state) const override;
			void sort() override;
			Basic_Term** match_intern(Basic_Term* pattern, std::list<Basic_Term*>& pattern_var_adresses, Basic_Term** storage_key) override;
			bool operator<(const Basic_Term& other) const override;
			bool operator==(const Basic_Term& other) const override;	//DER BRAUCHT NO NN BISSEL ÜBERLEGUNG
		};

	} //namespace intern
} //namespace bmath
