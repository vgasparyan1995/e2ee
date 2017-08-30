#pragma once

#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <limits>
#include <stddef.h>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <vector>

class BigInteger
{
public:
    typedef uint8_t unit_t;
    typedef uint16_t big_unit_t;

    BigInteger();
    explicit BigInteger(const std::string& value);

    template <typename T>
    BigInteger(T value, typename std::enable_if<std::is_integral<T>::value>::type* = 0);

    template <typename T>
    T to_integral(typename std::enable_if<std::is_integral<T>::value>::type* = 0) const;

    BigInteger(const BigInteger& that) = default;
    BigInteger& operator= (const BigInteger& that) = default;
    BigInteger(BigInteger&& that) = default;
    BigInteger& operator= (BigInteger&& that) = default;
    ~BigInteger() = default;

    bool operator== (const BigInteger& rhs) const;
    bool operator!= (const BigInteger& rhs) const;
    bool operator<  (const BigInteger& rhs) const;
    bool operator>  (const BigInteger& rhs) const;
    bool operator<= (const BigInteger& rhs) const;
    bool operator>= (const BigInteger& rhs) const;

    BigInteger operator+ (const BigInteger& rhs) const;
    BigInteger operator- (const BigInteger& rhs) const;
    BigInteger operator-                      () const;
    BigInteger operator* (const BigInteger& rhs) const;
    BigInteger operator/ (const BigInteger& rhs) const;
    BigInteger operator% (const BigInteger& rhs) const;
    BigInteger operator^ (const BigInteger& rhs) const;

    BigInteger& operator+= (const BigInteger& that);
    BigInteger& operator-= (const BigInteger& that);
    BigInteger& operator*= (const BigInteger& that);
    BigInteger& operator/= (const BigInteger& that);
    BigInteger& operator%= (const BigInteger& that);
    BigInteger& operator- ();
    BigInteger& operator++ ();
    BigInteger& operator-- ();
    const BigInteger operator++ (int);
    const BigInteger operator-- (int);

    std::string to_string() const;
    friend std::ostream& operator<< (std::ostream& os, const BigInteger& n);

private:
    void set_unit(const size_t index, const unit_t unit_value);
    unit_t get_unit(const size_t index) const;
    void refresh();
    BigInteger complement(const size_t degree) const;
    void multiply_by(const uint64_t rhs);

    template <bool LESS, bool EQUAL>
    static bool compare(const BigInteger& lhs, const BigInteger& rhs);

private:
    using BaseDecoderResult = std::tuple<bool, int, std::string_view, std::function<int (char, bool&)> >;

    static BaseDecoderResult decodeBase(const std::string& value, bool& ok);
    static int charToInt2(char, bool& ok);
    static int charToInt8(char, bool& ok);
    static int charToInt10(char, bool& ok);
    static int charToInt16(char, bool& ok);

private:
    std::vector<unit_t> m_value;
    bool m_sign;
    static const unit_t max_unit_value = std::numeric_limits<unit_t>::max();
};

template <typename T>
BigInteger::BigInteger(T value, typename std::enable_if<std::is_integral<T>::value>::type*)
    : m_sign(value >= 0)
{
    for (int i = 0; i < sizeof(T) / sizeof(unit_t); ++i) {
        set_unit(i, value % ((uint32_t)max_unit_value + 1));
        value /= ((uint32_t)max_unit_value + 1);
    }
}

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) > (b) ? (b) : (a))

template <typename T>
T BigInteger::to_integral(typename std::enable_if<std::is_integral<T>::value>::type*) const
{
    T result = 0;
    for (int i = 0; i < min(sizeof(T) / sizeof(unit_t), m_value.size()); ++i) {
        result += std::pow(((uint32_t)max_unit_value + 1), i) * get_unit(i);
    }
    return m_sign ? result : -result;
}

template <bool LESS, bool EQUAL>
bool BigInteger::compare(const BigInteger& lhs, const BigInteger& rhs)
{
    if (lhs.m_sign && rhs.m_sign) {
        const size_t max_degree = max(lhs.m_value.size(), rhs.m_value.size());
        size_t degree = max_degree;
        while (degree != 0) {
            --degree;
            if (lhs.get_unit(degree) < rhs.get_unit(degree)) {
                return LESS;
            } else if (lhs.get_unit(degree) < rhs.get_unit(degree)) {
                return !LESS;
            }
        }
        return EQUAL;
    } else if (!lhs.m_sign && rhs.m_sign) {
        return LESS;
    } else if (lhs.m_sign && !rhs.m_sign) {
        return !LESS;
    } else {
        const auto max_degree = max(lhs.m_value.size(), rhs.m_value.size());
        auto degree = max_degree;
        while (degree != 0) {
            --degree;
            if (lhs.get_unit(degree) < rhs.get_unit(degree)) {
                return !LESS;
            } else if (lhs.get_unit(degree) < rhs.get_unit(degree)) {
                return LESS;
            }
        }
        return EQUAL;
    }
    return false;
}

#undef max
#undef min

namespace std {

template <>
struct hash<BigInteger>
{
    std::size_t operator() (const BigInteger& n) const
    {
        std::hash<uint64_t> hasher;
        return hasher(n.to_integral<uint64_t>());
    }
};

} // namespace std

