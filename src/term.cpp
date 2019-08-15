#include "term.h"

using namespace bmath::intern;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Basic_Term\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Basic_Term::Basic_Term(Basic_Term* parent_)
	:parent(parent_)
{
}

Basic_Term::Basic_Term(const Basic_Term& source)
	:parent(source.parent)
{
}

Basic_Term::~Basic_Term()
{
	//cleaning up the tree is done in derived classes
}

void Basic_Term::combine_layers(Basic_Term*& storage_key)
{
	//the base class does not know of the tree structures, the derived classes provide.
	//therefore, no tree can be combined.
	//the classes Value and Variable dont need an override to this function.
	//every derived class not beeing leaf needs an overriding function.
}

bool Basic_Term::re_smaller_than_0()
{
	//only value needs overloading
	return false;
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
	if (preprocess_str(name_)) {
		this->term_ptr = build_subterm(name_, nullptr);
	}
	LOG_C("baue Term " << this << ": " << *this);
}

bmath::Term::Term(const Term& source)
	:term_ptr(copy_subterm(source.term_ptr, nullptr))
{
	LOG_C("kopiere Term " << &source << " zu " << this << ": " << *this);
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
	LOG_C("loesche Term " << this);
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
	std::vector<std::string> tree_lines;
	term_ptr->to_tree_str(tree_lines, 0, '\0');

	std::string return_str;
	for (unsigned int pos = 0; pos < tree_lines.size(); pos++) {
		if (pos != 0) {
			return_str.push_back('\n');
		}
		if (offset > 0) {
			return_str.append(std::string(offset, ' '));
		}
		return_str.append(tree_lines[pos]);
	}
	return return_str;
}

std::set<std::string> bmath::Term::get_var_names() const
{
	std::set<std::string> names;
	std::list<Basic_Term*> variables;
	this->term_ptr->list_subterms(variables, s_variable);
	for (auto it : variables) {
		names.insert(static_cast<Variable*>(it)->name);
	}
	return names;
}

bool bmath::Term::match_and_transform(Pattern& pattern)
{
	Basic_Term** match = this->term_ptr->match_intern(pattern.original.term_ptr, pattern.var_adresses, &(this->term_ptr));
	if (match != nullptr) {
		Basic_Term* transformed = pattern.changed.copy((*match)->parent);
		delete *match;
		*match = transformed;
		return true;
	}
	return false;
}

void bmath::Term::combine_values()
{
	Vals_Combined new_subterm = this->term_ptr->combine_values();
	if (new_subterm.known) {
		delete this->term_ptr;
		this->term_ptr = new Value(new_subterm.val, nullptr);
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
	Value value_term(value_, nullptr);
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

	std::list<recurring_term> recurring_terms;
	//HIER MUSS DIE liste MIT DOPPELTEN VARIABLEN GEFÜLLT WERDEN UND DIE VARIABLEN DANN VON IHREN FREUNDEN ABGESPALTET WERDEN.
	//GRAUSAM ABER NOTWENDIG 
	//UND DANN WERDEN DIE FÜR IMMER VON EINANDER WEGSORTIERT

	for (unsigned int i = 0; i < Pattern::patterns.size(); i++) {
		if (this->match_and_transform(*Pattern::patterns[i])) {
			i = -1;	//if match was successfull, pattern search starts again.
			this->term_ptr->combine_layers(this->term_ptr);
			this->combine_values();
		}
	}
	this->term_ptr->combine_layers(this->term_ptr);
}

void bmath::Term::cut_rounding_error(int pow_of_10_diff_to_set_0)
{
	std::list<Basic_Term*> values;
	this->term_ptr->list_subterms(values, s_value);
	double re_sum = 0;
	for (auto it : values) {
		re_sum += std::abs(static_cast<Value*>(it)->value.real());
	}
	if (re_sum != 0) {
		double re_average = re_sum / values.size();
		double limit_to_0 = re_average * std::pow(10, -pow_of_10_diff_to_set_0);
		for (auto it : values) {
			if (std::abs(static_cast<Value*>(it)->value.real()) < limit_to_0) {
				static_cast<Value*>(it)->value.real(0);
			}
			if (std::abs(static_cast<Value*>(it)->value.imag()) < limit_to_0) {
				static_cast<Value*>(it)->value.imag(0);
			}
		}
	}
}

bmath::Term& bmath::Term::operator+=(const Term& operand2)
{
	if (this->term_ptr->get_state() == s_value && operand2.term_ptr->get_state() == s_value) {
		static_cast<Value*>(this->term_ptr)->value += static_cast<Value*>(operand2.term_ptr)->value;
	}
	else {
		Sum* sum = new Sum(nullptr);
		this->term_ptr->parent = sum;
		sum->summands.push_back(this->term_ptr);
		sum->summands.push_back(copy_subterm(operand2.term_ptr, sum));
		this->term_ptr = sum;
		this->combine();
	}
	return *this;
}

bmath::Term& bmath::Term::operator-=(const Term& operand2)
{
	if (this->term_ptr->get_state() == s_value && operand2.term_ptr->get_state() == s_value) {
		static_cast<Value*>(this->term_ptr)->value -= static_cast<Value*>(operand2.term_ptr)->value;
	}
	else {
		Sum* sum = new Sum(nullptr);
		this->term_ptr->parent = sum;
		sum->summands.push_back(this->term_ptr);
		sum->subtractors.push_back(copy_subterm(operand2.term_ptr, sum));
		this->term_ptr = sum;
		this->combine();
	}
	return *this;
}

bmath::Term& bmath::Term::operator*=(const Term& operand2)
{
	if (this->term_ptr->get_state() == s_value && operand2.term_ptr->get_state() == s_value) {
		static_cast<Value*>(this->term_ptr)->value *= static_cast<Value*>(operand2.term_ptr)->value;
	}
	else {
		Product* product = new Product(nullptr);
		this->term_ptr->parent = product;
		product->factors.push_back(this->term_ptr);
		product->factors.push_back(copy_subterm(operand2.term_ptr, product));
		this->term_ptr = product;
		this->combine();
	}
	return *this;
}

bmath::Term& bmath::Term::operator/=(const Term& operand2)
{
	if (this->term_ptr->get_state() == s_value && operand2.term_ptr->get_state() == s_value) {
		static_cast<Value*>(this->term_ptr)->value /= static_cast<Value*>(operand2.term_ptr)->value;
	}
	else {
		Product* product = new Product(nullptr);
		this->term_ptr->parent = product;
		product->factors.push_back(this->term_ptr);
		product->divisors.push_back(copy_subterm(operand2.term_ptr, product));
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
