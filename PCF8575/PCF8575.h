/*
	Library for the PCF8575 port expander by Ian T Metcalf
		tested with the Arduino IDE v18 on a Duemilanova 328
	
	Configured for the texas instrument chip on a sparkfun breakout board
		http://focus.ti.com/docs/prod/folders/print/pcf8575.html
	
	Based on the sketch written by Keith Neufeld
		http://www.neufeld.newton.ks.us/electronics/?p=241
	
	Original description by Keith Neufeld
		i2c_gpio
		Keith Neufeld
		May 26, 2008
		
		Prototype I2C interface to TI 9535 and 9555 GPIO expanders.
		
		Arduino analog input 5 - I2C SCL
		Arduino analog input 4 - I2C SDA
	
	Changes by ITM:
		2010/04/30	restructured code to ease understanding for myself
		2010/04/30	turned sketch into a library
		2010/04/30	used Peter Fleury's i2c master library instead of the one in Wire 
						to greatly simplify the communication to the device (no ISR)
	
	All works by ITM are released under the creative commons attribution share alike license
		http://creativecommons.org/licenses/by-sa/3.0/
	
	I can be contacted at metcalfbuilt@gmail.com
*/


//******************************************************************************
//
// IMPORTANT:
//
//	You must have the avr-gcc library i2cmaster by Peter Fleury to compile code:
//		http://homepage.hispeed.ch/peterfleury/avr-software.html
//		or http://jump.to/fleury and follow the link for AVR Software
//	
//	I copied i2cmaster.h and twimaster.c to the arduino core directory:
//		arduino-0018\hardware\arduino\cores\arduino\
//	
//	That seemed to work for me, but if there is a better way let me know
//
//******************************************************************************


#ifndef PCF8575_H
#define PCF8575_H

#include <inttypes.h>

class PCF8575
{
	public:
		uint16_t mode;
		uint16_t buffer;
		
		PCF8575(uint8_t);
		
		void read(void);
		void write(void);
		
		void set(uint16_t);
		void clear(uint16_t);
		void toggle(uint16_t);
		
		void setPin(uint8_t);
		void clearPin(uint8_t);
		void togglePin(uint8_t);
		
		uint8_t readPin(uint8_t);
		
		void init(void);
		
	private:
		uint8_t _address;
};

#endif