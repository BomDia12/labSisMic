#include <msp430.h> 

int cap_1, cap_2; // capture signals
long t_hi, t_lo; // high and low signals

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
    _9
} Button;

// SMCLK -> 1048576
void config_timers() {
    TA1CTL = TASSEL_2 + MC_1;
    TA1CCR0 = 14680; // ~14ms
    TA1CCTL1 = CM_2 + CAP;
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

long capture_signal() {
    while((TA1CCTL1 & CCIFG) == 0) {};
    cap_1 = TA1CCR1;
    TA0CCTL1 &= ~CCIFG;

    while((TA1CCTL1 & CCIFG) == 0) {};
    cap_2 = TA1CCR1;
    TA0CCTL1 &= ~CCIFG;

    long diff = cap_2 - cap_1;
    if (diff < 0) {
        diff += 65536L;
    }

    return diff;
}

void capture_start() {
    while (1) {
        long signal = capture_signal();

        if (signal > 1363) { // more then 13 ms
            break;
        }
    }
}

char capture_bit() {
    long time = capture_signal();

    if (time > 209) { // more than 2 seconds
        return 1;
    }

    return 0;
}

char capture_byte() {
    char byte = 0;
    int i = 0;
    while (i < 8) {
        char bit = capture_bit();
        byte += bit;
        byte = byte << 1;
        i++;
    }

    return byte;
}

Button get_button_from_byte(char byte) {
    if (byte == 0xffa25d) {
        return CH_MINUS;
    }

    return _0;
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

	while (1) {
	    capture_start();
	    char addr = capture_byte();
	    char not_addr = capture_byte();
	    char cmd = capture_byte();
	    char not_cmd = capture_byte();

	    if (addr != ~not_addr) {
	        continue;
	    }

	    if (cmd != ~not_cmd) {
	        continue;
	    }

	    curr = get_button_from_byte(cmd);
	}
}
