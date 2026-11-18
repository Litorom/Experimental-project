#pragma once
#include <algorithm>
#include <iostream>
#include <string>
#include <utility>

// Exact nonnegative integers, stored as canonical decimal strings.
class Number {
    std::string digits_;

    explicit Number(std::string digits) : digits_(std::move(digits)) {
        const auto first = digits_.find_first_not_of('0');
        digits_ = first == std::string::npos ? "0" : digits_.substr(first);
    }

public:
    Number(unsigned value = 0) : digits_(std::to_string(value)) {}

    bool operator<(const Number& other) const {
        if (digits_.size() != other.digits_.size())
            return digits_.size() < other.digits_.size();
        return digits_ < other.digits_;
    }

    Number operator*(unsigned factor) const {
        std::string result;
        unsigned long long carry = 0;
        for (auto digit = digits_.rbegin(); digit != digits_.rend(); ++digit) {
            carry += static_cast<unsigned long long>(*digit - '0') * factor;
            result.push_back(char('0' + carry % 10));
            carry /= 10;
        }
        while (carry) {
            result.push_back(char('0' + carry % 10));
            carry /= 10;
        }
        std::reverse(result.begin(), result.end());
        return Number(std::move(result));
    }

    Number operator+(const Number& other) const {
        return combine(other, false);
    }

    // Subtraction requires *this >= other.
    Number operator-(const Number& other) const {
        return combine(other, true);
    }

    bool isBossFloor() const {
        return digits_.back() == '0' || digits_.back() == '5';
    }

    friend std::ostream& operator<<(std::ostream& out, const Number& number) {
        return out << number.digits_;
    }

    friend std::istream& operator>>(std::istream& in, Number& number) {
        std::string digits;
        if (in >> digits) {
            if (digits.find_first_not_of("0123456789") != std::string::npos)
                in.setstate(std::ios::failbit);
            else
                number = Number(std::move(digits));
        }
        return in;
    }

private:
    Number combine(const Number& other, bool subtract) const {
        auto left = digits_.rbegin();
        auto right = other.digits_.rbegin();
        std::string result;
        int carry = 0;
        while (left != digits_.rend() || right != other.digits_.rend()) {
            const int a = left != digits_.rend() ? *left++ - '0' : 0;
            const int b = right != other.digits_.rend() ? *right++ - '0' : 0;
            int total = a + (subtract ? -b : b) + carry;
            carry = total < 0 ? -1 : total / 10;
            if (total < 0) total += 10;
            result.push_back(char('0' + total % 10));
        }
        if (carry > 0) result.push_back('1');
        std::reverse(result.begin(), result.end());
        return Number(std::move(result));
    }
};