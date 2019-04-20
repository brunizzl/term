#pragma once

#include <string>
#include <sstream>
#include <list>
#include <vector>
#include <iostream>
#include <cmath>

#include "structs.h"

namespace bmath {

class Basic_Term 
{
protected:
	Basic_Term* parent;
public:
	Basic_Term(Basic_Term* parent_);
	Basic_Term(const Basic_Term& source);
	virtual ~Basic_Term();

	//appends this to str
	virtual void to_str(std::string& str) const = 0;

	//returns kinda true type of term (sum, product, value, etc.)
	virtual State get_state() const = 0;

	//if one term holds a pointer to a term of same type
	//this function combines both in the upper term (if possible)
	virtual void combine() = 0;

	friend class Term;
	friend class Product;
	friend class Sum;
};

class Term : public Basic_Term {
private:
	Basic_Term* term_ptr;
public:
	Term(std::string name_);
	Term(const Term& source);
	~Term();

	void to_str(std::string& str) const override;
	State get_state() const override;
	void combine() override;

	//arithmetic operators
	Term& operator+=(const Term& summand);
	Term& operator-=(const Term& subtractor);
	Term& operator*=(const Term& factor);
	Term& operator/=(const Term& divisor);
};

}//namespace bruno

#include "internal_functions.h"
#include "arguments.h"
#include "operations.h"
#include "par_op.h"