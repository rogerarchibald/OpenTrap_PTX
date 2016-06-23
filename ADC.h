/*
 * ADC.h
 *
 * Created: 6/13/15 1:22:20 PM
 *  Author: Roger
 */ 


#ifndef ADC_H_
#define ADC_H_


#include "NRF24_lib.h"
#include	"NRF24defs.h"


void ADC_Init(void);
u8 readADC(void);
u8 checkADC_Stat(void);
void start_ADC_conv (void);

#endif /* ADC_H_ */