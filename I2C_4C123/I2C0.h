// I2C0.h
// Runs on LM4F120/TM4C123
// Provide a function that initializes, sends, and receives the I2C0 module
// interfaced with an HMC6352 compass or TMP102 thermometer.
// Daniel Valvano
// May 3, 2015

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
   Section 8.6.4 Programs 8.5, 8.6 and 8.7

 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

// I2C0SCL connected to PB2 and to pin 4 of HMC6352 compass or pin 3 of TMP102 thermometer
// I2C0SDA connected to PB3 and to pin 3 of HMC6352 compass or pin 2 of TMP102 thermometer
// SCL and SDA lines pulled to +3.3 V with 10 k resistors (part of breakout module)
// ADD0 pin of TMP102 thermometer connected to GND
void I2C_Init(void);

// receives one byte from specified slave
// Note for HMC6352 compass only:
// Used with 'r' and 'g' commands
// Note for TMP102 thermometer only:
// Used to read the top byte of the contents of the pointer register
//  This will work but is probably not what you want to do.
uint8_t I2C_Recv(int8_t slave);

// receives two bytes from specified slave
// Note for HMC6352 compass only:
// Used with 'A' commands
// Note for TMP102 thermometer only:
// Used to read the contents of the pointer register
uint16_t I2C_Recv2(int8_t slave);

// sends one byte to specified slave
// Note for HMC6352 compass only:
// Used with 'S', 'W', 'O', 'C', 'E', 'L', and 'A' commands
//  For 'A' commands, I2C_Recv2() should also be called
// Note for TMP102 thermometer only:
// Used to change the pointer register
// Returns 0 if successful, nonzero if error
uint32_t I2C_Send1(int8_t slave, uint8_t data1);

// sends two bytes to specified slave
// Note for HMC6352 compass only:
// Used with 'r' and 'g' commands
//  For 'r' and 'g' commands, I2C_Recv() should also be called
// Note for TMP102 thermometer only:
// Used to change the top byte of the contents of the pointer register
//  This will work but is probably not what you want to do.
// Returns 0 if successful, nonzero if error
uint32_t I2C_Send2(int8_t slave, uint8_t data1, uint8_t data2);

// sends three bytes to specified slave
// Note for HMC6352 compass only:
// Used with 'w' and 'G' commands
// Note for TMP102 thermometer only:
// Used to change the contents of the pointer register
// Returns 0 if successful, nonzero if error
uint32_t I2C_Send3(int8_t slave, uint8_t data1, uint8_t data2, uint8_t data3);
