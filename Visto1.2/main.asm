;-------------------------------------------------------------------------------
; MSP430 Assembler Code Template for use with TI Code Composer Studio
;
;
;-------------------------------------------------------------------------------
            .cdecls C,LIST,"msp430.h"       ; Include device header file
            
;-------------------------------------------------------------------------------
            .def    RESET                   ; Export program entry-point to
                                            ; make it known to linker.
;-------------------------------------------------------------------------------
            .text                           ; Assemble into program memory.
            .retain                         ; Override ELF conditional linking
                                            ; and retain current section.
            .retainrefs                     ; And retain any sections that have
                                            ; references to current section.

;-------------------------------------------------------------------------------
RESET       mov.w   #__STACK_END,SP         ; Initialize stackpointer
StopWDT     mov.w   #WDTPW|WDTHOLD,&WDTCTL  ; Stop watchdog timer


;-------------------------------------------------------------------------------
; Main loop here
;-------------------------------------------------------------------------------

			mov.w #0xA02B, r12
			mov.w r12, r4
			mov.w #4, r6
			mov.w #out, r13
			add.w #3, r13

loop_start:	call #nib_asc
			mov.b r5, 0(r13)
			add.w #-1, r13
			rra r4
			rra r4
			rra r4
			rra r4
			add.w #-1, r6
			jnz loop_start

			jmp $


nib_asc:	mov.w #0x000F, r5
			and.w r4, r5
			cmp #0xA, r5
			jge greater
			add.b #0x30, r5
			ret

greater:	add.b #0x37, r5
			ret

;-------------------------------------------------------------------------------
; Segmentos de dados
;-------------------------------------------------------------------------------

			.data
mask:		.word	0x0001
out:		.byte	0, 0, 0, 0
                                            

;-------------------------------------------------------------------------------
; Stack Pointer definition
;-------------------------------------------------------------------------------
            .global __STACK_END
            .sect   .stack
            
;-------------------------------------------------------------------------------
; Interrupt Vectors
;-------------------------------------------------------------------------------
            .sect   ".reset"                ; MSP430 RESET Vector
            .short  RESET
            
