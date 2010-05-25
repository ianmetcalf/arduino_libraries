#include <T6963.h>
#include <Gui.h>

uint8_t tFlags;
uint8_t relayPort;

uint8_t boxC = 15;
uint8_t boxR = 7;


Element move_scheme[] = 
{
  {GUI_BUTTON, 30, 0, " Save "},
  {GUI_ARROW, 28, 4, NULL},
  {GUI_BUTTON, 30, 12, "Cancel"}
};


void setup()
{
  Serial.begin(9600);
  Serial.flush();
  
  DDRA |= 0xF0;
  DDRC |= 0xF0;
  
  LCD.init();
  
  GUI.init(move_scheme, sizeof(move_scheme)/sizeof(ELEMENT));
  //GUI.calibrate();
  
  GUI.draw();
  
  //relayPort = 0xFF;
  //setRelay();
  
  LCD.textTo(15, 7);
  LCD.text("X");
  LCD.text(-1, 0);
}

void loop()
{
  if (GUI.getTouch())
  {
    uint8_t scheme;
    uint16_t timeout = 10000;
    
    while (GUI.drag && timeout)
    {
      GUI.getTouch();
      timeout--;
    }
    
    scheme = GUI.checkScheme();
    
    if (scheme > 1 && scheme < 6)
    {
      switch (scheme)
      {
        case 2:
          LCD.text(0, -1);
          break;
        case 3:
          LCD.text(-1, 0);
          break;
        case 4:
          LCD.text(1, 0);
          break;
        case 5:
          LCD.text(0, 1);
          break;
      }
      
      LCD.text("X");
      LCD.text(-1, 0);
    }
  }
  
  
  
  /*
  tFlags = GUI.getTouch(0xFF);
  if (tFlags > 0)
  {
    Serial.println(tFlags, BIN);
  }
  
  
  tFlags = GUI.checkTouch();
  
  if (tFlags)
  {
    Serial.print("adcX = ");
    Serial.print(GUI._touchBuffer.x_pos, DEC);
    Serial.print(" , adcY = ");
    Serial.print(GUI._touchBuffer.y_pos, DEC);
    
    Serial.print(" , touchX = ");
    Serial.print(GUI.touchX, DEC);
    Serial.print(" , touchY = ");
    Serial.print(GUI.touchY, DEC);
    Serial.print(" , button : ");
    Serial.print(tFlags, HEX);
    Serial.println("");
  }
  */
}

void setRelay(void)
{
  PORTC &= 0x0F;
  PORTC |= relayPort << 4;
  PORTA &= 0x0F;
  PORTA |= relayPort & 0xF0;
}



