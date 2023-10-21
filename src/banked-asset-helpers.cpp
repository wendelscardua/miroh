#include "banked-asset-helpers.hpp"
#include "bank-helper.hpp"
#include <bank.h>
#include <neslib.h>

void banked_oam_meta_spr(char x, char y, const void *data) {
  ScopedBank scopedBank(GET_BANK(metasprite_list));
  oam_meta_spr(x, y, data);
}