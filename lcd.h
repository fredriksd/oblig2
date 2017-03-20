/************************************************************************/
/* LCD DISPLAY BIBLIOTEK FOR ST7036.
   INNEHOLDER FØLGENDE FUNKSJONER
   *lcd_write_string
   *lcd_write_num
   *cursor_pos
   *lcd_instrhalf
   *lcdinstr
   *lcd_init                      
   
   MERK: En delay legges til hver operasjon som gjennomføres av LCD-uK, da man må la mikrokontrolleren gjennomføres instruksjonene
   sine før noe nytt sendes til den, enkelt sagt.                                            */
/************************************************************************/

/*Definisjoner og konstanter
Gjør det enklere å lese og forstå koden*/
#define F_CPU 3686400	//Klokkefrekvensen til ATmega16
#define LCD_PORT PORTC //LCD output port
#define RS_PIN PC1
#define RW_PIN PC0
#define E_PIN PC2
#define DDRLCD DDRC 

/*Includes*/
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>

/*Initialisering av prototyper*/

void lcd_write_string(unsigned char row, unsigned char col, unsigned char *string);
void lcd_write_num(unsigned char row, unsigned char col, unsigned int num);
void cursor_pos(unsigned char row, unsigned char col);
void lcdinstrhalf(unsigned char rs, unsigned char rw, unsigned char data);
void lcdinstr(unsigned char rs, unsigned char rw, unsigned char data);
void lcd_init(void);

//Konverterer tall til strenger og sender dem
void lcd_write_num(unsigned char row, unsigned char col, unsigned int num)
{
	unsigned char buffer[5];
	sprintf((char*)buffer, "%4d", num); //Konverterer heltall til string
	lcd_write_string(row, col, buffer);
}

//Skrive tekststrenger. Velger først posisjonen av pekeren (cursor), og inkrementerer pekerverdien til 'string',
//og sender hver character via lcdinstr().
void lcd_write_string(unsigned char row, unsigned char col, unsigned char *string)
{
	cursor_pos(row, col);
	while (*string)
	{
		lcdinstr(1, 0, *string);
		*string++;
		_delay_us(30); //Minst en delay på 26,3 us
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

/*Sender bare instruksjoner på msb bits. Da en 4-bits LCD-uK bare bruker data bits 4 - 7,
sendes først en nibble om gangen, for så å skifte den laveste nibble til høyeste nibble og sende den. 
*/
void lcdinstrhalf(unsigned char rs, unsigned char rw, unsigned char data)
{
	/*Forsikrer at Enable Pin er høy*/
	LCD_PORT &= (1<<E_PIN); 
	if (rs)//Setter RS-pin høy dersom rs = 1
		LCD_PORT |= (1 << RS_PIN);
	else//Hvis ikke, vær lav
		LCD_PORT &= ~(1 << RS_PIN);
	if (rw)//Setter RW-pin høy dersom rw = 1
		LCD_PORT |= (1 << RW_PIN);
	else//Hvis ikke, vær lav
		LCD_PORT &= ~(1 << RW_PIN);
	//Legger inn høyeste nibble til sending
	LCD_PORT |= (data&0xf0);

	LCD_PORT &= ~(1 << E_PIN); //End of transmission
	_delay_us(30); //Optimal delay: 26,3 us
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


/*Initaliserer ST7036 for 4-bits modus. Ved oppstart er LCD-displayet i 8 bits modus, og Busy Flag (BF) kan ikke sjekkes før en instruksjon er blitt sendt til LCD-uK.
En instruksjon om å sette LCD-displayet i 8-bits modus blir sendt 4 gang for å initiere en 'reset', slik at den kan settes i 4-bits modus. Deretter settes de ulike
instillingene for lcd-skjermen, slik som interface data 4 bits, display on (duh), display clear, entry mode (cursor beveges ved skriving av informasjon).
*/
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
	_delay_us(28);//:bias set
	lcdinstr(0, 0, 0b01010000);
	_delay_us(28); //:power control
	lcdinstr(0, 0, 0b01101100);
	_delay_us(28); //:follower control
	lcdinstr(0, 0, 0b01111100);
	_delay_us(28); //: contrast set
	lcdinstr(0, 0, 0b00101000);
	_delay_us(28); //:function set
	lcdinstr(0, 0, 0b00001111);
	_delay_us(28); //display on/off
	lcdinstr(0, 0, 0b00000001);
	_delay_ms(3); //clear Display
	lcdinstr(0, 0, 0b00000110);
	_delay_us(28); //entry mode set
	_delay_ms(50);

}