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

extern "C"{
	#include <inttypes.h>
	#include <i2cmaster.h>
	#include <util/crc16.h>
}

#include "DS2482.h"
#include "DS2482_Commands.h"

#define DS2482_I2C_ADDRESS 		0x18
#define DS2482_I2C_ADDRESS_MASK	0x03

//-------------------------------------------------------------------------------------------------
//
// Constructor
//
//	Input	address: the address of the i2c device
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

DS2482::DS2482(uint8_t address)
{
	address &= DS2482_I2C_ADDRESS_MASK;
	
	_address = (DS2482_I2C_ADDRESS | address) << 1;
}

//-------------------------------------------------------------------------------------------------
//
// Reset Device
//
//	Input	none
//
//	Output	status register
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::reset(void)
{
	uint8_t status;
	
	_channel = 0;
	
	i2c_start_wait(_address | I2C_WRITE);
	i2c_write(DS2482_DEVICE_RESET);
	
	i2c_rep_start(_address | I2C_READ);
	status = i2c_readNak();
	i2c_stop();
	
	return status;
}

//-------------------------------------------------------------------------------------------------
//
// Read device register
//
//	Input	reg: device register
//
//	Output	byte read from device
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::getRegister(uint8_t reg)
{
	uint8_t result;
	
	if (reg)
	{
		i2c_start_wait(_address | I2C_WRITE);
		i2c_write(DS2482_SET_POINTER);
		i2c_write(reg);
		
		i2c_rep_start(_address | I2C_READ);
	}
	else
	{
		i2c_start_wait(_address | I2C_READ);
	}
	
	result = i2c_readNak();
	i2c_stop();
	
	return result;
}

//-------------------------------------------------------------------------------------------------
//
// Wait until not busy or timeout
//
//	Input	set: set the register pointer T/F
//
//	Output	status register
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::busy(uint8_t set)
{
	uint8_t status = getRegister((set) ? DS2482_STATUS_REG : 0);
	uint16_t count = 1000;
	
	while ((status & DS2482_STATUS_BUSY) && count)
	{
		delayMicroseconds(20);
		status = getRegister(0);
		count--;
	}
	
	return status;
}

//-------------------------------------------------------------------------------------------------
//
// Write configuration register
//
//	Input	config: configuration byte
//
//	Output	0 fail
//			1 success
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::writeConfig(uint8_t config)
{
	if (!(busy(1) & DS2482_STATUS_BUSY))
	{
		uint8_t result;
		
		i2c_start_wait(_address | I2C_WRITE);
		i2c_write(DS2482_WRITE_CONFIG);
		i2c_write(config & ((~config) << 4));
		
		i2c_rep_start(_address | I2C_READ);
		result = i2c_readNak();
		i2c_stop();
		
		if (result == config)
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
//	Input	channel: one wire channel
//
//	Output	0 fail
//			1 success
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::setChannel(uint8_t channel)
{
	if (!(busy(1) & DS2482_STATUS_BUSY))
	{
		uint8_t check, result;
		
		i2c_start_wait(_address | I2C_WRITE);
		i2c_write(DS2482_SELECT_CHANNEL);
		
		switch(channel)
		{
			default:
			case 0:
				i2c_write(DS2482_WRITE_CHANNEL_0);
				check = DS2482_READ_CHANNEL_0;
				break;
				
			case 1:
				i2c_write(DS2482_WRITE_CHANNEL_1);
				check = DS2482_READ_CHANNEL_1;
				break;
				
			case 2:
				i2c_write(DS2482_WRITE_CHANNEL_2);
				check = DS2482_READ_CHANNEL_2;
				break;
				
			case 3:
				i2c_write(DS2482_WRITE_CHANNEL_3);
				check = DS2482_READ_CHANNEL_3;
				break;
				
			case 4:
				i2c_write(DS2482_WRITE_CHANNEL_4);
				check = DS2482_READ_CHANNEL_4;
				break;
				
			case 5:
				i2c_write(DS2482_WRITE_CHANNEL_5);
				check = DS2482_READ_CHANNEL_5;
				break;
				
			case 6:
				i2c_write(DS2482_WRITE_CHANNEL_6);
				check = DS2482_READ_CHANNEL_6;
				break;
				
			case 7:
				i2c_write(DS2482_WRITE_CHANNEL_7);
				check = DS2482_READ_CHANNEL_7;
				break;
		}
		
		i2c_rep_start(_address | I2C_READ);
		result = i2c_readNak();
		i2c_stop();
		
		if (result == check)
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
//	Input	none
//
//	Output	0 fail
//			1 success
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::wireReset(void)
{
	if (!(busy(1) & DS2482_STATUS_BUSY))
	{
		i2c_start_wait(_address | I2C_WRITE);
		i2c_write(DS2482_ONE_WIRE_RESET);
		i2c_stop();
		
		return 1;
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Write byte to OneWire
//
//	Input	data: byte to write
//
//	Output	0 fail
//			1 success
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::wireWrite(uint8_t data)
{
	if (!(busy(1) & DS2482_STATUS_BUSY))
	{
		i2c_start_wait(_address | I2C_WRITE);
		i2c_write(DS2482_ONE_WIRE_WRITE_BYTE);
		i2c_write(data);
		i2c_stop();
		
		return 1;
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Read byte from OneWire
//
//	Input	none
//
//	Output	byte read (0 if failure)
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::wireRead(void)
{
	if (!(busy(1) & DS2482_STATUS_BUSY))
	{
		i2c_start_wait(_address | I2C_WRITE);
		i2c_write(DS2482_ONE_WIRE_READ_BYTE);
		i2c_stop();
		
		if (!(busy(0) & DS2482_STATUS_BUSY))
		{
			return getRegister(DS2482_DATA_REG);
		}
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Write bit to OneWire
//
//	Input	bit: byte to write
//
//	Output	0 fail
//			1 success
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::wireWriteBit(uint8_t bit)
{
	if (!(busy(1) & DS2482_STATUS_BUSY))
	{
		i2c_start_wait(_address | I2C_WRITE);
		i2c_write(DS2482_ONE_WIRE_SINGLE_BIT);
		i2c_write((bit) ? 0x80 : 0);
		i2c_stop();
		
		return 1;
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Read bit from OneWire
//
//	Input	none
//
//	Output	bit read (0 if failure)
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::wireReadBit(void)
{
	if (wireWriteBit(1))
	{
		return (busy(0) & DS2482_STATUS_SBR) ? 1 : 0;
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Read 2 bits, write 1 to OneWire
//
//	Input	dir: direction if discrepency
//
//	Output	0 fail
//			1 success
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::wireTriplet(uint8_t dir)
{
	if (!(busy(0) & DS2482_STATUS_BUSY))
	{
		i2c_start_wait(_address | I2C_WRITE);
		i2c_write(DS2482_ONE_WIRE_TRIPLET);
		i2c_write((dir) ? 0x80 : 0);
		i2c_stop();
		
		return 1;
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Get rom address from device
//
//	Input	*address: pointer to 8 byte device rom buffer
//
//	Output	0 fail
//			1 success
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::romRead(uint8_t *address)
{
	uint8_t status;
	
	wireReset();
	status = busy(0);
	
	if (!(status & DS2482_STATUS_BUSY) && (status & DS2482_STATUS_PPD) && wireWrite(ONE_WIRE_READ_ROM))
	{
		uint8_t crc, i;
		
		for (i = 0; i < 8; i++)
		{
			address[i] = wireRead();
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
//	Input	*address: pointer to 8 byte device rom buffer
//
//	Output	0 fail
//			1 success
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::romMatch(uint8_t *address)
{
	uint8_t status;
	
	wireReset();
	status = busy(0);
	
	if (!(status & DS2482_STATUS_BUSY) && (status & DS2482_STATUS_PPD) && wireWrite(ONE_WIRE_MATCH_ROM))
	{
		uint8_t i;
		
		for (i = 0; i < 8; i++)
		{
			wireWrite(address[i]);
		}
		
		return 1;
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Skip rom address
//
//	Input	none
//
//	Output	0 fail
//			1 success
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::romSkip(void)
{
	uint8_t status;
	
	wireReset();
	status = busy(0);
	
	if (!(status & DS2482_STATUS_BUSY) && (status & DS2482_STATUS_PPD) && wireWrite(ONE_WIRE_SKIP_ROM))
	{
		return 1;
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Search OneWire for devices
//
//	Input	*address: pointer to 8 byte device rom buffer
//			family: family of device to find, = 0 for all devices
//
//	Output	0 fail
//			1 success
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
	
	wireReset();
	status = busy(0);
	
	if (!(status & DS2482_STATUS_BUSY) && (status & DS2482_STATUS_PPD) && wireWrite(ONE_WIRE_SEARCH_ROM))
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
				
				wireTriplet(dir);
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
//	Input	*address: pointer to 8 byte device rom buffer
//
//	Output	0 fail
//			1 success
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
//	Input	*address: pointer to 8 byte device rom buffer
//			powered: = 1 if external power, = 0 if parasitic
//			resolution: resolution of search
//
//	Output	0 fail
//			1 success
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::tempConversion(uint8_t *address, uint8_t powered, uint8_t resolution)
{
	if (address[0] == DS18B20_FAMILY_CODE && romMatch(address) && wireWrite(DS18B20_CONVERT_TEMP))
	{
		if (powered)
		{
			while(!wireReadBit())
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
	if (romSkip() && wireWrite(DS18B20_CONVERT_TEMP))
	{
		if (powered)
		{
			while(!wireReadBit())
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
//	Input	*address: pointer to 8 byte device rom buffer
//			*scratchpad: pointer to 8 byte scratchpad buffer
//
//	Output	0 fail
//			1 success
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::tempWriteScratchpad(uint8_t *address, uint8_t *scratchpad)
{
	if (address[0] == DS18B20_FAMILY_CODE && romMatch(address) && wireWrite(DS18B20_WRITE_SCRATCHPAD))
	{
		wireWrite(scratchpad[DS18B20_SCRATCHPAD_HIGH_ALARM]);
		wireWrite(scratchpad[DS18B20_SCRATCHPAD_LOW_ALARM]);
		wireWrite(scratchpad[DS18B20_SCRATCHPAD_CONFIG_REG]);
		
		return 1;
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Read temperature device scratchpad
//
//	Input	*address: pointer to 8 byte device rom buffer
//			*scratchpad: pointer to 8 byte scratchpad buffer
//
//	Output	0 fail
//			1 success
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::tempReadScratchpad(uint8_t *address, uint8_t *scratchpad)
{
	if (address[0] == DS18B20_FAMILY_CODE && romMatch(address) && wireWrite(DS18B20_READ_SCRATCHPAD))
	{
		uint8_t i;
		
		uint8_t crc = 0;
		
		for (i = 0; i < 8; i++)
		{
			scratchpad[i] = wireRead();
			crc = _crc_ibutton_update(crc, scratchpad[i]);
		}
		crc = _crc_ibutton_update(crc, wireRead());
		
		if (crc == 0)
		{
			return 1;
		}
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Write scratchpad to device EEPROM
//
//	Input	*address: pointer to 8 byte device rom buffer
//
//	Output	0 fail
//			1 success
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::tempWriteEEprom(uint8_t *address)
{
	if (address[0] == DS18B20_FAMILY_CODE && romMatch(address) && wireWrite(DS18B20_COPY_SCRATCHPAD))
	{
		return 1;
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Read device EEPROM to scratchpad
//
//	Input	*address: pointer to 8 byte device rom buffer
//
//	Output	0 fail
//			1 success
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::tempReadEEprom(uint8_t *address)
{
	if (address[0] == DS18B20_FAMILY_CODE && romMatch(address) && wireWrite(DS18B20_RECALL_EEPROM))
	{
		return 1;
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Get the power mode of a device or all devices
//
//	Input	*address: pointer to 8 byte device rom buffer
//
//	Output	0 fail
//			1 success
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::tempPowerMode(uint8_t *address)
{
	if (address[0] == DS18B20_FAMILY_CODE && romMatch(address) && wireWrite(DS18B20_READ_POWER_MODE))
	{
		return wireReadBit();
	}
	
	return 0;
}

uint8_t DS2482::tempPowerModeAll(void)
{
	if (romSkip() && wireWrite(DS18B20_READ_POWER_MODE))
	{
		return wireReadBit();
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
//	Input	none
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void DS2482::init(void)
{
	i2c_init();
	
	reset();
	writeConfig(0);
}


