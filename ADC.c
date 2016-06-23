/*
 * ADC.c
 *
 * Created: 6/13/15 1:09:27 PM
 *  Author: Roger
 
 
 Want to set up the ADC as 8-bit, will left-align the results and drop the Low register...8-bits is plenty of resolution for my needs here and it's easier to deal with.
 In this first setup I've got a resistor divider on VIN such where Rtop = 100K and Rbottom = 47K so the divide by = /3.12.  7.2Vin/3.12 = 2.3V.  VCC = 3.3V 2.3/3.3 * 256 = 178.
 */ 

#include "ADC.h"

void ADC_Init(void) {
	
	ADMUX = 0x67;	// 6 is selecting Left Allign and reference voltage. LSNibble == 7 is for ADC7
	ADCSRA = 0x95;	//Enable ADC, write a 1 to the interrupt flag to clear it, and set prescaler to 32.  
}

//This will deterimine if an ADC conversion has been completed
u8 checkADC_Stat(void){
	if(ADCSRA & 0x10){
		return 1;
	}else{
		return 0;
	}
	
}

//initialize ADC conversion
void start_ADC_conv (void){
	ADCSRA |= 0x40;	//ADC Start Conversion
}


//For right now this is returning the left-alligned (upper 8 bits of the 10-bit ADC) 8-bit conversion.  Might just have this return 1 or zero depending on whether or not the value is above my minimum VCC threshold.
u8 readADC(void){

	ADCSRA |= 0x10;	//clear interrupt flag
	if(ADCH > 170){	//170 ~= 6.8V, if I'm above there then I should be good
		return 0;
	}
	return 1;	//TODO modify this function to compare read value to a low-battery threshold and set flag accordingly
	//return ADCH;
	}
	
	