#pragma once

#include "term.h"

namespace bmath {

//contains position of parentheses in constructor string
struct Pos_Pars {
	std::size_t start;	//position of '('
	std::size_t end;	//position of ')'
};

//specifies actual type of Basic_Term
enum State {
	product,
	sum,
	val,
	var,
	exponentiation,
	other
};


//finds matching closed parethesis to the open_par in name (used in find_pars)
std::size_t find_closed_par(std::size_t open_par, std::string& name);

//finds the highest level of parentheses in name
void find_pars(std::string& name, std::vector<Pos_Pars>& pars);

//skips parentheses, else finds_first_of characters in name like std::string function
std::size_t find_last_of_skip_pars(std::string& name, const char* characters, std::vector<Pos_Pars>& pars);

//deletes parentheses of parentheses list, which start after the end of the name
void del_pars_after(std::vector<Pos_Pars>& pars, std::string& name);

//used in constructor to split subterm of name
void cut_subterm_from_name(std::string& name, std::string& subterm_str, std::vector<Pos_Pars>& pars, std::size_t op);

//deides type of next subterm
State type_subterm(std::string& name, std::vector<Pos_Pars>& pars);

//used in constructor to do the constructing
Basic_Term* cut_subterm(std::string& name);

}

//allows ostream to output terms
std::ostream& operator<<(std::ostream& stream, const bmath::Basic_Term& term);
