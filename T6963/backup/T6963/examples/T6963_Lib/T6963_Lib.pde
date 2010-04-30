#include "T6963.h"
T6963 LCD(240,128,6,32);

void setup(){
  Serial.begin(9600);
  Serial.print("TH: ");
  Serial.println(LCD.getTH());
  Serial.print("GH: ");
  Serial.println(LCD.getGH());
  
  LCD.Initialize();
  
  Serial.println("Initialized");
  LCD.TextGoTo(0,0);
  LCD.WriteString("Hello World");
  
}
void loop(){
  for(byte _b = 0;_b<240;_b++){
    LCD.writePixel(_b,_b>>1,1);
  }
  for(byte _b=0;_b<128;_b +=8){
    LCD.TextGoTo(_b/8,_b/8);
    LCD.WriteString("Hello World");
  }
  delay(1000);
  LCD.clearGraphic();
  LCD.createLine(0,0,239,127);
  delay(1000);
  LCD.createLine(239,0,0,127);
  LCD.clearGraphic();
  delay(500);
  LCD.clearText();
  LCD.setPixel(0,0);
  LCD.setPixel(239,0);
  LCD.setPixel(239,127);
  LCD.setPixel(0,127);
  delay(1000);
  for(int x=0;x<128;x+=4){
    LCD.createLine(0,x,x<<1,127);
    delay(10);
  }
  delay(1000);
  LCD.createCircle(120,64,32);
  delay(1000);
  LCD.clearGraphic();
  LCD.clearText();
  
  LCD.setDispMode(true,true,true,false);
  for(byte _b=0;_b<8;_b++){
    for(byte _row = 0;_row < LCD.getTextRows();_row++){
      for(byte _col = 0;_col < LCD.getTextCols();_col++){
        LCD.setCursorPointer(_col,_row);
        delay(100);
      }
    }
    LCD.setCursorPattern(_b);
  }
  LCD.setDispMode(true,true,false,false);
}



