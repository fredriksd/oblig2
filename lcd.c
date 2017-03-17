/*Definisjoner og konstanter
Gjør det enklere å lese og forstå koden*/
#define F_CPU 3686400
#define LCD_PORT PORTC
#define RS_PIN PC1
#define RW_PIN PC0
#define E_PIN PC2
#define DDRLCD DDRC

/*Includes*/
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>

/*Initialisering av prototyper*/

unsigned char lcd_write_string(unsigned char row, unsigned char col, unsigned char *string);
void lcd_write_num(unsigned char row, unsigned char col, unsigned int num);
void cursor_pos(unsigned char row, unsigned char col);
void lcdinstrhalf(unsigned char rs, unsigned char rw, unsigned char data);
void lcdinstr(unsigned char rs, unsigned char rw, unsigned char data);
void lcd_init(void);

//Konverterer numre til strenger og sender dem
void lcd_write_num(unsigned char row, unsigned char col, unsigned int num)
{
	unsigned char buffer[5];
	sprintf((char*)buffer, "%d", num); //Konverterer heltall til string
	lcd_write_string(row, col, buffer);
}

//Skrive tekststrenger
unsigned char lcd_write_string(unsigned char row, unsigned char col, unsigned char *string)
{
	cursor_pos(row, col);
	while (*string)
	{
		lcdinstr(1, 0, *string);
		*string++;
		_delay_ms(80);
	}
}

//Sette posisjonen til cursor ved rad og kolonne
void cursor_pos(unsigned char row, unsigned char col)
{
	/*Da posisjonene på skjermen er gitt ved heksadesimale addresser, 
	kan posisjonen bestemmes ved å ta summen av ønsket rad og kolonne.
	Denne summen (position) blir OR-ed med 1000 0000, som er "set DDRAM address"-instruksjon*/
	unsigned char pos = (16 * row) + col;
	if (row == 0 && col == 0)
	lcdinstr(0, 0, 0b00000010);
	else
	lcdinstr(0, 0, 0b10000000 | pos);

}

//Sender bare instruksjoner på msb bits
void lcdinstrhalf(unsigned char rs, unsigned char rw, unsigned char data)
{
	/*Forsikrer at Enable Pin er høy*/
	LCD_PORT &= (1<<E_PIN); 
	if (rs)
		LCD_PORT |= (1 << RS_PIN);
	else
		LCD_PORT &= ~(1 << RS_PIN);
	if (rw)
		LCD_PORT |= (1 << RW_PIN);
	else
		LCD_PORT &= ~(1 << RW_PIN);
		
	LCD_PORT |= (data&0xf0);

	LCD_PORT &= ~(1 << E_PIN); //End of transmission
	_delay_us(30);
	LCD_PORT |= (1 << E_PIN); //Toggler Enable Pin
	_delay_us(30);

}

//Sender instruksjoner på msb bits og lsb bits
void lcdinstr(unsigned char rs, unsigned char rw, unsigned char data)
{
	/*Sending av instruksjoner ved 4 bits kommunikasjon.
	Dataen (bestående av 8 bits) blir delt opp i to 4-bits 
	sendinger. 
	*/
	lcdinstrhalf(rs,rw,(data&0xf0)); //Send først msb-bits
	lcdinstrhalf(rs,rw,(data<<4)); //Deretter send lsb-bits
}

void lcd_init(void)
{
	_delay_ms(100);

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
	lcdinstr(0, 0, 0b00000110);
	_delay_us(28); //entry mode set
	_delay_ms(50);

}