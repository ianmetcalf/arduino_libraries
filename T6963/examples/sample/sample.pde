#include <T6963.h>

int count;

void setup()
{
  LCD.init();
  
  LCD.moveTo(20, 13);
  LCD.line(3, -3);
  LCD.line(95, 0);
  LCD.line(3, 3);
  LCD.line(0, 95);
  LCD.line(-3, 3);
  LCD.line(-95, 0);
  LCD.line(-3, -3);
  LCD.line(0, -95);
  
  LCD.moveTo(23, 13);
  LCD.line(96, 96);
  LCD.moveTo(23, 108);
  LCD.line(96, -96);
  
  for (count = 0; count < 46; count++)
  {
    LCD.moveTo(25 + count, 12 + count);
    LCD.line(92 - 2 * count, 0);
    
    LCD.moveTo(119 - count, 15 + count);
    LCD.line(0, 92 - 2 * count);
    
    LCD.moveTo(116 - count, 109 - count);
    LCD.line(2 * count - 92, 0);
    
    LCD.moveTo(22 + count, 106 - count);
    LCD.line(0, 2 * count - 92);
  }
  
  LCD.moveTo(180, 50);
  LCD.line(15, 5);
  LCD.line(-15, 5);
  LCD.line(-15, -5);
  LCD.line(15, -5);
  
  LCD.moveTo(180, 80);
  LCD.line(5, 15);
  LCD.line(-5, 15);
  LCD.line(-5, -15);
  LCD.line(5, -15);
  
  LCD.moveTo(180, 20);
  LCD.line(5, 0);
  LCD.move(1, 2);
  LCD.line(0, 5);
  LCD.move(-2, 1);
  LCD.line(-5, 0);
  LCD.move(-1, -2);
  LCD.line(0, -5);
  
  LCD.moveTo(176, 18);
  LCD.rect(12, 12);
  
  LCD.textTo(23, 1);
  LCD.text("Hi, this is text");
}

void loop()
{

}

