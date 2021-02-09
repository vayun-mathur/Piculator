#pragma once
using word = unsigned long long;
using index = long long;

const index msb = 1<<8;

class BigInteger
{
public:
	BigInteger();
	BigInteger(word w);
	BigInteger(const BigInteger& other);

	BigInteger operator+(const BigInteger& other) const;
	BigInteger operator-(const BigInteger& other) const;
	BigInteger& operator+=(const BigInteger& other);
	BigInteger& operator-=(const BigInteger& other);

	BigInteger operator*(const BigInteger& other) const;
	BigInteger operator*(word other) const;
	BigInteger& operator*=(const BigInteger& other);
	BigInteger& operator*=(word other);

	BigInteger operator/(const BigInteger& other) const;
	BigInteger operator/(word other) const;
	BigInteger& operator/=(const BigInteger& other);
	BigInteger& operator/=(word other);

	BigInteger operator%(const BigInteger& other) const;
	word operator%(word other) const;

	BigInteger& operator++(); //prefix
	BigInteger operator++(int); //postfix
	BigInteger& operator--(); //prefix
	BigInteger operator--(int); //postfix

	BigInteger& operator=(word w);
	BigInteger& operator=(const BigInteger& other);

	BigInteger operator&(const BigInteger& other) const;
	BigInteger operator|(const BigInteger& other) const;
	BigInteger operator^(const BigInteger& other) const;
	BigInteger& operator&=(const BigInteger& other);
	BigInteger& operator|=(const BigInteger& other);
	BigInteger& operator^=(const BigInteger& other);
	BigInteger operator~() const;

	BigInteger operator<<(word shift_count) const;
	BigInteger operator>>(word shift_count) const;
	BigInteger& operator<<=(word shift_count);
	BigInteger& operator>>=(word shift_count);

	bool operator&&(const BigInteger& other) const;
	bool operator||(const BigInteger& other) const;
	bool operator!() const;

	operator bool() const;

	bool operator<(const BigInteger& other) const;
	bool operator>(const BigInteger& other) const;
	bool operator<=(const BigInteger& other) const;
	bool operator>=(const BigInteger& other) const;
	bool operator==(const BigInteger& other) const;
	bool operator!=(const BigInteger& other) const;

private:
	word* arr;
};
