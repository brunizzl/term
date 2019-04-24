#pragma once

#include "term.h"

namespace bmath {

class Value : public Basic_Term
{
public:
	double value;
	Value(Basic_Term* parent_);
	Value(std::string name_, Basic_Term* parent_);
	Value(const Value& source, Basic_Term* parent_ = nullptr);
	Value(double value_, Basic_Term* parent_);
	~Value();

	void to_str(std::string& str) const override;
	State get_state() const override; 
	Vals_Combinded combine_values() override;
	Vals_Combinded evaluate(const std::string & name_, double value_) const override;
	bool search_and_replace(const std::string& name_, double value_) override;
	bool valid_state() const override;

	friend class Term;
};


class Variable : public Basic_Term
{
public:
	std::string name;
	Variable(Basic_Term* parent_);
	Variable(std::string name_, Basic_Term* parent_);
	Variable(const Variable& source, Basic_Term* parent_ = nullptr);
	~Variable();

	void to_str(std::string& str) const override;
	State get_state() const override;
	Vals_Combinded combine_values() override;
	Vals_Combinded evaluate(const std::string & name_, double value_) const override;
	bool search_and_replace(const std::string& name_, double value_) override;
	bool valid_state() const override;

	friend class Term;
};

} //namespace bmath

