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


			call #fib32
			jmp $


fib32:		mov.w #0, r4 ; lower  1
			mov.w #0, r5 ; higher 1
			mov.w #1, r6 ; lower  2
			mov.w #0, r7 ; higher 2

loop:		mov.w r4, r12
			mov.w r5, r13
			add.w r6, r12
			addc.w r7, r13
			cmp.w r7, r13
			jl end
			mov.w r6, r4
			mov.w r7, r5
			mov.w r12, r6
			mov.w r13, r7
			jmp loop

end:		mov.w r7, r13
			mov.w r6, r12
			ret
                                            

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
            
