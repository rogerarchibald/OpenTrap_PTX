/*
 * timer.c
 *  Here I'm using Timer2 to generate an interrupt every mS.  Timer0 will be used to drive the Boost converter to step my battery voltage up high enough to hit the solenoid
 *
 * Created: 4/28/15 9:21:53 PM
 *  Author: Roger
 */ 

#include "timers.h"	



static u32 mSecond = 0;		//millisecond counter coming at Timer2 Rollover

static u32 camera_release = 0xFFFFFFFF;	//used by mS ISR to determine how long to hold down the camera shutter release.
static u8	fiftymsroll = 50;	//decremented every millisecond, when it reaches zero is a flag that 50mS have passed
static u8	fiftyms_flag = 0;	//flag to be set every 50 milliseconds.
static u8 trap_status = 0;		//This flag will be set to 1 after the trap is set

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
	enableNoise();  //start pulsing the ultrsaonic transmitter
}
}	//end of mS rollover


ISR(TIMER0_COMPB_vect){
    static uint16_t chopToDrop = 0; //this will be how many times the timer rolls over before I turn on the low-side FET, and ultimately until shutting hte timer off.
    chopToDrop ++;
    if(chopToDrop >= 15){
        PORTD |= 0x40;  //turn on low-side FET for solenoid activation
    }
    if(chopToDrop>= 65500){ //this will be roughly .5 seconds which I think should suffice.  If I need to fire londer will need ot make chopToDrop 32-bit
        
        chopToDrop = 0; //reset for next time
        TCCR0A &= ~(1 << COM0B1); //make sure that COM0B1 is low so that output will be off.
        TCCR0B &= ~(1 << CS00); //disable clock
        _delay_ms(1);   //bleed off extra voltage after turning off switcher
        PORTD &= ~(0x40);   //turn off low-side FET
        clear_trap_busy();
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





/*Will use Timer0 to drive teh Boost Converter.  Will set up Timer0 in Fast PWM Mode with no prescaler and OCR0A as Top.  With this setup
 and an OCR0A value of 60, my clock frequency is ~131k.  OCR0B = 24 = 40% DC.  May tweak these values in the future
 Timer0 init just sets up everything but it doesn't enable output on OC0B or turn on the clock.  These will be done in the 'SetTrap' function
 */

void Timer0_init(void){
    OCR0A = 60; //this will be TOP, determines PWM Frequency
    OCR0B =24;  //this is trip point, will determine DC
    TCCR0A |= (1 << WGM01) | (1 << WGM00);   //non-inverting PWM with OCR0A as Top
    TCCR0B |= (1 << WGM02);
    TIFR0 |= (1 << OCF0B);  //clear interrupt flag
    TIMSK0 |= (1 << OCIE0B);    //unmask Timer0CompB Interrupt
    
    
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



//this is determined by the state machine and used as a flag in the bitfield being sent to the PRX
u8 chk_trap_status(void){
	return trap_status;
}

//When this is called it will enable Timer0 and start the chain of events that results in the trap setting.
void set_trap (void){
    TIFR0 |= (1 << OCF0B);
    TCCR0A |= (1 << COM0B1);    //route output from PWM to OC0B
    TCCR0B |= (1 << CS00);  //enable clocksource
}

