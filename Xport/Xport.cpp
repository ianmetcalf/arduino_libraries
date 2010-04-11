/*
	Xport - Library for pcf8575 port extender.
	Created by Ian Metcalf, March 1, 2010.
	Released into the public domain.
*/

#include <inttypes.h>

extern "C" {
	#include "twi.h"
}

#include "Xport.h"

//-------------------------------------------------------------------------------------------------
//
// Constructor
//	
//-------------------------------------------------------------------------------------------------

Xport::Xport(uint8_t address)
{
	_address = address;
}

//-------------------------------------------------------------------------------------------------
//
// Read from the port into the buffer
//
//-------------------------------------------------------------------------------------------------

void Xport::read(void)
{
	uint8_t buf_data[2];
	uint16_t tmp;
	
	if (~mode)
	{
		twi_readFrom(_address, &buf_data[0], 2);
		
		tmp = (uint16_t)buf_data[0];
		tmp |= (uint16_t)buf_data[1] << 8;
		
		buffer = (mode & buffer) | (~mode & tmp);
	}
}

//-------------------------------------------------------------------------------------------------
//
// Write the buffer to the port
//
//-------------------------------------------------------------------------------------------------

void Xport::write(void)
{
	uint8_t buf_data[2];
	
	buf_data[0] = (uint8_t)((buffer | ~mode) & 0xFF);
	buf_data[1] = (uint8_t)((buffer | ~mode) >> 8);
	
	twi_writeTo(_address, &buf_data[0], 2, 1);
}

//-------------------------------------------------------------------------------------------------
//
// Set/Clear/Toggle port pins
//
//-------------------------------------------------------------------------------------------------

void Xport::set(uint16_t pins)
{
	buffer |= pins;
	write();
}

void Xport::clear(uint16_t pins)
{
	buffer &= ~pins;
	write();
}

void Xport::toggle(uint16_t pins)
{
	buffer ^= pins;
	write();
}

//-------------------------------------------------------------------------------------------------
//
// Set/Clear/Toggle a port pin
//
//-------------------------------------------------------------------------------------------------

void Xport::setPin(uint8_t pin)
{
	set((uint16_t)(1<<pin));
}

void Xport::clearPin(uint8_t pin)
{
	clear((uint16_t)(1<<pin));
}

void Xport::togglePin(uint8_t pin)
{
	toggle((uint16_t)(1<<pin));
}

//-------------------------------------------------------------------------------------------------
//
// Read a pin value
//
//-------------------------------------------------------------------------------------------------

uint8_t Xport::readPin(uint8_t pin)
{
	mode &= ~(1<<pin);
	read();
	return (uint8_t)((buffer>>pin) & 0x01);
}

//-------------------------------------------------------------------------------------------------
//
// Xport initalization
//
//-------------------------------------------------------------------------------------------------

void Xport::init(void)
{
	twi_init();
	mode = 0xFFFF;
	buffer = 0;
	write();
}
