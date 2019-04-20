#pragma once

#include "term.h"

namespace bmath {

class Value : public Basic_Term
{
private:
	double value;
public:
	Value(Basic_Term* parent_);
	Value(std::string name_, Basic_Term* parent_);
	Value(const Value& source, Basic_Term* parent_ = nullptr);
	~Value();

	void to_str(std::string& str) const override;
	State get_state() const override;
	void combine() override;

	friend class Term;
};


class Variable : public Basic_Term
{
private:
	std::string name;
public:
	Variable(Basic_Term* parent_);
	Variable(std::string name_, Basic_Term* parent_);
	Variable(const Variable& source, Basic_Term* parent_ = nullptr);
	~Variable();

	void to_str(std::string& str) const override;
	State get_state() const override;
	void combine() override;

	friend class Term;
};

} //namespace bmath

