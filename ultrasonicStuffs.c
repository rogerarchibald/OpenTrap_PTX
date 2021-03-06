//
/*
 There is already a lot happening in Timers and so I'll put everything here that's specific to the Ultrasonic RangeFinder.  The Flow is as follows:

    - At the start of Main.c prior to the infinite loop, I'll call prep4Fire which will set up Timer1 to be ready to go.
    
    - 25mS into my 50mS loop, timers.c will call 'enableNoise' which will enable the ouptut and turn on the timer, pulsing the Xducer for ~200uS.  After it's done pulsing, I'll call 'kickTheCan(30) which will give me an interrupt 960uS later.  This will give a suitable blanking delay
 
    -State Machine has 4 states:
        -Prepped for Fire: No Timer running, timer is set up to generate firing pulses but no clock is enabled
        -MakeNoise: Firing transducer for ~200uS
        -deadTime: An 960uS delay where I'm allowing any direct-coupled signal to subside before listening for a return
        -listenMode: Here I'll arm INT0 and listen.  From here I'm going to get an interrupt: either INT0 when something bounces back, or Timer1Overflow if the signal is blocked.
 
    Numbers to remember: 148uS/inch is sonic speed.  When I'm running kickTheCan my timer resolution is 32uS (8Mhz clock with a /256 prescaler
 
 */

#include "NRF24_lib.h"
#include "ultrasonicStuffs.h"

static u8 ultrasonicMode;	//index my ultrasonic state machine
volatile u8 sonicDistance;    //This will be the total number of timer ticks since starting firing (32uS/tick) divided by 5.  This is a pretty close approximation to the distance in inches, and close enough for me since it'll prevent me having to go into floating point areas





//This will indicate a signal bounced back to the receiver while we were listening...This is the most common outcome because the trap will be empty and the reflector plate is there
ISR(INT0_vect){
    sonicDistance = (TCNT1+30); //TCNT1 is the current number of 32uS ticks.  30 is the number called by kickTheCan when I finished firing. From here I will just send out the
  
    prep4Fire();
  
} //end of ISR.






ISR(TIMER1_COMPA_vect){ 
    static u8 noisecounter = 0;	//variable to count how many timer rollovers I've had
    
    switch(ultrasonicMode){
            
        case makeNoise:
            noisecounter ++;
            if(noisecounter >= 16){	//this will determine how many times to pulse the transmitter 16 transitions = 8 pulses
                noisecounter = 0;	//reset for next time
                ultrasonicMode = deadTime;	//need to wait for any directly coupled noise to finish
                PORTB &= ~(0x04); //make sure that PORTB-2 is low
                kickTheCan(30);//starting point here. 30 * 32 =960...960/148 =~6.5inches' worth of blanking time.
                
            }
            break;
            
        case deadTime:  //this will be the return of 'kickTheCan' approx. 800uS after firing completes, where I'm idle to prevent triggering on direct-coupled signal.
            armINT0(1);
            kickTheCan(225); //this is pushing my timer rollover ISR out to (225 * 32uS = 7.2mS).  This plus the 960uS of dead time gives a maximum listen time after firing of just over 8mS.  This corresponds to ~4.6' of rangefinding range.
            ultrasonicMode = listenMode;
            break;
  
            
        case listenMode: //if I got here it's because of a CTC interrupt while listening, which means that the timer timed out before the signal boucned back.
            sonicDistance = 0; //if nothing bounced back call this zero.
            prep4Fire();
            break;
                                                                                                                
}	//end of switch/Case
}	//End of ISR





//Set up Timer1 to make noise but don't yet enable it
void prep4Fire (void){
    TCNT1 = 0;  //ensure we start from scratch to get the full width
    TCCR1B = (1 << WGM12);	//CTC mode with OCR1A as top
    OCR1A = 100;	//will count to 100 before clearing, with a 125nS clock period this will flip output every 12.5uS => 25uS period = 40kHz
    ultrasonicMode = makeNoise;	//set state machine so ISR knows where to go.
    TIFR1 = (1<<OCF1A);	//clear flag for OCR1A match
    TIMSK1 = (1<<OCIE1A); //enable interrupt
    armINT0(0); //disable the interrupt
    //next line just turns on the LED if I get SOMETHING back from the ultrasonic..If it tiems out I turn it off
    if(sonicDistance){
        led3_on}else{
            led3_off
        }
        
    
}	//end of prep4Fire.



//Enable T1 with a x1 prescaler driving OUTPUT
void enableNoise (void){
    TCCR1A = (1 << COM1B0);	//Toggle OC1B on Compare Match TCCR1A = (1 << COM1B0);
    TCCR1B |= (1 << CS10);	//set clock source to enable clock
}



/*kickTheCan will set up Timer1 for CTC with an OCR1A value defined by 'kickDelay'.  This lets me call a fresh ISR at some piont in the future
 to either switch from dead time delay to listening, or to give up listening
 */
void kickTheCan (u8 kickDelay){
    
    TCCR1A = 0;	//I don't care about outputs from timer here
    TCCR1B = (1 << WGM12) | (1 << CS12);	//enable CTC mode with a /256 prescaler(32uS/tick with my 8M Xtal)
    TCNT1 = 0;	//reset timer to zero
    OCR1A =kickDelay;	//how far down the road do I want an ISR?
    TIFR1 = (1<<OCF1A);	//clear flag for OCR1A match
    TIMSK1 = (1<<OCIE1A); //enable interrupt
}



//enable/disable INT0 depending on the variable onOrOff
void armINT0 (u8 onOrOff){
    EICRA  =0; //configure interrupt on low level of INT0
    EIFR |= (1 << INTF0);   //clear the interrupt flag
    EIMSK = (onOrOff << INT0);  //enable or disable the interrupt based on the argument that's passed
    
}


u8 getDistance(void){
    return sonicDistance;
}
