#include <msp430.h> 


void config_buttons() {
    P1DIR &= ~BIT1;
    P1REN |= BIT1;
    P1OUT |= BIT1;
    P1IE = BIT1;

    P2DIR &= ~BIT1;
    P2REN |= BIT1;
    P2OUT |= BIT1;
    P2IE = BIT1;
}

void config_leds() {
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;

    P4DIR |= BIT7;
    P4OUT &= ~BIT7;
}

int * get_counter() {
    static int counter;
    return &counter;
}

void update_counter() {
    int * counter = get_counter();
    switch (*counter) {
    case 0:
        P1OUT &= ~BIT0;
        P4OUT &= ~BIT7;
        break;
    case 1:
        P1OUT |= BIT0;
        P4OUT &= ~BIT7;
        break;
    case 2:
        P1OUT &= ~BIT0;
        P4OUT |= BIT7;
        break;
    case 3:
        P1OUT |= BIT0;
        P4OUT |= BIT7;
    }
}

void increment_counter() {
    int * counter = get_counter();
    *counter += 1;

    if (*counter >= 4) {
        *counter = 0;
    }

    update_counter();
}

void decrement_counter() {
    int * counter = get_counter();
    *counter -= 1;

    if (*counter <= -1) {
        *counter = 4;
    }

    update_counter();
}

void debounce(void){
    volatile int i = 0;
    while(i < 10000) {
        i++;
    }
}

/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	config_buttons();
	config_leds();
	int * counter = get_counter();
	*counter = 0;

	__enable_interrupt();

	return 0;
}

#pragma vector = PORT1_VECTOR
__interrupt void button_1_interrupt() {
    if (P1IFG == BIT1) {
        decrement_counter();
    }
    debounce();
    P1IFG = 0;
}

#pragma vector = PORT2_VECTOR
__interrupt void button_2_interrupt() {
    if (P2IFG == BIT1) {
        increment_counter();
    }
    debounce();
    P2IFG = 0;
}
