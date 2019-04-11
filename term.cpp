#include "term.h"

#include <iostream>

using namespace bmath;

std::size_t bmath::find_closed_par(std::size_t open_par, std::string& name) 
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

void bmath::find_pars(std::string & name, std::vector<Pos_Pars>& pos_pars)
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

std::size_t bmath::find_last_of_skip_pars(std::string& name, const char* characters, std::vector<Pos_Pars>& pars) 
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

void bmath::del_pars_after(std::vector<Pos_Pars> pos_pars, std::string& name)
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

void bmath::cut_subterm_from_name(std::string & name, std::string & subterm_str, std::vector<Pos_Pars>& pos_pars, std::size_t op)
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

State bmath::type_subterm(std::string & name, std::list<Pos_Pars>& pars)
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
	//var und val
}

Basic_Term * bmath::cut_subterm(std::string & name)
{
	//letzten subterm abschneiden, in neuen constructor uebergeben
	//(quasi, was vorher konstruktor war)
	//wenn nix gefunden -> gebe nullptr zurueck
}

std::ostream& operator<<(std::ostream& stream, const Basic_Term& term)
{
	stream << term.to_str();
	return stream;
}

bmath::Basic_Term::Basic_Term(std::string name)
{
	
}

bmath::Basic_Term::~Basic_Term()
{
	//no dynamic allocation in base
}
