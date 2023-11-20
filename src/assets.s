    .section .prg_rom_1,"aR",@progbits

    .global level_bg_tiles
level_bg_tiles:
    .byte StarlitStablesBG_chr@mos16lo
    .byte RainbowRetreatBG_chr@mos16lo
    .byte FairyForestBG_chr@mos16lo
    .byte GlitteryGrottoBG_chr@mos16lo
    .byte MarshmallowMountainBG_chr@mos16lo
    .byte StarlitStablesBG_chr@mos16hi
    .byte RainbowRetreatBG_chr@mos16hi
    .byte FairyForestBG_chr@mos16hi
    .byte GlitteryGrottoBG_chr@mos16hi
    .byte MarshmallowMountainBG_chr@mos16hi

    .global level_bg_tile_blocks
level_bg_tile_blocks:
    .byte 41
    .byte 37
    .byte 0
    .byte 37
    .byte 41

    .global level_nametables
level_nametables:
    .byte StarlitStables_nam@mos16lo
    .byte RainbowRetreat_nam@mos16lo
    .byte FairyForest_nam@mos16lo
    .byte GlitteryGrotto_nam@mos16lo
    .byte MarshmallowMountain_nam@mos16lo
    .byte StarlitStables_nam@mos16hi
    .byte RainbowRetreat_nam@mos16hi
    .byte FairyForest_nam@mos16hi
    .byte GlitteryGrotto_nam@mos16hi
    .byte MarshmallowMountain_nam@mos16hi

StarlitStablesBG_chr: .incbin "StarlitStablesBG.suffix.chr.donut"
StarlitStables_nam: .incbin "StarlitStables.nam.zx02"

RainbowRetreatBG_chr: .incbin "RainbowRetreatBG.suffix.chr.donut"
RainbowRetreat_nam: .incbin "RainbowRetreat.nam.zx02"

FairyForestBG_chr: .incbin "FairyForestBG.chr.donut"
FairyForest_nam: .incbin "FairyForest.nam.zx02"

GlitteryGrottoBG_chr: .incbin "GlitteryGrottoBG.suffix.chr.donut"
GlitteryGrotto_nam: .incbin "GlitteryGrotto.nam.zx02"

MarshmallowMountainBG_chr: ;.incbin "MarshmallowMountainBG.suffix.chr.donut"
MarshmallowMountain_nam: ;.incbin "MarshmallowMountain.nam.zx02"

.global title_bg_tiles, title_nametable

title_bg_tiles: .incbin "TitleBG.suffix.chr.donut"
title_nametable: .incbin "Title.nam.zx02"

.global map_nametable

map_nametable: .incbin "Map.nam.zx02"

    ; Generic
.global base_bg_tiles
base_bg_tiles = FairyForestBG_chr

.global spr_tiles
spr_tiles: .incbin "SPR.chr.donut"

.global level_label_tiles
level_label_tiles: .incbin "Level.chr.donut"
.global time_label_tiles
time_label_tiles: .incbin "Time.chr.donut"

.global starlit_level_label_tiles
starlit_level_label_tiles: .incbin "LevelStarlit.chr.donut"
.global starlit_time_label_tiles
starlit_time_label_tiles: .incbin "TimeStarlit.chr.donut"

.global spare_characters
spare_characters: .incbin "SpareCharacters.chr.donut"

.global time_trial_prompt
time_trial_prompt:
	.byte $1d,$11,$19,$02,$0f,$04,$10,$1b,$02,$12,$11,$0c,$10,$16,$15,$02,$06,$04,$10,$02
	.byte $02,$1b,$11,$17,$02,$14,$04,$06,$0d,$02,$17,$12,$02,$05,$08,$09,$11,$14,$08,$02
	.byte $02,$02,$16,$0c,$0f,$08,$02,$14,$17,$10,$15,$02,$11,$17,$16,$3f,$02,$02,$02,$02

.global endless_prompt
endless_prompt:
	.byte $02,$02,$1d,$11,$19,$02,$0e,$11,$10,$0a,$02,$06,$04,$10,$02,$1b,$11,$17,$02,$02
	.byte $15,$17,$14,$18,$0c,$18,$08,$02,$16,$0b,$0c,$15,$02,$15,$17,$0a,$04,$14,$1b,$02
	.byte $02,$0a,$08,$0e,$04,$16,$0c,$10,$02,$05,$04,$14,$14,$04,$0a,$08,$3f,$2f,$02,$02

    .section .prg_rom_2,"aR",@progbits

    .global level_bg_palettes
level_bg_palettes:
    .byte StarlitStablesBG_pal@mos16lo
    .byte RainbowRetreatBG_pal@mos16lo
    .byte FairyForestBG_pal@mos16lo
    .byte GlitteryGrottoBG_pal@mos16lo
    .byte MarshmallowMountainBG_pal@mos16lo
    .byte StarlitStablesBG_pal@mos16hi
    .byte RainbowRetreatBG_pal@mos16hi
    .byte FairyForestBG_pal@mos16hi
    .byte GlitteryGrottoBG_pal@mos16hi
    .byte MarshmallowMountainBG_pal@mos16hi

    .global level_spr_palettes
level_spr_palettes:
    .byte StarlitStablesSPR_pal@mos16lo
    .byte RainbowRetreatSPR_pal@mos16lo
    .byte FairyForestSPR_pal@mos16lo
    .byte GlitteryGrottoSPR_pal@mos16lo
    .byte MarshmallowMountainSPR_pal@mos16lo
    .byte StarlitStablesSPR_pal@mos16hi
    .byte RainbowRetreatSPR_pal@mos16hi
    .byte FairyForestSPR_pal@mos16hi
    .byte GlitteryGrottoSPR_pal@mos16hi
    .byte MarshmallowMountainSPR_pal@mos16hi

StarlitStablesBG_pal: .incbin "StarlitStablesBG.pal"
StarlitStablesSPR_pal: .incbin "StarlitStablesSPR.pal"

RainbowRetreatBG_pal: .incbin "RainbowRetreatBG.pal"
RainbowRetreatSPR_pal: .incbin "RainbowRetreatSPR.pal"

FairyForestBG_pal: .incbin "FairyForestBG.pal"
FairyForestSPR_pal: .incbin "FairyForestSPR.pal"

GlitteryGrottoBG_pal: .incbin "GlitteryGrottoBG.pal"
GlitteryGrottoSPR_pal: .incbin "GlitteryGrottoSPR.pal"

MarshmallowMountainBG_pal: ;.incbin "MarshmallowMountainBG.pal"
MarshmallowMountainSPR_pal: .incbin "MarshmallowMountainSPR.pal"

.global title_bg_palette, title_spr_palette
title_bg_palette: .incbin "TitleBG.pal"
title_spr_palette: .incbin "TitleSPR.pal"

.global intro_text_nametable
intro_text_nametable: .incbin "IntroText.nam.zx02"
