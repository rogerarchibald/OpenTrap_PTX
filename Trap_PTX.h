/*
 * Trap_PTX.h
 *
 * Created: 12/21/15 7:07:22 PM
 *  Author: Roger
 */ 


#ifndef TRAP_PTX_H_
#define TRAP_PTX_H_

//#define u8 uint8_t

#include <util/delay.h>
#include <avr/io.h>
#include "ADC.h"
#include "timers.h"
#include "NRF24_lib.h"
#include	"NRF24defs.h"





//function prototypes that are specific to the PTX:


u8 fill_TX_bitfield(void);	//This is going to check the various status for things that go in the PTX packet adn stuff the bitfield accordingly

void read_PRX(void);	//will be called when a packet from PRX is detected, to determine if there are new commands and act accordingly.

void clear_trap_busy(void);	//this will be called from the Timer0 ISR after finishing the trap-setting state machine has done it's thing.  Any calls to set the trap will be ignored until the 'trap busy' flag has been cleared.  


void camera_done(void);	//this will be called when teh shutter release has been set and the delay for keeping shutter release down has passed.










#endif /* TRAP_PTX_H_ */