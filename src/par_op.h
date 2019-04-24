#pragma once

#include "term.h"

namespace bmath {

class Par_Operator : public Basic_Term
{
private:
	Par_Op_State op_state;
	Basic_Term* argument;
public:
	Par_Operator(Basic_Term* parent_);
	Par_Operator(std::string name_, Basic_Term* parent_, Par_Op_State op_state_);
	Par_Operator(const Par_Operator& source, Basic_Term* parent_ = nullptr);
	~Par_Operator();

	void to_str(std::string& str) const override;
	State get_state() const override;
	void combine_layers() override;
	Vals_Combinded combine_values() override;
	Vals_Combinded evaluate(const std::string & name_, double value_) const override;
	bool search_and_replace(const std::string& name_, double value_) override;
	bool valid_state() const override;

	friend class Term;
};

}