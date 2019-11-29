
#pragma once

#include <string>
#include <complex>
#include <list>

namespace bmath {
	namespace intern {

		//specifies actual type of Basic_Term 
		//types ordered after their uniqueness (more unique is smaller)
		//this order is used to sort in an order that makes it easyer to match transformations
		enum class Type 
		{
			//most unique	(likely different op_type and op_type is always given)
			par_operator,
			//second most unique	(base is always base, exponent always exponent)
			exponentiation,
			//third most unique		(operands can vary in the positioning relative to each other)
			product,
			sum,
			//fourth most unique	(name or val can take nearly infinite states)
			variable,
			value,
			//least unique			(pattern variable can be representative for all categories)
			pattern_variable,
			//not really part of order, as a term is guaranteed to not be undefined.
			undefined,
		};

		static const Type all_types[] = { Type::par_operator, Type::exponentiation, Type::sum, Type::product, 
										  Type::variable, Type::value, Type::pattern_variable, Type::undefined };

		//used to restrict pattern_variable to only match basic_terms complying with restriction
		enum class Restriction
		{
			natural,
			integer,
			real,
			value,
			not_minus_one,
			minus_one,
			none,
		};

		static const Restriction all_restrictions[] = { Restriction::natural, Restriction::integer, Restriction::real, Restriction::value, 
														Restriction::not_minus_one, Restriction::minus_one, Restriction::none };

		//used in Par_Operator class to specify whitch operator is actually used 
		//(comments are corresponding std::complex functions)
		//types names are sorted by length (used to be required, as the type_subterm() function searched for par_op at not only the beginning of the name string)
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
		//otherwise state == unknown and val is undefinded.
		struct Vals_Combined 
		{
			bool known;
			std::complex<double> val;
		};

		//planned use is to manipulate *key using func with second beeing a local in callers environment.
		//used in build_subterm and build_pattern_subterm to not allocate new memory, -
		//if there is an option to directly modify an existing value instead of returning a new one.
		struct Value_Manipulator
		{
			std::complex<double>* key;
			void(*func)(std::complex<double>* first, std::complex<double> second);
		};

	} //namespace intern

	//every time a variable is assosiated with a value, this struct is used
	struct Known_Variable 
	{
		const std::string name;
		const std::complex<double> value;
	};

} // namespace bmath
