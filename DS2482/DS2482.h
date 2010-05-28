/*
	Library for the DS2484 OneWire controller by Ian T Metcalf
		tested with the Arduino IDE v18 on:
		- Arduino Duemilanova with an atmega328p
		- Sanguino v1.0 with an atmega644p
	
	Configured for the DS2482-800 onewire bridge w/ 8 channels
		http://www.maxim-ic.com/quick_view2.cfm/qv_pk/4338
	
	Based on the library written by Paeae Technologies
		http://github.com/paeaetech/paeae
	
	Original description by Paeae Technologies:
		DS2482 library for Arduino
		Copyright (C) 2009 Paeae Technologies
		
		This program is free software: you can redistribute it and/or modify
		it under the terms of the GNU General Public License as published by
		the Free Software Foundation, either version 3 of the License, or
		(at your option) any later version.
		
		This program is distributed in the hope that it will be useful,
		but WITHOUT ANY WARRANTY; without even the implied warranty of
		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
		GNU General Public License for more details.
		
		You should have received a copy of the GNU General Public License
		along with this program.  If not, see <http://www.gnu.org/licenses/>.
	
	Also to give credit to the original OneWire library written by Jim Studt
		based on work by Derek Yerger and updated by Robin James and Paul Stoffregen
		http://www.pjrc.com/teensy/td_libs_OneWire.html
	
	And the temperature sensor library written by Miles Burton
		http://milesburton.com/index.php?title=Dallas_Temperature_Control_Library
	
	Changes by ITM:
		2010/04/30	restructured code to ease understanding for myself
		2010/04/30	moved ds2482 commands to a separate header file
		2010/04/30	used Peter Fleury's i2c master library instead of the one in Wire 
						to greatly simplify the communication to the device (no ISR)
		2010/04/30	wrote clean simple onewire search function
		2010/04/30	used crc routine in avr-libc to verify search and sensor scratchpad
		2010/04/30	wrote functions for the DS18B20 temperature sensor
		2010/04/30	wrote sensor management functions to find and store temp sensors in eeprom
		2010/05/24	rewrote error handleing, use flags instead of return values
		2010/05/25	seperated DS18B20 library from DS2482 library
	
	All works by ITM are released under the creative commons attribution share alike license
		http://creativecommons.org/licenses/by-sa/3.0/
	
	I can be contacted at metcalfbuilt@gmail.com
*/


#ifndef DS2482_h
#define DS2482_h


//*************************************************************************************************
//	Libraries
//*************************************************************************************************

extern "C"
{
	#include <inttypes.h>
	#include <util/delay.h>
	#include <util/crc16.h>
	#include "utility/i2cmaster.h"
}

#include "DS2482_Commands.h"


//*************************************************************************************************
//	Global Definitions
//*************************************************************************************************

#define DS2482_800


#define DS2482_I2C_ADDRESS 			0x18

#ifdef DS2482_800
#define DS2482_TOTAL_CHANNELS		8
#else
#define DS2482_TOTAL_CHANNELS		1
#endif

// error bits
#ifndef ERROR_FLAGS

#define ERROR_FLAGS
#define ERROR_TIMEOUT				0
#define ERROR_CONFIG				1
#define ERROR_CHANNEL				2
#define ERROR_SEARCH				3

#define ERROR_NO_DEVICE				4
#define ERROR_SHORT_FOUND			5
#define ERROR_CRC_MISMATCH			6
#define ERROR_EEPROM_FULL			7

#endif







//*************************************************************************************************
//	Class Definition
//*************************************************************************************************

class DS2482
{
	public:
		DS2482();
		
		uint8_t error_flags;
		uint8_t searchDone;
		
		void setConfig(uint8_t);
		
		#ifdef DS2482_800
		uint8_t setChannel(uint8_t);
		#endif
		
		void wireReset(void);
		void wireWrite(uint8_t);
		uint8_t wireRead(void);
		
		void wireWriteBit(uint8_t);
		uint8_t wireReadBit(void);
		void wireTriplet(uint8_t);
		
		void romRead(uint8_t*);
		void romMatch(uint8_t*);
		void romSkip(void);
		void romSearch(uint8_t*, uint8_t);
		
		void init(uint8_t);
		
	private:
		uint8_t _address;
		uint8_t _status;
		
		#ifdef DS2482_800
		uint8_t _channel;
		#endif
		
		uint8_t search_rom[8];
		uint8_t searchLast;
		
		void _reset(void);
		uint8_t _getRegister(uint8_t);
		void _busy(uint8_t);
		
};

extern DS2482 ds2482;

#endif