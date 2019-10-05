
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

void Basic_Term::combine_layers(Basic_Term*& storage_key)
{
	//the base class does not know of the tree structures, the derived classes provide.
	//therefore, no tree can be combined.
	//the classes Value, Variable and Pattern_Variable dont need an override to this function.
	//every derived class not beeing leaf needs an overriding function.
}

bool Basic_Term::operator!=(const Basic_Term& other) const
{
	return !(*this == other);
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
	this->term_ptr = build_subterm({ name_.data(), name_.length() }, nullptr);
}

bmath::Term::Term(std::complex<double> val) 
	:term_ptr(new Value(val, nullptr))
{}

bmath::Term::Term(const Term& source)
	:term_ptr(copy_subterm(source.term_ptr, nullptr))
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
		this->term_ptr = copy_subterm(source.term_ptr, nullptr);
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
	this->term_ptr->to_str(str);
	return str;
}

std::string bmath::Term::to_tree(std::size_t offset) const
{
	return ptr_to_tree(this->term_ptr, offset);
}

std::list<std::string> bmath::Term::get_var_names() const
{
	std::list<std::string> names;
	std::list<const Basic_Term*> variables;
	this->term_ptr->list_subterms(variables, Type::variable);
	for (const auto it : variables) {
		names.push_back(static_cast<const Variable*>(it)->name);
	}
	return names;
}

bool bmath::Term::match_and_transform(Pattern& pattern)
{
	Basic_Term** match = this->term_ptr->match_intern(pattern.original.term_ptr, pattern.var_adresses, &(this->term_ptr));
	if (match != nullptr) {
		Basic_Term* transformed = pattern.changed.copy((*match)->parent());
		delete *match;
		*match = transformed;
		return true;
	}
	return false;
}

void bmath::Term::combine_values()
{
	if (type(this->term_ptr) != Type::value) {
		auto [is_val, val] = this->term_ptr->combine_values();
		if (is_val) {
			delete this->term_ptr;
			this->term_ptr = new Value(val, nullptr);
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
	const Value value_term(value_, nullptr);
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
	std::list<const Basic_Term*> values;
	this->term_ptr->list_subterms(values, Type::value);
	double quadratic_sum = 0;	//real and imaginary part are added seperatly
	for (auto &it : values) {
		quadratic_sum += std::abs(static_cast<const Value*>(it)->real()) * std::abs(static_cast<const Value*>(it)->real());
		quadratic_sum += std::abs(static_cast<const Value*>(it)->imag()) * std::abs(static_cast<const Value*>(it)->imag());
	}
	if (quadratic_sum != 0) {
		const double quadratic_average = std::sqrt(quadratic_sum / values.size() / 2);	//equal to standard deviation from 0
		const double limit_to_0 = quadratic_average * std::pow(10, -pow_of_10_diff_to_set_0);
		for (auto &it : values) {
			if (std::abs(static_cast<const Value*>(it)->real()) < limit_to_0) {
				const_cast<Value*>(static_cast<const Value*>(it))->real(0);
			}
			if (std::abs(static_cast<const Value*>(it)->imag()) < limit_to_0) {
				const_cast<Value*>(static_cast<const Value*>(it))->imag(0);
			}
		}
	}
}

bmath::Term& bmath::Term::operator+=(const Term& operand2)
{
	if (this->term_ptr->get_type() == Type::value && operand2.term_ptr->get_type() == Type::value) {
		*static_cast<Value*>(this->term_ptr) += *static_cast<Value*>(operand2.term_ptr);
	}
	else {
		Sum* sum = new Sum(nullptr);
		this->term_ptr->set_parent(sum);
		sum->summands.push_back(this->term_ptr);
		sum->summands.push_back(copy_subterm(operand2.term_ptr, sum));
		this->term_ptr = sum;
		this->combine();
	}
	return *this;
}

bmath::Term& bmath::Term::operator-=(const Term& operand2)
{
	if (this->term_ptr->get_type() == Type::value && operand2.term_ptr->get_type() == Type::value) {
		*static_cast<Value*>(this->term_ptr) -= *static_cast<Value*>(operand2.term_ptr);
	}
	else {
		Sum* sum = new Sum(nullptr);
		this->term_ptr->set_parent(sum);
		sum->summands.push_back(this->term_ptr);

		Product* subtractor = new Product(sum);
		subtractor->factors.push_back(new Value(std::complex<double>{ -1, 0 }, subtractor));
		subtractor->factors.push_back(copy_subterm(operand2.term_ptr, subtractor));

		sum->summands.push_back(subtractor);
		this->term_ptr = sum;
		this->combine();
	}
	return *this;
}

bmath::Term& bmath::Term::operator*=(const Term& operand2)
{
	if (this->term_ptr->get_type() == Type::value && operand2.term_ptr->get_type() == Type::value) {
		*static_cast<Value*>(this->term_ptr) *= *static_cast<Value*>(operand2.term_ptr);
	}
	else {
		Product* product = new Product(nullptr);
		this->term_ptr->set_parent(product);
		product->factors.push_back(this->term_ptr);
		product->factors.push_back(copy_subterm(operand2.term_ptr, product));
		this->term_ptr = product;
		this->combine();
	}
	return *this;
}

bmath::Term& bmath::Term::operator/=(const Term& operand2)
{
	if (this->term_ptr->get_type() == Type::value && operand2.term_ptr->get_type() == Type::value) {
		*static_cast<Value*>(this->term_ptr) /= *static_cast<Value*>(operand2.term_ptr);
	}
	else {
		Product* product = new Product(nullptr);
		this->term_ptr->set_parent(product);
		product->factors.push_back(this->term_ptr);

		Exponentiation* divisor = new Exponentiation(product);
		divisor->exponent = new Value(std::complex<double>{ -1, 0 }, divisor);
		divisor->base = copy_subterm(operand2.term_ptr, divisor);

		product->factors.push_back(divisor);
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
	term.to_str(str);
	stream << str;
	return stream;
}

