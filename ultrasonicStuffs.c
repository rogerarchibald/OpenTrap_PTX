//
/*
 There is already a lot happening in Timers and so I'll put everything here that's specific to the Ultrasonic RangeFinder.  The Flow is as follows:
 
- At 25mS into my 50mS rollover, I'll set up Timer1 to drive a 40kHz pulse train onto the TX Xducer for 8 cycles (200uS total).  At the 200uS mark, will turn off the Xmit, clear TCNT1 and put Timer1 into normal mode with a /64 prescaler.  Will set OCR1A to 255 and enable an interrupt on OCR1A compare.  My hope in keeping OCR1A at 255 is that we I can keep everything in the 8-bit realm, but since Timer1 is 16-bit it would be easily expandable in teh future if need be.  Sonic velocity is ~74uS/inch  and so I'm looking at 148uS/inch round trip.  So if I'm using a /64 presaler and an 8M clock, I should get a timer tick every 8uS, with my compare interrupt at 255 happening every (8uS * 255 = 2.04mS).  So I've got a 2.04mS window and sound going at 148uS/" = a maximum distance of 2.04mS/148uS/inch = 13.78".  This is closer to the 12" trap disance than I'd like, but I can't see why it wouldn't work and I can always bump up OCR1A if I find that for some reason I need more
 
 -So after 200uS of pulsing 40kHz, I will shut down XMIT, clear TCNT1 and turn on Timer1 in normal mode as described above.  At that time I will set up an interrupt on INT0 which is the line where my envelop detector comes in.  If I get a OCR1A compare interrupt, that's telling me that the timer ran out (kitty blocking the sonic signal) If I get an INT0 interrupt, I need to look at TCNT1.  If it is in the happy range (~222, will need to learn tolerance on that) then the sound bounced off the bottom of the trap.  If it's substantially less, it's probably due to something bouncing the signal back.
 
 //
 
 */

#include "NRF24_lib.h"
#include "ultrasonicStuffs.h"
#include "USART.h"

static u8 mode = pulseGen; //pulseGen is for making pulses, Counter is for waiting.  The compare match ISR will use this to determine how to behave





ISR(TIMER1_COMPA_vect){ //Will use this to keep track of how many times the 40K pulser has pulsed or to know that the timer has reached its limit while waiting for a sonic bounce-back
    
static u8 pulsecounts = 0;
    
    if(mode == pulseGen){
        pulsecounts ++;
      
        if(pulsecounts >= 16){  //after 8 pulses of the 40Khz
            pulsecounts = 0;
            TCCR1A = 0; //give control back to PORT
            TCCR1B = 0; //kill timer input
            mode = Counter;//end of what to do if mode = Pulsegen
            wait4Rebound();
        }   //end of what do after 8 pulses
    } else{ //end of what to do if mode = Pulsegen, start of what to do if mode == Counter
        //this is the state I'll get to if I've set up Timer1 to interrupt after ~2.04mS and it has gotten to that point (i.e. no sound signal bounced back and caused the INT0 interrupt before this rolled over).
      
        
        TCCR1B = 0; //kill timer source for now, it will be re-enabled the next time 'make40K' is called
        mode = pulseGen;
    }

} //end of ISR



void make40K(void){
  
    TCNT1 = 0;  //start fresh
    TIFR1 |= (1<<OCR1A);    //clear interrupt flag for Timer1 Compare A
    TIMSK1 |= (1<<OCIE1A);  //enable interrupt
    OCR1A = 100;    //with no prescaler and a compare match value of 100, this should give me a CTC of 40Khz
    TCCR1A = (1<< COM1B0);  //set up OC1B to toggle on CTC match
    TCCR1B =   (1 << WGM12) | (1 << CS10);  //set CTC with OCR1A as compare value and a prescaler of 1
      led2_on

}


 
 
 
 void wait4Rebound(void){
    //TODO: set up T1 for normal mode, enable interrupt on falling edge of INT0
     TCNT1 = 0; //reset counter to start fresh
     OCR1A = 255;   //8uS period counting to 255 = 2.04mS
     TCCR1B = (1 << CS10) | (1<< CS11); //give me a /64 prescaler
   
     printByte (OCR1A);
     printString ("\n");
    
}



