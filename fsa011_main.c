/*Definerer klokkefrekvensen til ATmega16*/
#define F_CPU 3686400

/*Includes*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include "fsa011_lcd.h"

/*Deklareringer av variabler*/
volatile unsigned int voltage, high_voltage = 0, low_voltage = 0, adc_reader;
unsigned char voltage_text[] = { "Spenning: " };
unsigned char voltage_mv[] = { "mV" };
unsigned char adcw_reader[] = { "ADCW: " };
unsigned char high_text[] = { "Highest: " };
unsigned char low_text[] = { "Lowest: " };
unsigned char pressed = 0, changed = 0; //Tilstandsvariabler

/*Interrupt Service Routine for ADC. Her regnes ut spenninga gitt ved summen av bitsene i ADCL-og ADCH-registrene.
Deretter blir spenninga vist ut på skjermen i tillegg til ADCW-verdien.
Dersom switch 0 (her koblet til PORTD, se main()), blir trykket, vises høyest og lavest målt spenning, og hvis
switch 1 blir trykket, settes minste og høyeste spenning til siste målte verdi.
*/

ISR(ADC_vect)
{
	adc_reader = ADCW; //Pseudoregister for ADCL og ADCH "summert", som er to 8 bits registre(to bits i ADCH --> ADCW er et 10-bits register).
					   /*Spenninga gitt i mV*/
	voltage = (5000 * (long)adc_reader) / 1023;

	/*Dersom ikke trykket, vis målte spenning*/
	if (pressed == 0)
	{
		/*Dersom knappen ikke er trykket og ny informasjon ikke er vist, vis ny informasjon*/
		if (changed == 0)
		{
			/*"Clear display" - instruksjon*/
			lcdinstr(0, 0, 0b00000001);
			_delay_ms(3);
			lcd_write_string(0, 0, adcw_reader);
			lcd_write_string(1, 0, voltage_text);
			lcd_write_string(1, 14, voltage_mv);
			changed = 1;
		}
		if (changed == 2)
		{
			high_voltage = voltage; //Setter høyeste spenning til den siste spenninga målt
			low_voltage = voltage; //Setter laveste spenning til den siste spenninga målt
			changed = 1;
		}

		lcd_write_num(0, 6, adc_reader); //Skriver ut ADCW
		_delay_ms(50);//Delay for å fjerne flimring
		lcd_write_num(1, 10, voltage); //Skriver ut spenning
		_delay_ms(50);//Delay for å fjerne flimring
	}
	/*Hvis trykket, vis høyeste og laveste spenning*/
	else if (pressed == 1)
	{
		/*Dersom knappen er trykket og ny informasjon ikke er vist, vis ny informasjon*/
		if (changed == 1)
		{
			/*"Clear display" - instruksjon*/
			lcdinstr(0, 0, 0b00000001);
			_delay_ms(3);
			changed = 0;
			lcd_write_string(0, 0, high_text);
			lcd_write_string(0, 14, voltage_mv);
			lcd_write_string(1, 0, low_text);
			lcd_write_string(1, 14, voltage_mv);
		}
		if (changed == 2)
		{
			high_voltage = voltage; //Setter høyeste spenning til den siste spenninga målt
			low_voltage = voltage; //Setter laveste spenning til den siste spenninga målt
			changed = 0;
		}
		else
		{
			if (high_voltage < voltage) //Sjekker etter høyest målt spenning og lagrer denne verdien for display.
				high_voltage = voltage;
			if (low_voltage > voltage) //Sjekker etter lavest målt spenning og lagrer denne verdien for display.
				low_voltage = voltage;
		}
		lcd_write_num(0, 9, high_voltage);
		_delay_ms(50); //Delay for å fjerne flimring
		lcd_write_num(1, 9, low_voltage);
		_delay_ms(50); //Delay for å fjerne flimring
	}
	ADCSRA |= (1 << ADSC);
}

int main(void)
{
	/*Initialiserer register for Analog To Digital Conversion*/
	ADMUX = 0x07; //Bruker ADC input 7
	ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADSC);//ADC Enable, ADC Interrupt Enable, og 32-prescaler
																	//for å få maksimal oppløsning (3686400Hz/32 = 115200Hz). ADC må operere mellom 50kHz og 200kHz

	LCD_PORT |= (1 << E_PIN); //Sette enable pin høy for å starte en transmission/instruksjon.
	DDRLCD = 0b11110111; //Setter output for RW, RS, DB4-DB7. CSB (Chip Select Button) brukes ikke (bruker ikke seriell kommunikasjon), og denne blir ikke satt. 
	lcd_init(); //Initialiserer displayet.
	sei(); //Global interrupt enable.
	while (1)
	{
		/*Sjekker om knappene er blitt trykket eller ikke.
		For hvert av tilfellene endres "pressed" slik at ISR for
		ADC interrupt forandres.
		*/
		if (PIND == 255)
		{
			pressed = 0;
		}
		if (PIND == 254)
		{
			pressed = 1;
		}
		if (PIND == 252 || PIND == 253)
		{
			changed = 2;
		}
	}
}
