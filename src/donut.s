;;; "Donut", NES CHR codec decompressor,
;;; Copyright (c) 2018  Johnathan Roatch
;;;
;;; Copying and distribution of this file, with or without
;;; modification, are permitted in any medium without royalty provided
;;; the copyright notice and this notice are preserved in all source
;;; code copies.  This file is offered as-is, without any warranty.
;;;
;;; Version History:
;;; 2019-10-23: Slight API change to decompress_block. It returns
;;;             bytes read in Y instead of adding that to stream_ptr.
;;; 2019-02-15: Swapped the M and L bits, for conceptual consistency.
;;;             Also rearranged branches for speed.
;;; 2019-02-07: Removed "Duplicate" block type, and moved
;;;             Uncompressed block to below 0xc0 to make room
;;;             for block handling commands in the 0xc0~0xff space
;;; 2018-09-29: Removed block option of XORing with existing block
;;;             for extra speed in decoding.
;;; 2018-08-13: Changed the format of raw blocks to not be reversed.
;;;             Register X is now an argument for the buffer offset.
;;; 2018-04-30: Initial release.
;;;
;;; Wendel Scardua tweaks:
;;; 2023-09-17: Ported to llvm-mos
;;; 2023-07-16: Fix buffer offset; donut was using only the $40 bytes *past* it
;;; 2022-09-13: Don't use fixed memory locations, add support for C code

        .global _asm_donut_decompress_to_ppu

        .section .bss
donut_block_buffer: .zero 64

        .section .zp
donut_stream_ptr:       .zero 2
temp: .zero 16

        .section .prg_rom_last,"a"

        ;; void _asm_donut_decompress_to_ppu(void *stream_ptr, char num_blocks)
        ;; decompress num_blocks * 64 bytes from stream_ptr to the PPU
        ;; (remember to turn off rendering before using)
_asm_donut_decompress_to_ppu:
        ;; stream_ptr is on rc2,rc3 and num_blocks is in A
        tax
        lda mos8(__rc2)
        sta donut_stream_ptr
        lda mos8(__rc3)
        sta donut_stream_ptr+1
        jmp donut_bulk_load_x

        ;; void donut_decompress_to_ppu(int num_blocks);
        ;; decompress NUM_BLOCKS*64 bytes from stream pointed by donut_stream_ptr
        ;; to current PPU address (remember to turn off rendering before using)
_donut_decompress_to_ppu:
        tax
        jmp donut_bulk_load_x

        ;;
        ;; Decompress X*64 bytes starting at AAYY to the NES PPU via $2007 PPU_DATA
        ;; Assumes The PPU is in forced blank, and $2006 is loaded with the desired address
        ;;
        ;; Trashes A, X, Y, temp 0 ~ temp 16.
donut_bulk_load_ayx:
        sty donut_stream_ptr+0
        sta donut_stream_ptr+1
        ;;,; jmp donut_bulk_load_x
donut_bulk_load_x:
        PPU_DATA = $2007
        block_count = temp+15
        stx block_count
block_loop:
        ldx #64
        jsr donut_decompress_block
        bcs end_block_upload  ; bail on error.
        ldx #64
upload_loop:
        lda donut_block_buffer-64, x
        sta PPU_DATA
        inx
        bpl upload_loop
        tya
        ;;,; clc
        adc donut_stream_ptr+0
        sta donut_stream_ptr+0
        bcc add_stream_ptr_no_inc_high_byte
        inc donut_stream_ptr+1
add_stream_ptr_no_inc_high_byte:
        dec block_count
        bne block_loop
end_block_upload:
        rts

        ;;
        ;; donut_decompress_block
        ;;
        ;; Decompresses a single variable sized block pointed to by donut_stream_ptr
        ;; Outputing 64 bytes to donut_block_buffer offsetted by the X register.
        ;;
        ;; Carry flag is cleared on success and set on failure.
        ;; The returned Y register is the number of input bytes read. (0 on failure)
        ;; and 64 will be added to the X register. (unchanged on failure)
        ;;
        ;; Block header:
        ;; LMlmbbBR
        ;; |||||||+-- Rotate plane bits (135° reflection)
        ;; ||||000--- All planes: 0x00
        ;; ||||010--- L planes: 0x00, M planes:  pb8
        ;; ||||100--- L planes:  pb8, M planes: 0x00
        ;; ||||110--- All planes: pb8
        ;; ||||001--- In another header byte, For each bit starting from MSB
        ;; ||||         0: 0x00 plane
        ;; ||||         1: pb8 plane
        ;; ||||011--- In another header byte, Decode only 1 pb8 plane and
        ;; ||||       duplicate it for each bit starting from MSB
        ;; ||||         0: 0x00 plane
        ;; ||||         1: duplicated plane
        ;; ||||       If extra header byte = 0x00, no pb8 plane is decoded.
        ;; ||||1x1--- Reserved for Uncompressed block bit pattern
        ;; |||+------ M planes predict from 0xff
        ;; ||+------- L planes predict from 0xff
        ;; |+-------- M = M XOR L
        ;; +--------- L = M XOR L
        ;; 00101010-- Uncompressed block of 64 bytes (bit pattern is ascii '*' )
        ;; Header >= 0xc0: Error, avaliable for outside processing.
        ;; X >= 192: Also returns in Error, the buffer would of unexpectedly page warp.
        ;;
        ;; Trashes A, temp 0 ~ temp 15.
        ;; bytes: 242, average cycles: 3700, cycle range: 1258 ~ 7225.
        ;; The subroutine name is donut_decompress_block
        plane_buffer        = temp+0 ; 8 bytes
        pb8_ctrl            = temp+8
        temp_y              = pb8_ctrl
        even_odd            = temp+9
        block_offset        = temp+10
        plane_def           = temp+11
        block_offset_end    = temp+12
        block_header        = temp+13
        is_rotated          = temp+14
        ;;_donut_unused_temp  = temp+15  ; Used as block_count in donut_bulk_load

                                ; these 2 routines (do_raw_block and read_plane_def_from_stream)
                                ; are placed above decompress_block due to branch distance
do_raw_block:
raw_block_loop:
        lda (donut_stream_ptr), y
        iny
        sta donut_block_buffer-64, x
        inx
        cpy #65  ; size of a raw block
        bcc raw_block_loop
        clc  ; to indicate success
exit_error:
        rts

read_plane_def_from_stream:
        ror
        lda (donut_stream_ptr), y
        iny
        bne plane_def_ready  ;,; jmp plane_def_ready

decompress_block:
        ldy #$00
        txa
        clc
        adc #64
        bcs exit_error
                                ; If we don't exit here, xor_l_onto_m can underflow into the previous page.
        sta block_offset_end

        lda (donut_stream_ptr), y
        cmp #$c0
        bcs exit_error
                                ; Return to caller to let it do the processing of headers >= 0xc0.
        iny  ; Y represents the number of successfully processed bytes.

        cmp #$2a
        beq do_raw_block
                                ;,; bne do_normal_block
do_normal_block:
        sta block_header
        stx block_offset

                                ;,; lda block_header
        and #$df
                                ; The 0 are bits selected for the even ("lower") planes
                                ; The 1 are bits selected for the odd planes
                                ; bits 0~3 should be set to allow the mask after this to work.
        sta even_odd
                                ; even_odd toggles between the 2 fields selected above for each plane.

                                ;,; lda block_header
        lsr
        ror is_rotated
        lsr
        bcs read_plane_def_from_stream
                                ;,; bcc unpack_shorthand_plane_def
unpack_shorthand_plane_def:
        and #$03
        tax
        lda shorthand_plane_def_table, x
plane_def_ready:
        ror is_rotated
        sta plane_def
        sty temp_y

        clc
        lda block_offset
plane_loop:
        adc #8
        sta block_offset

        lda even_odd
        eor block_header
        sta even_odd

                                ;,; lda even_odd
        and #$30
        beq not_predicted_from_ff
        lda #$ff
not_predicted_from_ff:
                                ; else A = 0x00

        asl plane_def
        bcc do_zero_plane
                                ;,; bcs do_pb8_plane
do_pb8_plane:
        ldy temp_y
        bit is_rotated
        bpl no_rewind_input_pointer
        ldy #$02
no_rewind_input_pointer:
        tax
        lda (donut_stream_ptr), y
        iny
        sta pb8_ctrl
        txa

                                ;,; bit is_rotated
        bvs do_rotated_pb8_plane
                                ;,; bvc do_normal_pb8_plane
do_normal_pb8_plane:
        ldx block_offset
                                ;,; sec  ; C is set from 'asl plane_def' above
        rol pb8_ctrl
pb8_loop:
        bcc pb8_use_prev
        lda (donut_stream_ptr), y
        iny
pb8_use_prev:
        dex
        sta donut_block_buffer-64, x
        asl pb8_ctrl
        bne pb8_loop
        sty temp_y
                                ;,; beq end_plane  ;,; jmp end_plane
end_plane:
        bit even_odd
        bpl not_xor_m_onto_l
xor_m_onto_l:
        ldy #8
xor_m_onto_l_loop:
        dex
        lda donut_block_buffer-64, x
        eor donut_block_buffer-64+8, x
        sta donut_block_buffer-64, x
        dey
        bne xor_m_onto_l_loop
not_xor_m_onto_l:

        bvc not_xor_l_onto_m
xor_l_onto_m:
        ldy #8
xor_l_onto_m_loop:
        dex
        lda donut_block_buffer-64, x
        eor donut_block_buffer-64+8, x
        sta donut_block_buffer-64+8, x
        dey
        bne xor_l_onto_m_loop
not_xor_l_onto_m:

        lda block_offset
        cmp block_offset_end
        bcc plane_loop
        ldy temp_y
        tax  ;,; ldx block_offset_end
        clc  ; to indicate success
        rts

do_zero_plane:
        ldx block_offset
        ldy #8
fill_plane_loop:
        dex
        sta donut_block_buffer-64, x
        dey
        bne fill_plane_loop
        beq end_plane  ;,; jmp end_plane

do_rotated_pb8_plane:
        ldx #8
buffered_pb8_loop:
        asl pb8_ctrl
        bcc buffered_pb8_use_prev
        lda (donut_stream_ptr), y
        iny
buffered_pb8_use_prev:
        dex
        sta plane_buffer, x
        bne buffered_pb8_loop
        sty temp_y
        ldy #8
        ldx block_offset
flip_bits_loop:
        asl plane_buffer+0
        ror
        asl plane_buffer+1
        ror
        asl plane_buffer+2
        ror
        asl plane_buffer+3
        ror
        asl plane_buffer+4
        ror
        asl plane_buffer+5
        ror
        asl plane_buffer+6
        ror
        asl plane_buffer+7
        ror
        dex
        sta donut_block_buffer-64, x
        dey
        bne flip_bits_loop
        beq end_plane  ;,; jmp end_plane

shorthand_plane_def_table:
        .byte $00, $55, $aa, $ff

        donut_decompress_block = decompress_block
