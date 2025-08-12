#pragma once

#include "common.hpp"
#include <mapper.h>
#include <type_traits>

class ScopedBank {
  u8 old_bank;

public:
  ScopedBank(u8 bank) {
    old_bank = get_prg_bank();
    set_prg_bank(bank);
  };

  ~ScopedBank() { set_prg_bank(old_bank); }
};

template <typename Func>
__attribute__((noinline, section(".prg_rom_fixed.text.banked_lambda"))) auto
banked_lambda(char bank_id, Func lambda) -> decltype(lambda()) {
  ScopedBank bank(bank_id);
  if constexpr (std::is_void_v<decltype(lambda())>) {
    lambda();
    return;
  } else {
    auto retval = lambda();
    return retval;
  }
}
