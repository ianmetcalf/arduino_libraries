#include <DS2482.h>

DS2482 bridge = DS2482(0);

uint8_t count, totalDevices;

Device device;
Scratch scratchpad;

void error(uint8_t err)
{
	switch(err)
	{
		case ERROR_NONE:
			Serial.println("no errors");
			break;
		case ERROR_TIMEOUT:
			Serial.println("timeout");
			break;
		case ERROR_NO_DEVICE:
			Serial.println("no devices");
			break;
		case ERROR_SHORT_FOUND:
			Serial.println("short found");
			break;
		case ERROR_EEPROM_FULL:
			Serial.println("eeprom full");
			break;
		case ERROR_CRC_MISMATCH:
			Serial.println("crc mismatch");
			break;
		default:
			Serial.println("other");
			break;
	}
}

void printTemp(int16_t temp)
{
	uint8_t decimal;
	
	if (temp < 0)
	{
		Serial.print("-");
		temp = -temp;
	}
	
	Serial.print(temp / 16, DEC);
	
	decimal = temp & 0x0F;
	
	switch (decimal)
	{
		case 0x00:
			Serial.println(".0");
			break;
		case 0x01:
			Serial.println(".0625");
			break;
		case 0x02:
			Serial.println(".125");
			break;
		case 0x04:
			Serial.println(".25");
			break;
		case 0x08:
			Serial.println(".5");
			break;
		default:
			Serial.print(".");
			Serial.println(decimal * 625, DEC);
			break;
	}
}

void setup()
{
	Serial.begin(9600);
	Serial.flush();
	
	bridge.init();
	
	totalDevices = bridge.tempSensorTotal();
	
	Serial.print("total devices: ");
	Serial.println(totalDevices, DEC);
	
	Serial.println("varifying sensors...");
	Serial.println(" ");
	
	for (count = 1; count <= totalDevices; count++)
	{
		if (bridge.tempSensorLoad(count, device))
		{
			if (bridge.tempSensorVarify(count, device))
			{
				Serial.print("sensor: ");
				Serial.print(count);
				Serial.print("; ch: ");
				Serial.print(device.config.channel, DEC);
				Serial.print("; res: ");
				Serial.print(device.config.resolution, DEC);
				Serial.println(device.config.powered ? "; powered" : "; not powered");
			}
			else
			{
				Serial.println("reseting sensors...");
				// add prompt before reseting
				bridge.tempSensorReset();
				totalDevices = 0;
			}
		}
		
		error(bridge.getError());
		
		Serial.println(" ");
	}
	
	Serial.print("finding new sensors...");
	
	while(bridge.tempSensorFind(device, scratchpad))
	{
		Serial.println(" ");
		Serial.print("found device: ");
		
		for (count = 0; count < 8; count++)
		{
			Serial.print(" ");
			Serial.print(device.addr[count], HEX);
		}
		Serial.println(" ");
		
		if (scratchpad.config != CONFIG_RES_10_BIT)
		{
			scratchpad.config = CONFIG_RES_10_BIT;
			
			if (bridge.tempWriteScratchpad(device, scratchpad))
			{
				device.config.resolution = (CONFIG_RES_10_BIT CONFIG_RES_SHIFT) & 0x03;
				Serial.println("new config saved");
			}
		}
		
		totalDevices++;
		
		Serial.print("storing devices: ");
		Serial.println(bridge.tempSensorStore(totalDevices, device), DEC);
		
		error(bridge.getError());
	}
	
	Serial.println("done");
}

void loop()
{
	for (count = 1; count <= totalDevices; count++)
	{
		Serial.print("--------Device ");
		Serial.print(count, DEC);
		Serial.println("--------");
		
		if (bridge.tempSensorLoad(count, device))
		{
			Serial.print("Starting conversion...");
			
			if (bridge.tempConversion(device))
			{
				Serial.println("getting temp...");
				
				if (bridge.tempReadScratchpad(device, scratchpad))
				{
					Serial.print(" temperature C: ");
					printTemp(scratchpad.temp[TEMP_C]);
					
					Serial.print(" temperature F: ");
					printTemp(scratchpad.temp[TEMP_F]);
				}
				else
				{
					Serial.println("failed to get temps");
				}
			}
			else
			{
				Serial.println("conversion failed");
			}
		}
		else
		{
			Serial.println("Did not load");
		}
		
		error(bridge.getError());
		
		Serial.println(" ");
		
		delay(5000);
	}
}

