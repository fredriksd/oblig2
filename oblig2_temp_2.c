#define F_CPU 3686400
#define LCD_PORT PORTC
#define RS_PIN PC1
#define RW_PIN PC0
#define E_PIN PC2
#define DDRLCD DDRC

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

/*Deklareringer av variabler*/
volatile unsigned int voltage, adc_reader;

ISR(ADC_vect)
{
	adc_reader = ADCW; //Pseudoregister for ADCL og ADCH "summert"
	voltage = 5000 * adc_reader / 1023;
}

int main(void)
{
	ADMUX = 0x04; //Bruker ADC input 4 
	ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADPS2);
	unsigned char test_string[] = { "Hello world!" };
	LCD_PORT = 0xff;
	DDRLCD = 0b11110111;
	lcd_init();
	lcd_write_string(0, 0, test_string);
}

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
	unsigned char pos = (16 * row) + col;
	if (row == 0 && col == 0)
		lcdinstr(0, 0, 0b00000010);
	else
		lcdinstr(0, 0, 0b10000000 | pos);

}

//Sender bare instruksjoner p� msb bits
void lcdinstrhalf(unsigned char rs, unsigned char rw, unsigned char data)
{
	//LCD_PORT &= (1 << E_PIN); 
	unsigned char upper_nibble = (data & 0xf0);
	LCD_PORT &= 0x0f;
	if (rs)
		LCD_PORT |= (1 << RS_PIN);
	else
		LCD_PORT &= ~(1 << RS_PIN);
	if (rw)
		LCD_PORT |= (1 << RW_PIN);
	else
		LCD_PORT &= ~(1 << RW_PIN);

	if (upper_nibble&(1 << 7))
		LCD_PORT |= (1 << 7);
	if (upper_nibble&(1 << 6))
		LCD_PORT |= (1 << 6);
	if (upper_nibble&(1 << 5))
		LCD_PORT |= (1 << 5);
	if (upper_nibble&(1 << 4))
		LCD_PORT |= (1 << 4);

	LCD_PORT &= ~(1 << E_PIN); //End of transmission
	_delay_us(30);
	LCD_PORT |= (1 << E_PIN); //Toggler Enable Pin
	_delay_us(30);

}
//Sender instruksjoner p� msb bits og lsb bits

void lcdinstr(unsigned char rs, unsigned char rw, unsigned char data)
{
	// FORSLAG TIL SENDING VIA 4 BITS-COMM
	LCD_PORT &= (1 << E_PIN);
	unsigned char upper_nibble;
	if (rs)
		LCD_PORT |= (1 << RS_PIN);
	else
		LCD_PORT &= ~(1 << RS_PIN);
	if (rw)
		LCD_PORT |= (1 << RW_PIN);
	else
		LCD_PORT &= ~(1 << RW_PIN);

	LCD_PORT &= 0x0f;
	//LCD_PORT|=(data&0xf0);
	//upper_nibble = (data&0xf0);//Send f�rst msb-bitsene
	if (data&(1 << 7))
		LCD_PORT |= (1 << 7);
	if (data&(1 << 6))
		LCD_PORT |= (1 << 6);
	if (data&(1 << 5))
		LCD_PORT |= (1 << 5);
	if (data&(1 << 4))
		LCD_PORT |= (1 << 4);

	LCD_PORT &= ~(1 << E_PIN); //End of transmission
	_delay_us(30);
	LCD_PORT |= (1 << E_PIN); //Toggler Enable Pin
	_delay_us(30);

	LCD_PORT &= 0x0F;
	//LCD_PORT |= (data<<4);
	upper_nibble = (data & 0x0f) << 4; //S� lsb-bitsene
	if (upper_nibble&(1 << 7))
		LCD_PORT |= (1 << 7);
	if (upper_nibble&(1 << 6))
		LCD_PORT |= (1 << 6);
	if (upper_nibble&(1 << 5))
		LCD_PORT |= (1 << 5);
	if (upper_nibble&(1 << 4))
		LCD_PORT |= (1 << 4);

	LCD_PORT &= ~(1 << E_PIN); //End of transmission
	_delay_us(30);
	LCD_PORT |= (1 << E_PIN); //Toggler Enable Pin
	_delay_us(30);

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
	lcdinstrhalf(0, 0, 0b00100000);  //'Real Function 'set. N� kan BF settes h�yt
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