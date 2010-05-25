/*
	Library for the DS18B20 OneWire temperature sensor by Ian T Metcalf
		tested with the Arduino IDE v18 on:
		- Arduino Duemilanova with an atmega328p
		- Sanguino v1.0 with an atmega644p
	
	Configured for the DS18B20 onewire temperature sensor
		http://www.maxim-ic.com/quick_view2.cfm?qv_pk=2812
	
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
		2010/05/24	seperated DS18B20 library from DS2482 library
	
	All works by ITM are released under the creative commons attribution share alike license
		http://creativecommons.org/licenses/by-sa/3.0/
	
	I can be contacted at metcalfbuilt@gmail.com
*/


#ifndef DS18B20_h
#define DS18B20_h


//*************************************************************************************************
//	Libraries
//*************************************************************************************************

extern "C"
{
	#include <inttypes.h>
	#include <avr/eeprom.h>
	#include <util/delay.h>
	#include <util/crc16.h>
}

#include <DS2482.h>
#include "DS18B20_Commands.h"


//*************************************************************************************************
//	Global Definitions
//*************************************************************************************************

#define DS18B20_EEPROM_MAX_ALLOC	(E2END >> 1)

#define TEMP_C	0
#define TEMP_F	1

#define CONFIG_RES_SHIFT		>>5
#define CONFIG_RES_9_BIT		0x1F
#define CONFIG_RES_10_BIT		0x3F
#define CONFIG_RES_11_BIT		0x5F
#define CONFIG_RES_12_BIT		0x7F


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
//	Global Types
//*************************************************************************************************

typedef struct Device
{
	uint8_t addr[8];
	struct {
		uint8_t powered		:1;
		uint8_t channel		:3;
		uint8_t resolution	:2;
	} config;
} DEVICE;

typedef struct Scratch
{
	int16_t temp[2];
	uint8_t alarmHigh;
	uint8_t alarmLow;
	uint8_t config;
} SCRATCH;




//*************************************************************************************************
//	Class Definition
//*************************************************************************************************

class DS18B20
{
	public:
		DS18B20();
		
		void startConversion(uint8_t, uint8_t, uint8_t);
		void startConversion(Device&, uint8_t);
		
		void writeScratchpad(Device&, Scratch&);
		void readScratchpad(Device&, Scratch&);
		
		void resetSensors(void);
		uint8_t totalSensors(void);
		
		void loadSensor(uint8_t, Device&);
		void storeSensor(uint8_t, Device&);
		
		uint8_t varifySensor(uint8_t, Device&);
		uint8_t findSensor(Device&, Scratch&);
		
		void init(void);
		
	private:
		uint8_t eepromTotal;
		
		uint8_t powerMode(void);
		uint8_t powerMode(Device&);
		
		void storeSensorEE(Device&);
		void loadSensorEE(Device&);
		
};

extern DS18B20 dsTemp;

#endif