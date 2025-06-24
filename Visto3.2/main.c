#include <msp430.h>
#include <stdint.h>

int waiting;

void config_timers() {
    TA0CTL = TASSEL_1 + MC_0 + TAIE;
}

void wait_time(uint8_t time) {
    TA0CCR0 = time;
    TA0CTL |= MC_2;
    waiting = 1;
    while(waiting) {}
    TA0CTL |= MC_0;
}

void config_i2c() {
    // config out pins
    P3DIR &= ~BIT0 & ~BIT1;
    P3REN |= BIT0 | BIT1;
    P3OUT |= BIT0 | BIT1;
    P3SEL |= BIT0 | BIT1;

    UCB0CTL1 |= UCSWRST; // enter reset mode

    UCB0CTL0 |= UCMST + UCMODE_3 + UCSYNC; // master + i2c
    UCB0CTL1 |= UCSSEL_2; // SMCLK
    UCB0BR0 = 12;

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

void lcd_write_nibble(uint8_t nibble, uint8_t is_char) {
    uint8_t addr = 0x27;
    uint8_t data = ((nibble & 0x0f) << 4) + 0x08;
    if (is_char) {
        data += 1;
    }

    i2c_send(addr, data);
    i2c_send(addr, data | 0b0100); // enable
    i2c_send(addr, data);
}

void lcd_write_byte(uint8_t byte, uint8_t is_char) {
    lcd_write_nibble(byte >> 4, is_char);
    lcd_write_nibble(byte, is_char);
}

void lcd_init() {
    // reset to 8 bit mode
    lcd_write_nibble(0x03, 0);
    lcd_write_nibble(0x03, 0);
    lcd_write_nibble(0x03, 0);

    // start 4 bit mode
    lcd_write_nibble(0x02, 0);

    lcd_write_byte(0x01, 0); // apaga o display
    wait_time(66); // wait ~2ms

    lcd_write_byte(0x28, 0); // 2 line display
    lcd_write_byte(0x0f, 0);
}

void lcd_write(char * str) {
    char *ptr = str;
    int counter = 0;
    while (*ptr != '\0') {
        lcd_write_byte(*ptr, 1);
        ptr++;
        counter++;
        if (counter == 16) {
            lcd_write_byte(0xb8, 0);
            counter = 0;
        }
    }
}

/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    config_i2c();
    config_timers();

    __enable_interrupt();

    lcd_init();
    lcd_write("01234567890123456789");

    while (1) {}
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void timer_inturrupt() {
    TA0CTL &= ~TAIFG;
    waiting = 0;
}
