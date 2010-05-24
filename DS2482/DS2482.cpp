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
		2010/05/24	rewrote error handleing, use flags instead of return values
	
	All works by ITM are released under the creative commons attribution share alike license
		http://creativecommons.org/licenses/by-sa/3.0/
	
	I can be contacted at metcalfbuilt@gmail.com
*/


//*************************************************************************************************
//	Libraries
//*************************************************************************************************

#include "DS2482.h"









//*************************************************************************************************
//	Onewire controller functions
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

void DS2482::_reset(void)
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

uint8_t DS2482::_getRegister(uint8_t reg)
{
	uint8_t tmp;
	
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
	
	tmp = i2c_readNak();
	i2c_stop();
	
	return tmp;
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

void DS2482::_busy(uint8_t set)
{
	uint16_t timeout = 1000;
	
	_status = _getRegister((set) ? DS2482_STATUS_REG : 0);
	
	while ((_status & DS2482_STATUS_BUSY) && (timeout > 0))
	{
		_delay_us(20);
		
		_status = _getRegister(0);
		timeout--;
	}
	
	if (_status & DS2482_STATUS_BUSY)
	{
		error_flags |= (1 << ERROR_TIMEOUT);
	}
}

//-------------------------------------------------------------------------------------------------
//
// Write configuration to chip
//
//	Input	config: configuration nibble
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void DS2482::_setConfig(uint8_t config)
{
	uint8_t tmp;
	
	_busy(1);
	
	if (error_flags)
	{
		return;
	}
	
	tmp = ((~config) << 4) | (config & 0x0F);
	
	i2c_start_wait(_address | I2C_WRITE);
	i2c_write(DS2482_WRITE_CONFIG);
	i2c_write(tmp);
	
	i2c_rep_start(_address | I2C_READ);
	tmp = i2c_readNak();
	i2c_stop();
	
	if (tmp == config)
	{
		_config = config;
	}
	else
	{
		error_flags |= (1 << ERROR_CONFIG);
	}
}

//-------------------------------------------------------------------------------------------------
//
// Set chip channel
//
//	Input	channel: one wire channel
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

#ifdef DS2482_800
void DS2482::_setChannel(uint8_t channel)
{
	uint8_t check, tmp;
	
	_busy(1);
	
	if (error_flags)
	{
		return;
	}
	
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
	tmp = i2c_readNak();
	i2c_stop();
	
	if (tmp == check)
	{
		_channel = channel;
	}
	else
	{
		error_flags |= (1 << ERROR_CHANNEL);
	}
}
#endif








//*************************************************************************************************
//	Onewire Functions
//*************************************************************************************************

//-------------------------------------------------------------------------------------------------
//
// Reset OneWire
//
//	Input	none
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void DS2482::wireReset(void)
{
	_busy(1);
	
	if (error_flags)
	{
		return;
	}
	
	i2c_start_wait(_address | I2C_WRITE);
	i2c_write(DS2482_ONE_WIRE_RESET);
	i2c_stop();
	
	_busy(0);
	
	if (_status &  DS2482_STATUS_SD)
	{
		error_flags |= (1 << ERROR_SHORT_FOUND);
	}
	else if (!(_status & DS2482_STATUS_PPD))
	{
		error_flags |= (1 << ERROR_NO_DEVICE);
	}
}

//-------------------------------------------------------------------------------------------------
//
// Write byte to OneWire
//
//	Input	data: byte to write
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void DS2482::wireWrite(uint8_t data)
{
	_busy(1);
	
	if (error_flags)
	{
		return;
	}
	
	i2c_start_wait(_address | I2C_WRITE);
	i2c_write(DS2482_ONE_WIRE_WRITE_BYTE);
	i2c_write(data);
	i2c_stop();
}

//-------------------------------------------------------------------------------------------------
//
// Read byte from OneWire
//
//	Input	none
//
//	Output	byte read
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::wireRead(void)
{
	_busy(1);
	
	if (error_flags)
	{
		return 0;
	}
	
	i2c_start_wait(_address | I2C_WRITE);
	i2c_write(DS2482_ONE_WIRE_READ_BYTE);
	i2c_stop();
	
	_busy(0);
	
	if (error_flags)
	{
		return 0;
	}
	
	return _getRegister(DS2482_DATA_REG);
}

//-------------------------------------------------------------------------------------------------
//
// Write bit to OneWire
//
//	Input	bit: byte to write
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void DS2482::wireWriteBit(uint8_t bit)
{
	_busy(1);
	
	if (error_flags)
	{
		return;
	}
	
	i2c_start_wait(_address | I2C_WRITE);
	i2c_write(DS2482_ONE_WIRE_SINGLE_BIT);
	i2c_write((bit) ? 0x80 : 0);
	i2c_stop();
}

//-------------------------------------------------------------------------------------------------
//
// Read bit from OneWire
//
//	Input	none
//
//	Output	bit read
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::wireReadBit(void)
{
	wireWriteBit(1);
	_busy(0);
	
	return (_status & DS2482_STATUS_SBR) ? 1 : 0;
}

//-------------------------------------------------------------------------------------------------
//
// Read 2 bits, write 1 to OneWire
//
//	Input	dir: direction if discrepency
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void DS2482::wireTriplet(uint8_t dir)
{
	_busy(0);
	
	if (error_flags)
	{
		return;
	}
	
	i2c_start_wait(_address | I2C_WRITE);
	i2c_write(DS2482_ONE_WIRE_TRIPLET);
	i2c_write((dir) ? 0x80 : 0);
	i2c_stop();
	
	_busy(0);
}












//*************************************************************************************************
//	Onewire ROM functions
//*************************************************************************************************

//-------------------------------------------------------------------------------------------------
//
// Get rom address from device
//
//	Input	*address: pointer to 8 byte device rom buffer
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void DS2482::romRead(uint8_t *address)
{
	uint8_t crc, i;
	
	wireReset();
	wireWrite(ONE_WIRE_READ_ROM);
	
	if (error_flags)
	{
		return;
	}
	
	for (i = 0; i < 8; i++)
	{
		address[i] = wireRead();
		crc = _crc_ibutton_update(crc, address[i]);
	}
	
	if ((crc != 0) || (address[0] == 0))
	{
		error_flags |= (1 << ERROR_CRC_MISMATCH);
	}
}

//-------------------------------------------------------------------------------------------------
//
// Get device with rom address
//
//	Input	*address: pointer to 8 byte device rom buffer
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void DS2482::romMatch(uint8_t *address)
{
	uint8_t i;
	
	wireReset();
	wireWrite(ONE_WIRE_MATCH_ROM);
	
	if (error_flags)
	{
		return;
	}
	
	for (i = 0; i < 8; i++)
	{
		wireWrite(address[i]);
	}
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

void DS2482::romSkip(void)
{
	wireReset();
	wireWrite(ONE_WIRE_SKIP_ROM);
}

//-------------------------------------------------------------------------------------------------
//
// Search OneWire for devices
//
//	Input	*address: pointer to 8 byte device rom buffer
//			family: family of device to find, = 0 for all devices
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void DS2482::romSearch(uint8_t *address, uint8_t family)
{
	uint8_t lastZero, count, crc, i;
	
	if (searchDone == 1)
	{
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
	
	wireReset();
	wireWrite(ONE_WIRE_SEARCH_ROM);
	
	if (error_flags)
	{
		searchDone = 1;
		return;
	}
	
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
			
			wireTriplet(dir);
			
			if (error_flags)
			{
				searchDone = 1;
				return;
			}
			
			sbr = (_status & DS2482_STATUS_SBR);
			tsb = (_status & DS2482_STATUS_TSB);
			dir = (_status & DS2482_STATUS_DIR);
			
			if (sbr && tsb)
			{
				error_flags |= (1 << ERROR_SEARCH);
				
				searchDone = 1;
				return;
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
	
	if ((crc != 0) || (search_rom[0] == 0))
	{
		error_flags |= (1 << ERROR_CRC_MISMATCH);
		
		searchDone = 1;
		return;
	}
	
	if ((family != 0) && (search_rom[0] != family))
	{
		error_flags |= (1 << ERROR_SEARCH);
		
		searchDone = 1;
		return;
	}
	
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
}














//*************************************************************************************************
//	Onewire temperature sensor functions
//*************************************************************************************************

//-------------------------------------------------------------------------------------------------
//
// Get the power mode of all devices on channel
//
//	Input	none
//
//	Output	power mode
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::tempPowerMode(void)
{
	romSkip();
	wireWrite(DS18B20_READ_POWER_MODE);
	
	return wireReadBit();
}

//-------------------------------------------------------------------------------------------------
//
// Get the power mode of a device
//
//	Input	&sensor: reference to device data
//
//	Output	power mode
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::tempPowerMode(Device &sensor)
{
	romMatch(sensor.addr);
	wireWrite(DS18B20_READ_POWER_MODE);
	
	return wireReadBit();
}



//-------------------------------------------------------------------------------------------------
//
// Store scratchpad to device EEPROM
//
//	Input	&sensor: reference to device data
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void DS2482::tempStoreEE(Device &sensor)
{
	if (sensor.addr[0] != DS18B20_FAMILY_CODE)
	{
		return;
	}
	
	romMatch(sensor.addr);
	
	if (!sensor.config.powered)
	{
		_setConfig(_config | DS2482_CONFIG_SPU);
	}
	
	wireWrite(DS18B20_COPY_SCRATCHPAD);
	
	if (sensor.config.powered)
	{
		while(!wireReadBit())
		{
			_delay_us(20);
		}
	}
	else
	{
		_delay_ms(10);
	}
}

//-------------------------------------------------------------------------------------------------
//
// Load device EEPROM to scratchpad
//
//	Input	&sensor: reference to device data
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void DS2482::tempLoadEE(Device &sensor)
{
	if (sensor.addr[0] != DS18B20_FAMILY_CODE)
	{
		return;
	}
	
	romMatch(sensor.addr);
	wireWrite(DS18B20_RECALL_EEPROM);
}


//-------------------------------------------------------------------------------------------------
//
// Initiate temperature conversion for all devices on channel
//
//	Input	channel: channel to convert
//			resolution: max resolution on channel
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------


void DS2482::tempConversion(uint8_t channel, uint8_t resolution)
{
	uint8_t powered;
	
	#ifdef DS2482_800
	if (_channel != channel)
	{
		_setChannel(channel);
	}
	#endif
	
	powered = tempPowerMode();
	
	romSkip();
	
	if (!powered)
	{
		_setConfig(_config | DS2482_CONFIG_SPU);
	}
	
	wireWrite(DS18B20_CONVERT_TEMP);
	
	if (powered)
	{
		while(!wireReadBit())
		{
			_delay_us(20);
		}
	}
	else
	{
		_delay_ms(94 << resolution);
	}
}
//-------------------------------------------------------------------------------------------------
//
// Initiate temperature conversion for device
//
//	Input	&sensor: reference to device data
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void DS2482::tempConversion(Device &sensor)
{
	if (sensor.addr[0] != DS18B20_FAMILY_CODE)
	{
		return;
	}
	
	#ifdef DS2482_800
	if (_channel != sensor.config.channel)
	{
		_setChannel(sensor.config.channel);
	}
	#endif
	
	romMatch(sensor.addr);
	
	if (!sensor.config.powered)
	{
		_setConfig(_config | DS2482_CONFIG_SPU);
	}
	
	wireWrite(DS18B20_CONVERT_TEMP);
	
	if (sensor.config.powered)
	{
		while(!wireReadBit())
		{
			_delay_us(20);
		}
	}
	else
	{
		_delay_ms(94 << sensor.config.resolution);
	}
}


//-------------------------------------------------------------------------------------------------
//
// Write scratchpad to temperature device
//
//	Input	&sensor: reference to device data
//			&scratch: reference to scratchpad
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void DS2482::tempWriteScratchpad(Device &sensor, Scratch &scratch)
{
	if (sensor.addr[0] != DS18B20_FAMILY_CODE)
	{
		return;
	}
	
	#ifdef DS2482_800
	if (_channel != sensor.config.channel)
	{
		_setChannel(sensor.config.channel);
	}
	#endif
	
	romMatch(sensor.addr);
	wireWrite(DS18B20_WRITE_SCRATCHPAD);
	
	wireWrite(scratch.alarmHigh);
	wireWrite(scratch.alarmLow);
	wireWrite(scratch.config);
	
	tempStoreEE(sensor);
}

//-------------------------------------------------------------------------------------------------
//
// Read temperature device scratchpad
//
//	Input	&sensor: reference to device data
//			&scratch: reference to scratchpad
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void DS2482::tempReadScratchpad(Device &sensor, Scratch &scratch)
{
	uint8_t scratch_buf[9];
	uint8_t i, crc;
	
	if (sensor.addr[0] != DS18B20_FAMILY_CODE)
	{
		return;
	}
	
	#ifdef DS2482_800
	if (_channel != sensor.config.channel)
	{
		_setChannel(sensor.config.channel);
	}
	#endif
	
	romMatch(sensor.addr);
	wireWrite(DS18B20_READ_SCRATCHPAD);
	
	crc = 0;
	
	for (i = 0; i < 9; i++)
	{
		scratch_buf[i] = wireRead();
		crc = _crc_ibutton_update(crc, scratch_buf[i]);
	}
	
	if (crc != 0)
	{
		error_flags |= (1 << ERROR_CRC_MISMATCH);
	}
	
	scratch.temp[TEMP_C] = scratch_buf[DS18B20_SCRATCHPAD_TEMP_LSB];
	scratch.temp[TEMP_C] |= ((int16_t)scratch_buf[DS18B20_SCRATCHPAD_TEMP_MSB]) << 8;
	
	scratch.temp[TEMP_F] = (scratch.temp[TEMP_C] * 9) / 5;
	scratch.temp[TEMP_F] += (32 << 4);
	
	scratch.alarmHigh = scratch_buf[DS18B20_SCRATCHPAD_HIGH_ALARM];
	scratch.alarmLow = scratch_buf[DS18B20_SCRATCHPAD_LOW_ALARM];
	
	scratch.config = scratch_buf[DS18B20_SCRATCHPAD_CONFIG_REG];
}

//-------------------------------------------------------------------------------------------------
//
// Search OneWire for temperature devices
//
//	Input	&sensor: reference to device data
//			&scratch: reference to scratchpad
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void DS2482::tempSearch(Device &sensor, Scratch &scratch)
{
	romSearch(sensor.addr, DS18B20_FAMILY_CODE);
	sensor.config.channel = _channel & 0x07;
	sensor.config.powered = tempPowerMode(sensor) ? 0x01 : 0;
	
	tempReadScratchpad(sensor, scratch);
	sensor.config.resolution = (scratch.config CONFIG_RES_SHIFT) & 0x03;
}
















//*************************************************************************************************
//	Onewire temperature sensor management functions
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
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void DS2482::tempSensorLoad(uint8_t num, DEVICE &sensor)
{
	uint16_t offset;
	uint8_t crc, i;
	
	if (num <= 0 || num > eepromTotal)
	{
		return;
	}
	
	offset = num * sizeof(DEVICE);
	
	if (offset > DS2482_EEPROM_MAX_ALLOC)
	{
		error_flags |= (1 << ERROR_EEPROM_FULL);
		return;
	}
	
	eeprom_read_block((void*)&sensor, (const void*)(E2END - offset), sizeof(DEVICE));
	
	crc = 0;
	
	for (i = 0; i < 8; i++)
	{
		crc = _crc_ibutton_update(crc, sensor.addr[i]);
	}
	
	if (crc != 0)
	{
		error_flags |= (1 << ERROR_CRC_MISMATCH);
	}
}

//-------------------------------------------------------------------------------------------------
//
// Store sensor data to Eeprom
//
//	Input	num: device number
//			&sensor: reference to device data
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void DS2482::tempSensorStore(uint8_t num, DEVICE &sensor)
{
	uint16_t offset;
	
	if (num <= 0 && num > eepromTotal + 1)
	{
		return;
	}
	
	offset = num * sizeof(DEVICE);
	
	if (offset > DS2482_EEPROM_MAX_ALLOC)
	{
		error_flags |= (1 << ERROR_EEPROM_FULL);
		return;
	}
	
	eeprom_write_block((const void*)&sensor, (void*)(E2END - offset), sizeof(DEVICE));
	
	if (num > eepromTotal)
	{
		eepromTotal++;
		eeprom_write_byte((uint8_t*)E2END, eepromTotal);
	}
}

//-------------------------------------------------------------------------------------------------
//
// Verify sensor exists (writes to eeprom if config info has changed)
//
//	Input	num: device number
//			&sensor: reference to device data
//
//	Output	0 device not found
//			1 device found
//			2 settings changed
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::tempSensorVarify(uint8_t num, Device &sensor)
{
	uint8_t channel = sensor.config.channel;
	
	do
	{
		Scratch scratch_buff;
		
		tempReadScratchpad(sensor, scratch_buff);
		
		if (error_flags == 0)
		{
			uint8_t resolution, powered;
			
			resolution = (scratch_buff.config CONFIG_RES_SHIFT) & 0x03;
			powered = tempPowerMode(sensor) ? 0x01 : 0;
			
			if (sensor.config.resolution != resolution || sensor.config.powered != powered || sensor.config.channel != channel)
			{
				sensor.config.resolution = resolution;
				sensor.config.powered = powered;
				tempSensorStore(num, sensor);
				
				return 2;
			}
			return 1;
		}
		else
		{
			error_flags &= ~((1 << ERROR_NO_DEVICE) | (1 << ERROR_CRC_MISMATCH));
			
			if (error_flags)
			{
				return 0;
			}
		}
		
		#ifdef DS2482_800
		if (sensor.config.channel < TOTAL_CHANNELS - 1)
		{
			sensor.config.channel++;
		}
		else
		{
			sensor.config.channel = 0;
		}
		#endif
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
//	Output	0 device not found
//			1 new device found
//
//-------------------------------------------------------------------------------------------------

uint8_t DS2482::tempSensorFind(Device &sensor, Scratch &scratch)
{
	uint8_t channel;
	
	searchDone = 1;
	channel = 0;
	
	do
	{
		#ifdef DS2482_800
		if (_channel != channel)
		{
			_setChannel(channel);
		}
		#endif
		
		tempSearch(sensor, scratch);
		
		if (error_flags == 0)
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
		else
		{
			error_flags &= ~(1 << ERROR_NO_DEVICE);
			
			if (error_flags)
			{
				return 0;
			}
		}
		
		if (searchDone == 1)
		{
			channel++;
		}
	}
	while (searchDone != 1 || channel < TOTAL_CHANNELS);
	
	return 0;
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
	
	error_flags = 0;
	
	_reset();
	_setConfig(0);
	
	searchLast = 0;
	searchDone = 1;
	
	eepromTotal = eeprom_read_byte((const uint8_t*)E2END);
	
	if (eepromTotal > (DS2482_EEPROM_MAX_ALLOC / sizeof(DEVICE)))
	{
		eepromTotal = 0;
	}
}



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

