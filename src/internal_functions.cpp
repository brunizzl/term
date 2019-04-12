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

std::size_t bmath::find_last_of_skip_pars(const std::string& name, const char* characters, std::vector<Pos_Pars>& pars)
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

void bmath::del_pars_after(std::vector<Pos_Pars>& pars, const std::string& name)
{
	int new_end = pars.size();
	for (int i = 0; i < pars.size(); i++) {
		if (pars[i].start > name.size() - 1) {
			new_end = i;
			break;
		}
	}
	pars.resize(new_end);
	return;
}

State bmath::type_subterm(const std::string & name, std::vector<Pos_Pars>& pars)
{
	std::size_t op;
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
	if (pars.size() != 0) {
		return other;
	}
	op = name.find_last_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]_$");
	if (op != std::string::npos) {
		return var;
	}
	op = name.find_last_of("0123456789");
	if (op != std::string::npos) {
		return val;
	}
	std::cout << "error: string " << name << " is not of expected format.\n";
	return other;
}

Basic_Term* bmath::build_subterm(std::string& subtermstr, Basic_Term* parent_)
{
	std::vector<Pos_Pars> pars;
	while (subtermstr.size() != 0) {
		find_pars(subtermstr, pars);
		State state = type_subterm(subtermstr, pars);
		switch (state) {
		case exponentiation:
			return new Exponentiation(subtermstr, parent_);
		case product:
			return new Product(subtermstr, parent_);
		case sum:
			return new Sum(subtermstr, parent_);
		case var:
			return new Variable(subtermstr, parent_);
		case val:
			return new Value(subtermstr, parent_);
		}
		subtermstr.pop_back();
		subtermstr.erase(0, 1);
		pars.clear();
	}
	std::cout << "Error: could not find any type to build term (function build_subterm)\n";
	return nullptr;
}



std::ostream& operator<<(std::ostream& stream, const Basic_Term& term)
{
	//stream << term.to_str();
	return stream;
}