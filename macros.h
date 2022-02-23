/* 
 * File:   macros.h
 * Author: Andrew
 *
 * Created on February 3, 2017, 3:31 PM
 */

#ifndef MACROS_H
#define	MACROS_H

#define __delay_1s() for(char i=0;i<10;i++){__delay_ms(98);}


// Go to address 0x00 (start of first line)
#define __lcd_line1() lcdInst(0b10000000);__delay_ms(10);
// Go to address 0x40 (start of 2nd line)
#define __lcd_line2() lcdInst(0b11000000);__delay_ms(10);

#define __lcd_clear() lcdInst(0x01); __delay_ms(10);

#define __bcd_to_num(num) (num & 0x0F) + ((num & 0xF0)>>4)*10

#endif	/* MACROS_H */

