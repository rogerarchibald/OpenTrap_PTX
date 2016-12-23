/*
 * timer.c
 *Will use this file to increment the mS timer and eventually to control the PWM for the LEDs
 * Created: 4/28/15 9:21:53 PM
 *  Author: Roger
 */ 

#include "timers.h"	



static u32 mSecond = 0;		//millisecond counter coming at Timer2 Rollover

static u32 camera_release = 0xFFFFFFFF;	//used by mS ISR to determine how lonw to hold down the camera shutter release.
static u8	fiftymsroll = 50;	//decremented every millisecond, when it reaches zero is a flag that 50mS have passed
static u8	fiftyms_flag = 0;	//flag to be set every 50 milliseconds.
static u8 trap_status = 0;		//This flag will be set to 1 after the trap is set and the state machine to set it has gone through all of its stages.

ISR(TIMER2_COMPA_vect) //Will use this ISR to manage mS timer and call functions that need to get ramped/amped every X mS 
{   

mSecond ++;
fiftymsroll --;
if (0 == fiftymsroll){
	fiftymsroll = 50;
	fiftyms_flag = 1;	
}

if(mSecond >= camera_release){
	camera_done();	//if the mS timer has passed its target value, let the 'Trap_PTX' file know via timesup.
}

    if(fiftymsroll == 25){
	make40K();  //start pulsing the ultrsaonic transmitter
}
}	//end of mS rollover



ISR(TIMER0_OVF_vect) //Timer0 will rollover about every 65mS when running, as this is the clock for generating low freq. pulses for the servo.  Will use this rollover to increment a variable and compare it to a value set in the state machine
{
	static u8 t0ovf = 0;	//initialize compare variable. This gets incremented every rollover and compared to a target value to advance the state machine.
	static u8 t0ovf_target =1;		//this is the target variable that gets set every rollover
	u8 tmp_returnvar = 0;	//state runner will return this variable.  When the state machine is done this will be set to 1.  need to implement checking of this at end of ISR.
	t0ovf ++;
	if (t0ovf == t0ovf_target){
		t0ovf = 0;
		tmp_returnvar = state_runner(&t0ovf_target);	//call 'state_runner' which is a function pointer that's pointed to the applicable state.  The state machine will set the next t0ovf_target value and point the function pointer to the correct stage.  Will return a flag 
		if(tmp_returnvar == 1) {}	//state_runner will return a 1 when the servo is in the armed position and has been turned off.
			else if (tmp_returnvar == 2) {
				clear_trap_busy();	//let the PTX file know that the state machine has run its course
				trap_status = 1;	//setting flag that will be checked by the PTX TX function
				}	
				
	}
}


//return current mS timer
u32 get_mS_cnt (void){
	return (mSecond);
}



//when I want to take a picture will pull down the shutter release pin and then use this to set a delay.  When hte mS timer passes delay_time I'll call a callback function to release the shutter release and clare teh 'camera busy' flag.
void set_camera_delay(u32 delay_time){
	if(delay_time){
		camera_release = (mSecond + delay_time);
	}else{
		camera_release = 0xFFFFFFFF;
	}
}





/*Using Timer 0 output A to drive pulses into Servo.  Here's some early/initial config.  Will also want to have the ability to turn timer off to save battery when not drivng the servo.
with the 15.3Hz frequency I'm getting, the resoulution is ~256uS/timer unit.  Not a whole lot of range but it will get me the extremes I need for setting/initializing the servo*/
void Timer0_init(void){
	TCCR0A = 0x81;	//setting up OC0A and WGM00
	TCCR0B = 0x05;	//setting /1024 prescaler which should be good for ~15Hz, perfect for servo
	OCR0A = 0x05;		//Arm Trap
	state_runner = pre_arm;	//initializing pointer to point at the pre-arm function.  This will turn on the servo's power and initialize the timer.
	TIFR0 |= 0x01;	//write one to interrupt flag to clear it prior to enable.
	TIMSK0 |= 0x01;	//enable interrupt on Timer0 overflow
}


//millisecond timer implemented via compare ISR
void Timer2_init(void) {
//8-bit timer 2 for mS timer	
	OCR2A = 250;	//this is top.  Timer has 8M clock /32 prescaler(*250) = 1Khz
	TCCR2A = 0x42;	//This will change to 0x02 eventually, right now using COMA2 as output for testing
	TCCR2B = 0x03;
	TIFR2 |= 0X02;	//CLEAR Interrupt flag prior to enabling interrupts (by writing a 1 to it weird huh?)
	TIMSK2 |= 0X02;	//enable Timer2 Compare Interrupts
}





//see if 50mS have rolled over and if so reset the roll.
u8	fifty_stat(void){
	if(fiftyms_flag){
		fiftyms_flag = 0;
		return 1;
	}else{
		return 0;
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////
/*State Machine Functions which will be cycled through based on calls from Timer0 rollover interrupt.  The flow is a bit weird but seems to me like the best way to do it.  Initially will go to pre-arm, this is pointed to by the Timer0 init function
*/

u8 pre_set (u8* wait_time){
	OCR0A = 0;	//make sure timer is off
	TCCR0B |= 0x05;	//setting prescaler bits which will enable clock
	PORTD |= 0x20;	//turn on servo
	*wait_time = 0x03;	//this will be the number of 65mS period counter rollovers...3 corresponds to ~185mSeconds...need to scope the time for caps to charge.
	state_runner = set;
	return 0;	//will only return 1 at end of 'off' pointer
}


u8 set (u8* wait_time){
	OCR0A = 0x09;	//value to set trap
	*wait_time = 0x10;	//rollovers of 65mS clock...This is about 1 second
	state_runner = arm;	//skip over pre-arm, that's where we'll intially jump into this at power-up
	take_picture();	//trigger shutter release to snap a shot as the trap is setting
	return 2;	//if seeing a 2 on the return can set a flag saying that we're set.
}


//pre-arm will get pointed to right after the timer is initialized, as this will get everything set up for arming the trap and waiting for the timer to be re-activated after it is pointed towards 'pre-set'.
u8 pre_arm (u8* wait_time){
	OCR0A = 0;	//ensure we're off
	TCCR0B |= 0x05;	//make sure clock is enabled
	PORTD |= 0x20;	//turn on servo
	*wait_time = 0x08;		//this will be the number of 65mS period counter rollovers...8 corresponds to ~.5 seconds
	state_runner = arm;	//point to arm where this will actually run the servo to the arm position.
	return 0;
}


//was hoping to disable timer in the name of saving power when the micro is sleeping.  Turns out that the micro gives a quick short pulse while turning off/on which is making noise on the servo.  Need to either 
//shut off servo before disabling/enabling pulser or just live with the timer drawing current while down.  
u8 arm (u8* wait_time){
	OCR0A = 0x05;	//Value to arm trap
	*wait_time = 0x10;	//wait about 1 second to ensure that the servo has reached it's goal.
	state_runner = off;	//point to off.
	return 0;
}


u8 off (u8* wait_time){
	OCR0A = 0;
	PORTD &= ~(0x20);	//turn off power to servo
	*wait_time = 0x01;	//make sure that the new overflow counter target is going to get reached next ISR. 
	TIMSK2 &= ~(0x01);	//disable Timer0 Overflow Interrupts
	return 1;
}

//////////////////////////////////////////////////////////////end of state machine functions

//this is determined by the state machine and used as a flag in the bitfield being sent to the PRX
u8 chk_trap_status(void){
	return trap_status;
}

//When this is called it will re-initialize the Timer0 interrupt and point function pointer towards getting ready to set.
void set_trap (void){
	TIMSK2 |= 0x01;	//re-enable Timer0 Overflow Interrupts
	state_runner = pre_set;	//point to preparing to arm
}

