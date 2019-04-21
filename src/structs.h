#pragma once

//#define CONSTRUCTOR_LOG
#ifdef CONSTRUCTOR_LOG
	#define LOG_C(x) std::cout << x << '\n'
#else
	#define LOG_C(x)
#endif //CONSTRUCTOR_LOG

#include <string>

namespace bmath {

//contains position of parentheses in constructor string
struct Pos_Pars {
	std::size_t start;	//position of '('
	std::size_t end;	//position of ')'
};

//return of function combine_values()
//if function was able to combine to one single value, this gets returned
//otherwise known == false and val is uninitialised.
struct Vals_Combinded {
	bool known;
	double val;
};

//specifies actual type of Basic_Term 
//(types ordered for parentheses to allow > / < / == etc. operators)
enum State {
	s_undefined,
	s_par_operator,		//already has parentheses -> lower order than in reality
	s_value,
	s_variable,
	s_sum,
	s_product,
	s_exponentiation
};

//used in Par_Operator class to specify whitch operator is actually used 
//(comments are corresponding cmath functions)
enum Par_Op_State {
	ln,			//log()
	log10,		//log10()
	log2,		//log2()
	exp,		//exp()
	sin,		//sin()
	cos,		//cos()
	tan,		//tan()
	asin,		//asin()
	acos,		//acos()
	atan,		//atan()
	sinh,		//sinh()
	cosh,		//cosh()
	tanh,		//tanh()
	gamma,		//tgamma()
	abs,		//fabs()
	sqrt,		//sqrt()
	error
};

}