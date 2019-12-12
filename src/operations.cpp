

#include <algorithm>

#include "operations.h"
#include "internal_functions.h"


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

		if (type_of(it) == Type::product) {	//beautifies sum to print '-'
			const Product* const product = static_cast<const Product*>(it);
			if (complies_with(product->operands.back(), Restriction::negative)) {
				product->to_str(str, operator_precedence(Type::sum));
				nothing_printed_yet = false;
				continue;
			}
		}

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
		summand->to_tree_str(tree_lines, dist_root + 1, '\0');
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

	return this->factoring() || this->unpack_minus() || this->factor_polinomial(&storage_key);
}

bool bmath::intern::Sum::factoring()
{
	for (auto product_it = find_first_of(this->operands, Type::product); product_it != this->operands.end() && type_of(*product_it) == Type::product; ++product_it) {
		std::list<Basic_Term*>& first_factors = static_cast<Product*>(*product_it)->operands;
		for (auto first_factor = first_factors.begin(); first_factor != first_factors.end(); ++first_factor) {
			if (complies_with(*first_factor, Restriction::minus_one)) {	//meaning first_factor == -1.0
				continue;	//dont want to pack "a-b-c" to "a-(b+c)", as this would make it harder to detect cases like "a-(a+b)" (easier to detect as "a-a-b")
			}
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

bool bmath::intern::Sum::unpack_minus()
{
	for (auto product_it = find_first_of(this->operands, Type::product); product_it != this->operands.end() && type_of(*product_it) == Type::product; ++product_it) {
		std::list<Basic_Term*>& factors = static_cast<Product*>(*product_it)->operands;	//operands is sorted with sums in front of values
		if (factors.size() == 2 && type_of(factors.front()) == Type::sum && complies_with(factors.back(), Restriction::minus_one)) {
			//we now know product_it to be of form "(a+b+...)*(-1)" and want to convert product_it's factors to summands in this: "a*(-1)+b*(-1)..."
			Sum* negative_sum = static_cast<Sum*>(factors.front());
			for (auto summand : negative_sum->operands) {
				this->operands.push_back(new Product(summand, -1.0));
			}
			negative_sum->operands.clear();	//product_it now of form "()*-1"
			delete* product_it;
			this->operands.erase(product_it);
			return true;
		}
	}
	return false;
}

bool bmath::intern::Sum::factor_polinomial(Basic_Term** storage_key)
{
	static Monom monom_pattern;	//Pattern to match summands with
	monom_pattern.full_reset();

	using Monom_Storage = std::pair<std::complex<double>, int>;	//first is factor a, second exponent n in a*x^n
	std::vector<Monom_Storage> monoms;
	monoms.reserve(this->operands.size());

	for (auto& summand : this->operands) {
		monom_pattern.partial_reset();
		if (monom_pattern.matching(summand, summand)) {
			monoms.push_back(std::make_pair(monom_pattern.factor(), monom_pattern.exponent()));
		}
		else {
			return false;
		}
	}

	//monom with biggest exponent in polynom
	auto [max_monom_fac, max_monom_exp] = *std::max_element(monoms.begin(), monoms.end(), [](const Monom_Storage& a, const Monom_Storage& b) {return a.second < b.second; });
	if (max_monom_exp < 2) {
		return false;
	}
	else {
		std::vector<std::complex<double>> polynom;	//at polynom[n] the factor in front of x^n is stored.
		polynom.reserve(max_monom_exp);
		for (int i = 0; i < max_monom_exp; i++) {
			polynom.push_back({ 0.0, 0.0 });
		}
		for (auto [monom_fac, monom_exp] : monoms) {
			if (monom_exp != max_monom_exp) {
				polynom[monom_exp] = monom_fac / max_monom_fac;
			}
		}
		find_roots(polynom);	//now polynom vector stores roots
		std::list<Basic_Term*> product;	//operands list of future product
		if (max_monom_fac != 1.0) {
			product.push_back(new Value(max_monom_fac));
		}
		for (auto root : polynom) {
			product.push_back(new Sum({ monom_pattern.copy_base(), new Value(-root) }));
		}
		delete this;
		*storage_key = new Product(std::move(product));
		return true;
	}
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
				this->operands.push_front(new Power(new_subterm, { -1, 0 }));
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
				this->operands.push_front(new Power(new_subterm, { -1, 0 }));
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
	for (const auto it : backwards(this->operands)) {

		if (type_of(it) == Type::power) {	//cancerous part to beautify 
			const Power* pow_it = static_cast<const Power*>(it);
			if (complies_with(pow_it->expo, Restriction::negative)) {
				if (!std::exchange(nothing_printed_yet, false)) {
					str.push_back('/');
				}
				else {
					str.append("1/");
				}
				pow_it->base->to_str(str, operator_precedence(Type::power));
				if (complies_with(pow_it->expo, Restriction::not_minus_one)) {
					str.push_back('^');
					const Value* const exponent_val = static_cast<const Value*>(pow_it->expo);
					str.append(to_string(exponent_val->val(), operator_precedence(Type::power), true));
				}
				continue;
			}
		}

		if (complies_with(it, Restriction::minus_one) && this->operands.size() > 1) {
			str.push_back('-');
		}
		else if (complies_with(it, Restriction::negative) && caller_operator_precedence == operator_precedence(Type::sum)) {	//still cancerous :(
			it->to_str(str, operator_precedence(Type::sum));
			nothing_printed_yet = false;
		}
		else {
			if (!std::exchange(nothing_printed_yet, false)) {
				str.push_back('*');
			}
			it->to_str(str, operator_precedence(Type::product));
		}
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
		factor->to_tree_str(tree_lines, dist_root + 1, '\0');
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
	return this->unpack_division();
}

bool bmath::intern::Product::unpack_division()
{
	for (auto pow_it = find_first_of(this->operands, Type::power); pow_it != this->operands.end() && type_of(*pow_it) == Type::power; ++pow_it) {
		Power* const power = static_cast<Power*>(*pow_it);
		if (complies_with(power->expo, Restriction::minus_one) && type_of(power->base) == Type::product) {
			//we now know pow_it to be of form "(a*b*...)^(-1)" and want to convert pow_it's base to factors in this: "a^(-1)*b^(-1)..."
			Product* const base_product = static_cast<Product*>(power->base);
			for (auto factor : base_product->operands) {
				this->operands.push_back(new Power(factor, -1.0));
			}
			base_product->operands.clear();
			delete power;
			this->operands.erase(pow_it);
			return true;
		}
	}
	return false;
}

const std::vector<Transformation*> Product::product_transforms = transforms_of(Type::product);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Power\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Power::Power()
	:base(nullptr), expo(nullptr)
{
}

Power::Power(std::string_view name_, std::size_t op)
{
	std::string_view subterm_view;
	subterm_view = name_.substr(op + 1);
	this->expo = build_subterm(subterm_view);
	name_.remove_suffix(name_.length() - op);
	this->base = build_subterm(name_);
}

Power::Power(Basic_Term* base_, std::complex<double> exponent_) 
	:expo(new Value(exponent_)), base(base_)
{
}

bmath::intern::Power::Power(Basic_Term* base_, Basic_Term* expo_)
	:base(base_), expo(expo_)
{
}

Power::Power(std::string_view name_, std::size_t op, std::list<Pattern_Variable*>& variables)
{
	std::string_view subterm_view;
	subterm_view = name_.substr(op + 1);
	this->expo = build_pattern_subterm(subterm_view, variables);
	name_.remove_suffix(name_.length() - op);
	this->base = build_pattern_subterm(name_, variables);
}

Power::Power(const Power& source)
	:base(copy_subterm(source.base)), expo(copy_subterm(source.expo))
{
}

Power::~Power()
{
	delete expo;
	delete base;
}

void Power::to_str(std::string& str, int caller_operator_precedence) const
{
	const bool pars = caller_operator_precedence >= operator_precedence(Type::power);
	if (pars) {
		str.push_back('(');
	}

	if (complies_with(this->expo, Restriction::negative)) {	//beautifies negative exponents
		str.append("1/");
		this->base->to_str(str, operator_precedence(Type::power));
		if (complies_with(this->expo, Restriction::not_minus_one)) {
			str.push_back('^');
			const Value* const exponent_val = static_cast<const Value*>(this->expo);
			str.append(to_string(exponent_val->val(), operator_precedence(Type::power), true));
		}
	}
	else {
		this->base->to_str(str, operator_precedence(Type::power));
		str.push_back('^');
		this->expo->to_str(str, operator_precedence(Type::power));
	}
	
	if (pars) {
		str.push_back(')');
	}
}

void Power::to_tree_str(std::vector<std::string>& tree_lines, unsigned int dist_root, char line_prefix) const
{
	std::string new_line(dist_root * 5, ' ');	//building string with spaces matching dept of this
	new_line.append("power");
	tree_lines.push_back(std::move(new_line));
	append_last_line(tree_lines, line_prefix);

	this->base->to_tree_str(tree_lines, dist_root + 1, '_');
	this->expo->to_tree_str(tree_lines, dist_root + 1, '^');
}

Type Power::type() const
{
	return Type::power;
}

bool Power::combine_layers(Basic_Term*& storage_key)
{
	bool changed = false;
	changed |= this->base->combine_layers(this->base);
	changed |= this->expo->combine_layers(this->expo);
	return changed;
}

Vals_Combined Power::combine_values()
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

std::complex<double> Power::evaluate(const std::list<bmath::Known_Variable>& known_variables) const
{
	const std::complex<double> base_ = this->base->evaluate(known_variables);
	const std::complex<double> exponent_ = this->expo->evaluate(known_variables);
	return std::pow(base_, exponent_);
}

void Power::search_and_replace(const std::string& name_, const Basic_Term* value_, Basic_Term*& storage_key)
{
	this->base->search_and_replace(name_, value_, this->base);
	this->expo->search_and_replace(name_, value_, this->expo);
}

void bmath::intern::Power::for_each(std::function<void(Basic_Term* this_ptr, Type this_type)> func)
{
	this->base->for_each(func);
	this->expo->for_each(func);
	func(this, Type::power);
}

bool bmath::intern::Power::transform(Basic_Term *& storage_key)
{
	if (this->base->transform(this->base)) {
		return true;
	}
	if (this->expo->transform(this->expo)) {
		return true;
	}

	for (auto trans : pow_transforms) {
		reset_all_pattern_vars(trans->var_adresses);
		if (this->equal_to_pattern(trans->input, nullptr, storage_key)) {
			replace(&storage_key, trans);
			return true;
		}
	}
	return false;
}

bool bmath::intern::Power::equal_to_pattern(Basic_Term* pattern, Basic_Term* patterns_parent, Basic_Term *& storage_key)
{
	const Type pattern_type = type_of(pattern);
	if (pattern_type == Type::power) {
		const Power* pattern_exp = static_cast<const Power*>(pattern);
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

void bmath::intern::Power::reset_own_matches(Basic_Term* parent)
{
	this->expo->reset_own_matches(this);
	this->base->reset_own_matches(this);
}

bool Power::operator<(const Basic_Term& other) const
{
	if (Type::power != type_of(other)) {
		return Type::power < type_of(other);
	}
	else {
		const Power* other_exp = static_cast<const Power*>(&other);
		if (*(this->base) < *(other_exp->base)) {
			return true;
		}
		if (*(other_exp->base) < *(this->base)) {
			return false;
		}
		return *(this->expo) < *(other_exp->expo);
	}
}

bool Power::operator==(const Basic_Term& other) const
{
	if (type_of(other) == Type::power) {
		const Power* other_exp = static_cast<const Power*>(&other);
		if (*(this->base) != *(other_exp->base)) {
			return false;
		}
		return *(this->expo) == *(other_exp->expo);
	}
	return false;
}

const std::vector<Transformation*> Power::pow_transforms = transforms_of(Type::power);


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

bool Par_Operator::combine_layers(Basic_Term*& storage_key)
{
	return this->argument->combine_layers(this->argument);
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


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Monom\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bmath::intern::Monom::Monom()
	:a("a", Restriction::value), n("n", Restriction::natural), x("x", Restriction::none), 
	x_n(&x, &n), monom({ &a, &x_n }), a_x({&a, &x})
{
}

bmath::intern::Monom::~Monom()
{
	x_n.expo = nullptr;
	x_n.base = nullptr;
	monom.clear_operands();
	a_x.clear_operands();
}

bool bmath::intern::Monom::matching(Basic_Term* test, Basic_Term*& storage_key)
{
	static Value zero({ 0.0, 0.0 });
	static Value one({ 1.0, 0.0 });

	if (test->equal_to_pattern(&this->monom, nullptr, storage_key)) {	//check whole monom
		return true;
	}
	if (test->equal_to_pattern(&this->x_n, nullptr, storage_key)) {		//check for x^n
		a.matched_term = &one;
		return true;
	}
	if (test->equal_to_pattern(&this->a_x, nullptr, storage_key)) {		//check for a*x
		n.matched_term = &one;
		return true;
	}
	if (test->equal_to_pattern(&this->a, nullptr, storage_key)) {		//check for a
		n.matched_term = &zero;
		return true;
	}
	if (x.is_matched() && test->equal_to_pattern(&this->x, nullptr, storage_key)) {	//check for x
		a.matched_term = &one;
		n.matched_term = &one;
		return true;
	}
	return false;
}

void bmath::intern::Monom::full_reset()
{
	this->a.reset();
	this->n.reset();
	this->x.reset();
}

void bmath::intern::Monom::partial_reset()
{
	this->a.reset();
	this->n.reset();
}

std::complex<double> bmath::intern::Monom::factor() const
{
	return this->a.matched_value();
}

int bmath::intern::Monom::exponent() const
{
	return static_cast<int>(this->n.matched_value().real());
}

Basic_Term* bmath::intern::Monom::copy_base() const
{
	return this->x.copy_matched_term();
}

