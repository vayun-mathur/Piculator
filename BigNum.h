#pragma once
#include <iostream>
#include <memory>

using word = unsigned long long;
using index = long long;

class BigNum
{
public:
	BigNum(index msb, index lsb);
	BigNum(word w, index msb, index lsb);
	BigNum(const BigNum& other, index msb, index lsb);
	BigNum(const BigNum& other);

	BigNum operator+(const BigNum& other) const;
	BigNum operator-(const BigNum& other) const;
	BigNum& operator+=(const BigNum& other);
	BigNum& operator-=(const BigNum& other);

	BigNum operator*(const BigNum& other) const;
	BigNum operator*(word other) const;
	BigNum& operator*=(const BigNum& other);
	BigNum& operator*=(word other);

	BigNum operator/(const BigNum& other) const;
	BigNum operator/(word other) const;
	BigNum& operator/=(const BigNum& other);
	BigNum& operator/=(word other);

	BigNum operator%(const BigNum& other) const;
	word operator%(word other) const;

	BigNum& operator++(); //prefix
	BigNum operator++(int); //postfix
	BigNum& operator--(); //prefix
	BigNum operator--(int); //postfix

	BigNum& operator=(word w);
	BigNum& operator=(const BigNum& other);

	BigNum operator&(const BigNum& other) const;
	BigNum operator|(const BigNum& other) const;
	BigNum operator^(const BigNum& other) const;
	BigNum& operator&=(const BigNum& other);
	BigNum& operator|=(const BigNum& other);
	BigNum& operator^=(const BigNum& other);
	BigNum operator~() const;

	BigNum operator<<(word shift_count) const;
	BigNum operator>>(word shift_count) const;
	BigNum& operator<<=(word shift_count);
	BigNum& operator>>=(word shift_count);

	bool operator&&(const BigNum& other) const;
	bool operator||(const BigNum& other) const;
	bool operator!() const;

	operator bool() const;

	bool operator<(const BigNum& other) const;
	bool operator>(const BigNum& other) const;
	bool operator<=(const BigNum& other) const;
	bool operator>=(const BigNum& other) const;
	bool operator==(const BigNum& other) const;
	bool operator!=(const BigNum& other) const;

	friend std::ostream& operator<<(std::ostream& os, const BigNum& ref);

	friend BigNum invsqrt(const BigNum& x);

private:
	word* arr;
	const index msb;
	const index lsb;
};