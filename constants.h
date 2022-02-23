/* 
 * File:   constants.h
 * Author: Andrew
 *
 * Created on February 4, 2017, 10:07 PM
 */

#ifndef CONSTANTS_H
#define	CONSTANTS_H         //Prevent multiple inclusion 

//LCD Control Registers
#define RS          LATDbits.LATD2          
#define E           LATDbits.LATD3
#define	LCD_PORT    LATD   //On LATD[4,7] to be specific
#define LCD_DELAY   25


#endif	/* CONSTANTS_H */

