//*****************************************************************************
//Universidad del Valle de Guatemala
//Programación de Microcontroladores
//Archivo:Proyecto
//Hardware:ATMEGA328P
//Autor:Esteban Longo
//Carnet:22649
//*****************************************************************************

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "PWM/PWM0.h"
#include "PWM/PWM1.h"
#include "PWM/PWM2.h"
#include "ADC/ADC.h"

uint8_t DutyC1 = 0;
uint8_t DutyC2 = 0;
uint8_t DutyC3 = 0;
uint8_t DutyC4 = 0;
volatile int Estado = 0;

void setup(void);
unsigned char R_eeprom(unsigned int Adress); // Declaración de la función R_eeprom
void W_eeprom(unsigned int Adress, unsigned char Data); // Declaración de la función W_eeprom

int main(void) {
	cli(); 

	DDRD &= ~((1 << DDD6) | (1 << DDD7) | (1 << DDD4)| (1 << DDD2)); // PD6, PD7 y PD4 como entradas
	PORTD |= (1 << PORTD6) | (1 << PORTD7) | (1 << PORTD4)| (1 << PORTD2); // Activar pull-ups en PD6, PD7 y PD4
	
	DDRB &= ~((1 << DDB0) | (1 << DDB4)| (1 << DDB5)); // PB0 y PB4 como entradas
	PORTB |= (1 << PORTB0) | (1 << PORTB4)| (1 << PORTB5); // Activar pull-ups en PB0 y PB4
	
	DDRC |= (1 << DDC0) | (1 << DDC1) | (1 << DDC2); // PC0, PC1 y PC2 como salidas
	PORTC &= ~((1 << PORTC0) | (1 << PORTC1) | (1 << PORTC2)); // Iniciar LEDs apagados

	initADC();
	initPWM1A(no_invertido, 8, 39999);
	initPWM1B(no_invertido, 8, 39999);
	initPWM2A(no_invertido, 1024);
	initPWM2B(no_invertido, 1024);

	PCICR |= (1 << PCIE2); 
	PCMSK2 |= (1 << PCINT23)| (1 << PCINT20)| (1 << PCINT18); 

	PCICR |= (1 << PCIE0); 
	PCMSK0 |= (1 << PCINT0) | (1 << PCINT4)| (1 << PCINT5); 

	UCSR0B = 0; // Deshabilitar TX y RX

	sei(); 

	while (1) {
		switch (Estado) {
			case 0:
			PORTC = (1 << PC0); // Enciende el LED en PC0
			//lectura ADC 6 para el PWM 1A
			_delay_ms(10);
			DutyC1 = ADC_CONVERT(6);
			updateDutyCA1(DutyC1);
			//lectura ADC 6 para el PWM 2A
			_delay_ms(10);
			DutyC2 = ADC_CONVERT(4);
			updateDutyCA2(DutyC2);
			//lectura ADC 6 para el PWM 1B
			_delay_ms(10);
			DutyC3 = ADC_CONVERT(5);
			updateDutyCB1(DutyC3);
			//lectura ADC 7 para el PWM 2B
			_delay_ms(10);
			DutyC4 = ADC_CONVERT(7);
			updateDutyCB2(DutyC4);

			break;
			//lectura de la memoria EEPROM
			case 1:
			PORTC = (1 << PC1); // Enciende el LED en PC1

			break;
		
			case 2:
			PORTC = (1 << PC2);
			break;
			
			default:
			Estado = 0;
			break;
		}
	}
}

//Interrupcion para cambios de estado 
ISR(PCINT2_vect) {
	if ((PIND & (1 << PIND7)) == 0) {
		_delay_ms(50);
		if ((PIND & (1 << PIND7)) == 0) { // Verificar si el botón sigue presionado
			Estado++;
			if (Estado > 2) {
				Estado = 0; // Resetear estado si sobrepasa el límite
			}
		}
	}
	//Guardado de las 2 posiciones en la EEPROM con los botones en el puerto D
	else if ((PIND & (1 << PIND4)) == 0) {
		
		PORTC = (1 << PC2);
		if (Estado == 0) {
			W_eeprom(0, DutyC1);
			W_eeprom(1, DutyC2);
			W_eeprom(2, DutyC3);
			W_eeprom(3, DutyC4);
			} else if (Estado == 1) {
			uint8_t readDutyC1 = R_eeprom(0);
			updateDutyCA1(readDutyC1);
			uint8_t readDutyC2 = R_eeprom(1);
			updateDutyCA2(readDutyC2);
			uint8_t readDutyC3 = R_eeprom(2);
			updateDutyCB1(readDutyC3);
			uint8_t readDutyC4 = R_eeprom(3);
			updateDutyCB2(readDutyC4);
		}
	}
	else if ((PIND & (1 << PIND2)) == 0) {
		if (Estado == 0) {
			PORTC = (1 << PC2);
			W_eeprom(12, DutyC1);
			W_eeprom(13, DutyC2);
			W_eeprom(14, DutyC3);
			W_eeprom(15, DutyC4);
			} else if (Estado == 1) {
			PORTC = (1 << PC2);
			uint8_t readDutyC1 = R_eeprom(12);
			updateDutyCA1(readDutyC1);
			uint8_t readDutyC2 = R_eeprom(13);
			updateDutyCA2(readDutyC2);
			uint8_t readDutyC3 = R_eeprom(14);
			updateDutyCB1(readDutyC3);
			uint8_t readDutyC4 = R_eeprom(15);
			updateDutyCB2(readDutyC4);
		}
	}
}
//Interrupciones por botones
ISR(PCINT0_vect) {
	//Guardado de las 2 posiciones en la EEPROM con los botones en el puerto B
	if ((PINB & (1 << PINB0)) == 0) {
		PORTC = (1 << PC2);
		if (Estado == 0) {
			W_eeprom(4, DutyC1);
			W_eeprom(5, DutyC2);
			W_eeprom(6, DutyC3);
			W_eeprom(7, DutyC4);
			} else if (Estado == 1) {
			uint8_t readDutyC1 = R_eeprom(4);
			updateDutyCA1(readDutyC1);
			uint8_t readDutyC2 = R_eeprom(5);
			updateDutyCA2(readDutyC2);
			uint8_t readDutyC3 = R_eeprom(6);
			updateDutyCB1(readDutyC3);
			uint8_t readDutyC4 = R_eeprom(7);
			updateDutyCB2(readDutyC4);
		}
	}
	else if ((PINB & (1 << PINB4)) == 0) {
		PORTC = (1 << PC2);
		
		if (Estado == 0) {
			W_eeprom(8, DutyC1);
			W_eeprom(9, DutyC2);
			W_eeprom(10, DutyC3);
			W_eeprom(11, DutyC4);
			} else if (Estado == 1) {
			uint8_t readDutyC1 = R_eeprom(8);
			updateDutyCA1(readDutyC1);
			uint8_t readDutyC2 = R_eeprom(9);
			updateDutyCA2(readDutyC2);
			uint8_t readDutyC3 = R_eeprom(10);
			updateDutyCB1(readDutyC3);
			uint8_t readDutyC4 = R_eeprom(11);
			updateDutyCB2(readDutyC4);
		}
	}
	
}
//Modulo de lectura de le EEPROM 
unsigned char R_eeprom(unsigned int Adress) {
	// Esperar la finalización de la escritura anterior
	while (EECR & (1 << EEPE));
	// Configurar el registro de dirección
	EEAR = Adress;
	// Iniciar la lectura de la EEPROM escribiendo EERE
	EECR |= (1 << EERE);
	// Devolver los datos del registro de datos
	return EEDR;
}
//Modulo de escritura de le EEPROM 
void W_eeprom(unsigned int Adress, unsigned char Data) {
	// Esperar la finalización de la escritura anterior
	while (EECR & (1 << EEPE));
	
	EEAR = Adress;
	EEDR = Data;

	EECR |= (1 << EEMPE);
	
	EECR |= (1 << EEPE);
}