#include <msp430.h>
#include <stdint.h>
#include <string.h>

typedef enum {
    true = 1,
    false = 0
} bool;

typedef enum {
    QUESTION,
    CORRECT,
    WRONG,
    OVER
} State;

typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    OK,
    _HASH,
    _ASTERISK,
    _0,
    _1,
    _2,
    _3,
    _4,
    _5,
    _6,
    _7,
    _8,
    _9,
    ERROR
} Button;

volatile int8_t curr_line = 0;
volatile char * curr_string;
uint8_t curr_string_size;
uint8_t curr_question = 0;
Button curr_answer;
int cap; // capture signals
volatile bool started, new;
volatile int waiting;
volatile State state = QUESTION;

void config_port() {
    P2DIR &= ~BIT0;
    P2REN |= BIT0;
    P2OUT |= BIT0;
    P2SEL |= BIT0;
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

        if (signal > 13000) { // more then 13 ms
            break;
        }
    }
}

char capture_bit() {
    int time = capture_signal();

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
    case 0x4A:
        return DOWN;
    case 0x18:
        return UP;
    case 0xA2:
        return _1;
    case 0x62:
        return _2;
    case 0xE2:
        return _3;
    case 0x22:
        return _4;
    case 0x81:
        return _5;
    case 0xE1:
        return _6;
    case 0xE0:
        return _7;
    case 0xD4:
        return _8;
    case 0x90:
        return _9;
    case 0xCC:
        return _0;
    case 0x38:
        return OK;
    case 0x88:
        return LEFT;
    case 0xAD:
        return RIGHT;
    }

    return ERROR;
}

typedef struct {
    char * question;
    Button answer;
} question_data;

question_data get_question(int8_t index) {
    switch(index) {
    case 0:
        return (question_data) {
            .question = "Bom dia! Bem vin"
                    "do a este quiz, "
                    "a sua primeira p"
                    "ergunta: Qual do"
                    "s anos seguintes"
                    " e bisexto:     "
                    "1 - 2000        "
                    "2 - 1900        "
                    "3 - 1800        "
                    "4 - 1700        "
                    "5 - 1918        ",
            .answer = _1
        };
    case 1:
        return (question_data) {
            .question = "Qual a capital  "
                    "da australia?   "
                    "1 - Sydney      "
                    "2 - Melbourne   "
                    "3 - Canberra    "
                    "4 - Perth       "
                    "5 - Adelaide    ",
            .answer = _3
        };
    case 2:
        return (question_data) {
            .question = "Aonde foi o esto"
                    "pim da 2 guerra "
                    "mundial?        "
                    "1-Tcheslovaquia "
                    "2-Polonia       "
                    "3-Servia        "
                    "4-Franca        "
                    "5-Austria       ",
            .answer = _2
        };
    }
    state = OVER;
    return (question_data) {
        .question = "Parabens! Voce  "
                "terminou o quiz",
        .answer = OK
    };
}

void config_timers() {
    TA0CTL = TASSEL_1 + MC_0;
    TA1CTL = TASSEL_2 + MC_1;
    TA1CCR0 = 15728; // ~15ms
    TA1CCTL1 = CM_2 + CCIE + CAP;
}

void wait_time(uint8_t time) {
    TA0CCR0 = time;
    TA0CTL |= MC_1 | TACLR | TAIE;
    waiting = 1;
    while(waiting) {}
    TA0CTL &= ~MC_1 & ~TAIE;
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
    const uint8_t addr = 0x27;
    uint8_t data = ((nibble & 0x0f) << 4) + 0x08;
    if (is_char) {
        data |= 1;
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

void lcd_print_line(char * str, uint8_t curr_line, uint8_t max_size) {
    uint8_t str_ptr = curr_line * 16;
    while (str[str_ptr] != '\0' && str_ptr < (curr_line * 16 + 16)) {
        lcd_write_byte(str[str_ptr], 1);
        str_ptr++;
    }
}

void update_lcd() {
    lcd_write_byte(0x01, 0);
    wait_time(70);

    lcd_print_line(curr_string, curr_line, curr_string_size);
    lcd_write_byte(0xa8, 0);
    lcd_print_line(curr_string, curr_line + 1, curr_string_size);
}

void set_current_question(uint8_t index) {
    question_data data = get_question(index);

    curr_string = data.question;
    curr_string_size = strlen(data.question);
    curr_answer = data.answer;
    curr_line = 0;
}

void run_command(Button button) {
    switch(button) {
    case ERROR:
        return;
    case DOWN:
        curr_line++;
        if ((curr_line + 1) * 16 >= curr_string_size) {
            curr_line--;
        }
        break;
    case UP:
        curr_line--;
        if (curr_line < 0) {
            curr_line = 0;
        }
        break;
    default:
        switch(state) {
        case QUESTION:
            if (button == curr_answer) {
                state = CORRECT;
                curr_string = "Parabens! Voce acertou, aperte qualquer botao para continuar";
                curr_string_size = strlen(curr_string);
            } else {
                state = WRONG;
                curr_string = "Resposta errada, aperte qualquer botao para continuar";
            }
            curr_string_size = strlen(curr_string);
            curr_line = 0;
            break;
        case CORRECT:
            curr_question++;
        case WRONG:
            state = QUESTION;
            set_current_question(curr_question);
        }
    }
    update_lcd();
}

/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    config_i2c();
    config_timers();
    config_port();

    __enable_interrupt();

    lcd_init();
    set_current_question(0);
    update_lcd();

    while (1) {
        capture_start();
        char addr = capture_byte();
        char not_addr = capture_byte();
        char cmd = capture_byte();
        char not_cmd = capture_byte();

        Button curr = get_button_from_byte(cmd);
        run_command(curr);
        new = false;
    }
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void timer_inturrupt() {
    TA0CTL &= ~TAIFG;
    waiting = 0;
}

#pragma vector = TIMER1_A1_VECTOR;
__interrupt void timer_interrupt() {
    TA1CTL |= TACLR;
    new = true;
    cap = TA1CCR1;
    TA1CCTL1 &= ~CCIFG;
}
