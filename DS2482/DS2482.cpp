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
	i2c_start_wait(_address | I2C_WRITE);
	i2c_write(DS2482_DEVICE_RESET);
	
	i2c_rep_start(_address | I2C_READ);
	_status = i2c_readNak();
	i2c_stop();
	
	#ifdef DS2482_800
	_channel = 0;
	#endif
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

void DS2482::setConfig(uint8_t config)
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
	
	if (tmp != config)
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
uint8_t DS2482::setChannel(uint8_t channel)
{
	uint8_t check, tmp;
	
	if (channel < DS2482_TOTAL_CHANNELS && _channel != channel)
	{
		_busy(1);
		
		if (error_flags == 0)
		{
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
	}
	
	return _channel;
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
	
	if (!(_status & DS2482_STATUS_PPD))
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





















//-------------------------------------------------------------------------------------------------
//
// DS2482 initalization
//
//	Input	address: the address of the i2c device
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void DS2482::init(uint8_t address)
{
	_address = (DS2482_I2C_ADDRESS | (address & 0x03)) << 1;
	
	i2c_init();
	_reset();
	
	error_flags = 0;
	
	setConfig(0);
	
	searchLast = 0;
	searchDone = 1;
}



















//*************************************************************************************************
//	Constructor
//*************************************************************************************************

DS2482::DS2482()
{
}


//*************************************************************************************************
//	Preinstantiate object
//*************************************************************************************************

DS2482 ds2482 = DS2482();






