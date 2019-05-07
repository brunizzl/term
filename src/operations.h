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
			Product(const Product& source, Basic_Term* parent_ = nullptr);
			~Product();

			void to_str(std::string& str) const override;
			State get_state_intern() const override;
			void combine_layers() override;
			Vals_Combined combine_values() override;
			Vals_Combined evaluate(const std::string& name_, std::complex<double> value_) const override;
			bool search_and_replace(const std::string& name_, std::complex<double> value_) override;
			bool valid_state() const override;
		};


		class Sum : public Basic_Term
		{
		public:
			std::list<Basic_Term*> summands;
			std::list<Basic_Term*> subtractors;

			Sum(Basic_Term* parent_);
			Sum(std::string name_, Basic_Term* parent_, std::size_t op);
			Sum(const Sum& source, Basic_Term* parent_ = nullptr);
			~Sum();

			void to_str(std::string& str) const override;
			State get_state_intern() const override;
			void combine_layers() override;
			Vals_Combined combine_values() override;
			Vals_Combined evaluate(const std::string& name_, std::complex<double> value_) const override;
			bool search_and_replace(const std::string& name_, std::complex<double> value_) override;
			bool valid_state() const override;
		};


		class Exponentiation : public Basic_Term
		{
		public:
			Basic_Term* exponent;
			Basic_Term* base;

			Exponentiation(Basic_Term* parent_);
			Exponentiation(std::string name_, Basic_Term* parent_, std::size_t op);
			Exponentiation(const Exponentiation& source, Basic_Term* parent_ = nullptr);
			~Exponentiation();

			void to_str(std::string& str) const override;
			State get_state_intern() const override;
			void combine_layers() override;
			Vals_Combined combine_values() override;
			Vals_Combined evaluate(const std::string& name_, std::complex<double> value_) const override;
			bool search_and_replace(const std::string& name_, std::complex<double> value_) override;
			bool valid_state() const override;
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
			Par_Operator(const Par_Operator& source, Basic_Term* parent_ = nullptr);
			~Par_Operator();

			void to_str(std::string& str) const override;
			State get_state_intern() const override;
			void combine_layers() override;
			Vals_Combined combine_values() override;
			Vals_Combined evaluate(const std::string& name_, std::complex<double> value_) const override;
			bool search_and_replace(const std::string& name_, std::complex<double> value_) override;
			bool valid_state() const override;

			friend class bmath::Term;
		};

	} //namespace intern
} //namespace bmath

