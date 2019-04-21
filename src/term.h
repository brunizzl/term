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
	//pointer to whoever owns this (basic_term does not own parent)
	Basic_Term* parent;
public:
	Basic_Term(Basic_Term* parent_);
	Basic_Term(const Basic_Term& source);
	virtual ~Basic_Term();

	//appends this to str
	virtual void to_str(std::string& str) const = 0;

	//returns kinda true type of term (sum, product, value, etc.)
	virtual State get_state() const = 0;

	//if one term holds a pointer to a term of same type both are combinded (if possible)
	virtual void combine_layers();

	//values are added, multiplied, etc.
	virtual Vals_Combinded combine_values();

	//NOCH NICHT AUSGEDACHT
	virtual void combine_variables();

	//returns {true, whatever it adds up to} if only variable of name "name_" is present
	//returns {fale, undefined} if more variables are present
	virtual Vals_Combinded evaluate(const std::string & name_, double value_) const = 0;

	//searches an replaces all variables with name "name_" with values of value "value_"
	//this function is meant for permanent changes. else use evaluate()
	virtual bool search_and_replace(const std::string& name_, double value_) = 0;

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
	Vals_Combinded evaluate(const std::string & name_, double value_) const override;
	bool search_and_replace(const std::string& name_, double value_) override;

	void combine(); 

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