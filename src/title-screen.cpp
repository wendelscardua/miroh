#include "title-screen.hpp"

#include <bank.h>
#include <nesdoug.h>
#include <neslib.h>

#include "bank-helper.hpp"
#include "common.hpp"
#include "donut.hpp"

#include "chr-data.hpp"
#include "nametables.hpp"
#include "palettes.hpp"

namespace TitleScreen {
  using Donut::decompress_to_ppu;

  void init() {
    set_chr_bank(0);

    set_prg_bank(GET_BANK(bg_chr));
    vram_adr(PPU_PATTERN_TABLE_0);
    decompress_to_ppu((void *)&bg_chr, PPU_PATTERN_TABLE_SIZE / 64);

    set_prg_bank(GET_BANK(sprites_chr));
    vram_adr(PPU_PATTERN_TABLE_1);
    decompress_to_ppu((void *)&sprites_chr, PPU_PATTERN_TABLE_SIZE / 64);

    set_prg_bank(GET_BANK(title_nam));

    vram_adr(NAMETABLE_A);
    vram_write(title_nam, 1024);

    set_prg_bank(GET_BANK(bg_palette));
    pal_bg(bg_palette);
    set_prg_bank(GET_BANK(sprites_palette));
    pal_spr(sprites_palette);

    pal_bright(0);

    ppu_on_all();

    pal_fade_to(0, 4);
  }

  void deinit() {
    pal_fade_to(4, 0);
    ppu_off();
  }

  void update() { u8 pressed = get_pad_new(0); }
} // namespace TitleScreen
