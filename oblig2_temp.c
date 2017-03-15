#include <stdio.h>
#include <avr/io.h>
#define F_CPU 3686400
#include <util/delay.h>

void curser_position(unsigned char row, unsigned char col)
{
    if (row == 0)
        PORTC = 0x00;
    else if(row == 1)
        PORTC = 0x10;
    else if(row == 2)
        PORTC = 0x20;
}

void lcd_write(unsigned char row, unsigned char col, flash unsigned char *string)
{
    curser_position(row,col);
    while(*string)
    {
        lcdinstr(0,0,*string);
        _delay_us(28);
        *string++;
    }
}


void lcdinstrhalf(unsigned char rs, unsigned char rw, unsigned char modes)
{
    if (rw)
        PORTC |= 0x01;
    if(rs)
        PORTC |= 0x02;

    for(unsigned char i = 4; i<7;i++)
    {
        if(modes&(1<<i))
        {
            if (i == 4)
                PORTC |= 0x10;
            else if(i == 5)
                PORTC |= 0x20;
            else if(i == 6)
                PORTC |= 0x40;
            else if(i == 7)
                PORTC |= 0x80;
        }
    }
    PORTC |= 0x04;

}

void lcdinstr(unsigned char rs, unsigned char rw, unsigned char modes)
{
    if(rw)
        PORTC |= 0x01;
    if(rs)
        PORTC |= 0x02;

    for(unsigned char i = 0; i<7;i++)
    {
        if(modes&(1<<i))
        {
            if(i == 0 )
                PORTC |= 0x01;
            else if (i == 1)
                PORTC |= 0x02;
            else if (i == 2)
                PORTC |= 0x04;
            else if (i == 3)
                PORTC |= 0x08;
            else if(i == 4)
                PORTC |= 0x10;
            else if(i == 5)
                PORTC |= 0x20;
            else if(i == 6)
                PORTC |= 0x40;
            else if(i == 7)
                PORTC |= 0x80;
        }
    }
    PORTC |= 0x04;
}

void init_lcd(void)
{
    lcdinstrhalf(0,0,0b00110000); _delay_ms(2); //8-bits
    lcdinstrhalf(0,0,0b00110000); _delay_ms(2); //8-bits
    lcdinstrhalf(0,0,0b00110000); _delay_ms(2); //8-bits
    lcdinstrhalf(0,0,0b00100000); _delay_ms(2); //4-bits
    lcdinstr(0,0,0b00101001); _delay_us(28); //function set
    lcdinstr(0,0,0b00011101); _delay_us(28) ;//1:bias set
    lcdinstr(0,0,0b01010000); _delay_us(28); //1:power control
    lcdinstr(0,0,0b01101100); _delay_us(28); //1:follower control
    lcdinstr(0,0,0b01111100); _delay_us(28); //1: contrast set
    lcdinstr(0,0,0b00101000); _delay_us(28); //1:function set
    lcdinstr(0,0,0b00001111); _delay_us(28); //display on/off
    lcdinstr(0,0,0b00000001); _delay_ms(3); //clear Display
    lcdinstr(0,0,0b00000110); _delay_us(28); //entry mode set
    _delay_ms(100);
}

