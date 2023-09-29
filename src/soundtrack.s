        .section .prg_rom_2,"aR",@progbits
        .include "ggsound.inc"

        .global song_list
        .global sfx_list
        .global instrument_list

        .include "soundtrack.asm"
