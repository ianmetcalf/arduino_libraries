/*
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
*/

#ifndef DS2482_h
#define DS2482_h

#include <inttypes.h>

#define DS2482_TOTAL_CHANNELS 8

#define DS2482_EEPROM_MAX_ALLOC	(E2END >> 1)

#define CONFIG_RES_SHIFT	>>5
#define CONFIG_RES_9_BIT	0x1F
#define CONFIG_RES_10_BIT	0x3F
#define CONFIG_RES_11_BIT	0x5F
#define CONFIG_RES_12_BIT	0x7F

#define ERROR_NONE			0
#define ERROR_TIMEOUT		1
#define ERROR_NO_DEVICE		2
#define ERROR_SHORT_FOUND	3
#define ERROR_EEPROM_FULL	4
#define ERROR_CRC_MISMATCH	5

#define TEMP_C	0
#define TEMP_F	1

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




class DS2482
{
	public:
		DS2482(uint8_t addr);
		
		uint8_t getError(void);
		
		
		uint8_t wireReset(void);
		
		uint8_t wireWrite(uint8_t);
		uint8_t wireRead(void);
		
		uint8_t wireWriteBit(uint8_t);
		uint8_t wireReadBit(void);
		uint8_t wireTriplet(uint8_t);
		
		
		uint8_t romRead(uint8_t*);
		uint8_t romMatch(uint8_t*);
		uint8_t romSkip(void);
		
		uint8_t romSearch(uint8_t*, uint8_t);
		uint8_t romSearch(uint8_t*);
		
		
		uint8_t tempPowerMode(Device&);
		uint8_t tempPowerModeAll(void);
		
		uint8_t tempStoreEE(Device&);
		uint8_t tempLoadEE(Device&);
		
		uint8_t tempConversion(Device&);
		uint8_t tempConversionAll(uint8_t, uint8_t);
		
		uint8_t tempWriteScratchpad(Device&, Scratch&);
		uint8_t tempReadScratchpad(Device&, Scratch&);
		
		uint8_t tempSearch(Device&, Scratch&);
		
		void tempSensorReset(void);
		uint8_t tempSensorTotal(void);
		
		uint8_t tempSensorLoad(uint8_t, Device&);
		uint8_t tempSensorStore(uint8_t, Device&);
		
		uint8_t tempSensorVarify(uint8_t, Device&);
		uint8_t tempSensorFind(Device&, Scratch&);
		
		
		void init(void);
		
	private:
		uint8_t _address;
		
		uint8_t _channel;
		uint8_t _config;
		uint8_t _status;
		
		uint8_t _error;
		
		uint8_t search_rom[8];
		uint8_t searchLast;
		uint8_t searchDone;
		
		uint8_t eepromTotal;
		
		void chipReset(void);
		void chipBusy(uint8_t);
		
		uint8_t chipRegister(uint8_t);
		uint8_t chipConfigure(uint8_t);
		uint8_t chipChannel(uint8_t);
};

#endif