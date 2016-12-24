//
/*
 There is already a lot happening in Timers and so I'll put everything here that's specific to the Ultrasonic RangeFinder.  The Flow is as follows:

    - At the start of Main.c prior to the infinite loop, I'll call prep4Fire which will set up Timer1 to be ready to go.
    
    - 25mS into my 50mS loop, timers.c will call 'enableNoise' which will enable the ouptut and turn on the timer, pulsing the Xducer for ~200uS.  After it's done pulsing, I'll call 'kickTheCan(25) which will give me an interrupt 800uS later.  This will give a suitable blanking delay
 
 at 2' and .5" I saw a delay of 3.68mS from the end of firing to a return...With 22.5" I saw3.32mS....17.5" gave 2.56mS
 
 */

#include "NRF24_lib.h"
#include "ultrasonicStuffs.h"
#include "USART.h"

static u8 ultrasonicMode;	//index my ultrasonic state machine





ISR(TIMER1_COMPA_vect){ 
    static u8 noisecounter = 0;	//variable to count how many timer rollovers I've had
    
    switch(ultrasonicMode){
            
        case makeNoise:
            noisecounter ++;
            if(noisecounter >= 16){	//this will determine how many times to pulse the transmitter 16 transitions = 8 pulses
                noisecounter = 0;	//reset for next time
                ultrasonicMode = deadTime;	//need to wait for any directly coupled noise to finish
                TCCR1A = 0;	//I don't care about outputs from timer here
                PORTB &= ~(0x04); //make sure that PORTB-2 is low
                kickTheCan(25);//starting point here. 25 * 32 =800...800/148 =~5.4inches' worth of blanking time.
            }
            break;
            
        case deadTime:
            
        {prep4Fire();
        tog_tp1}
            break;
    /*    TODO: clean up all of the above code and test/make sure that I understand the delays and can get tothis point.
            Once here, I need to check if the pin is already receiving something and if so, return with minimum(4"???) delay.
        If I'm not already receiving anything, call 'kickTheCan' to start the timer. This will limit how long I'll be able to
        listen (via ISR) and also I can check the value of TCNT1 when the receiver does getsomething, and this will be the manner to determine distance
     
    */
                                                                                                                
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
