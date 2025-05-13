#include <msp430.h> 
#define BUTTON1 42
#define BUTTON2 47

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

void config_timer() {
    TA1CTL = TASSEL_1 | MC_1;
    TA1CCR0 = 256; // makes the cycle 128 Hz
    TA1CCTL1 = OUTMOD_3;

    // Config output
    P2DIR |= BIT0;
    P2SEL |= BIT0;
    P2REN &= ~BIT0;
}

int * get_step() {
    static int step;
    return &step;
}

void set_duty_cycle() {
    int * step = get_step();

    int amount = *step * 32;
    TA1CCR1 = amount;
}

void increment_step() {
    int * step = get_step();
    (*step)++;

    if (*step >= 8) {
        *step = 8;
    }

    set_duty_cycle();
}

void decrement_step() {
    int * step = get_step();
    (*step)--;

    if (*step <= 1) {
        *step = 1;
    }

    set_duty_cycle();
}

/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	int * step = get_step();
	*step = 6;

	config_buttons();
	//config_leds();
	config_timer();
	set_duty_cycle();

	__enable_interrupt();

	while (1) {}
}

#pragma vector = PORT1_VECTOR
__interrupt void button_1_interrupt() {
    if (P1IFG == BIT1) {
        decrement_step();
    }
    P1IFG = 0;
}

#pragma vector = PORT2_VECTOR
__interrupt void button_2_interrupt() {
    if (P2IFG == BIT1) {
        increment_step();
    }
    P2IFG = 0;
}
