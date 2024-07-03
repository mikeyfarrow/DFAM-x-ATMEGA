	.file	"MIDI.cpp"
__SP_H__ = 0x3e
__SP_L__ = 0x3d
__SREG__ = 0x3f
__tmp_reg__ = 0
__zero_reg__ = 1
	.text
.Ltext0:
	.cfi_sections	.debug_frame
	.section	.text._ZN4midi11encodeSysExEPKhPhjb,"ax",@progbits
.global	_ZN4midi11encodeSysExEPKhPhjb
	.type	_ZN4midi11encodeSysExEPKhPhjb, @function
_ZN4midi11encodeSysExEPKhPhjb:
.LFB108:
	.file 1 ".././MIDI.cpp"
	.loc 1 50 0
	.cfi_startproc
.LVL0:
	push r14
.LCFI0:
	.cfi_def_cfa_offset 3
	.cfi_offset 14, -2
	push r15
.LCFI1:
	.cfi_def_cfa_offset 4
	.cfi_offset 15, -3
	push r16
.LCFI2:
	.cfi_def_cfa_offset 5
	.cfi_offset 16, -4
	push r17
.LCFI3:
	.cfi_def_cfa_offset 6
	.cfi_offset 17, -5
	push r28
.LCFI4:
	.cfi_def_cfa_offset 7
	.cfi_offset 28, -6
	push r29
.LCFI5:
	.cfi_def_cfa_offset 8
	.cfi_offset 29, -7
/* prologue: function */
/* frame size = 0 */
/* stack size = 6 */
.L__stack_usage = 6
	movw r28,r22
	mov r15,r18
.LVL1:
	.loc 1 53 0
	st Y,__zero_reg__
.LVL2:
.LBB2:
	.loc 1 55 0
	ldi r26,0
	ldi r27,0
.LBE2:
	.loc 1 52 0
	ldi r19,0
	.loc 1 51 0
	ldi r22,0
	ldi r23,0
.LVL3:
.L6:
.LBB4:
	.loc 1 55 0 discriminator 1
	cp r26,r20
	cpc r27,r21
	brsh .L2
.LBB3:
	.loc 1 57 0
	movw r30,r24
	add r30,r26
	adc r31,r27
	ld r31,Z
.LVL4:
	.loc 1 58 0
	mov r16,r31
	ldi r17,0
.LVL5:
	.loc 1 59 0
	mov r18,r31
	andi r18,lo8(127)
.LVL6:
	.loc 1 61 0
	ld r14,Y
	movw r30,r16
	lsl r30
	mov r30,r31
	rol r30
	sbc r31,r31
	neg r31
	tst r15
	breq .L3
	.loc 1 61 0 is_stmt 0 discriminator 1
	mov r16,r19
.LVL7:
	rjmp .L4
.LVL8:
.L3:
	.loc 1 61 0 discriminator 2
	ldi r16,lo8(6)
	ldi r17,0
.LVL9:
	sub r16,r19
	sbc r17,__zero_reg__
.L4:
	.loc 1 61 0 discriminator 4
	rjmp 2f
	1:
	lsl r30
	rol r31
	2:
	dec r16
	brpl 1b
	or r30,r14
	st Y,r30
	.loc 1 62 0 is_stmt 1 discriminator 4
	mov r30,r19
	ldi r31,0
.LVL10:
	adiw r30,1
	add r30,r28
	adc r31,r29
	st Z,r18
	.loc 1 64 0 discriminator 4
	ldi r30,lo8(1)
	add r30,r19
.LVL11:
	cpi r19,lo8(6)
	brne .L5
.LVL12:
	.loc 1 67 0
	subi r22,-8
	sbci r23,-1
.LVL13:
	.loc 1 68 0
	std Y+8,__zero_reg__
.LVL14:
	.loc 1 66 0
	adiw r28,8
.LVL15:
	.loc 1 69 0
	ldi r30,0
.LVL16:
.L5:
.LBE3:
	.loc 1 55 0 discriminator 2
	adiw r26,1
.LVL17:
	mov r19,r30
	rjmp .L6
.LVL18:
.L2:
.LBE4:
	.loc 1 72 0
	add r22,r19
	adc r23,__zero_reg__
.LVL19:
	tst r19
	breq .L8
	ldi r24,lo8(1)
	ldi r25,0
.LVL20:
	rjmp .L7
.LVL21:
.L8:
	ldi r24,0
	ldi r25,0
.LVL22:
.L7:
	.loc 1 73 0 discriminator 4
	add r24,r22
	adc r25,r23
/* epilogue start */
	pop r29
	pop r28
.LVL23:
	pop r17
	pop r16
	pop r15
.LVL24:
	pop r14
	ret
	.cfi_endproc
.LFE108:
	.size	_ZN4midi11encodeSysExEPKhPhjb, .-_ZN4midi11encodeSysExEPKhPhjb
	.section	.text._ZN4midi11decodeSysExEPKhPhjb,"ax",@progbits
.global	_ZN4midi11decodeSysExEPKhPhjb
	.type	_ZN4midi11decodeSysExEPKhPhjb, @function
_ZN4midi11decodeSysExEPKhPhjb:
.LFB109:
	.loc 1 91 0
	.cfi_startproc
.LVL25:
	push r13
.LCFI6:
	.cfi_def_cfa_offset 3
	.cfi_offset 13, -2
	push r14
.LCFI7:
	.cfi_def_cfa_offset 4
	.cfi_offset 14, -3
	push r15
.LCFI8:
	.cfi_def_cfa_offset 5
	.cfi_offset 15, -4
	push r16
.LCFI9:
	.cfi_def_cfa_offset 6
	.cfi_offset 16, -5
	push r17
.LCFI10:
	.cfi_def_cfa_offset 7
	.cfi_offset 17, -6
	push r28
.LCFI11:
	.cfi_def_cfa_offset 8
	.cfi_offset 28, -7
	push r29
.LCFI12:
	.cfi_def_cfa_offset 9
	.cfi_offset 29, -8
/* prologue: function */
/* frame size = 0 */
/* stack size = 7 */
.L__stack_usage = 7
	movw r28,r24
.LVL26:
.LBB5:
	.loc 1 96 0
	ldi r30,0
	ldi r31,0
.LBE5:
	.loc 1 94 0
	ldi r25,0
	.loc 1 93 0
	ldi r19,0
	.loc 1 92 0
	ldi r16,0
	ldi r17,0
.LVL27:
.L14:
.LBB8:
	.loc 1 96 0 discriminator 1
	cp r30,r20
	cpc r31,r21
	brsh .L10
.LBB6:
	.loc 1 98 0
	movw r26,r30
	andi r26,7
	clr r27
	or r26,r27
	brne .L11
	.loc 1 100 0
	movw r26,r28
	add r26,r30
	adc r27,r31
	ld r19,X
.LVL28:
	.loc 1 101 0
	ldi r25,lo8(6)
	rjmp .L12
.LVL29:
.L11:
.LBB7:
	.loc 1 105 0
	movw r26,r28
	add r26,r30
	adc r27,r31
	ld r13,X
.LVL30:
	.loc 1 106 0
	tst r18
	breq .L15
	.loc 1 106 0 is_stmt 0 discriminator 1
	ldi r24,lo8(6)
	sub r24,r25
	rjmp .L13
.L15:
	.loc 1 106 0
	mov r24,r25
.L13:
.LVL31:
	.loc 1 107 0 is_stmt 1 discriminator 4
	mov r26,r19
	ldi r27,0
	rjmp 2f
	1:
	asr r27
	ror r26
	2:
	dec r24
	brpl 1b
	lsr r27
	mov r27,r26
	clr r26
	ror r27
	ror r26
.LVL32:
	.loc 1 108 0 discriminator 4
	subi r25,lo8(-(-1))
.LVL33:
	.loc 1 109 0 discriminator 4
	movw r14,r22
	add r14,r16
	adc r15,r17
	or r13,r26
.LVL34:
	movw r26,r14
.LVL35:
	st X,r13
.LVL36:
	subi r16,-1
	sbci r17,-1
.LVL37:
.L12:
.LBE7:
.LBE6:
	.loc 1 96 0 discriminator 2
	adiw r30,1
.LVL38:
	rjmp .L14
.L10:
.LBE8:
	.loc 1 113 0
	movw r24,r16
/* epilogue start */
	pop r29
	pop r28
.LVL39:
	pop r17
	pop r16
.LVL40:
	pop r15
	pop r14
	pop r13
	ret
	.cfi_endproc
.LFE109:
	.size	_ZN4midi11decodeSysExEPKhPhjb, .-_ZN4midi11decodeSysExEPKhPhjb
	.text
.Letext0:
	.file 2 "c:\\program files (x86)\\atmel\\studio\\7.0\\toolchain\\avr8\\avr8-gnu-toolchain\\avr\\include\\stdint.h"
	.file 3 ".././midi_Defs.h"
	.file 4 ".././midi_Namespace.h"
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.long	0x2db
	.word	0x2
	.long	.Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.long	.LASF32
	.byte	0x4
	.long	.LASF33
	.long	.LASF34
	.long	.Ldebug_ranges0+0x30
	.long	0
	.long	0
	.long	.Ldebug_line0
	.uleb128 0x2
	.byte	0x1
	.byte	0x6
	.long	.LASF0
	.uleb128 0x3
	.long	.LASF2
	.byte	0x2
	.byte	0x7e
	.long	0x3b
	.uleb128 0x2
	.byte	0x1
	.byte	0x8
	.long	.LASF1
	.uleb128 0x4
	.byte	0x2
	.byte	0x5
	.string	"int"
	.uleb128 0x3
	.long	.LASF3
	.byte	0x2
	.byte	0x80
	.long	0x54
	.uleb128 0x2
	.byte	0x2
	.byte	0x7
	.long	.LASF4
	.uleb128 0x2
	.byte	0x4
	.byte	0x5
	.long	.LASF5
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.long	.LASF6
	.uleb128 0x2
	.byte	0x8
	.byte	0x5
	.long	.LASF7
	.uleb128 0x2
	.byte	0x8
	.byte	0x7
	.long	.LASF8
	.uleb128 0x3
	.long	.LASF9
	.byte	0x3
	.byte	0x24
	.long	0x30
	.uleb128 0x5
	.long	.LASF35
	.byte	0x4
	.byte	0x24
	.long	0x117
	.uleb128 0x6
	.byte	0x1
	.long	.LASF10
	.byte	0x1
	.byte	0x2e
	.long	.LASF12
	.long	0x54
	.byte	0x1
	.long	0xb7
	.uleb128 0x7
	.long	0x123
	.uleb128 0x7
	.long	0x12e
	.uleb128 0x7
	.long	0x54
	.uleb128 0x7
	.long	0x117
	.byte	0
	.uleb128 0x6
	.byte	0x1
	.long	.LASF11
	.byte	0x1
	.byte	0x57
	.long	.LASF13
	.long	0x54
	.byte	0x1
	.long	0xe1
	.uleb128 0x7
	.long	0x123
	.uleb128 0x7
	.long	0x12e
	.uleb128 0x7
	.long	0x54
	.uleb128 0x7
	.long	0x117
	.byte	0
	.uleb128 0x8
	.long	.LASF14
	.byte	0x3
	.byte	0x33
	.long	0x11e
	.byte	0x1
	.word	0x12c
	.uleb128 0x9
	.long	.LASF15
	.byte	0x3
	.byte	0x3f
	.long	0x2c7
	.byte	0x1
	.byte	0
	.uleb128 0x9
	.long	.LASF16
	.byte	0x3
	.byte	0x40
	.long	0x2c7
	.byte	0x1
	.byte	0x1
	.uleb128 0x9
	.long	.LASF17
	.byte	0x3
	.byte	0x41
	.long	0x2c7
	.byte	0x1
	.byte	0x2
	.byte	0
	.uleb128 0x2
	.byte	0x1
	.byte	0x2
	.long	.LASF18
	.uleb128 0xa
	.long	0x49
	.uleb128 0xb
	.byte	0x2
	.long	0x129
	.uleb128 0xa
	.long	0x77
	.uleb128 0xb
	.byte	0x2
	.long	0x77
	.uleb128 0xc
	.long	0x8d
	.long	.LFB108
	.long	.LFE108
	.long	.LLST0
	.byte	0x1
	.long	0x1f2
	.uleb128 0xd
	.long	.LASF19
	.byte	0x1
	.byte	0x2e
	.long	0x123
	.long	.LLST1
	.uleb128 0xd
	.long	.LASF20
	.byte	0x1
	.byte	0x2f
	.long	0x12e
	.long	.LLST2
	.uleb128 0xe
	.long	.LASF21
	.byte	0x1
	.byte	0x30
	.long	0x54
	.byte	0x6
	.byte	0x64
	.byte	0x93
	.uleb128 0x1
	.byte	0x65
	.byte	0x93
	.uleb128 0x1
	.uleb128 0xd
	.long	.LASF22
	.byte	0x1
	.byte	0x31
	.long	0x117
	.long	.LLST3
	.uleb128 0xf
	.long	.LASF23
	.byte	0x1
	.byte	0x33
	.long	0x54
	.long	.LLST4
	.uleb128 0xf
	.long	.LASF24
	.byte	0x1
	.byte	0x34
	.long	0x77
	.long	.LLST5
	.uleb128 0x10
	.long	.Ldebug_ranges0+0
	.uleb128 0x11
	.string	"i"
	.byte	0x1
	.byte	0x37
	.long	0x54
	.long	.LLST6
	.uleb128 0x12
	.long	.LBB3
	.long	.LBE3
	.uleb128 0xf
	.long	.LASF25
	.byte	0x1
	.byte	0x39
	.long	0x129
	.long	.LLST7
	.uleb128 0x11
	.string	"msb"
	.byte	0x1
	.byte	0x3a
	.long	0x129
	.long	.LLST8
	.uleb128 0xf
	.long	.LASF26
	.byte	0x1
	.byte	0x3b
	.long	0x129
	.long	.LLST9
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0xc
	.long	0xb7
	.long	.LFB109
	.long	.LFE109
	.long	.LLST10
	.byte	0x1
	.long	0x2c0
	.uleb128 0xd
	.long	.LASF27
	.byte	0x1
	.byte	0x57
	.long	0x123
	.long	.LLST11
	.uleb128 0xe
	.long	.LASF28
	.byte	0x1
	.byte	0x58
	.long	0x12e
	.byte	0x6
	.byte	0x66
	.byte	0x93
	.uleb128 0x1
	.byte	0x67
	.byte	0x93
	.uleb128 0x1
	.uleb128 0xe
	.long	.LASF21
	.byte	0x1
	.byte	0x59
	.long	0x54
	.byte	0x6
	.byte	0x64
	.byte	0x93
	.uleb128 0x1
	.byte	0x65
	.byte	0x93
	.uleb128 0x1
	.uleb128 0xe
	.long	.LASF22
	.byte	0x1
	.byte	0x5a
	.long	0x117
	.byte	0x1
	.byte	0x62
	.uleb128 0xf
	.long	.LASF24
	.byte	0x1
	.byte	0x5c
	.long	0x54
	.long	.LLST12
	.uleb128 0xf
	.long	.LASF29
	.byte	0x1
	.byte	0x5d
	.long	0x77
	.long	.LLST13
	.uleb128 0xf
	.long	.LASF30
	.byte	0x1
	.byte	0x5e
	.long	0x77
	.long	.LLST14
	.uleb128 0x10
	.long	.Ldebug_ranges0+0x18
	.uleb128 0x11
	.string	"i"
	.byte	0x1
	.byte	0x60
	.long	0x54
	.long	.LLST15
	.uleb128 0x12
	.long	.LBB7
	.long	.LBE7
	.uleb128 0xf
	.long	.LASF26
	.byte	0x1
	.byte	0x69
	.long	0x129
	.long	.LLST16
	.uleb128 0xf
	.long	.LASF31
	.byte	0x1
	.byte	0x6a
	.long	0x129
	.long	.LLST17
	.uleb128 0x11
	.string	"msb"
	.byte	0x1
	.byte	0x6b
	.long	0x129
	.long	.LLST18
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x13
	.long	0xe1
	.word	0x12c
	.uleb128 0xa
	.long	0x30
	.uleb128 0x14
	.long	0xef
	.byte	0
	.uleb128 0x14
	.long	0xfc
	.byte	0x1
	.uleb128 0x14
	.long	0x109
	.byte	0x2
	.byte	0
	.section	.debug_abbrev,"",@progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1b
	.uleb128 0xe
	.uleb128 0x55
	.uleb128 0x6
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x52
	.uleb128 0x1
	.uleb128 0x10
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0
	.byte	0
	.uleb128 0x3
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x4
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0x39
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x2007
	.uleb128 0xe
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3c
	.uleb128 0xc
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0x5
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3c
	.uleb128 0xc
	.uleb128 0x1c
	.uleb128 0x5
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3c
	.uleb128 0xc
	.uleb128 0x1c
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0x26
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xb
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xc
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x47
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0x6
	.uleb128 0x2117
	.uleb128 0xc
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xd
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0xe
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0xf
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0x10
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x55
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0x11
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0x12
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.byte	0
	.byte	0
	.uleb128 0x13
	.uleb128 0x34
	.byte	0
	.uleb128 0x47
	.uleb128 0x13
	.uleb128 0x1c
	.uleb128 0x5
	.byte	0
	.byte	0
	.uleb128 0x14
	.uleb128 0x34
	.byte	0
	.uleb128 0x47
	.uleb128 0x13
	.uleb128 0x1c
	.uleb128 0xb
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_loc,"",@progbits
.Ldebug_loc0:
.LLST0:
	.long	.LFB108
	.long	.LCFI0
	.word	0x3
	.byte	0x92
	.uleb128 0x20
	.sleb128 2
	.long	.LCFI0
	.long	.LCFI1
	.word	0x3
	.byte	0x92
	.uleb128 0x20
	.sleb128 3
	.long	.LCFI1
	.long	.LCFI2
	.word	0x3
	.byte	0x92
	.uleb128 0x20
	.sleb128 4
	.long	.LCFI2
	.long	.LCFI3
	.word	0x3
	.byte	0x92
	.uleb128 0x20
	.sleb128 5
	.long	.LCFI3
	.long	.LCFI4
	.word	0x3
	.byte	0x92
	.uleb128 0x20
	.sleb128 6
	.long	.LCFI4
	.long	.LCFI5
	.word	0x3
	.byte	0x92
	.uleb128 0x20
	.sleb128 7
	.long	.LCFI5
	.long	.LFE108
	.word	0x3
	.byte	0x92
	.uleb128 0x20
	.sleb128 8
	.long	0
	.long	0
.LLST1:
	.long	.LVL0
	.long	.LVL20
	.word	0x6
	.byte	0x68
	.byte	0x93
	.uleb128 0x1
	.byte	0x69
	.byte	0x93
	.uleb128 0x1
	.long	.LVL20
	.long	.LVL21
	.word	0x4
	.byte	0xf3
	.uleb128 0x1
	.byte	0x68
	.byte	0x9f
	.long	.LVL21
	.long	.LVL22
	.word	0x6
	.byte	0x68
	.byte	0x93
	.uleb128 0x1
	.byte	0x69
	.byte	0x93
	.uleb128 0x1
	.long	.LVL22
	.long	.LFE108
	.word	0x4
	.byte	0xf3
	.uleb128 0x1
	.byte	0x68
	.byte	0x9f
	.long	0
	.long	0
.LLST2:
	.long	.LVL0
	.long	.LVL3
	.word	0x6
	.byte	0x66
	.byte	0x93
	.uleb128 0x1
	.byte	0x67
	.byte	0x93
	.uleb128 0x1
	.long	.LVL3
	.long	.LVL12
	.word	0x6
	.byte	0x6c
	.byte	0x93
	.uleb128 0x1
	.byte	0x6d
	.byte	0x93
	.uleb128 0x1
	.long	.LVL12
	.long	.LVL15
	.word	0x3
	.byte	0x8c
	.sleb128 8
	.byte	0x9f
	.long	.LVL15
	.long	.LVL23
	.word	0x6
	.byte	0x6c
	.byte	0x93
	.uleb128 0x1
	.byte	0x6d
	.byte	0x93
	.uleb128 0x1
	.long	0
	.long	0
.LLST3:
	.long	.LVL0
	.long	.LVL3
	.word	0x1
	.byte	0x62
	.long	.LVL3
	.long	.LVL24
	.word	0x1
	.byte	0x5f
	.long	.LVL24
	.long	.LFE108
	.word	0x4
	.byte	0xf3
	.uleb128 0x1
	.byte	0x62
	.byte	0x9f
	.long	0
	.long	0
.LLST4:
	.long	.LVL1
	.long	.LVL3
	.word	0x2
	.byte	0x30
	.byte	0x9f
	.long	.LVL3
	.long	.LVL19
	.word	0x6
	.byte	0x66
	.byte	0x93
	.uleb128 0x1
	.byte	0x67
	.byte	0x93
	.uleb128 0x1
	.long	0
	.long	0
.LLST5:
	.long	.LVL1
	.long	.LVL3
	.word	0x2
	.byte	0x30
	.byte	0x9f
	.long	.LVL3
	.long	.LVL11
	.word	0x1
	.byte	0x63
	.long	.LVL11
	.long	.LVL14
	.word	0x1
	.byte	0x6e
	.long	.LVL14
	.long	.LVL16
	.word	0x2
	.byte	0x30
	.byte	0x9f
	.long	.LVL16
	.long	.LVL18
	.word	0x1
	.byte	0x6e
	.long	.LVL18
	.long	.LFE108
	.word	0x1
	.byte	0x63
	.long	0
	.long	0
.LLST6:
	.long	.LVL2
	.long	.LVL3
	.word	0x2
	.byte	0x30
	.byte	0x9f
	.long	.LVL3
	.long	.LFE108
	.word	0x6
	.byte	0x6a
	.byte	0x93
	.uleb128 0x1
	.byte	0x6b
	.byte	0x93
	.uleb128 0x1
	.long	0
	.long	0
.LLST7:
	.long	.LVL4
	.long	.LVL8
	.word	0x1
	.byte	0x6f
	.long	.LVL8
	.long	.LVL9
	.word	0x1
	.byte	0x60
	.long	.LVL9
	.long	.LVL10
	.word	0x1
	.byte	0x6f
	.long	0
	.long	0
.LLST8:
	.long	.LVL5
	.long	.LVL7
	.word	0x5
	.byte	0x80
	.sleb128 0
	.byte	0x37
	.byte	0x26
	.byte	0x9f
	.long	.LVL8
	.long	.LVL9
	.word	0x5
	.byte	0x80
	.sleb128 0
	.byte	0x37
	.byte	0x26
	.byte	0x9f
	.long	0
	.long	0
.LLST9:
	.long	.LVL6
	.long	.LVL18
	.word	0x1
	.byte	0x62
	.long	0
	.long	0
.LLST10:
	.long	.LFB109
	.long	.LCFI6
	.word	0x3
	.byte	0x92
	.uleb128 0x20
	.sleb128 2
	.long	.LCFI6
	.long	.LCFI7
	.word	0x3
	.byte	0x92
	.uleb128 0x20
	.sleb128 3
	.long	.LCFI7
	.long	.LCFI8
	.word	0x3
	.byte	0x92
	.uleb128 0x20
	.sleb128 4
	.long	.LCFI8
	.long	.LCFI9
	.word	0x3
	.byte	0x92
	.uleb128 0x20
	.sleb128 5
	.long	.LCFI9
	.long	.LCFI10
	.word	0x3
	.byte	0x92
	.uleb128 0x20
	.sleb128 6
	.long	.LCFI10
	.long	.LCFI11
	.word	0x3
	.byte	0x92
	.uleb128 0x20
	.sleb128 7
	.long	.LCFI11
	.long	.LCFI12
	.word	0x3
	.byte	0x92
	.uleb128 0x20
	.sleb128 8
	.long	.LCFI12
	.long	.LFE109
	.word	0x3
	.byte	0x92
	.uleb128 0x20
	.sleb128 9
	.long	0
	.long	0
.LLST11:
	.long	.LVL25
	.long	.LVL27
	.word	0x6
	.byte	0x68
	.byte	0x93
	.uleb128 0x1
	.byte	0x69
	.byte	0x93
	.uleb128 0x1
	.long	.LVL27
	.long	.LVL39
	.word	0x6
	.byte	0x6c
	.byte	0x93
	.uleb128 0x1
	.byte	0x6d
	.byte	0x93
	.uleb128 0x1
	.long	.LVL39
	.long	.LFE109
	.word	0x4
	.byte	0xf3
	.uleb128 0x1
	.byte	0x68
	.byte	0x9f
	.long	0
	.long	0
.LLST12:
	.long	.LVL26
	.long	.LVL27
	.word	0x2
	.byte	0x30
	.byte	0x9f
	.long	.LVL27
	.long	.LVL33
	.word	0x6
	.byte	0x60
	.byte	0x93
	.uleb128 0x1
	.byte	0x61
	.byte	0x93
	.uleb128 0x1
	.long	.LVL33
	.long	.LVL37
	.word	0x3
	.byte	0x80
	.sleb128 1
	.byte	0x9f
	.long	.LVL37
	.long	.LVL40
	.word	0x6
	.byte	0x60
	.byte	0x93
	.uleb128 0x1
	.byte	0x61
	.byte	0x93
	.uleb128 0x1
	.long	.LVL40
	.long	.LFE109
	.word	0x6
	.byte	0x68
	.byte	0x93
	.uleb128 0x1
	.byte	0x69
	.byte	0x93
	.uleb128 0x1
	.long	0
	.long	0
.LLST13:
	.long	.LVL26
	.long	.LVL27
	.word	0x2
	.byte	0x30
	.byte	0x9f
	.long	.LVL27
	.long	.LFE109
	.word	0x1
	.byte	0x63
	.long	0
	.long	0
.LLST14:
	.long	.LVL26
	.long	.LVL27
	.word	0x2
	.byte	0x30
	.byte	0x9f
	.long	.LVL27
	.long	.LVL28
	.word	0x1
	.byte	0x69
	.long	.LVL28
	.long	.LVL29
	.word	0x2
	.byte	0x36
	.byte	0x9f
	.long	.LVL29
	.long	.LFE109
	.word	0x1
	.byte	0x69
	.long	0
	.long	0
.LLST15:
	.long	.LVL26
	.long	.LVL27
	.word	0x2
	.byte	0x30
	.byte	0x9f
	.long	.LVL27
	.long	.LFE109
	.word	0x6
	.byte	0x6e
	.byte	0x93
	.uleb128 0x1
	.byte	0x6f
	.byte	0x93
	.uleb128 0x1
	.long	0
	.long	0
.LLST16:
	.long	.LVL30
	.long	.LVL34
	.word	0x1
	.byte	0x5d
	.long	.LVL34
	.long	.LVL36
	.word	0x5
	.byte	0x8c
	.sleb128 0
	.byte	0x8e
	.sleb128 0
	.byte	0x22
	.long	0
	.long	0
.LLST17:
	.long	.LVL31
	.long	.LVL37
	.word	0x1
	.byte	0x68
	.long	0
	.long	0
.LLST18:
	.long	.LVL32
	.long	.LVL35
	.word	0x1
	.byte	0x6a
	.long	0
	.long	0
	.section	.debug_aranges,"",@progbits
	.long	0x24
	.word	0x2
	.long	.Ldebug_info0
	.byte	0x4
	.byte	0
	.word	0
	.word	0
	.long	.LFB108
	.long	.LFE108-.LFB108
	.long	.LFB109
	.long	.LFE109-.LFB109
	.long	0
	.long	0
	.section	.debug_ranges,"",@progbits
.Ldebug_ranges0:
	.long	.LBB2
	.long	.LBE2
	.long	.LBB4
	.long	.LBE4
	.long	0
	.long	0
	.long	.LBB5
	.long	.LBE5
	.long	.LBB8
	.long	.LBE8
	.long	0
	.long	0
	.long	.LFB108
	.long	.LFE108
	.long	.LFB109
	.long	.LFE109
	.long	0
	.long	0
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.section	.debug_str,"MS",@progbits,1
.LASF31:
	.string	"shift"
.LASF30:
	.string	"byteIndex"
.LASF13:
	.string	"_ZN4midi11decodeSysExEPKhPhjb"
.LASF24:
	.string	"count"
.LASF18:
	.string	"bool"
.LASF21:
	.string	"inLength"
.LASF10:
	.string	"encodeSysEx"
.LASF28:
	.string	"outData"
.LASF9:
	.string	"byte"
.LASF25:
	.string	"data"
.LASF1:
	.string	"unsigned char"
.LASF23:
	.string	"outLength"
.LASF27:
	.string	"inSysEx"
.LASF6:
	.string	"long unsigned int"
.LASF12:
	.string	"_ZN4midi11encodeSysExEPKhPhjb"
.LASF29:
	.string	"msbStorage"
.LASF15:
	.string	"ErrorParse"
.LASF26:
	.string	"body"
.LASF4:
	.string	"unsigned int"
.LASF35:
	.string	"midi"
.LASF8:
	.string	"long long unsigned int"
.LASF2:
	.string	"uint8_t"
.LASF33:
	.string	".././MIDI.cpp"
.LASF32:
	.string	"GNU C++11 5.4.0 -mn-flash=1 -mmcu=avr5 -g2 -Og -std=c++11 -funsigned-char -funsigned-bitfields -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -mn-flash=1 -mno-skip-bug -fno-rtti -fno-enforce-eh-specs -fno-exceptions"
.LASF11:
	.string	"decodeSysEx"
.LASF22:
	.string	"inFlipHeaderBits"
.LASF20:
	.string	"outSysEx"
.LASF17:
	.string	"WarningSplitSysEx"
.LASF7:
	.string	"long long int"
.LASF3:
	.string	"uint16_t"
.LASF19:
	.string	"inData"
.LASF5:
	.string	"long int"
.LASF0:
	.string	"signed char"
.LASF34:
	.string	"C:\\\\Users\\\\mkfar\\\\Desktop\\\\Enlistments\\\\Microchip-Studio-Projects\\\\MAFD-repo\\\\Debug"
.LASF16:
	.string	"ErrorActiveSensingTimeout"
.LASF14:
	.string	"ActiveSensingTimeout"
	.ident	"GCC: (AVR_8_bit_GNU_Toolchain_3.6.2_1778) 5.4.0"
