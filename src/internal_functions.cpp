
#include <charconv>
#include <cassert>
#include <algorithm>

#include "internal_functions.h"
#include "arguments.h"
#include "operations.h"

using namespace bmath::intern;

void bmath::intern::preprocess_str(std::string& str)
{
	int par_diff = 0;	//counts '(' and ')' (parentheses)
	int brac_diff = 0;	//counts '[' and ']' (brackets)
	std::size_t last_operator = std::string::npos - 1;	//remembers the position of last '+', '-', '*', '/', '^'  (-1 to allow operator at str[0])

	for (std::size_t i = 0; i < str.length(); i++) {	//deleting whitespace and counting parentheses
		switch (str[i]) {
		case '\t':
		case '\n':
		case ' ': 
			str.erase(i--, 1);	//erase this char -> set i one back, as string got shorter
			break;
		case '(': 
			par_diff++;
			break;
		case ')': 
			if (--par_diff < 0) {	//one can not have more closing, than opening parentheses at any point
				throw XTermConstructionError("the parentheses or brackets of string do not obey the syntax rules");
			}
			break;
		case '[': 
			brac_diff++;
			str[i] = '(';	//later functions expect only parentheses, not brackets
			break;
		case ']':
			if (--brac_diff < 0) {	//one can not have more closing, than opening brackets at any point
				throw XTermConstructionError("the parentheses or brackets of string do not obey the syntax rules");
			}
			str[i] = ')';	//later functions expect only parentheses, not brackets
			break;
		case '+':
		case '*':
		case '/':
		case '^':
			if (i == 0 || i == str.length() - 1) {
				throw XTermConstructionError("found binary operator (+ * / ^) at beginning or end of string");
			}
			else if (str[i - 1] == '(' || str[i + 1] == ')') {
				throw XTermConstructionError("found binary operator (+ * / ^) next to enclosing bracket or parenthesis");
			}
		case '-':
			if (last_operator == i - 1) {
				throw XTermConstructionError("found two operators (+ - * / ^) in direct succession");
			}
			last_operator = i;
			break;
		default:
			break; //nothing to do this character
		}
	}
	if (par_diff != 0 || brac_diff != 0) {
		throw XTermConstructionError("the parentheses or brackets of string do not obey the syntax rules");
	}
	const char* const allowed_chars = "1234567890.abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ+-*/^[]()!_";	//reserving '#', '$', and braces for internal stuff
	if (str.find_first_not_of(allowed_chars) != std::string::npos) {
		throw XTermConstructionError("string contains characters other than: 1234567890.abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ+-*/^[]()!_");
	}
}

std::size_t bmath::intern::find_open_par(std::size_t clsd_par, const std::string_view name)
{
	unsigned int deeper_clsd_par = 0;	//counts how many closed parentheses have been encountered minus how many open parentheses
	std::size_t nxt_par = clsd_par;
	while (true) {
		nxt_par = name.find_last_of("()", nxt_par - 1);
		assert(nxt_par != std::string::npos);	//function preprocess_str() already guarantees valid parentheses.

		switch (name[nxt_par]) {
		case ')':
			deeper_clsd_par++;
			break;
		case '(':
			if (deeper_clsd_par-- == 0) {
				return nxt_par;
			}
			break;
		}
	}
}

std::size_t bmath::intern::find_last_of_skip_pars(const std::string_view name, const char* const characters)
{
	std::size_t open_par = name.length() - 1;
	std::size_t clsd_par = name.find_last_of(')');
	while (clsd_par != std::string::npos) {
		const std::string_view search_view(name.data() + clsd_par, open_par - clsd_par);
		const std::size_t found = search_view.find_last_of(characters);
		if (found != std::string::npos) {
			return found + clsd_par;	//search_view starts with offset of clsd_par (two lines above). this offset has to be added to get distance from begin of name
		} 
		else {
			open_par = find_open_par(clsd_par, name);
			clsd_par = name.find_last_of(')', open_par);	//one could start the search with an offset of one, only to have an underflow of open_par == 0
		}
	}
	return name.find_last_of(characters, open_par);
}

std::size_t bmath::intern::find_last_of_skip_pars(const std::string_view name, const char character)
{
	std::size_t open_par = name.length() - 1;
	std::size_t clsd_par = name.find_last_of(')');
	while (clsd_par != std::string::npos) {
		const std::string_view search_view(name.data() + clsd_par, open_par - clsd_par);
		const std::size_t found = search_view.find_last_of(character);
		if (found != std::string::npos) {
			return found + clsd_par;	//search_view starts with offset of clsd_par (two lines above). this offset has to be added to get distance from begin of name
		}
		else {
			open_par = find_open_par(clsd_par, name);
			clsd_par = name.find_last_of(')', open_par);	//one could start the search with an offset of one, only to have an underflow of open_par == 0
		}
	}
	return name.find_last_of(character, open_par);
}

Type bmath::intern::type_subterm(const std::string_view name, std::size_t& op, Par_Op_Type& par_op_type)
{
	//starting search for "basic" operators
	op = find_last_of_skip_pars(name, "+-");
	if (op != std::string::npos) {
		return Type::sum;
	}
	op = find_last_of_skip_pars(name, "*/");
	if (op != std::string::npos) {
		return Type::product;
	}
	op = find_last_of_skip_pars(name, '^');
	if (op != std::string::npos) {
		return Type::exponentiation;
	}
	//searching for parenthesis operators 
	for (Par_Op_Type op_type : all_par_op_types) {
		const std::string_view op_name = name_of(op_type);
		if (name.compare(0, op_name.length(), op_name) == 0) {
			par_op_type = op_type;
			return Type::par_operator;
		}
	}
	if (name.front() == '(' && name.back() == ')') {	
		return Type::undefined;
	}
	if (name.length()) {
		return Type::variable;
	}
	throw XTermConstructionError("string is not of expected format");
}

Basic_Term* bmath::intern::build_subterm(std::string_view subterm, Value_Manipulator manipulator)
{
	if (is_computable(subterm)) {
		const std::complex<double> result = compute(subterm);
		if (manipulator.key != nullptr) {
			assert(manipulator.func != nullptr);
			manipulator.func(manipulator.key, result);
			return nullptr;
		}
		else {
			return new Value(result);
		}
	}
	while (subterm.size() > 0) {

		std::size_t op = std::string::npos;
		Par_Op_Type par_op_type;	//only used if parenthesis operator is found
		Type type = type_subterm(subterm, op, par_op_type);

		switch (type) {
		case Type::exponentiation:
			return new Exponentiation(subterm, op);
		case Type::product:
			return new Product(subterm, op);
		case Type::sum:
			return new Sum(subterm, op);
		case Type::variable:
			return new Variable(subterm);
		case Type::par_operator:
			return new Par_Operator(subterm, par_op_type);
			//case value is already handled at beginning of function
		}
		//not variable/value -> string contains parentheses, but no operation outside was found. 
		subterm.remove_prefix(1); //chopping of enclosing parentheses
		subterm.remove_suffix(1);
	}
	throw XTermConstructionError("could not find any type to build term(function build_subterm)");
}

Basic_Term* bmath::intern::build_pattern_subterm(std::string_view subterm, std::list<Pattern_Variable*>& variables, Value_Manipulator manipulator)
{
	if (is_computable(subterm)) {
		const std::complex<double> result = compute(subterm);
		if (manipulator.key != nullptr) {
			assert(manipulator.func != nullptr);
			manipulator.func(manipulator.key, result);
			return nullptr;
		}
		else {
			return new Value(result);
		}
	}
	while (subterm.size() > 0) {

		std::size_t op = std::string::npos;
		Par_Op_Type par_op_type;	//only used if parenthesis operator is found
		Type type = type_subterm(subterm, op, par_op_type);

		switch (type) {
		case Type::exponentiation:
			return new Exponentiation(subterm, op, variables);
		case Type::product:
			return new Product(subterm, op, variables);
		case Type::sum:
			return new Sum(subterm, op, variables);
		case Type::par_operator:
			return new Par_Operator(subterm, par_op_type, variables);
		case Type::variable:
			{
				Type restriction = Type::undefined;
				const std::size_t bang = subterm.find_first_of('!');
				if (bang != std::string::npos) {
					//subterm is expected to be of format "name!type". type_name only views "type"
					std::string_view type_name(subterm.data() + bang + 1, subterm.size() - bang - 1);
					for (Type type : all_types) {
						if (name_of(type) == type_name) {
							restriction = type;
							break;
						}
					}
					subterm.remove_suffix(subterm.size() - bang);	//"name!type -> "name"
				}
				for (auto& variable : variables) {
					if (variable->name == subterm) {
						if (variable->type_restriction == Type::undefined) {
							variable->type_restriction = restriction;
						}
						return variable;
					}
				}
				Pattern_Variable* new_variable = new Pattern_Variable(subterm, restriction);
				variables.push_back(new_variable);
				return new_variable;
			}			
		//case value is already handled at beginning of function
		}
		//not variable/value -> string contains parentheses, but no operation outside was found. 
		subterm.remove_prefix(1); //chopping of enclosing parentheses
		subterm.remove_suffix(1);
	}
	throw XTermConstructionError("could not find any type to build term (function build_pattern_subterm)");
}

Basic_Term* bmath::intern::copy_subterm(const Basic_Term* source)
{
	switch (type_of(source)) {
	case Type::par_operator:
		return new Par_Operator(*(static_cast<const Par_Operator*>(source)));
	case Type::value:
		return new Value(*(static_cast<const Value*>(source)));
	case Type::variable:
		return new Variable(*(static_cast<const Variable*>(source)));
	case Type::sum:
		return new Sum(*(static_cast<const Sum*>(source)));
	case Type::product:
		return new Product(*(static_cast<const Product*>(source)));
	case Type::exponentiation:
		return new Exponentiation(*(static_cast<const Exponentiation*>(source)));
	case Type::pattern_variable: {
		//as copy subterm is called, when a pattern has succesfully been matched and should be transformed, 
		//we do not want to actually copy the pattern_variable, but the subterm held by it.
		const Pattern_Variable* const pattern_variable = static_cast<const Pattern_Variable*>(source);
		return pattern_variable->copy_matched_term();
	}
	}
	throw XTermConstructionError("function copy_subterm expected known type to copy");
}

int bmath::intern::operator_precedence(Type operator_type)
{
	switch (operator_type) {
	case Type::par_operator:		return 1;	//lower order because, it already brings its own parentheses.
	case Type::sum:					return 2;
	case Type::value:				return 3;	//complex number in carthesian coordinates itself is sum -> we don't want parenthesis around a sum of complex numbers
	case Type::variable:			return 3;	
	case Type::product:				return 4;
	case Type::exponentiation:		return 5;
	case Type::pattern_variable:	return 0;	//brings its own curly braces in to_str() -> lowest precedence
	default:						return 0;
	}
}

// used to create lines of tree output
constexpr static signed char LINE_UP_DOWN = -77;		//(179)
constexpr static signed char LINE_UP_RIGHT = -64;		//(192)
constexpr static signed char LINE_UP_RIGHT_DOWN = -61;	//(195)
constexpr static signed char LINE_LEFT_RIGHT = -60;		//(196)

std::string bmath::intern::ptr_to_tree(const Basic_Term* term_ptr, std::size_t offset)
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

void bmath::intern::append_last_line(std::vector<std::string>& tree_lines, char operation)
{
	std::size_t column = tree_lines.back().find_first_not_of(' ');
	if (column > 0) {
		column -= 5;
		tree_lines.back()[column] = LINE_UP_RIGHT;						// adding angle char
		if (operation != '\0') {
			tree_lines.back()[column + 1] = '{';
			tree_lines.back()[column + 2] = operation;
			tree_lines.back()[column + 3] = '}';
			tree_lines.back()[column + 4] = LINE_LEFT_RIGHT;
		}
		else {
			tree_lines.back().replace(column + 1, 4, 4, LINE_LEFT_RIGHT);
		}

		std::size_t line = tree_lines.size() - 2;
		while (tree_lines[line][column] == ' ') {						// adding bars to connect angle with tree above
			tree_lines[line][column] = LINE_UP_DOWN;
			line--;
		}
		if (tree_lines[line][column] == LINE_UP_RIGHT) {
			tree_lines[line][column] = LINE_UP_RIGHT_DOWN;				//connecting bars to previous angle char
		}
	}
}

void bmath::intern::reset_all_pattern_vars(std::list<Pattern_Variable*>& var_adresses)
{
	for (auto pattern_var : var_adresses) {
		pattern_var->matched_term = nullptr;
	}
}

std::list<Basic_Term*>::iterator bmath::intern::find_first_of(std::list<Basic_Term*>& search, Type type)
{
	for (auto it = search.begin(); it != search.end(); ++it) {
		if (type_of(*it) == type) {
			return it;
		}
	}
	return search.end();
}

bool bmath::intern::is_natural(std::complex<double> test)
{
	const double real = test.real();
	return (real >= 0) && (real - static_cast<int>(real) == 0) && (test.imag() == 0);
}

namespace bmath::intern {

	// returns, whether the difference of first and second are within the allowed difference
	//(helper function for to_string())
	bool about_equal(const double first, const double second, const double allowed_difference = 0.00000000000001)
	{
		return std::abs(first - second) <= allowed_difference;
	}

	//adds the imaginary part of a complex number (im) to buffer
	//(helper function for to_string())
	void add_im_to_stream(std::stringstream& buffer, const double im, bool showpos = false)
	{
		if (about_equal(im, -1.0)) {
			buffer << '-';
		}
		else if (about_equal(im, 1.0)) {
			if (showpos) {
				buffer << '+';
			}
		}
		else {
			buffer << (showpos ? std::showpos : std::noshowpos) << im;
		}
		buffer << 'i';
	}
}

std::string bmath::intern::to_string(std::complex<double> val, int parent_operator_precedence, bool inverse)
{
	const double re = inverse ? -(val.real()) : val.real();
	const double im = inverse ? -(val.imag()) : val.imag();
	bool parentheses = false;
	std::stringstream buffer;

	if (re != 0 && im != 0) {
		parentheses = parent_operator_precedence > operator_precedence(Type::value);
		buffer << re;
		add_im_to_stream(buffer, im, true);		
	}
	else if (re != 0 && im == 0) {
		parentheses = re < 0 && parent_operator_precedence > operator_precedence(Type::value);	//leading '-'
		buffer << re;
	}
	else if (re == 0 && im != 0) {
		parentheses = im < 0 && parent_operator_precedence > operator_precedence(Type::value);	//leading '-'		
		add_im_to_stream(buffer, im);
	}
	else {
		buffer << '0';
	}

	if (parentheses) {
		return '(' + buffer.str() + ')';
	}
	else {
		return buffer.str();
	}
}

constexpr std::string_view bmath::intern::name_of(Par_Op_Type op_type)
{
	switch (op_type) {
	case Par_Op_Type::log10:	return { "log10(" };
	case Par_Op_Type::asinh:	return { "asinh(" };
	case Par_Op_Type::acosh:	return { "acosh(" };
	case Par_Op_Type::atanh:	return { "atanh(" }; 
	case Par_Op_Type::asin:		return { "asin(" };
	case Par_Op_Type::acos:		return { "acos(" };
	case Par_Op_Type::atan:		return { "atan(" };
	case Par_Op_Type::sinh:		return { "sinh(" };
	case Par_Op_Type::cosh:		return { "cosh(" };
	case Par_Op_Type::tanh:		return { "tanh(" };
	case Par_Op_Type::sqrt:		return { "sqrt(" };
	case Par_Op_Type::exp:		return { "exp(" };
	case Par_Op_Type::sin:		return { "sin(" };
	case Par_Op_Type::cos:		return { "cos(" };
	case Par_Op_Type::tan:		return { "tan(" };
	case Par_Op_Type::abs:		return { "abs(" };
	case Par_Op_Type::arg:		return { "arg(" };
	case Par_Op_Type::ln:		return { "ln(" };
	case Par_Op_Type::re:		return { "re(" };
	case Par_Op_Type::im:		return { "im(" };
	}
	assert(false);
	return {};
}

constexpr std::complex<double> bmath::intern::value_of(std::complex<double> argument, Par_Op_Type op_type)
{
	switch (op_type) {
	case Par_Op_Type::log10:	return std::log10(argument);
	case Par_Op_Type::asinh:	return std::asinh(argument);
	case Par_Op_Type::acosh:	return std::acosh(argument);
	case Par_Op_Type::atanh:	return std::atanh(argument);
	case Par_Op_Type::asin:		return std::asin(argument);
	case Par_Op_Type::acos:		return std::acos(argument);
	case Par_Op_Type::atan:		return std::atan(argument);
	case Par_Op_Type::sinh:		return std::sinh(argument);
	case Par_Op_Type::cosh:		return std::cosh(argument);
	case Par_Op_Type::tanh:		return std::tanh(argument);
	case Par_Op_Type::sqrt:		return std::sqrt(argument);
	case Par_Op_Type::exp:		return std::exp(argument);
	case Par_Op_Type::sin:		return std::sin(argument);
	case Par_Op_Type::cos:		return std::cos(argument);
	case Par_Op_Type::tan:		return std::tan(argument);
	case Par_Op_Type::abs:		return std::abs(argument);
	case Par_Op_Type::arg:		return std::arg(argument);
	case Par_Op_Type::ln:		return std::log(argument);
	case Par_Op_Type::re:		return std::real(argument);
	case Par_Op_Type::im:		return std::imag(argument);
	}
	assert(false);
	return {};
}

constexpr std::string_view bmath::intern::name_of(Math_Constant constant)
{
	switch (constant) {
	case Math_Constant::i:	return { "i" };
	case Math_Constant::e:	return { "e" };
	case Math_Constant::pi:	return { "pi" };
	}
	assert(false);
	return {};
}

constexpr std::complex<double> bmath::intern::value_of(Math_Constant constant)
{
	switch (constant) {
	case Math_Constant::i:	return { 0, 1 };
	case Math_Constant::e:	return { 2.718281828459045, 0 };
	case Math_Constant::pi:	return { 3.141592653589793, 0 };
	}
	assert(false);
	return {};
}

std::string_view bmath::intern::name_of(Type type)
{
	switch (type) {
	case Type::par_operator:		return { "par_operator" };
	case Type::exponentiation:		return { "expo" };
	case Type::sum:					return { "sum" };
	case Type::product:				return { "product" };
	case Type::variable:			return { "variable" };
	case Type::value:				return { "value" };
	case Type::pattern_variable:	return { "pattern_variable" };
	case Type::undefined:			return { "undefined" };
	}
	assert(false);
	return {};
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//stack based calculation\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool bmath::intern::is_computable(std::string_view name) 
{
	const char* const numeric_symbols = "0123456789.+-*/^()";
	std::size_t letter_pos = name.find_first_not_of(numeric_symbols);
	while (letter_pos != std::string::npos)	{

		//testing if letter is part of (or full) Math_Constant
		for (Math_Constant constant : all_math_constants) {
			const std::string_view const_name = name_of(constant);
			if (name.compare(letter_pos, const_name.length(), const_name) == 0) {
				const std::size_t next_pos = name.find_first_not_of(numeric_symbols, letter_pos + const_name.length());
				if (next_pos != letter_pos + const_name.length()) {	//if next non numeric symbol is not the next symbol -> identified letter as Math_Constant
					letter_pos = next_pos;
					goto test_next;	//forgive me father, for i have sinned.
				}
			}
		}
		//testing if letter is beginning of Par_Operator
		for (Par_Op_Type op_type : all_par_op_types) {
			const std::string_view op_name = name_of(op_type);
			if (name.compare(letter_pos, op_name.length(), op_name) == 0) {
				//we dont need another check if the next letter is not the next char, because all op_name strings end with '('.
				letter_pos = name.find_first_not_of(numeric_symbols, letter_pos + op_name.length());
				goto test_next;
			}
		}
		return false;
		test_next:;	//this letter was found to not be part of an variable -> next letter will be tested in next while iteration
	}
	return true;
}

namespace bmath::intern {

	inline std::complex<double> compute_sum(std::string_view name, std::size_t op)
	{
		const std::string_view first = name.substr(0, op);
		name.remove_prefix(op + 1);
		return compute(first) + compute(name);
	}

	inline std::complex<double> compute_sub(std::string_view name, std::size_t op)
	{
		if (op != 0) {	//testing for leading '-'
			const std::string_view first = name.substr(0, op);
			name.remove_prefix(op + 1);
			return compute(first) - compute(name);
		}
		else {
			name.remove_prefix(1);
			return - compute(name);
		}
	}

	inline std::complex<double> compute_mul(std::string_view name, std::size_t op)
	{
		const std::string_view first = name.substr(0, op);
		name.remove_prefix(op + 1);
		return compute(first) * compute(name);
	}

	inline std::complex<double> compute_div(std::string_view name, std::size_t op)
	{
		const std::string_view first = name.substr(0, op);
		name.remove_prefix(op + 1);
		return compute(first) / compute(name);
	}

	inline std::complex<double> compute_pow(std::string_view name, std::size_t op)
	{
		const std::string_view first = name.substr(0, op);
		name.remove_prefix(op + 1);
		return std::pow(compute(first), compute(name));
	}

	inline std::complex<double> compute_par_op(std::string_view name, std::size_t op, Par_Op_Type op_type)
	{
		name.remove_suffix(1);								//closing parenthesis gets cut of
		name.remove_prefix(name_of(op_type).length());	//funktionname and opening parenthesis get cut of
		return value_of(compute(name), op_type);
	}

	inline std::complex<double> compute_val(std::string_view name)
	{
		double factor = 1;	//if name only consists of the constant, from chars will not touch factor -> initialized to 1 to not change result if constant is found.
		const auto [value_end, error] = std::from_chars(name.data(), name.data() + name.size(), factor);	//value_end points to first char not read as part of number.

		const char* const name_end = name.data() + name.length();
		if (value_end != name_end) {
			const std::string_view rest(value_end, name_end - value_end);
			for (Math_Constant constant : all_math_constants) {
				if (rest == name_of(constant)) {
					return factor * value_of(constant);
				}
			}
		}
		return { factor, 0 };
	}
}

std::complex<double> bmath::intern::compute(std::string_view name)
{
	while (name.length()) {
		std::size_t 
		op = find_last_of_skip_pars(name, '+');
		if (op != std::string::npos) {
			return compute_sum(name, op);
		}
		op = find_last_of_skip_pars(name, '-');
		if (op != std::string::npos) {
			return compute_sub(name, op);
		}
		op = find_last_of_skip_pars(name, '*');
		if (op != std::string::npos) {
			return compute_mul(name, op);
		}
		op = find_last_of_skip_pars(name, '/');
		if (op != std::string::npos) {
			return compute_div(name, op);
		}
		op = find_last_of_skip_pars(name, '^');
		if (op != std::string::npos) {
			return compute_pow(name, op);
		}
		//searching for parenthesis operators 
		for (Par_Op_Type op_type : all_par_op_types) {
			const std::string_view op_name = name_of(op_type);
			if (name.compare(0, op_name.length(), op_name) == 0) {
				return compute_par_op(name, op, op_type);
			}
		}
		if (name.front() != '(' && name.back() != ')') {
			return compute_val(name);
		}
		name.remove_prefix(1);
		name.remove_suffix(1);
	}
	throw XTermConstructionError("function compute() run out of characters to find calculations");
}
