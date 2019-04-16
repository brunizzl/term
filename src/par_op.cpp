#include "par_op.h"

using namespace bmath;



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
		name_.erase(0, 4);
		LOG_C("shortened name: " << name_);
		this->argument = build_subterm(name_, this);
		break;
	case log2:
	case asin:
	case acos:
	case atan:
	case sinh:
	case cosh:
	case tanh:
		name_.erase(0, 5);
		LOG_C("shortened name: " << name_);
		this->argument = build_subterm(name_, this);
		break;
	case log10:
	case gamma:
		name_.erase(0, 6);
		LOG_C("shortened name: " << name_);
		this->argument = build_subterm(name_, this);
		break;
	}
}

bmath::Par_Operator::~Par_Operator()
{
	LOG_C("loesche Par_Operator: " << *this);
	delete this->argument;
}

void bmath::Par_Operator::to_str(std::string & str) const
{
	switch (this->op_state) {
	case ln:
		str.append("ln(");
		break;
	case log10:
		str.append("log10(");
		break;
	case log2:
		str.append("log2(");
		break;
	case exp:
		str.append("exp(");
		break;
	case sin:
		str.append("sin(");
		break;
	case cos:
		str.append("cos(");
		break;
	case tan:
		str.append("tan(");
		break;
	case asin:
		str.append("asin(");
		break;
	case acos:
		str.append("acos(");
		break;
	case atan:
		str.append("atan(");
		break;
	case sinh:
		str.append("sinh(");
		break;
	case cosh:
		str.append("cosh(");
		break;
	case tanh:
		str.append("tanh(");
		break;
	case gamma:
		str.append("gamma(");
		break;
	}
	this->argument->to_str(str);
	str.push_back(')');
}

State bmath::Par_Operator::get_state() const
{
	return State();
}

void bmath::Par_Operator::sort()
{
}

bool bmath::Par_Operator::operator<(const Basic_Term & other) const
{
	return false;
}
