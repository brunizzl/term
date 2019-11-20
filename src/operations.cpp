
#include "operations.h"
#include "internal_functions.h"

//#include <sstream>


using namespace bmath::intern;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Sum\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Sum::Sum()
{
}

Sum::Sum(std::string_view name_, std::size_t op)
{
	std::complex<double> value = 0;
	const Value_Manipulator value_add = { &value, add };
	const Value_Manipulator value_sub = { &value, sub };
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
	if (value != 0.0) {
		this->operands.push_back(new Value(value));
	}
}

Sum::Sum(std::string_view name_, std::size_t op, std::list<Pattern_Variable*>& variables)
{
	std::complex<double> value = 0;
	const Value_Manipulator value_add = { &value, add };
	const Value_Manipulator value_sub = { &value, sub };
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
	if (value != 0.0) {
		this->operands.push_back(new Value(value));
	}
}

bmath::intern::Sum::Sum(std::list<Basic_Term*>&& operands)
	:Variadic_Operator<add, Type::sum, 0>(std::move(operands))
{
}

Sum::Sum(const Sum& source)
	:Variadic_Operator<add, Type::sum, 0>(source)
{
}

void Sum::to_str(std::string& str, int caller_operator_precedence) const
{
	const bool pars = caller_operator_precedence >= operator_precedence(Type::sum);
	if (pars) {
		str.push_back('(');
	}
	bool nothing_printed_yet = true;
	for (const auto it : this->operands) {
		if (!std::exchange(nothing_printed_yet, false)) {
			str.push_back('+');
		}
		it->to_str(str, operator_precedence(Type::sum));
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

	for (auto summand : this->operands) {
		summand->to_tree_str(tree_lines, dist_root + 1, '+');
	}
}

bool bmath::intern::Sum::transform(Basic_Term *& storage_key)
{
	for (auto& summand : this->operands) {
		if (summand->transform(summand)) {
			return true;
		}
	}

	for (auto trans : sum_transforms) {
		reset_all_pattern_vars(trans->var_adresses);
		Basic_Term** const found_part = this->part_equal_to_pattern(trans->input, nullptr, storage_key);
		if (found_part) {
			replace(found_part, trans);
			return true;
		}
	}

	return this->factoring();
}

bool bmath::intern::Sum::factoring()
{
	for (auto product_it = find_first_of(this->operands, Type::product); product_it != this->operands.end() && type_of(*product_it) == Type::product; ++product_it) {
		std::list<Basic_Term*>& first_factors = static_cast<Product*>(*product_it)->operands;
		for (auto first_factor = first_factors.begin(); first_factor != first_factors.end(); ++first_factor) {
			//assume this sum has form "a*b+a*c+d+..."
			for (auto next_product_it = std::next(product_it); next_product_it != this->operands.end() && type_of(*next_product_it) == Type::product; ++next_product_it) {
				std::list<Basic_Term*>& next_factors = static_cast<Product*>(*next_product_it)->operands;
				for (auto next_factor = next_factors.begin(); next_factor != next_factors.end(); ++next_factor) {
					if ((**first_factor) == (**next_factor)) { //both are "a"
						next_factors.erase(next_factor);	//now this sum has form "a*b+1*c+d+..."
						Product* outer_product = new Product;
						outer_product->operands.splice(outer_product->operands.end(), first_factors, first_factor);	//now this sum has form "1*b+1*c+d+..."
						Sum* inner_sum = new Sum;
						inner_sum->operands.splice(inner_sum->operands.end(), this->operands, product_it);	//this sum has form "1*c+d+..."
						inner_sum->operands.splice(inner_sum->operands.end(), this->operands, next_product_it); //this sum has form "d+..."
						outer_product->operands.push_back(inner_sum);	//outher_product has form "a*(b+c)"
						this->operands.push_back(outer_product);	//this sum has final form "a*(b+c)+d+..."
						return true;
					}
				}
			}
			//assume this sum to be of form "a*b+a+d+..."
			for (auto next_summand = find_first_of(this->operands, type_of(*first_factor)); next_summand != this->operands.end() && type_of(*next_summand) == type_of(*first_factor); ++next_summand) {
				if ((**first_factor) == (**next_summand)) {	//both are "a"
					Product* outer_product = new Product;
					outer_product->operands.splice(outer_product->operands.end(), first_factors, first_factor); //now this sum has form "1*b+a+d+..."
					Sum* inner_sum = new Sum;
					inner_sum->operands.splice(inner_sum->operands.end(), this->operands, product_it);	//this sum has form "a+d+..."
					inner_sum->push_back(new Value(1.0));
					outer_product->push_back(inner_sum);
					this->operands.erase(next_summand);		//this sum now has form "d+..."
					this->operands.push_back(outer_product); // this sum has final form "a*(b+1)+d+..."
					return true;
				}
			}
		}
	}
	return false;
}

bool bmath::intern::Sum::factor_polinomial(Basic_Term** storage_key)
{
	//static Pattern monom("a!value*x^n!value");
	//jetzt irgendwie auch direkten zugriff auf nur x und nur x^n erlauben
	//idee: habe nicht pattern, sondern pattern_term und manage a, x und x^n direkt.
	return false;
}

const std::vector<Transformation*> Sum::sum_transforms = transforms_of(Type::sum);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Product\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Product::Product()
{
}

Product::Product(std::string_view name_, std::size_t op)
{
	std::complex<double> value = 1;
	const Value_Manipulator value_mul = { &value, mul };
	const Value_Manipulator value_div = { &value, div };
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
	if (value != 1.0) {
		this->operands.push_back(new Value(value));
	}
}

Product::Product(Basic_Term* name_, std::complex<double> factor)
{
	if (type_of(name_) == Type::product) {	//not making new factor in this, but taking the existing factors of name_ instead
		Product* const name_product = static_cast<Product*>(name_);
		this->operands.splice(this->operands.end(), name_product->operands);
		delete name_product;
	}
	else {
		this->operands.push_back(name_);
	}
	this->operands.push_back(new Value(factor));
}

Product::Product(std::string_view name_, std::size_t op, std::list<Pattern_Variable*>& variables)
{
	std::complex<double> value = 1;
	const Value_Manipulator value_mul = { &value, mul };
	const Value_Manipulator value_div = { &value, div };
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
	if (value != 1.0) {
		this->operands.push_back(new Value(value));
	}
}

bmath::intern::Product::Product(std::list<Basic_Term*>&& operands)
	:Variadic_Operator<mul, Type::product, 1>(std::move(operands))
{
}

Product::Product(const Product& source)
	: Variadic_Operator<mul, Type::product, 1>(source)
{	
	//nothing to do here
}

void Product::to_str(std::string& str, int caller_operator_precedence) const
{
	const bool pars = caller_operator_precedence >= operator_precedence(Type::product);
	if (pars) {
		str.push_back('(');
	}
	bool nothing_printed_yet = true;
	for (const auto it : this->operands) {
		if (!std::exchange(nothing_printed_yet, false)) {
			str.push_back('*');
		}
		it->to_str(str, operator_precedence(Type::product));
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

	for (const auto factor : this->operands) {
		factor->to_tree_str(tree_lines, dist_root + 1, '*');
	}
}

bool bmath::intern::Product::transform(Basic_Term *& storage_key)
{
	for (auto& factor : this->operands) {
		if (factor->transform(factor)) {
			return true;
		}
	}

	for (auto trans : product_transforms) {
		reset_all_pattern_vars(trans->var_adresses);
		Basic_Term** const found_part = this->part_equal_to_pattern(trans->input, nullptr, storage_key);
		if (found_part) {
			replace(found_part, trans);
			return true;
		}
	}
	return false;
}

const std::vector<Transformation*> Product::product_transforms = transforms_of(Type::product);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Exponentiation\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Exponentiation::Exponentiation()
	:base(nullptr), expo(nullptr)
{
}

Exponentiation::Exponentiation(std::string_view name_, std::size_t op)
{
	std::string_view subterm_view;
	subterm_view = name_.substr(op + 1);
	this->expo = build_subterm(subterm_view);
	name_.remove_suffix(name_.length() - op);
	this->base = build_subterm(name_);
}

Exponentiation::Exponentiation(Basic_Term* base_, std::complex<double> exponent_) 
	:expo(new Value(exponent_)), base(base_)
{
}

Exponentiation::Exponentiation(std::string_view name_, std::size_t op, std::list<Pattern_Variable*>& variables)
{
	std::string_view subterm_view;
	subterm_view = name_.substr(op + 1);
	this->expo = build_pattern_subterm(subterm_view, variables);
	name_.remove_suffix(name_.length() - op);
	this->base = build_pattern_subterm(name_, variables);
}

Exponentiation::Exponentiation(const Exponentiation& source)
	:base(copy_subterm(source.base)), expo(copy_subterm(source.expo))
{
}

Exponentiation::~Exponentiation()
{
	delete expo;
	delete base;
}

void Exponentiation::to_str(std::string& str, int caller_operator_precedence) const
{
	const bool pars = caller_operator_precedence >= operator_precedence(Type::exponentiation);
	if (pars) {
		str.push_back('(');
	}
	this->base->to_str(str, operator_precedence(Type::exponentiation));
	str.push_back('^');
	this->expo->to_str(str, operator_precedence(Type::exponentiation));
	if (pars) {
		str.push_back(')');
	}
}

void Exponentiation::to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const
{
	std::string new_line(dist_root * 5, ' ');	//building string with spaces matching dept of this
	new_line.append("exponentiation");
	tree_lines.push_back(std::move(new_line));
	append_last_line(tree_lines, line_prefix);

	this->base->to_tree_str(tree_lines, dist_root + 1, '_');
	this->expo->to_tree_str(tree_lines, dist_root + 1, '^');
}

Type Exponentiation::type() const
{
	return Type::exponentiation;
}

void Exponentiation::combine_layers(Basic_Term*& storage_key)
{
	this->base->combine_layers(this->base);
	this->expo->combine_layers(this->expo);
	if (type_of(this->expo) == Type::value) {
		Value* const val_exp = static_cast<Value*>(this->expo);
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
	if (type_of(this->base) == Type::value) {
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
	const Vals_Combined exponent_ = this->expo->combine_values();

	if (base_.known && exponent_.known) {
		const std::complex<double> result = std::pow(base_.val, exponent_.val);
		return { true, result };
	}
	else if (base_.known && !exponent_.known) {
		if (type_of(this->base) != Type::value) {
			delete this->base;
			this->base = new Value(base_.val);
		}
	}
	else if (!base_.known && exponent_.known) {
		if (type_of(this->expo) != Type::value) {
			delete this->expo;
			this->expo = new Value(exponent_.val);
		}
	}

	return { false, 0 };
}

std::complex<double> Exponentiation::evaluate(const std::list<bmath::Known_Variable>& known_variables) const
{
	const std::complex<double> base_ = this->base->evaluate(known_variables);
	const std::complex<double> exponent_ = this->expo->evaluate(known_variables);
	return std::pow(base_, exponent_);
}

void Exponentiation::search_and_replace(const std::string& name_, const Basic_Term* value_, Basic_Term*& storage_key)
{
	this->base->search_and_replace(name_, value_, this->base);
	this->expo->search_and_replace(name_, value_, this->expo);
}

void bmath::intern::Exponentiation::for_each(std::function<void(Basic_Term* this_ptr, Type this_type)> func)
{
	this->base->for_each(func);
	this->expo->for_each(func);
	func(this, Type::exponentiation);
}

bool bmath::intern::Exponentiation::transform(Basic_Term *& storage_key)
{
	if (this->base->transform(this->base)) {
		return true;
	}
	if (this->expo->transform(this->expo)) {
		return true;
	}

	for (auto trans : exp_transforms) {
		reset_all_pattern_vars(trans->var_adresses);
		if (this->equal_to_pattern(trans->input, nullptr, storage_key)) {
			replace(&storage_key, trans);
			return true;
		}
	}
	return false;
}

bool bmath::intern::Exponentiation::equal_to_pattern(Basic_Term* pattern, Basic_Term* patterns_parent, Basic_Term *& storage_key)
{
	const Type pattern_type = type_of(pattern);
	if (pattern_type == Type::exponentiation) {
		const Exponentiation* pattern_exp = static_cast<const Exponentiation*>(pattern);
		if (!this->base->equal_to_pattern(pattern_exp->base, pattern, this->base)) {
			return false;
		}
		if (!this->expo->equal_to_pattern(pattern_exp->expo, pattern, this->expo)) {
			return false;
		}
		return true;
	}
	else if (pattern_type == Type::pattern_variable) {
		Pattern_Variable* pattern_var = static_cast<Pattern_Variable*>(pattern);
		return pattern_var->try_matching(this, patterns_parent, storage_key);
	}
	else {
		return false;
	}
}

void bmath::intern::Exponentiation::reset_own_matches(Basic_Term* parent)
{
	this->expo->reset_own_matches(this);
	this->base->reset_own_matches(this);
}

bool Exponentiation::operator<(const Basic_Term& other) const
{
	if (Type::exponentiation != type_of(other)) {
		return Type::exponentiation < type_of(other);
	}
	else {
		const Exponentiation* other_exp = static_cast<const Exponentiation*>(&other);
		if (*(this->base) < *(other_exp->base)) {
			return true;
		}
		if (*(other_exp->base) < *(this->base)) {
			return false;
		}
		return *(this->expo) < *(other_exp->expo);
	}
}

bool Exponentiation::operator==(const Basic_Term& other) const
{
	if (type_of(other) == Type::exponentiation) {
		const Exponentiation* other_exp = static_cast<const Exponentiation*>(&other);
		if (*(this->base) != *(other_exp->base)) {
			return false;
		}
		return *(this->expo) == *(other_exp->expo);
	}
	return false;
}

const std::vector<Transformation*> Exponentiation::exp_transforms = transforms_of(Type::exponentiation);


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

void Par_Operator::to_str(std::string & str, int caller_operator_precedence) const
{
	str.append(name_of(this->op_type));	//already includes '('
	this->argument->to_str(str, operator_precedence(Type::par_operator));
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

Type Par_Operator::type() const
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

bool bmath::intern::Par_Operator::transform(Basic_Term *& storage_key)
{
	if (this->argument->transform(this->argument)) {
		return true;
	}

	for (auto trans : parop_transforms) {
		reset_all_pattern_vars(trans->var_adresses);
		if (this->equal_to_pattern(trans->input, nullptr, storage_key)) {
			replace(&storage_key, trans);
			return true;
		}
	}
	return false;
}

bool bmath::intern::Par_Operator::equal_to_pattern(Basic_Term* pattern, Basic_Term* patterns_parent, Basic_Term *& storage_key)
{
	const Type pattern_type = type_of(pattern);
	if (pattern_type == Type::par_operator) {
		const Par_Operator* const pattern_par_op = static_cast<const Par_Operator*>(pattern);
		if (this->op_type != pattern_par_op->op_type) {
			return false;
		}
		return this->argument->equal_to_pattern(pattern_par_op->argument, pattern, this->argument);
	}
	else if (pattern_type == Type::pattern_variable) {
		Pattern_Variable* pattern_var = static_cast<Pattern_Variable*>(pattern);
		return pattern_var->try_matching(this, patterns_parent, storage_key);
	}
	else {
		return false;
	}
}

void bmath::intern::Par_Operator::reset_own_matches(Basic_Term* parent)
{
	this->argument->reset_own_matches(this);
}

bool Par_Operator::operator<(const Basic_Term& other) const
{
	if (Type::par_operator != type_of(other)) {
		return Type::par_operator < type_of(other);
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
	if (type_of(other) == Type::par_operator) {
		const Par_Operator* other_par_op = static_cast<const Par_Operator*>(&other);
		if (this->op_type != other_par_op->op_type) {
			return false;
		}
		return *(this->argument) == *(other_par_op->argument);
	}
	return false;
}

const std::vector<Transformation*> Par_Operator::parop_transforms = transforms_of(Type::par_operator);
