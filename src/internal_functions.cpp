#include "internal_functions.h"

using namespace bmath::intern;

std::size_t bmath::intern::find_closed_par(std::size_t open_par, const std::string& name)
{	//par for parethesis
	int deeper_open_par = 0;
	std::size_t nxt_par = open_par;
	while (true) {
		nxt_par = name.find_first_of("()", nxt_par + 1);
		if (nxt_par == std::string::npos) {
			std::cout << "Error: function find_closed_par() expected more closed parentheses in \"" << name << "\".\n";
			return 0;
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
			std::cout << "Error: function find_closed_par() string search error (no parentheses)\n";
			return 0;
		}
	}
}

void bmath::intern::find_pars(const std::string & name, std::vector<Pos_Pars>& pars)
{
	std::size_t open_par = name.find_first_of('(');
	while (open_par != std::string::npos) {
		std::size_t clsd_par = find_closed_par(open_par, name);
		if (clsd_par == std::string::npos) {
			std::cout << "Error: expected more closed parentheses in \"" << name << "\".\n";
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
	for (int op_state = 0; op_state < static_cast<int>(error); op_state++) {
		op = rfind_skip_pars(name, op_name(static_cast<Par_Op_State>(op_state)), pars);
		if (op != std::string::npos) {
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
	std::cout << "error: string " << name << " is not of expected format.\n";
	return s_undefined;
}

const char* bmath::intern::op_name(Par_Op_State op_state)
{
	switch (op_state) {
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
	case ln:
		return "ln(";
	}
	return nullptr;
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
		std::cout << "Error: the parenthesis of string \"" << str << "\" do not obey the syntax rules.\n";
		return false;
	}
	const char* allowed_chars = "1234567890.abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ+-*/^[]()_$";
	if (str.find_first_not_of(allowed_chars) != std::string::npos) {
		std::cout << "Error: String \"" << str << "\" contains characters other than: \n";
		std::cout << allowed_chars << '\n';
		return false;
	}
	return true;
}

State bmath::intern::get_state(const Basic_Term* obj)
{
	if (obj != nullptr) {
		return obj->get_state_intern();
	}
	else {
		return s_undefined;
	}
}

Basic_Term* bmath::intern::standardize_structure_extern(Basic_Term* obj)
{
	//obj->standardize_structure_intern();
	switch (obj->get_state_intern()) {
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
	return nullptr;
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
		}
		subtermstr.pop_back();
		subtermstr.erase(0, 1);
		pars.clear();
		//LOG_C("shortened name to: " << subtermstr << " in build_subterm");
	}
	std::cout << "Error: could not find any type to build term (function build_subterm)\n";
	return nullptr;
}

Basic_Term* bmath::intern::copy_subterm(const Basic_Term* source, Basic_Term* parent_)
{
	State type = get_state(source);
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
	}
	std::cout << "Error: function copy_subterm expected known type to copy: " << type << '\n';
	return nullptr;
}



std::ostream& operator<<(std::ostream& stream, const bmath::Term& term)
{
	std::string str;
	term.to_str(str);
	stream << str;
	return stream;
}
