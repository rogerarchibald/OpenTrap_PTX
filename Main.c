/*
main.c
 *
 * Created: 5/9/15 6:07:47 PM
 *  Author: Roger
 Starting notes here at PTX_rev3 which has a new type of motion indicator: Will use an optical gate as opposed to the PIR.  Hoping this will give better noise/thermal immunity.  There are a few changes to the circuit that will need firmware changes.  They are:
 The light control was on PB2.  It's been moved to PD3, will need to change LED/LDO drive
 Now need to generate a 38Khz drive signal, will use Timer1B which is tied to TB2...Hence need for above change.
 PIR_IN is now IR_IN.  The signal in is active low and will be inactive when the path is blocked, so the logic from before holds true.  Initially I'm going to consistently drive the IR_LED, but for battery conservation (>100mA draw from that guy) I'll eventually disable it after a read.  Will need to determine the best moment for that.
 */ 


#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "NRF24_lib.h"
#include "NRF24defs.h"
#include "ADC.h"
#include "timers.h"
#include "Trap_PTX.h"








//use following line if device is acting as PTX or PRX.  //out the define if used as PTX.  Will use #ifdef in initialization function 
//#define	PRX


u8 dataout [] = {0x15, 0x55};	//Here are my 2 bytes, the first is a bitfield and the second is initially unused
u8 packetin = 0;



int main(void)
{
	
DDRD |= 0xFA;	//Chip Select Not, TXData UART, Timer0A outputs, servo power enable, IR input on PD2.  Power_on output on PD4.  Light control on PD3
PORTD |= 0x10;	//Setting PortD4 to keep the power turned on.
Timer0_init();	//initialize Timer0 for Servo drive
Timer1_init();	//generate the 38Khz signal for driving the IR LED
Timer2_init();	//using this for mS timer/50mS rollover
ADC_Init();
DDRB = 0x2F;		//All of portB = outputs with exception of PB6-7 (XTAL), and PB4 (MISO).
/*PB0 = Chip Enable, PB1 = LED Out for switcher, PB2 = LED_PWM (also is SS, not used here but can't be left floating input), PB3 = MOSI, PB4 = MISO, PB5 = SCK, PB6-7 = oscillator*/
//This rev board has an external pullup, want to try that experimentally for now.  PORTB |= 0x02;	//setting pullup on PB1 which is the shutter release.
DDRC |= 0xFF;	//PC0 = OnBoard LEDs, PC3 = TP1
PORTC = 0;
//TODO: following line is leftover from PIR, 
EICRA = 0x01;	//configure INT0 to trigger on any logical change, the interrupt will be unmasked in Trap_PTX.c when it's needed.
sei();	//enable globals


//initialize SPI on AT328
SPCR = (1<<SPE)|(1<<MSTR);	

#ifdef PRX
enable_PRX();
#endif

initialize_NRF();

    while(1)
 {   
	
	if(fifty_stat()){
		
		dataout[0] = fill_TX_bitfield();		//function to populate bitfield after scanning appropriate stuffs.
		T38KOff();	//turn of 38K Timer.  This should probably go elsewhere, just trying it for now
	
	
	sendPayLoad(W_TX_PAYLOAD, dataout, 2);
	}	//end of where to go if we've detected a 50mS rollover via fifty_stat

		if(check_Flag(RX_DR)){		//did I detect an awk-pack?
			led1_tog;		//here just want to see if I can see that I've received data and understand the mechanics of resetting the flag.
			read_PRX();			
		}	//end of what to do if an AWK-PAC
		


	if(check_Flag(MAX_RT)){	//this is saying that max retries have been sent out without a successful response.  All of this might be a bit excessive.
		flushTx();	//clear transmitter FIFO
		flushRx();
		clear_Flag(ALL_FLAG);
		//TODO: Increment an error counter	
	}	//end of what to do if an error flag comes back from teh NRF




	}	//end of while(1)

}	//end of main





