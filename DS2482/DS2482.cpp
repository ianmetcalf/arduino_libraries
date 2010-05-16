/*
	Library for the DS2484 OneWire controller by Ian T Metcalf
		tested with the Arduino IDE v18 on a Duemilanova 328
	
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
	
	All works by ITM are released under the creative commons attribution share alike license
		http://creativecommons.org/licenses/by-sa/3.0/
	
	I can be contacted at metcalfbuilt@gmail.com
*/

#include "DS2482.h"


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

uint8_t DS2482::getError(void)
{
	uint8_t error = _error;
	
	_error = ERROR_NONE;
	
	return error;
}







//*************************************************************************************************
//
//		Basic Chip Functions
//
//*************************************************************************************************

//-------------------------------------------------------------------------------------------------
//
// Reset the chip
//
//	Input	none
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void DS2482::chipReset(void)
{
	_channel = 0;
	_config = 0;
	
	i2c_start_wait(_address | I2C_WRITE);
	i2c_write(DS2482_DEVICE_RESET);
	
	i2c_rep_start(_address | I2C_READ);
	_status = i2c_readNak();
	i2c_stop();
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

uint8_t DS2482::chipRegister(uint8_t reg)
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
// Wait until the chip is not busy or it times out
//
//	Input	set: set the register pointer T/F
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void DS2482::chipBusy(uint8_t set)
{
	uint16_t timeout = 1000;
	
	_status = chipRegister((set) ? DS2482_STATUS_REG : 0);
	
	while ((_status & DS2482_STATUS_BUSY) && (timeout > 0))
	{
		delayMicroseconds(20);
		_status = chipRegister(0);
		timeout--;
	}
	
	if (_status & DS2482_STATUS_BUSY)
	{
		_error = ERROR_TIMEOUT;
	}
}

//-------------------------------------------------------------------------------------------------
//
// Write configuration to chip
//
//	Input	config: configuration nibble
//
//	Output	0 fail
//			1 success
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::chipConfigure(uint8_t config)
{
	chipBusy(1);
	
	if (!_error)
	{
		uint8_t result;
		
		i2c_start_wait(_address | I2C_WRITE);
		i2c_write(DS2482_WRITE_CONFIG);
		i2c_write(config | ((~config) << 4));
		
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
// Set chip channel
//
//	Input	channel: one wire channel
//
//	Output	0 fail
//			1 success
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::chipChannel(uint8_t channel)
{
	chipBusy(1);
	
	if (!_error && channel < DS2482_TOTAL_CHANNELS)
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









//*************************************************************************************************
//
//		Basic OneWire Functions
//
//*************************************************************************************************

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
	chipBusy(1);
	
	if (!_error)
	{
		i2c_start_wait(_address | I2C_WRITE);
		i2c_write(DS2482_ONE_WIRE_RESET);
		i2c_stop();
		
		chipBusy(0);
		
		if (!_error)
		{
			if (_status &  DS2482_STATUS_SD)
			{
				_error = ERROR_SHORT_FOUND;
			}
			else if (!(_status & DS2482_STATUS_PPD))
			{
				_error = ERROR_NO_DEVICE;
			}
			else
			{
				return 1;
			}
		}
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
	chipBusy(1);
	
	if (!_error)
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
	chipBusy(1);
	
	if (!_error)
	{
		i2c_start_wait(_address | I2C_WRITE);
		i2c_write(DS2482_ONE_WIRE_READ_BYTE);
		i2c_stop();
		
		chipBusy(0);
		
		if (!_error)
		{
			return chipRegister(DS2482_DATA_REG);
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
	chipBusy(1);
	
	if (!_error)
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
		chipBusy(0);
		
		if (!_error)
		{
			return (_status & DS2482_STATUS_SBR) ? 1 : 0;
		}
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
	chipBusy(0);
	
	if (!_error)
	{
		i2c_start_wait(_address | I2C_WRITE);
		i2c_write(DS2482_ONE_WIRE_TRIPLET);
		i2c_write((dir) ? 0x80 : 0);
		i2c_stop();
		
		chipBusy(0);
		
		if (!_error)
		{
			return 1;
		}
	}
	
	return 0;
}












//*************************************************************************************************
//
//		Basic Device ROM Functions
//
//*************************************************************************************************

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
	if (wireReset() && wireWrite(ONE_WIRE_READ_ROM))
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
	if (wireReset() && wireWrite(ONE_WIRE_MATCH_ROM))
	{
		uint8_t i;
		
		for (i = 0; i < 8; i++)
		{
			wireWrite(address[i]);
		}
		
		if (!_error)
		{
			return 1;
		}
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
	if (wireReset() && wireWrite(ONE_WIRE_SKIP_ROM))
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
	if (searchDone == 1)
	{
		uint8_t i;
		
		if (family == 0)
		{
			search_rom[0] = 0;
			searchLast = 0;
		}
		else
		{
			search_rom[0] = family;
			searchLast = 64;
		}
		
		for (i = 1; i < 8; i++)
		{
			search_rom[i] = 0x00;
		}
		
		searchDone = 0;
	}
	
	if (wireReset() && wireWrite(ONE_WIRE_SEARCH_ROM))
	{
		uint8_t lastZero, count, crc, i;
		
		lastZero = 0;
		count = 0;
		crc = 0;
		
		for (i = 0; i < 8; i++)
		{
			uint8_t romMask;
			
			for (romMask = 1; romMask; romMask <<= 1)
			{
				uint8_t sbr, tsb, dir;
				
				dir = (count < searchLast) ? (search_rom[i] & romMask) : ((count == searchLast) ? 1 : 0);
				
				if (!wireTriplet(dir))
				{
					searchDone = 1;
					return 0;
				}
				
				sbr = (_status & DS2482_STATUS_SBR);
				tsb = (_status & DS2482_STATUS_TSB);
				dir = (_status & DS2482_STATUS_DIR);
				
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
			
			if (lastZero == 0)
			{
				searchLast = 0;
				searchDone = 1;
			}
			else
			{
				searchLast = lastZero;
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













//*************************************************************************************************
//
//		Temperature Sensor Functions
//
//*************************************************************************************************

//-------------------------------------------------------------------------------------------------
//
// Get the power mode of a device or all devices
//
//	Input	&sensor: reference to device data
//
//	Output	0 fail
//			1 success
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::tempPowerMode(Device &sensor)
{
	if (sensor.addr[0] == DS18B20_FAMILY_CODE && romMatch(sensor.addr) && wireWrite(DS18B20_READ_POWER_MODE))
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

//-------------------------------------------------------------------------------------------------
//
// Store scratchpad to device EEPROM
//
//	Input	&sensor: reference to device data
//
//	Output	0 fail
//			1 success
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::tempStoreEE(Device &sensor)
{
	if (sensor.addr[0] == DS18B20_FAMILY_CODE && romMatch(sensor.addr))
	{
		if (!sensor.config.powered && !chipConfigure(_config | DS2482_CONFIG_SPU))
		{
			return 0;
		}
		
		if (wireWrite(DS18B20_COPY_SCRATCHPAD))
		{
			if (sensor.config.powered)
			{
				while(!wireReadBit())
				{
					delayMicroseconds(20);
				}
			}
			else
			{
				delay(10);
			}
			
			return 1;
		}
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Load device EEPROM to scratchpad
//
//	Input	&sensor: reference to device data
//
//	Output	0 fail
//			1 success
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::tempLoadEE(Device &sensor)
{
	if (sensor.addr[0] == DS18B20_FAMILY_CODE && romMatch(sensor.addr) && wireWrite(DS18B20_RECALL_EEPROM))
	{
		return 1;
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Initiate temperature conversion for device
//
//	Input	&sensor: reference to device data
//
//	Output	0 fail
//			1 success
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::tempConversion(Device &sensor)
{
	if (_channel != sensor.config.channel && !chipChannel(sensor.config.channel))
	{
		return 0;
	}
	
	if (sensor.addr[0] == DS18B20_FAMILY_CODE && romMatch(sensor.addr))
	{
		if (!sensor.config.powered && !chipConfigure(_config | DS2482_CONFIG_SPU))
		{
			return 0;
		}
		
		if (wireWrite(DS18B20_CONVERT_TEMP))
		{
			if (sensor.config.powered)
			{
				while(!wireReadBit())
				{
					delayMicroseconds(20);
				}
			}
			else
			{
				delay(94 << sensor.config.resolution);
			}
			
			return 1;
		}
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Initiate temperature conversion for all devices on channel
//
//	Input	channel: channel to convert
//			resolution: max resolution on channel
//
//	Output	0 fail
//			1 success
//
//-------------------------------------------------------------------------------------------------


uint8_t DS2482::tempConversionAll(uint8_t channel, uint8_t resolution)
{
	uint8_t powered;
	
	if (_channel != channel && !chipChannel(channel))
	{
		return 0;
	}
	
	powered = tempPowerModeAll();
	
	if (romSkip())
	{
		if (!powered)
		{
			chipConfigure(_config | DS2482_CONFIG_SPU);
		}
		
		if (wireWrite(DS18B20_CONVERT_TEMP))
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
				delay(94 << resolution);
			}
			
			return 1;
		}
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Write scratchpad to temperature device
//
//	Input	&sensor: reference to device data
//			&scratch: reference to scratchpad
//
//	Output	0 fail
//			1 success
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::tempWriteScratchpad(Device &sensor, Scratch &scratch)
{
	if (_channel != sensor.config.channel && !chipChannel(sensor.config.channel))
	{
		return 0;
	}
	
	if (sensor.addr[0] == DS18B20_FAMILY_CODE && romMatch(sensor.addr) && wireWrite(DS18B20_WRITE_SCRATCHPAD))
	{
		if (wireWrite(scratch.alarmHigh) && wireWrite(scratch.alarmLow) && wireWrite(scratch.config))
		{
			if (tempStoreEE(sensor))
			{
				return 1;
			}
		}
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Read temperature device scratchpad
//
//	Input	&sensor: reference to device data
//			&scratch: reference to scratchpad
//
//	Output	0 fail
//			1 success
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::tempReadScratchpad(Device &sensor, Scratch &scratch)
{
	if (_channel != sensor.config.channel && !chipChannel(sensor.config.channel))
	{
		return 0;
	}
	
	if (sensor.addr[0] == DS18B20_FAMILY_CODE && romMatch(sensor.addr) && wireWrite(DS18B20_READ_SCRATCHPAD))
	{
		uint8_t scratch_buf[9];
		uint8_t i;
		
		uint8_t crc = 0;
		
		for (i = 0; i < 9; i++)
		{
			scratch_buf[i] = wireRead();
			crc = _crc_ibutton_update(crc, scratch_buf[i]);
		}
		
		if (crc == 0)
		{
			scratch.temp[TEMP_C] = scratch_buf[DS18B20_SCRATCHPAD_TEMP_LSB];
			scratch.temp[TEMP_C] |= ((int16_t)scratch_buf[DS18B20_SCRATCHPAD_TEMP_MSB]) << 8;
			
			scratch.temp[TEMP_F] = (scratch.temp[TEMP_C] * 9) / 5;
			scratch.temp[TEMP_F] += (32 << 4);
			
			scratch.alarmHigh = scratch_buf[DS18B20_SCRATCHPAD_HIGH_ALARM];
			scratch.alarmLow = scratch_buf[DS18B20_SCRATCHPAD_LOW_ALARM];
			
			scratch.config = scratch_buf[DS18B20_SCRATCHPAD_CONFIG_REG];
			
			return 1;
		}
		else
		{
			_error = ERROR_CRC_MISMATCH;
		}
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Search OneWire for temperature devices
//
//	Input	&sensor: reference to device data
//			&scratch: reference to scratchpad
//
//	Output	0 fail
//			1 success
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::tempSearch(Device &sensor, Scratch &scratch)
{
	if (romSearch(sensor.addr, DS18B20_FAMILY_CODE))
	{
		sensor.config.channel = _channel & 0x07;
		sensor.config.powered = tempPowerMode(sensor) ? 0x01 : 0;
		
		if (tempReadScratchpad(sensor, scratch))
		{
			sensor.config.resolution = (scratch.config CONFIG_RES_SHIFT) & 0x03;
			
			return 1;
		}
	}
	
	return 0;
}
















//*************************************************************************************************
//
//		Temperature Sensor Management Functions
//
//*************************************************************************************************

//-------------------------------------------------------------------------------------------------
//
// Reset the number of sensors to zero (saves total to eeprom)
//
//	Input	none
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void DS2482::tempSensorReset(void)
{
	eepromTotal = 0;
	eeprom_write_byte((uint8_t*)E2END, eepromTotal);
}

//-------------------------------------------------------------------------------------------------
//
// Get the total number of sensors stored
//
//	Input	none
//
//	Output	device count
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::tempSensorTotal(void)
{
	return eepromTotal;
}

//-------------------------------------------------------------------------------------------------
//
// Load sensor data from Eeprom
//
//	Input	num: device number
//			&sensor: reference to device data
//
//	Output	0 fail
//			1 success
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::tempSensorLoad(uint8_t num, DEVICE &sensor)
{
	if (num > 0 && num <= eepromTotal)
	{
		uint16_t offset;
		
		offset = num * sizeof(DEVICE);
		
		if (offset < DS2482_EEPROM_MAX_ALLOC)
		{
			uint8_t crc, i;
			
			eeprom_read_block((void*)&sensor, (const void*)(E2END - offset), sizeof(DEVICE));
			
			crc = 0;
			
			for (i = 0; i < 8; i++)
			{
				crc = _crc_ibutton_update(crc, sensor.addr[i]);
			}
			
			if (crc == 0)
			{
				return 1;
			}
		}
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Store sensor data to Eeprom
//
//	Input	num: device number
//			&sensor: reference to device data
//
//	Output	0 fail
//			1 success
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::tempSensorStore(uint8_t num, DEVICE &sensor)
{
	if (num > 0 && num <= eepromTotal + 1)
	{
		uint16_t offset;
		
		offset = num * sizeof(DEVICE);
		
		if (offset < DS2482_EEPROM_MAX_ALLOC)
		{
			eeprom_write_block((const void*)&sensor, (void*)(E2END - offset), sizeof(DEVICE));
			
			if (num > eepromTotal)
			{
				eepromTotal++;
				
				eeprom_write_byte((uint8_t*)E2END, eepromTotal);
			}
			
			return 1;
		}
		else
		{
			_error = ERROR_EEPROM_FULL;
		}
	}
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Verify sensor exists (writes to eeprom if config info has changed)
//
//	Input	num: device number
//			&sensor: reference to device data
//
//	Output	0 fail
//			1 success
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::tempSensorVarify(uint8_t num, Device &sensor)
{
	uint8_t channel = sensor.config.channel;
	
	do
	{
		Scratch scratch_buff;
		
		if (tempReadScratchpad(sensor, scratch_buff))
		{
			uint8_t resolution, powered;
			
			resolution = (scratch_buff.config CONFIG_RES_SHIFT) & 0x03;
			powered = tempPowerMode(sensor) ? 0x01 : 0;
			
			if (sensor.config.resolution == resolution && sensor.config.powered == powered && sensor.config.channel == channel)
			{
				return 1;
			}
			
			sensor.config.resolution = resolution;
			sensor.config.powered = powered;
			
			if (tempSensorStore(num, sensor))
			{
				return 2;
			}
		}
		else if (_error == ERROR_NO_DEVICE || _error == ERROR_CRC_MISMATCH)
		{
			_error = ERROR_NONE;
		}
		
		if (sensor.config.channel < DS2482_TOTAL_CHANNELS - 1)
		{
			sensor.config.channel++;
		}
		else
		{
			sensor.config.channel = 0;
		}
	}
	while (sensor.config.channel != channel);
	
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Find devices not stored in eeprom
//
//	Input	&sensor: reference to device data
//			&scratch: reference to scratchpad
//
//	Output	0 fail
//			1 success
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::tempSensorFind(Device &sensor, Scratch &scratch)
{
	uint8_t channel = 0;
	
	searchDone = 1;
	
	do
	{
		if (searchDone == 1 && !chipChannel(channel))
		{
			return 0;
		}
		
		if (tempSearch(sensor, scratch))
		{
			uint8_t num, romByte;
			num = 1;
			
			do
			{
				Device device;
				
				if (num > eepromTotal)
				{
					return 1;
				}
				
				romByte = 0;
				tempSensorLoad(num, device);
				
				while (romByte < 8 && sensor.addr[romByte] == device.addr[romByte])
				{
					romByte++;
				}
				
				num++;
			}
			while (romByte < 8);
		}
		else if (_error == ERROR_NO_DEVICE)
		{
			_error = ERROR_NONE;
		}
		
		if (searchDone == 1)
		{
			channel++;
		}
	}
	while (channel < DS2482_TOTAL_CHANNELS);
	
	return 0;
}















/*
//-------------------------------------------------------------------------------------------------
//
// Rescan and store device address to eeprom
//
//	Input	store: whether to store results or compare them
//			config: if set then write config to sensors
//
//	Output	0 fail
//			1 success
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::tempSensorRescan(uint8_t store, uint8_t config)
{
	uint16_t address;
	uint8_t devices, i;
	
	address = E2END;
	devices = 0;
	
	searchDone = 1;
	
	
	for (i = 0; i < DS2482_TOTAL_CHANNELS; i++)
	{
		if (!chipChannel(i))
		{
			return 0;
		}
		
		do
		{
			Device device_buff;
			Scratch scratch_buff;
			
			if (tempSearch(device_buff, scratch_buff))
			{
				devices++;
				
				if (store != 0)
				{
					address -= sizeof(DEVICE);
					
					if (address < E2END - DS2482_EEPROM_MAX_ALLOC)
					{
						_error = ERROR_EEPROM_FULL;
						return 0;
					}
					
					if (config != 0 && scratch_buff.config != config)
					{
						scratch_buff.config = config;
						
						if (!tempWriteScratchpad(device_buff, scratch_buff) || !tempStoreEE(device_buff))
						{
							return 0;
						}
						
						device_buff.config.resolution = (scratch_buff.config DS18B20_CONFIG_RESOLUTION_SHIFT) & 0x03;
					}
					
					eeprom_write_block((const void*)&device_buff, (void*)address, sizeof(DEVICE));
				}
			}
			else if (_error == ERROR_NO_DEVICE)
			{
				_error = ERROR_NONE;
			}
			else
			{
				return 0;
			}
		}
		while (searchDone == 0);
	}
	
	if (store != 0)
	{
		eepromTotal = devices;
		eeprom_write_byte((uint8_t*)E2END, eepromTotal);
		
		return 1;
	}
	else 
	{
		return (eepromTotal == devices) ? 1 : 0;
	}
}
*/















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
	
	_error = ERROR_NONE;
	
	chipReset();
	chipConfigure(0);
	
	searchLast = 0;
	searchDone = 1;
	
	eepromTotal = eeprom_read_byte((const uint8_t*)E2END);
	
	if (eepromTotal > (DS2482_EEPROM_MAX_ALLOC / sizeof(DEVICE)))
	{
		eepromTotal = 0;
	}
}


