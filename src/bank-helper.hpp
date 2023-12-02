#pragma once

#include "common.hpp"
#include <mapper.h>
#define GET_BANK(symbol)                                                       \
  []() {                                                                       \
    register u8 bank asm("a");                                                 \
    asm("ld%0 #mos24bank(" #symbol ")\n" : "=r"(bank) : "r"(bank) : "a");      \
    return bank;                                                               \
  }()

template <typename Func>
__attribute__((noinline, section(".prg_rom_last.text"))) void
banked_lambda(char bank_id, Func lambda) {
  char old_bank = get_prg_bank();
  set_prg_bank(bank_id);
  lambda();
  set_prg_bank(old_bank);
}

class ScopedBank {
  u8 old_bank;

public:
  ScopedBank(u8 bank) {
    old_bank = get_prg_bank();
    set_prg_bank(bank);
  };

  ~ScopedBank() { set_prg_bank(old_bank); }
};