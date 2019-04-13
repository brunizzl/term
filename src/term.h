#pragma once

#include <set>
#include <string>
#include <sstream>
#include <list>
#include <vector>
#include <iostream>
#include <math.h>

#include "structs.h"

namespace bmath {

class Basic_Term 
{
protected:
	Basic_Term* parent;
public:
	Basic_Term(Basic_Term* parent_);

	virtual void to_str(std::string& str) const = 0;
	virtual State get_state() const = 0;
};

class Term : public Basic_Term {
private:
	Basic_Term* term_ptr;
public:
	Term(std::string name_);
	~Term();
	void to_str(std::string& str) const override;
	State get_state() const override;
};

}//namespace bruno

#include "internal_functions.h"
#include "arguments.h"
#include "operations.h"