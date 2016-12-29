//
//  ultrasonicStuffs.h
//  Smart_trap_sonic
//
//  Created by Roger on 6/25/16.
//  Copyright © 2016 Roger. All rights reserved.
//

#ifndef ultrasonicStuffs_h
#define ultrasonicStuffs_h

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>


//function prototypes
void prep4Fire(void);	//set up timer to make 40K signal but don't turn it on.  Will be called at startup and also after getting a sample
void enableNoise(void);    //enable Timer after it being previously prepped
void kickTheCan (u8 kickDelay); //set a callback for some point in the future...To either wait for blanking time or set a timeout for listening
void armINT0 (u8 onOrOff);  //arm or disarm INTO based on variable onOrOff which will just be 0/1.



//the two modes that the ISR will use to determine if I'm in pulse-making mode, blanking-time mode or waiting mode
enum sonicTrapModes{
    makeNoise,
    deadTime,
    listenMode
};









#endif /* ultrasonicStuffs_h */
