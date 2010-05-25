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


//*************************************************************************************************
//	Libraries
//*************************************************************************************************

#include "DS18B20.h"


















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

uint8_t DS18B20::powerMode(void)
{
	ds2482.romSkip();
	ds2482.wireWrite(DS18B20_READ_POWER_MODE);
	
	return ds2482.wireReadBit();
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

uint8_t DS18B20::powerMode(Device &sensor)
{
	ds2482.romMatch(sensor.addr);
	ds2482.wireWrite(DS18B20_READ_POWER_MODE);
	
	return ds2482.wireReadBit();
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

void DS18B20::storeSensorEE(Device &sensor)
{
	if (sensor.addr[0] != DS18B20_FAMILY_CODE)
	{
		return;
	}
	
	ds2482.romMatch(sensor.addr);
	
	if (!sensor.config.powered)
	{
		ds2482.setConfig(DS2482_CONFIG_SPU);
	}
	
	ds2482.wireWrite(DS18B20_COPY_SCRATCHPAD);
	
	if (sensor.config.powered)
	{
		while(!ds2482.wireReadBit())
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

void DS18B20::loadSensorEE(Device &sensor)
{
	if (sensor.addr[0] != DS18B20_FAMILY_CODE)
	{
		return;
	}
	
	ds2482.romMatch(sensor.addr);
	ds2482.wireWrite(DS18B20_RECALL_EEPROM);
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


void DS18B20::startConversion(uint8_t channel, uint8_t resolution, uint8_t delay)
{
	uint8_t powered;
	
	#ifdef DS2482_800
	ds2482.setChannel(channel);
	#endif
	
	powered = powerMode();
	
	ds2482.romSkip();
	
	if (!powered)
	{
		ds2482.setConfig(DS2482_CONFIG_SPU);
	}
	
	ds2482.wireWrite(DS18B20_CONVERT_TEMP);
	
	if (delay)
	{
		if (powered)
		{
			while(!ds2482.wireReadBit())
			{
				_delay_us(20);
			}
		}
		else
		{
			_delay_ms(94 << resolution);
		}
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

void DS18B20::startConversion(Device &sensor, uint8_t delay)
{
	if (sensor.addr[0] != DS18B20_FAMILY_CODE)
	{
		return;
	}
	
	#ifdef DS2482_800
	ds2482.setChannel(sensor.config.channel);
	#endif
	
	ds2482.romMatch(sensor.addr);
	
	if (!sensor.config.powered)
	{
		ds2482.setConfig(DS2482_CONFIG_SPU);
	}
	
	ds2482.wireWrite(DS18B20_CONVERT_TEMP);
	
	if (delay)
	{
		if (sensor.config.powered)
		{
			while(!ds2482.wireReadBit())
			{
				_delay_us(20);
			}
		}
		else
		{
			_delay_ms(94 << sensor.config.resolution);
		}
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

void DS18B20::writeScratchpad(Device &sensor, Scratch &scratch)
{
	if (sensor.addr[0] != DS18B20_FAMILY_CODE)
	{
		return;
	}
	
	#ifdef DS2482_800
	ds2482.setChannel(sensor.config.channel);
	#endif
	
	ds2482.romMatch(sensor.addr);
	ds2482.wireWrite(DS18B20_WRITE_SCRATCHPAD);
	
	ds2482.wireWrite(scratch.alarmHigh);
	ds2482.wireWrite(scratch.alarmLow);
	ds2482.wireWrite(scratch.config);
	
	storeSensorEE(sensor);
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

void DS18B20::readScratchpad(Device &sensor, Scratch &scratch)
{
	uint8_t scratch_buf[9];
	uint8_t i, crc;
	
	if (sensor.addr[0] != DS18B20_FAMILY_CODE)
	{
		return;
	}
	
	#ifdef DS2482_800
	ds2482.setChannel(sensor.config.channel);
	#endif
	
	ds2482.romMatch(sensor.addr);
	ds2482.wireWrite(DS18B20_READ_SCRATCHPAD);
	
	crc = 0;
	
	for (i = 0; i < 9; i++)
	{
		scratch_buf[i] = ds2482.wireRead();
		crc = _crc_ibutton_update(crc, scratch_buf[i]);
	}
	
	if (crc != 0)
	{
		ds2482.error_flags |= (1 << ERROR_CRC_MISMATCH);
	}
	
	scratch.temp[TEMP_C] = scratch_buf[DS18B20_SCRATCHPAD_TEMP_LSB];
	scratch.temp[TEMP_C] |= ((int16_t)scratch_buf[DS18B20_SCRATCHPAD_TEMP_MSB]) << 8;
	
	scratch.temp[TEMP_F] = (scratch.temp[TEMP_C] * 9) / 5;
	scratch.temp[TEMP_F] += (32 << 4);
	
	scratch.alarmHigh = scratch_buf[DS18B20_SCRATCHPAD_HIGH_ALARM];
	scratch.alarmLow = scratch_buf[DS18B20_SCRATCHPAD_LOW_ALARM];
	
	scratch.config = scratch_buf[DS18B20_SCRATCHPAD_CONFIG_REG];
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

void DS18B20::resetSensors(void)
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

uint8_t DS18B20::totalSensors(void)
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

void DS18B20::loadSensor(uint8_t num, DEVICE &sensor)
{
	uint16_t offset;
	uint8_t crc, i;
	
	if (num <= 0 || num > eepromTotal)
	{
		return;
	}
	
	offset = num * sizeof(DEVICE);
	
	if (offset > DS18B20_EEPROM_MAX_ALLOC)
	{
		ds2482.error_flags |= (1 << ERROR_EEPROM_FULL);
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
		ds2482.error_flags |= (1 << ERROR_CRC_MISMATCH);
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

void DS18B20::storeSensor(uint8_t num, DEVICE &sensor)
{
	uint16_t offset;
	
	if (num <= 0 && num > eepromTotal + 1)
	{
		return;
	}
	
	offset = num * sizeof(DEVICE);
	
	if (offset > DS18B20_EEPROM_MAX_ALLOC)
	{
		ds2482.error_flags |= (1 << ERROR_EEPROM_FULL);
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

uint8_t DS18B20::varifySensor(uint8_t num, Device &sensor)
{
	uint8_t channel = sensor.config.channel;
	
	do
	{
		Scratch scratch_buff;
		
		readScratchpad(sensor, scratch_buff);
		
		if (ds2482.error_flags == 0)
		{
			uint8_t resolution, powered;
			
			resolution = (scratch_buff.config CONFIG_RES_SHIFT) & 0x03;
			powered = powerMode(sensor) ? 0x01 : 0;
			
			if (sensor.config.resolution != resolution || sensor.config.powered != powered || sensor.config.channel != channel)
			{
				sensor.config.resolution = resolution;
				sensor.config.powered = powered;
				storeSensor(num, sensor);
				
				return 2;
			}
			return 1;
		}
		else
		{
			ds2482.error_flags &= ~((1 << ERROR_NO_DEVICE) | (1 << ERROR_CRC_MISMATCH));
			
			if (ds2482.error_flags)
			{
				return 0;
			}
		}
		
		#ifdef DS2482_800
		if (sensor.config.channel < DS2482_TOTAL_CHANNELS - 1)
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

uint8_t DS18B20::findSensor(Device &sensor, Scratch &scratch)
{
	uint8_t channel = 0;
	
	ds2482.searchDone = 1;
	
	do
	{
		#ifdef DS2482_800
		ds2482.setChannel(channel);
		#endif
		
		//tempSearch(sensor, scratch);
		
		ds2482.romSearch(sensor.addr, DS18B20_FAMILY_CODE);
		
		sensor.config.channel = channel;
		sensor.config.powered = powerMode(sensor) ? 0x01 : 0;
		
		readScratchpad(sensor, scratch);
		sensor.config.resolution = (scratch.config CONFIG_RES_SHIFT) & 0x03;
		
		if (ds2482.error_flags == 0)
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
				loadSensor(num, device);
				
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
			ds2482.error_flags &= ~(1 << ERROR_NO_DEVICE);
			
			if (ds2482.error_flags)
			{
				return 0;
			}
		}
		
		if (ds2482.searchDone == 1)
		{
			channel++;
		}
	}
	while (ds2482.searchDone != 1 || channel < DS2482_TOTAL_CHANNELS);
	
	return 0;
}














//-------------------------------------------------------------------------------------------------
//
// DS18B20 initalization
//
//	Input	none
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void DS18B20::init(void)
{
	eepromTotal = eeprom_read_byte((const uint8_t*)E2END);
	
	if (eepromTotal > (DS18B20_EEPROM_MAX_ALLOC / sizeof(DEVICE)))
	{
		eepromTotal = 0;
	}
}



















//*************************************************************************************************
//	Constructor
//*************************************************************************************************

DS18B20::DS18B20()
{
}


//*************************************************************************************************
//	Preinstantiate object
//*************************************************************************************************

DS18B20 dsTemp = DS18B20();






