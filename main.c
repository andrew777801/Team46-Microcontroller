/*
 * File:   main.c
 * Author: Andrew
 *
 * Created on February 1, 2017, 10:36 PM
 */

#include <xc.h>
#include "configBits.h"
#include <stdio.h>
#include "lcd.h"
#include "constants.h"
#include "macros.h"
#include "I2C.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <eeprom_routines.h>

void set_time(void){
    I2C_Master_Start(); //Start condition
    I2C_Master_Write(0b11010000); //7 bit RTC address + Write
    I2C_Master_Write(0x00); //Set memory pointer to seconds
    for(char i=0; i<2; i++){
        I2C_Master_Write(0x00); // Set initial time to 0 min 0 sec.
    }    
    I2C_Master_Stop(); //Stop condition
}

void activate_stepper(unsigned char motor){
    LATA = 0xFF;     // Initialize/pause motors 
    __delay_ms(50);
    if(motor < 4){
       
        LATA = (1 << ((motor*2)-1));    // Select motor based on input
        __delay_1s();
        __delay_ms(200);                // Wait until the motor rotated 90 deg.
    }
    LATA = 0xFF;     // Initialize/pause motors
    return;
}

void rotate(){
    LATEbits.LATE2 = 0;
}

void stop_rotate(){
    LATEbits.LATE2 = 1;
}

void agitator_move(unsigned bit a, unsigned bit b){
    // Macro to activate agitator
    LATEbits.LATE0 = a;         
    LATEbits.LATE1 = b;
}

void read_sensor(unsigned char r[], unsigned char g[], unsigned char b[], unsigned char c[]){
    // Initialize I2C for TCS34725
        I2C_Master_Start();
        I2C_Master_Write(0b01010010); //Sensor address 0x28 + Write
        I2C_Master_Write(0b10000000); //Write Command, specify enable
        I2C_Master_Write(0b00000011); //Enable RGBC and power
        I2C_Master_Stop();
        // Set CLEAR register
        I2C_Master_Start();
        I2C_Master_Write(0b01010010); //Sensor address 0x28 + Write
        I2C_Master_Write(0b10110100); //Write COMMAND, specify CLEAR and auto-increment
        
        //Read sensor data
        //Read clear data
        I2C_Master_Start();
        I2C_Master_Write(0b01010011); //Sensor address 0x28 + Read
        c[0] = I2C_Master_Read(1);
        c[1] = I2C_Master_Read(1); 

        r[0] = I2C_Master_Read(1);
        r[1] = I2C_Master_Read(1);

        g[0] = I2C_Master_Read(1);
        g[1] = I2C_Master_Read(1);
        
        b[0] = I2C_Master_Read(1);
        b[1] = I2C_Master_Read(0);
        I2C_Master_Stop();
        return;
}

void poweroff_sensor (){
    // Power off sensor when done
    I2C_Master_Start();
    I2C_Master_Write(0x00); //ENABLE address + write
    I2C_Master_Write(0x00); //Disable RGBC and power
    I2C_Master_Stop();
    return;
}

void return_motors(unsigned char motor){
    //Return motors back to original position
    LATA = 0xFF;     // Initialize/pause motors

    switch (motor){
        case 1:
            LATAbits.LATA0 = 1;
            LATAbits.LATA1 = 0;
            break;
        case 2:
            LATAbits.LATA2 = 1;
            LATAbits.LATA3 = 0;
            break;
        case 3:
            LATAbits.LATA4 = 1;
            LATAbits.LATA5 = 0;
            break;
    }
    __delay_1s();
    __delay_ms(210);
    
    return;
}

void timer_done(unsigned char time[]){
    //Reset RTC memory pointer 
    I2C_Master_Start(); //Start condition
    I2C_Master_Write(0b11010000); //7 bit RTC address + Write
    I2C_Master_Write(0x00); //Set memory pointer to seconds
    I2C_Master_Stop(); //Stop condition

    //Read Current Time
    I2C_Master_Start();
    I2C_Master_Write(0b11010001); //7 bit RTC address + Read 

    time[0] = I2C_Master_Read(1);
    time[1] = I2C_Master_Read(0); //Final Read without ack
    I2C_Master_Stop();
    return;

}

void tca_select(unsigned char i){
    I2C_Master_Start();
    I2C_Master_Write(0b11100000);     // TCA9548A Address +write
    I2C_Master_Write(1 << i);         // Chooses a sensor based on input
    I2C_Master_Stop();
}

char check_B_press(unsigned char key, unsigned char time[]){
    
    timer_done(time);
    if (time[1] >= 1 && time[0] >= 60)  // Check the time first
        return 1;
    if (PORTBbits.RB1 == 1 && key == 0b0111){
        // Wait until B key has been released
        while(PORTBbits.RB1){
        }
        Nop();  //Apply breakpoint here because of compiler optimizations
        Nop();
        // Stop motors
        
        LATA = 0xFF;     // Initialize/pause motors
        return 1;

    }
    return 0;
}

void main(void){
    
    // <editor-fold defaultstate="collapsed" desc=" Register Definitions ">
    OSCCON = 0xF0;  //8Mhz
    
    // Defining inputs/outputs
    TRISA = 0x00;   //Motor outputs
    TRISB = 0xFF;   //Keypad - all inputs
    TRISC = 0xFF;   //Sensor input
    TRISD = 0x00;   //LCD - all outputs
    TRISE = 0x00;   //Solenoid outputs
    
    LATB = 0x00;
    LATA = 0xFF;     // Initialize/pause motors    
    LATE = 0xFF;    // Initialize solenoids
   
    ADCON0 = 0x00;  //Disable ADC
    ADCON1 = 0xFE;  //Set PORTB to be digital instead of analog default, set AN1 analog  

    //</editor-fold>    
    
    initLCD();
    
    unsigned int binsCount[4] = {0,0,0,0}; // Count bottles for each bin
    unsigned char finalTime[2];            // Records time of operation
    
    // <editor-fold defaultstate="collapsed" desc=" Sensor Variable Definitions ">
    
    //Higher is more opaque
    unsigned char C1 [2];   
    unsigned char R1 [2];   
    unsigned char G1 [2];
    unsigned char B1 [2];    

    unsigned char C2 [2];    
    unsigned char R2 [2];   
    unsigned char G2 [2];
    unsigned char B2 [2];
    
    unsigned char C3 [2];    
    unsigned char R3 [2];   
    unsigned char G3 [2];
    unsigned char B3 [2];
    
    // 16 bit versions
    unsigned int r1;
    unsigned int b1;
    unsigned int c1;
    unsigned int r2;
    unsigned int b2;
    unsigned int c2;
    unsigned int r3;
    unsigned int b3;
    unsigned int c3;
    // Ratios
    unsigned int rcap1;
    unsigned int rcap2;
    unsigned int bcap1;
    unsigned int bcap3;
    
    unsigned int tot1;
    unsigned int tot2;
    unsigned int tot3;
    //</editor-fold>
    
    // <editor-fold defaultstate="collapsed" desc=" Startup + Standby ">
    __lcd_line1();
    printf ("(Startup) Press");
    __lcd_line2();
    printf ("A to begin.");
    // Standby #1
    while(1){
        unsigned char keypress = (PORTB & 0xF0)>>4;
        
        if (PORTBbits.RB1 == 1 && keypress == 0b0011){
        // Wait until key has been released
        while(PORTBbits.RB1){}
        Nop();  //Apply breakpoint here because of compiler optimizations
        Nop();
        break;
        }
    }
    // </editor-fold>
    
    __lcd_clear();
    __lcd_line1();
    printf("Sorting.");
    __lcd_line2();
    printf("Press B to stop.");
    
    //Initialize I2C Master with 100KHz clock
    I2C_Master_Init(10000);
    di(); // Disable all interrupts
    set_time();


    
    // <editor-fold defaultstate="collapsed" desc=" Main Operation ">
    // Standby #2
        
    // Scan -> gate up -> gate down -> solenoid push
    while(1){            
        unsigned char keypress = (PORTB & 0xF0)>>4; // Read key press
        unsigned char currentMotor = 4;             // Default: no gate raised
        // Scan sensors
        tca_select(1);
        read_sensor(R1, G1, B1, C1);
        tca_select(2);
        read_sensor(R2, G2, B2, C2);
        tca_select(3);
        read_sensor(R3, G3, B3, C3);    
        
        // Set threshold values
        r1 = (R1[1] << 8) | R1[0];
        b1 = (B1[1] << 8) | B1[0];
        c1 = (C1[1] << 8) | C1[0];
        r2 = (R2[1] << 8) | R2[0];
        b2 = (B2[1] << 8) | B2[0];
        c2 = (C2[1] << 8) | C2[0];
        r3 = (R3[1] << 8) | R3[0];
        b3 = (B3[1] << 8) | B3[0];
        c3 = (C3[1] << 8) | C3[0];
        rcap1 = r1/b1;
        rcap2 = r2/b2;
        tot1 = r1 + b1 + c1;
        tot2 = r2 + b2 + c2;
        bcap1 = b1/r1;
        bcap3 = b3/r3;
        tot3 = r3 + b3 +c3;
  
        
        
        // If a signal is received, write to motors, then dispense, then count
        // BIN 1: YOP CAPPED
        if ((rcap1 > 1.8 && tot2 > 10000) || (rcap2 > 1.8 && tot1 > 10000)){

            currentMotor = 1;
            activate_stepper(currentMotor);
            rotate();
            binsCount[0]++;
        }
        // BIN 2: YOP UNCAPPED
        else if (tot2 > 10000 || tot1 > 10000){

            currentMotor = 2;
            activate_stepper(currentMotor);
            rotate();
            binsCount[1]++;

        }
        // BIN 3: ESKA CAPPED   
        else if (bcap1 > 0.9 || bcap3 > 0.9){


            currentMotor = 3;
            activate_stepper(currentMotor);
            rotate();
            binsCount[2]++;
        }
       
        //BIN 4: ESKA UNCAPPED 
        else if ((tot1 > 2800 || c3/c1 > 1.0)){
            // No gate raised
            currentMotor = 4;
            activate_stepper(currentMotor);
            rotate();
            binsCount[3]++;
        } 
  
        // Periodically check for stop conditions
        if (check_B_press(keypress, finalTime)==1)
            break;        
        
        // Change agitator directions
        agitator_move(0,1);
        
        __delay_ms(1128);
        
         // Change agitator directions
        stop_rotate();
        
        // Periodically check for stop conditions
        if (check_B_press(keypress, finalTime)==1)
            break;
        
        LATA = 0xFF;     // Initialize/pause motors
        
        __delay_ms(1200);

        // Change agitator directions
        agitator_move(1,0);  
        
        // Periodically check for stop conditions
        if (check_B_press(keypress, finalTime)==1)
            break;

        // Return gate
        return_motors(currentMotor);

        LATA = 0xFF;     // Initialize/pause motors
        
        // Periodically check for stop conditions
        if (check_B_press(keypress, finalTime)==1)
            break;
        
        // Check for maximum of 10 bottles
        if (binsCount[0]+binsCount[1]+binsCount[2]+binsCount[3] >= 10)  
            break;
    }
    
    // Power off all sensors
    tca_select(1);
    poweroff_sensor();
    tca_select(2);
    poweroff_sensor();
    tca_select(3);
    poweroff_sensor();
    
    // Stop agitator
    agitator_move(0,0);

    // </editor-fold>
    
    // <editor-fold defaultstate="collapsed" desc=" Post-Operation Display ">
    __lcd_clear();
    printf("Push 2,3,5,6,8,9");
    __lcd_line2();
    printf("to view log");
    
    // Calculate total number of bottles
    unsigned short totalCount = binsCount[0]+binsCount[1]+binsCount[2]+binsCount[3];
    
    while(1){
        unsigned char keypress = (PORTB & 0xF0)>>4;
        
        if (PORTBbits.RB1 == 1){
            while(PORTBbits.RB1){}
            __lcd_clear();
            __lcd_line1();
                    
            switch (keypress){
                // '2' key is pressed
                case 0b1:
                    printf("YOP with cap");
                    __lcd_line2();
                    printf("(bin #1): %02x", binsCount[0]);
                    break;
                // '3' key is pressed
                case 0b10:
                    printf("YOP without cap");
                    __lcd_line2();
                    printf("(bin #2): %d", binsCount[1]);
                    break;
                // '5' key is pressed
                case 0b101:
                    printf("ESKA with cap");
                    __lcd_line2();
                    printf("(bin #3): %d", binsCount[2]);
                    break;
                // '6' key is pressed
                case 0b110:
                    printf("ESKA without cap");
                    __lcd_line2();
                    printf("(bin #4): %d", binsCount[3]);
                    break;
                // '8' key is pressed
                case 0b1001:
                    printf("Total number of  ");
                    __lcd_line2();
                    printf("bottles: %d", totalCount);
                    break;
                // '9' key is pressed
                case 0b1010:
                    printf("Total time taken: ");
                    __lcd_line2();
                    printf("%02x min. %02x sec.", finalTime[1], finalTime[0]);
                    break;
            }
        }
    }
    
    //</editor-fold>
    
}