#pragma once

#include "common.hpp"

struct fixed_point {
  union {
    u16 value;
    struct {
      u8 frac;
      u8 whole;
    };
  };

  constexpr fixed_point() : value(0) {}
  constexpr fixed_point(u16 v) : value(v) {}
  constexpr fixed_point(u8 w, u8 f) : frac(f), whole(w) {}

  constexpr fixed_point operator+(const fixed_point rhs) const {
    return fixed_point(value + rhs.value);
  }
  constexpr fixed_point operator-(const fixed_point rhs) const {
    return fixed_point(value - rhs.value);
  }
  constexpr fixed_point operator-() const { return fixed_point(0) - (*this); }
  constexpr fixed_point operator*(const u8 rhs) {
    return fixed_point(value * rhs);
  }
  constexpr fixed_point operator/(const u8 rhs) {
    return fixed_point(value / rhs);
  }
  fixed_point &operator+=(const fixed_point rhs) {
    (*this) = (*this) + rhs;
    return *this;
  }
  fixed_point &operator-=(const fixed_point rhs) {
    (*this) = (*this) - rhs;
    return *this;
  }
  constexpr bool operator==(const fixed_point rhs) const {
    return value == rhs.value;
  }
  constexpr bool operator!=(const fixed_point rhs) const {
    return !(*this == rhs);
  }

  constexpr bool operator<(const fixed_point rhs) const {
    return value < rhs.value;
  }
  constexpr bool operator>(const fixed_point rhs) const { return rhs < *this; }
  constexpr bool operator<=(const fixed_point rhs) const {
    return !(*this > rhs);
  }
  constexpr bool operator>=(const fixed_point rhs) const {
    return !(*this < rhs);
  }

  constexpr u8 round() {
    if (frac >= 0x80) {
      return whole + 1;
    } else {
      return whole;
    }
  }
};
