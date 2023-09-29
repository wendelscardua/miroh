        .section .prg_rom_1,"a"

        .global bg_palette
bg_palette:
        .incbin "bg.pal"

        .global sprites_palette
sprites_palette:
        .incbin "sprites.pal"
