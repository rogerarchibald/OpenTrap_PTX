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
void make40K(void);	//turn on the 40Khz drive
void wait4Rebound(void);    //set up timer to cound time until sound bounces back or compare interrupt happens




//the two modes that the ISR will use to determine if I'm in pulse-making mode or waiting mode
enum timermodes{
    pulseGen,
    Counter
};









#endif /* ultrasonicStuffs_h */
