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

class DS2482
{
	public:
		DS2482(uint8_t addr);
		
		uint8_t setChannel(uint8_t);
		
		uint8_t oneWireReset(void);
		
		uint8_t oneWireWrite(uint8_t);
		uint8_t oneWireRead(void);
		
		uint8_t oneWireWriteBit(uint8_t);
		uint8_t oneWireReadBit(void);
		uint8_t oneWireTriplet(uint8_t);
		
		uint8_t romRead(uint8_t*);
		uint8_t romMatch(uint8_t*);
		uint8_t romSkip(void);
		uint8_t romSearch(uint8_t*, uint8_t);
		uint8_t romSearch(uint8_t*);
		
		uint8_t tempSearch(uint8_t*);
		
		uint8_t tempConversion(uint8_t*, uint8_t, uint8_t);
		uint8_t tempConversionAll(uint8_t, uint8_t);
		
		uint8_t tempWriteScratchpad(uint8_t*, uint8_t*);
		uint8_t tempReadScratchpad(uint8_t*, uint8_t*);
		
		uint8_t tempWriteEEprom(uint8_t*);
		uint8_t tempReadEEprom(uint8_t*);
		
		uint8_t tempPowerMode(uint8_t*);
		uint8_t tempPowerModeAll(void);
		
		
		int16_t tempCelsius(uint8_t*);
		int16_t tempFahrenheit(uint8_t*);
		
		uint8_t test(void);
		
		void init(void);
		
	private:
		uint8_t _address;
		uint8_t _config;
		uint8_t _channel;
		
		uint8_t search_rom[8];
		uint8_t lastBranch;
		uint8_t searchDone;
		
		void reset(void);
		uint8_t read(void);
		void setReadPtr(uint8_t);
		
		uint8_t getStatus(void);
		uint8_t getData(void);
		uint8_t getChannel(void);
		uint8_t getConfig(void);
		
		uint8_t busy(uint8_t);
		uint8_t writeConfig(uint8_t);
};

#endif