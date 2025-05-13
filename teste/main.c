#include <msp430.h> 



void wait_for_signal() {
    while ((TA1CCTL1 & CCIFG) == 0) {}
    TA1CCTL1 &= ~CCIFG;
    while ((TA1CCTL1 & CCIFG) == 0) {}
    TA1CCTL1 &= ~CCIFG;
};

int capture_bit() {
    while ((TA1CCTL1 & CCIFG) == 0) {}
    TA1CCTL1 &= ~CCIFG;
    return TA1CCR1;
}

int* capture_signal() {
    int signal[32];

    int i = 0;

    while (i < 32) {

        volatile int t0 = TA1CCR1;
        volatile int t1 = capture_bit();
        signal[i] = t1 - t0;

        i++;
    }

    return signal;
};

void process_signal(int signal[32]) {
    unsigned long out = 0;
    unsigned long mask = 0x01;

    int i = 0;
    while (i < 32) {
        if (signal[i] > 2000) {
            out += (1 << i);
        }

        i++;
    }

    switch (out) {
    case 14660:
        P1OUT |= BIT0;
        break;
    case 12429:
        P1OUT &= ~BIT0;
        break;
    case 11919:
        P1OUT ^= BIT0;
        break;
    case 13449:
        P4OUT |= BIT0;
        break;
    case 15489:
        P4OUT &= ~BIT7;
        break;
    case 13959:
        P4OUT ^=BIT7;
        break;
    }
}

/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

	// LED RED
	P1DIR |= BIT0;
	P1OUT |= BIT0;

	// LED GREEN
	P4DIR |= BIT7;
	P4OUT |= BIT7;

	// Setup GPIO in
	P2DIR &= ~BIT0;
	P2REN |= BIT0;
	P2IE |= BIT0;
	P2IES |= BIT0;
	P2OUT |= BIT0;
	P2SEL |= BIT0;
	P2IFG &= (~BIT0); // Clear interrupt flag for P1.3

	// Setup Timer
	TA1CTL = TASSEL_2 + MC_2 + TACLR;
	TA1CCTL1 = CM1 + CCIE + CAP;
	TA1CCTL1 &= ~CCIFG;

	while (1) {
	    wait_for_signal();
	    int *signal = capture_signal();
	    process_signal(signal);
	}
}
