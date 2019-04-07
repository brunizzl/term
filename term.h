#pragma once

#include <set>
#include <string>
#include <sstream>
#include <list>
#include <vector>
#include <math.h>


namespace bruno {

//contains position of subtherm in constructor string
struct Pos_Substr {
	std::size_t start;	//position of '('
	std::size_t end;	//position of ')'
};

//specifies whether the set, the double or the Variable contains the therm
enum State {
	set_product,
	set_sum,
	val,
	var
};

//extra functions:

//finds matching closed parethesis to the open_par in name (used in find_pars)
std::size_t find_closed_par(std::size_t open_par, std::string& name);

//finds the highest level of parentheses in name
void find_pars(std::string& name, std::vector<Pos_Substr>& pos_pars);

//skips parentheses, else finds_first_of characters in name like std::string function
std::size_t find_last_of_skip_pars(std::string& name, const char* characters, std::vector<Pos_Substr>& pars);

//deletes parentheses of parentheses list, which start after the end of the name
void del_pars_after(std::vector<Pos_Substr> pos_pars, std::string& name);

//used in constructor to split subterm of name
void cut_subterm_from_name(std::string& name, std::string& subterm_str, std::vector<Pos_Substr>& pos_pars, std::size_t op);

//main class:

class Term {
private:
	State state;
	Term* parent;

    std::list<Term*> subterms;   //contains summands or factors
	union {
		double value;		//used if state == val
		double exponent;	//used if state == set or var
	};
	std::string var_name;
	bool negative;			//used as sign of var
	Term(std::string name, Term* parent, bool negative, double pow);	//parent is one layer above (nullptr is highest layer)
	void to_str_intern(std::string& buffer, bool first_subterm_in_parent) const;

public:
	std::string& to_str(std::string& str) const;
	Term(std::string name);
	~Term();
};

}//namespace bruno

std::ostream& operator<<(std::ostream& stream, const bruno::Term& term);