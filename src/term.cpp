#include "term.h"

using namespace bmath;

bmath::Basic_Term::Basic_Term(Basic_Term* parent_)
	:parent(parent_)
{
}

bmath::Term::Term(std::string name_)
	:Basic_Term(nullptr), term_ptr(build_subterm(name_, this))
{
}

bmath::Term::~Term()
{
	delete term_ptr;
}
