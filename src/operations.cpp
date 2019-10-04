
#include "operations.h"
#include "internal_functions.h"

//#include <sstream>

using namespace bmath::intern;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Product\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Product::Product(Basic_Term* parent_)
	:Basic_Term(parent_)
{
}

Product::Product(std::string_view name_, Basic_Term* parent_, std::size_t op)
	:Basic_Term(parent_)
{
	std::complex<double> known_factor = 1.0;
	while (op != std::string::npos) {
		const std::string_view subterm_view = name_.substr(op + 1);	//we don't want the operator itself to be part of the substr
		switch (name_[op]) {
		case '*':
			this->factors.push_front(build_subterm(subterm_view, this));
			break;
		case '/':
			this->factors.push_front(new Exponentiation(subterm_view, this, { -1, 0 }));
			break;
		}
		name_.remove_suffix(name_.length() - op);
		op = find_last_of_skip_pars(name_, "*/");
	}
	//last part of name without any '*' or '/' in front
	this->factors.push_front(build_subterm(name_, this));
}

Product::Product(std::string_view name_, Basic_Term* parent_, std::complex<double> factor) 
	:Basic_Term(parent_), factors({ new Value( factor, this ), build_subterm(name_, this) })
{
}

Product::Product(std::string_view name_, Basic_Term* parent_, std::size_t op, std::list<Pattern_Variable*>& variables)
	:Basic_Term(parent_)
{
	std::complex<double> known_factor = 1.0;
	while (op != std::string::npos) {
		const std::string_view subterm_view = name_.substr(op + 1);	//we don't want the operator itself to be part of the substr
		switch (name_[op]) {
		case '*':
			this->factors.push_front(build_pattern_subterm(subterm_view, this, variables));
			break;
		case '/':
			this->factors.push_back(new Exponentiation(subterm_view, this, { -1, 0 }, variables));
			break;
		}
		name_.remove_suffix(name_.length() - op);
		op = find_last_of_skip_pars(name_, "*/");
	}
	//last part of name without any '*' or '/' in front
	this->factors.push_front(build_pattern_subterm(name_, this, variables));
}

Product::Product(std::string_view name_, Basic_Term* parent_, std::complex<double> factor, std::list<Pattern_Variable*>& variables) 
	:Basic_Term(parent_), factors({ new Value(factor, this), build_pattern_subterm(name_, this, variables) })
{}

Product::Product(const Product& source, Basic_Term* parent_)
	:Basic_Term(parent_)
{
	for (auto it : source.factors) {
		this->factors.push_back(copy_subterm(it, this));
	}
}

Product::~Product()
{
	for (auto it : this->factors) {
		delete it;
	}
}


std::optional<double> Product::factor_value() const {
	if (type(this->factors.front()) == Type::value) {
		const Value* const value = static_cast<Value*>(this->factors.front());
		if (value->val.imag() == 0.0) {
			return value->val.real();
		}
	}
	return {};
}

void Product::to_str(std::string& str) const
{
	const std::optional<double> fctr_val = factor_value();
	const bool negative_fctr = fctr_val && *fctr_val < 0.0;
	const bool pars = type(this->parent) > this->get_type();
	if (pars) {
		str.push_back('(');
	}
	bool need_operator = false;
	bool first = true;	//both bools have nearly opposite meaning, but none is redundant.
	for (auto it : this->factors) {
		if (std::exchange(first, false) && fctr_val) {	//it is first factor and of type value
			if (std::abs(*fctr_val) != 1.0) {
				const Value* const val = static_cast<Value*>(it);
				str.append(val->val_to_str(negative_fctr));
				need_operator = true;
			}
		}
		else {
			if (it->expect_inverse_str()) {
				if (!std::exchange(need_operator, true)) {	//first factor -> needs '1' before '/'
					str.push_back('1');
				}
				str.push_back('/');
			}
			else if (std::exchange(need_operator, true)) {
				str.push_back('*');
			}
			it->to_str(str);
		}
	}
	if (pars) {
		str.push_back(')');
	}
}

bool Product::expect_inverse_str() const {
	const std::optional<double> fctr_val = this->factor_value();
	if (fctr_val) {
		return *fctr_val < 0 && type(this->parent) == Type::sum;
	}
	return false;
}

void Product::to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const
{
	std::string new_line(dist_root * 5, ' ');	//building string with spaces matching dept of this
	new_line.append("product");
	tree_lines.push_back(std::move(new_line));
	append_last_line(tree_lines, line_prefix);

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
			for (auto it_red : redundant->factors) {
				it_red->parent = this;
			}
			this->factors.splice(this->factors.end(), redundant->factors);
			delete redundant;
			it = this->factors.erase(it);
		}
		else {
			++it;
		}
	}
	if (this->factors.size() == 1) {	//this only consists of one factor -> layer is not needed and this is deleted
		Basic_Term* const only_factor = *(this->factors.begin());
		storage_key = only_factor;
		only_factor->parent = this->parent;
		this->factors.clear();
		delete this;		
	}
}

Vals_Combined Product::combine_values()
{
	std::complex<double> buffer_factor = 1;
	bool only_known = true;
	for (auto it = this->factors.begin(); it != this->factors.end();) {
		const auto [known, factor] = (*it)->combine_values();
		if (known) {
			buffer_factor *= factor;
			delete (*it);
			it = this->factors.erase(it);
		}
		else {
			only_known = false;
			++it;
		}
	}
	if (only_known) {
		return { true, buffer_factor };
	}
	else {	//if product is known completely it will be deleted and replaced with buffer_factor one layer above anyway.
		if (buffer_factor != 1.0) {
			this->factors.push_front(new Value(buffer_factor, this));
		}
		return { false, 0 };
	}
}

std::complex<double> Product::evaluate(const std::list<bmath::Known_Variable>& known_variables) const
{
	std::complex<double> result(1);
	for (auto it : this->factors) {
		const std::complex<double> factor_combined = it->evaluate(known_variables);
		result *= factor_combined;
	}
	return result;
}

void Product::search_and_replace(const std::string& name_, const Basic_Term* value_, Basic_Term*& storage_key)
{
	for (auto &it : this->factors) {	//reference is needed in next line
		it->search_and_replace(name_, value_, it);
	}
}

void Product::list_subterms(std::list<Basic_Term*>& subterms, Type listed_type) const
{
	if (listed_type == Type::product) {
		subterms.push_back(const_cast<Product*>(this));
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
			if (argument_match != nullptr) {
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
		//the operator assumes from now on to have sorted products to compare
		auto it_this = this->factors.begin();
		auto it_other = other_product->factors.begin();
		for (; it_this != this->factors.end() && it_other != other_product->factors.end(); ++it_this, ++it_other) {
			if (!((**it_this) == (**it_other))) {
				return (**it_this) < (**it_other);
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
	:Basic_Term(parent_)
{
}

Sum::Sum(std::string_view name_, Basic_Term* parent_, std::size_t op)
	:Basic_Term(parent_)
{
	std::complex<double> known_summand = 0.0;
	while (op != std::string::npos) {
		const std::string_view subterm_view = name_.substr(op + 1);	//we don't want the operator itself to be part of the substr
		switch (name_[op]) {
		case '+':
			this->summands.push_front(build_subterm(subterm_view, this));
			break;
		case '-':
			this->summands.push_front(new Product(subterm_view, this, { -1, 0 }));
			break;
		}			
		name_.remove_suffix(name_.length() - op);
		op = find_last_of_skip_pars(name_, "+-");
	}
	//last part of name without any '+' or '-' in front
	if (name_.size()) {
		this->summands.push_front(build_subterm(name_, this));
	}
}

Sum::Sum(std::string_view name_, Basic_Term* parent_, std::size_t op, std::list<Pattern_Variable*>& variables)
	:Basic_Term(parent_)
{
	std::complex<double> known_summand = 0.0;
	while (op != std::string::npos) {
		const std::string_view subterm_view = name_.substr(op + 1);	//we don't want the operator itself to be part of the substr
		switch (name_[op]) {
		case '+':
			this->summands.push_front(build_pattern_subterm(subterm_view, this, variables));
			break;
		case '-':
			this->summands.push_front(new Product(subterm_view, this, { -1, 0 }, variables));
			break;
		}
		name_.remove_suffix(name_.length() - op);
		op = find_last_of_skip_pars(name_, "+-");
	}
	//last part of name without any '+' or '-' in front
	if (name_.size()) {
		this->summands.push_front(build_pattern_subterm(name_, this, variables));
	}
}

Sum::Sum(const Sum& source, Basic_Term* parent_)
	:Basic_Term(parent_)
{
	for (auto it : source.summands) {
		this->summands.push_back(copy_subterm(it, this));
	}
}

Sum::~Sum()
{
	for (auto it : this->summands) {
		delete it;
	}
}

void Sum::to_str(std::string& str) const
{
	const bool pars = type(this->parent) > this->get_type();
	if (pars) {
		str.push_back('(');
	}
	bool need_operator = false;
	for (auto it : this->summands) {
		if (it->expect_inverse_str()) {
			str.push_back('-');
			need_operator = true;
		}
		else if (std::exchange(need_operator, true)) {
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
			for (auto it_red : redundant->summands) {
				it_red->parent = this;
			}
			this->summands.splice(this->summands.end(), redundant->summands);
			delete redundant;
			it = this->summands.erase(it);
		}
		else {
			++it;
		}
	}
	if (this->summands.size() == 1) {	//this only consists of one summand -> layer is not needed
		Basic_Term* const only_summand = *(this->summands.begin());
		storage_key = only_summand;
		only_summand->parent = this->parent;
		this->summands.clear();
		delete this;
	}
}

Vals_Combined Sum::combine_values()
{
	std::complex<double> buffer_summand = 0;
	bool only_known = true;
	for (auto it = this->summands.begin(); it != this->summands.end();) {
		const auto [known, summand] = (*it)->combine_values();
		if (known) {
			buffer_summand += summand;
			delete (*it);
			it = this->summands.erase(it);
		}
		else {
			only_known = false;
			++it;
		}
	}
	if (only_known) {
		return { true, buffer_summand };
	}
	else {	//if sum is known completely it will be deleted and replaced with buffer_summand one layer above anyway.
		if (buffer_summand != 0.0) {
			this->summands.push_front(new Value(buffer_summand, this));
		}
		return { false, 0 };
	}	
}

std::complex<double> Sum::evaluate(const std::list<bmath::Known_Variable>& known_variables) const
{
	std::complex<double> result = 0;
	for (auto it : this->summands) {
		const std::complex<double> summand_combined = it->evaluate(known_variables);
		result += summand_combined;
	}
	return result;
}

void Sum::search_and_replace(const std::string& name_, const Basic_Term* value_, Basic_Term*& storage_key)
{
	for (auto &it : this->summands) {	//reference is needed in next line
		it->search_and_replace(name_, value_, it);
	}
}

void Sum::list_subterms(std::list<Basic_Term*>& subterms, Type listed_type) const
{
	if (listed_type == Type::sum) {
		subterms.push_back(const_cast<Sum*>(this));
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
			if (argument_match != nullptr) {
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
	const Sum* other_product = static_cast<const Sum*>(&other);
	if (this->summands.size() != other_product->summands.size()) {
		return false;
	}
	//the operator assumes from now on to have sorted products to compare
	auto it_this = this->summands.begin();
	auto it_other = other_product->summands.begin();
	for (; it_this != this->summands.end() && it_other != other_product->summands.end(); ++it_this, ++it_other) {
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
	:Basic_Term(parent_), base(nullptr), exponent(nullptr)
{
}

Exponentiation::Exponentiation(std::string_view name_, Basic_Term* parent_, std::size_t op)
	:Basic_Term(parent_)
{
	std::string_view subterm_view;
	subterm_view = name_.substr(op + 1);
	this->exponent = build_subterm(subterm_view, this);
	name_.remove_suffix(name_.length() - op);
	this->base = build_subterm(name_, this);
}

Exponentiation::Exponentiation(std::string_view base_, Basic_Term* parent_, std::complex<double> exponent_) 
	:Basic_Term(parent_), exponent(new Value(exponent_, this)), base(build_subterm(base_, this))
{
}

Exponentiation::Exponentiation(std::string_view name_, Basic_Term* parent_, std::size_t op, std::list<Pattern_Variable*>& variables)
	:Basic_Term(parent_)
{
	std::string_view subterm_view;
	subterm_view = name_.substr(op + 1);
	this->exponent = build_pattern_subterm(subterm_view, this, variables);
	name_.remove_suffix(name_.length() - op);
	this->base = build_pattern_subterm(name_, this, variables);
}

Exponentiation::Exponentiation(std::string_view base_, Basic_Term* parent_, std::complex<double> exponent_, std::list<Pattern_Variable*>& variables) 
	:Basic_Term(parent_), exponent(new Value(exponent_, this)), base(build_pattern_subterm(base_, this, variables))
{
}

Exponentiation::Exponentiation(const Exponentiation& source, Basic_Term* parent_)
	:Basic_Term(parent_), base(copy_subterm(source.base, this)), exponent(copy_subterm(source.exponent, this))
{
}

Exponentiation::~Exponentiation()
{
	delete exponent;
	delete base;
}

std::optional<double> Exponentiation::exponent_value() const {
	if (type(this->exponent) == Type::value) {
		const Value* const value = static_cast<Value*>(this->exponent);
		if (value->val.imag() == 0.0) {
			return value->val.real();
		}
	}
	return {};
}

void Exponentiation::to_str(std::string& str) const
{
	const std::optional<double> exp_val = this->exponent_value();
	const bool negative_exp = exp_val && *exp_val < 0.0;
	const bool pars = type(this->parent) >= this->get_type();
	if (pars) {
		str.push_back('(');
	}
	if (negative_exp && type(this->parent) != Type::product) {
		str.append("1/");
	}
	this->base->to_str(str);
	if (exp_val && std::abs(*exp_val) != 1.0) {
		str.push_back('^');
		const Value* const val = static_cast<Value*>(this->exponent);
		str.append(val->val_to_str(negative_exp));
	}
	else if (!exp_val) {
		str.push_back('^');
		this->exponent->to_str(str);
	}	
	if (pars) {
		str.push_back(')');
	}
}

bool Exponentiation::expect_inverse_str() const {
	const std::optional<double> exp_val = this->exponent_value();
	if (exp_val) {
		return *exp_val < 0.0 && type(this->parent) == Type::product;
	}
	return false;
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
		if (val_exp->val == 1.0) {
			storage_key = this->base;
			this->base->parent = this->parent;
			this->base = nullptr;
			delete this;
			return;
		}
		if (val_exp->val == 0.0) {
			storage_key = new Value({ 1.0, 0.0 }, this->parent);
			delete this;
			return;
		}
	}
	if (this->base->get_type() == Type::value) {
		Value* const val_base = static_cast<Value*>(this->base);
		if (val_base->val == 1.0) {
			storage_key = val_base;
			val_base->parent = this->parent;
			this->base = nullptr;
			delete this;
			return;
		}
		if (val_base->val == 0.0) {
			storage_key = new Value({ 0.0, 0.0 }, this->parent);
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

void Exponentiation::list_subterms(std::list<Basic_Term*>& subterms, Type listed_type) const
{
	if (listed_type == Type::exponentiation) {
		subterms.push_back(const_cast<Exponentiation*>(this));
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
		if (argument_match != nullptr) {
			return argument_match;
		}
		reset_pattern_vars(pattern_var_adresses);
		argument_match = exponent->match_intern(pattern, pattern_var_adresses, &exponent);
		if (argument_match != nullptr) {
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
		if (*(this->base) != *(other_exp->base)) {
			return *(this->base) < *(other_exp->base);
		}
		if (*(this->exponent) != *(other_exp->exponent)) {
			return *(this->exponent) < *(other_exp->exponent);
		}
	}
	return false;
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
	:Basic_Term(parent_), argument(nullptr), op_type(Par_Op_Type::log10)	//just some default initialisation
{
}

Par_Operator::Par_Operator(std::string_view name_, Basic_Term* parent_, Par_Op_Type op_type_)
	:Basic_Term(parent_), op_type(op_type_), argument(nullptr)
{
	name_.remove_suffix(1);								//closing parenthesis gets cut of
	name_.remove_prefix(strlen(par_op_name(op_type)));	//funktionname and opening parenthesis get cut of
	this->argument = build_subterm(name_, this);
}

Par_Operator::Par_Operator(std::string_view name_, Basic_Term* parent_, Par_Op_Type op_type_, std::list<Pattern_Variable*>& variables)
	:Basic_Term(parent_), op_type(op_type_), argument(nullptr)
{
	name_.remove_suffix(1);								//closing parenthesis gets cut of
	name_.remove_prefix(strlen(par_op_name(op_type)));	//funktionname and opening parenthesis get cut of
	this-> argument = build_pattern_subterm(name_, this, variables);
}


Par_Operator::Par_Operator(const Par_Operator & source, Basic_Term * parent_)
	:Basic_Term(parent_), argument(copy_subterm(source.argument, this)), op_type(source.op_type)
{
}

Par_Operator::~Par_Operator()
{
	delete this->argument;
}

void Par_Operator::to_str(std::string & str) const
{
	str.append(par_op_name(this->op_type));
	this->argument->to_str(str);
	str.push_back(')');
}

void Par_Operator::to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const
{
	std::string new_line(dist_root * 5, ' ');	//building string with spaces matching dept of this
	new_line.append(par_op_name(this->op_type));
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
	return { known, evaluate_par_op(val, this->op_type) };
}

std::complex<double> Par_Operator::evaluate(const std::list<bmath::Known_Variable>& known_variables) const
{
	
	return evaluate_par_op(argument->evaluate(known_variables), this->op_type);
}

void Par_Operator::search_and_replace(const std::string & name_, const Basic_Term* value_, Basic_Term*& storage_key)
{
	this->argument->search_and_replace(name_, value_, this->argument);	
}

void Par_Operator::list_subterms(std::list<Basic_Term*>& subterms, Type listed_type) const
{
	if (listed_type == Type::par_operator) {
		subterms.push_back(const_cast<Par_Operator*>(this));
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
		if (argument_match != nullptr) {
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
		if (*(this->argument) != *(other_par_op->argument)) {
			return *(this->argument) < *(other_par_op->argument);
		}
	}
	return false;
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
