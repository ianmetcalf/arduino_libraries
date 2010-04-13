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

#define DS2482_EEPROM_ADDRESS	((E2END + 1) >> 1)

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
	int16_t temp;
	uint8_t alarmHigh;
	uint8_t alarmLow;
	uint8_t config;
} SCRATCH;

class DS2482
{
	public:
		DS2482(uint8_t addr);
		
		uint8_t setChannel(uint8_t);
		
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
		
		uint8_t tempSearch(Device&, Scratch&);
		
		uint8_t tempConversion(Device&);
		uint8_t tempConversionAll(uint8_t, uint8_t);
		
		uint8_t tempWriteScratchpad(Device&, Scratch&);
		uint8_t tempReadScratchpad(Device&, Scratch&);
		
		uint8_t tempStoreEE(Device&);
		uint8_t tempLoadEE(Device&);
		
		uint8_t tempPowerMode(Device&);
		uint8_t tempPowerModeAll(void);
		
		void eepromLoadCount(void);
		void eepromStoreCount(void);
		void eepromResetCount(uint8_t);
		
		uint8_t eepromCount(void);
		
		uint8_t eepromLoadSensor(DEVICE&, uint8_t);
		uint8_t eepromStoreSensor(DEVICE&);
		
		int16_t tempFahrenheit(Scratch&);
		
		uint8_t test(void);
		
		void init(void);
		
	private:
		uint8_t _address;
		uint8_t _config;
		uint8_t _channel;
		uint8_t _eeprom;
		
		uint8_t search_rom[8];
		uint8_t searchLast;
		uint8_t searchDone;
		
		uint8_t reset(void);
		uint8_t getRegister(uint8_t);
		uint8_t busy(uint8_t);
		uint8_t writeConfig(uint8_t);
};

#endif