/*
main.c
 *
 * Created: 5/9/15 6:07:47 PM
 *  Author: Roger
 
This code is currently for board Rev4 which uses an ultrasonic range finder as opposed to the older IR.  All the sonic stuff is un ultrasonicStuffs.c.
 
 
 I'm making a mS timer off of Timer2 and this will crank up some RF comms every 50 mS.  halfway through that 50mS countdown I'm going to fire the Ultrasonic transmitter and then wait to see what bounces back.
 
 */


#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "NRF24_lib.h"
#include "NRF24defs.h"
#include "ADC.h"
#include "timers.h"
#include "Trap_PTX.h"
#include "USART.h"







//use following line if device is acting as PTX or PRX.  //out the define if used as PTX.  Will use #ifdef in initialization function 
//#define	PRX


u8 dataout [] = {0x15, 0x55};	//Here are my 2 bytes, the first is a bitfield and the second is initially unused
u8 packetin = 0;



int main(void)
{
	
DDRD |= 0xFA;	//Chip Select Not, TXData UART, Timer0A outputs, servo power enable, IR input on PD2.  Power_on output on PD4.  Light control on PD3
PORTD |= 0x10;	//Setting PortD4 to keep the power turned on.
Timer0_init();	//initialize Timer0 for Servo drive
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
initUSART();

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





