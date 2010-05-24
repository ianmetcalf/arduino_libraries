#include <T6963.h>
#include <DS2482.h>

#define RELAY_PORT_INIT  (DDRA |= 0xF0, DDRC |= 0xF0)
#define RelayPort(data)  (PORTC &= 0x0F, PORTA &= 0x0F, PORTC |= data << 4, PORTA |= data & 0xF0)

DS2482 bridge = DS2482(0);

Device device;
Scratch scratchpad;

char strBuffer[32];
char *strErrors[] =
{
  "Timeout      ",
  "Config Error ",
  "Channel Error",
  "Search Error ",
  "No Devices   ",
  "Short Found  ",
  "CRC Mismatch ",
  "Eeprom Full  "
};
char *strRes[] =
{
  " 9bit",
  "10bit",
  "11bit",
  "12bit"
};

int totalDevices;
int count;
byte power;

void showErrors(void)
{
  LCD.textTo(8, 15);
  if (bridge.error_flags == 0)
  {
    LCD.text("No Errors");
  }
  else
  {
    LCD.text("         ");
    LCD.text(-9, 0);
    LCD.text(itoa(bridge.error_flags, strBuffer, 16));
  }
}

void setup()
{
  RELAY_PORT_INIT;
  LCD.init();
  bridge.init();
  
  totalDevices = bridge.tempSensorTotal();
  
  LCD.textTo(1, 1);
  LCD.text("Devices:");
  LCD.text(1, 0);
  LCD.text(itoa(totalDevices, strBuffer, 10));
  
  LCD.textTo(15, 1);
  LCD.text("Varifying Sensors...");
  LCD.textTo(1, 15);
  LCD.text("Error:");
  
  for (count = 1; count <= totalDevices; count++)
  {
    bridge.tempSensorLoad(count, device);
    
    if (bridge.tempSensorVarify(count, device))
    {
      LCD.textTo(2, 2 + count);
      LCD.text("Sensor ");
      LCD.text(itoa(count, strBuffer, 10));
      
      LCD.text(2, 0);
      LCD.text("Ch");
      LCD.text(itoa(device.config.channel, strBuffer, 10));
      
      LCD.text(2, 0);
      LCD.text(strRes[device.config.resolution]);
    }
    else
    {
      LCD.textTo(15, 1);
      LCD.text("Resetting Sensors...");
      bridge.tempSensorReset();
      totalDevices = 0;
    }
    
    showErrors();
  }
  
  LCD.textTo(15, 1);
  LCD.text("Finding Sensors...  ");
  
  while(bridge.tempSensorFind(device, scratchpad))
  {
    if (scratchpad.config != CONFIG_RES_12_BIT)
    {
      scratchpad.config = CONFIG_RES_12_BIT;
      bridge.tempWriteScratchpad(device, scratchpad);
      device.config.resolution = (scratchpad.config CONFIG_RES_SHIFT) & 0x03;
    }
    
    bridge.tempSensorStore(totalDevices + 1, device);
    totalDevices++;
    
    LCD.textTo(2, 2 + totalDevices);
    LCD.text("Sensor ");
    LCD.text(itoa(totalDevices, strBuffer, 10));
    
    LCD.text(2, 0);
    LCD.text("Ch");
    LCD.text(itoa(device.config.channel, strBuffer, 10));
    
    LCD.text(2, 0);
    LCD.text(strRes[device.config.resolution]);
    
    showErrors();
  }
}

void loop()
{
  LCD.textTo(15, 1);
  LCD.text("Getting Temps...    ");
  
  for (count = 1; count <= totalDevices; count++)
  {
    uint16_t frac;
    
    bridge.tempSensorLoad(count, device);
    bridge.tempConversion(device);
    bridge.tempReadScratchpad(device, scratchpad);
    
    frac = (scratchpad.temp[TEMP_F] & 0x0F) * 625;
    
    LCD.textTo(24, 2 + count);
    LCD.text(itoa(scratchpad.temp[TEMP_F]/16, strBuffer, 10));
    LCD.text("      ");
    LCD.text(-6, 0);
    
    if (frac > 0)
    {
      LCD.text(".");
      
      if (frac == 625)
      {
        LCD.text("0625");
      }
      else
      {
        while (frac % 10 == 0)
        {
          frac /= 10;
        }
        
        LCD.text(itoa(frac, strBuffer, 10));
      }
    }
    LCD.text("F");
    
    showErrors();
  }
}













