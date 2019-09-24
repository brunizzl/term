#include "internal_functions.h"

using namespace bmath::intern;

extern Basic_Term* nullptr_term;

std::size_t bmath::intern::find_closed_par(std::size_t open_par, const std::string& name)
{	//par for parethesis
	int deeper_open_par = 0;
	std::size_t nxt_par = open_par;
	while (true) {
		nxt_par = name.find_first_of("()", nxt_par + 1);
		if (nxt_par == std::string::npos) {
			throw XTermConstructionError("function find_closed_par() expected more closed parentheses in \"" + name + "\".");
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

void bmath::intern::find_pars(const std::string & name, std::vector<Pos_Pars>& pars)
{
	std::size_t open_par = name.find_first_of('(');
	while (open_par != std::string::npos) {
		std::size_t clsd_par = find_closed_par(open_par, name);
		if (clsd_par == std::string::npos) {
			throw XTermConstructionError("expected more closed parentheses in \"" + name + "\".");
		}
		else {
			pars.push_back({ open_par, clsd_par });
		}
		open_par = name.find_first_of('(', clsd_par + 1);
	}
	return;
}

std::size_t bmath::intern::find_last_of_skip_pars(const std::string& name, const char* characters, const std::vector<Pos_Pars>& pars)
{
	std::size_t found = name.size() - 1;
	int skipped_pars = pars.size() - 1;
	bool found_valid;
	do {
		found = name.find_last_of(characters, found - 1);
		found_valid = true;
		for (int it = skipped_pars; it >= 0; it--) {
			if (pars[it].start < found && pars[it].end > found) {
				skipped_pars = it;
				found_valid = false;
				break;
			}
		}
	} while (!found_valid && found != std::string::npos);
	return found;
}

std::size_t bmath::intern::rfind_skip_pars(const std::string& name, const char* searchstr, const std::vector<Pos_Pars>& pars)
{
	std::size_t found = name.size() - 1;
	int skipped_pars = pars.size() - 1;
	bool found_valid;
	do {
		found = name.rfind(searchstr, found - 1);
		if (found == std::string::npos) {
			return std::string::npos;
		}
		found_valid = true;
		for (int it = skipped_pars; it >= 0; it--) {
			if (pars[it].start < found && pars[it].end > found) {
				skipped_pars = it;
				found_valid = false;
				break;
			}
		}
	} while (!found_valid);
	return found;
}

void bmath::intern::del_pars_after(std::vector<Pos_Pars>& pars, const std::string& name)
{
	int new_end = pars.size();
	for (unsigned int i = 0; i < pars.size(); i++) {
		if (pars[i].start > name.size() - 1) {
			new_end = i;
			break;
		}
	}
	pars.resize(new_end);
	return;
}

State bmath::intern::type_subterm(const std::string & name, const std::vector<Pos_Pars>& pars, std::size_t& op, Par_Op_State & par_op_state)
{
	//starting search for "basic" operators
	op = find_last_of_skip_pars(name, "+-", pars);
	if (op != std::string::npos) {
		return s_sum;
	}
	op = find_last_of_skip_pars(name, "*/", pars);
	if (op != std::string::npos) {
		return s_product;
	}
	op = find_last_of_skip_pars(name, "^", pars);
	if (op != std::string::npos) {
		return s_exponentiation;
	}
	//searching for parenthesis operators 
	for (int op_state = 0; op_state < static_cast<int>(op_error); op_state++) {
		op = rfind_skip_pars(name, Par_Operator::op_name(static_cast<Par_Op_State>(op_state)), pars);
		if (op != std::string::npos) {
			//tests if op marks only substring of longer, fauly parenthesis operator (for example "tan(" beeing substring of "arctan(" (term knows "arctan" as "atan"))
			if (name.find_last_of("abcdefghjklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]_$", op - 1) == op - 1) {
				continue;
			}
			par_op_state = static_cast<Par_Op_State>(op_state);
			return s_par_operator;
		}
	}
	if (pars.size() != 0) {
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
	throw XTermConstructionError("string " + name + " is not of expected format.");
}

bool bmath::intern::preprocess_str(std::string& str)
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
		throw XTermConstructionError("the parenthesis of string \"" + str + "\" do not obey the syntax rules.");
	}
	const char* allowed_chars = "1234567890.abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ+-*/^[]()_$";
	if (str.find_first_not_of(allowed_chars) != std::string::npos) {
		throw XTermConstructionError("String \"" + str + "\" contains characters other than: " + allowed_chars);
	}
	return true;
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

Basic_Term* bmath::intern::standardize_structure(Basic_Term* obj)
{
	switch (obj->get_state()) {
	case s_product: {
		Product* product = static_cast<Product*>(obj);
		//this access to front in product.factors already assumes, if product contains a value,
		// it is the only value in factors and divisors and placed at first position.
		//aka. it is assumed combine_values() did already run.
		if (!product->factors.empty() && product->factors.front()->re_smaller_than_0()) {
			Sum* new_sum = new Sum(product->parent);
			new_sum->subtractors.push_back(product);
			product->parent = new_sum;
			return new_sum;
		}
		break;
	}
	case s_exponentiation: {
		Exponentiation* exponentiation = static_cast<Exponentiation*>(obj);
		if (exponentiation->exponent->re_smaller_than_0()) {
			Product* new_product = new Product(exponentiation->parent);
			new_product->divisors.push_back(exponentiation);
			exponentiation->parent = new_product;
			return new_product;
		}
		break;
	}
	}
	return obj;
}

Basic_Term* bmath::intern::build_subterm(std::string& subtermstr, Basic_Term* parent_)
{
	std::vector<Pos_Pars> pars;
	while (subtermstr.size() != 0) {

		find_pars(subtermstr, pars);
		std::size_t op;
		Par_Op_State par_op_state;	//only used if parenthesis operator is found
		State type = type_subterm(subtermstr, pars, op, par_op_state);

		switch (type) {
		case s_exponentiation:
			return new Exponentiation(subtermstr, parent_, op);
		case s_product:
			return new Product(subtermstr, parent_, op);
		case s_sum:
			return new Sum(subtermstr, parent_, op);
		case s_variable:
			return new Variable(subtermstr, parent_);
		case s_value:
			return new Value(subtermstr, parent_);
		case s_par_operator:
			return new Par_Operator(subtermstr, parent_, par_op_state);
		case s_undefined:
			//not variable/value -> string contains parentheses, but no operation outside was found. 
			//if parentheses dont enclose all of the subterm, there is something wrong.
			if (pars.front().start != 0 || pars.front().end != subtermstr.size() - 1) {
				throw XTermConstructionError("could not determine operation in string \"" + subtermstr + "\"  (function build_subterm)");
			}			
		}
		subtermstr.pop_back();
		subtermstr.erase(0, 1);
		pars.clear();
	}
	throw XTermConstructionError("could not find any type to build term (function build_subterm)");
}

Basic_Term* bmath::intern::build_pattern_subterm(std::string& subtermstr, Basic_Term* parent_, std::list<Pattern_Variable*>& variables)
{
	std::vector<Pos_Pars> pars;
	while (subtermstr.size() != 0) {

		find_pars(subtermstr, pars);
		std::size_t op;
		Par_Op_State par_op_state;	//only used if parenthesis operator is found
		State type = type_subterm(subtermstr, pars, op, par_op_state);

		switch (type) {
		case s_exponentiation:
			return new Exponentiation(subtermstr, parent_, op, variables);
		case s_product:
			return new Product(subtermstr, parent_, op, variables);
		case s_sum:
			return new Sum(subtermstr, parent_, op, variables);
		case s_par_operator:
			return new Par_Operator(subtermstr, parent_, par_op_state, variables);
		case s_value:
			return new Value(subtermstr, parent_);
		case s_variable:
			for (auto variable : variables) {
				if (variable->name == subtermstr) {
					return variable;
				}
			}
			Pattern_Variable* new_variable = new Pattern_Variable(subtermstr, parent_);
			variables.push_back(new_variable);
			return new_variable;
		}
		subtermstr.pop_back();
		subtermstr.erase(0, 1);
		pars.clear();
		//LOG_C("shortened name to: " << subtermstr << " in build_subterm");
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
	throw XTermConstructionError("function copy_subterm expected known type to copy: " + type);
}

// used to create lines of tree output
constexpr static unsigned char LINE_UP_DOWN = 179 ;
constexpr static unsigned char LINE_UP_RIGHT = 192;
constexpr static unsigned char LINE_UP_RIGHT_DOWN = 195;
constexpr static unsigned char LINE_LEFT_RIGHT = 196;

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
		static_cast<Pattern_Variable*>(pattern_var)->pattern_value = nullptr;
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
