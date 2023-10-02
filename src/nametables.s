        .section .prg_rom_1,"a"
        .global title_nam
title_nam: .incbin "title.nam"
        .global gameplay_nam
gameplay_nam: .incbin "gameplay.nam"
        .global game_over_nam
game_over_nam: .incbin "game-over.nam"
