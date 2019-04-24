#include "par_op.h"

using namespace bmath;



bmath::Par_Operator::Par_Operator(Basic_Term* parent_)
	:Basic_Term(parent_), argument(nullptr), op_state(error)
{
}

bmath::Par_Operator::Par_Operator(std::string name_, Basic_Term* parent_, Par_Op_State op_state_)
	:Basic_Term(parent_), op_state(op_state_), argument(nullptr)
{
	LOG_C("baue Par_Operator: " << name_);
	name_.pop_back(); //closing parenthesis gets cut of
	switch (op_state_) {
		//erase function cuts of operator + opening par -> operator cases sorted by length
	case ln:
		name_.erase(0, 3);
		LOG_C("shortened name: " << name_);
		this->argument = build_subterm(name_, this);
		break;
	case exp:
	case sin:
	case cos:
	case tan:
	case abs:
		name_.erase(0, 4);
		LOG_C("shortened name: " << name_);
		this->argument = build_subterm(name_, this);
		break;
	case asin:
	case acos:
	case atan:
	case sinh:
	case cosh:
	case tanh:
	case sqrt:
		name_.erase(0, 5);
		LOG_C("shortened name: " << name_);
		this->argument = build_subterm(name_, this);
		break;
	case log10:
		name_.erase(0, 6);
		LOG_C("shortened name: " << name_);
		this->argument = build_subterm(name_, this);
		break;
	}
}

bmath::Par_Operator::Par_Operator(const Par_Operator& source, Basic_Term* parent_)
	:Basic_Term(parent_), argument(copy_subterm(source.argument, this)), op_state(source.op_state)
{
	LOG_C("kopiere Par_Operator: " << source);
}

bmath::Par_Operator::~Par_Operator()
{
	LOG_C("loesche Par_Operator: " << *this);
	delete this->argument;
}

void bmath::Par_Operator::to_str(std::string & str) const
{
	str.append(op_name(this->op_state));
	this->argument->to_str(str);
	str.push_back(')');
}

State bmath::Par_Operator::get_state() const
{
	return s_par_operator;
}

void bmath::Par_Operator::combine_layers()
{
	this->argument->combine_layers();
}

Vals_Combinded bmath::Par_Operator::combine_values()
{
	Vals_Combinded argument_ = argument->combine_values();
	if (argument_.known) {
		switch (this->op_state) {
		case ln:
			return Vals_Combinded{ true, std::log(argument_.val) };
		case log10:
			return Vals_Combinded{ true, std::log10(argument_.val) };
		case exp:
			return Vals_Combinded{ true, std::exp(argument_.val) };
		case sin:
			return Vals_Combinded{ true, std::sin(argument_.val) };
		case cos:
			return Vals_Combinded{ true, std::cos(argument_.val) };
		case tan:
			return Vals_Combinded{ true, std::tan(argument_.val) };
		case asin:
			return Vals_Combinded{ true, std::asin(argument_.val) };
		case acos:
			return Vals_Combinded{ true, std::acos(argument_.val) };
		case atan:
			return Vals_Combinded{ true, std::atan(argument_.val) };
		case sinh:
			return Vals_Combinded{ true, std::sinh(argument_.val) };
		case cosh:
			return Vals_Combinded{ true, std::cosh(argument_.val) };
		case tanh:
			return Vals_Combinded{ true, std::tanh(argument_.val) };
		case abs:
			return Vals_Combinded{ true, std::abs(argument_.val) };
		case sqrt:
			return Vals_Combinded{ true, std::sqrt(argument_.val) };
		}
	}
	return Vals_Combinded{ false, 0 };
}

Vals_Combinded bmath::Par_Operator::evaluate(const std::string & name_, std::complex<double> value_) const
{
	Vals_Combinded argument_ = argument->evaluate(name_, value_);
	if (argument_.known) {
		switch (this->op_state) {
		case ln:
			return Vals_Combinded{ true, std::log(argument_.val) };
		case log10:
			return Vals_Combinded{ true, std::log10(argument_.val) };
		case exp:
			return Vals_Combinded{ true, std::exp(argument_.val) };
		case sin:
			return Vals_Combinded{ true, std::sin(argument_.val) };
		case cos:
			return Vals_Combinded{ true, std::cos(argument_.val) };
		case tan:
			return Vals_Combinded{ true, std::tan(argument_.val) };
		case asin:
			return Vals_Combinded{ true, std::asin(argument_.val) };
		case acos:
			return Vals_Combinded{ true, std::acos(argument_.val) };
		case atan:
			return Vals_Combinded{ true, std::atan(argument_.val) };
		case sinh:
			return Vals_Combinded{ true, std::sinh(argument_.val) };
		case cosh:
			return Vals_Combinded{ true, std::cosh(argument_.val) };
		case tanh:
			return Vals_Combinded{ true, std::tanh(argument_.val) };
		case abs:
			return Vals_Combinded{ true, std::abs(argument_.val) };
		case sqrt:
			return Vals_Combinded{ true, std::sqrt(argument_.val) };
		}
	}
	return Vals_Combinded{ false, 0 };
}

bool bmath::Par_Operator::search_and_replace(const std::string& name_, std::complex<double> value_)
{
	if (this->argument->search_and_replace(name_, value_)) {
		delete this->argument;
		this->argument = new Value(value_, this);
	}
	return false;
}

bool bmath::Par_Operator::valid_state() const
{
	if (this->argument == nullptr) {
		return false;
	}
	return this->argument->valid_state();
}

//void bmath::Par_Operator::sort()
//{
//}
//
//bool bmath::Par_Operator::operator<(const Basic_Term & other) const
//{
//	return false;
//}
