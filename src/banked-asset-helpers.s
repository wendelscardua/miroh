.section .prg_rom_last,"axR",@progbits

.global banked_oam_meta_spr

; void banked_oam_meta_spr(char x, int y, const void *data);
; A/rc6: x
; X/rc7: y.l
; rc2: y.h 
; rc4: data.l
; rc5: data.h

banked_oam_meta_spr:
	sta __rc6
	stx __rc7

    jsr get_prg_bank
    pha
    lda #mos24bank(metasprite_list)
    jsr set_prg_bank

	ldx SPRID
	ldy #0
1:
	lda (__rc4),y		;x offset
	cmp #$80
	beq 2f
	iny
	clc
	adc __rc6
	sta OAM_BUF+3,x
	lda (__rc4),y		;y offset
	iny
	clc
	adc __rc7
	sta OAM_BUF+0,x
	lda (__rc4),y		;tile
	iny
	sta OAM_BUF+1,x
	lda (__rc4),y		;attribute
	iny
	sta OAM_BUF+2,x
	inx
	inx
	inx
	inx
	jmp 1b
2:
	stx SPRID
    pla
    jsr set_prg_bank
    rts