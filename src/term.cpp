#include "term.h"

using namespace bmath;

bmath::Basic_Term::Basic_Term(std::string name_, Basic_Term* parent_)
{
}

bmath::Term::Term(std::string name_)
	:term_ptr(nullptr)
{
	std::vector<Pos_Pars> pars;
	find_pars(name_, pars);
	//koennte man in funktion ausladen (sollte man auch)
	State state = type_subterm(name_, pars);
	switch (state) {
	case exponentiation:
		this->term_ptr = new Exponentiation(name_);
		return;
	case product:
		this->term_ptr = new Product(name_);
		return;
	case sum:
		this->term_ptr = new Sum(name_);
		return;
	case var:
		this->term_ptr = new Variable(name_);
		return;
	case val:
		this->term_ptr = new Value(name_);
		return;
	}

}