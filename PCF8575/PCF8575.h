/*
	Library for pcf8575 port extender.
	Created by Ian Metcalf, March 1, 2010.
	Released into the public domain.
*/

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