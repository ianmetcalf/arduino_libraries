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

	crc code is from OneWire library
*/

#include <WProgram.h>
#include "DS2482.h"
#include "DS2482_Commands.h"

#include <inttypes.h>
#include <util/crc16.h>

extern "C" {
  #include "twi.h"
}
/*
//-------------------------------------------------------------------------------------------------
//
// Crc function (replaced by util function)
//	
//-------------------------------------------------------------------------------------------------

void computeCrc(uint8_t &crc, uint8_t value)
{
	uint8_t i;
	
	for (i = 0; i < 8; i++)
	{
		uint8_t mix = (crc ^ value) & 0x01;
		
		crc >>= 1;
		if (mix)
		{
			crc ^= 0x8C;
		}
		value >>= 1;
	}
}
*/
//-------------------------------------------------------------------------------------------------
//
// Constructor
//	
//-------------------------------------------------------------------------------------------------

DS2482::DS2482(uint8_t address)
{
	_address = address;
}

//-------------------------------------------------------------------------------------------------
//
// Reset Device
//	
//-------------------------------------------------------------------------------------------------

void DS2482::reset(void)
{
	uint8_t buf_data = DS2482_DEVICE_RESET;
	
	twi_writeTo(_address, &buf_data, 1, 1);
	
	_channel = 0;
}

//-------------------------------------------------------------------------------------------------
//
// Read byte at pointer
//	
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::read(void)
{
	uint8_t buf_data;
	
	twi_readFrom(_address, &buf_data, 1);
	
	return buf_data;
}

//-------------------------------------------------------------------------------------------------
//
// Set read pointer
//	
//-------------------------------------------------------------------------------------------------

void DS2482::setReadPtr(uint8_t readPtr)
{
	uint8_t buf_data[2];
	
	buf_data[0] = DS2482_SET_POINTER;
	buf_data[1] = readPtr;
	
	twi_writeTo(_address, &buf_data[0], 2, 1);
}

//-------------------------------------------------------------------------------------------------
//
// Return byte at register
//	
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::getStatus(void)
{
	setReadPtr(DS2482_STATUS_REG);
	return read();
}

uint8_t DS2482::getData(void)
{
	setReadPtr(DS2482_DATA_REG);
	return read();
}

uint8_t DS2482::getChannel(void)
{
	setReadPtr(DS2482_CHANNEL_REG);
	return read();
}

uint8_t DS2482::getConfig(void)
{
	setReadPtr(DS2482_CONFIG_REG);
	return read();
}

//-------------------------------------------------------------------------------------------------
//
// Wait until not busy or timeout
//	
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::busy(uint8_t setPtr)
{
	uint8_t status = (setPtr) ? getStatus() : read();
	uint16_t count = 1000;
	
	while ((status & DS2482_STATUS_BUSY) && count)
	{
		delayMicroseconds(20);
		status = read();
		count--;
	}
	
	return status;
}

//-------------------------------------------------------------------------------------------------
//
// Write configuration register
//	
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::writeConfig(uint8_t config)
{
	if (!(busy(1) & DS2482_STATUS_BUSY))
	{
		uint8_t buf_data[2];
		
		buf_data[0] = DS2482_WRITE_CONFIG;
		buf_data[1] = config & ((~config) << 4);
		
		twi_writeTo(_address, &buf_data[0], 2, 1);
		
		if (read() == config)
		{
			_config = config;
			return 1;
		}
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Set channel
//	
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::setChannel(uint8_t channel)
{
	if (!(busy(1) & DS2482_STATUS_BUSY))
	{
		uint8_t buf_data[2];
		uint8_t check;
		
		buf_data[0] = DS2482_SELECT_CHANNEL;
		
		switch(channel)
		{
			default:
			case 0:
				buf_data[1] = DS2482_WRITE_CHANNEL_0;
				check = DS2482_READ_CHANNEL_0;
				break;
			case 1:
				buf_data[1] = DS2482_WRITE_CHANNEL_1;
				check = DS2482_READ_CHANNEL_1;
				break;
			case 2:
				buf_data[1] = DS2482_WRITE_CHANNEL_2;
				check = DS2482_READ_CHANNEL_2;
				break;
			case 3:
				buf_data[1] = DS2482_WRITE_CHANNEL_3;
				check = DS2482_READ_CHANNEL_3;
				break;
			case 4:
				buf_data[1] = DS2482_WRITE_CHANNEL_4;
				check = DS2482_READ_CHANNEL_4;
				break;
			case 5:
				buf_data[1] = DS2482_WRITE_CHANNEL_5;
				check = DS2482_READ_CHANNEL_5;
				break;
			case 6:
				buf_data[1] = DS2482_WRITE_CHANNEL_6;
				check = DS2482_READ_CHANNEL_6;
				break;
			case 7:
				buf_data[1] = DS2482_WRITE_CHANNEL_7;
				check = DS2482_READ_CHANNEL_7;
				break;
		}
		
		twi_writeTo(_address, &buf_data[0], 2, 1);
		
		if (read() == check)
		{
			_channel = channel;
			return 1;
		}
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Reset OneWire
//	
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::oneWireReset(void)
{
	if (!(busy(1) & DS2482_STATUS_BUSY))
	{
		uint8_t buf_data = DS2482_ONE_WIRE_RESET;
		
		twi_writeTo(_address, &buf_data, 1, 1);
		
		return 1;
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Write byte to OneWire
//	
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::oneWireWrite(uint8_t data)
{
	if (!(busy(1) & DS2482_STATUS_BUSY))
	{
		uint8_t buf_data[2];
		
		buf_data[0] = DS2482_ONE_WIRE_WRITE_BYTE;
		buf_data[1] = data;
		
		twi_writeTo(_address, &buf_data[0], 2, 1);
		
		return 1;
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Read byte from OneWire
//	
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::oneWireRead(void)
{
	if (!(busy(1) & DS2482_STATUS_BUSY))
	{
		uint8_t buf_data = DS2482_ONE_WIRE_READ_BYTE;
		
		twi_writeTo(_address, &buf_data, 1, 1);
		
		if (!(busy(0) & DS2482_STATUS_BUSY))
		{
			return getData();
		}
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Write bit to OneWire
//	
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::oneWireWriteBit(uint8_t bit)
{
	if (!(busy(1) & DS2482_STATUS_BUSY))
	{
		uint8_t buf_data[2];
		
		buf_data[0] = DS2482_ONE_WIRE_SINGLE_BIT;
		buf_data[1] = (bit) ? 0x80 : 0;
		
		twi_writeTo(_address, &buf_data[0], 2, 1);
		
		return 1;
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Read bit from OneWire
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::oneWireReadBit(void)
{
	if (oneWireWriteBit(1))
	{
		return (busy(0) & DS2482_STATUS_SBR) ? 1 : 0;
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Read 2, write 1 from OneWire
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::oneWireTriplet(uint8_t dir)
{
	if (!(busy(0) & DS2482_STATUS_BUSY))
	{
		uint8_t buf_data[2];
		
		buf_data[0] = DS2482_ONE_WIRE_TRIPLET;
		buf_data[1] = (dir) ? 0x80 : 0;
		
		twi_writeTo(_address, &buf_data[0], 2, 1);
		
		return 1;
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Get rom address from device
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::romRead(uint8_t *address)
{
	uint8_t status;
	
	oneWireReset();
	status = busy(0);
	
	if (!(status & DS2482_STATUS_BUSY) && (status & DS2482_STATUS_PPD) && oneWireWrite(ONE_WIRE_READ_ROM))
	{
		uint8_t crc, i;
		
		for (i = 0; i < 8; i++)
		{
			address[i] = oneWireRead();
			crc = _crc_ibutton_update(crc, address[i]);
		}
		
		if ((crc == 0) && (address[0] != 0))
		{
			return 1;
		}
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Get device with rom address
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::romMatch(uint8_t *address)
{
	uint8_t status;
	
	oneWireReset();
	status = busy(0);
	
	if (!(status & DS2482_STATUS_BUSY) && (status & DS2482_STATUS_PPD) && oneWireWrite(ONE_WIRE_MATCH_ROM))
	{
		uint8_t i;
		
		for (i = 0; i < 8; i++)
		{
			oneWireWrite(address[i]);
		}
		
		return 1;
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Skip rom address
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::romSkip(void)
{
	uint8_t status;
	
	oneWireReset();
	status = busy(0);
	
	if (!(status & DS2482_STATUS_BUSY) && (status & DS2482_STATUS_PPD) && oneWireWrite(ONE_WIRE_SKIP_ROM))
	{
		return 1;
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Search OneWire for devices
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::romSearch(uint8_t *address, uint8_t family)
{
	uint8_t status;
	
	if (searchDone)
	{
		uint8_t i;
		
		for (i = 0; i < 8; i++)
		{
			search_rom[i] = 0x00;
		}
		
		if (family != 0)
		{
			search_rom[0] = family;
			lastBranch = 64;
		}
		else
		{
			lastBranch = 0;
		}
		
		searchDone = 0;
	}
	
	oneWireReset();
	status = busy(0);
	
	if (!(status & DS2482_STATUS_BUSY) && (status & DS2482_STATUS_PPD) && oneWireWrite(ONE_WIRE_SEARCH_ROM))
	{
		uint8_t lastZero, i;
		
		uint8_t count = 0;
		uint8_t crc = 0;
		
		for (i = 0; i < 8; i++)
		{
			uint8_t romMask;
			
			for (romMask = 1; romMask; romMask <<= 1)
			{
				uint8_t sbr, tsb, dir;
				
				dir = (count < lastBranch) ? (search_rom[i] & romMask) : ((count == lastBranch) ? 1 : 0);
				
				oneWireTriplet(dir);
				status = busy(0);
				
				sbr = (status & DS2482_STATUS_SBR);
				tsb = (status & DS2482_STATUS_TSB);
				dir = (status & DS2482_STATUS_DIR);
				
				if (sbr && tsb)
				{
					searchDone = 1;
					return 0;
				}
				else if (!sbr && !tsb && !dir)
				{
					lastZero = count;
				}
				
				if (dir)
				{
					search_rom[i] |= romMask;
				}
				else
				{
					search_rom[i] &= ~romMask;
				}
				
				count++;
			}
			
			crc = _crc_ibutton_update(crc, search_rom[i]);
		}
		
		if ((crc == 0) && (search_rom[0] != 0) && !((family != 0) && (search_rom[0] != family)))
		{
			for (i = 0; i < 8; i++)
			{
				address[i] = search_rom[i];
			}
			
			lastBranch = lastZero;
			
			if (lastBranch == 0)
			{
				searchDone = 1;
			}
			
			return 1;
		}
	}
	
	searchDone = 1;
	return 0;
}

uint8_t DS2482::romSearch(uint8_t *address)
{
	return romSearch(address, 0);
}

//-------------------------------------------------------------------------------------------------
//
// Search OneWire for temperature devices
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::tempSearch(uint8_t *address)
{
	return romSearch(address, DS18B20_FAMILY_CODE);
}

//-------------------------------------------------------------------------------------------------
//
// Initiate temperature conversion
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::tempConversion(uint8_t *address, uint8_t powered, uint8_t resolution)
{
	if (address[0] == DS18B20_FAMILY_CODE && romMatch(address) && oneWireWrite(DS18B20_CONVERT_TEMP))
	{
		if (powered)
		{
			while(!oneWireReadBit())
			{
				delayMicroseconds(20);
			}
		}
		else
		{
			writeConfig(_config | DS2482_CONFIG_SPU);
			delay(94 << resolution);
			writeConfig(_config & ~DS2482_CONFIG_SPU);
		}
		
		return 1;
	}
	
	return 0;
}

uint8_t DS2482::tempConversionAll(uint8_t powered, uint8_t resolution)
{
	if (romSkip() && oneWireWrite(DS18B20_CONVERT_TEMP))
	{
		if (powered)
		{
			while(!oneWireReadBit())
			{
				delayMicroseconds(20);
			}
		}
		else
		{
			writeConfig(_config | DS2482_CONFIG_SPU);
			delay(94 << resolution);
			writeConfig(_config & ~DS2482_CONFIG_SPU);
		}
		
		return 1;
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Write scratchpad to temperature device
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::tempWriteScratchpad(uint8_t *address, uint8_t *scratchpad)
{
	if (address[0] == DS18B20_FAMILY_CODE && romMatch(address) && oneWireWrite(DS18B20_WRITE_SCRATCHPAD))
	{
		oneWireWrite(scratchpad[DS18B20_SCRATCHPAD_HIGH_ALARM]);
		oneWireWrite(scratchpad[DS18B20_SCRATCHPAD_LOW_ALARM]);
		oneWireWrite(scratchpad[DS18B20_SCRATCHPAD_CONFIG_REG]);
		
		return 1;
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Read temperature device scratchpad
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::tempReadScratchpad(uint8_t *address, uint8_t *scratchpad)
{
	if (address[0] == DS18B20_FAMILY_CODE && romMatch(address) && oneWireWrite(DS18B20_READ_SCRATCHPAD))
	{
		uint8_t i;
		
		uint8_t crc = 0;
		
		for (i = 0; i < 8; i++)
		{
			scratchpad[i] = oneWireRead();
			crc = _crc_ibutton_update(crc, scratchpad[i]);
		}
		crc = _crc_ibutton_update(crc, oneWireRead());
		
		if (crc == 0)
		{
			return 1;
		}
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Write scratchpad to EEPROM
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::tempWriteEEprom(uint8_t *address)
{
	if (address[0] == DS18B20_FAMILY_CODE && romMatch(address) && oneWireWrite(DS18B20_COPY_SCRATCHPAD))
	{
		return 1;
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Read EEPROM to scratchpad
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::tempReadEEprom(uint8_t *address)
{
	if (address[0] == DS18B20_FAMILY_CODE && romMatch(address) && oneWireWrite(DS18B20_RECALL_EEPROM))
	{
		return 1;
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Get the power mode of a device or all devices
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::tempPowerMode(uint8_t *address)
{
	if (address[0] == DS18B20_FAMILY_CODE && romMatch(address) && oneWireWrite(DS18B20_READ_POWER_MODE))
	{
		return oneWireReadBit();
	}
	
	return 0;
}

uint8_t DS2482::tempPowerModeAll(void)
{
	if (romSkip() && oneWireWrite(DS18B20_READ_POWER_MODE))
	{
		return oneWireReadBit();
	}
	
	return 0;
}

















int16_t DS2482::tempCelsius(uint8_t *address)
{
	uint8_t scratch[8];
	int16_t temp;
	
	if (tempReadScratchpad(address, scratch))
	{
		temp = ((int16_t)scratch[1] << 8) | scratch[0];
		
		return temp;
	}
}

int16_t DS2482::tempFahrenheit(uint8_t *address)
{
	int16_t temp;
	
	temp = (tempCelsius(address) * 9) / 5;
	temp += (32 << 4);
	
	return temp;
}

uint8_t DS2482::test(void)
{
	return (busy(1) & DS2482_STATUS_PPD) ? 1 : 0;
}

//-------------------------------------------------------------------------------------------------
//
// DS2482 initalization
//
//-------------------------------------------------------------------------------------------------

void DS2482::init(void)
{
	twi_init();
	
	reset();
	writeConfig(0);
}


