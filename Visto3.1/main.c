#include <msp430.h>
#include <stdint.h>

void config_i2c() {
    // config out pins
    P3DIR &= ~BIT0 & ~BIT1;
    P3REN |= BIT0 | BIT1;
    P3OUT |= BIT0 | BIT1;
    P3SEL |= BIT0 | BIT1;

    UCB0CTL1 |= UCSWRST; // enter reset mode

    UCB0CTL0 |= UCMST + UCMODE_3 + UCSYNC; // master + i2c
    UCB0CTL1 |= UCSSEL_2; // SMCLK
    UCB0BBR0 = 12;

    UCB0CTL1 &= ~UCSWRST; // leave reset mode
}

uint8_t i2c_send(uint8_t addr, uint8_t data) {
    UCB0I2CSA = addr;

    UCB0CTL1 |= UCTR | UCTXSTT; // transmit mode + start

    while (!(UCB0IFG & UCTXIFG)) {} // wait until transmit

    UCB0TXBUF = data;

    while (UCB0CTL1 & UCTXSTT) {} // wait until start = 0


    UCB0CTL1 |= UCTXSTP; // stop
    UCB0IFG &= UCTXIFG;

    while (UCB0CTL1 & UCTXSTP) {}

    return UCB0IFG & UCNACKIFG;
}

/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	config_i2c();

	i2c_send(0x27, 0x00);

	return 0;
}
