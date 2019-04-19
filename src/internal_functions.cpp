#include "internal_functions.h"

using namespace bmath;

std::size_t bmath::find_closed_par(std::size_t open_par, const std::string& name)
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

void bmath::find_pars(const std::string & name, std::vector<Pos_Pars>& pars)
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

std::size_t bmath::find_last_of_skip_pars(const std::string& name, const char* characters, const std::vector<Pos_Pars>& pars)
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

std::size_t bmath::rfind_skip_pars(const std::string& name, const char* searchstr, const std::vector<Pos_Pars>& pars)
{
	std::size_t found = name.size() - 1;
	int skipped_pars = pars.size() - 1;
	bool found_valid;
	do {
		found = name.rfind(searchstr, found - 1);
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

void bmath::del_pars_after(std::vector<Pos_Pars>& pars, const std::string& name)
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

State bmath::type_subterm(const std::string & name, const std::vector<Pos_Pars>& pars, std::size_t& op, Par_Op_State & par_op_state)
{
	//starting search for "basic" operators
	op = find_last_of_skip_pars(name, "+-", pars);
	if (op != std::string::npos) {
		return sum;
	}
	op = find_last_of_skip_pars(name, "*/", pars);
	if (op != std::string::npos) {
		return product;
	}
	op = find_last_of_skip_pars(name, "^", pars);
	if (op != std::string::npos) {
		return exponentiation;
	}

	//starting search for parenthesis operators 
	//longest to shortest -> no problems with operators beeing substrs of each other
	op = rfind_skip_pars(name, "log10(", pars);
	if (op != std::string::npos) {
		par_op_state = log10;
		return par_op;
	}
	op = rfind_skip_pars(name, "gamma(", pars);
	if (op != std::string::npos) {
		par_op_state = gamma;
		return par_op;
	}
	op = rfind_skip_pars(name, "log2(", pars);
	if (op != std::string::npos) {
		par_op_state = log2;
		return par_op;
	}
	op = rfind_skip_pars(name, "asin(", pars);
	if (op != std::string::npos) {
		par_op_state = asin;
		return par_op;
	}
	op = rfind_skip_pars(name, "acos(", pars);
	if (op != std::string::npos) {
		par_op_state = acos;
		return par_op;
	}
	op = rfind_skip_pars(name, "atan(", pars);
	if (op != std::string::npos) {
		par_op_state = atan;
		return par_op;
	}
	op = rfind_skip_pars(name, "sinh(", pars);
	if (op != std::string::npos) {
		par_op_state = sinh;
		return par_op;
	}
	op = rfind_skip_pars(name, "cosh(", pars);
	if (op != std::string::npos) {
		par_op_state = cosh;
		return par_op;
	}
	op = rfind_skip_pars(name, "tanh(", pars);
	if (op != std::string::npos) {
		par_op_state = tanh;
		return par_op;
	}
	op = rfind_skip_pars(name, "exp(", pars);
	if (op != std::string::npos) {
		par_op_state = exp;
		return par_op;
	}
	op = rfind_skip_pars(name, "sin(", pars);
	if (op != std::string::npos) {
		par_op_state = sin;
		return par_op;
	}
	op = rfind_skip_pars(name, "cos(", pars);
	if (op != std::string::npos) {
		par_op_state = cos;
		return par_op;
	}
	op = rfind_skip_pars(name, "tan(", pars);
	if (op != std::string::npos) {
		par_op_state = tan;
		return par_op;
	}
	op = rfind_skip_pars(name, "ln(", pars);
	if (op != std::string::npos) {
		par_op_state = ln;
		return par_op;
	}

	if (pars.size() != 0) {
		return undefined;
	}

	//staring search for arguments (variable or value)
	op = name.find_last_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]_$");
	if (op != std::string::npos) {
		return var;
	}
	op = name.find_last_of("0123456789");	//number kann also contain '.', however not as only character
	if (op != std::string::npos) {
		return val;
	}
	std::cout << "error: string " << name << " is not of expected format.\n";
	return undefined;
}

Basic_Term* bmath::build_subterm(std::string& subtermstr, Basic_Term* parent_)
{
	std::vector<Pos_Pars> pars;
	while (subtermstr.size() != 0) {

		find_pars(subtermstr, pars);
		std::size_t op;
		Par_Op_State par_op_state;	//only used if parenthesis operator is found
		State type = type_subterm(subtermstr, pars, op, par_op_state);

		switch (type) {
		case exponentiation:
			return new Exponentiation(subtermstr, parent_, op);
		case product:
			return new Product(subtermstr, parent_, op);
		case sum:
			return new Sum(subtermstr, parent_, op);
		case var:
			return new Variable(subtermstr, parent_);
		case val:
			return new Value(subtermstr, parent_);
		case par_op:
			return new Par_Operator(subtermstr, parent_, par_op_state);
		}
		subtermstr.pop_back();
		subtermstr.erase(0, 1);
		pars.clear();
		LOG_C("shortened name to: " << subtermstr << " in build_subterm");
	}
	std::cout << "Error: could not find any type to build term (function build_subterm)\n";
	return nullptr;
}

Basic_Term* bmath::copy_subterm(const Basic_Term* source, Basic_Term* parent_)
{
	State type = source->get_state();
	switch (type) {
	case par_op:
		return new Par_Operator(*(static_cast<const Par_Operator*>(source)), parent_);
	case val:
		return new Value(*(static_cast<const Value*>(source)), parent_);
	case var:
		return new Variable(*(static_cast<const Variable*>(source)), parent_);
	case sum:
		return new Sum(*(static_cast<const Sum*>(source)), parent_);
	case product:
		return new Product(*(static_cast<const Product*>(source)), parent_);
	case exponentiation:
		return new Exponentiation(*(static_cast<const Exponentiation*>(source)), parent_);
	}
	std::cout << "Error: function copy_subterm expected known type to copy: " << type << '\n';
	return nullptr;
}



std::ostream& operator<<(std::ostream& stream, const Basic_Term& term)
{
	std::string str;
	term.to_str(str);
	stream << str;
	return stream;
}
