#pragma once

#include "term.h"

namespace bmath {

class Value : public Basic_Term
{
private:
	double value;
public:
	Value(std::string name_, Basic_Term* parent_);
	~Value();

	void to_str(std::string& str) const override;
	State get_state() const override;
	void sort() override;
	bool operator<(const Basic_Term& other) const override;
};


class Variable : public Basic_Term
{
private:
	std::string name;
public:
	Variable(std::string name_, Basic_Term* parent_);
	~Variable();

	void to_str(std::string& str) const override;
	State get_state() const override;
	void sort() override;
	bool operator<(const Basic_Term& other) const override;
};

} //namespace bmath

