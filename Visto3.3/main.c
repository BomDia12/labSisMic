#include <msp430.h> 
#include <stdint.h>

int waiting_conversion = 0;
volatile uint16_t data;

uint16_t adc_read(uint8_t pin) {
    switch(pin) {
    case 0:
        ADC12MCTL0 |= ADC12INCH_0;
        break;
    }

    ADC12CTL0 |= ADC12SC;
    volatile int i = 0;
    while (i < 1000) { i++; }
    ADC12CTL0 &= ~ADC12SC;

    waiting_conversion = 1;
    while (waiting_conversion) {}

    uint16_t out;

    switch(pin) {
    case 0:
        out = ADC12MEM0;
        break;
    }

    return out;
}

/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	// config port
	P6SEL |= 0xffff;
	P6DIR &= 0x0000;

	ADC12CTL0 = ADC12SHT1_15 | ADC12SHT0_15 | ADC12MSC | ADC12ON | ADC12ENC;
	ADC12CTL1 = ADC12CONSEQ_0;
	ADC12CTL2 = ADC12RES_0;
	ADC12IE = 1;
	ADC12MCTL0 = ADC12EOS | ADC12INCH_0;

	__enable_interrupt();

	while (0XFFFF) {
	    data = adc_read(0);
	}
}

#pragma vector = ADC12_VECTOR
__interrupt void adc_interrupt() {
    waiting_conversion = 0;
    ADC12IFG = 0;
}
