
#include "operations.h"
#include "internal_functions.h"

//#include <sstream>


using namespace bmath::intern;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Sum\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Sum::Sum()
	: Variadic_Operator<add, Type::sum, 0>()
{
}

Sum::Sum(std::string_view name_, std::size_t op)
	: Variadic_Operator<add, Type::sum, 0>()
{
	const Value_Manipulator value_add = { &(this->operand.val()), add };
	const Value_Manipulator value_sub = { &(this->operand.val()), sub };
	Basic_Term* new_subterm = nullptr;

	while (op != std::string::npos) {
		const std::string_view subterm_view = name_.substr(op + 1);	//we don't want the operator itself to be part of the substr
		switch (name_[op]) {
		case '+':
			new_subterm = build_subterm(subterm_view, value_add);
			if (new_subterm) {
				this->operands.push_front(new_subterm);
			}
			break;
		case '-':
			new_subterm = build_subterm(subterm_view, value_sub);
			if (new_subterm) {
				this->operands.push_front(new Product(new_subterm, { -1, 0 }));
			}
			break;
		}
		name_.remove_suffix(name_.length() - op);
		op = find_last_of_skip_pars(name_, "+-");
	}
	//last part of name without any '+' or '-' in front
	if (name_.size()) {
		Basic_Term* const new_subterm = build_subterm(name_, value_add);
		if (new_subterm != nullptr) {
			this->operands.push_front(new_subterm);
		}
	}
}

Sum::Sum(std::string_view name_, std::size_t op, std::list<Pattern_Variable*>& variables)
	: Variadic_Operator<add, Type::sum, 0>()
{
	const Value_Manipulator value_add = { &(this->operand.val()), add };
	const Value_Manipulator value_sub = { &(this->operand.val()), sub };
	Basic_Term* new_subterm = nullptr;

	while (op != std::string::npos) {
		const std::string_view subterm_view = name_.substr(op + 1);	//we don't want the operator itself to be part of the substr
		switch (name_[op]) {
		case '+':
			new_subterm = build_pattern_subterm(subterm_view, variables, value_add);
			if (new_subterm) {
				this->operands.push_front(new_subterm);
			}
			break;
		case '-':
			new_subterm = build_pattern_subterm(subterm_view, variables, value_sub);
			if (new_subterm) {
				this->operands.push_front(new Product(new_subterm, { -1, 0 }));
			}
			break;
		}
		name_.remove_suffix(name_.length() - op);
		op = find_last_of_skip_pars(name_, "+-");
	}
	//last part of name without any '+' or '-' in front
	if (name_.size()) {
		new_subterm = build_pattern_subterm(name_, variables, value_add);
		if (new_subterm != nullptr) {
			this->operands.push_front(new_subterm);
		}
	}
}

Sum::Sum(const Sum& source)
	:Variadic_Operator<add, Type::sum, 0>(source)
{
	//nothing to do here
}

void Sum::to_str(std::string& str, Type caller_type) const
{
	const bool pars = caller_type > Type::sum;
	if (pars) {
		str.push_back('(');
	}
	bool nothing_printed_yet = true;
	if (this->operand.val() != 0.0) {
		this->operand.to_str(str, Type::sum);
		nothing_printed_yet = false;
	}
	for (const auto it : this->operands) {
		if (!std::exchange(nothing_printed_yet, false)) {
			str.push_back('+');
		}
		it->to_str(str, Type::sum);
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

Product::Product()
	: Variadic_Operator<mul, Type::product, 1>()
{
}

Product::Product(std::string_view name_, std::size_t op)
	: Variadic_Operator<mul, Type::product, 1>()
{
	const Value_Manipulator value_mul = { &(this->operand.val()), mul };
	const Value_Manipulator value_div = { &(this->operand.val()), div };
	Basic_Term* new_subterm = nullptr;

	while (op != std::string::npos) {
		const std::string_view subterm_view = name_.substr(op + 1);	//we don't want the operator itself to be part of the substr
		switch (name_[op]) {
		case '*':
			new_subterm = build_subterm(subterm_view, value_mul);
			if (new_subterm) {
				this->operands.push_front(new_subterm);
			}
			break;
		case '/':
			new_subterm = build_subterm(subterm_view, value_div);
			if (new_subterm) {
				this->operands.push_front(new Exponentiation(new_subterm, { -1, 0 }));
			}
			break;
		}
		name_.remove_suffix(name_.length() - op);
		op = find_last_of_skip_pars(name_, "*/");
	}
	//last part of name without any '*' or '/' in front
	new_subterm = build_subterm(name_, value_mul);
	if (new_subterm != nullptr) {
		this->operands.push_front(new_subterm);
	}
}

Product::Product(Basic_Term* name_, std::complex<double> factor)
	: Variadic_Operator<mul, Type::product, 1>()
{
	this->operand.val() = factor;
	if (name_->get_type() == Type::product) {	//not making new factor in this, but taking the existing factors of name_ instead
		Product* const name_product = static_cast<Product*>(name_);
		this->operands.splice(this->operands.end(), name_product->operands);
		this->operand.val() *= name_product->operand.val();
		delete name_product;
	}
	else {
		this->operands.push_back(name_);
	}
}

Product::Product(std::string_view name_, std::size_t op, std::list<Pattern_Variable*>& variables)
	: Variadic_Operator<mul, Type::product, 1>()
{
	const Value_Manipulator value_mul = { &this->operand.val(), mul };
	const Value_Manipulator value_div = { &this->operand.val(), div };
	Basic_Term* new_subterm = nullptr;

	while (op != std::string::npos) {
		const std::string_view subterm_view = name_.substr(op + 1);	//we don't want the operator itself to be part of the substr
		switch (name_[op]) {
		case '*':
			new_subterm = build_pattern_subterm(subterm_view, variables, value_mul);
			if (new_subterm) {
				this->operands.push_front(new_subterm);
			}
			break;
		case '/':
			new_subterm = build_pattern_subterm(subterm_view, variables, value_div);
			if (new_subterm) {
				this->operands.push_front(new Exponentiation(new_subterm, { -1, 0 }));
			}
			break;
		}
		name_.remove_suffix(name_.length() - op);
		op = find_last_of_skip_pars(name_, "*/");
	}
	//last part of name without any '*' or '/' in front
	new_subterm = build_pattern_subterm(name_, variables, value_mul);
	if (new_subterm != nullptr) {
		this->operands.push_front(new_subterm);
	}
}

Product::Product(const Product& source)
	: Variadic_Operator<mul, Type::product, 1>(source)
{	
	//nothing to do here
}

void Product::to_str(std::string& str, Type caller_type) const
{
	const bool pars = caller_type > Type::product;
	if (pars) {
		str.push_back('(');
	}
	bool nothing_printed_yet = true;
	if (this->operand.val() != 1.0) {
		this->operand.to_str(str, Type::product);
		nothing_printed_yet = false;
	}
	for (const auto it : this->operands) {
		if (!std::exchange(nothing_printed_yet, false)) {
			str.push_back('*');
		}
		it->to_str(str, Type::product);
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


Exponentiation::Exponentiation()
	:base(nullptr), exponent(nullptr)
{
}

Exponentiation::Exponentiation(std::string_view name_, std::size_t op)
{
	std::string_view subterm_view;
	subterm_view = name_.substr(op + 1);
	this->exponent = build_subterm(subterm_view);
	name_.remove_suffix(name_.length() - op);
	this->base = build_subterm(name_);
}

Exponentiation::Exponentiation(Basic_Term* base_, std::complex<double> exponent_) 
	:exponent(new Value(exponent_)), base(base_)
{
}

Exponentiation::Exponentiation(std::string_view name_, std::size_t op, std::list<Pattern_Variable*>& variables)
{
	std::string_view subterm_view;
	subterm_view = name_.substr(op + 1);
	this->exponent = build_pattern_subterm(subterm_view, variables);
	name_.remove_suffix(name_.length() - op);
	this->base = build_pattern_subterm(name_, variables);
}

Exponentiation::Exponentiation(const Exponentiation& source)
	:base(copy_subterm(source.base)), exponent(copy_subterm(source.exponent))
{
}

Exponentiation::~Exponentiation()
{
	delete exponent;
	delete base;
}

void Exponentiation::to_str(std::string& str, Type caller_type) const
{
	str.push_back('(');
	this->base->to_str(str, Type::exponentiation);
	str.push_back('^');
	this->exponent->to_str(str, Type::exponentiation);
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
			this->base = nullptr;	//otherwise the term now owned by parent of this, formerly this base, would be destroyed
			delete this;
			return;
		}
		if (val_exp->val() == 0.0) {
			storage_key = new Value({ 1.0, 0.0 });
			delete this;
			return;
		}
	}
	if (this->base->get_type() == Type::value) {
		Value* const val_base = static_cast<Value*>(this->base);
		if (val_base->val() == 1.0) {
			storage_key = val_base;
			this->base = nullptr;
			delete this;
			return;
		}
		if (val_base->val() == 0.0) {
			storage_key = new Value({ 0.0, 0.0 });
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
			this->base = new Value(base_.val);
		}
	}
	else if (!base_.known && exponent_.known) {
		if (type(this->exponent) != Type::value) {
			delete this->exponent;
			this->exponent = new Value(exponent_.val);
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

void bmath::intern::Exponentiation::for_each(std::function<void(Basic_Term* this_ptr, Type this_type)> func)
{
	this->base->for_each(func);
	this->exponent->for_each(func);
	func(this, Type::exponentiation);
}

Basic_Term** Exponentiation::match_intern(Basic_Term* pattern, std::list<Pattern_Variable*>& pattern_var_adresses, Basic_Term** storage_key)
{
	if (this->equal_to_pattern(pattern, storage_key)) {
		return storage_key;
	}
	else {
		reset_all_pattern_vars(pattern_var_adresses);
		Basic_Term** argument_match = base->match_intern(pattern, pattern_var_adresses, &base);
		if (argument_match) {
			return argument_match;
		}
		reset_all_pattern_vars(pattern_var_adresses);
		argument_match = exponent->match_intern(pattern, pattern_var_adresses, &exponent);
		if (argument_match) {
			return argument_match;
		}
		reset_all_pattern_vars(pattern_var_adresses);
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

Par_Operator::Par_Operator()
	:argument(nullptr), op_type(Par_Op_Type::log10)	//just some default initialisation for op_type
{
}

Par_Operator::Par_Operator(std::string_view name_, Par_Op_Type op_type_)
	:op_type(op_type_), argument(nullptr)
{
	name_.remove_suffix(1);								//closing parenthesis gets cut of
	name_.remove_prefix(name_of(op_type).length());	//funktionname and opening parenthesis get cut of
	this->argument = build_subterm(name_);
}

Par_Operator::Par_Operator(std::string_view name_, Par_Op_Type op_type_, std::list<Pattern_Variable*>& variables)
	:op_type(op_type_), argument(nullptr)
{
	name_.remove_suffix(1);								//closing parenthesis gets cut of
	name_.remove_prefix(name_of(op_type).length());	//funktionname and opening parenthesis get cut of
	this-> argument = build_pattern_subterm(name_, variables);
}


Par_Operator::Par_Operator(const Par_Operator & source)
	:argument(copy_subterm(source.argument)), op_type(source.op_type)
{
}

Par_Operator::~Par_Operator()
{
	delete this->argument;
}

void Par_Operator::to_str(std::string & str, Type caller_type) const
{
	str.append(name_of(this->op_type));	//already includes '('
	this->argument->to_str(str, Type::par_operator);
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

void bmath::intern::Par_Operator::for_each(std::function<void(Basic_Term* this_ptr, Type this_type)> func)
{
	this->argument->for_each(func);
	func(this, Type::par_operator);
}

Basic_Term** Par_Operator::match_intern(Basic_Term* pattern, std::list<Pattern_Variable*>& pattern_var_adresses, Basic_Term** storage_key)
{
	if (this->equal_to_pattern(pattern, storage_key)) {
		return storage_key;
	}
	else {
		reset_all_pattern_vars(pattern_var_adresses);
		Basic_Term** argument_match = argument->match_intern(pattern, pattern_var_adresses, &argument);
		if (argument_match) {
			return argument_match;
		}
		reset_all_pattern_vars(pattern_var_adresses);
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
