
#include "internal_functions.h"

#include <charconv>

using namespace bmath::intern;

std::size_t bmath::intern::find_open_par(std::size_t clsd_par, const std::string_view name) 
{
	unsigned int deeper_clsd_par = 0;
	std::size_t nxt_par = clsd_par;
	while (true) {
		nxt_par = name.find_last_of("()", nxt_par - 1);
		if (nxt_par == std::string::npos) {
			throw XTermConstructionError("function find_closed_par() expected more closed parentheses in string");
		}
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
		std::size_t found = search_view.find_last_of(characters);
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

Type bmath::intern::type_subterm(const std::string_view name, std::size_t& op, Par_Op_Type& par_op_type)
{
	//starting search for "basic" operators
	op = find_last_of_skip_pars(name, "+-");
	if (op != std::string::npos) {
		return sum;
	}
	op = find_last_of_skip_pars(name, "*/");
	if (op != std::string::npos) {
		return product;
	}
	op = find_last_of_skip_pars(name, "^");
	if (op != std::string::npos) {
		return exponentiation;
	}
	//searching for parenthesis operators 
	for (Par_Op_Type op_type : all_par_op_types) {
		const char* const op_name = par_op_name(op_type);
		if (name.compare(0, strlen(op_name), op_name) == 0) {
			par_op_type = op_type;
			return par_operator;
		}
	}
	if (name[0] == '(' && name[name.length() - 1] == ')') {	
		return undefined;
	}
	//staring search for arguments (variable or value)
	op = name.find_last_of("abcdefghjklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]_$");	//search does not search for 'i'
	if (name.find_last_of("i") != std::string::npos && op == std::string::npos) {
		return value;
	}
	if (op != std::string::npos) {
		return variable;
	}
	op = name.find_last_of("0123456789");	//number kann also contain '.', however not as only character
	if (op != std::string::npos) {
		return value;
	}
	throw XTermConstructionError("string is not of expected format");
}

void bmath::intern::preprocess_str(std::string& str)
{
	int par_diff = 0;
	for (std::size_t i = 0; i < str.length(); i++) {	//deleting whitespace and counting parentheses
		switch (str[i]) {
		case '\t':
		case '\n':
		case ' ':
			str.erase(i, 1);
			i--;
			break;
		case '(':
			par_diff++;
			break;
		case ')':
			par_diff--;
			break;
		default:
			break; //nothing to do this character
		}
	}
	if (par_diff != 0) {
		throw XTermConstructionError("the parenthesis of string do not obey the syntax rules");
	}
	const char* const allowed_chars = "1234567890.abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ+-*/^[]()_$";
	if (str.find_first_not_of(allowed_chars) != std::string::npos) {
		throw XTermConstructionError("string contains characters other than: 1234567890.abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ+-*/^[]()_$");
	}
}

Type bmath::intern::type(const Basic_Term* obj)
{
	if (obj != nullptr) {
		return obj->get_type();
	}
	else {
		return undefined;
	}
}

Basic_Term* bmath::intern::build_subterm(std::string_view subterm_view, Basic_Term* parent_)
{
	while (subterm_view.size() > 0) {	// two can (if valid) always be build and dont need further chopping of

		std::size_t op = std::string::npos;
		Par_Op_Type par_op_type;	//only used if parenthesis operator is found
		Type type = type_subterm(subterm_view, op, par_op_type);

		switch (type) {
		case exponentiation:
			return new Exponentiation(subterm_view, parent_, op);
		case product:
			return new Product(subterm_view, parent_, op);
		case sum:
			return new Sum(subterm_view, parent_, op);
		case variable:
			return new Variable(subterm_view, parent_);
		case value:
			return new Value(subterm_view, parent_);
		case par_operator:
			return new Par_Operator(subterm_view, parent_, par_op_type);
		}

		//not variable/value -> string contains parentheses, but no operation outside was found. 
		//if parentheses dont enclose all of the subterm, there is something wrong.
		if (subterm_view[0] != '(' || subterm_view[subterm_view.length() - 1] != ')' || subterm_view.size() < 2) {
			throw XTermConstructionError("could not determine operation to split string in function build_subterm()");
		}
		subterm_view.remove_prefix(1); //chopping of enclosing parentheses
		subterm_view.remove_suffix(1);
	}
	throw XTermConstructionError("could not find any type to build term(function build_subterm)");
}

Basic_Term* bmath::intern::build_pattern_subterm(std::string_view subterm_view, Basic_Term* parent_, std::list<Pattern_Variable*>& variables)
{
	while (subterm_view.size() > 0) {

		std::size_t op = std::string::npos;
		Par_Op_Type par_op_type;	//only used if parenthesis operator is found
		Type type = type_subterm(subterm_view, op, par_op_type);

		switch (type) {
		case exponentiation:
			return new Exponentiation(subterm_view, parent_, op, variables);
		case product:
			return new Product(subterm_view, parent_, op, variables);
		case sum:
			return new Sum(subterm_view, parent_, op, variables);
		case par_operator:
			return new Par_Operator(subterm_view, parent_, par_op_type, variables);
		case value:
			return new Value(subterm_view, parent_);
		case variable:
			for (auto& variable : variables) {
				if (variable->name == subterm_view) {
					return variable;
				}
			}
			Pattern_Variable* new_variable = new Pattern_Variable(subterm_view, parent_);
			variables.push_back(new_variable);
			return new_variable;
		}
		//not variable/value -> string contains parentheses, but no operation outside was found. 
		//if parentheses dont enclose all of the subterm, there is something wrong.
		if (subterm_view[0] != '(' || subterm_view[subterm_view.length() - 1] != ')' || subterm_view.size() < 2) {
			throw XTermConstructionError("could not determine operation to split string in function build_pattern_subterm()");
		}
		subterm_view.remove_prefix(1); //chopping of enclosing parentheses
		subterm_view.remove_suffix(1);
	}
	throw XTermConstructionError("could not find any type to build term (function build_pattern_subterm)");
}

Basic_Term* bmath::intern::copy_subterm(const Basic_Term* source, Basic_Term* parent_)
{
	switch (type(source)) {
	case par_operator:
		return new Par_Operator(*(static_cast<const Par_Operator*>(source)), parent_);
	case value:
		return new Value(*(static_cast<const Value*>(source)), parent_);
	case variable:
		return new Variable(*(static_cast<const Variable*>(source)), parent_);
	case sum:
		return new Sum(*(static_cast<const Sum*>(source)), parent_);
	case product:
		return new Product(*(static_cast<const Product*>(source)), parent_);
	case exponentiation:
		return new Exponentiation(*(static_cast<const Exponentiation*>(source)), parent_);
	case pattern_variable: {
		const Pattern_Variable* pattern_variable = static_cast<const Pattern_Variable*>(source);
		if (pattern_variable->pattern_value != nullptr) {
			return copy_subterm(pattern_variable->pattern_value, parent_);
		}
		else {
			throw XTermConstructionError("can not copy Pattern_Variable with pattern_value nullptr.");
		}
	}
	}
	throw XTermConstructionError("function copy_subterm expected known type to copy");
}

// used to create lines of tree output
const static signed char LINE_UP_DOWN = -77;		//(179)
const static signed char LINE_UP_RIGHT = -64;		//(192)
const static signed char LINE_UP_RIGHT_DOWN = -61;	//(195)
const static signed char LINE_LEFT_RIGHT = -60;		//(196)

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

void bmath::intern::reset_pattern_vars(std::list<Pattern_Variable*>& var_adresses)
{
	for (auto& pattern_var : var_adresses) {
		pattern_var->pattern_value = nullptr;
	}
}

const char* const bmath::intern::par_op_name(Par_Op_Type op_type)
{
	switch (op_type) {
	case log10:	return "log10(";
	case asinh:	return "asinh(";
	case acosh:	return "acosh(";
	case atanh:	return "atanh(";
	case asin:	return "asin(";
	case acos:	return "acos(";
	case atan:	return "atan(";
	case sinh:	return "sinh(";
	case cosh:	return "cosh(";
	case tanh:	return "tanh(";
	case sqrt:	return "sqrt(";
	case exp:	return "exp(";
	case sin:	return "sin(";
	case cos:	return "cos(";
	case tan:	return "tan(";
	case abs:	return "abs(";
	case arg:	return "arg(";
	case ln:	return "ln(";
	case re:	return "re(";
	case im:	return "im(";
	}
}

std::complex<double> bmath::intern::evaluate_par_op(std::complex<double> argument, Par_Op_Type op_type)
{
	switch (op_type) {
	case log10:	return std::log10(argument);
	case asinh:	return std::asinh(argument);
	case acosh:	return std::acosh(argument);
	case atanh:	return std::atanh(argument);
	case asin:	return std::asin(argument);
	case acos:	return std::acos(argument);
	case atan:	return std::atan(argument);
	case sinh:	return std::sinh(argument);
	case cosh:	return std::cosh(argument);
	case tanh:	return std::tanh(argument);
	case sqrt:	return std::sqrt(argument);
	case exp:	return std::exp(argument);
	case sin:	return std::sin(argument);
	case cos:	return std::cos(argument);
	case tan:	return std::tan(argument);
	case abs:	return std::abs(argument);
	case arg:	return std::arg(argument);
	case ln:	return std::log(argument);
	case re:	return std::real(argument);
	case im:	return std::imag(argument);
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//stack based calculation\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool bmath::intern::is_computable(std::string_view name) 
{
	const char* const numeric_symbols = "0123456789.+-*/^()";
	std::size_t letter_pos = name.find_first_not_of(numeric_symbols);
	while (letter_pos != std::string::npos)	{
		if (name[letter_pos] == 'i') { //testing if letter is only "i", followed by numeric syntax
			if (name.length() == letter_pos + 1 || name.find_first_of(numeric_symbols, letter_pos + 1) == letter_pos + 1) {
				letter_pos = name.find_first_not_of(numeric_symbols, letter_pos + 2);
				continue;
			}			
		}
		//testing if letter is beginning of Par_Operator
		bool found_op = false;
		for (Par_Op_Type op_type : all_par_op_types) {
			const char* const op_name = par_op_name(op_type);
			if (name.compare(letter_pos, strlen(op_name), op_name) == 0) {
				letter_pos = name.find_first_not_of(numeric_symbols, letter_pos + strlen(op_name));
				found_op = true;
				break;
			}
		}
		if (found_op) {
			continue;
		}
		else {
			return false;
		}
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
		if (op != 0) {
			const std::string_view first = name.substr(0, op);
			name.remove_prefix(op + 1);
			return compute(first) - compute(name);
		}
		else {
			name.remove_prefix(1);
			return -compute(name);
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
		name.remove_prefix(strlen(par_op_name(op_type)));	//funktionname and opening parenthesis get cut of
		return evaluate_par_op(compute(name), op_type);
	}

	inline std::complex<double> compute_val(std::string_view name)
	{
		if (name == "i") {
			return { 0, 1 };
		}
		else {
			double factor;
			std::from_chars(name.data(), name.data() + name.size(), factor);
			if (name.find_first_of('i') != std::string::npos) {
				return { 0, factor };
			}
			else {
				return { factor, 0 };
			}
		}
	}
}

std::complex<double> bmath::intern::compute(std::string_view name)
{
	while (name.length()) {
		std::size_t op;
		op = find_last_of_skip_pars(name, "+");
		if (op != std::string::npos) {
			return compute_sum(name, op);
		}
		op = find_last_of_skip_pars(name, "-");
		if (op != std::string::npos) {
			return compute_sub(name, op);
		}
		op = find_last_of_skip_pars(name, "*");
		if (op != std::string::npos) {
			return compute_mul(name, op);
		}
		op = find_last_of_skip_pars(name, "/");
		if (op != std::string::npos) {
			return compute_div(name, op);
		}
		op = find_last_of_skip_pars(name, "^");
		if (op != std::string::npos) {
			return compute_pow(name, op);
		}
		//searching for parenthesis operators 
		for (Par_Op_Type op_type : all_par_op_types) {
			const char* const op_name = par_op_name(op_type);
			if (name.compare(0, strlen(op_name), op_name) == 0) {
				return compute_par_op(name, op, op_type);
			}
		}
		if (name[0] != '(' && name[name.length() - 1] != ')') {
			return compute_val(name);
		}
		name.remove_prefix(1);
		name.remove_suffix(1);
	}
	throw XTermConstructionError("function compute() run out of characters to find calculations");
}
