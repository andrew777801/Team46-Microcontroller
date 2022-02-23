/* 
 * File:   I2C.h
 * Author: Andrew
 *
 * Created on February 13, 2017, 4:19 PM
 */

void I2C_Master_Init(const unsigned long c);
void I2C_Master_Write(unsigned d);
unsigned char I2C_Master_Read(unsigned char a);
void I2C_Master_Stop();



