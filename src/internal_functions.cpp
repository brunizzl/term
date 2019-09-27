
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
		open_par = name.find_first_of('(', clsd_par + 1);
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

std::size_t bmath::intern::find_last_of_in_views(const std::string_view name, const std::vector<std::string_view>& views, const char* characters) {
	for (auto view = views.rbegin(); view != views.rend(); ++view) {
		std::size_t found = view->find_last_of(characters);
		if (found != std::string::npos) {
			return found + view->data() - name.data(); // adding distance from view's start to name's start to found
		}
	}
	return std::string::npos;
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

State bmath::intern::type_subterm(const std::string_view name, const std::vector<std::string_view>& exposed_parts, std::size_t& op, Par_Op_State & par_op_state)
{
	//starting search for "basic" operators
	op = find_last_of_in_views(name, exposed_parts, "+-");
	if (op != std::string::npos) {
		return s_sum;
	}
	op = find_last_of_in_views(name, exposed_parts, "*/");
	if (op != std::string::npos) {
		return s_product;
	}
	op = find_last_of_in_views(name, exposed_parts, "^");
	if (op != std::string::npos) {
		return s_exponentiation;
	}
	//searching for parenthesis operators 
	for (int op_state = 0; op_state < static_cast<int>(op_error); op_state++) {
		op = rfind_in_views(name, exposed_parts, Par_Operator::op_name(static_cast<Par_Op_State>(op_state)));
		if (op != std::string::npos) {
			//tests if op marks only substring of longer, fauly parenthesis operator (for example "tan(" beeing substring of "arctan(" (term knows "arctan" as "atan"))
			if (name.find_last_of("abcdefghjklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]_$", op - 1) == op - 1) {
				continue;
			}
			par_op_state = static_cast<Par_Op_State>(op_state);
			return s_par_operator;
		}
	}
	if (exposed_parts.size() == 0) {
		return s_undefined;
	}
	//staring search for arguments (variable or value)
	op = name.find_last_of("abcdefghjklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]_$");
	if (name.find_last_of("i") != std::string::npos && op == std::string::npos) {
		return s_value;
	}
	if (op != std::string::npos) {
		return s_variable;
	}
	op = name.find_last_of("0123456789");	//number kann also contain '.', however not as only character
	if (op != std::string::npos) {
		return s_value;
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
	const char* allowed_chars = "1234567890.abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ+-*/^[]()_$";
	if (str.find_first_not_of(allowed_chars) != std::string::npos) {
		throw XTermConstructionError("string contains characters other than: 1234567890.abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ+-*/^[]()_$");
	}
}

State bmath::intern::state(const Basic_Term* obj)
{
	if (obj != nullptr) {
		return obj->get_state();
	}
	else {
		return s_undefined;
	}
}

Basic_Term* bmath::intern::build_subterm(std::string_view subterm_view, Basic_Term* parent_)
{
	std::vector<std::string_view> exposed_parts;
	while (subterm_view.size() > 0) {	// two can (if valid) always be build and dont need further chopping of

		find_exposed_parts(subterm_view, exposed_parts);
		std::size_t op;
		Par_Op_State par_op_state;	//only used if parenthesis operator is found
		State type = type_subterm(subterm_view, exposed_parts, op, par_op_state);

		switch (type) {
		case s_exponentiation:
			return new Exponentiation(subterm_view, parent_, op);
		case s_product:
			return new Product(subterm_view, parent_, op);
		case s_sum:
			return new Sum(subterm_view, parent_, op);
		case s_variable:
			return new Variable(subterm_view, parent_);
		case s_value:
			return new Value(subterm_view, parent_);
		case s_par_operator:
			return new Par_Operator(subterm_view, parent_, par_op_state);
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
		std::size_t op;
		Par_Op_State par_op_state;	//only used if parenthesis operator is found
		State type = type_subterm(subterm_view, exposed_parts, op, par_op_state);

		switch (type) {
		case s_exponentiation:
			return new Exponentiation(subterm_view, parent_, op, variables);
		case s_product:
			return new Product(subterm_view, parent_, op, variables);
		case s_sum:
			return new Sum(subterm_view, parent_, op, variables);
		case s_par_operator:
			return new Par_Operator(subterm_view, parent_, par_op_state, variables);
		case s_value:
			return new Value(subterm_view, parent_);
		case s_variable:
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
	State type = state(source);
	switch (type) {
	case s_par_operator:
		return new Par_Operator(*(static_cast<const Par_Operator*>(source)), parent_);
	case s_value:
		return new Value(*(static_cast<const Value*>(source)), parent_);
	case s_variable:
		return new Variable(*(static_cast<const Variable*>(source)), parent_);
	case s_sum:
		return new Sum(*(static_cast<const Sum*>(source)), parent_);
	case s_product:
		return new Product(*(static_cast<const Product*>(source)), parent_);
	case s_exponentiation:
		return new Exponentiation(*(static_cast<const Exponentiation*>(source)), parent_);
	case s_pattern_variable: {
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
constexpr static signed char LINE_UP_DOWN = -77;		//(179)
constexpr static signed char LINE_UP_RIGHT = -64;		//(192)
constexpr static signed char LINE_UP_RIGHT_DOWN = -61;	//(195)
constexpr static signed char LINE_LEFT_RIGHT = -60;		//(196)

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


std::ostream& operator<<(std::ostream& stream, const bmath::Term& term)
{
	stream << term.to_str();
	return stream;
}

std::ostream& operator<<(std::ostream& stream, const bmath::intern::Basic_Term& term)
{
	std::string str;
	term.to_str(str);
	stream << str;
	return stream;
}
