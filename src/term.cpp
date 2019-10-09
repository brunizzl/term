
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

bool Basic_Term::operator!=(const Basic_Term& other) const
{
	return !(*this == other);
}

void bmath::intern::Basic_Term::sort()
{
	this->for_each([](Basic_Term* this_ptr, Type this_type) { \
		if (this_type == Type::sum) {
			Sum* const this_sum = static_cast<Sum*>(this_ptr);
			this_sum->sort_operands([](Basic_Term*& a, Basic_Term*& b) -> bool {return *a < *b; });
		}
		else if (this_type == Type::product) {
			Product* const this_product = static_cast<Product*>(this_ptr);
			this_product->sort_operands([](Basic_Term*& a, Basic_Term*& b) -> bool {return *a < *b; });
		}
		});
}

std::list<Basic_Term*> bmath::intern::Basic_Term::list_subterms(Type listed_type)
{
	std::list<Basic_Term*> erg_list;

	struct
	{
		std::list<Basic_Term*>& list;
		Type search_type;
		void operator()(Basic_Term* this_ptr, Type this_type)
		{
			if (this_type == search_type) {
				list.push_back(this_ptr);
			}
		}
	} term_collector = { erg_list, listed_type };

	this->for_each(term_collector);
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
	struct
	{
		std::list<std::string>& list;
		void operator()(Basic_Term* this_ptr, Type this_type)
		{
			if (this_type == Type::variable) {
				const Variable* const var_ptr = static_cast<const Variable*>(this_ptr);
				list.push_back(var_ptr->name);
			}
		}
	} name_collector = { names };

	this->term_ptr->for_each(name_collector);
	return names;
}

bool bmath::Term::match_and_transform(Pattern& pattern)
{
	Basic_Term** match = this->term_ptr->match_intern(pattern.original.term_ptr, pattern.var_adresses, &(this->term_ptr));
	if (match != nullptr) {
		Basic_Term* const transformed = pattern.changed.copy();
		delete *match;
		*match = transformed;	//here the pointer to pointer is needed, as we overwrite the original storage position with the new term.
		return true;
	}
	return false;
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

void bmath::Term::combine()
{
	this->term_ptr->combine_layers(this->term_ptr);
	this->combine_values();
	this->term_ptr->sort();

	for (unsigned int i = 0; i < Pattern::patterns.size();) {
		if (this->match_and_transform(*Pattern::patterns[i])) {
			this->term_ptr->combine_layers(this->term_ptr);
			this->combine_values();

			this->term_ptr->combine_layers(this->term_ptr);	//guarantee well definded structure to match next pattern
			this->combine_values();
			this->term_ptr->sort();
			i = 0;	//if match was successfull, pattern search starts again.
		}
		else {
			i++;
		}
	}
	this->term_ptr->combine_layers(this->term_ptr);
}

void bmath::Term::cut_rounding_error(int pow_of_10_diff_to_set_0)
{
	std::list<Basic_Term*> values = this->term_ptr->list_subterms(Type::value);
	double quadratic_sum = 0;	//real and imaginary part are added seperatly
	for (auto &it : values) {
		quadratic_sum += std::abs(static_cast<const Value*>(it)->val().real()) * std::abs(static_cast<const Value*>(it)->val().real());
		quadratic_sum += std::abs(static_cast<const Value*>(it)->val().imag()) * std::abs(static_cast<const Value*>(it)->val().imag());
	}
	if (quadratic_sum != 0) {
		const double quadratic_average = std::sqrt(quadratic_sum / values.size() / 2);	//equal to standard deviation from 0
		const double limit_to_0 = quadratic_average * std::pow(10, -pow_of_10_diff_to_set_0);
		for (auto &it : values) {
			if (std::abs(static_cast<const Value*>(it)->val().real()) < limit_to_0) {
				const_cast<Value*>(static_cast<const Value*>(it))->val().real(0);
			}
			if (std::abs(static_cast<const Value*>(it)->val().imag()) < limit_to_0) {
				const_cast<Value*>(static_cast<const Value*>(it))->val().imag(0);
			}
		}
	}
}

bmath::Term& bmath::Term::operator+=(const Term& operand2)
{
	if (type_of(this->term_ptr) == Type::value && type_of(operand2.term_ptr) == Type::value) {
		static_cast<Value*>(this->term_ptr)->val() += static_cast<Value*>(operand2.term_ptr)->val();
	}
	else {
		Sum* const sum = new Sum;
		sum->move_into_operands(this->term_ptr);
		sum->copy_into_operands(operand2.term_ptr);
		this->term_ptr = sum;
		this->combine();
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
		sum->move_into_operands(this->term_ptr);

		Product* const subtractor = new Product;
		subtractor->calc_onto_value({ -1, 0 });
		subtractor->copy_into_operands(operand2.term_ptr);

		sum->move_into_operands(subtractor);
		this->term_ptr = sum;
		this->combine();
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
		product->move_into_operands(this->term_ptr);
		product->copy_into_operands(operand2.term_ptr);
		this->term_ptr = product;
		this->combine();
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
		product->move_into_operands(this->term_ptr);

		Exponentiation* const divisor = new Exponentiation;
		divisor->exponent = new Value(std::complex<double>{ -1, 0 });
		divisor->base = copy_subterm(operand2.term_ptr);

		product->move_into_operands(divisor);
		this->term_ptr = product;
		this->combine();
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
