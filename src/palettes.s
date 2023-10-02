        .section .prg_rom_1,"a"

        .global bg_palette
bg_palette:
        .incbin "bg.pal"

        .global sprites_player_palette
sprites_player_palette:
        .incbin "sprites-player.pal"

        .global sprites_polyomino_palette
sprites_polyomino_palette:
        .incbin "sprites-polyomino.pal"
