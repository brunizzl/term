#pragma once

#include <string>

namespace bmath {

//contains position of parentheses in constructor string
struct Pos_Pars {
	std::size_t start;	//position of '('
	std::size_t end;	//position of ')'
};

//specifies actual type of Basic_Term (types ordered for parentheses to allow > / < / == etc. operators)
enum State {
	other,
	val,
	var,
	sum,
	product,
	exponentiation
};

}