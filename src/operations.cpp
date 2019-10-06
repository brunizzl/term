
#include "operations.h"
#include "internal_functions.h"

//#include <sstream>


using namespace bmath::intern;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Sum\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Sum::Sum(Basic_Term* parent_)
	: Variadic_Operator<add, Type::sum, 0>(parent_)
{
}

Sum::Sum(std::string_view name_, Basic_Term* parent_, std::size_t op)
	: Variadic_Operator<add, Type::sum, 0>(parent_)
{
	const Value_Manipulator value_add = { &(this->operand.val()), add };
	const Value_Manipulator value_sub = { &(this->operand.val()), sub };
	Basic_Term* new_subterm = nullptr;

	while (op != std::string::npos) {
		const std::string_view subterm_view = name_.substr(op + 1);	//we don't want the operator itself to be part of the substr
		switch (name_[op]) {
		case '+':
			new_subterm = build_subterm(subterm_view, this, value_add);
			if (new_subterm) {
				this->operands.push_front(new_subterm);
			}
			break;
		case '-':
			new_subterm = build_subterm(subterm_view, this, value_sub);
			if (new_subterm) {
				this->operands.push_front(new Product(new_subterm, this, { -1, 0 }));
			}
			break;
		}
		name_.remove_suffix(name_.length() - op);
		op = find_last_of_skip_pars(name_, "+-");
	}
	//last part of name without any '+' or '-' in front
	if (name_.size()) {
		Basic_Term* const new_subterm = build_subterm(name_, this, value_add);
		if (new_subterm != nullptr) {
			this->operands.push_front(new_subterm);
		}
	}
}

Sum::Sum(std::string_view name_, Basic_Term* parent_, std::size_t op, std::list<Pattern_Variable*>& variables)
	: Variadic_Operator<add, Type::sum, 0>(parent_)
{
	const Value_Manipulator value_add = { &(this->operand.val()), add };
	const Value_Manipulator value_sub = { &(this->operand.val()), sub };
	Basic_Term* new_subterm = nullptr;

	while (op != std::string::npos) {
		const std::string_view subterm_view = name_.substr(op + 1);	//we don't want the operator itself to be part of the substr
		switch (name_[op]) {
		case '+':
			new_subterm = build_pattern_subterm(subterm_view, this, variables, value_add);
			if (new_subterm) {
				this->operands.push_front(new_subterm);
			}
			break;
		case '-':
			new_subterm = build_pattern_subterm(subterm_view, this, variables, value_sub);
			if (new_subterm) {
				this->operands.push_front(new Product(new_subterm, this, { -1, 0 }));
			}
			break;
		}
		name_.remove_suffix(name_.length() - op);
		op = find_last_of_skip_pars(name_, "+-");
	}
	//last part of name without any '+' or '-' in front
	if (name_.size()) {
		new_subterm = build_pattern_subterm(name_, this, variables, value_add);
		if (new_subterm != nullptr) {
			this->operands.push_front(new_subterm);
		}
	}
}

Sum::Sum(const Sum& source, Basic_Term* parent_)
	:Variadic_Operator<add, Type::sum, 0>(source, parent_)
{
	//nothing to do here
}

void Sum::to_str(std::string& str) const
{
	const bool pars = type(this->parent_ptr) > this->get_type();
	if (pars) {
		str.push_back('(');
	}
	bool nothing_printed_yet = true;
	if (this->operand.val() != 0.0) {
		this->operand.to_str(str);
		nothing_printed_yet = false;
	}
	for (const auto it : this->operands) {
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

	this->operand.to_tree_str(tree_lines, dist_root + 1, '+');

	for (auto summand : this->operands) {
		summand->to_tree_str(tree_lines, dist_root + 1, '+');
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Product\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Product::Product(Basic_Term* parent_)
	: Variadic_Operator<mul, Type::product, 1>(parent_)
{
}

Product::Product(std::string_view name_, Basic_Term* parent_, std::size_t op)
	: Variadic_Operator<mul, Type::product, 1>(parent_)
{
	const Value_Manipulator value_mul = { &(this->operand.val()), mul };
	const Value_Manipulator value_div = { &(this->operand.val()), div };
	Basic_Term* new_subterm = nullptr;

	while (op != std::string::npos) {
		const std::string_view subterm_view = name_.substr(op + 1);	//we don't want the operator itself to be part of the substr
		switch (name_[op]) {
		case '*':
			new_subterm = build_subterm(subterm_view, this, value_mul);
			if (new_subterm) {
				this->operands.push_front(new_subterm);
			}
			break;
		case '/':
			new_subterm = build_subterm(subterm_view, this, value_div);
			if (new_subterm) {
				this->operands.push_front(new Exponentiation(new_subterm, this, { -1, 0 }));
			}
			break;
		}
		name_.remove_suffix(name_.length() - op);
		op = find_last_of_skip_pars(name_, "*/");
	}
	//last part of name without any '*' or '/' in front
	new_subterm = build_subterm(name_, this, value_mul);
	if (new_subterm != nullptr) {
		this->operands.push_front(new_subterm);
	}
}

Product::Product(Basic_Term* name_, Basic_Term* parent_, std::complex<double> factor)
	: Variadic_Operator<mul, Type::product, 1>(parent_)
{
	this->operand.val() = factor;

	this->operands.push_back(name_);
	name_->set_parent(this);
}

Product::Product(std::string_view name_, Basic_Term* parent_, std::size_t op, std::list<Pattern_Variable*>& variables)
	: Variadic_Operator<mul, Type::product, 1>(parent_)
{
	const Value_Manipulator value_mul = { &this->operand.val(), mul };
	const Value_Manipulator value_div = { &this->operand.val(), div };
	Basic_Term* new_subterm = nullptr;

	while (op != std::string::npos) {
		const std::string_view subterm_view = name_.substr(op + 1);	//we don't want the operator itself to be part of the substr
		switch (name_[op]) {
		case '*':
			new_subterm = build_pattern_subterm(subterm_view, this, variables, value_mul);
			if (new_subterm) {
				this->operands.push_front(new_subterm);
			}
			break;
		case '/':
			new_subterm = build_pattern_subterm(subterm_view, this, variables, value_div);
			if (new_subterm) {
				this->operands.push_front(new Exponentiation(new_subterm, this, { -1, 0 }));
			}
			break;
		}
		name_.remove_suffix(name_.length() - op);
		op = find_last_of_skip_pars(name_, "*/");
	}
	//last part of name without any '*' or '/' in front
	new_subterm = build_pattern_subterm(name_, this, variables, value_mul);
	if (new_subterm != nullptr) {
		this->operands.push_front(new_subterm);
	}
}

Product::Product(const Product& source, Basic_Term* parent_)
	: Variadic_Operator<mul, Type::product, 1>(source, parent_)
{	
	//nothing to do here
}

void Product::to_str(std::string& str) const
{
	const bool pars = type(this->parent_ptr) > this->get_type();
	if (pars) {
		str.push_back('(');
	}
	bool nothing_printed_yet = true;
	if (this->operand.val() != 1.0) {
		this->operand.to_str(str);
		nothing_printed_yet = false;
	}
	for (const auto it : this->operands) {
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

	this->operand.to_tree_str(tree_lines, dist_root + 1, '*');

	for (const auto factor : this->operands) {
		factor->to_tree_str(tree_lines, dist_root + 1, '*');
	}
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
		if (val_exp->val() == 1.0) {
			storage_key = this->base;
			this->base->set_parent(this->parent_ptr);
			this->base = nullptr;
			delete this;
			return;
		}
		if (val_exp->val() == 0.0) {
			storage_key = new Value({ 1.0, 0.0 }, this->parent_ptr);
			delete this;
			return;
		}
	}
	if (this->base->get_type() == Type::value) {
		Value* const val_base = static_cast<Value*>(this->base);
		if (val_base->val() == 1.0) {
			storage_key = val_base;
			val_base->set_parent(this->parent_ptr);
			this->base = nullptr;
			delete this;
			return;
		}
		if (val_base->val() == 0.0) {
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
	if (this->equal_to_pattern(pattern, storage_key)) {
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

bool bmath::intern::Exponentiation::equal_to_pattern(Basic_Term* pattern, Basic_Term** storage_key)
{
	const Type pattern_type = pattern->get_type();
	if (pattern_type == Type::exponentiation) {
		const Exponentiation* pattern_exp = static_cast<const Exponentiation*>(pattern);
		if (!this->base->equal_to_pattern(pattern_exp->base, &this->base)) {
			return false;
		}
		if (!this->exponent->equal_to_pattern(pattern_exp->exponent, &this->exponent)) {
			return false;
		}
		return true;
	}
	else if (pattern_type == Type::pattern_variable) {
		Pattern_Variable* pattern_var = static_cast<Pattern_Variable*>(pattern);
		return pattern_var->try_matching(this, storage_key);
	}
	else {
		return false;
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
	if (other.get_type() == Type::exponentiation) {
		const Exponentiation* other_exp = static_cast<const Exponentiation*>(&other);
		if (*(this->base) != *(other_exp->base)) {
			return false;
		}
		return *(this->exponent) == *(other_exp->exponent);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Parenthesis_Operator\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Par_Operator::Par_Operator(Basic_Term* parent_)
	:parent_ptr(parent_), argument(nullptr), op_type(Par_Op_Type::log10)	//just some default initialisation for op_type
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
	if (this->equal_to_pattern(pattern, storage_key)) {
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

bool bmath::intern::Par_Operator::equal_to_pattern(Basic_Term* pattern, Basic_Term** storage_key)
{
	const Type pattern_type = pattern->get_type();
	if (pattern_type == Type::par_operator) {
		const Par_Operator* pattern_par_op = static_cast<const Par_Operator*>(pattern);
		if (this->op_type != pattern_par_op->op_type) {
			return false;
		}
		return this->argument->equal_to_pattern(pattern_par_op->argument, &this->argument);
	}
	else if (pattern_type == Type::pattern_variable) {
		Pattern_Variable* pattern_var = static_cast<Pattern_Variable*>(pattern);
		return pattern_var->try_matching(this, storage_key);
	}
	else {
		return false;
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
	if (other.get_type() == Type::par_operator) {
		const Par_Operator* other_par_op = static_cast<const Par_Operator*>(&other);
		if (this->op_type != other_par_op->op_type) {
			return false;
		}
		return *(this->argument) == *(other_par_op->argument);
	}
	return false;
}
