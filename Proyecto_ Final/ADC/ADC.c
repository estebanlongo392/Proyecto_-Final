#include "ADC.h"

void initADC(void){
	ADMUX = 0;
	ADMUX |= (1 << REFS0);		
	ADMUX &= ~(1 << REFS1);		
	ADMUX |= (1 << ADLAR);		// JUSTIFICACION A LA DERECHA
	
	ADCSRA = 0;
	ADCSRA |= (1 << ADEN);				
	ADCSRA |= (1 << ADPS0);
	ADCSRA |= (1 << ADPS1);
	ADCSRA |= (1 << ADPS2);		
	
	DIDR0 |= (1 << ADC0D);		
	DIDR0 |= (1 << ADC1D);		
	DIDR0 |= (1 << ADC2D);		
}

uint8_t ADC_CONVERT(uint8_t canal){
	ADMUX = (ADMUX & 0xF0)|canal;	// SELECCION DEL CANAL
	ADCSRA |= (1<<ADSC);			// INICIA EL ADC
	while((ADCSRA)&(1<<ADSC));		// FINALIZA LA CONVERSION
	return(ADCH);
}
