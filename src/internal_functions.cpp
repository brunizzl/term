
#include "internal_functions.h"

using namespace bmath::intern;

extern Basic_Term* nullptr_term;

std::size_t bmath::intern::find_closed_par(std::size_t open_par, const std::string_view name)
{	//par for parethesis
	int deeper_open_par = 0;
	std::size_t nxt_par = open_par;
	while (true) {
		nxt_par = name.find_first_of("()", nxt_par + 1);
		if (nxt_par == std::string::npos) {
			throw XTermConstructionError("function find_closed_par() expected more closed parentheses in string");
		}
		switch (name[nxt_par]) {
		case '(':
			deeper_open_par++;
			break;
		case ')':
			if (deeper_open_par-- == 0) {
				return nxt_par;
			}
			break;
		default:
			throw XTermConstructionError("function find_closed_par() string search error (no parentheses)");
		}
	}
}

void bmath::intern::find_exposed_parts(std::string_view name, std::vector<std::string_view>& exposed) 
{
	std::size_t clsd_par;	//location of next closed parenthesis in name
	std::size_t open_par;	//location of next open parenthesis in name

	//if name starts with parentheses, these are skipped.
	if (name.front() == '(') {
		clsd_par = find_closed_par(0, name);
		open_par = name.find_first_of('(', clsd_par + 1);	//no extra check if clsd_par was found, because we can already guarantee to have as many closed as open pars.
	}
	else {
		clsd_par = -1;
		open_par = name.find_first_of('(');
	}

	while (open_par != std::string::npos) {
		exposed.push_back({ name.data() + clsd_par + 1, open_par - clsd_par });	//view starts after clsd_par -> + 1
		clsd_par = find_closed_par(open_par, name);
		open_par = name.find_first_of('(', clsd_par + 1);
	}

	//if string ends with parentheses, these are skipped too.
	if (name.back() != ')') {
		exposed.push_back({ name.data() + clsd_par + 1, name.length() - clsd_par - 1 }); //view starts after clsd_par -> + 1, length is one longer than highest index -> -1
	}
}

std::size_t bmath::intern::find_last_of_in_views(const std::string_view name, const std::vector<std::string_view>& views, const char* const characters) 
{
	for (auto view = views.rbegin(); view != views.rend(); ++view) {
		std::size_t found = view->find_last_of(characters);
		if (found != std::string::npos) {
			return found + view->data() - name.data(); // adding distance from view's start to name's start to found
		}
	}
	return std::string::npos;
}

std::size_t bmath::intern::find_first_of_skip_pars(const std::string_view name, const char* const characters) 
{
	std::size_t clsd_par = 0;
	std::size_t open_par = name.find_first_of('(');
	while (open_par != std::string::npos) {
		std::size_t found = name.find_first_of(characters, clsd_par,open_par - clsd_par);
		if (found != std::string::npos) {
			return found;
		}
		else {
			clsd_par = find_closed_par(open_par, name);
			open_par = name.find_first_of('(', clsd_par + 1);
		}
	}
	return name.find_first_of(characters, clsd_par);
}

std::size_t bmath::intern::rfind_in_views(const std::string_view name, const std::vector<std::string_view>& views, const char* searchstr) 
{
	for (auto view = views.rbegin(); view != views.rend(); ++view) {
		std::size_t found = view->rfind(searchstr);
		if (found != std::string::npos) {
			return found + view->data() - name.data(); // adding distance from view's start to name's start to found
		}
	}
	return std::string::npos;
}

std::size_t bmath::intern::find_skip_pars(const std::string_view name, const char* const searchstr) {
	std::size_t clsd_par = 0;
	std::size_t open_par = name.find_first_of('(');
	while (open_par != std::string::npos) {
		std::size_t found = name.find(searchstr, clsd_par, open_par - clsd_par);
		if (found != std::string::npos) {
			return found;
		}
		else {
			clsd_par = find_closed_par(open_par, name);
			open_par = name.find_first_of('(', clsd_par + 1);
		}
	}
	return name.find(searchstr, clsd_par);
}

void bmath::intern::update_views(const std::string_view name, std::vector<std::string_view>& views) 
{
	const char* const name_end = name.data() + name.length();	//pointer to first bit not in string
	while (!views.empty()) {
		std::string_view& view = views.back();
		if (view.data() >= name_end) {			//view starts after end of name -> gets murdered
			views.pop_back();
		}
		else if (view.data() + view.length() > name_end) {			//view ends after end of name -> gets shortend
			view.remove_suffix(view.length() - (name_end - view.data()));	
			return;
		}
		else {		//view is completely part of name -> all views before that are too
			return;
		}
	}
}

Type bmath::intern::type_subterm(const std::string_view name, const std::vector<std::string_view>& exposed_parts, std::size_t& op, Par_Op_Type & par_op_type)
{
	//starting search for "basic" operators
	op = find_last_of_in_views(name, exposed_parts, "+-");
	if (op != std::string::npos) {
		return sum;
	}
	op = find_last_of_in_views(name, exposed_parts, "*/");
	if (op != std::string::npos) {
		return product;
	}
	op = find_last_of_in_views(name, exposed_parts, "^");
	if (op != std::string::npos) {
		return exponentiation;
	}
	//searching for parenthesis operators 
	for (Par_Op_Type op_type : all_par_op_types) {
		op = rfind_in_views(name, exposed_parts, Par_Operator::op_name(op_type));
		if (op != std::string::npos) {
			//tests if op marks only substring of longer, fauly parenthesis operator (for example "tan(" beeing substring of "arctan(" (term knows "arctan" as "atan"))
			if (name.find_last_of("abcdefghjklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]_$", op - 1) == op - 1) {
				continue;
			}
			par_op_type = op_type;
			return par_operator;
		}
	}
	if (exposed_parts.size() == 0) {	
		return undefined;
	}
	//staring search for arguments (variable or value)
	op = name.find_last_of("abcdefghjklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]_$");
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
	std::vector<std::string_view> exposed_parts;
	while (subterm_view.size() > 0) {	// two can (if valid) always be build and dont need further chopping of

		find_exposed_parts(subterm_view, exposed_parts);
		std::size_t op = std::string::npos;
		Par_Op_Type par_op_type;	//only used if parenthesis operator is found
		Type type = type_subterm(subterm_view, exposed_parts, op, par_op_type);

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
		if (exposed_parts.size() != 0 || subterm_view.size() < 2) {
			throw XTermConstructionError("could not determine operation to split string in function build_subterm()");
		}
		subterm_view.remove_prefix(1); //chopping of enclosing parentheses
		subterm_view.remove_suffix(1);
	}
	throw XTermConstructionError("could not find any type to build term(function build_subterm)");
}

Basic_Term* bmath::intern::build_pattern_subterm(std::string_view subterm_view, Basic_Term* parent_, std::list<Pattern_Variable*>& variables)
{
	std::vector<std::string_view> exposed_parts;
	while (subterm_view.size() > 0) {

		find_exposed_parts(subterm_view, exposed_parts);
		std::size_t op = std::string::npos;
		Par_Op_Type par_op_type;	//only used if parenthesis operator is found
		Type type = type_subterm(subterm_view, exposed_parts, op, par_op_type);

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
		if (exposed_parts.size() != 0 || subterm_view.size() < 2) {
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


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//stack based calculation\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool bmath::intern::computable(std::string_view name) {
	const char* const numeric_symbols = "0123456789.+-*/^()";

	std::size_t letter_pos = name.find_first_not_of(numeric_symbols);
	while (letter_pos != std::string::npos)	{
		//testing if letter is only "i", followed by numeric syntax
		if (name[letter_pos] == 'i' && name.find_first_of(numeric_symbols, letter_pos + 1) == letter_pos + 1) {
			letter_pos = name.find_first_not_of(numeric_symbols, letter_pos + 2);
			continue;
		}
		//testing if letter is beginning of Par_Operator
		bool found_op = false;
		for (Par_Op_Type op_type : all_par_op_types) {
			const char* const op_name = Par_Operator::op_name(op_type);
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