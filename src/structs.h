#pragma once

//#define CONSTRUCTOR_LOG
#ifdef CONSTRUCTOR_LOG
	#define LOG_C(x) std::cout << x << '\n'
#else
	#define LOG_C(x)
#endif //CONSTRUCTOR_LOG

//#define PATTERN_LOG
#ifdef PATTERN_LOG
	#define LOG_P(x) std::cout << x << '\n'
#else
	#define LOG_P(x)
#endif //PATTERN_LOG

#include <string>
#include <complex>

namespace bmath {
	namespace intern {

		//specifies actual type of Basic_Term 
		//(types ordered for parentheses to allow > / < / == etc. operators to test whether parentheses are needed)
		enum State {
			s_undefined,
			s_par_operator,		//already has parentheses -> lower order than in reality
			s_sum,
			s_value,
			s_variable,
			s_pattern_variable,
			s_product,
			s_exponentiation,
		};

		//used in Par_Operator class to specify whitch operator is actually used 
		//(comments are corresponding cmath functions)
		//states names are sorted by length
		enum Par_Op_State {
			op_log10,		//log10()
			op_asinh,		//asinh()
			op_acosh,		//acosh()
			op_atanh,		//atanh()
			op_asin,		//asin()
			op_acos,		//acos()
			op_atan,		//atan()
			op_sinh,		//sinh()
			op_cosh,		//cosh()
			op_tanh,		//tanh()
			op_sqrt,		//sqrt()
			op_exp,			//exp()
			op_sin,			//sin()
			op_cos,			//cos()
			op_tan,			//tan()
			op_abs,			//abs()
			op_arg,			//arg()
			op_ln,			//log()
			op_re,			//real()
			op_im,			//imag()
			op_error,		//needs to be last element to enable loops to go trough the states (used as break condition)
		};

		//contains position of parentheses in constructor string
		struct Pos_Pars {
			std::size_t start;	//position of '('
			std::size_t end;	//position of ')'
		};

		//return of function combine_values()
		//if function was able to combine to one single value, this gets returned
		//otherwise known == false and val is uninitialised.
		struct Vals_Combined {
			bool known;
			std::complex<double> val;
		};

	} //namespace intern

	//every time a variable is assosiated with a value, this struct is used
	struct Known_Variable {
		const std::string name;
		const std::complex<double> value;
	};

} // namespace bmath
