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


extern "C"{
	#include <inttypes.h>
	#include <i2cmaster.h>
}

#include "PCF8575.h"

#define PCF8575_I2C_ADDRESS 		0x20
#define PCF8575_I2C_ADDRESS_MASK	0x03

//-------------------------------------------------------------------------------------------------
//
// Constructor
//
//	Input	address: the address of the i2c device
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

PCF8575::PCF8575(uint8_t address)
{
	address &= PCF8575_I2C_ADDRESS_MASK;
	
	_address = (PCF8575_I2C_ADDRESS | address) << 1;
}

//-------------------------------------------------------------------------------------------------
//
// Read from the port into the buffer
//
//	Input	none
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void PCF8575::read(void)
{
	if (mode != 0xFF)
	{
		uint16_t tmp;
		
		i2c_start_wait(_address | I2C_READ);
		tmp = i2c_readAck();
		tmp |= (uint16_t)i2c_readNak() << 8;
		i2c_stop();
		
		buffer = (mode & buffer) | (~mode & tmp);
	}
}

//-------------------------------------------------------------------------------------------------
//
// Write the buffer to the port
//
//	Input	none
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void PCF8575::write(void)
{
	uint16_t tmp = (buffer | ~mode);
	
	i2c_start_wait(_address | I2C_WRITE);
	i2c_write((uint8_t)(tmp & 0xFF));
	i2c_write((uint8_t)(tmp >> 8));
	i2c_stop();
}

//-------------------------------------------------------------------------------------------------
//
// Set/Clear/Toggle port pins
//
//	Input	pins: the pins to change
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void PCF8575::set(uint16_t pins)
{
	buffer |= pins;
	write();
}

void PCF8575::clear(uint16_t pins)
{
	buffer &= ~pins;
	write();
}

void PCF8575::toggle(uint16_t pins)
{
	buffer ^= pins;
	write();
}

//-------------------------------------------------------------------------------------------------
//
// Set/Clear/Toggle a port pin
//
//	Input	pin: the pin to change
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void PCF8575::setPin(uint8_t pin)
{
	set((uint16_t)1 << pin);
}

void PCF8575::clearPin(uint8_t pin)
{
	clear((uint16_t)1 << pin);
}

void PCF8575::togglePin(uint8_t pin)
{
	toggle((uint16_t)1 << pin);
}

//-------------------------------------------------------------------------------------------------
//
// Read a pin value
//
//	Input	pin: the pin to read
//
//	Output	the value read
//
//-------------------------------------------------------------------------------------------------

uint8_t PCF8575::readPin(uint8_t pin)
{
	mode &= ~(1 << pin);
	read();
	return ((uint8_t)(buffer >> pin) & 0x01);
}

//-------------------------------------------------------------------------------------------------
//
// Initalization
//
//	Input	none
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void PCF8575::init(void)
{
	i2c_init();
	mode = 0xFFFF;
	buffer = 0;
	write();
}
