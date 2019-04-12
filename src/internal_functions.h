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
std::size_t find_closed_par(std::size_t open_par, const std::string& name);

//finds the highest level of parentheses in name
void find_pars(const std::string& name, std::vector<Pos_Pars>& pars);

//skips parentheses, else finds_first_of characters in name like std::string function
std::size_t find_last_of_skip_pars(const std::string& name, const char* characters, std::vector<Pos_Pars>& pars);

//deletes parentheses of parentheses list, which start after the end of the name
void del_pars_after(std::vector<Pos_Pars>& pars, const std::string& name);

//decides type of next subterm
State type_subterm(const std::string& name, std::vector<Pos_Pars>& pars);

//returns pointer to newly build term of right type
Basic_Term* build_subterm(std::string& subtermstr, Basic_Term* parent_);

}

//allows ostream to output terms
std::ostream& operator<<(std::ostream& stream, const bmath::Basic_Term& term);
