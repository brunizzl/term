#include "term.h"

using namespace bmath;

bmath::Basic_Term::Basic_Term(Basic_Term* parent_)
	:parent(parent_)
{
}

bmath::Basic_Term::Basic_Term(const Basic_Term& source)
	:parent(source.parent)
{
}

bmath::Basic_Term::~Basic_Term()
{
	//cleaning up the tree is done in derived classes
}

bmath::Term::Term(std::string name_)
	:Basic_Term(nullptr), term_ptr(build_subterm(name_, this))
{
}

bmath::Term::Term(const Term& source)
	:Basic_Term(nullptr), term_ptr(copy_subterm(source.term_ptr, this))
{
	LOG_C("kopiere Term: " << *this);
}

bmath::Term::~Term()
{
	LOG_C("loesche Term: " << *this);
	delete term_ptr;
}

void bmath::Term::to_str(std::string& str) const
{
	this->term_ptr->to_str(str);
}

State bmath::Term::get_state() const
{
	return undefined;
}

//void bmath::Term::sort()
//{
//}
//
//bool bmath::Term::operator<(const Basic_Term& other) const
//{
//	return false;
//}
