/*
	Library for pcf8575 port extender.
	Created by Ian Metcalf, March 1, 2010.
	Released into the public domain.
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
