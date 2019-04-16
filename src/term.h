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
	virtual ~Basic_Term();

	//appends this to str
	virtual void to_str(std::string& str) const = 0;

	//returns kinda true type of term (sum, product, value, etc.)
	virtual State get_state() const = 0;

	//sorts lists in anything containing lists (sum and product atm)
	virtual void sort() = 0;

	//specified for sort() function (does not always make sense mathematically!)
	virtual bool operator<(const Basic_Term& other) const = 0;
};

class Term : public Basic_Term {
private:
	Basic_Term* term_ptr;
public:
	Term(std::string name_);
	~Term();

	void to_str(std::string& str) const override;
	State get_state() const override;
	void sort() override; 
	bool operator<(const Basic_Term& other) const override;
};

}//namespace bruno

#include "internal_functions.h"
#include "arguments.h"
#include "operations.h"
#include "par_op.h"