/*
 * timers.h
 *
 * Created: 4/28/15 9:20:42 PM
 *  Author: Roger
 OCR0A values for running trap:
 OCR0A = 9	arm	~2.3mS	
 OCR0A = 5	set	~1.7mS
 */ 


#ifndef TIMERS_H_
#define TIMERS_H_





#include <avr/io.h>
#include <avr/interrupt.h>
#include "NRF24_lib.h"
#include "ADC.h"
#include "Trap_PTX.h"
#include "ultrasonicStuffs.h"




//////////////////////////////////////////////////////////
/*Initialization function prototypes*/
void Timer2_init(void);	//Timer2 will be used for mS timer
void Timer1_init(void);	//Timer1 will generate 38Khz signal for LED
void Timer0_init(void);	//Timer0 will drive Servo output for trap.

//prototype function pointer that'll run through the state machine of setting the trap based on Timer0 rollovers.
u8 (*state_runner)(u8* time);

//Servo Drive State Machine Function Prototypes:
u8 pre_set (u8* wait_time);
u8 set (u8* wait_time);
u8 pre_arm (u8* wait_time);
u8 arm (u8* wait_time);
u8 off (u8* wait_time);

//functions related to mS timer.  One will return current mS value, the other will set a value that the mS ISR will compare to the current mS timer and call a function if the timer has gotten there.
u32 get_mS_cnt (void);	
void set_camera_delay (u32 delay_time);

u8	fifty_stat(void);	//see if 50mS have rolled over and if so reset the roll
u8 chk_trap_status(void);	//this will return a 0 if the trap hasn't been set, a 1 if it has been set.
void set_trap (void);		//once everything is armed this will set the wheels in motion to set the trap.
void take_picture(void);		//trigger shutter release

//functions to enable/disable 38K timer for Ir LED
void T38KOff(void);
void T38KOn(void);


#endif /* TIMERS_H_ */