
#pragma once

#include <string>
#include <complex>
#include <list>

namespace bmath {
	namespace intern {

		//specifies actual type of Basic_Term 
		//(types ordered for parentheses to allow > / < / == etc. operators to test whether parentheses are needed)
		enum class Type 
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
		enum class Par_Op_Type 
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

		//array with instance of every Par_Op_Type to iterate over them
		static const Par_Op_Type all_par_op_types[] = { Par_Op_Type::log10, Par_Op_Type::asinh, Par_Op_Type::acosh, Par_Op_Type::atanh, 
														Par_Op_Type::asin, Par_Op_Type::acos, Par_Op_Type::atan, Par_Op_Type::sinh, 
														Par_Op_Type::cosh, Par_Op_Type::tanh, Par_Op_Type::sqrt, Par_Op_Type::exp, 
														Par_Op_Type::sin, Par_Op_Type::cos, Par_Op_Type::tan, Par_Op_Type::abs, 
														Par_Op_Type::arg, Par_Op_Type::ln, Par_Op_Type::re, Par_Op_Type::im };

		//these constants will not cause function is_computable to return false, as they are known. 
		//("i" is more of a unit to be exact, but i needed a name for all of them) 
		enum class Math_Constant
		{
			i,
			e,
			pi,
		};

		//array with instance of every Math_Constant to iterate over them
		static const Math_Constant all_math_constants[] = { Math_Constant::i, Math_Constant::e, Math_Constant::pi };


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
