#pragma once

#include <string>
#include "term.h"
#include "structs.h"

namespace bmath {

//finds matching closed parethesis to the open_par in name 
std::size_t find_closed_par(std::size_t open_par, const std::string& name);

//finds the highest level of parentheses in name
void find_pars(const std::string& name, std::vector<Pos_Pars>& pars);

//skips parentheses, else finds_last_of characters in name like std::string function
std::size_t find_last_of_skip_pars(const std::string& name, const char* characters, const std::vector<Pos_Pars>& pars);

//skips parentheses, else rfind in name like std::string function
std::size_t rfind_skip_pars(const std::string& name, const char* searchstr, const std::vector<Pos_Pars>& pars);

//deletes parentheses of parentheses list, which start after the end of the name
void del_pars_after(std::vector<Pos_Pars>& pars, const std::string& name);

//returns pointer to newly build term of right type (u now have ownership of object)
//(needs modification if new termtype is added)
Basic_Term* build_subterm(std::string& subtermstr, Basic_Term* parent_);

//returns pointer to newly build term of right type (u now have ownership of object)
//(needs modification if new termtype is added)
Basic_Term* copy_subterm(const Basic_Term* source, Basic_Term* parent_);

//decides type of next subterm (finds the next operation to split)
//(needs modification if new termtype is added)
State type_subterm(const std::string& name, const std::vector<Pos_Pars>& pars, std::size_t& op, Par_Op_State& type_par_op);

} //namespace bmath

//allows ostream to output terms
std::ostream& operator<<(std::ostream& stream, const bmath::Basic_Term& term);
