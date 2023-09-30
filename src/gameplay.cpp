
#include <bank.h>
#include <nesdoug.h>
#include <neslib.h>

#include "bank-helper.hpp"

#include "chr-data.hpp"
#include "common.hpp"
#include "donut.hpp"
#include "fixed-point.hpp"
#include "nametables.hpp"
#include "palettes.hpp"
#include "gameplay.hpp"

Gameplay::Gameplay() :
  board(0x20, 0x20),
  player(board, fixed_point(0x50, 0x00), fixed_point(0x50, 0x00)) {
    set_chr_bank(0);

    set_prg_bank(GET_BANK(bg_chr));
    vram_adr(PPU_PATTERN_TABLE_0);
    Donut::decompress_to_ppu((void *)&bg_chr, PPU_PATTERN_TABLE_SIZE / 64);

    set_prg_bank(GET_BANK(sprites_chr));
    vram_adr(PPU_PATTERN_TABLE_1);
    Donut::decompress_to_ppu((void *)&sprites_chr, PPU_PATTERN_TABLE_SIZE / 64);

    set_prg_bank(GET_BANK(title_nam));

    vram_adr(NAMETABLE_A);
    vram_write(gameplay_nam, 1024);

    board.render();

    set_prg_bank(GET_BANK(bg_palette));
    pal_bg(bg_palette);
    set_prg_bank(GET_BANK(sprites_palette));
    pal_spr(sprites_palette);

    pal_bright(0);

    ppu_on_all();

    pal_fade_to(0, 4);
}

Gameplay::~Gameplay() {
    pal_fade_to(4, 0);
    ppu_off();
}

void Gameplay::render() {
  oam_clear();
  player.render();
}

void Gameplay::loop() {
  while(current_mode == GameMode::Gameplay) {
    ppu_wait_nmi();
    pad_poll(0);

    u8 pressed = get_pad_new(0);
    u8 held = pad_state(0);

    player.update(pressed, held);

    render();
  }
}
