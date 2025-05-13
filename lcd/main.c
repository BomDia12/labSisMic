#include <msp430.h> 
#include <stdint.h>

void i2cInit(void) {
 // Configurar pinos P3.0 (SDA) e P3.1 (SCL) para I2C
 P3SEL |= BIT0 + BIT1; // Seleciona função I2C para os pinos
 P3REN |= BIT0 + BIT1; // Habilita resistores
 P3OUT |= BIT0 + BIT1; // Pull-up

 // Configurar USCI_B0 para modo I2C Master
 UCB0CTL1 |= UCSWRST;  // Coloca em reset durante config

 UCB0CTL0 = UCMST +  // Modo Mestre
  UCMODE_3 + // Modo I2C
  UCSYNC;  // Modo Síncrono

 UCB0CTL1 = UCSWRST +  // Mantém em reset
  UCSSEL_2;  // SMCLK como fonte

 // Configura clock para 100kHz (SMCLK=1MHz)
 UCB0BR0 = 10;  // Divisor = 10
 UCB0BR1 = 0;

 UCB0CTL1 &= ~UCSWRST; // Remove do reset
}

uint8_t i2cSend(uint8_t addr, uint8_t data) {
 // 1. Escreve o endereço do escravo
 UCB0I2CSA = addr;

 // 2. Indica que vamos transmitir e pede start
 UCB0CTL1 |= UCTR;  // Transmitter mode
 UCB0CTL1 |= UCTXSTT;  // Gera START

 // 3. Aguarda TXIFG=1 para enviar o byte
 while(!(UCB0IFG & UCTXIFG));
 UCB0TXBUF = data;

 // 4. Aguarda START ser enviado (STT=0)
 while(UCB0CTL1 & UCTXSTT);

 // 5. Se houve NACK, gera STOP e retorna erro
 if(UCB0IFG & UCNACKIFG) {
  UCB0CTL1 |= UCTXSTP; // Gera STOP
  while(UCB0CTL1 & UCTXSTP); // Aguarda STOP
  UCB0IFG &= ~UCNACKIFG; // Limpa flag de NACK
  return 1; // Retorna NACK
 }

 // 6. Aguarda byte ser transmitido
 while(!(UCB0IFG & UCTXIFG));

 // 7. Gera STOP
 UCB0CTL1 |= UCTXSTP;
 while(UCB0CTL1 & UCTXSTP); // Aguarda STOP

 return 0; // Retorna ACK
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    i2cInit();

    int res = i2cSend(0x20, 0x3);

    res = res + 1;

    return 0;
}
