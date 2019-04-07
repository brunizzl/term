#include "term.h"

#include <iostream>

using namespace bruno;

std::size_t bruno::find_closed_par(std::size_t open_par, std::string& name) 
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

void bruno::find_pars(std::string & name, std::vector<Pos_Pars>& pos_pars)
{
	std::size_t open_par = name.find_first_of('(');
	while (open_par != std::string::npos) {
		std::size_t clsd_par = find_closed_par(open_par, name);
		if (clsd_par == std::string::npos) {
			std::cout << "Error: expected more closed parentheses in \"" << name << "\".\n";
		}
		else {
			pos_pars.push_back({ open_par, clsd_par });
		}
		open_par = name.find_first_of('(', clsd_par + 1);
	}
	return;
}

std::size_t bruno::find_last_of_skip_pars(std::string& name, const char* characters, std::vector<Pos_Pars>& pars) 
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

void bruno::del_pars_after(std::vector<Pos_Pars> pos_pars, std::string& name)
{
	int new_end = pos_pars.size();
	for (int i = 0; i < pos_pars.size(); i++) {
		if (pos_pars[i].start > name.size() - 1) {
			new_end = i;
			break;
		}
	}
	pos_pars.resize(new_end);
	return;
}

void bruno::cut_subterm_from_name(std::string & name, std::string & subterm_str, std::vector<Pos_Pars>& pos_pars, std::size_t op)
{
	if (!pos_pars.empty()) {
		if (pos_pars.back().start == op + 1 && pos_pars.back().end == name.length() - 1) {
			//only parentheses after operator
			subterm_str = name.substr(pos_pars.back().start, pos_pars.back().end - pos_pars.back().start + 1);
			name.erase(op);
			pos_pars.pop_back();
		}
		else if (pos_pars.back().start == op + 1 && name.find_first_of("+-*/", pos_pars.back().end) == std::string::npos) {
			//only parentheses and ^ after operator
			subterm_str = name.substr(pos_pars.back().start);
			name.erase(op);
		}
		else {	//not one pair of parentheses (more):
			subterm_str = name.substr(op + 1);
			name.erase(op);
		}
	}
	else {	//not one pair of parentheses (none):
		subterm_str = name.substr(op + 1);
		name.erase(op);
	}
	del_pars_after(pos_pars, name);
	return;
}


Term::Term(std::string name, Term* parent, bool negative, double exponent)
	:parent(parent), exponent(exponent), negative(negative)
{
	std::cout << "constructing term   minus:" << negative << " exponent: " << exponent << " name: " << name << '\n';
	while (true) {
		//finding the "highest layer" of parentheses
		std::vector<Pos_Pars> pos_pars;
		find_pars(name, pos_pars);

		//splitting name at operator (^)
		std::size_t op = find_last_of_skip_pars(name, "^", pos_pars);
		if (find_last_of_skip_pars(name, "+-*/", pos_pars) == std::string::npos && op != std::string::npos) {
			if (name.find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", op) != std::string::npos) {
				std::cout << "Error: can not have variables or terms as exponent :(\n";
			}
			else {
				std::stringstream stream;
				stream << name.substr(op + (name[op + 1] == '(' ? 2 : 1));
				double new_exponent;
				stream >> new_exponent;
				this->exponent *= new_exponent;
			}
			name.resize(op);
			if (name[0] == '(' && name.back() == ')') {
				name.erase(name.size() - 1);
				name.erase(0, 1);
			}
			pos_pars.clear();
			find_pars(name, pos_pars);
			//no return
		}

		//splitting name at operators (+-)
		op = find_last_of_skip_pars(name, "+-", pos_pars);
		if (op != std::string::npos) {
			this->state = list_sum;
			//split in subterms
			while (op != std::string::npos) {
				std::string subterm_str;
				bool op_minus = name[op] == '-';
				cut_subterm_from_name(name, subterm_str, pos_pars, op);
				//call new constructor:
				this->subterms.push_back(new Term(subterm_str, this, op_minus, 1));
				op = find_last_of_skip_pars(name, "+-", pos_pars);
			}
			if (name.size() > 0) {
				this->subterms.push_back(new Term(name, this, false, 1));
			}
			this->subterms.reverse();
			return;
		}

		//splitting name at operators (*/)
		op = find_last_of_skip_pars(name, "*/", pos_pars);
		if (op != std::string::npos) {
			this->state = list_product;
			//split in subterms
			while (op != std::string::npos) {
				std::string subterm_str;
				bool op_divisor = name[op] == '/';
				cut_subterm_from_name(name, subterm_str, pos_pars, op);
				//call new constructor:
				this->subterms.push_back(new Term(subterm_str, this, false, op_divisor ? -1 : 1));
				op = find_last_of_skip_pars(name, "*/", pos_pars);
			}
			this->subterms.push_back(new Term(name, this, false, 1));
			this->subterms.reverse();
			return;
		}

		//store value / var	
		if (find_last_of_skip_pars(name, "1234567890", pos_pars) != std::string::npos) {
			double buffer_exponent = exponent;
			this->state = val;
			std::stringstream stream;
			stream << name;
			stream >> this->value;
			this->value = pow(this->value, buffer_exponent);
			if (this->negative) {
				this->value *= -1;
				this->negative = false;
			}
			return;
		}
		else if (find_last_of_skip_pars(name, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", pos_pars) != std::string::npos) {
			this->state = var;
			this->var_name = name;
			return;
		}
		else {
			name.pop_back();
			name.erase(0, 1);
		}
	}
}

void bruno::Term::to_str_intern(std::string& buffer, bool first_subterm_of_parent) const
{
	bool parentheses = ((this->state == list_sum || exponent != 1) && this->subterms.size() > 1) && this->parent != nullptr;
	//((state != val && exponent != 1) || (this->subterms.size() > 1 && this->state == list_sum));		
	if (parentheses) {
		buffer.push_back('(');
	}

	bool first_subterm = true;
	switch (this->state) {	
	case list_product:
		for (auto it : this->subterms) {
			if (!first_subterm) {
				if (it->exponent == -1) {
					buffer.push_back('/');
				}
				else {
					buffer.push_back('*');
				}
			}
			it->to_str_intern(buffer, first_subterm);
			first_subterm = false;
		}
		break;

	case list_sum:
		for (auto it : this->subterms) {
			if (it->state != val) {
				switch (it->negative) {
				case true:
					buffer.push_back('-');
					break;
				case false:
					if (!first_subterm) {
						buffer.push_back('+');
					}
					break;
				}
			}
			it->to_str_intern(buffer, first_subterm);
			first_subterm = false;
		}
		break;

	case var:
		buffer.append(var_name);
		break;

	case val:
		std::stringstream stream;
		stream << value;
		buffer.append(stream.str());
		break;
	}

	if (parentheses) {
		buffer.push_back(')');
		if (this->exponent != 1 && (this->exponent != -1 || first_subterm_of_parent)) {
			buffer.push_back('^');
			if (exponent < 0) {
				buffer.push_back('(');
			}
			std::stringstream stream;
			stream << exponent;
			buffer.append(stream.str());
			if (exponent < 0) {
				buffer.push_back(')');
			}
		}
	}
}

void bruno::Term::simplify()
{
	//if this and term in this->subterms have same state: add subterms of subterm to this->subterms directly
	switch (this->state)
	{
	case list_product:
	case list_sum:
		for (auto it : this->subterms) {

		}
	}
}

std::string& bruno::Term::to_str() const
{
	std::string* str = new std::string;
	this->to_str_intern(*str, true);
	return *str;
}

bruno::Term::Term(std::string name)
	:Term(name, nullptr, false, 1)
{
}

Term::~Term() {
	std::cout << "destructor of: " << *this << '\n';
	for (auto it : this->subterms) {
		delete it;
	}
}

std::ostream& operator<<(std::ostream& stream, const Term& term)
{
	stream << term.to_str();
	return stream;
}
