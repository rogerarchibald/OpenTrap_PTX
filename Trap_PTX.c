/*
 * Trap_PTX.c
 *
 * Created: 12/21/15 7:06:26 PM
 *  Author: Roger

 */ 

#include "Trap_PTX.h"

#define	MAX_PACKET_LENGTH	2
u8 datain [MAX_PACKET_LENGTH];	//what I'm receiving from PRX
volatile static u8 trap_busy = 0;	//this is a flag that will be set when starting the 'set trap' state machine and cleared when the state machine is finished.
volatile static u8 camera_busy = 0;	//flag to detect if the camera is busy or not.  Will be set when hitting the shutter release cleared when releasing the shutter release.









//This will call the PRX receive function and return the LSByte
void read_PRX(void){
	get_RX_Data(datain);

	//this will be the check to see if the remote is sending command to set the trap and also the trap state machine is not currently active
	if((datain[0] & 0x80)&&(!trap_busy)){
		trap_busy = 1;	//set busy flag
		set_trap();
	}//end of checking if the trap command is being sent
	if((datain[0] & 0x10)&&(!camera_busy)){
		take_picture();
		}
		//PWM lights based on signal from PRX. 
	if(datain[0] & 0x40){
		PORTD |= 0x08;
	}	else{
		PORTD &= ~(0x08);
	}//turn hi-power external LED on or off based on status of datain bit6.

	}	//end of read_PRX function


void take_picture(void){
	camera_busy = 1;	//set busy flag
    PORTB |= 0x02;	//set base of BJT controlling shutter release
	set_camera_delay(1000);	//set a 1 second delay before the mS timer will call back the 'camera_done' function
}



//clearing the 'trap_busy' flag.  This will be called when the 'set trap' state machine has run its course.
void clear_trap_busy(void){
	trap_busy = 0;
}




void camera_done(void){
PORTB &= ~(0x02);	//stop pulling shutter release
set_camera_delay(0);	//sending zero here will stop callbacks(push them back until the end of time)
camera_busy = 0;	
}

