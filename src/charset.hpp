#include "log.hpp"
#if __cplusplus < 202002L
#error charset support requies C++20
#else
#ifndef _CHARSET_H
#define _CHARSET_H

#include <cstddef>

namespace charset_impl {

  template <size_t N> struct TileString {
    char Str[N]{};

    constexpr TileString(char const (&Src)[N]) {
      for (size_t I = 0, J = 0; I < N; ++I) {
        fake_assert(Src[I] < 0x80);
        auto new_char = TranslateUnicode(Src[I]);
        Str[J++] = new_char;
      }
    }

    constexpr TileString(char16_t const (&Src)[N]) {
      for (size_t I = 0, J = 0; I < N; ++I) {
        fake_assert(Src[I] < 0xD800 || Src[I] > 0xDFFF);
        auto new_char = TranslateUnicode(Src[I]);
        Str[J++] = new_char;
      }
    }

    constexpr TileString(char32_t const (&Src)[N]) {
      for (size_t I = 0, J = 0; I < N; ++I) {
        auto new_char = TranslateUnicode(Src[I]);
        Str[J++] = new_char;
      }
    }

    constexpr char TranslateUnicode(char32_t C) {
      switch (C) {
      default:
        fake_assert(false);
        return 0xff;

      // C0 control codes are uninterpreted.
      case 0x0000 ... 0x001f:
        return (char)C;

      case U' ':
        return 0x02;
      case U'!':
        return 0x2f;
      case U'?':
        return 0x3f;
      case U'-':
        return 0x1c;
      case U'0' ... U'9':
        return (char)(C - U'0' + 0x64);
      case U'a' ... U'i':
        return (char)(C - U'a' + 0x04);
      case U'k' ... U'y':
        return (char)(C - U'k' + 0x0d);
      case U'B':
        return 0x1f;
      case U'H':
        return 0x1d;
      case U'P':
        return 0x1e;
      case U'G':
      case U'F':
      case U'M':
      case U'R':
      case U'S':
        return 0x6f;
      }
    }
  };

} // namespace charset_impl

// Converts strings to tiles
template <charset_impl::TileString S> constexpr auto operator""_ts() {
  return S.Str;
}

#endif // not _CHARSET_H
#endif // __cplusplus >= 202002L
