#include <msp430.h> 


/**
 * main.c
 */
void debounce(void){
    volatile int i = 0;
    while(i < 10000) {
        i++;
    }
}

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	P1DIR &= ~BIT1;
	P1REN |= BIT1;
	P1OUT |= BIT1;

	P1DIR |= BIT0;

	while(1) {
	    while(P1IN & BIT1) {}
	    debounce();
	    P1OUT ^= BIT0;
        while(!(P1IN & BIT1)) {}
        debounce();
	}
}
