        .include "ggsound.inc"

        .section .zp.ggsound_vars,"za",@nobits

        extern_c sound_disable_update
        extern_c sfx_list_address
        extern_c base_address_instruments
        extern_c base_address_note_table_lo
        extern_c base_address_note_table_hi
        extern_c apu_data_ready
        extern_c apu_register_sets
        extern_c apu_square_1_old
        extern_c apu_square_2_old
        extern_c sound_bank

        .global MAX_STREAMS

sound_disable_update: .fill 1
sound_local_byte_0: .fill 1
sound_local_byte_1: .fill 1
sound_local_byte_2: .fill 1

sound_local_word_0: .fill 2
sound_local_word_1: .fill 2
sound_local_word_2: .fill 2

base_address_instruments: .fill 2
base_address_note_table_lo: .fill 2
base_address_note_table_hi: .fill 2

apu_data_ready: .fill 1
apu_square_1_old: .fill 1
apu_square_2_old: .fill 1

apu_register_sets: .fill 20
sound_bank: .fill 1

        .section .noinit.ggsound_vars,"a",@nobits
        extern_c stream_tempo_lo
        extern_c stream_tempo_hi
        extern_c stream_tempo_counter_lo
        extern_c stream_tempo_counter_hi
        extern_c stream_note_length_lo
        extern_c stream_note_length_hi
        extern_c stream_note_length_counter_lo
        extern_c stream_note_length_counter_hi
        extern_c stream_instrument_index
        extern_c stream_volume_offset
        extern_c stream_pitch_offset
        extern_c stream_duty_offset
        .ifdef FEATURE_ARPEGGIOS
        extern_c stream_arpeggio_offset
        .endif
        extern_c stream_channel
        extern_c stream_read_address_lo
        extern_c stream_read_address_hi
        extern_c stream_channel_register_1
        extern_c stream_flags

stream_flags:                  .fill MAX_STREAMS
stream_note:                   .fill MAX_STREAMS
stream_note_length_lo:         .fill MAX_STREAMS
stream_note_length_hi:         .fill MAX_STREAMS
stream_note_length_counter_lo: .fill MAX_STREAMS
stream_note_length_counter_hi: .fill MAX_STREAMS
stream_instrument_index:       .fill MAX_STREAMS
stream_volume_offset:          .fill MAX_STREAMS
        .ifdef FEATURE_ARPEGGIOS
stream_arpeggio_offset:        .fill MAX_STREAMS
        .endif
stream_pitch_offset:           .fill MAX_STREAMS
stream_duty_offset:            .fill MAX_STREAMS

stream_channel:                .fill MAX_STREAMS
stream_channel_register_1:     .fill MAX_STREAMS
stream_channel_register_2:     .fill MAX_STREAMS
stream_channel_register_3:     .fill MAX_STREAMS
stream_channel_register_4:     .fill MAX_STREAMS

stream_read_address_lo:        .fill MAX_STREAMS
stream_read_address_hi:        .fill MAX_STREAMS
stream_return_address_lo:      .fill MAX_STREAMS
stream_return_address_hi:      .fill MAX_STREAMS

stream_tempo_counter_lo:       .fill MAX_STREAMS
stream_tempo_counter_hi:       .fill MAX_STREAMS
stream_tempo_lo:               .fill MAX_STREAMS
stream_tempo_hi:               .fill MAX_STREAMS

        .section .prg_rom_2,"axR",@progbits

                                ;Updates all playing streams, if active. Streams 0 through MAX_MUSIC_STREAMS-1
                                ;are assumed to be music streams. The last two streams, are assumed to be sound
                                ;effect streams. When these are playing, their channel control registers are
                                ;copied overtop what the corresponding music streams had written, so the sound
                                ;effect streams essentially take over while they are playing. When the sound
                                ;effect streams are finished, they signify their corresponding music stream
                                ;(via the TRM callback) to silence themselves until the next note to avoid
                                ;ugly volume envelope transitions. DPCM is handled within this framework by
                                ;a state machine that handles sound effect priority.
sound_update:

                                ;Save regs.
        txa
        pha

                                ;Signal apu data not ready.
        lda #0
        sta apu_data_ready

                                ;First copy all music streams.
        ldx #0
song_stream_register_copy_loop:

                                ;Load whether this stream is active.
        lda stream_flags,x
        and #STREAM_ACTIVE_TEST
        beq song_stream_not_active

                                ;Update the stream.
        jsr stream_update

                                ;Load channel number.
        lda stream_channel,x
                                ;Multiply by four to get location within apu_register_sets.
        asl
        asl
        tay
                                ;Copy the registers over.
        lda stream_channel_register_1,x
        sta apu_register_sets,y
        lda stream_channel_register_2,x
        sta apu_register_sets+1,y
        lda stream_channel_register_3,x
        sta apu_register_sets+2,y
        lda stream_channel_register_4,x
        sta apu_register_sets+3,y
song_stream_not_active:

        inx
        cpx #MAX_MUSIC_STREAMS
        bne song_stream_register_copy_loop
do_not_update_music:

        ldx #soundeffect_one
sfx_stream_register_copy_loop:

                                ;Load whether this stream is active.
        lda stream_flags,x
        and #STREAM_ACTIVE_TEST
        beq sfx_stream_not_active

                                ;Update the stream.
        jsr stream_update

                                ;Load channel number
        lda stream_channel,x
                                ;Multiply by four to get location within apu_register_sets.
        asl
        asl
        tay
                                ;Copy the registers over.
        lda stream_channel_register_1,x
        sta apu_register_sets,y
        lda stream_channel_register_2,x
        sta apu_register_sets+1,y
        lda stream_channel_register_3,x
        sta apu_register_sets+2,y
        lda stream_channel_register_4,x
        sta apu_register_sets+3,y
sfx_stream_not_active:

        inx
        cpx #MAX_STREAMS
        bne sfx_stream_register_copy_loop

                                ;Signial apu data ready.
        lda #1
        sta apu_data_ready

                                ;Restore regs.
        pla
        tax

        rts

                                ;****************************************************************
                                ;These callbacks are all note playback and only execute once per
                                ;frame.
                                ;****************************************************************

square_1_play_note:

                                ;Load instrument index.
        ldy stream_instrument_index,x
                                ;Load instrument address.
        lda (base_address_instruments),y
        sta sound_local_word_0
        iny
        lda (base_address_instruments),y
        sta sound_local_word_0+1

                                ;Set negate flag for sweep unit.
        lda #$08
        sta stream_channel_register_2,x

        .ifdef FEATURE_ARPEGGIOS

                                ;Get arpeggio type.
        ldy #instrument_header_arpeggio_type
        lda (sound_local_word_0),y
        tay

                                ;Get the address.
        lda #>(s1pn_return_from_arpeggio_callback-1)
        pha
        lda #<(s1pn_return_from_arpeggio_callback-1)
        pha
        lda arpeggio_callback_table_hi,y
        pha
        lda arpeggio_callback_table_lo,y
        pha
        rts
s1pn_return_from_arpeggio_callback:

        .else

        ldy stream_note,x

        .endif

                                ;Skip loading note pitch if already loaded, to allow envelopes
                                ;to modify the pitch.
        lda stream_flags,x
        and #STREAM_PITCH_LOADED_TEST
        bne s1pn_pitch_already_loaded
        lda stream_flags,x
        ora #STREAM_PITCH_LOADED_SET
        sta stream_flags,x
                                ;Load low byte of note.
        lda (base_address_note_table_lo),y
                                ;Store in low 8 bits of pitch.
        sta stream_channel_register_3,x
                                ;Load high byte of note.
        lda (base_address_note_table_hi),y
        sta stream_channel_register_4,x
s1pn_pitch_already_loaded:

        lda stream_flags,x
        and #STREAM_SILENCE_TEST
        bne s1pn_silence_until_note
        //note_not_silenced:

                                ;Load volume offset.
        ldy stream_volume_offset,x

                                ;Load volume value for this frame, branch if opcode.
        lda (sound_local_word_0),y
        cmp #ENV_STOP
        beq s1pn_volume_stop
        cmp #ENV_LOOP
        bne s1pn_skip_volume_loop

                                ;We hit a loop opcode, advance envelope index and load loop point.
        iny
        lda (sound_local_word_0),y
        sta stream_volume_offset,x
        tay

s1pn_skip_volume_loop:

                                ;Initialize channel control register with envelope decay and
                                ;length counter disabled but preserving current duty cycle.
        lda stream_channel_register_1,x
        and #0b11000000
        ora #0b00110000

                                ;Load current volume value.
        ora (sound_local_word_0),y
        sta stream_channel_register_1,x

        inc stream_volume_offset,x

s1pn_volume_stop:

        jmp s1pn_done
s1pn_silence_until_note:
        lda stream_channel_register_1,x
        and #0b11000000
        ora #0b00110000
        sta stream_channel_register_1,x

s1pn_done:

                                ;Load pitch offset.
        ldy stream_pitch_offset,x

                                ;Load pitch value.
        lda (sound_local_word_0),y
        cmp #ENV_STOP
        beq s1pn_pitch_stop
        cmp #ENV_LOOP
        bne s1pn_skip_pitch_loop

                                ;We hit a loop opcode, advance envelope index and load loop point.
        iny
        lda (sound_local_word_0),y
        sta stream_pitch_offset,x
        tay

s1pn_skip_pitch_loop:

                                ;Test sign.
        lda (sound_local_word_0),y
        bmi s1pn_pitch_delta_negative
        //pitch_delta_positive:

        clc
        lda stream_channel_register_3,x
        adc (sound_local_word_0),y
        sta stream_channel_register_3,x
        lda stream_channel_register_4,x
        adc #0
        sta stream_channel_register_4,x

        jmp s1pn_pitch_delta_test_done

s1pn_pitch_delta_negative:

        clc
        lda stream_channel_register_3,x
        adc (sound_local_word_0),y
        sta stream_channel_register_3,x
        lda stream_channel_register_4,x
        adc #$ff
        sta stream_channel_register_4,x

s1pn_pitch_delta_test_done:

                                ;Move pitch offset along.
        inc stream_pitch_offset,x

s1pn_pitch_stop:

        //duty_code:

        ldy stream_duty_offset,x

                                ;Load duty value for this frame, but hard code flags and duty for now.
        lda (sound_local_word_0),y
        cmp #DUTY_ENV_STOP
        beq s1pn_duty_stop
        cmp #DUTY_ENV_LOOP
        bne s1pn_skip_duty_loop

                                ;We hit a loop opcode, advance envelope index and load loop point.
        iny
        lda (sound_local_word_0),y
        sta stream_duty_offset,x
        tay

s1pn_skip_duty_loop:

                                ;Or the duty value into the register.
        lda stream_channel_register_1,x
        and #0b00111111
        ora (sound_local_word_0),y
        sta stream_channel_register_1,x

                                ;Move duty offset along.
        inc stream_duty_offset,x

s1pn_duty_stop:

        rts

        square_2_play_note = square_1_play_note

triangle_play_note:

                                ;Load instrument index.
        ldy stream_instrument_index,x
                                ;Load instrument address.
        lda (base_address_instruments),y
        sta sound_local_word_0
        iny
        lda (base_address_instruments),y
        sta sound_local_word_0+1

        .ifdef FEATURE_ARPEGGIOS
                                ;Get arpeggio type.
        ldy #instrument_header_arpeggio_type
        lda (sound_local_word_0),y
        tay

                                ;Get the address.
        lda #>(tpn_return_from_arpeggio_callback-1)
        pha
        lda #<(tpn_return_from_arpeggio_callback-1)
        pha
        lda arpeggio_callback_table_hi,y
        pha
        lda arpeggio_callback_table_lo,y
        pha
        rts
tpn_return_from_arpeggio_callback:

        .else

        ldy stream_note,x

        .endif

                                ;Skip loading note pitch if already loaded, to allow envelopes
                                ;to modify the pitch.
        lda stream_flags,x
        and #STREAM_PITCH_LOADED_TEST
        bne tpn_pitch_already_loaded
        lda stream_flags,x
        ora #STREAM_PITCH_LOADED_SET
        sta stream_flags,x
                                ;Load low byte of note.
        lda (base_address_note_table_lo),y
                                ;Store in low 8 bits of pitch.
        sta stream_channel_register_3,x
                                ;Load high byte of note.
        lda (base_address_note_table_hi),y
        sta stream_channel_register_4,x
tpn_pitch_already_loaded:

                                ;Load volume offset.
        ldy stream_volume_offset,x

                                ;Load volume value for this frame, but hard code flags and duty for now.
        lda (sound_local_word_0),y
        cmp #ENV_STOP
        beq tpn_volume_stop
        cmp #ENV_LOOP
        bne tpn_skip_volume_loop

                                ;We hit a loop opcode, advance envelope index and load loop point.
        iny
        lda (sound_local_word_0),y
        sta stream_volume_offset,x
        tay

tpn_skip_volume_loop:

        lda #0b10000000
        ora (sound_local_word_0),y
        sta stream_channel_register_1,x

        inc stream_volume_offset,x

tpn_volume_stop:

                                ;Load pitch offset.
        ldy stream_pitch_offset,x

                                ;Load pitch value.
        lda (sound_local_word_0),y
        cmp #ENV_STOP
        beq tpn_pitch_stop
        cmp #ENV_LOOP
        bne tpn_skip_pitch_loop

                                ;We hit a loop opcode, advance envelope index and load loop point.
        iny
        lda (sound_local_word_0),y
        sta stream_pitch_offset,x
        tay

tpn_skip_pitch_loop:

                                ;Test sign.
        lda (sound_local_word_0),y
        bmi tpn_pitch_delta_negative
        //pitch_delta_positive:

        clc
        lda stream_channel_register_3,x
        adc (sound_local_word_0),y
        sta stream_channel_register_3,x
        lda stream_channel_register_4,x
        adc #0
        sta stream_channel_register_4,x

        jmp tpn_pitch_delta_test_done

tpn_pitch_delta_negative:

        clc
        lda stream_channel_register_3,x
        adc (sound_local_word_0),y
        sta stream_channel_register_3,x
        lda stream_channel_register_4,x
        adc #$ff
        sta stream_channel_register_4,x

tpn_pitch_delta_test_done:

                                ;Move pitch offset along.
        inc stream_pitch_offset,x

tpn_pitch_stop:

        rts

noise_play_note:

                                ;Load instrument index.
        ldy stream_instrument_index,x
                                ;Load instrument address.
        lda (base_address_instruments),y
        sta sound_local_word_0
        iny
        lda (base_address_instruments),y
        sta sound_local_word_0+1

        .ifdef FEATURE_ARPEGGIOS
                                ;Get arpeggio type.
        ldy #instrument_header_arpeggio_type
        lda (sound_local_word_0),y
        tay

                                ;Get the address.
        lda #>(npn_return_from_arpeggio_callback-1)
        pha
        lda #<(npn_return_from_arpeggio_callback-1)
        pha
        lda arpeggio_callback_table_hi,y
        pha
        lda arpeggio_callback_table_lo,y
        pha
        rts
npn_return_from_arpeggio_callback:

        .else

        ldy stream_note,x

        .endif

        tya
        and #0b01111111
        sta sound_local_byte_0

                                ;Skip loading note pitch if already loaded, to allow envelopes
                                ;to modify the pitch.
        lda stream_flags,x
        and #STREAM_PITCH_LOADED_TEST
        bne npn_pitch_already_loaded
        lda stream_flags,x
        ora #STREAM_PITCH_LOADED_SET
        sta stream_flags,x
        lda stream_channel_register_3,x
        and #0b10000000
        ora sound_local_byte_0
        sta stream_channel_register_3,x
npn_pitch_already_loaded:

                                ;Load volume offset.
        ldy stream_volume_offset,x

                                ;Load volume value for this frame, hard code disable flags.
        lda (sound_local_word_0),y
        cmp #ENV_STOP
        beq npn_volume_stop
        cmp #ENV_LOOP
        bne npn_skip_volume_loop

                                ;We hit a loop opcode, advance envelope index and load loop point.
        iny
        lda (sound_local_word_0),y
        sta stream_volume_offset,x
        tay

npn_skip_volume_loop:

        lda #0b00110000
        ora (sound_local_word_0),y
        sta stream_channel_register_1,x

                                ;Move volume offset along.
        inc stream_volume_offset,x
npn_volume_stop:

                                ;Load pitch offset.
        ldy stream_pitch_offset,x

                                ;Load pitch value.
        lda (sound_local_word_0),y
        cmp #ENV_STOP
        beq npn_pitch_stop
        cmp #ENV_LOOP
        bne npn_skip_pitch_loop

                                ;We hit a loop opcode, advance envelope index and load loop point.
        iny
        lda (sound_local_word_0),y
        sta stream_pitch_offset,x
        tay

npn_skip_pitch_loop:

                                ;Save off current duty bit.
        lda stream_channel_register_3,x
        and #0b10000000
        sta sound_local_byte_0

                                ;Advance pitch regardless of duty bit.
        clc
        lda stream_channel_register_3,x
        adc (sound_local_word_0),y
        and #0b00001111
                                ;Get duty bit back in.
        ora sound_local_byte_0
        sta stream_channel_register_3,x

                                ;Move pitch offset along.
        inc stream_pitch_offset,x

npn_pitch_stop:

        //duty_code:
                                ;Load duty offset.
        ldy stream_duty_offset,x

                                ;Load duty value for this frame, but hard code flags and duty for now.
        lda (sound_local_word_0),y
        cmp #DUTY_ENV_STOP
        beq npn_duty_stop
        cmp #DUTY_ENV_LOOP
        bne npn_skip_duty_loop

                                ;We hit a loop opcode, advance envelope index and load loop point.
        iny
        lda (sound_local_word_0),y
        sta stream_duty_offset,x
        tay

npn_skip_duty_loop:

                                ;We only care about bit 6 for noise, and we want it in bit 7 position.
        lda (sound_local_word_0),y
        asl
        sta sound_local_byte_0

        lda stream_channel_register_3,x
        and #0b01111111
        ora sound_local_byte_0
        sta stream_channel_register_3,x

                                ;Move duty offset along.
        inc stream_duty_offset,x

npn_duty_stop:

        rts

        .ifdef FEATURE_ARPEGGIOS

arpeggio_absolute:

        ldy stream_arpeggio_offset,x

        lda (sound_local_word_0),y
        cmp #ENV_STOP
        beq aa_arpeggio_stop
        cmp #ENV_LOOP
        beq aa_arpeggio_loop
        //arpeggio_play:

                                ;We're changing notes.
        lda stream_flags,x
        and #STREAM_PITCH_LOADED_CLEAR
        sta stream_flags,x

                                ;Load the current arpeggio value and add it to current note.
        clc
        lda (sound_local_word_0),y
        adc stream_note,x
        tay
                                ;Advance arpeggio offset.
        inc stream_arpeggio_offset,x

        jmp aa_done
aa_arpeggio_stop:

                                ;Just load the current note.
        ldy stream_note,x

        jmp aa_done
aa_arpeggio_loop:

                                ;We hit a loop opcode, advance envelope index and load loop point.
        iny
        lda (sound_local_word_0),y
        sta stream_arpeggio_offset,x
        tay

                                ;We're changing notes.
        lda stream_flags,x
        and #STREAM_PITCH_LOADED_CLEAR
        sta stream_flags,x

                                ;Load the current arpeggio value and add it to current note.
        clc
        lda (sound_local_word_0),y
        adc stream_note,x
        tay
                                ;Advance arpeggio offset.
        inc stream_arpeggio_offset,x
aa_done:

        rts

arpeggio_fixed:

        ldy stream_arpeggio_offset,x

        lda (sound_local_word_0),y
        cmp #ENV_STOP
        beq af_arpeggio_stop
        cmp #ENV_LOOP
        beq af_arpeggio_loop
        //arpeggio_play:

                                ;We're changing notes.
        lda stream_flags,x
        and #STREAM_PITCH_LOADED_CLEAR
        sta stream_flags,x

                                ;Load the current arpeggio value and use it as the current note.
        lda (sound_local_word_0),y
                                ;sta stream_note,x
        tay
                                ;Advance arpeggio offset.
        inc stream_arpeggio_offset,x

        jmp af_done
af_arpeggio_stop:

                                ;When a fixed arpeggio is done, we're changing notes to the
                                ;currently playing note. (This is FamiTracker's behavior)
                                ;However, we only do this if we're stopping at any point other
                                ;than one, which indicates an arpeggio did in fact execute.
        lda stream_arpeggio_offset,x
        cmp #1
        beq af_skip_clear_pitch_loaded
        lda stream_flags,x
        and #STREAM_PITCH_LOADED_CLEAR
        sta stream_flags,x
af_skip_clear_pitch_loaded:

                                ;Just load the current note.
        ldy stream_note,x

        jmp af_done
af_arpeggio_loop:

                                ;We hit a loop opcode, advance envelope index and load loop point.
        iny
        lda (sound_local_word_0),y
        sta stream_arpeggio_offset,x
        tay

                                ;We're changing notes.
        lda stream_flags,x
        and #STREAM_PITCH_LOADED_CLEAR
        sta stream_flags,x

                                ;Load the current arpeggio value and use it as the current note.
        lda (sound_local_word_0),y
        tay
                                ;Advance arpeggio offset.
        inc stream_arpeggio_offset,x
af_done:

        rts

arpeggio_relative:

        ldy stream_arpeggio_offset,x

        lda (sound_local_word_0),y
        cmp #ENV_STOP
        beq ar_arpeggio_stop
        cmp #ENV_LOOP
        beq ar_arpeggio_loop
        //arpeggio_play:

                                ;We're changing notes.
        lda stream_flags,x
        and #STREAM_PITCH_LOADED_CLEAR
        sta stream_flags,x

                                ;Load the current arpeggio value and add it to current note.
        clc
        lda (sound_local_word_0),y
        adc stream_note,x
        cmp #HIGHEST_NOTE
        bmi ar_skip
        lda #HIGHEST_NOTE
ar_skip:
        sta stream_note,x
        tay
                                ;Advance arpeggio offset.
        inc stream_arpeggio_offset,x

        jmp ar_done
ar_arpeggio_stop:

                                ;Just load the current note.
        ldy stream_note,x

        jmp ar_done
ar_arpeggio_loop:

                                ;We hit a loop opcode, advance envelope index and load loop point.
        iny
        lda (sound_local_word_0),y
        sta stream_arpeggio_offset,x
        tay

                                ;We're changing notes.
        lda stream_flags,x
        and #STREAM_PITCH_LOADED_CLEAR
        sta stream_flags,x

                                ;Load the current arpeggio value and add it to current note.
        clc
        lda (sound_local_word_0),y
        adc stream_note,x
        tay
                                ;Advance arpeggio offset.
        inc stream_arpeggio_offset,x
ar_done:

        rts

        .endif

                                ;****************************************************************
                                ;These callbacks are all stream control and execute in sequence
                                ;until exhausted.
                                ;****************************************************************

stream_set_instrument:

        advance_stream_read_address
                                ;Load byte at read address.
        lda stream_read_address_lo,x
        sta sound_local_word_0
        lda stream_read_address_hi,x
        sta sound_local_word_0+1
        ldy #0
        lda (sound_local_word_0),y
        asl
        sta stream_instrument_index,x
        tay

        lda (base_address_instruments),y
        sta sound_local_word_0
        iny
        lda (base_address_instruments),y
        sta sound_local_word_0+1

        ldy #0
        lda (sound_local_word_0),y
        sta stream_volume_offset,x
        iny
        lda (sound_local_word_0),y
        sta stream_pitch_offset,x
        iny
        lda (sound_local_word_0),y
        sta stream_duty_offset,x
        .ifdef FEATURE_ARPEGGIOS
        iny
        lda (sound_local_word_0),y
        sta stream_arpeggio_offset,x
        .endif

        rts

                                ;Set a standard note length. This callback works for a set
                                ;of opcodes which can set the note length for values 1 through 16.
                                ;This helps reduce ROM space required by songs.
stream_set_length_s:

                                ;determine note length from opcode
        sec
        lda stream_note,x
        sbc #OPCODES_BASE
        clc
        adc #1
        sta stream_note_length_lo,x
        sta stream_note_length_counter_lo,x
        lda #0
        sta stream_note_length_hi,x
        sta stream_note_length_counter_hi,x

        rts

stream_set_length_lo:

        advance_stream_read_address
                                ;Load byte at read address.
        lda stream_read_address_lo,x
        sta sound_local_word_0
        lda stream_read_address_hi,x
        sta sound_local_word_0+1
        ldy #0
        lda (sound_local_word_0),y
        sta stream_note_length_lo,x
        sta stream_note_length_counter_lo,x
        lda #0
        sta stream_note_length_hi,x
        sta stream_note_length_counter_hi,x

        rts

stream_set_length_hi:

        advance_stream_read_address
                                ;Load byte at read address.
        lda stream_read_address_lo,x
        sta sound_local_word_0
        lda stream_read_address_hi,x
        sta sound_local_word_0+1
        ldy #0
        lda (sound_local_word_0),y
        sta stream_note_length_hi,x
        sta stream_note_length_counter_hi,x

        rts

                                ;This opcode loops to the beginning of the stream. It expects the two
                                ;following bytes to contain the address to loop to.
stream_goto:

        advance_stream_read_address
                                ;Load byte at read address.
        lda stream_read_address_lo,x
        sta sound_local_word_0
        lda stream_read_address_hi,x
        sta sound_local_word_0+1
        ldy #0
        lda (sound_local_word_0),y
        sta stream_read_address_lo,x
        ldy #1
        lda (sound_local_word_0),y
        sta stream_read_address_hi,x

        sec
        lda stream_read_address_lo,x
        sbc #1
        sta stream_read_address_lo,x
        lda stream_read_address_hi,x
        sbc #0
        sta stream_read_address_hi,x

        rts

                                ;This opcode stores the current stream read address in
                                ;return_stream_read_address (lo and hi) and then reads the
                                ;following two bytes and stores them in the current stream read address.
                                ;It is assumed that a RET opcode will be encountered in the stream which
                                ;is being called, which will restore the return stream read address.
                                ;This is how the engine can allow repeated chunks of a song.
stream_call:

        advance_stream_read_address
        lda stream_read_address_lo,x
        sta sound_local_word_0
        lda stream_read_address_hi,x
        sta sound_local_word_0+1

                                ;Retrieve lo byte of destination address from first CAL parameter.
        ldy #0
        lda (sound_local_word_0),y
        sta sound_local_word_1
        iny
                                ;Retrieve hi byte of destination address from second CAL parameter.
        lda (sound_local_word_0),y
        sta sound_local_word_1+1

        advance_stream_read_address

                                ;Now store current stream read address in stream's return address.
        lda stream_read_address_lo,x
        sta stream_return_address_lo,x
        lda stream_read_address_hi,x
        sta stream_return_address_hi,x

                                ;Finally, transfer address we are calling to current read address.
        sec
        lda sound_local_word_1
        sbc #<1
        sta stream_read_address_lo,x
        lda sound_local_word_1+1
        sbc #>1
        sta stream_read_address_hi,x

        rts

                                ;This opcode restores the stream_return_address to the stream_read_address
                                ;and continues where it left off.
stream_return:

        lda stream_return_address_lo,x
        sta stream_read_address_lo,x
        lda stream_return_address_hi,x
        sta stream_read_address_hi,x

        rts

                                ;This opcode returns from the parent caller by popping two bytes off
                                ;the stack and then doing rts.
stream_terminate:

                                ;Set the current stream to inactive.
        lda #0
        sta stream_flags,x

        cpx #soundeffect_one
        bmi st_not_sound_effect

                                ;Load channel this sfx writes to.
        ldy stream_channel,x
                                ;Use this as index into streams to tell corresponding music channel
                                ;to silence until the next note.
        lda stream_flags,y
        ora #STREAM_SILENCE_SET
        sta stream_flags,y

st_not_sound_effect:

                                ;Pop current address off the stack.
        pla
        pla

                                ;Return from parent caller.
        rts

                                ;Updates a single stream.
                                ;Expects x to be pointing to a stream instance as an offset from streams.
stream_update:
        callback_address = sound_local_word_0
        read_address = sound_local_word_1

        lda stream_flags,x
        and #STREAM_PAUSE_TEST
        beq skip0
        rts
skip0:

                                ;Load current read address of stream.
        lda stream_read_address_lo,x
        sta read_address
        lda stream_read_address_hi,x
        sta read_address+1

                                ;Load next byte from stream data.
        lda stream_flags,x
        and #STREAM_PITCH_LOADED_TEST
        bne skip1
        ldy #0
        lda (read_address),y
        sta stream_note,x
skip1:

                                ;Is this byte a note or a stream opcode?
        cmp #OPCODES_BASE
        bcc process_note
process_opcode:

                                ;Look up the opcode in the stream callbacks table.
        sec
        sbc #OPCODES_BASE
        tay
                                ;Get the address.
        lda stream_callback_table_lo,y
        sta callback_address
        lda stream_callback_table_hi,y
        sta callback_address+1
                                ;Call the callback!
        jsr indirect_jsr_callback_address

                                ;Advance the stream's read address.
        advance_stream_read_address

                                ;Immediately process the next opcode or note. The idea here is that
                                ;all stream control opcodes will execute during the current frame as "setup"
                                ;for the next note. All notes will execute once per frame and will always
                                ;return from this routine. This leaves the problem, how would the stream
                                ;control opcode "terminate" work? It works by pulling the current return
                                ;address off the stack and then performing an rts, effectively returning
                                ;from its caller, this routine.
        jmp stream_update

process_note:

                                ;Determine which channel callback to use.
        lda stream_channel,x
        tay
        lda channel_callback_table_lo,y
        sta callback_address
        lda channel_callback_table_hi,y
        sta callback_address+1

                                ;Call the channel callback!
        jsr indirect_jsr_callback_address

        sec
        lda stream_tempo_counter_lo,x
        sbc #<256
        sta stream_tempo_counter_lo,x
        lda stream_tempo_counter_hi,x
        sbc #>256
        sta stream_tempo_counter_hi,x
        bcs do_not_advance_note_length_counter

                                ;Reset tempo counter when we cross 0 by adding original tempo back on.
                                ;This way we have a wrap-around value that does not get lost when we count
                                ;down to the next note.
        clc
        lda stream_tempo_counter_lo,x
        adc stream_tempo_lo,x
        sta stream_tempo_counter_lo,x
        lda stream_tempo_counter_hi,x
        adc stream_tempo_hi,x
        sta stream_tempo_counter_hi,x

                                ;Decrement the note length counter.. On zero, advance the stream's read address.
        sec
        lda stream_note_length_counter_lo,x
        sbc #<1
        sta stream_note_length_counter_lo,x
        lda stream_note_length_counter_hi,x
        sbc #>1
        sta stream_note_length_counter_hi,x

        lda stream_note_length_counter_lo,x
        ora stream_note_length_counter_hi,x

        bne note_length_counter_not_zero

                                ;Reset the note length counter.
        lda stream_note_length_lo,x
        sta stream_note_length_counter_lo,x
        lda stream_note_length_hi,x
        sta stream_note_length_counter_hi,x

        ldy stream_instrument_index,x
        lda (base_address_instruments),y
        sta sound_local_word_0
        iny
        lda (base_address_instruments),y
        sta sound_local_word_0+1
        ldy #0
        lda (sound_local_word_0),y
        sta stream_volume_offset,x
        iny
        lda (sound_local_word_0),y
        sta stream_pitch_offset,x
        iny
        lda (sound_local_word_0),y
        sta stream_duty_offset,x
        .ifdef FEATURE_ARPEGGIOS
        iny
        lda (sound_local_word_0),y
        sta stream_arpeggio_offset,x
        .endif

                                ;Reset silence until note and pitch loaded flags.
        lda stream_flags,x
        and #STREAM_SILENCE_CLEAR
        and #STREAM_PITCH_LOADED_CLEAR
        sta stream_flags,x

                                ;Advance the stream's read address.
        advance_stream_read_address
do_not_advance_note_length_counter:
note_length_counter_not_zero:

        rts

indirect_jsr_callback_address:
        jmp (callback_address)
        rts

sound_upload:

        lda apu_data_ready
        beq apu_data_not_ready

        jsr sound_upload_apu_register_sets

apu_data_not_ready:

        rts

sound_upload_apu_register_sets:
square1:
        lda apu_register_sets+0
        sta $4000
        lda apu_register_sets+1
        sta $4001
        lda apu_register_sets+2
        sta $4002
        lda apu_register_sets+3
                                ;Compare to last write.
        cmp apu_square_1_old
                                ;Don't write this frame if they were equal.
        beq square2
        sta $4003
                                ;Save the value we just wrote to $4003.
        sta apu_square_1_old
square2:
        lda apu_register_sets+4
        sta $4004
        lda apu_register_sets+5
        sta $4005
        lda apu_register_sets+6
        sta $4006
        lda apu_register_sets+7
        cmp apu_square_2_old
        beq triangle
        sta $4007
                                ;Save the value we just wrote to $4007.
        sta apu_square_2_old
triangle:
        lda apu_register_sets+8
        sta $4008
        lda apu_register_sets+10
        sta $400A
        lda apu_register_sets+11
        sta $400B
noise:
        lda apu_register_sets+12
        sta $400C
        lda apu_register_sets+14
                                ;Our notes go from 0 to 15 (low to high)
                                ;but noise channel's low to high is 15 to 0.
        eor #$0f
        sta $400E
        lda apu_register_sets+15
        sta $400F

                                ;Clear out all volume values from this frame in case a sound effect is killed suddenly.
        lda #0b00110000
        sta apu_register_sets
        sta apu_register_sets+4
        sta apu_register_sets+12
        lda #0b10000000
        sta apu_register_sets+8
        rts

        .section .prg_rom_2.rodata,"aR",@progbits

channel_callback_table_lo:
        .byte square_1_play_note@mos16lo
        .byte square_2_play_note@mos16lo
        .byte triangle_play_note@mos16lo
        .byte noise_play_note@mos16lo

channel_callback_table_hi:
        .byte square_1_play_note@mos16hi
        .byte square_2_play_note@mos16hi
        .byte triangle_play_note@mos16hi
        .byte noise_play_note@mos16hi

stream_callback_table_lo:
        .byte stream_set_length_s@mos16lo
        .byte stream_set_length_s@mos16lo
        .byte stream_set_length_s@mos16lo
        .byte stream_set_length_s@mos16lo
        .byte stream_set_length_s@mos16lo
        .byte stream_set_length_s@mos16lo
        .byte stream_set_length_s@mos16lo
        .byte stream_set_length_s@mos16lo
        .byte stream_set_length_s@mos16lo
        .byte stream_set_length_s@mos16lo
        .byte stream_set_length_s@mos16lo
        .byte stream_set_length_s@mos16lo
        .byte stream_set_length_s@mos16lo
        .byte stream_set_length_s@mos16lo
        .byte stream_set_length_s@mos16lo
        .byte stream_set_length_s@mos16lo
        .byte stream_set_length_lo@mos16lo
        .byte stream_set_length_hi@mos16lo
        .byte stream_set_instrument@mos16lo
        .byte stream_goto@mos16lo
        .byte stream_call@mos16lo
        .byte stream_return@mos16lo
        .byte stream_terminate@mos16lo

stream_callback_table_hi:
        .byte stream_set_length_s@mos16hi
        .byte stream_set_length_s@mos16hi
        .byte stream_set_length_s@mos16hi
        .byte stream_set_length_s@mos16hi
        .byte stream_set_length_s@mos16hi
        .byte stream_set_length_s@mos16hi
        .byte stream_set_length_s@mos16hi
        .byte stream_set_length_s@mos16hi
        .byte stream_set_length_s@mos16hi
        .byte stream_set_length_s@mos16hi
        .byte stream_set_length_s@mos16hi
        .byte stream_set_length_s@mos16hi
        .byte stream_set_length_s@mos16hi
        .byte stream_set_length_s@mos16hi
        .byte stream_set_length_s@mos16hi
        .byte stream_set_length_s@mos16hi
        .byte stream_set_length_lo@mos16hi
        .byte stream_set_length_hi@mos16hi
        .byte stream_set_instrument@mos16hi
        .byte stream_goto@mos16hi
        .byte stream_call@mos16hi
        .byte stream_return@mos16hi
        .byte stream_terminate@mos16hi

        .ifdef FEATURE_ARPEGGIOS

arpeggio_callback_table_lo:
        .byte (arpeggio_absolute-1)@mos16lo
        .byte (arpeggio_fixed-1)@mos16lo
        .byte (arpeggio_relative-1)@mos16lo

arpeggio_callback_table_hi:
        .byte (arpeggio_absolute-1)@mos16hi
        .byte (arpeggio_fixed-1)@mos16hi
        .byte (arpeggio_relative-1)@mos16hi

        .endif

        .section .nmi.200,"axR",@progbits
        soundengine_update

        .section .init.200,"axR",@progbits
        lda #1
        sta sound_disable_update
