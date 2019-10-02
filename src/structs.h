
#pragma once

#include <string>
#include <complex>
#include <list>

namespace bmath {
	namespace intern {

		//specifies actual type of Basic_Term 
		//(types ordered for parentheses to allow > / < / == etc. operators to test whether parentheses are needed)
		enum Type 
		{
			undefined,
			par_operator,		//already has parentheses -> lower order than in reality
			sum,
			value,
			variable,
			pattern_variable,
			product,
			exponentiation,
		};

		//used in Par_Operator class to specify whitch operator is actually used 
		//(comments are corresponding std::complex functions)
		//types names are sorted by length
		enum Par_Op_Type 
		{
			log10,			//log10()
			asinh,			//asinh()
			acosh,			//acosh()
			atanh,			//atanh()
			asin,			//asin()
			acos,			//acos()
			atan,			//atan()
			sinh,			//sinh()
			cosh,			//cosh()
			tanh,			//tanh()
			sqrt,			//sqrt()
			exp,			//exp()
			sin,			//sin()
			cos,			//cos()
			tan,			//tan()
			abs,			//abs()
			arg,			//arg()
			ln,				//log()
			re,				//real()
			im,				//imag()
		};

		//array with instance of every Par_Op_Type to ierate over them
		static const Par_Op_Type all_par_op_types[] = { log10, asinh, acosh, atanh, asin, acos, atan, sinh, cosh, tanh, sqrt, 
														exp, sin, cos, tan, abs, arg, ln, re, im };


		//return of function combine_values()
		//if function was able to combine to one single value, this gets returned
		//otherwise state == unknown and val is uninitialised.
		//special case: 
		struct Vals_Combined 
		{
			bool known;
			std::complex<double> val;
		};

	} //namespace intern

	//every time a variable is assosiated with a value, this struct is used
	struct Known_Variable 
	{
		const std::string name;
		const std::complex<double> value;
	};

} // namespace bmath
