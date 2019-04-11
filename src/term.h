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
};

class Term {
private:
	Basic_Term* term_ptr;
public:
	Term(std::string name_);
};

}//namespace bruno

#include "internal_functions.h"
#include "Exponentiation.h"
#include "Product.h"
#include "Sum.h"
#include "Value.h"
#include "Variable.h"