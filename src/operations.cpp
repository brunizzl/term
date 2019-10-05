
#include "operations.h"
#include "internal_functions.h"

//#include <sstream>

using namespace bmath::intern;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Product\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Product::Product(Basic_Term* parent_)
	:parent_ptr(parent_), value_factor(1.0, this)
{
}

Product::Product(std::string_view name_, Basic_Term* parent_, std::size_t op)
	:parent_ptr(parent_), value_factor(1.0, this)
{
	const Value_Manipulator factor_multiplier = { &this->value_factor, \
		[](std::complex<double>* first, std::complex<double> second) -> void {*first *= second; } };
	const Value_Manipulator factor_divider = { &this->value_factor, \
		[](std::complex<double>* first, std::complex<double> second) -> void {*first /= second; } };
	Basic_Term* new_subterm = nullptr;

	while (op != std::string::npos) {
		const std::string_view subterm_view = name_.substr(op + 1);	//we don't want the operator itself to be part of the substr
		switch (name_[op]) {
		case '*':
			new_subterm = build_subterm(subterm_view, this, factor_multiplier);
			if (new_subterm) {
				this->factors.push_front(new_subterm);
			}
			break;
		case '/':
			new_subterm = build_subterm(subterm_view, this, factor_divider);
			if (new_subterm) {
				this->factors.push_front(new Exponentiation(new_subterm, this, { -1, 0 }));
			}
			break;
		}
		name_.remove_suffix(name_.length() - op);
		op = find_last_of_skip_pars(name_, "*/");
	}
	//last part of name without any '*' or '/' in front
	new_subterm = build_subterm(name_, this, factor_multiplier);
	if (new_subterm != nullptr) {
		this->factors.push_front(new_subterm);
	}
}

Product::Product(Basic_Term* name_, Basic_Term* parent_, std::complex<double> factor) 
	:parent_ptr(parent_), value_factor(factor, this), factors({ name_ })
{
	name_->set_parent(this);
}

Product::Product(std::string_view name_, Basic_Term* parent_, std::size_t op, std::list<Pattern_Variable*>& variables)
	:parent_ptr(parent_), value_factor(1.0, this)
{
	const Value_Manipulator factor_multiplier = { &this->value_factor, \
		[](std::complex<double>* first, std::complex<double> second) -> void {*first *= second; } };
	const Value_Manipulator factor_divider = { &this->value_factor, \
		[](std::complex<double>* first, std::complex<double> second) -> void {*first /= second; } };
	Basic_Term* new_subterm = nullptr;

	while (op != std::string::npos) {
		const std::string_view subterm_view = name_.substr(op + 1);	//we don't want the operator itself to be part of the substr
		switch (name_[op]) {
		case '*':
			new_subterm = build_pattern_subterm(subterm_view, this, variables, factor_multiplier);
			if (new_subterm) {
				this->factors.push_front(new_subterm);
			}			
			break;
		case '/':
			new_subterm = build_pattern_subterm(subterm_view, this, variables, factor_divider);
			if (new_subterm) {
				this->factors.push_front(new Exponentiation(new_subterm, this, { -1, 0 }));
			}
			break;
		}
		name_.remove_suffix(name_.length() - op);
		op = find_last_of_skip_pars(name_, "*/");
	}
	//last part of name without any '*' or '/' in front
	new_subterm = build_pattern_subterm(name_, this, variables, factor_multiplier);
	if (new_subterm != nullptr) {
		this->factors.push_front(new_subterm);
	}
}

Product::Product(const Product& source, Basic_Term* parent_)
	:parent_ptr(parent_), value_factor(source.value_factor)
{
	for (const auto it : source.factors) {
		this->factors.push_back(copy_subterm(it, this));
	}
}

Product::~Product()
{
	for (const auto it : this->factors) {
		delete it;
	}
}

Basic_Term* bmath::intern::Product::parent() const
{
	return this->parent_ptr;
}

void bmath::intern::Product::set_parent(Basic_Term* new_parent)
{
	this->parent_ptr = new_parent;
}

void Product::to_str(std::string& str) const
{
	const bool pars = type(this->parent_ptr) > this->get_type();
	if (pars) {
		str.push_back('(');
	}
	bool nothing_printed_yet = true;
	if (this->value_factor != 1.0) {
		value_factor.to_str(str);
		nothing_printed_yet = false;
	}
	for (const auto it : this->factors) {
		if (!std::exchange(nothing_printed_yet, false)) {
			str.push_back('*');
		}
		it->to_str(str);
	}
	if (pars) {
		str.push_back(')');
	}
}

void Product::to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const
{
	std::string new_line(dist_root * 5, ' ');	//building string with spaces matching dept of this
	new_line.append("product");
	tree_lines.push_back(std::move(new_line));
	append_last_line(tree_lines, line_prefix);

	value_factor.to_tree_str(tree_lines, dist_root + 1, '*');

	for (auto factor : this->factors) {
		factor->to_tree_str(tree_lines, dist_root + 1, '*');
	}
}

Type Product::get_type() const
{
	return Type::product;
}

void Product::combine_layers(Basic_Term*& storage_key)
{
	for (auto &it = this->factors.begin(); it != this->factors.end();) {	//reference is needed in next line
		(*it)->combine_layers(*it);
		if (type(*it) == Type::product) {
			Product* redundant = static_cast<Product*>((*it));
			this->value_factor *= redundant->value_factor;
			for (auto it_red : redundant->factors) {
				it_red->set_parent(this);
			}
			this->factors.splice(this->factors.end(), redundant->factors);
			delete redundant;
			it = this->factors.erase(it);
		}
		else {
			++it;
		}
	}
	if (this->factors.size() == 1 && this->value_factor == 1.0) {	//this only consists of one "real" factor -> layer is not needed and this is deleted
		Basic_Term* const only_factor = *(this->factors.begin());
		storage_key = only_factor;
		only_factor->set_parent(this->parent_ptr);
		this->factors.clear();
		delete this;
	}
	else if (this->factors.size() == 0) {	//this only consists of value_factor -> layer is not needed and this is deleted
		Value* const only_factor = new Value(this->value_factor, this->parent_ptr);
		storage_key = only_factor;
		delete this;
	}
}

Vals_Combined Product::combine_values()
{
	bool only_known = true;
	for (auto it = this->factors.begin(); it != this->factors.end();) {
		const auto [it_known, it_factor] = (*it)->combine_values();
		if (it_known) {
			this->value_factor *= it_factor;
			delete (*it);
			it = this->factors.erase(it);
		}
		else {
			only_known = false;
			++it;
		}
	}
	return { only_known, this->value_factor };
}

std::complex<double> Product::evaluate(const std::list<bmath::Known_Variable>& known_variables) const
{
	std::complex<double> result = this->value_factor;
	for (const auto it : this->factors) {
		result *= it->evaluate(known_variables);
	}
	return result;
}

void Product::search_and_replace(const std::string& name_, const Basic_Term* value_, Basic_Term*& storage_key)
{
	for (auto &it : this->factors) {	//reference is needed in next line
		it->search_and_replace(name_, value_, it);
	}
}

void Product::list_subterms(std::list<const Basic_Term*>& subterms, Type listed_type) const
{
	if (listed_type == Type::product) {
		subterms.push_back(this);
	}
	else if (listed_type == Type::value) {
		subterms.push_back(&this->value_factor);
	}
	for (auto it : this->factors) {
		it->list_subterms(subterms, listed_type);
	}
}

void Product::sort()
{
	for (auto it : this->factors) {
		it->sort();
	}
	this->factors.sort([](Basic_Term * &a, Basic_Term * &b) -> bool {return *a < *b; });
}

Basic_Term** Product::match_intern(Basic_Term* pattern, std::list<Pattern_Variable*>& pattern_var_adresses, Basic_Term** storage_key)
{
	//MUSS NOCH VERGLEICHEN, OB NUR TEILE VON THIS GLEICH GANZEM PATTERN SIND
	if (*this == *pattern) {
		return storage_key;
	}
	else {
		Basic_Term** argument_match;
		for (auto &it : this->factors) {	//references are important, because we want to return the position of it.
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

bool Product::operator<(const Basic_Term& other) const
{
	if (this->get_type() != other.get_type()) {
		return this->get_type() < other.get_type();
	}
	else {
		const Product* other_product = static_cast<const Product*>(&other);
		if (this->factors.size() != other_product->factors.size()) {
			return this->factors.size() < other_product->factors.size();
		}
		if (this->value_factor.real() != other_product->value_factor.real()) {
			return this->value_factor.real() < other_product->value_factor.real();
		}
		if (this->value_factor.imag() != other_product->value_factor.imag()) {
			return this->value_factor.imag() < other_product->value_factor.imag();
		}
		//the operator assumes from now on to have sorted products to compare
		auto it_this = this->factors.begin();
		auto it_other = other_product->factors.begin();
		for (; it_this != this->factors.end() && it_other != other_product->factors.end(); ++it_this, ++it_other) {
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

bool Product::operator==(const Basic_Term& other) const
{
	switch (other.get_type()) {
	case Type::product:
		break;
	case Type::pattern_variable:
		return other == *this;
	default:
		return false;
	}
	const Product* other_product = static_cast<const Product*>(&other);
	if (this->factors.size() != other_product->factors.size()) {
		return false;
	}
	if (this->value_factor.real() != other_product->value_factor.real()) {
		return false;
	}
	if (this->value_factor.imag() != other_product->value_factor.imag()) {
		return false;
	}
	//the operator assumes from now on to have sorted products to compare
	auto it_this = this->factors.begin();
	auto it_other = other_product->factors.begin();
	for (; it_this != this->factors.end() && it_other != other_product->factors.end(); ++it_this, ++it_other) {
		if ((**it_this) != (**it_other)) {
			return false;
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Sum\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Sum::Sum(Basic_Term* parent_)
	:parent_ptr(parent_), value_summand(0.0, this)
{
}

Sum::Sum(std::string_view name_, Basic_Term* parent_, std::size_t op)
	:parent_ptr(parent_), value_summand(0.0, this)
{
	const Value_Manipulator summand_adder = { &this->value_summand, \
		[](std::complex<double>* first, std::complex<double> second) -> void {*first += second; } };
	const Value_Manipulator summand_subtractor = { &this->value_summand, \
		[](std::complex<double>* first, std::complex<double> second) -> void {*first -= second; } };
	Basic_Term* new_subterm = nullptr;

	while (op != std::string::npos) {
		const std::string_view subterm_view = name_.substr(op + 1);	//we don't want the operator itself to be part of the substr
		switch (name_[op]) {
		case '+':
			new_subterm = build_subterm(subterm_view, this, summand_adder);
			if (new_subterm) {
				this->summands.push_front(new_subterm);
			}
			break;
		case '-':
			new_subterm = build_subterm(subterm_view, this, summand_subtractor);
			if (new_subterm) {
				this->summands.push_front(new Product(new_subterm, this, { -1, 0 }));
			}
			break;
		}			
		name_.remove_suffix(name_.length() - op);
		op = find_last_of_skip_pars(name_, "+-");
	}
	//last part of name without any '+' or '-' in front
	if (name_.size()) {
		Basic_Term* const new_subterm = build_subterm(name_, this, summand_adder);
		if (new_subterm != nullptr) {
			this->summands.push_front(new_subterm);
		}
	}
}

Sum::Sum(std::string_view name_, Basic_Term* parent_, std::size_t op, std::list<Pattern_Variable*>& variables)
	:parent_ptr(parent_), value_summand(0.0, this)
{
	const Value_Manipulator summand_adder = { &this->value_summand, \
		[](std::complex<double>* first, std::complex<double> second) -> void {*first += second; } };
	const Value_Manipulator summand_subtractor = { &this->value_summand, \
		[](std::complex<double>* first, std::complex<double> second) -> void {*first -= second; } };
	Basic_Term* new_subterm = nullptr;

	while (op != std::string::npos) {
		const std::string_view subterm_view = name_.substr(op + 1);	//we don't want the operator itself to be part of the substr
		switch (name_[op]) {
		case '+':
			new_subterm = build_pattern_subterm(subterm_view, this, variables, summand_adder);
			if (new_subterm) {
				this->summands.push_front(new_subterm);
			}
			break;
		case '-':
			new_subterm = build_pattern_subterm(subterm_view, this, variables, summand_subtractor);
			if (new_subterm) {
				this->summands.push_front(new Product(new_subterm, this, { -1, 0 }));
			}
			break;
		}
		name_.remove_suffix(name_.length() - op);
		op = find_last_of_skip_pars(name_, "+-");
	}
	//last part of name without any '+' or '-' in front
	if (name_.size()) {
		new_subterm = build_pattern_subterm(name_, this, variables, summand_adder);
		if (new_subterm != nullptr) {
			this->summands.push_front(new_subterm);
		}
	}
}

Sum::Sum(const Sum& source, Basic_Term* parent_)
	:parent_ptr(parent_), value_summand(source.value_summand)
{
	for (const auto it : source.summands) {
		this->summands.push_back(copy_subterm(it, this));
	}
}

Sum::~Sum()
{
	for (const auto it : this->summands) {
		delete it;
	}
}

Basic_Term* bmath::intern::Sum::parent() const
{
	return this->parent_ptr;
}

void bmath::intern::Sum::set_parent(Basic_Term* new_parent)
{
	this->parent_ptr = new_parent;
}

void Sum::to_str(std::string& str) const
{
	const bool pars = type(this->parent_ptr) > this->get_type();
	if (pars) {
		str.push_back('(');
	}
	bool nothing_printed_yet = true;
	if (this->value_summand != 0.0) {
		value_summand.to_str(str);
		nothing_printed_yet = false;
	}
	for (const auto it : this->summands) {
		if (!std::exchange(nothing_printed_yet, false)) {
			str.push_back('+');
		}
		it->to_str(str);
	}
	if (pars) {
		str.push_back(')');
	}
}

void Sum::to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const
{
	std::string new_line(dist_root * 5, ' ');	//building string with spaces matching dept of this
	new_line.append("sum");
	tree_lines.push_back(std::move(new_line));
	append_last_line(tree_lines, line_prefix);

	value_summand.to_tree_str(tree_lines, dist_root + 1, '+');

	for (auto summand : this->summands) {
		summand->to_tree_str(tree_lines, dist_root + 1, '+');
	}
}

Type Sum::get_type() const
{
	return Type::sum;
}

void Sum::combine_layers(Basic_Term*& storage_key)
{
	for (auto &it = this->summands.begin(); it != this->summands.end();) {	//reference is needed next line
		(*it)->combine_layers(*it);
		if (type(*it) == Type::sum) {
			Sum* redundant = static_cast<Sum*>((*it));
			this->value_summand += redundant->value_summand;
			for (auto it_red : redundant->summands) {
				it_red->set_parent(this);
			}
			this->summands.splice(this->summands.end(), redundant->summands);
			delete redundant;
			it = this->summands.erase(it);
		}
		else {
			++it;
		}
	}
	if (this->summands.size() == 1 && this->value_summand == 0.0) {	//this only consists of one "real" summand -> layer is not needed and this is deleted
		Basic_Term* const only_summand = *(this->summands.begin());
		storage_key = only_summand;
		only_summand->set_parent(this->parent_ptr);
		this->summands.clear();
		delete this;
	}
	else if (this->summands.size() == 0) {	//this only consists of value_summand -> layer is not needed and this is deleted
		Value* const only_summand = new Value(this->value_summand, this->parent_ptr);
		storage_key = only_summand;
		delete this;
	}
}

Vals_Combined Sum::combine_values()
{
	bool only_known = true;
	for (auto it = this->summands.begin(); it != this->summands.end();) {
		const auto [known, summand] = (*it)->combine_values();
		if (known) {
			this->value_summand += summand;
			delete (*it);
			it = this->summands.erase(it);
		}
		else {
			only_known = false;
			++it;
		}
	}
	return { only_known, value_summand };
}

std::complex<double> Sum::evaluate(const std::list<bmath::Known_Variable>& known_variables) const
{
	std::complex<double> result = this->value_summand;
	for (const auto it : this->summands) {
		result += it->evaluate(known_variables);
	}
	return result;
}

void Sum::search_and_replace(const std::string& name_, const Basic_Term* value_, Basic_Term*& storage_key)
{
	for (auto &it : this->summands) {	//reference is needed in next line
		it->search_and_replace(name_, value_, it);
	}
}

void Sum::list_subterms(std::list<const Basic_Term*>& subterms, Type listed_type) const
{
	if (listed_type == Type::sum) {
		subterms.push_back(this);
	}
	else if (listed_type == Type::value) {
		subterms.push_back(&this->value_summand);
	}
	for (auto it : this->summands) {
		it->list_subterms(subterms, listed_type);
	}
}

void Sum::sort()
{
	for (auto it : this->summands) {
		it->sort();
	}
	this->summands.sort([](Basic_Term*& a, Basic_Term*& b) -> bool {return *a < *b; });
}

Basic_Term** Sum::match_intern(Basic_Term* pattern, std::list<Pattern_Variable*>& pattern_var_adresses, Basic_Term** storage_key)
{
	//MUSS NOCH VERGLEICHEN, OB NUR TEILE VON THIS GLEICH GANZEM PATTERN SIND
	if (*this == *pattern) {
		return storage_key;
	}
	else {
		Basic_Term** argument_match;
		for (auto &it : this->summands) {	//references are important, because we want to return the position of it.
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

bool Sum::operator<(const Basic_Term& other) const
{
	if (this->get_type() != other.get_type()) {
		return this->get_type() < other.get_type();
	}
	else {
		const Sum* other_sum = static_cast<const Sum*>(&other);
		if (this->summands.size() != other_sum->summands.size()) {
			return this->summands.size() < other_sum->summands.size();
		}
		if (this->value_summand.real() != other_sum->value_summand.real()) {
			return this->value_summand.real() < other_sum->value_summand.real();
		}
		if (this->value_summand.imag() != other_sum->value_summand.imag()) {
			return this->value_summand.imag() < other_sum->value_summand.imag();
		}
		//the operator assumes from now on to have sorted sums to compare
		auto it_this = this->summands.begin();
		auto it_other = other_sum->summands.begin();
		for (; it_this != this->summands.end() && it_other != other_sum->summands.end(); ++it_this, ++it_other) {
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

bool Sum::operator==(const Basic_Term& other) const
{
	switch (other.get_type()) {
	case Type::sum:
		break;
	case Type::pattern_variable:
		return other == *this;
	default:
		return false;
	}
	const Sum* other_sum = static_cast<const Sum*>(&other);
	if (this->summands.size() != other_sum->summands.size()) {
		return false;
	}
	if (this->value_summand.real() != other_sum->value_summand.real()) {
		return false;
	}
	if (this->value_summand.imag() != other_sum->value_summand.imag()) {
		return false;
	}
	//the operator assumes from now on to have sorted products to compare
	auto it_this = this->summands.begin();
	auto it_other = other_sum->summands.begin();
	for (; it_this != this->summands.end() && it_other != other_sum->summands.end(); ++it_this, ++it_other) {
		if ((**it_this) != (**it_other)) {
			return false;
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Exponentiation\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Exponentiation::Exponentiation(Basic_Term* parent_)
	:parent_ptr(parent_), base(nullptr), exponent(nullptr)
{
}

Exponentiation::Exponentiation(std::string_view name_, Basic_Term* parent_, std::size_t op)
	:parent_ptr(parent_)
{
	std::string_view subterm_view;
	subterm_view = name_.substr(op + 1);
	this->exponent = build_subterm(subterm_view, this);
	name_.remove_suffix(name_.length() - op);
	this->base = build_subterm(name_, this);
}

Exponentiation::Exponentiation(Basic_Term* base_, Basic_Term* parent_, std::complex<double> exponent_) 
	:parent_ptr(parent_), exponent(new Value(exponent_, this)), base(base_)
{
	base_->set_parent(this);
}

Exponentiation::Exponentiation(std::string_view name_, Basic_Term* parent_, std::size_t op, std::list<Pattern_Variable*>& variables)
	:parent_ptr(parent_)
{
	std::string_view subterm_view;
	subterm_view = name_.substr(op + 1);
	this->exponent = build_pattern_subterm(subterm_view, this, variables, { nullptr, nullptr });
	name_.remove_suffix(name_.length() - op);
	this->base = build_pattern_subterm(name_, this, variables, { nullptr, nullptr });
}

Exponentiation::Exponentiation(const Exponentiation& source, Basic_Term* parent_)
	:parent_ptr(parent_), base(copy_subterm(source.base, this)), exponent(copy_subterm(source.exponent, this))
{
}

Exponentiation::~Exponentiation()
{
	delete exponent;
	delete base;
}

Basic_Term* bmath::intern::Exponentiation::parent() const
{
	return this->parent_ptr;
}

void bmath::intern::Exponentiation::set_parent(Basic_Term* new_parent)
{
	this->parent_ptr = new_parent;
}

void Exponentiation::to_str(std::string& str) const
{
	str.push_back('(');
	this->base->to_str(str);
	str.push_back('^');
	this->exponent->to_str(str);
	str.push_back(')');
}

void Exponentiation::to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const
{
	std::string new_line(dist_root * 5, ' ');	//building string with spaces matching dept of this
	new_line.append("exponentiation");
	tree_lines.push_back(std::move(new_line));
	append_last_line(tree_lines, line_prefix);

	this->base->to_tree_str(tree_lines, dist_root + 1, '_');
	this->exponent->to_tree_str(tree_lines, dist_root + 1, '^');
}

Type Exponentiation::get_type() const
{
	return Type::exponentiation;
}

void Exponentiation::combine_layers(Basic_Term*& storage_key)
{
	this->base->combine_layers(this->base);
	this->exponent->combine_layers(this->exponent);
	if (this->exponent->get_type() == Type::value) {
		Value* const val_exp = static_cast<Value*>(this->exponent);
		if (*val_exp == 1.0) {
			storage_key = this->base;
			this->base->set_parent(this->parent_ptr);
			this->base = nullptr;
			delete this;
			return;
		}
		if (*val_exp == 0.0) {
			storage_key = new Value({ 1.0, 0.0 }, this->parent_ptr);
			delete this;
			return;
		}
	}
	if (this->base->get_type() == Type::value) {
		Value* const val_base = static_cast<Value*>(this->base);
		if (*val_base == 1.0) {
			storage_key = val_base;
			val_base->parent_ptr = this->parent_ptr;
			this->base = nullptr;
			delete this;
			return;
		}
		if (*val_base == 0.0) {
			storage_key = new Value({ 0.0, 0.0 }, this->parent_ptr);
			delete this;
			return;
		}
	}
}

Vals_Combined Exponentiation::combine_values()
{
	const Vals_Combined base_ = this->base->combine_values();
	const Vals_Combined exponent_ = this->exponent->combine_values();

	if (base_.known && exponent_.known) {
		const std::complex<double> result = std::pow(base_.val, exponent_.val);
		return { true, result };
	}
	else if (base_.known && !exponent_.known) {
		if (type(this->base) != Type::value) {
			delete this->base;
			this->base = new Value(base_.val, this);
		}
	}
	else if (!base_.known && exponent_.known) {
		if (type(this->exponent) != Type::value) {
			delete this->exponent;
			this->exponent = new Value(exponent_.val, this);
		}
	}

	return { false, 0 };
}

std::complex<double> Exponentiation::evaluate(const std::list<bmath::Known_Variable>& known_variables) const
{
	const std::complex<double> base_ = this->base->evaluate(known_variables);
	const std::complex<double> exponent_ = this->exponent->evaluate(known_variables);
	return std::pow(base_, exponent_);
}

void Exponentiation::search_and_replace(const std::string& name_, const Basic_Term* value_, Basic_Term*& storage_key)
{
	this->base->search_and_replace(name_, value_, this->base);
	this->exponent->search_and_replace(name_, value_, this->exponent);
}

void Exponentiation::list_subterms(std::list<const Basic_Term*>& subterms, Type listed_type) const
{
	if (listed_type == Type::exponentiation) {
		subterms.push_back(this);
	}
	this->base->list_subterms(subterms, listed_type);
	this->exponent->list_subterms(subterms, listed_type);
}

void Exponentiation::sort()
{
	this->base->sort();
	this->exponent->sort();
}

Basic_Term** Exponentiation::match_intern(Basic_Term* pattern, std::list<Pattern_Variable*>& pattern_var_adresses, Basic_Term** storage_key)
{
	if (*this == *pattern) {
		return storage_key;
	}
	else {
		reset_pattern_vars(pattern_var_adresses);
		Basic_Term** argument_match = base->match_intern(pattern, pattern_var_adresses, &base);
		if (argument_match) {
			return argument_match;
		}
		reset_pattern_vars(pattern_var_adresses);
		argument_match = exponent->match_intern(pattern, pattern_var_adresses, &exponent);
		if (argument_match) {
			return argument_match;
		}
		reset_pattern_vars(pattern_var_adresses);
		return nullptr;
	}
}

bool Exponentiation::operator<(const Basic_Term& other) const
{
	if (this->get_type() != other.get_type()) {
		return this->get_type() < other.get_type();
	}
	else {
		const Exponentiation* other_exp = static_cast<const Exponentiation*>(&other);
		if (*(this->base) < *(other_exp->base)) {
			return true;
		}
		if (*(other_exp->base) < *(this->base)) {
			return false;
		}
		return *(this->exponent) < *(other_exp->exponent);
	}
}

bool Exponentiation::operator==(const Basic_Term& other) const
{
	switch (other.get_type()) {
	case Type::exponentiation:
		break;
	case Type::pattern_variable:
		return other == *this;
	default:
		return false;
	}
	const Exponentiation* other_exp = static_cast<const Exponentiation*>(&other);
	if (*(this->base) != *(other_exp->base)) {
		return false;
	}
	if (*(this->exponent) != *(other_exp->exponent)) {
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Parenthesis_Operator\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Par_Operator::Par_Operator(Basic_Term* parent_)
	:parent_ptr(parent_), argument(nullptr), op_type(Par_Op_Type::log10)	//just some default initialisation
{
}

Par_Operator::Par_Operator(std::string_view name_, Basic_Term* parent_, Par_Op_Type op_type_)
	:parent_ptr(parent_), op_type(op_type_), argument(nullptr)
{
	name_.remove_suffix(1);								//closing parenthesis gets cut of
	name_.remove_prefix(name_of(op_type).length());	//funktionname and opening parenthesis get cut of
	this->argument = build_subterm(name_, this);
}

Par_Operator::Par_Operator(std::string_view name_, Basic_Term* parent_, Par_Op_Type op_type_, std::list<Pattern_Variable*>& variables)
	:parent_ptr(parent_), op_type(op_type_), argument(nullptr)
{
	name_.remove_suffix(1);								//closing parenthesis gets cut of
	name_.remove_prefix(name_of(op_type).length());	//funktionname and opening parenthesis get cut of
	this-> argument = build_pattern_subterm(name_, this, variables, { nullptr, nullptr });
}


Par_Operator::Par_Operator(const Par_Operator & source, Basic_Term * parent_)
	:parent_ptr(parent_), argument(copy_subterm(source.argument, this)), op_type(source.op_type)
{
}

Par_Operator::~Par_Operator()
{
	delete this->argument;
}

Basic_Term* bmath::intern::Par_Operator::parent() const
{
	return this->parent_ptr;
}

void bmath::intern::Par_Operator::set_parent(Basic_Term* new_parent)
{
	this->parent_ptr = new_parent;
}

void Par_Operator::to_str(std::string & str) const
{
	str.append(name_of(this->op_type));
	this->argument->to_str(str);
	str.push_back(')');
}

void Par_Operator::to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const
{
	std::string new_line(dist_root * 5, ' ');	//building string with spaces matching dept of this
	new_line.append(name_of(this->op_type));
	new_line.pop_back();
	tree_lines.push_back(std::move(new_line));
	append_last_line(tree_lines, line_prefix);

	this->argument->to_tree_str(tree_lines, dist_root + 1, '\0');
}

Type Par_Operator::get_type() const
{
	return Type::par_operator;
}

void Par_Operator::combine_layers(Basic_Term*& storage_key)
{
	this->argument->combine_layers(this->argument);
}

Vals_Combined Par_Operator::combine_values()
{
	const auto [known, val] = argument->combine_values();
	return { known, value_of(val, this->op_type) };
}

std::complex<double> Par_Operator::evaluate(const std::list<bmath::Known_Variable>& known_variables) const
{
	
	return value_of(argument->evaluate(known_variables), this->op_type);
}

void Par_Operator::search_and_replace(const std::string & name_, const Basic_Term* value_, Basic_Term*& storage_key)
{
	this->argument->search_and_replace(name_, value_, this->argument);	
}

void Par_Operator::list_subterms(std::list<const Basic_Term*>& subterms, Type listed_type) const
{
	if (listed_type == Type::par_operator) {
		subterms.push_back(this);
	}
	this->argument->list_subterms(subterms, listed_type);
}

void Par_Operator::sort()
{
	this->argument->sort();
}

Basic_Term** Par_Operator::match_intern(Basic_Term* pattern, std::list<Pattern_Variable*>& pattern_var_adresses, Basic_Term** storage_key)
{
	if (*this == *pattern) {
		return storage_key;
	}
	else {
		reset_pattern_vars(pattern_var_adresses);
		Basic_Term** argument_match = argument->match_intern(pattern, pattern_var_adresses, &argument);
		if (argument_match) {
			return argument_match;
		}
		reset_pattern_vars(pattern_var_adresses);
		return nullptr;
	}
}

bool Par_Operator::operator<(const Basic_Term& other) const
{
	if (this->get_type() != other.get_type()) {
		return this->get_type() < other.get_type();
	}
	else {
		const Par_Operator* other_par_op = static_cast<const Par_Operator*>(&other);
		if (this->op_type != other_par_op->op_type) {
			return this->op_type < other_par_op->op_type;
		}
		return *(this->argument) < *(other_par_op->argument);
	}
}

bool Par_Operator::operator==(const Basic_Term& other) const
{
	switch (other.get_type()) {
	case Type::par_operator:
		break;
	case Type::pattern_variable:
		return other == *this;
	default:
		return false;
	}
	const Par_Operator* other_par_op = static_cast<const Par_Operator*>(&other);
	if (this->op_type != other_par_op->op_type) {
		return false;
	}
	if (*(this->argument) != *(other_par_op->argument)) {
		return false;
	}
	return true;
}
