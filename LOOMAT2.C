#include <pic.h>
#define _XTAL_FREQ 2000000

#define relay RB0
#define sw RB1
#define emergency_switch RB2
#define rs RC0
#define rw RC1
#define en RC2
#define lcd_data_high RB4
#define lcd_data_mid RB5
#define lcd_data_low RB6
#define lcd_data_lowest RB7
#define red_led RB3

void UART_Init() {
   SYNC = 0;
   TX9 = 0;
   RX9 = 0;
   SPEN = 1;
   TXEN = 1;
   CREN = 1;
   SPBRG = 12;
}

void UART_Write(unsigned char data) {
    while (!TXIF);
    TXREG = data;
}

void UART_Write_Text(char *text) {
    while (*text) {
        UART_Write(*text);
        text++;
    }
}

void LCD_Command(unsigned char cmd) {
  lcd_data_high = (cmd >> 4) & 0x01;
   lcd_data_mid = (cmd >> 5) & 0x01;
   lcd_data_low = (cmd >> 6) & 0x01;
   lcd_data_lowest = (cmd >> 7) & 0x01;

    rs = 0;
    rw = 0;
    en = 1;
    __delay_ms(1);
    en = 0;
    __delay_ms(1);

    lcd_data_high = (cmd & 0x01);
    lcd_data_mid = (cmd >> 1) & 0x01;
    lcd_data_low = (cmd >> 2) & 0x01;
    lcd_data_lowest = (cmd >> 3) & 0x01;

    en = 1;
    __delay_ms(1);
    en = 0;
}

void LCD_Data(unsigned char data) {
    lcd_data_high = (data >> 4) & 0x01;
    lcd_data_mid = (data >> 5) & 0x01;
    lcd_data_low = (data >> 6) & 0x01;
    lcd_data_lowest = (data >> 7) & 0x01;

    rs = 1;
    rw = 0;
    en = 1;
    __delay_ms(1);
    en = 0;
    __delay_ms(1);

    lcd_data_high = (data & 0x01);
    lcd_data_mid = (data >> 1) & 0x01;
    lcd_data_low = (data >> 2) & 0x01;
    lcd_data_lowest = (data >> 3) & 0x01;

    en = 1;
    __delay_ms(1);
    en = 0;
}

void LCD_Init() {
    __delay_ms(50);
    LCD_Command(0x02);
    LCD_Command(0x28);
    LCD_Command(0x06);
    LCD_Command(0x0C);
    LCD_Command(0x01);
    __delay_ms(10);
}

void LCD_Print(char* str) {
    while(*str) {
        LCD_Data(*str);
        str++;
    }
}

void main(void) {
    TRISB0 = 0;
    TRISB1 = 1;
    TRISB2 = 1;
    TRISC0 = 0;
    TRISC1 = 0;
    TRISC2 = 0;
    TRISB4 = 0;
    TRISB5 = 0;
    TRISB6 = 0;
    TRISB7 = 0;
    TRISB3 = 0;

    unsigned int press_count = 0;
    unsigned char prev_relay_state = 0;
    unsigned char emergency_state = 0;

    relay = 0;
    red_led = 1;

    LCD_Init();
    UART_Init();

    LCD_Print("Relay Control");

    while (1) {
        if (emergency_switch == 1) {
            if (!emergency_state) {
                emergency_state = 1;
                relay = 0;
                red_led = 1;
                LCD_Command(0x01);
                __delay_ms(2);
                LCD_Print("EMERGENCY OFF");
                prev_relay_state = 0;
                press_count = 0;
__delay_ms(1000);
                UART_Write_Text("Relay OFF\r\n");
            }
        }
        else if (emergency_switch == 0 && emergency_state == 1) {
            emergency_state = 0;
            __delay_ms(200);
            if (press_count >= 10) {
                relay = 0;
                red_led = 1;
                LCD_Command(0x01);
                __delay_ms(2);
                LCD_Print("Relay OFF");
                prev_relay_state = 0;
__delay_ms(1000);
                UART_Write_Text("Relay OFF \r\n");
            } else {
                relay = 1;
                red_led = 0;
                LCD_Command(0x01);
                __delay_ms(2);
                LCD_Print("Relay ON");
                prev_relay_state = 1;
__delay_ms(1000);
                UART_Write_Text("Relay ON \r\n");
            }
        }
        else if (sw == 1) {
            __delay_ms(200);
            press_count++;
            if (press_count >= 10) {
                if (prev_relay_state != 0) {
                    relay = 0;
                    red_led = 1;
                    LCD_Command(0x01);
                    __delay_ms(2);
                    LCD_Print("Relay OFF");
                    prev_relay_state = 0;
__delay_ms(1000);
                    UART_Write_Text("Relay OFF \r\n");
                }
            } else {
                if (prev_relay_state != 1) {
                    relay = 1;
                    red_led = 0;
                    LCD_Command(0x01);
                    __delay_ms(2);
                    LCD_Print("Relay ON");
                    prev_relay_state = 1;
                }
            }
        } else {
            press_count = 0;
            if (prev_relay_state != 0) {
                relay = 0;
                red_led = 1;
                LCD_Command(0x01);
                __delay_ms(2);
                LCD_Print("Relay OFF");
                prev_relay_state = 0;
__delay_ms(1000);
                UART_Write_Text("Relay OFF \r\n");
            }
        }
    }
}
