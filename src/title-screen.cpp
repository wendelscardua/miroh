
#include <bank.h>
#include <nesdoug.h>
#include <neslib.h>

#include "bank-helper.hpp"

#include "chr-data.hpp"
#include "common.hpp"
#include "donut.hpp"
#include "nametables.hpp"
#include "palettes.hpp"
#include "title-screen.hpp"

TitleScreen::TitleScreen() {
    set_chr_bank(0);

    set_prg_bank(GET_BANK(bg_chr));
    vram_adr(PPU_PATTERN_TABLE_0);
    Donut::decompress_to_ppu((void *)&bg_chr, PPU_PATTERN_TABLE_SIZE / 64);

    set_prg_bank(GET_BANK(sprites_chr));
    vram_adr(PPU_PATTERN_TABLE_1);
    Donut::decompress_to_ppu((void *)&sprites_chr, PPU_PATTERN_TABLE_SIZE / 64);

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

TitleScreen::~TitleScreen() {
    pal_fade_to(4, 0);
    ppu_off();
}

void TitleScreen::loop() {
  while(current_mode == GameMode::TitleScreen) {
    ppu_wait_nmi();
    pad_poll(0);

    rand16();

    u8 pressed = get_pad_new(0);
    if (pressed & (PAD_START | PAD_A)) {
      current_mode = GameMode::Gameplay;
    }
  }
}
