/*
 * File:   lcd.c
 * Author: Andrew
 *
 * Created on February 1, 2017, 10:32 PM
 */

#include <xc.h>
#include "configBits.h"
#include <stdio.h>
#include "lcd.h"
#include "constants.h"

void initLCD(void) {
    __delay_ms(15);
    lcdInst(0b00110011);
    lcdInst(0b00110010);
    lcdInst(0b00101000);
    lcdInst(0b00001100); // Hide cursor, not blinking
    lcdInst(0b00000110);
    lcdInst(0b00000001);
    __delay_ms(15);
}

void lcdInst(char data) {
    RS = 0;
    lcdNibble(data);
}

void putch(char data){
    RS = 1;
    lcdNibble(data);
}

void lcdNibble(char data){
    // Send of 4 most sig bits, then the 4 least sig bits (MSD,LSD)
    
    //temp = 4 MSB
    char temp = data & 0xF0;
    
    // choose 4 LSB of LATD
    LATD = LATD & 0x0F;
    
    // put 4 MSB of data into 4 MSB of LATD
    LATD = temp | LATD;

    E = 0;
    __delay_us(LCD_DELAY);
    E = 1;    
    __delay_us(LCD_DELAY);
    
    // data = 
    data = data << 4;
    
    temp = data & 0xF0;
    LATD = LATD & 0x0F;
    LATD = temp | LATD;

    E = 0;
    __delay_us(LCD_DELAY);
    E = 1;
    __delay_us(LCD_DELAY);
}