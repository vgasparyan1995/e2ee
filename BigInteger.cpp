#include <stdexcept>
#include <iterator>
#include <algorithm>

#include "BigInteger.h"

BigInteger::BigInteger()
    : m_sign(true)
{}

BigInteger::BigInteger(const std::string& value)
    : m_sign(true)
{
    BigInteger tmp;
    bool apply = true;
    auto [ sign, base, digits, converter ] = decodeBase(value, apply);
    if (!apply) {
        return;
    }

    for (const auto d : digits) {
		tmp *= base;
		tmp += converter(d, apply);
		if (!apply) {
		    return;
		}
    }
	m_sign = sign;
    if (apply) {
        *this = std::move(tmp);
    }
}

bool BigInteger::operator== (const BigInteger& rhs) const
{
    return m_sign == rhs.m_sign
        && m_value == rhs.m_value;
}

bool BigInteger::operator!= (const BigInteger& rhs) const
{
    return !(*this == rhs);
}

bool BigInteger::operator< (const BigInteger& rhs) const
{
    return compare<true, false>(*this, rhs);
}

bool BigInteger::operator> (const BigInteger& rhs) const
{
    return compare<false, false>(*this, rhs);
}

bool BigInteger::operator<= (const BigInteger& rhs) const
{
    return compare<true, true>(*this, rhs);
}

bool BigInteger::operator>= (const BigInteger& rhs) const
{
    return compare<false, true>(*this, rhs);
}

BigInteger BigInteger::operator+ (const BigInteger& rhs) const
{
    BigInteger result;

    const auto& lhs = *this;
    if ((lhs.m_sign && rhs.m_sign) || (!lhs.m_sign && !rhs.m_sign)) {
        const auto max_degree = std::max(lhs.m_value.size(), rhs.m_value.size());
        result.m_value.reserve(max_degree);
        big_unit_t remainder = 0;
        for (int i = 0; i < max_degree; ++i) {
            big_unit_t tmp = big_unit_t(lhs.get_unit(i)) +
                big_unit_t(rhs.get_unit(i)) +
                remainder;
            result.set_unit(i, tmp % ((uint32_t)max_unit_value + 1));
            remainder = tmp / ((uint32_t)max_unit_value + 1);
        }
        if (remainder != 0) {
            result.set_unit(max_degree, remainder % ((uint32_t)max_unit_value + 1));
        }
        result.m_sign = lhs.m_sign;
    } else if (lhs.m_sign && !rhs.m_sign) {
        return lhs - (-rhs);
    } else if (!lhs.m_sign && rhs.m_sign) {
        return rhs - (-lhs);
    }
    
    result.refresh();
    return std::move(result);
}

BigInteger BigInteger::operator- (const BigInteger& rhs) const
{
    BigInteger result;

    const auto& lhs = *this;
    if ((lhs.m_sign && rhs.m_sign) || (!lhs.m_sign && !rhs.m_sign)) {
        if (!lhs.m_sign) {
            result = (-lhs) - (-rhs);
            result.m_sign = false;
        } else {
            if (lhs < rhs) {
                result = rhs - lhs;
            } else {
                const auto max_degree = lhs.m_value.size();
                result = lhs + rhs.complement(max_degree) + 1;
                result.set_unit(max_degree, 0);
                result.m_sign = true;
            }
        }
    } else if (lhs.m_sign && !rhs.m_sign) {
        return lhs + (-rhs);
    } else if (!lhs.m_sign && rhs.m_sign) {
        return rhs + (-lhs);
    }
    
    result.refresh();
    return std::move(result);
}

BigInteger BigInteger::operator- () const
{
    BigInteger result(*this);
    if (result != 0) {
        result.m_sign = !result.m_sign;
    }
    return std::move(result);
}

BigInteger BigInteger::operator* (const BigInteger& rhs) const
{
    const auto& lhs = *this;
    if (lhs == 0 || rhs == 0) {
        return 0;
    }

    uint64_t lhs_right = 0;
    uint64_t rhs_right = 0;
    const auto uint_size = sizeof(uint32_t) / sizeof(unit_t);
    for (int i = 0; i < uint_size; ++i) {
        lhs_right += (uint64_t)lhs.get_unit(i) * (uint64_t)std::pow((uint32_t)max_unit_value + 1, i);
        rhs_right += (uint64_t)rhs.get_unit(i) * (uint64_t)std::pow((uint32_t)max_unit_value + 1, i);
    }
    BigInteger lhs_left;
    BigInteger rhs_left;
    if (lhs.m_value.size() > uint_size) {
        lhs_left.m_value.assign(lhs.m_value.begin() + uint_size, lhs.m_value.end());
    }
    if (rhs.m_value.size() > uint_size) {
        rhs_left.m_value.assign(rhs.m_value.begin() + uint_size, rhs.m_value.end());
    }
    
    BigInteger tmp_third_part = lhs_left * rhs_left;
    BigInteger third_part;
    third_part.m_value.reserve(2 * uint_size + tmp_third_part.m_value.size());
    for (int i = 0; i < 2 * uint_size; ++i) {
        third_part.m_value.push_back(0);
    }
    std::copy(tmp_third_part.m_value.begin(), tmp_third_part.m_value.end(), std::back_inserter(third_part.m_value));
    third_part.refresh();

    lhs_left.multiply_by(rhs_right);
    rhs_left.multiply_by(lhs_right);
    BigInteger tmp_second_part = lhs_left + rhs_left;
    BigInteger second_part;
    second_part.m_value.reserve(uint_size + tmp_second_part.m_value.size());
    for (int i = 0; i < uint_size; ++i) {
        second_part.m_value.push_back(0);
    }
    std::copy(tmp_second_part.m_value.begin(), tmp_second_part.m_value.end(), std::back_inserter(second_part.m_value));
    second_part.refresh();

    BigInteger first_part(lhs_right * rhs_right);
    
    BigInteger result = first_part + second_part + third_part;
    result.m_sign = (lhs.m_sign == rhs.m_sign);
    result.refresh();
    return std::move(result);
}

BigInteger BigInteger::operator/ (const BigInteger& rhs) const
{
    if (rhs == 0) {
        throw std::overflow_error("Divide by zero error.");
    }
    const auto& lhs = *this;
    if (!lhs.m_sign && rhs.m_sign) {
        return -((-lhs) / rhs);
    } else if (lhs.m_sign && !rhs.m_sign) {
        return -(lhs / (-rhs));
    } else if (!lhs.m_sign && !rhs.m_sign) {
        return (-lhs) / (-rhs);
    }

    BigInteger quotient;
    BigInteger remainder;
    for (int i = lhs.m_value.size() * 8 - 1; i >= 0; --i) {
        remainder *= 2;
        remainder.set_unit(0, remainder.get_unit(0) + ((lhs.get_unit(i / 8) / (unit_t)std::pow(2, i % 8)) % 2));
        if (remainder >= rhs) {
            remainder -= rhs;
            quotient.set_unit(i / 8, quotient.get_unit(i / 8) + (unit_t)std::pow(2, i % 8));
        }
    }
    quotient.refresh();
    return std::move(quotient);
}

BigInteger BigInteger::operator% (const BigInteger& rhs) const
{
    if (rhs == 0) {
        throw std::overflow_error("Divide by zero error.");
    }
    const auto& lhs = *this;
    if (!lhs.m_sign && rhs.m_sign) {
        return -((-lhs) % rhs);
    } else if (lhs.m_sign && !rhs.m_sign) {
        return -(lhs % (-rhs));
    } else if (!lhs.m_sign && !rhs.m_sign) {
        return (-lhs) % (-rhs);
    }

    BigInteger quotient;
    BigInteger remainder;
    for (int i = lhs.m_value.size() * 8 - 1; i >= 0; --i) {
        remainder *= 2;
        remainder.set_unit(0, remainder.get_unit(0) + ((lhs.get_unit(i / 8) / (unit_t)std::pow(2, i % 8)) % 2));
        if (remainder >= rhs) {
            remainder -= rhs;
            quotient.set_unit(i / 8, quotient.get_unit(i / 8) + (unit_t)std::pow(2, i % 8));
        }
    }
    remainder.refresh();
    return std::move(remainder);
}

BigInteger BigInteger::operator^ (const BigInteger& rhs) const
{
    BigInteger result(1);
    for (BigInteger i = 0; i < rhs; ++i) {
        if (rhs.m_sign) {
            result *= *this;
        } else {
            result /= *this;
        }
    }
    return result;
}

BigInteger& BigInteger::operator+= (const BigInteger& that)
{
    *this = *this + that;
    return *this;
}

BigInteger& BigInteger::operator-= (const BigInteger& that)
{
    *this = *this - that;
    return *this;
}

BigInteger& BigInteger::operator*= (const BigInteger& that)
{
    *this = *this * that;
    return *this;
}

BigInteger& BigInteger::operator/= (const BigInteger& that)
{
    *this = *this / that;
    return *this;
}

BigInteger& BigInteger::operator%= (const BigInteger& that)
{
    *this = *this % that;
    return *this;
}

BigInteger& BigInteger::operator- ()
{
    m_sign = !m_sign;
    return *this;
}

BigInteger& BigInteger::operator++ ()
{
    return *this += 1;
}

BigInteger& BigInteger::operator-- ()
{
    *this -= 1;
    refresh();
    return *this;
}

const BigInteger BigInteger::operator++ (int)
{
    auto tmp = *this;
    ++(*this);
    return std::move(tmp);
}

const BigInteger BigInteger::operator-- (int)
{
    auto tmp = *this;
    --(*this);
    refresh();
    return std::move(tmp);
}

std::string BigInteger::to_string() const
{
    std::string result;
    BigInteger tmp(*this);
    if (tmp == 0) {
        result.push_back('0');
    } else {
        while (tmp != 0) {
            result.push_back(int8_t((tmp % 10).to_integral<int8_t>() + 48));
            tmp /= 10;
        }
        if (!m_sign) {
            result.push_back('-');
        }
    }
    std::reverse(result.begin(), result.end());
    return result;
}

std::ostream& operator<< (std::ostream& os, const BigInteger& n)
{
    os << n.to_string();
    return os;
}

void BigInteger::set_unit(const size_t index, const unit_t unit_value)
{
    if (m_value.size() > index) {
        m_value[index] = unit_value;
    } else {
        if (unit_value != 0) {
            m_value.resize(index + 1);
            m_value[index] = unit_value;
        }
    }
}

BigInteger::unit_t BigInteger::get_unit(const size_t index) const
{
    if (m_value.size() > index) {
        return m_value[index];
    } else {
        return 0;
    }
}

void BigInteger::refresh()
{
    int index = m_value.size() - 1;
    while (index >= 0 && m_value[index] == 0) {
        --index;
    }
    m_value.resize(index + 1);
}

BigInteger BigInteger::complement(const size_t degree) const
{
    BigInteger result;
    result.m_value.resize(degree);
    for (int i = 0; i < degree; ++i) {
        result.set_unit(i, max_unit_value - get_unit(i));
    }
    return std::move(result);
}

void BigInteger::multiply_by(const uint64_t rhs)
{
    BigInteger result;
    for (int i = 0; i < m_value.size(); ++i) {
        uint64_t sub_result = rhs * get_unit(i);
        BigInteger tmp;
        for (int j = i; j < i + sizeof(uint64_t) / sizeof(unit_t); ++j) {
            tmp.set_unit(j, sub_result % ((int)max_unit_value + 1));
            sub_result /= ((int)max_unit_value + 1);
        }
        result += tmp;
    }
    result.refresh();
    *this = result;
}

BigInteger::BaseDecoderResult BigInteger::decodeBase(const std::string& value, bool& ok)
{
    auto result = std::make_tuple(true, 0, std::string_view(value.c_str()), std::function<int (char, bool&)>() );
    auto valueView = std::string_view(value.c_str());
    if (valueView.empty()) {
        ok = false;
        return result;
    }
    if (valueView[0] == '-') {
        std::get<0>(result) = false;
        valueView.remove_prefix(1);
        if (valueView.empty()) {
            ok = false;
            return result;
        }
    }
    if (valueView[0] == '0') {
        valueView.remove_prefix(1);
        if (valueView.empty()) {
            return result;
        }

        if (valueView[0] == 'b') {
            valueView.remove_prefix(1);
            std::get<1>(result) = 2;
            std::get<3>(result) = &BigInteger::charToInt2;
        } else if (valueView[0] == 'x') {
            valueView.remove_prefix(1);
            std::get<1>(result) = 16;
            std::get<3>(result) = &BigInteger::charToInt16;
        } else {
            std::get<1>(result) = 8;
            std::get<3>(result) = &BigInteger::charToInt8;
        }
    } else {
        std::get<1>(result) = 10;
        std::get<3>(result) = &BigInteger::charToInt10;
    }
    if (valueView.empty()) {
        ok = false;
        return result;
    }
    std::get<2>(result) = valueView;
    return result;
}

int BigInteger::charToInt2(char digit, bool& ok)
{
    switch (digit) {
    case '0':
        return 0;
    case '1':
        return 1;
    default:
        ok = false;
    }
    return 0;
}

int BigInteger::charToInt8(char digit, bool& ok)
{
    if (digit >= '0' && digit <= '7') {
        return static_cast<int>(digit - '0');
    }
    ok = false;
    return 0;
}

int BigInteger::charToInt10(char digit, bool& ok)
{
    if (digit >= '0' && digit <= '9') {
        return static_cast<int>(digit - '0');
    }
    ok = false;
    return 0;
}

int BigInteger::charToInt16(char digit, bool& ok)
{
    if (digit >= '0' && digit <= '9') {
        return static_cast<int>(digit - '0');
    } else if (digit >= 'a' && digit <= 'f') {
        return static_cast<int>(10 + digit - 'a');
    } else if (digit >= 'A' && digit <= 'F') {
        return static_cast<int>(10 + digit - 'A');
    }
    ok = false;
    return 0;
}
