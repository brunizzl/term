
#include "term.h"
#include "internal_functions.h"
#include "arguments.h"
#include "operations.h"

using namespace bmath::intern;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Basic_Term\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Basic_Term::~Basic_Term()
{
	//cleaning up the tree is done in derived classes
}

std::string bmath::intern::Basic_Term::debug_print() const
{
	std::string str;
	this->to_str(str, 0);
	return str;
}

bool Basic_Term::operator!=(const Basic_Term& other) const
{
	return !(*this == other);
}

void bmath::intern::Basic_Term::sort()
{
	this->for_each([](Basic_Term* this_ptr, Type this_type) { 
		if (this_type == Type::sum) {
			Sum* const this_sum = static_cast<Sum*>(this_ptr);
			this_sum->sort_operands();
		}
		else if (this_type == Type::product) {
			Product* const this_product = static_cast<Product*>(this_ptr);
			this_product->sort_operands();
		}
	});
}

std::list<Basic_Term*> bmath::intern::Basic_Term::list_subterms(Type requested_type)
{
	std::list<Basic_Term*> erg_list;
	this->for_each([&erg_list, requested_type](Basic_Term* this_ptr, Type this_type) {
		if (this_type == requested_type) {
			erg_list.push_back(this_ptr);
		}
	});
	return erg_list;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Term\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bmath::Term::Term()
	:term_ptr(nullptr)
{
}

bmath::Term::Term(std::string name_)
	:term_ptr(nullptr)
{
	preprocess_str(name_);
	this->term_ptr = build_subterm({ name_.data(), name_.length() });
}

bmath::Term::Term(std::complex<double> val) 
	:term_ptr(new Value(val))
{}

bmath::Term::Term(const Term& source)
	:term_ptr(copy_subterm(source.term_ptr))
{
}

bmath::Term::Term(Term&& source) noexcept
	:term_ptr(std::exchange(source.term_ptr, nullptr))
{
}

bmath::Term& bmath::Term::operator=(const Term& source)
{
	if (this != &source) {
		delete this->term_ptr;
		this->term_ptr = copy_subterm(source.term_ptr);
	}
	return *this;
}

bmath::Term& bmath::Term::operator=(Term&& source) noexcept
{
	if (this != &source) {
		delete this->term_ptr;
		this->term_ptr = std::exchange(source.term_ptr, nullptr);
	}
	return *this;
}

bmath::Term::~Term()
{
	delete term_ptr;
}

std::string bmath::Term::to_str() const
{
	std::string str;
	this->term_ptr->to_str(str, operator_precedence(Type::undefined));
	return str;
}

std::string bmath::Term::to_tree(std::size_t offset) const
{
	return ptr_to_tree(this->term_ptr, offset);
}

std::list<std::string> bmath::Term::get_var_names() const
{
	std::list<std::string> names;
	this->term_ptr->for_each([&names](Basic_Term* this_ptr, Type this_type) {
		if (this_type == Type::variable) {
			const Variable* const var_ptr = static_cast<const Variable*>(this_ptr);
			names.push_back(var_ptr->name);
		}
	});
	return names;
}

void bmath::Term::combine_values()
{
	if (type_of(this->term_ptr) != Type::value) {
		auto [is_val, val] = this->term_ptr->combine_values();
		if (is_val) {
			delete this->term_ptr;
			this->term_ptr = new Value(val);
		}
	}
}

std::complex<double> bmath::Term::evaluate(const std::string& name_, std::complex<double> value_) const
{
	std::list<Known_Variable> known_variables{ Known_Variable{ name_, value_ } };
	return this->term_ptr->evaluate(known_variables);
}

std::complex<double> bmath::Term::evaluate(const std::list<Known_Variable>& known_variables) const
{
	return this->term_ptr->evaluate(known_variables);
}

void bmath::Term::search_and_replace(const std::string& name_, std::complex<double> value_)
{
	const Value value_term(value_);
	this->term_ptr->search_and_replace(name_, &value_term, this->term_ptr);
}

void bmath::Term::search_and_replace(const std::string& name_, const bmath::Term& value_)
{
	this->term_ptr->search_and_replace(name_, value_.term_ptr, this->term_ptr);
}

void bmath::Term::simplify()
{
	do {
		this->term_ptr->combine_layers(this->term_ptr);
		this->combine_values();
		this->term_ptr->sort();
	} while (this->term_ptr->transform(this->term_ptr));
	this->term_ptr->combine_layers(this->term_ptr);
}

void bmath::Term::cut_rounding_error(int pow_of_10_diff_to_set_0)
{
	double quadratic_sum = 0;	//only considers real part of vaues
	std::size_t number_summands = 0;
	this->term_ptr->for_each([&](Basic_Term* this_ptr, Type this_type) {
		if (this_type == Type::value) {
			double real_part = static_cast<Value*>(this_ptr)->val().real();
			if (abs(real_part) != 1.0) {	//exponent of -1 and factor of -1 should not interfer
				quadratic_sum += real_part * real_part;
				number_summands++;
			}
		}
	});
	if (quadratic_sum != 0) {
		const double quadratic_average = std::sqrt(quadratic_sum / number_summands);	//equal to standard deviation from 0
		const double limit_to_0 = quadratic_average * std::pow(10, -pow_of_10_diff_to_set_0);
		this->term_ptr->for_each([=](Basic_Term* this_ptr, Type this_type) {
			if (this_type == Type::value) {
				std::complex<double>& val = static_cast<Value*>(this_ptr)->val();
				if (abs(val.real()) < limit_to_0) {
					val.real(0);
				}
				if (abs(val.imag()) < limit_to_0) {
					val.imag(0);
				}
			}
		});
	}
}

bmath::Term& bmath::Term::operator+=(const Term& operand2)
{
	if (type_of(this->term_ptr) == Type::value && type_of(operand2.term_ptr) == Type::value) {
		static_cast<Value*>(this->term_ptr)->val() += static_cast<Value*>(operand2.term_ptr)->val();
	}
	else {
		Sum* const sum = new Sum;
		sum->push_back(this->term_ptr);
		sum->push_back(copy_subterm(operand2.term_ptr));
		this->term_ptr = sum;
		this->simplify();
	}
	return *this;
}

bmath::Term& bmath::Term::operator-=(const Term& operand2)
{
	if (type_of(this->term_ptr) == Type::value && type_of(operand2.term_ptr) == Type::value) {
		static_cast<Value*>(this->term_ptr)->val() -= static_cast<Value*>(operand2.term_ptr)->val();
	}
	else {
		Sum* const sum = new Sum;
		sum->push_back(this->term_ptr);

		Product* const subtractor = new Product;
		subtractor->push_back(new Value({ -1, 0 }));
		subtractor->push_back(copy_subterm(operand2.term_ptr));

		sum->push_back(subtractor);
		this->term_ptr = sum;
		this->simplify();
	}
	return *this;
}

bmath::Term& bmath::Term::operator*=(const Term& operand2)
{
	if (type_of(this->term_ptr) == Type::value && type_of(operand2.term_ptr) == Type::value) {
		static_cast<Value*>(this->term_ptr)->val() *= static_cast<Value*>(operand2.term_ptr)->val();
	}
	else {
		Product* const product = new Product;
		product->push_back(this->term_ptr);
		product->push_back(copy_subterm(operand2.term_ptr));
		this->term_ptr = product;
		this->simplify();
	}
	return *this;
}

bmath::Term& bmath::Term::operator/=(const Term& operand2)
{
	if (type_of(this->term_ptr) == Type::value && type_of(operand2.term_ptr) == Type::value) {
		static_cast<Value*>(this->term_ptr)->val() /= static_cast<Value*>(operand2.term_ptr)->val();
	}
	else {
		Product* const product = new Product;
		product->push_back(this->term_ptr);

		Exponentiation* const divisor = new Exponentiation;
		divisor->expo = new Value(std::complex<double>{ -1, 0 });
		divisor->base = copy_subterm(operand2.term_ptr);

		product->push_back(divisor);
		this->term_ptr = product;
		this->simplify();
	}
	return *this;
}

bmath::Term bmath::Term::operator+(const Term& operand2) const
{
	bmath::Term operand1(*this);
	return operand1 += operand2;
}

bmath::Term bmath::Term::operator-(const Term& operand2) const
{
	bmath::Term operand1(*this);
	return operand1 -= operand2;
}

bmath::Term bmath::Term::operator*(const Term& operand2) const
{
	bmath::Term operand1(*this);
	return operand1 *= operand2;
}

bmath::Term bmath::Term::operator/(const Term& operand2) const
{
	bmath::Term operand1(*this);
	return operand1 /= operand2;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//ostream overloads\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& stream, const bmath::Term& term) {
	stream << term.to_str();
	return stream;
}

std::ostream& operator<<(std::ostream& stream, const bmath::intern::Basic_Term& term) {
	std::string str;
	term.to_str(str, operator_precedence(Type::undefined));
	stream << str;
	return stream;
}
