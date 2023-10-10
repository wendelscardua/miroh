#include "banked-asset-helpers.hpp"
#include "bank-helper.hpp"
#include "common.hpp"
#include <bank.h>
#include <neslib.h>

void banked_oam_meta_spr(char x, char y, const void *data) {
  u8 old_bank = get_prg_bank();
  u8 bank = GET_BANK(metasprite_list);

  if (bank != old_bank) set_prg_bank(bank);

  oam_meta_spr(x, y, data);

  if (bank != old_bank) set_prg_bank(old_bank);

}
