
#include "operations.h"

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
	std::vector<std::string_view> exposed_parts;
	find_exposed_parts(name_, exposed_parts);

	while (op != std::string::npos) {
		const std::string_view subterm_view = name_.substr(op + 1);
		switch (name_[op]) {
		case '*':
			this->factors.push_front(build_subterm(subterm_view, this));
			break;
		case '/':
			this->factors.push_front(new Exponentiation(subterm_view, this, { -1, 0 }));
			break;
		}
		name_.remove_suffix(name_.length() - op);
		update_views(name_, exposed_parts);
		op = find_last_of_in_views(name_, exposed_parts, "*/");
	}
	this->factors.push_front(build_subterm(name_, this));
}

bmath::intern::Product::Product(std::string_view name_, Basic_Term* parent_, std::complex<double> factor) 
	:Basic_Term(parent_), factors({ new Value( factor, this ), build_subterm(name_, this) })
{
}

Product::Product(std::string_view name_, Basic_Term* parent_, std::size_t op, std::list<Pattern_Variable*>& variables)
	:Basic_Term(parent_)
{
	std::vector<std::string_view> exposed_parts;
	find_exposed_parts(name_, exposed_parts);

	while (op != std::string::npos) {
		const std::string_view subterm_view = name_.substr(op + 1);
		switch (name_[op]) {
		case '*':
			this->factors.push_front(build_pattern_subterm(subterm_view, this, variables));
			break;
		case '/':
			this->factors.push_front(new Exponentiation(subterm_view, this, { -1, 0 }, variables));
			break;
		}
		name_.remove_suffix(name_.length() - op);
		update_views(name_, exposed_parts);
		op = find_last_of_in_views(name_, exposed_parts, "*/");
	}
	this->factors.push_front(build_pattern_subterm(name_, this, variables));
}

bmath::intern::Product::Product(std::string_view name_, Basic_Term* parent_, std::complex<double> factor, std::list<Pattern_Variable*>& variables) 
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

void Product::to_str(std::string& str) const
{
	if (type(this->parent) >= this->get_type()) {
		str.push_back('(');
	}
	bool already_printed_smth = false;
	for (auto it : this->factors) {
		if (std::exchange(already_printed_smth, true)) {
			str.push_back('*');
		}
		it->to_str(str);
	}
	if (!already_printed_smth) {
		str.push_back('1');
	}
	/*for (auto it : this->divisors) {
		str.push_back('/');
		it->to_str(str);
	}*/
	if (type(this->parent) >= this->get_type()) {
		str.push_back(')');
	}
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
	return product;
}

void Product::combine_layers(Basic_Term*& storage_key)
{
	for (auto &it = this->factors.begin(); it != this->factors.end();) {	//reference is needed in next line
		(*it)->combine_layers(*it);
		if (type(*it) == product) {
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
		const Vals_Combined factor = (*it)->combine_values();
		if (factor.known) {
			buffer_factor *= factor.val;
			delete (*it);
			it = this->factors.erase(it);
		}
		else {
			only_known = false;
			++it;
		}
	}
	if (only_known) {
		return Vals_Combined{ true, buffer_factor };
	}
	else {	//if product is known completely it will be deleted and replaced with buffer_factor one layer above anyway.
		if (buffer_factor != 1.0) {
			this->factors.push_front(new Value(buffer_factor, this));
		}
		return Vals_Combined{ false, 0 };
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
	if (listed_type == product) {
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
	case product:
		break;
	case pattern_variable:
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
	std::vector<std::string_view> exposed_parts;
	find_exposed_parts(name_, exposed_parts);

	while (op != std::string::npos) {
		const std::string_view subterm_view = name_.substr(op + 1);
		switch (name_[op]) {
		case '+':
			this->summands.push_front(build_subterm(subterm_view, this));
			break;
		case '-':
			this->summands.push_front(new Product(subterm_view, this, { -1, 0 }));
			break;
		}
		name_.remove_suffix(name_.length() - op);
		update_views(name_, exposed_parts);
		op = find_last_of_in_views(name_, exposed_parts, "+-");
	}
	if (name_.size() != 0) {
		this->summands.push_front(build_subterm(name_, this));
	}
}

Sum::Sum(std::string_view name_, Basic_Term* parent_, std::size_t op, std::list<Pattern_Variable*>& variables)
	:Basic_Term(parent_)
{
	std::vector<std::string_view> exposed_parts;
	find_exposed_parts(name_, exposed_parts);

	while (op != std::string::npos) {
		const std::string_view subterm_view = name_.substr(op + 1);
		switch (name_[op]) {
		case '+':
			this->summands.push_front(build_pattern_subterm(subterm_view, this, variables));
			break;
		case '-':
			this->summands.push_front(new Product(subterm_view, this, { -1, 0 }, variables));
			break;
		}
		name_.remove_suffix(name_.length() - op);
		update_views(name_, exposed_parts);
		op = find_last_of_in_views(name_, exposed_parts, "+-");
	}
	if (name_.size() != 0) {
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
	if (type(this->parent) >= this->get_type()) {
		str.push_back('(');
	}
	bool need_operator = false;
	for (auto it : this->summands) {
		if (std::exchange(need_operator, true)) {
			str.push_back('+');
		}
		it->to_str(str);
	}
	/*for (auto it : this->subtractors) {
		str.push_back('-');
		it->to_str(str);
	}*/
	if (type(this->parent) >= this->get_type()) {
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
	return sum;
}

void Sum::combine_layers(Basic_Term*& storage_key)
{
	for (auto &it = this->summands.begin(); it != this->summands.end();) {	//reference is needed next line
		(*it)->combine_layers(*it);
		if (type(*it) == sum) {
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
		const Vals_Combined summand = (*it)->combine_values();
		if (summand.known) {
			buffer_summand += summand.val;
			delete (*it);
			it = this->summands.erase(it);
		}
		else {
			only_known = false;
			++it;
		}
	}
	if (only_known) {
		return Vals_Combined{ true, buffer_summand };
	}
	else {	//if sum is known completely it will be deleted and replaced with buffer_summand one layer above anyway.
		if (buffer_summand != 0.0) {
			this->summands.push_front(new Value(buffer_summand, this));
		}
		return Vals_Combined{ false, 0 };
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
	if (listed_type == sum) {
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
	case sum:
		break;
	case pattern_variable:
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

bmath::intern::Exponentiation::Exponentiation(std::string_view base_, Basic_Term* parent_, std::complex<double> exponent_) 
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

bmath::intern::Exponentiation::Exponentiation(std::string_view base_, Basic_Term* parent_, std::complex<double> exponent_, std::list<Pattern_Variable*>& variables) 
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

void Exponentiation::to_str(std::string& str) const
{
	if (type(this->parent) >= this->get_type()) {
		str.push_back('(');
	}
	this->base->to_str(str);
	str.push_back('^');
	this->exponent->to_str(str);
	if (type(this->parent) >= this->get_type()) {
		str.push_back(')');
	}
}

void bmath::intern::Exponentiation::to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const
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
	return exponentiation;
}

void Exponentiation::combine_layers(Basic_Term*& storage_key)
{
	this->base->combine_layers(this->base);
	this->exponent->combine_layers(this->exponent);
	if (this->exponent->get_type() == value) {
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
	if (this->base->get_type() == value) {
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
		return Vals_Combined{ true, result };
	}
	else if (base_.known && !exponent_.known) {
		if (type(this->base) != value) {
			delete this->base;
			this->base = new Value(base_.val, this);
		}
	}
	else if (!base_.known && exponent_.known) {
		if (type(this->exponent) != value) {
			delete this->exponent;
			this->exponent = new Value(exponent_.val, this);
		}
	}

	return Vals_Combined{ false, 0 };
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
	if (listed_type == exponentiation) {
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
	case exponentiation:
		break;
	case pattern_variable:
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

const char* Par_Operator::op_name(Par_Op_Type op_type)
{
	switch (op_type) {
	case log10:
		return "log10(";
	case asinh:
		return "asinh(";
	case acosh:
		return "acosh(";
	case atanh:
		return "atanh(";
	case asin:
		return "asin(";
	case acos:
		return "acos(";
	case atan:
		return "atan(";
	case sinh:
		return "sinh(";
	case cosh:
		return "cosh(";
	case tanh:
		return "tanh(";
	case sqrt:
		return "sqrt(";
	case exp:
		return "exp(";
	case sin:
		return "sin(";
	case cos:
		return "cos(";
	case tan:
		return "tan(";
	case abs:
		return "abs(";
	case arg:
		return "arg(";
	case ln:
		return "ln(";
	case re:
		return "re(";
	case im:
		return "im(";
	}
	return nullptr;
}

Par_Operator::Par_Operator(Basic_Term* parent_)
	:Basic_Term(parent_), argument(nullptr), op_type(error)
{
}

Vals_Combined Par_Operator::internal_combine(Vals_Combined argument_) const
{
	if (argument_.known) {
		switch (this->op_type) {
		case log10:
			return Vals_Combined{ true, std::log10(argument_.val) };
		case asin:
			return Vals_Combined{ true, std::asin(argument_.val) };
		case acos:			
			return Vals_Combined{ true, std::acos(argument_.val) };
		case atan:		
			return Vals_Combined{ true, std::atan(argument_.val) };
		case asinh:			
			return Vals_Combined{ true, std::asinh(argument_.val) };
		case acosh:			
			return Vals_Combined{ true, std::acosh(argument_.val) };
		case atanh:		
			return Vals_Combined{ true, std::atanh(argument_.val) };
		case sinh:	
			return Vals_Combined{ true, std::sinh(argument_.val) };
		case cosh:			
			return Vals_Combined{ true, std::cosh(argument_.val) };
		case tanh:			
			return Vals_Combined{ true, std::tanh(argument_.val) };
		case sqrt:			
			return Vals_Combined{ true, std::sqrt(argument_.val) };
		case exp:				
			return Vals_Combined{ true, std::exp(argument_.val) };
		case sin:				
			return Vals_Combined{ true, std::sin(argument_.val) };
		case cos:			
			return Vals_Combined{ true, std::cos(argument_.val) };
		case tan:			
			return Vals_Combined{ true, std::tan(argument_.val) };
		case abs:			
			return Vals_Combined{ true, std::abs(argument_.val) };
		case arg:			
			return Vals_Combined{ true, std::arg(argument_.val) };
		case ln:				
			return Vals_Combined{ true, std::log(argument_.val) };
		case re:			
			return Vals_Combined{ true, std::real(argument_.val) };
		case im:				
			return Vals_Combined{ true, std::imag(argument_.val) };
		}
	}
	return Vals_Combined{ false, 0 };
}

Par_Operator::Par_Operator(std::string_view name_, Basic_Term* parent_, Par_Op_Type op_type_)
	:Basic_Term(parent_), op_type(op_type_), argument(nullptr)
{
	name_.remove_suffix(1);							//closing parenthesis gets cut of
	name_.remove_prefix(strlen(op_name(op_type)));	//funktionname and opening parenthesis get cut of
	this->argument = build_subterm(name_, this);
}

Par_Operator::Par_Operator(std::string_view name_, Basic_Term* parent_, Par_Op_Type op_type_, std::list<Pattern_Variable*>& variables)
	:Basic_Term(parent_), op_type(op_type_), argument(nullptr)
{
	name_.remove_suffix(1);							//closing parenthesis gets cut of
	name_.remove_prefix(strlen(op_name(op_type)));	//funktionname and opening parenthesis get cut of
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
	str.append(op_name(this->op_type));
	this->argument->to_str(str);
	str.push_back(')');
}

void bmath::intern::Par_Operator::to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const
{
	std::string new_line(dist_root * 5, ' ');	//building string with spaces matching dept of this
	new_line.append(op_name(this->op_type));
	new_line.pop_back();
	tree_lines.push_back(std::move(new_line));
	append_last_line(tree_lines, line_prefix);

	this->argument->to_tree_str(tree_lines, dist_root + 1, '\0');
}

Type Par_Operator::get_type() const
{
	return par_operator;
}

void Par_Operator::combine_layers(Basic_Term*& storage_key)
{
	this->argument->combine_layers(this->argument);
}

Vals_Combined Par_Operator::combine_values()
{
	return this->internal_combine(argument->combine_values());
}

std::complex<double> Par_Operator::evaluate(const std::list<bmath::Known_Variable>& known_variables) const
{
	//the return type and parameter of internal_combine is not std::complex but Vals_Combined. However, this contains std::complex as val
	return this->internal_combine(Vals_Combined{ true, argument->evaluate(known_variables) }).val;
}

void Par_Operator::search_and_replace(const std::string & name_, const Basic_Term* value_, Basic_Term*& storage_key)
{
	this->argument->search_and_replace(name_, value_, this->argument);	
}

void Par_Operator::list_subterms(std::list<Basic_Term*>& subterms, Type listed_type) const
{
	if (listed_type == par_operator) {
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
	case par_operator:
		break;
	case pattern_variable:
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
