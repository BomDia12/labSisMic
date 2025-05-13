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

            mov.w #vector, r13
            mov.w &size, r12
            add.w #-1, r12
            rla.w r12

main_loop:	mov.w #0, r4	;; index do inner loop
			call #inner_loop
			add.w #-2, r12
			jnz main_loop

			jmp $

inner_loop:	mov.w r13, r5
			add.w r4, r5
			mov.w 0(r5), r6
			mov.w 2(r5), r7
			cmp.w r6, r7
			jl change
return:		add.w #2, r4
			cmp.w r4, r12
			jne	inner_loop
			ret

change:		mov.w r7, 0(r5)
			mov.w r6, 2(r5)
			jmp return

;-------------------------------------------------------------------------------
; Segmentos de dados
;-------------------------------------------------------------------------------

			.data
vector:		.word	3, 2, 1, 5, 7, 0
size:		.word	6

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
            
