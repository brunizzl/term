#pragma once

//#define CONSTRUCTOR_LOG
#ifdef CONSTRUCTOR_LOG
	#define LOG_C(x) std::cout << x << '\n'
#else
	#define LOG_C(x)
#endif //CONSTRUCTOR_LOG

#include <string>

namespace bmath {
	namespace intern {

		//contains position of parentheses in constructor string
		struct Pos_Pars {
			std::size_t start;	//position of '('
			std::size_t end;	//position of ')'
		};

		//specifies actual type of Basic_Term 
		//(types ordered for parentheses to allow > / < / == etc. operators)
		enum State {
			s_undefined,
			s_par_operator,		//already has parentheses -> lower order than in reality
			s_sum,
			s_value,
			s_variable,
			s_product,
			s_exponentiation
		};

		//used in Par_Operator class to specify whitch operator is actually used 
		//(comments are corresponding cmath functions)
		//states names are sorted by length
		enum Par_Op_State {
			log10,		//log10()
			asinh,		//asinh()
			acosh,		//acosh()
			atanh,		//atanh()
			asin,		//asin()
			acos,		//acos()
			atan,		//atan()
			sinh,		//sinh()
			cosh,		//cosh()
			tanh,		//tanh()
			sqrt,		//sqrt()
			exp,		//exp()
			sin,		//sin()
			cos,		//cos()
			tan,		//tan()
			abs,		//fabs()
			ln,			//log()
			error		//needs to be last element to enable loops to go trough the states (used as break condition)
		};

	} //namespace intern

	//return of function combine_values()
	//if function was able to combine to one single value, this gets returned
	//otherwise known == false and val is uninitialised.
	struct Vals_Combinded {
		bool known;
		std::complex<double> val;
	};

} // namespace bmath