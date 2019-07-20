#pragma once

#include "structs.h"
#include "term.h"

namespace bmath {
	namespace intern {

		class Product : public Basic_Term
		{
		private:
			Product(Basic_Term* parent_);
			Product(std::string name_, Basic_Term* parent_, std::size_t op);
			Product(std::string name_, Basic_Term* parent_, std::size_t op, std::list<Pattern_Variable*>& variables);
			Product(const Product& source, Basic_Term* parent_ = nullptr);

			//access to constructors:
			friend Basic_Term* build_subterm(std::string& subtermstr, Basic_Term* parent_);
			friend Basic_Term* build_pattern_subterm(std::string& subtermstr, Basic_Term* parent_, std::list<Pattern_Variable*>& variables);
			friend Basic_Term* copy_subterm(const Basic_Term* source, Basic_Term* parent_);
			friend Basic_Term* standardize_structure(Basic_Term* obj);
			friend class bmath::Term;
		public:
			std::list<Basic_Term*> factors;
			std::list<Basic_Term*> divisors;

			~Product();

			void to_str(std::string& str) const override;
			State get_state_intern() const override;
			void combine_layers(Basic_Term*& storage_key) override;
			Vals_Combined combine_values() override;
			std::complex<double> evaluate(const std::list<Known_Variable>& known_variables) const override;
			void search_and_replace(const std::string& name_, const Basic_Term* value_, Basic_Term*& storage_key) override;
			void list_subterms(std::list<Basic_Term*>& subterms, State listed_state) const override;
			void sort() override;
			Basic_Term** match_intern(Basic_Term* pattern, std::list<Pattern_Variable*>& pattern_var_adresses, Basic_Term** storage_key) override;	//NOCH NICHT GANZ IMPLEMENTIERT!
			bool operator<(const Basic_Term& other) const override;
			bool operator==(const Basic_Term& other) const override;
		};


		class Sum : public Basic_Term
		{
		private:
			Sum(Basic_Term* parent_);
			Sum(std::string name_, Basic_Term* parent_, std::size_t op);
			Sum(std::string name_, Basic_Term* parent_, std::size_t op, std::list<Pattern_Variable*>& variables);
			Sum(const Sum& source, Basic_Term* parent_ = nullptr);

			//access to constructors:
			friend Basic_Term* build_subterm(std::string& subtermstr, Basic_Term* parent_);
			friend Basic_Term* build_pattern_subterm(std::string& subtermstr, Basic_Term* parent_, std::list<Pattern_Variable*>& variables);
			friend Basic_Term* copy_subterm(const Basic_Term* source, Basic_Term* parent_);
			friend Basic_Term* standardize_structure(Basic_Term* obj);
			friend class bmath::Term;
		public:
			std::list<Basic_Term*> summands;
			std::list<Basic_Term*> subtractors;

			~Sum();

			void to_str(std::string& str) const override;
			State get_state_intern() const override;
			void combine_layers(Basic_Term*& storage_key) override;
			Vals_Combined combine_values() override;
			std::complex<double> evaluate(const std::list<Known_Variable>& known_variables) const override;
			void search_and_replace(const std::string& name_, const Basic_Term* value_, Basic_Term*& storage_key) override;
			void list_subterms(std::list<Basic_Term*>& subterms, State listed_state) const override;
			void sort() override;
			Basic_Term** match_intern(Basic_Term* pattern, std::list<Pattern_Variable*>& pattern_var_adresses, Basic_Term** storage_key) override;	//NOCH NICHT GANZ IMPLEMENTIERT!
			bool operator<(const Basic_Term& other) const override;
			bool operator==(const Basic_Term& other) const override;
		};


		class Exponentiation : public Basic_Term
		{
		private:
			Exponentiation(Basic_Term* parent_);
			Exponentiation(std::string name_, Basic_Term* parent_, std::size_t op);
			Exponentiation(std::string name_, Basic_Term* parent_, std::size_t op, std::list<Pattern_Variable*>& variables);
			Exponentiation(const Exponentiation& source, Basic_Term* parent_ = nullptr);

			//access to constructors:
			friend Basic_Term* build_subterm(std::string& subtermstr, Basic_Term* parent_);
			friend Basic_Term* build_pattern_subterm(std::string& subtermstr, Basic_Term* parent_, std::list<Pattern_Variable*>& variables);
			friend Basic_Term* copy_subterm(const Basic_Term* source, Basic_Term* parent_);
		public:
			Basic_Term* exponent;
			Basic_Term* base;

			~Exponentiation();

			void to_str(std::string& str) const override;
			State get_state_intern() const override;
			void combine_layers(Basic_Term*& storage_key) override;
			Vals_Combined combine_values() override;
			std::complex<double> evaluate(const std::list<Known_Variable>& known_variables) const override;
			void search_and_replace(const std::string& name_, const Basic_Term* value_, Basic_Term*& storage_key) override;
			void list_subterms(std::list<Basic_Term*>& subterms, State listed_state) const override;
			void sort() override;
			Basic_Term** match_intern(Basic_Term* pattern, std::list<Pattern_Variable*>& pattern_var_adresses, Basic_Term** storage_key) override;
			bool operator<(const Basic_Term& other) const override;
			bool operator==(const Basic_Term& other) const override;
		};


		class Par_Operator : public Basic_Term
		{
		private:
			Par_Op_State op_state;
			Basic_Term* argument;

			Vals_Combined internal_combine(Vals_Combined argument_) const;

			Par_Operator(Basic_Term* parent_);
			Par_Operator(std::string name_, Basic_Term* parent_, Par_Op_State op_state_);
			Par_Operator(std::string name_, Basic_Term* parent_, Par_Op_State op_state_, std::list<Pattern_Variable*>& variables);
			Par_Operator(const Par_Operator& source, Basic_Term* parent_ = nullptr);

			//access to constructors:
			friend Basic_Term* build_subterm(std::string& subtermstr, Basic_Term* parent_);
			friend Basic_Term* build_pattern_subterm(std::string& subtermstr, Basic_Term* parent_, std::list<Pattern_Variable*>& variables);
			friend Basic_Term* copy_subterm(const Basic_Term* source, Basic_Term* parent_);
		public:
			~Par_Operator();

			void to_str(std::string& str) const override;
			State get_state_intern() const override;
			void combine_layers(Basic_Term*& storage_key) override;
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
