#pragma once

#include <set>
#include <string>
#include <sstream>
#include <list>
#include <vector>
#include <iostream>
#include <math.h>

namespace bmath {

class Basic_Term 
{
protected:
	Basic_Term* parent;
public:
	Basic_Term(Basic_Term* parent_);
};

class Term : public Basic_Term {
private:
	Basic_Term* term_ptr;
public:
	Term(std::string name_);
	~Term();
};

}//namespace bruno

#include "internal_functions.h"
#include "arguments.h"
#include "operations.h"