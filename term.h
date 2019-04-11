#pragma once

#include <set>
#include <string>
#include <sstream>
#include <list>
#include <vector>
#include <math.h>


namespace bmath {

//contains position of parentheses in constructor string
struct Pos_Pars {
	std::size_t start;	//position of '('
	std::size_t end;	//position of ')'
};

//specifies whether the set, the double or the Variable contains the therm
enum State {
	product,
	sum,
	val,
	var,
	exponentiation
};

//extra functions:

//finds matching closed parethesis to the open_par in name (used in find_pars)
std::size_t find_closed_par(std::size_t open_par, std::string& name);

//finds the highest level of parentheses in name
void find_pars(std::string& name, std::vector<Pos_Pars>& pos_pars);

//skips parentheses, else finds_first_of characters in name like std::string function
std::size_t find_last_of_skip_pars(std::string& name, const char* characters, std::vector<Pos_Pars>& pars);

//deletes parentheses of parentheses list, which start after the end of the name
void del_pars_after(std::vector<Pos_Pars> pos_pars, std::string& name);

//used in constructor to split subterm of name
void cut_subterm_from_name(std::string& name, std::string& subterm_str, std::vector<Pos_Pars>& pos_pars, std::size_t op);

//deides type of next subterm
State type_subterm(std::string& name, std::list<Pos_Pars>& pars);

//main class:

class Basic_Term 
{
protected:
	Basic_Term* parent;

public:
	Basic_Term(std::string name);
	virtual ~Basic_Term();
};

class Product : public Therm
{
private:
	std::list<Basic_Term*> factors;
public:
};

class Sum : public Therm
{
private:
	std::list<Basic_Term*> summands;
public:
};

class Variable : public Therm
{
private:
	std::string name;
	bool negative;
	double exponent;
	double factor;
public:
};

class Value : public Therm
{
private:
	double value;
public:
};

//used in constructor to do the constructing
Basic_Term* cut_subterm(std::string& name);

}//namespace bruno

std::ostream& operator<<(std::ostream& stream, const bmath::Basic_Term& term);


