#pragma once

#include "common.hpp"

struct fixed_point {
  union {
    s32 value;
    struct {
      u16 frac;
      s16 whole;
    };
  };

  constexpr fixed_point() : value(0) {}
  constexpr fixed_point(s32 v) : value(v) {}
  constexpr fixed_point(s16 w, u16 f) : frac(f), whole(w) {}

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

  constexpr s16 round() {
    if (whole >= 0) {
      if (frac >= 0x8000) {
        return whole + 1;
      } else {
        return whole;
      }
    } else {
      if (frac >= 0x8000) {
        return whole - 1;
      } else {
        return whole;
      }
    }
  }
};
