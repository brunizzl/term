#include "operations.h"

using namespace bmath::intern;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Product\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bmath::intern::Product::Product(Basic_Term* parent_)
	:Basic_Term(parent_)
{
}

bmath::intern::Product::Product(std::string name_, Basic_Term* parent_, std::size_t op)
	:Basic_Term(parent_)
{
	LOG_C("baue Produkt: " << name_);
	std::vector<Pos_Pars> pars;
	find_pars(name_, pars);
	std::string subterm;
	while (op != std::string::npos) {
		subterm = name_.substr(op + 1);
		switch (name_[op]) {
		case '*':
			this->factors.push_front(build_subterm(subterm, this));
			break;
		case '/':
			this->divisors.push_front(build_subterm(subterm, this));
			break;
		}
		name_.erase(op);
		op = find_last_of_skip_pars(name_, "*/", pars);
	}
	this->factors.push_front(build_subterm(name_, this));
}

bmath::intern::Product::Product(std::string name_, Basic_Term* parent_, std::size_t op, std::list<Pattern_Variable*>& variables)
	:Basic_Term(parent_)
{
	LOG_C("baue Produkt: " << name_);
	std::vector<Pos_Pars> pars;
	find_pars(name_, pars);
	std::string subterm;
	while (op != std::string::npos) {
		subterm = name_.substr(op + 1);
		switch (name_[op]) {
		case '*':
			this->factors.push_front(build_pattern_subterm(subterm, this, variables));
			break;
		case '/':
			this->divisors.push_front(build_pattern_subterm(subterm, this, variables));
			break;
		}
		name_.erase(op);
		op = find_last_of_skip_pars(name_, "*/", pars);
	}
	this->factors.push_front(build_pattern_subterm(name_, this, variables));
}

bmath::intern::Product::Product(const Product& source, Basic_Term* parent_)
	:Basic_Term(parent_)
{
	LOG_C("kopiere Produkt: " << source);
	for (auto it : source.factors) {
		this->factors.push_back(copy_subterm(it, this));
	}
	for (auto it : source.divisors) {
		this->divisors.push_back(copy_subterm(it, this));
	}
}


bmath::intern::Product::~Product()
{
	LOG_C("loesche Produkt: " << *this);
	for (auto it : factors) {
		delete it;
	}
	for (auto it : divisors) {
		delete it;
	}
}

void bmath::intern::Product::to_str(std::string& str) const
{
	if (get_state(this->parent) >= this->get_state_intern()) {
		str.push_back('(');
	}
	bool already_printed_smth = false;
	for (auto it : this->factors) {
		if (std::exchange(already_printed_smth, true)) {
			str.push_back('*');
		}
		it->to_str(str);
	}
	if (!already_printed_smth) {
		str.push_back('1');
	}
	for (auto it : this->divisors) {
		str.push_back('/');
		it->to_str(str);
	}
	if (get_state(this->parent) >= this->get_state_intern()) {
		str.push_back(')');
	}
}

State bmath::intern::Product::get_state_intern() const
{
	return s_product;
}

void bmath::intern::Product::combine_layers()
{
	for (auto it = this->factors.begin(); it != this->factors.end();) {
		(*it)->combine_layers();
		if (get_state(*it) == s_product) {
			Product* redundant = static_cast<Product*>((*it));
			for (auto it_red : redundant->factors) {
				it_red->parent = this;
			}
			for (auto it_red : redundant->divisors) {
				it_red->parent = this;
			}
			this->factors.splice(this->factors.end(), redundant->factors);
			this->divisors.splice(this->divisors.end(), redundant->divisors);
			delete redundant;
			std::list<Basic_Term*>::iterator it_2 = it;
			++it;
			this->factors.erase(it_2);
			continue;
		}
		++it;
	}
	for (auto it = this->divisors.begin(); it != this->divisors.end();) {
		(*it)->combine_layers();
		if (get_state(*it) == s_product) {
			Product* redundant = static_cast<Product*>((*it));
			for (auto it_red : redundant->factors) {
				it_red->parent = this;
			}
			for (auto it_red : redundant->divisors) {
				it_red->parent = this;
			}
			this->factors.splice(this->factors.end(), redundant->divisors);
			this->divisors.splice(this->divisors.end(), redundant->factors);
			delete redundant;
			std::list<Basic_Term*>::iterator it_2 = it;
			++it;
			this->divisors.erase(it_2);
			continue;
		}
		++it;
	}
}

Vals_Combined bmath::intern::Product::combine_values()
{
	std::complex<double> buffer_factor = 1;
	bool only_known = true;
	for (auto it = this->factors.begin(); it != this->factors.end();) {
		Vals_Combined factor = (*it)->combine_values();
		if (factor.known) {
			buffer_factor *= factor.val;
			delete (*it);
			std::list<Basic_Term*>::iterator it_2 = it;
			++it;
			this->factors.erase(it_2);
			continue;
		}
		else {
			only_known = false;
		}
		++it;
	}
	for (auto it = this->divisors.begin(); it != this->divisors.end();) {
		Vals_Combined divisor = (*it)->combine_values();
		if (divisor.known) {
			buffer_factor /= divisor.val;
			delete (*it);
			std::list<Basic_Term*>::iterator it_2 = it;
			++it;
			this->divisors.erase(it_2);
			continue;
		}
		else {
			only_known = false;
		}
		++it;
	}
	if (buffer_factor != std::complex<double>(1, 0)) {
		this->factors.push_front(new Value(buffer_factor, this));
	}
	if (only_known) {
		return Vals_Combined{ true, buffer_factor };
	}
	return Vals_Combined{ false, 0 };
}

Vals_Combined bmath::intern::Product::evaluate(const std::list<Known_Variable>& known_variables) const
{
	Vals_Combined result{ true, 1 };
	for (auto it : this->factors) {
		Vals_Combined factor_combined = it->evaluate(known_variables);
		if (factor_combined.known) {
			result.val *= factor_combined.val;
		}
		else {
			return Vals_Combined{ false, 0 };
		}
	}
	for (auto it : this->divisors) {
		Vals_Combined divisor_combined = it->evaluate(known_variables);
		if (divisor_combined.known) {
			result.val /= divisor_combined.val;
		}
		else {
			return Vals_Combined{ false, 0 };
		}
	}
	return result;
}

void bmath::intern::Product::search_and_replace(const std::string& name_, std::complex<double> value_, Basic_Term*& storage_key)
{
	for (auto& it : this->factors) {
		it->search_and_replace(name_, value_, it);
	}
	for (auto& it : this->divisors) {
		it->search_and_replace(name_, value_, it);
	}
}

bool bmath::intern::Product::valid_state() const
{
	for (auto it : this->factors) {

		if (it == nullptr || it->valid_state() == false) {
			return false;
		}
	}
	for (auto it : this->divisors) {
		if (it == nullptr || it->valid_state() == false) {
			return false;
		}
	}
	return true;
}

void bmath::intern::Product::list_subterms(std::list<Basic_Term*>& subterms, State listed_state) const
{
	if (listed_state == s_product) {
		subterms.push_back(const_cast<Product*>(this));
	}
	for (auto it : this->factors) {
		it->list_subterms(subterms, listed_state);
	}
	for (auto it : this->divisors) {
		it->list_subterms(subterms, listed_state);
	}
}

void bmath::intern::Product::sort()
{
	for (auto& it : this->factors) {
		it->sort();
	}
	for (auto& it : this->divisors) {
		it->sort();
	}
	this->factors.sort([](Basic_Term * &a, Basic_Term * &b) -> bool {return *a < *b; });
	this->divisors.sort([](Basic_Term * &a, Basic_Term * &b) -> bool {return *a < *b; });
}

Basic_Term* bmath::intern::Product::match_intern(Basic_Term* pattern, std::list<Basic_Term*>& pattern_var_adresses)
{
	if (this->get_state_intern() == pattern->get_state_intern()) {
		Product* pattern_product = static_cast<Product*>(pattern);
		if (this->factors.size() >= pattern_product->factors.size() && this->divisors.size() >= pattern_product->divisors.size()) {
			bool full_match = true;
			std::list<Basic_Term*> matched_factors;
			std::list<Basic_Term*> matched_divisors;
			//it is not possible so search this->factors always from start, as for example pattern "a*a" would match with every product, because the same factor would be read as first AND second "a"
			std::list<Basic_Term*>::iterator this_factor = const_cast<Product*>(this)->factors.begin();	//therefore we assume sorted lists and only iterate trough this->factors once.
			for (auto& pattern_factor : pattern_product->factors) {
				Basic_Term* factor_match;
				for (; this_factor != this->factors.end(); ++this_factor) {
					factor_match = (*this_factor)->match_intern(pattern_factor, pattern_var_adresses);
					if (factor_match != nullptr) {
						matched_factors.splice(matched_factors.end(), this->factors, this_factor);
						break;
					}
				}
				if (factor_match == nullptr) {
					full_match = false;
					break;
				}
			}
			if (full_match) {	//same for divisors
				std::list<Basic_Term*>::iterator this_divisor = const_cast<Product*>(this)->divisors.begin();
				for (auto& pattern_divisor : pattern_product->divisors) {
					Basic_Term* divisor_match;
					for (; this_divisor != this->divisors.end(); ++this_divisor) {
						divisor_match = (*this_divisor)->match_intern(pattern_divisor, pattern_var_adresses);
						if (divisor_match != nullptr) {
							matched_divisors.splice(matched_divisors.end(), this->divisors, this_divisor);
							break;
						}
					}
					if (divisor_match == nullptr) {
						full_match = false;
						break;
					}
				}
			}
			if (full_match) {
				//pushing matched factors + divisors into their own product
				if (this->factors.size() > pattern_product->factors.size() || this->divisors.size() > pattern_product->divisors.size()) {
					Product* matched_product = new Product(this);
					matched_product->factors.splice(matched_product->factors.end(), matched_factors);
					matched_product->divisors.splice(matched_product->divisors.end(), matched_divisors);
					for (auto& it : matched_product->factors) {
						it->parent = matched_product;
					}
					for (auto& it : matched_product->divisors) {
						it->parent = matched_product;
					}
					this->factors.push_back(matched_product);
					return matched_product;
				}
				else {
					this->factors = std::move(matched_factors);
					this->divisors = std::move(matched_divisors);
					return const_cast<Product*>(this);
				}
			}
		}
	}
	else {
		Basic_Term* argument_match;
		for (auto& it : factors) {
			reset_pattern_vars(pattern_var_adresses);
			argument_match = it->match_intern(pattern, pattern_var_adresses);
			if (argument_match != nullptr) {
				return argument_match;
			}
		}
		for (auto& it : divisors) {
			reset_pattern_vars(pattern_var_adresses);
			argument_match = it->match_intern(pattern, pattern_var_adresses);
			if (argument_match != nullptr) {
				return argument_match;
			}
		}
	}
	reset_pattern_vars(pattern_var_adresses);
	return nullptr;
}

bool bmath::intern::Product::operator<(const Basic_Term& other) const
{
	if (this->get_state_intern() != other.get_state_intern()) {
		return this->get_state_intern() < other.get_state_intern();
	}
	else {
		const Product* other_product = static_cast<const Product*>(&other);
		if (this->factors.size() != other_product->factors.size()) {
			return this->factors.size() < other_product->factors.size();
		}
		if (this->divisors.size() != other_product->divisors.size()) {
			return this->divisors.size() < other_product->divisors.size();
		}
		//the operator assumes from now on to have sorted products to compare
		auto it_this = this->factors.begin();
		auto it_other = other_product->factors.begin();
		for (; it_this != this->factors.end() && it_other != other_product->factors.end(); ++it_this, ++it_other) {
			if (!((**it_this) == (**it_other))) {
				return (**it_this) < (**it_other);
			}
		}
		it_this = this->divisors.begin();
		it_other = other_product->divisors.begin();
		for (; it_this != this->divisors.end() && it_other != other_product->divisors.end(); ++it_this, ++it_other) {
			if (!((**it_this) == (**it_other))) {
				return (**it_this) < (**it_other);
			}
		}
		return false;
	}
}

bool bmath::intern::Product::operator==(const Basic_Term& other) const
{
	if (this->get_state_intern() != other.get_state_intern()) {
		return false;
	}
	else {
		const Product* other_product = static_cast<const Product*>(&other);
		if (this->factors.size() != other_product->factors.size()) {
			return false;
		}
		if (this->divisors.size() != other_product->divisors.size()) {
			return false;
		}
		//the operator assumes from now on to have sorted products to compare
		auto it_this = this->factors.begin();
		auto it_other = other_product->factors.begin();
		for (; it_this != this->factors.end() && it_other != other_product->factors.end(); ++it_this, ++it_other) {
			if ((**it_this) != (**it_other)) {
				return false;
			}
		}
		it_this = this->divisors.begin();
		it_other = other_product->divisors.begin();
		for (; it_this != this->divisors.end() && it_other != other_product->divisors.end(); ++it_this, ++it_other) {
			if ((**it_this) != (**it_other)) {
				return false;
			}
		}
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Sum\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bmath::intern::Sum::Sum(Basic_Term* parent_)
	:Basic_Term(parent_)
{
}

bmath::intern::Sum::Sum(std::string name_, Basic_Term* parent_, std::size_t op)
	:Basic_Term(parent_)
{
	LOG_C("baue Summe: " << name_);
	std::vector<Pos_Pars> pars;
	find_pars(name_, pars);
	std::string subterm;
	while (op != std::string::npos) {
		subterm = name_.substr(op + 1);
		switch (name_[op]) {
		case '+':
			this->summands.push_front(build_subterm(subterm, this));
			break;
		case '-':
			this->subtractors.push_front(build_subterm(subterm, this));
			break;
		}
		name_.erase(op);
		op = find_last_of_skip_pars(name_, "+-", pars);
	}
	if (name_.size() != 0) {
		this->summands.push_front(build_subterm(name_, this));
	}
}

bmath::intern::Sum::Sum(std::string name_, Basic_Term* parent_, std::size_t op, std::list<Pattern_Variable*>& variables)
	:Basic_Term(parent_)
{
	LOG_C("baue Summe: " << name_);
	std::vector<Pos_Pars> pars;
	find_pars(name_, pars);
	std::string subterm;
	while (op != std::string::npos) {
		subterm = name_.substr(op + 1);
		switch (name_[op]) {
		case '+':
			this->summands.push_front(build_pattern_subterm(subterm, this, variables));
			break;
		case '-':
			this->subtractors.push_front(build_pattern_subterm(subterm, this, variables));
			break;
		}
		name_.erase(op);
		op = find_last_of_skip_pars(name_, "+-", pars);
	}
	if (name_.size() != 0) {
		this->summands.push_front(build_pattern_subterm(name_, this, variables));
	}
}

bmath::intern::Sum::Sum(const Sum& source, Basic_Term* parent_)
	:Basic_Term(parent_)
{
	LOG_C("kopiere Summe: " << source);
	for (auto it : source.summands) {
		this->summands.push_back(copy_subterm(it, this));
	}
	for (auto it : source.subtractors) {
		this->subtractors.push_back(copy_subterm(it, this));
	}
}

bmath::intern::Sum::~Sum()
{
	LOG_C("loesche Summe: " << *this);
	for (auto it : summands) {
		delete it;
	}
	for (auto it : subtractors) {
		delete it;
	}
}

void bmath::intern::Sum::to_str(std::string& str) const
{
	if (get_state(this->parent) >= this->get_state_intern()) {
		str.push_back('(');
	}
	bool need_operator = false;
	for (auto it : this->summands) {
		if (std::exchange(need_operator, true)/* && get_state(it) != s_value*/) {
			str.push_back('+');
		}
		it->to_str(str);
	}
	for (auto it : this->subtractors) {
		str.push_back('-');
		it->to_str(str);
	}
	if (get_state(this->parent) >= this->get_state_intern()) {
		str.push_back(')');
	}
}

State bmath::intern::Sum::get_state_intern() const
{
	return s_sum;
}

void bmath::intern::Sum::combine_layers()
{
	for (auto it = this->summands.begin(); it != this->summands.end();) {
		(*it)->combine_layers();
		if (get_state(*it) == s_sum) {
			Sum* redundant = static_cast<Sum*>((*it));
			for (auto it_red : redundant->summands) {
				it_red->parent = this;
			}
			for (auto it_red : redundant->subtractors) {
				it_red->parent = this;
			}
			this->summands.splice(this->summands.end(), redundant->summands);
			this->subtractors.splice(this->subtractors.end(), redundant->subtractors);
			delete redundant;
			std::list<Basic_Term*>::iterator it_2 = it;
			++it;
			this->summands.erase(it_2);
			continue;
		}
		++it;
	}
	for (auto it = this->subtractors.begin(); it != this->subtractors.end();) {
		(*it)->combine_layers();
		if (get_state(*it) == s_sum) {
			Sum* redundant = static_cast<Sum*>((*it));
			for (auto it_red : redundant->summands) {
				it_red->parent = this;
			}
			for (auto it_red : redundant->subtractors) {
				it_red->parent = this;
			}
			this->summands.splice(this->summands.end(), redundant->subtractors);
			this->subtractors.splice(this->subtractors.end(), redundant->summands);
			delete redundant;
			std::list<Basic_Term*>::iterator it_2 = it;
			++it;
			this->subtractors.erase(it_2);
			continue;
		}
		++it;
	}
}

Vals_Combined bmath::intern::Sum::combine_values()
{
	std::complex<double> buffer_summand = 0;
	bool only_known = true;
	for (auto it = this->summands.begin(); it != this->summands.end();) {
		Vals_Combined summand = (*it)->combine_values();
		if (summand.known) {
			buffer_summand += summand.val;
			delete (*it);
			std::list<Basic_Term*>::iterator it_2 = it;
			++it;
			this->summands.erase(it_2);
			continue;
		}
		else {
			only_known = false;
		}
		++it;
	}
	for (std::list<Basic_Term*>::iterator it = this->subtractors.begin(); it != this->subtractors.end();) {
		Vals_Combined subtractor = (*it)->combine_values();
		if (subtractor.known) {
			buffer_summand -= subtractor.val;
			delete (*it);
			std::list<Basic_Term*>::iterator it_2 = it;
			++it;
			this->subtractors.erase(it_2);
			continue;
		}
		else {
			only_known = false;
		}
		++it;
	}
	if (buffer_summand != std::complex<double>(0, 0)) {
		this->summands.push_front(new Value(buffer_summand, this));
	}
	if (only_known) {
		return Vals_Combined{ true, buffer_summand };
	}	
	return Vals_Combined{ false, 0 };
}

Vals_Combined bmath::intern::Sum::evaluate(const std::list<Known_Variable>& known_variables) const
{
	Vals_Combined result{ true, 0 };
	for (auto it : this->summands) {
		Vals_Combined summand_combined = it->evaluate(known_variables);
		if (summand_combined.known) {
			result.val += summand_combined.val;
		}
		else {
			return Vals_Combined{ false, 0 };
		}
	}
	for (auto it : this->subtractors) {
		Vals_Combined subtractor_combined = it->evaluate(known_variables);
		if (subtractor_combined.known) {
			result.val -= subtractor_combined.val;
		}
		else {
			return Vals_Combined{ false, 0 };
		}
	}
	return result;
}

void bmath::intern::Sum::search_and_replace(const std::string& name_, std::complex<double> value_, Basic_Term*& storage_key)
{
	for (auto& it : this->summands) {
		it->search_and_replace(name_, value_, it);
	}
	for (auto& it : this->subtractors) {
		it->search_and_replace(name_, value_, it);
	}
}

bool bmath::intern::Sum::valid_state() const
{
	for (auto it : this->summands) {
		if (it == nullptr || it->valid_state() == false) {
			return false;
		}
	}
	for (auto it : this->subtractors) {
		if (it == nullptr || it->valid_state() == false) {
			return false;
		}
	}
	return true;
}

void bmath::intern::Sum::list_subterms(std::list<Basic_Term*>& subterms, State listed_state) const
{
	if (listed_state == s_sum) {
		subterms.push_back(const_cast<Sum*>(this));
	}
	for (auto it : this->summands) {
		it->list_subterms(subterms, listed_state);
	}
	for (auto it : this->subtractors) {
		it->list_subterms(subterms, listed_state);
	}
}

void bmath::intern::Sum::sort()
{
	//vielleicht hier vor noch die standardisierung mit produkt einfügen etc.?
	for (auto& it : this->summands) {
		it->sort();
	}
	for (auto& it : this->subtractors) {
		it->sort();
	}
	this->summands.sort([](Basic_Term*& a, Basic_Term*& b) -> bool {return *a < *b; });
	this->subtractors.sort([](Basic_Term*& a, Basic_Term*& b) -> bool {return *a < *b; });
}

Basic_Term* bmath::intern::Sum::match_intern(Basic_Term* pattern, std::list<Basic_Term*>& pattern_var_adresses)
{
	//MUSS NOCH VERGLEICHEN, OB NUR TEILE VON THIS GLEICH GANZEM PATTERN SIND
	if (*this == *pattern) {
		return const_cast<Sum*>(this);
	}
	else {
		Basic_Term* argument_match;
		for (auto& it : summands) {
			reset_pattern_vars(pattern_var_adresses);
			argument_match = it->match_intern(pattern, pattern_var_adresses);
			if (argument_match != nullptr) {
				return argument_match;
			}
		}
		for (auto& it : subtractors) {
			reset_pattern_vars(pattern_var_adresses);
			argument_match = it->match_intern(pattern, pattern_var_adresses);
			if (argument_match != nullptr) {
				return argument_match;
			}
		}
		reset_pattern_vars(pattern_var_adresses);
		return nullptr;
	}
}

bool bmath::intern::Sum::operator<(const Basic_Term& other) const
{
	if (this->get_state_intern() != other.get_state_intern()) {
		return this->get_state_intern() < other.get_state_intern();
	}
	else {
		const Sum* other_sum = static_cast<const Sum*>(&other);
		if (this->summands.size() != other_sum->summands.size()) {
			return this->summands.size() < other_sum->summands.size();
		}
		if (this->subtractors.size() != other_sum->subtractors.size()) {
			return this->subtractors.size() < other_sum->subtractors.size();
		}
		//the operator assumes from now on to have sorted sums to compare
		auto it_this = this->summands.begin();
		auto it_other = other_sum->summands.begin();
		for (; it_this != this->summands.end() && it_other != other_sum->summands.end(); ++it_this, ++it_other) {
			if ((**it_this) < (**it_other)) {
				return true;
			}
			if ((**it_other) < (**it_this)) {
				return false;
			}
		}
		it_this = this->subtractors.begin();
		it_other = other_sum->subtractors.begin();
		for (; it_this != this->subtractors.end() && it_other != other_sum->subtractors.end(); ++it_this, ++it_other) {
			if ((**it_this) < (**it_other)) {
				return true;
			}
			if ((**it_other) < (**it_this)) {
				return false;
			}
		}
		return false;
	}
}

bool bmath::intern::Sum::operator==(const Basic_Term& other) const
{

	if (this->get_state_intern() != other.get_state_intern()) {
		return false;
	}
	else {
		const Sum* other_product = static_cast<const Sum*>(&other);
		if (this->summands.size() != other_product->summands.size()) {
			return false;
		}
		if (this->subtractors.size() != other_product->subtractors.size()) {
			return false;
		}
		//the operator assumes from now on to have sorted products to compare
		auto it_this = this->summands.begin();
		auto it_other = other_product->summands.begin();
		for (; it_this != this->summands.end() && it_other != other_product->summands.end(); ++it_this, ++it_other) {
			if ((**it_this) != (**it_other)) {
				return false;
			}
		}
		it_this = this->subtractors.begin();
		it_other = other_product->subtractors.begin();
		for (; it_this != this->subtractors.end() && it_other != other_product->subtractors.end(); ++it_this, ++it_other) {
			if ((**it_this) != (**it_other)) {
				return false;
			}
		}
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Exponentiation\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bmath::intern::Exponentiation::Exponentiation(Basic_Term* parent_)
	:Basic_Term(parent_), base(nullptr), exponent(nullptr)
{
}

bmath::intern::Exponentiation::Exponentiation(std::string name_, Basic_Term* parent_, std::size_t op)
	:Basic_Term(parent_)
{
	LOG_C("baue Potenz: " << name_);
	std::string subterm;
	subterm = name_.substr(op + 1);
	this->exponent = build_subterm(subterm, this);
	name_.erase(op);
	this->base = build_subterm(name_, this);
}

bmath::intern::Exponentiation::Exponentiation(std::string name_, Basic_Term* parent_, std::size_t op, std::list<Pattern_Variable*>& variables)
	:Basic_Term(parent_)
{
	LOG_C("baue Potenz: " << name_);
	std::string subterm;
	subterm = name_.substr(op + 1);
	this->exponent = build_pattern_subterm(subterm, this, variables);
	name_.erase(op);
	this->base = build_pattern_subterm(subterm, this, variables);
}

bmath::intern::Exponentiation::Exponentiation(const Exponentiation& source, Basic_Term* parent_)
	:Basic_Term(parent_), base(copy_subterm(source.base, this)), exponent(copy_subterm(source.exponent, this))
{
	LOG_C("kopiere Potenz: " << source);
}

bmath::intern::Exponentiation::~Exponentiation()
{
	LOG_C("loesche Potenz: " << *this);
	delete exponent;
	delete base;
}

void bmath::intern::Exponentiation::to_str(std::string& str) const
{
	if (get_state(this->parent) > this->get_state_intern()) {
		str.push_back('(');
	}
	this->base->to_str(str);
	str.push_back('^');
	this->exponent->to_str(str);
	if (get_state(this->parent) > this->get_state_intern()) {
		str.push_back(')');
	}
}

State bmath::intern::Exponentiation::get_state_intern() const
{
	return s_exponentiation;
}

void bmath::intern::Exponentiation::combine_layers()
{
	this->base->combine_layers();
	this->exponent->combine_layers();
}

Vals_Combined bmath::intern::Exponentiation::combine_values()
{
	Vals_Combined base_ = this->base->combine_values();
	Vals_Combined exponent_ = this->exponent->combine_values();
	if (base_.known && exponent_.known) {
		std::complex<double> result = std::pow(base_.val, exponent_.val);
		return Vals_Combined{ true, result };
	}
	else if (base_.known && !exponent_.known) {
		if (get_state(this->base) != s_value) {
			delete this->base;
			this->base = new Value(base_.val, this);
		}
	}
	else if (!base_.known && exponent_.known) {
		if (get_state(this->exponent) != s_value) {
			delete this->exponent;
			this->exponent = new Value(exponent_.val, this);
		}
	}

	return Vals_Combined{ false, 0 };
}

Vals_Combined bmath::intern::Exponentiation::evaluate(const std::list<Known_Variable>& known_variables) const
{
	Vals_Combined base_ = this->base->evaluate(known_variables);
	Vals_Combined exponent_ = this->exponent->evaluate(known_variables);
	if (base_.known && exponent_.known) {
		std::complex<double> result = std::pow(base_.val, exponent_.val);
		return Vals_Combined{ true, result };
	}
	return Vals_Combined{ false, 0 };
}

void bmath::intern::Exponentiation::search_and_replace(const std::string& name_, std::complex<double> value_, Basic_Term*& storage_key)
{
	this->base->search_and_replace(name_, value_, this->base);
	this->exponent->search_and_replace(name_, value_, this->exponent);
}

bool bmath::intern::Exponentiation::valid_state() const
{
	if (this->base == nullptr || this->exponent == nullptr) {
		return false;
	}
	return this->base->valid_state() && this->exponent->valid_state();
}

void bmath::intern::Exponentiation::list_subterms(std::list<Basic_Term*>& subterms, State listed_state) const
{
	if (listed_state == s_exponentiation) {
		subterms.push_back(const_cast<Exponentiation*>(this));
	}
	this->base->list_subterms(subterms, listed_state);
	this->exponent->list_subterms(subterms, listed_state);
}

void bmath::intern::Exponentiation::sort()
{
	this->base->sort();
	this->exponent->sort();
}

Basic_Term* bmath::intern::Exponentiation::match_intern(Basic_Term* pattern, std::list<Basic_Term*>& pattern_var_adresses)
{

	if (*this == *pattern) {
		return const_cast<Exponentiation*>(this);
	}
	else {
		reset_pattern_vars(pattern_var_adresses);
		Basic_Term* argument_match = base->match_intern(pattern, pattern_var_adresses);
		if (argument_match != nullptr) {
			return argument_match;
		}
		reset_pattern_vars(pattern_var_adresses);
		argument_match = exponent->match_intern(pattern, pattern_var_adresses);
		if (argument_match != nullptr) {
			return argument_match;
		}
		reset_pattern_vars(pattern_var_adresses);
		return nullptr;
	}
}

bool bmath::intern::Exponentiation::operator<(const Basic_Term& other) const
{
	if (this->get_state_intern() != other.get_state_intern()) {
		return this->get_state_intern() < other.get_state_intern();
	}
	else {
		const Exponentiation* other_exp = static_cast<const Exponentiation*>(&other);
		if (*(this->base) != *(other_exp->base)) {
			return *(this->base) < *(other_exp->base);
		}
		if (*(this->exponent) != *(other_exp->exponent)) {
			return *(this->exponent) < *(other_exp->exponent);
		}
	}
	return false;
}

bool bmath::intern::Exponentiation::operator==(const Basic_Term& other) const
{
	if (this->get_state_intern() != other.get_state_intern()) {
		return false;
	}
	else {
		const Exponentiation* other_exp = static_cast<const Exponentiation*>(&other);
		if (*(this->base) != *(other_exp->base)) {
			return false;
		}
		if (*(this->exponent) != *(other_exp->exponent)) {
			return false;
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Parenthesis_Operator\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bmath::intern::Par_Operator::Par_Operator(Basic_Term* parent_)
	:Basic_Term(parent_), argument(nullptr), op_state(error)
{
}

Vals_Combined bmath::intern::Par_Operator::internal_combine(Vals_Combined argument_) const
{
	if (argument_.known) {
		switch (this->op_state) {
		case log10:
			return Vals_Combined{ true, std::log10(argument_.val) };
		case asin:
			return Vals_Combined{ true, std::asin(argument_.val) };
		case acos:
			return Vals_Combined{ true, std::acos(argument_.val) };
		case atan:
			return Vals_Combined{ true, std::atan(argument_.val) };
		case asinh:
			return Vals_Combined{ true, std::asinh(argument_.val) };
		case acosh:
			return Vals_Combined{ true, std::acosh(argument_.val) };
		case atanh:
			return Vals_Combined{ true, std::atanh(argument_.val) };
		case sinh:
			return Vals_Combined{ true, std::sinh(argument_.val) };
		case cosh:
			return Vals_Combined{ true, std::cosh(argument_.val) };
		case tanh:
			return Vals_Combined{ true, std::tanh(argument_.val) };
		case sqrt:
			return Vals_Combined{ true, std::sqrt(argument_.val) };
		case exp:
			return Vals_Combined{ true, std::exp(argument_.val) };
		case sin:
			return Vals_Combined{ true, std::sin(argument_.val) };
		case cos:
			return Vals_Combined{ true, std::cos(argument_.val) };
		case tan:
			return Vals_Combined{ true, std::tan(argument_.val) };
		case abs:
			return Vals_Combined{ true, std::abs(argument_.val) };;
		case ln:
			return Vals_Combined{ true, std::log(argument_.val) };
		}
	}
	return Vals_Combined{ false, 0 };
}

bmath::intern::Par_Operator::Par_Operator(std::string name_, Basic_Term* parent_, Par_Op_State op_state_)
	:Basic_Term(parent_), op_state(op_state_), argument(nullptr)
{
	LOG_C("baue Par_Operator: " << name_);
	name_.pop_back(); //closing parenthesis gets cut of
	switch (op_state_) {
		//erase function cuts of operator + opening par -> operator cases sorted by length
	case ln:
		name_.erase(0, 3);
		LOG_C("shortened name: " << name_);
		this->argument = build_subterm(name_, this);
		break;
	case exp:
	case sin:
	case cos:
	case tan:
	case abs:
		name_.erase(0, 4);
		LOG_C("shortened name: " << name_);
		this->argument = build_subterm(name_, this);
		break;
	case asin:
	case acos:
	case atan:
	case sinh:
	case cosh:
	case tanh:
	case sqrt:
		name_.erase(0, 5);
		LOG_C("shortened name: " << name_);
		this->argument = build_subterm(name_, this);
		break;
	case log10:
	case asinh:
	case acosh:
	case atanh:
		name_.erase(0, 6);
		LOG_C("shortened name: " << name_);
		this->argument = build_subterm(name_, this);
		break;
	}
}

bmath::intern::Par_Operator::Par_Operator(std::string name_, Basic_Term* parent_, Par_Op_State op_state_, std::list<Pattern_Variable*>& variables)
	:Basic_Term(parent_), op_state(op_state_), argument(nullptr)
{
	LOG_C("baue Par_Operator: " << name_);
	name_.pop_back(); //closing parenthesis gets cut of
	switch (op_state_) {
		//erase function cuts of operator + opening par -> operator cases sorted by length
	case ln:
		name_.erase(0, 3);
		LOG_C("shortened name: " << name_);
		this->argument = build_pattern_subterm(name_, this, variables);
		break;
	case exp:
	case sin:
	case cos:
	case tan:
	case abs:
		name_.erase(0, 4);
		LOG_C("shortened name: " << name_);
		this->argument = build_pattern_subterm(name_, this, variables);
		break;
	case asin:
	case acos:
	case atan:
	case sinh:
	case cosh:
	case tanh:
	case sqrt:
		name_.erase(0, 5);
		LOG_C("shortened name: " << name_);
		this->argument = build_pattern_subterm(name_, this, variables);
		break;
	case log10:
	case asinh:
	case acosh:
	case atanh:
		name_.erase(0, 6);
		LOG_C("shortened name: " << name_);
		this->argument = build_pattern_subterm(name_, this, variables);
		break;
	}
}


bmath::intern::Par_Operator::Par_Operator(const Par_Operator & source, Basic_Term * parent_)
	:Basic_Term(parent_), argument(copy_subterm(source.argument, this)), op_state(source.op_state)
{
	LOG_C("kopiere Par_Operator: " << source);
}

bmath::intern::Par_Operator::~Par_Operator()
{
	LOG_C("loesche Par_Operator: " << *this);
	delete this->argument;
}

void bmath::intern::Par_Operator::to_str(std::string & str) const
{
	str.append(op_name(this->op_state));
	this->argument->to_str(str);
	str.push_back(')');
}

State bmath::intern::Par_Operator::get_state_intern() const
{
	return s_par_operator;
}

void bmath::intern::Par_Operator::combine_layers()
{
	this->argument->combine_layers();
}

Vals_Combined bmath::intern::Par_Operator::combine_values()
{
	return this->internal_combine(argument->combine_values());
}

Vals_Combined bmath::intern::Par_Operator::evaluate(const std::list<Known_Variable>& known_variables) const
{
	return this->internal_combine(argument->evaluate(known_variables));
}

void bmath::intern::Par_Operator::search_and_replace(const std::string & name_, std::complex<double> value_, Basic_Term*& storage_key)
{
	this->argument->search_and_replace(name_, value_, this->argument);
	
}

bool bmath::intern::Par_Operator::valid_state() const
{
	if (this->argument == nullptr) {
		return false;
	}
	return this->argument->valid_state();
}

void bmath::intern::Par_Operator::list_subterms(std::list<Basic_Term*>& subterms, State listed_state) const
{
	if (listed_state == s_par_operator) {
		subterms.push_back(const_cast<Par_Operator*>(this));
	}
	this->argument->list_subterms(subterms, listed_state);
}

void bmath::intern::Par_Operator::sort()
{
	this->argument->sort();
}

Basic_Term* bmath::intern::Par_Operator::match_intern(Basic_Term* pattern, std::list<Basic_Term*>& pattern_var_adresses)
{
	if (*this == *pattern) {
		return const_cast<Par_Operator*>(this);
	}
	else {
		reset_pattern_vars(pattern_var_adresses);
		Basic_Term* argument_match = argument->match_intern(pattern, pattern_var_adresses);
		if (argument_match != nullptr) {
			return argument_match;
		}
		reset_pattern_vars(pattern_var_adresses);
		return nullptr;
	}
}

bool bmath::intern::Par_Operator::operator<(const Basic_Term& other) const
{
	if (this->get_state_intern() != other.get_state_intern()) {
		return this->get_state_intern() < other.get_state_intern();
	}
	else {
		const Par_Operator* other_par_op = static_cast<const Par_Operator*>(&other);
		if (this->op_state != other_par_op->op_state) {
			return this->op_state < other_par_op->op_state;
		}
		if (*(this->argument) != *(other_par_op->argument)) {
			return *(this->argument) < *(other_par_op->argument);
		}
	}
	return false;
}

bool bmath::intern::Par_Operator::operator==(const Basic_Term& other) const
{
	if (this->get_state_intern() != other.get_state_intern()) {
		return false;
	}
	else {
		const Par_Operator* other_par_op = static_cast<const Par_Operator*>(&other);
		if (this->op_state != other_par_op->op_state) {
			return false;
		}
		if (*(this->argument) != *(other_par_op->argument)) {
			return false;
		}
	}
	return true;
}
