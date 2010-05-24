#include <T6963.h>
#include <Touch.h>
#include <PCF8575.h>

PCF8575 xport = PCF8575(0);

int count = 8;

void touch_calibrate(void)
{
  byte press;
  
  POINT screenSample[3];
  POINT displaySample[3] = {{24,32},{115,100},{190,47}};
  POINT capturePoint;
  
  Serial.println("calibrate");
  for (byte i=0; i<3; i++)
  {
    LCD.clear();
    LCD.moveTo(displaySample[i].x - 5, displaySample[i].y - 5);
    LCD.rect(10, 10);
    //LCD.createCircle(displaySample[i].x, displaySample[i].y, 4);
    
    press = 0;
    touch.screenPoint.x = 0;
    touch.screenPoint.y = 0;
    while (!press)
    {
      capturePoint.x = touch.screenPoint.x;
      capturePoint.y = touch.screenPoint.y;
      touch.sampling(&touch.screenPoint, 50);
      if (touch.screenPoint.x > 0)
      {
        if((capturePoint.x == touch.screenPoint.x) && (capturePoint.y == touch.screenPoint.y))
        {
          press = 1;
        }
      }
    }
    screenSample[i].x = touch.screenPoint.x;
    screenSample[i].y = touch.screenPoint.y;
    
    delay(500);
  }
  
  Serial.println("calibrating");
  LCD.clear();
  touch.calibrate(&displaySample[0], &screenSample[0]);
  Serial.println("calibrated");
}

void showPoint(void)
{
  byte tx, ty;
  
  touch.readPoint();
  if (touch.screenPoint.x > 0)
  {
    tx = touch.displayPoint.x;
    ty = touch.displayPoint.y;
    
    LCD.textTo((tx / 6) - 2, (ty - 4) / 8);
    LCD.text("Relay");
    
    xport.setPin(count);
    if (++count > 15) count = 8;
    
    Serial.print("X: ");
    Serial.print(touch.screenPoint.x);
    Serial.print(", ");
    Serial.print(touch.displayPoint.x);
    Serial.print("; Y: ");
    Serial.print(touch.screenPoint.y);
    Serial.print(", ");
    Serial.println(touch.displayPoint.y);
  }
}

void setup()
{
  Serial.begin(9600);
  Serial.flush();
  
  LCD.init();
  touch.init();
  xport.init();
  
  touch_calibrate();
}

void loop()
{
  showPoint();
}




