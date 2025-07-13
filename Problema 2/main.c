#include <msp430.h>

typedef enum {
    true = 1,
    false = 0
} bool;

int cap; // capture signals
bool started, new;

typedef enum {
    CH_MINUS,
    CH,
    CH_PLUS,
    PREV,
    NEXT,
    PLAY,
    MINUS,
    PLUS,
    EQ,
    _0,
    _100,
    _200,
    _1,
    _2,
    _3,
    _4,
    _5,
    _6,
    _7,
    _8,
    _9,
    error
} Button;

// SMCLK -> 1048576
void config_timers() {
    TA1CTL = TASSEL_2 + MC_1;
    TA1CCR0 = 15728; // ~15ms
    TA1CCTL1 = CM_2 + CCIE + CAP;
}

void config_port() {
    P2DIR &= ~BIT0;
    P2REN |= BIT0;
    P2OUT |= BIT0;
    P2SEL |= BIT0;
}


void config_leds() {
    P1DIR |= BIT0;
    P1OUT |= BIT0;

    P4DIR |= BIT7;
    P4OUT |= BIT7;
}

int capture_signal() {
     while (new != true) {}
    new = false;
    return cap;
}

void capture_start() {
    while (1) {
        capture_signal();
        int signal = capture_signal();

        if (signal > 13631) { // more then 13 ms
            break;
        }
    }
}

char capture_bit() {
    long time = capture_signal();

    if (time > 1600) { // more than 2 seconds
        return 1;
    }

    return 0;
}

char capture_byte() {
    char byte = 0;
    int i = 0;
    while (i < 8) {
        char bit = capture_bit();
        byte = byte + bit;
        if (i < 7) {
            byte = byte << 1;
        }
        i++;
    }

    return byte;
}

Button get_button_from_byte(char byte) {
    switch (byte) {
    case 0xa2:
        return CH_MINUS;
    case 0x62:
        return CH;
    case 0xe2:
        return CH_PLUS;
    case 0x22:
        return PREV;
    case 0x02:
        return NEXT;
    }

    return _0;
}

void control_leds(Button button) {
    switch (button) {
    case CH_MINUS:
        P1OUT |= BIT0;
        P4OUT |= BIT7;
        return;
    case CH:
        P1OUT &= ~BIT0;
        P4OUT &= ~BIT7;
        return;
    case CH_PLUS:
        P1OUT &= ~BIT0;
        P4OUT |= BIT7;
        return;
    case PREV:
        P1OUT |= BIT0;
        P4OUT &= ~BIT7;
    }
}

/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	config_timers();
	config_port();
	config_leds();

	volatile Button curr = _0;

    __enable_interrupt();

	while (1) {
	    capture_start();
	    char addr = capture_byte();
	    char not_addr = capture_byte();
	    char cmd = capture_byte();
	    char not_cmd = capture_byte();

	    curr = get_button_from_byte(cmd);
	    control_leds(curr);
	}
}

#pragma vector = 48 // TA1CCR1
__interrupt void timer_interrupt() {
    TA1CTL |= TACLR;
    new = true;
    cap = TA1CCR1;
    TA1CCTL1 &= ~CCIFG;
}
