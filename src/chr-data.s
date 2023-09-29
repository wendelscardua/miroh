        .section .prg_rom_1,"a"

        .global bg_chr
bg_chr:
        .incbin "bg.chr.donut"

        .global sprites_chr
sprites_chr:
        .incbin "sprites.chr.donut"
