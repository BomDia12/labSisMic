#include <msp430.h>

typedef unsigned char u8;
typedef unsigned int u16;

// Pinos P4.4 - TX
//       P4.5 - RX
// Fb = 9600
// 1) BRW = 2^20/9600 = 109,23
//    BRW = 109
//    BRS = 0,23 x 8 = 1,8 => 2

uartPrint(char * string)
{
    while(*string)
    {
        while(!(UCA1IFG & UCTXIFG));
        UCA1TXBUF = *string++;
    }
}

u8 ready;

void main()
{
    UCA1CTL1  = UCSWRST;        // RST a interface
    UCA1CTL1 |= UCSSEL__SMCLK;  // SM@2^20 Hz
    UCA1BRW   = 109;
    UCA1MCTL  = UCBRF_2;
    P4SEL    |= BIT4 | BIT5;
    UCA1CTL1 &= ~UCSWRST;
    UCA1IE = UCRXIE;

    __enable_interrupt();

    while(1)
    {

        uartPrint("recebido!\n\r");

    }
}


u8 rxBuffer[20];

#pragma vector = 46 //USCI A1
__interrupt void uart_isr()
{
    static u8 i = 0;
    u8 c;

    c = UCA1RXBUF;
    UCA1TXBUF = c;
    rxBuffer[i++] = c;

    if(i==20)
        i = 0;

    if(c == 0x0D)
        ready = 1;


}
