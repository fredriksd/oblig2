#include <stdio.h>
#include <avr/io.h>
#define LCD_PORT PORTC
#define RS_PIN PC1
#define RW_PIN PC0
#define E_PIN PC2
#define DDRLCD DDRC;

/*Initialisering av prototyper*/

unsigned char lcd_write_string(unsigned char row, unsigned char col, unsigned char *string);
void cursor_pos(unsigned char row, unsigned char col);
void lcdinstrhalf(unsigned char rs, unsigned char rw, unsigned char data);
void lcdinstr(unsigned char rs, unsigned char rw, unsigned char data);
void lcd_init(void);

int main(void)
{
	DDRLCD = 0b11101111;
	lcd_init();
}

//Skrive tekststrenger

void lcd_write_num(unsigned char row, unsigned char col,unsigned int num)
{
	unsigned char buffer[];
	sprintf(buffer, '%d', num); //Konverterer heltall til string
	lcd_write_string(row, col, buffer);
}

unsigned char lcd_write_string(unsigned char row, unsigned char col, unsigned char *string)
{
	cursor_pos(row, col);
	while (*string)
	{
		lcdinstr(0, 0, *string);
		*string++;
		_delay_ms(80);
	}
}

//Sette posisjonen til cursor ved rad og kolonne

void cursor_pos(unsigned char row, unsigned char col)
{
	unsigned char pos = (16 * row) + col;
	if (row == 0 & col == 0)
		lcdinstr(0, 0, 0b00000010);
	else
		lcdinstr(0, 0, 0b10000000 | pos);

}
//Sender bare instruksjoner på msb bits
void lcdinstrhalf(unsigned char rs, unsigned char rw, unsigned char data)
{
	if (rs)
		LCD_PORT &= (1 << RS_PIN);
	else
		LCD_PORT &= ~(1 << RS_PIN);
	if (rw)
		LCD_PORT &= (1 << RW_PIN);
	else
		LCD_PORT &= ~(1 << RW_PIN);

	LCD_PORT |= (1 << E_PIN);
	if (upper_nibble&(1 << 7))
		LCD_PORT |= (1 << 7);
	if (upper_nibble&(1 << 6))
		LCD_PORT |= (1 << 6);
	if (upper_nibble&(1 << 5))
		LCD_PORT |= (1 << 5);
	if (upper_nibble&(1 << 4))
		LCD_PORT |= (1 << 4);

	LCD_PORT &= ~(1 << E_PIN);	//Falling edge på Enable bit, slik at en puls er laget.
}
//Sender instruksjoner på msb bits og lsb bits
void lcdinstr(unsigned char rs, unsigned char rw, unsigned char data)
{
	// FORSLAG TIL SENDING VIA 4 BITS-COMM

	unsigned char upper_nibble;
	if (rs)
		LCD_PORT &= (1 << RS_PIN);
	else
		LCD_PORT &= ~(1 << RS_PIN);
	if (rw)
		LCD_PORT &= (1 << RW_PIN);
	else
		LCD_PORT &= ~(1 << RW_PIN);

	upper_nibble = (data&0xf0)//Send først msb-bitsene
	LCD_PORT |= (1 << E_PIN); //Toggler Enable Pin
	if (upper_nibble&(1 << 7))
		LCD_PORT |= (1 << 7);
	if (upper_nibble&(1 << 6))
		LCD_PORT |= (1 << 6);
	if (upper_nibble&(1 << 5))
		LCD_PORT |= (1 << 5);
	if (upper_nibble&(1 << 4))
		LCD_PORT |= (1 << 4);

	LCD_PORT &= ~(1<<E_PIN); //End of transmission

	data = (data&0x0f); //Så lsb-bitsene
	LCD_PORT |= (1 << E_PIN);
	if (upper_nibble&(1 << 7))
		LCD_PORT |= (1 << 7);
	if (upper_nibble&(1 << 6))
		LCD_PORT |= (1 << 6);
	if (upper_nibble&(1 << 5))
		LCD_PORT |= (1 << 5);
	if (upper_nibble&(1 << 4))
		LCD_PORT |= (1 << 4);

	LCD_PORT &= ~(1 << E_PIN); //Falling edge på Enable bit, slik at en puls er laget.
}

void lcd_init(void)
{
	_delay_ms(100);
	LCD_PORT |= (1 << E_PIN); //Forsikrer at enable-pin er lav

	lcdinstrhalf(0, 0, 0b00110000); 
	_delay_ms(2); //sette i 8-bits
	lcdinstrhalf(0, 0, 0b00110000); 
	_delay_ms(2); //sette i 8-bits
	lcdinstrhalf(0, 0, 0b00110000); 
	_delay_ms(2); //sette i 8-bits
	lcdinstrhalf(0, 0, 0b00100000);  //'Real Function 'set. Nå kan BF settes høyt
	_delay_ms(2); //sette i 4-bits	// Og LCD-uK mottar en reset. 
	lcdinstr(0, 0, 0b00101001);
	_delay_us(28); //function set
	lcdinstr(0, 0, 0b00011101); 
	_delay_us(28);//1:bias set
	lcdinstr(0, 0, 0b01010000);
	_delay_us(28); //1:power control
	lcdinstr(0, 0, 0b01101100);
	_delay_us(28); //1:follower control
	lcdinstr(0, 0, 0b01111100);
	_delay_us(28); //1: contrast set
	lcdinstr(0, 0, 0b00101000);
	_delay_us(28); //1:function set
	lcdinstr(0, 0, 0b00001111);
	_delay_us(28); //display on/off
	lcdinstr(0, 0, 0b00000001);
	_delay_ms(3); //clear Display
	lcdinstr(0, 0, 0b00000110);	_delay_us(28); //entry mode set	_delay_ms(50);
}