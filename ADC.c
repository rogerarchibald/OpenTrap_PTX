/*
 * ADC.c
 *
 * Created: 6/13/15 1:09:27 PM
 *  Author: Roger
 
 
 Want to set up the ADC as 8-bit, will left-align the results and drop the Low register...8-bits is plenty of resolution for my needs here and it's easier to deal with.
 I'll use VCC as the analog reference so in this case it's 3.3V.  My divider is 715k into 499k for a grand total of a .411 divide-by.  That means my max input voltage is 8V to keep this from exceeding VCC...I could squeeze a little more precision out of this but for now I'm close enough to check a battery.

 So the ultimate math = ((ADCH/255)*3.3)/.411= battery voltage....->ADCH * .0313 = battery voltage.
 
 */

#include "ADC.h"
#include "USART.h"
#include "NRF24_lib.h"


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


//For right now this is returning the left-alligned (upper 8 bits of the 10-bit ADC) 8-bit conversion.  Will set or clear LED2 based on battery level...below 6.8V I will set it.
u8 readADC(void){
	ADCSRA |= 0x10;	//clear interrupt flag
	if(ADCH > 217){	//217 is roughly 6.8V with my 715k/499k divider)
        led2_off}else{
            led2_on
        }
	return ADCH;
}
	
	
