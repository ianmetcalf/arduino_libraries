#include <T6963.h>
#include <Gui.h>
#include <DS2482.h>
#include <DS18B20.h>

#define RELAY_PORT_INIT    (DDRA |= 0xF0, DDRC |= 0xF0)
#define RelayPort(data)    (PORTC &= 0x0F, PORTA &= 0x0F, PORTC |= data << 4, PORTA |= data & 0xF0)
#define ToggleRelay(data)  (PORTC ^= data << 4, PORTA ^= data & 0xF0)

#define BACKLIGHT_INIT     (DDRD |= (1 << 6))
#define BACKLIGHT_ON       (PORTD |= (1 << 6))
#define BACKLIGHT_OFF      (PORTD &= ~(1 << 6))
#define BACKLIGHT_TOGGLE   (PORTD ^= (1 << 6))

Element home_scheme[] =
{
  {GUI_BUTTON, 3, 5, "Temps "},
  {GUI_BUTTON, 16, 5, "Relays"},
  {GUI_BUTTON, 29, 5, " Draw "},
  {GUI_BUTTON, 27, 12, "Backlight"}
};

Element temp_scheme[] =
{
  {GUI_BUTTON, 29, 12, " Back "}
};

Element relay_scheme[] =
{
  {GUI_BUTTON, 29, 12, " Back "},
  {GUI_BUTTON, 1, 1, "Relay 1"},
  {GUI_BUTTON, 15, 1, "Relay 2"},
  {GUI_BUTTON, 28, 1, "Relay 3"},
  {GUI_BUTTON, 1, 5, "Relay 4"},
  {GUI_BUTTON, 15, 5, "Relay 5"},
  {GUI_BUTTON, 28, 5, "Relay 6"},
  {GUI_BUTTON, 1, 9, "Relay 7"},
  {GUI_BUTTON, 15, 9, "Relay 8"}
};

Element move_scheme[] =
{
  {GUI_BUTTON, 29, 12, " Back  "},
  {GUI_BUTTON, 29, 8, " Clear "},
  {GUI_ARROW, 28, 0, NULL}
};

char strBuffer[32];
char *strRes[] =
{
  " 9bit",
  "10bit",
  "11bit",
  "12bit"
};

void setup()
{
  uint8_t count, row, total;
  Device sensor;
  Scratch scratchpad;
  
  RELAY_PORT_INIT;
  BACKLIGHT_INIT;
  BACKLIGHT_OFF;
  
  LCD.init();
  GUI.init(home_scheme, sizeof(home_scheme)/sizeof(ELEMENT));
  
  ds2482.init(0);
  dsTemp.init();
  /*
  count = 0;
  row = 0;
  
  while (count < 8)
  {
    relay_scheme[count].type = GUI_BUTTON;
    relay_scheme[count].col = (count - row * 3) * 9;
    relay_scheme[count].row = row * 4;
    relay_scheme[count].string = strcat("R ", itoa(count, strBuffer, 10));
    
    count++;
    
    if (count % 3 == 0)
    {
      row++;
    }
  }
  
  relay_scheme[count].type = GUI_BUTTON;
  relay_scheme[count].col = (count - row * 2) * 14;
  relay_scheme[count].row = row * 4;
  relay_scheme[count].string = "Back";
  */
  total = dsTemp.totalSensors();
  
  for (count = 1; count <= total; count++)
  {
    dsTemp.loadSensor(count, sensor);
    
    if (!dsTemp.varifySensor(count, sensor))
    {
      dsTemp.resetSensors();
      total = 0;
    }
    
    RelayPort(ds2482.error_flags);
  }
  
  while (dsTemp.findSensor(sensor, scratchpad))
  {
    if (scratchpad.config != CONFIG_RES_12_BIT)
    {
      scratchpad.config = CONFIG_RES_12_BIT;
      dsTemp.writeScratchpad(sensor, scratchpad);
      sensor.config.resolution = (scratchpad.config CONFIG_RES_SHIFT) & 0x03;
    }
    
    dsTemp.storeSensor(total + 1, sensor);
    total++;
    
    RelayPort(ds2482.error_flags);
  }
  
  dsTemp.polling(1);
  
  GUI.draw();
}

void loop()
{
  if (GUI.getTouch())
  {
    uint8_t press;
    
    while (GUI.drag)
    {
      GUI.getTouch();
    }
    
    press = GUI.checkScheme();
    
    if (press > 0 && press < 4)
    {
      LCD.clearText();
      LCD.clearGraph();
      
      switch (press)
      {
        case 1:
          dsTemp.polling(1);
          tempDraw();
          tempScheme();
          dsTemp.polling(0);
          break;
          
        case 2:
          relayDraw();
          relayScheme();
          break;
          
        case 3:
          moveDraw();
          moveScheme();
          break;
      }
      
      LCD.clearText();
      LCD.clearGraph();
      homeDraw();
    }
    else if (press == 4)
    {
      BACKLIGHT_TOGGLE;
    }
  }
}

void homeDraw(void)
{
  GUI.setScheme(home_scheme, sizeof(home_scheme)/sizeof(ELEMENT));
  GUI.draw();
}

void tempDraw(void)
{
  uint8_t total, count;
  Device sensor;
  
  total = dsTemp.totalSensors();
  
  LCD.textTo(1, 1);
  LCD.text("Devices:");
  LCD.text(1, 0);
  LCD.text(itoa(total, strBuffer, 10));
  
  for (count = 1; count <= total; count++)
  {
    dsTemp.loadSensor(count, sensor);
    
    LCD.textTo(2, 2 + count);
    LCD.text("Sensor ");
    LCD.text(itoa(count, strBuffer, 10));
    
    LCD.text(2, 0);
    LCD.text("Ch");
    LCD.text(itoa(sensor.config.channel, strBuffer, 10));
    
    LCD.text(2, 0);
    LCD.text(strRes[sensor.config.resolution]);
  }
  
  GUI.setScheme(temp_scheme, sizeof(temp_scheme)/sizeof(ELEMENT));
  GUI.draw();
}

void tempScheme(void)
{ 
  uint8_t total, count;
  
  total = dsTemp.totalSensors();
  
  for (;;)
  {
    if (GUI.getTouch())
    {
      uint8_t press;
      
      while (GUI.drag)
      {
        GUI.getTouch();
      }
      
      press = GUI.checkScheme();
      
      if (press == 1)
      {
        return;
      }
    }
    
    if (dsTemp.isr_flags & (1 << ISR_FLAG_NEW_TEMPS))
    {
      dsTemp.isr_flags &= ~(1 << ISR_FLAG_NEW_TEMPS);
      
      for (count = 1; count <= total; count++)
      {
        uint16_t temp, frac;
        
        cli();
        temp = dsTemp.temps[count];
        sei();
        
        LCD.textTo(24, 2 + count);
        LCD.text(itoa(temp/16, strBuffer, 10));
        LCD.text("      ");
        LCD.text(-6, 0);
        
        frac = (temp & 0x0F) * 625;
        
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
        
        if (dsTemp.isr_flags & (TEMP_F << ISR_FLAG_UNITS))
        {
          LCD.text("F");
        }
        else
        {
          LCD.text("C");
        }
      }
    }   
  }
}

void relayDraw(void)
{
  GUI.setScheme(relay_scheme, sizeof(relay_scheme)/sizeof(ELEMENT));
  GUI.draw();
}

void relayScheme(void)
{
  for (;;)
  {
    if (GUI.getTouch())
    {
      uint8_t press;
      
      while (GUI.drag)
      {
        GUI.getTouch();
      }
      
      press = GUI.checkScheme();
      
      if (press == 1)
      {
        return;
      }
      else if (press > 1 && press < 10)
      {
        ToggleRelay(1 << (press - 2));
      }
    }
  }
}
  
void moveDraw(void)
{
  GUI.setScheme(move_scheme, sizeof(move_scheme)/sizeof(ELEMENT));
  GUI.draw();
  
  LCD.textTo(15, 7);
  LCD.text("X");
  LCD.text(-1, 0);
}

void moveScheme(void)
{
  for (;;)
  {
    if (GUI.getTouch())
    {
      uint16_t timeout = 10000;
      
      while (GUI.drag && timeout)
      {
        GUI.getTouch();
        timeout--;
      }
      
      switch (GUI.checkScheme())
      {
        case 2:
          LCD.clearText();
          moveDraw();
          break;
        case 3:
          LCD.text(0, -1);
          LCD.text("X");
          LCD.text(-1, 0);
          break;
        case 4:
          LCD.text(-1, 0);
          LCD.text("X");
          LCD.text(-1, 0);
          break;
        case 5:
          LCD.text(1, 0);
          LCD.text("X");
          LCD.text(-1, 0);
          break;
        case 6:
          LCD.text(0, 1);
          LCD.text("X");
          LCD.text(-1, 0);
          break;
        case 1:
          return;
      }
    }
  }
}

