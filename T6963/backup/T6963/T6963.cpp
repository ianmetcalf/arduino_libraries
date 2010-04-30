/*-------------------------------------------------------------------------------------------------
0.0- What these 2 guys built that I worked from
      Graphic LCD with Toshiba T6963 controller
      Copyright (c) Rados�aw Kwiecie�, 2007r
      http://en.radzio.dxp.pl/t6963/
      Compiler : avr-gcc
      Modified By -Gil- to work on Arduino easily : http://domoduino.tumblr.com/
0.1- Invocable class T6963
      Commands moved to T6963_commands.h
      For some reason I don't have reset hooked up and all is working fine.
0.2- rbrsidedn
      renamed SetPixel(byte,byte,byte) -> writePixel(x,y,color)
      added setPixel(x,y)
      added clearPixel(x,y)
      added createline(x1,y1,x2,y2)
      added createCircle(x,y,radius)       
r6 - Checked in with SVN
r7 - Checked in with cursor controls added
r8 - Got 6bit font width (s/b any fonth width) working.
-----------------------------------------------------------------------------------------------*/
#include "WProgram.h"
#include "T6963.h"
#include "T6963_Commands.h"
//-------------------------------------------------------------------------------------------------
//
// Delay function
//	
//-------------------------------------------------------------------------------------------------
void n_delay(void)
{
  volatile byte i;
  for(i = 0; i < (F_CPU/1000000); i++)
  {
    asm("nop");
  }
}
//-------------------------------------------------------------------------------------------------
//
// Constructor
//	
//-------------------------------------------------------------------------------------------------

T6963::T6963(int pixHoriz,int pixVert,int fontWidth, int sizeMem){
  GLCD_NUMBER_OF_LINES = pixVert;
  GLCD_PIXELS_PER_LINE = pixHoriz;
  _FW = fontWidth;
  _GA = pixHoriz / fontWidth;	//Supercedes GLCD_GRAPHIC_AREA
  _TA = pixHoriz / fontWidth;	//Supercedes GLCD_TEXT_AREA
  _sizeMem = constrain(sizeMem,0,64); //size of attached memory in kb.
  sizeGA = _GA*pixVert;		//Supercedes GLCD_GRAPHIC_SIZE
  sizeTA = _TA*pixVert/8;	        //Supercedes GLCD_TEXT_SIZE
  setTH(0);
  setGH(0);
}
void T6963::setTH(unsigned int addr){
  _TH=addr;
} //expose _TH

unsigned int T6963::getTH(){
  return _TH;
}	//return _TH

void T6963::setGH(unsigned int addr){
  if(addr == _TH){
    _GH=_TH+sizeTA;
  }
  else{
    _GH=addr;
  }
}
unsigned int T6963::getGH(){
  return _GH;
}

byte T6963::setCursorPattern(byte _b){
  byte tmp = T6963_CURSOR_PATTERN_SELECT;
  _b=constrain(_b,0,7);
  tmp|=_b;
  writeCommand(tmp);
  return tmp;
}

byte T6963::setCursorPointer(byte _col,byte _row){
  _col=constrain(_col,0,(_TA-1));
  _row=constrain(_row,0,((GLCD_NUMBER_OF_LINES/8)-1));
  writeData(_col);
  writeData(_row);
  writeCommand(0x21); //Cursor pointer Set
}

byte T6963::getTextRows(){
  return (GLCD_NUMBER_OF_LINES/8);
}
byte T6963::getTextCols(){
  return _TA;
}
//-------------------------------------------------------------------------------------------------
//
// Ports intalization
//
//-------------------------------------------------------------------------------------------------
void T6963::InitalizeInterface(void){
  //GLCD_DATA_DDR = 0xFF;
  GLCD_DATA_DDR1 |= GLCD_DATA_MASK1;
  GLCD_DATA_DDR2 |= GLCD_DATA_MASK2;

  GLCD_CTRL_DDR = ((1 << GLCD_WR) | (1 << GLCD_RD) | (1 << GLCD_CE) | (1 << GLCD_CD));
  GLCD_CTRL_PORT |= ((1 << GLCD_WR) | (1 << GLCD_RD) | (1 << GLCD_CE) | (1 << GLCD_CD));
}
//-------------------------------------------------------------------------------------------------
//
// Reads dispay status
//
//-------------------------------------------------------------------------------------------------
byte T6963::checkStatus(void){
  uint8_t tmp;
  //GLCD_DATA_DDR = 0x00;
  GLCD_DATA_DDR1 &= ~GLCD_DATA_MASK1;
  GLCD_DATA_DDR2 &= ~GLCD_DATA_MASK2;

  GLCD_CTRL_PORT &= ~((1 << GLCD_RD) | (1 << GLCD_CE));
  n_delay();
  //tmp = GLCD_DATA_PIN;
  tmp = (GLCD_DATA_PIN1 GLCD_DATA_RSHIFT1) | (GLCD_DATA_PIN2 GLCD_DATA_RSHIFT2);
  //GLCD_DATA_DDR = 0xFF;
  GLCD_DATA_DDR1 |= GLCD_DATA_MASK1;
  GLCD_DATA_DDR2 |= GLCD_DATA_MASK2;
  GLCD_CTRL_PORT |= ((1 << GLCD_RD) | (1 << GLCD_CE));
  return tmp;
}
//-------------------------------------------------------------------------------------------------
//
// Writes instruction 
//
//-------------------------------------------------------------------------------------------------
void T6963::writeCommand(byte command){
  while(!(checkStatus()&0x03));
  // GLCD_DATA_PORT = command;
  GLCD_DATA_PORT1 &= ~GLCD_DATA_MASK1;
  GLCD_DATA_PORT1 |= (command GLCD_DATA_SHIFT1);
  GLCD_DATA_PORT2 &= ~GLCD_DATA_MASK2;
  GLCD_DATA_PORT2 |= (command GLCD_DATA_SHIFT2);

  GLCD_CTRL_PORT &= ~((1 << GLCD_WR) | (1 << GLCD_CE));
  n_delay();
  GLCD_CTRL_PORT |= ((1 << GLCD_WR) | (1 << GLCD_CE));
}

//-------------------------------------------------------------------------------------------------
//
// Writes data
//
//-------------------------------------------------------------------------------------------------
void T6963::writeData(byte data){
  while(!(checkStatus()&0x03));
  // GLCD_DATA_PORT = data;
  GLCD_DATA_PORT1 &= ~GLCD_DATA_MASK1;
  GLCD_DATA_PORT1 |= (data GLCD_DATA_SHIFT1);
  GLCD_DATA_PORT2 &= ~GLCD_DATA_MASK2;
  GLCD_DATA_PORT2 |= (data GLCD_DATA_SHIFT2);

  GLCD_CTRL_PORT &= ~((1 << GLCD_WR) | (1 << GLCD_CE) | (1 << GLCD_CD));
  n_delay();
  GLCD_CTRL_PORT |= ((1 << GLCD_WR) | (1 << GLCD_CE) | (1 << GLCD_CD));
}
//-------------------------------------------------------------------------------------------------
//
// Reads data
//
//-------------------------------------------------------------------------------------------------
byte T6963::ReadData(void){
  byte tmp;
  while(!(checkStatus()&0x03));
  //GLCD_DATA_DDR = 0x00;
  GLCD_DATA_DDR1 &= ~GLCD_DATA_MASK1;
  GLCD_DATA_DDR2 &= ~GLCD_DATA_MASK2;

  GLCD_CTRL_PORT &= ~((1 << GLCD_RD) | (1 << GLCD_CE) | (1 << GLCD_CD));
  n_delay();

  //tmp = GLCD_DATA_PIN;

  tmp = (GLCD_DATA_PIN1 GLCD_DATA_RSHIFT1) | (GLCD_DATA_PIN2 GLCD_DATA_RSHIFT2);
  GLCD_CTRL_PORT |= ((1 << GLCD_RD) | (1 << GLCD_CE) | (1 << GLCD_CD));

  //GLCD_DATA_DDR = 0xFF;

  GLCD_DATA_DDR1 |= GLCD_DATA_MASK1;
  GLCD_DATA_DDR2 |= GLCD_DATA_MASK2;
  return tmp;
}
//-------------------------------------------------------------------------------------------------
//
// Sets address pointer for display RAM memory
//
//-------------------------------------------------------------------------------------------------
void T6963::SetAddressPointer(unsigned int address){
  writeData(address & 0xFF);
  writeData(address >> 8);
  writeCommand(T6963_SET_ADDRESS_POINTER);
}
//-------------------------------------------------------------------------------------------------
//
// Clears text area of display RAM memory
//
//-------------------------------------------------------------------------------------------------
void T6963::clearText(){
  SetAddressPointer(_TH);
  for(int i = 0; i < sizeTA; i++){
    WriteDisplayData(0);
  }
}
//-------------------------------------------------------------------------------------------------
// Clears characters generator area of display RAM memory
//-------------------------------------------------------------------------------------------------
void T6963::clearCG(){
  unsigned int i=((_sizeMem/2)-1)*0x800;
  SetAddressPointer(i);
  for(i = 0; i < 256 * 8; i++){
    WriteDisplayData(0);
  }
}
//-------------------------------------------------------------------------------------------------
// Clears graphics area of display RAM memory
//-------------------------------------------------------------------------------------------------
void T6963::clearGraphic(){
  SetAddressPointer(_GH);
  for(unsigned int i = 0; i < sizeGA; i++){
    WriteDisplayData(0x00);
  }
}
//-------------------------------------------------------------------------------------------------
// Writes a single character (ASCII code) to display RAM memory
//-------------------------------------------------------------------------------------------------
void T6963::WriteChar(char charCode)
{
  WriteDisplayData(charCode - 32);
}
//-------------------------------------------------------------------------------------------------
// Writes null-terminated string to display RAM memory
//-------------------------------------------------------------------------------------------------
void T6963::WriteString(char * string){
  while(*string){
    WriteChar(*string++);
  }
}
//-------------------------------------------------------------------------------------------------
// Writes null-terminated string from program memory to display RAM memory
//-------------------------------------------------------------------------------------------------
void T6963::WriteStringPgm(prog_char * string){
  char ch;
  while((ch = pgm_read_byte(string++))){
    WriteChar(ch);
  }
}
//-------------------------------------------------------------------------------------------------
// Sets display coordinates
//-------------------------------------------------------------------------------------------------
void T6963::TextGoTo(unsigned char x, unsigned char y){
  unsigned int address;
  address = _TH +  x + (_TA * y);
  SetAddressPointer(address);
}
//-------------------------------------------------------------------------------------------------
// Writes single char pattern to character generator area of display RAM memory
//-------------------------------------------------------------------------------------------------
void T6963::DefineCharacter(byte charCode, unsigned char * defChar){
  unsigned int address=((_sizeMem/2)-1)*0x800;
  SetAddressPointer(address);
  for(byte i = 0; i < 8 ; i++){
    WriteDisplayData(*(defChar + i));
  }
}

//-------------------------------------------------------------------------------------------------
// Set (if color==1) or clear (if color==0) pixel on screen
//-------------------------------------------------------------------------------------------------
void T6963::writePixel(byte x, byte y, byte color){
  unsigned char tmp;
  unsigned int address;
  address = _GH + (x / _FW) + (_GA * y);
  SetAddressPointer(address);
  writeCommand(T6963_DATA_READ_AND_NONVARIABLE);
  tmp = ReadData();
  if(color){
    tmp |= (1 <<  (_FW - 1 - (x % _FW)));
  }
  else{
    tmp &= ~(1 <<  (_FW - 1 - (x % _FW)));
  }
  WriteDisplayData(tmp);
}
//-------------------------------------------------------------------------------------------------
// Set a single pixel at x,y (in pixels) to 1 (on)
//-------------------------------------------------------------------------------------------------
byte T6963::setPixel(byte x, byte y){
  SetAddressPointer((_GH + (x / _FW) + (_GA * y)));
  byte tmp=B11111000;
  tmp |= (_FW-1)-(x%_FW); //LSB Direction Correction
  writeCommand(tmp);
  return tmp;
}
//-------------------------------------------------------------------------------------------------
// Set a single pixel at x,y (in pixels) to 0 (off)
//-------------------------------------------------------------------------------------------------
byte T6963::clearPixel(byte x, byte y){
  SetAddressPointer((_GH + (x / _FW) + (_GA * y)));
  byte tmp=B11110000;
  tmp |= (_FW - 1) - (x % _FW); //LSB Direction Correction
  writeCommand(tmp);
  return tmp;
} 
//-------------------------------------------------------------------------------------------------
// Writes display data and increment address pointer
//-------------------------------------------------------------------------------------------------
void T6963::WriteDisplayData(byte x){
  writeData(x);
  writeCommand(T6963_DATA_WRITE_AND_INCREMENT);
}
//-------------------------------------------------------------------------------------------------
// Sets graphics coordinates
//-------------------------------------------------------------------------------------------------
void T6963::GraphicGoTo(byte x, byte y){
  unsigned int address;
  address = _GH + (x / _FW) + (_GA * y);
  SetAddressPointer(address);
}
//-------------------------------------------------------------------------------------------------
// Displays bitmap from program memory
//-------------------------------------------------------------------------------------------------
/*
void T6963::Bitmap(unsigned char * bitmap, byte x, byte y, byte width, unsigned char height){
 unsigned char i, j;
 for(j = 0; j < height; j++){
 GraphicGoTo(x, y + j);
 for(i = 0; i < width/_FW; i++){
 WriteDisplayData(pgm_read_byte(bitmap + i + (_GA * j)));
 }
 }
 }
 */
byte T6963::setMode(char _mode, char _CG){
  byte tmp = T6963_MODE_SET;
  if(_mode=='X' || _mode =='x' || _mode=='^'){
    tmp |= 1;
  }
  else if(_mode == 'A' || _mode=='&' ||_mode=='a'){
    tmp |= 3;
  }
  else if(_mode == 'T'||_mode=='t'){
    tmp |=4;
  }
  else{
    tmp |= 0; //OR mode default
  }
  if(_CG =='E'||_CG=='e'){
    tmp |=8;
  }
  else{
    tmp |=0;
  }
  writeCommand(tmp);
  return tmp;
}

byte T6963::clearDispMode(){
  writeCommand(T6963_DISPLAY_MODE);
}
byte T6963::setDispMode(boolean _text,boolean _graphics, boolean _cursor, boolean _blink){
  byte tmp=T6963_DISPLAY_MODE;
  if(_graphics){
    tmp |= B1000; //T6963_GRAPHIC_DISPLAY_ON
  }
  if(_text){
    tmp |= B0100; //T6963_TEXT_DISPLAY_ON
  }
  if(_cursor){
    tmp |= B0010; //T6963_CURSOR_DISPLAY_ON
  }
  if(_blink){
    tmp |=B0001; //T6963_CURSOR_BLINK_ON
  }
  writeCommand(tmp);
  return tmp;
}
/*------------------------------------------------------------------------
Geometric shapes, scavenged from online resources.
why re-invent the wheel
*/

void T6963::createLine(int x0,int y0,int x1,int y1){
/*BreshenhamLine algorithm - From wikipedia so it must be right
http://en.wikipedia.org/wiki/Bresenham's_line_algorithm
*/
  boolean steep = abs(y1 - y0) > abs(x1 - x0);
  if(steep){
    //swap(x0, y0)
    //swap(x1, y1)
    int tmp=x0;
    x0=y0;
    y0=tmp;
    tmp=x1;
    x1=y1;
    y1=tmp;
  }
  if(x0 > x1){
    //swap(x0, x1)
    //swap(y0, y1)
    int tmp=x0;
    x0=x1;
    x1=tmp;
    tmp=y0;
    y0=y1;
    y1=tmp;
  }
  int deltax = x1 - x0;
  int deltay = abs(y1 - y0);
  int error = deltax / 2;
  int ystep=-1;
  int y = y0;
  if(y0 < y1){ystep= 1;}
  for(int x =x0;x<=x1;x++){
    if(steep){setPixel(y,x);}else{setPixel(x,y);}
    error = error - deltay;
    if(error < 0){
      y = y + ystep;
      error = error + deltax;
    }
  }
}
/*------------------------------------------------------------------------------------------------
Circles: http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
------------------------------------------------------------------------------------------------*/

void T6963::createCircle(int cx, int cy, int radius) //cx and cy mark the distance from the origin point.
{
	int error = -radius;
	int x = radius;
	int y = 0;
 
	while (x >= y){
		plot8points(cx, cy, x, y);
 
		error += y;
		++y;
		error += y;
 
		if (error >= 0){
			--x;
			error -= x;
			error -= x;
		}
	}
}
 
void T6963::plot8points(int cx, int cy, int x, int y)
{
	plot4points(cx, cy, x, y);
	if (x != y) plot4points(cx, cy, y, x);
}
 
void T6963::plot4points(int cx, int cy, int x, int y)
{
	setPixel(cx + x, cy + y);
	if (x != 0) setPixel(cx - x, cy + y);
	if (y != 0) setPixel(cx + x, cy - y);
	if (x != 0 && y != 0) setPixel(cx - x, cy - y);
}



//-------------------------------------------------------------------------------------------------
//
// Display initalization
//
//-------------------------------------------------------------------------------------------------
void T6963::Initialize(void){
  //Set up data and control ports
  InitalizeInterface();

  //reset LCD, should be able to have hardware solution
  //GLCD_CTRL_PORT &= ~(1 << GLCD_RESET);
  //_delay_ms(1);
  //GLCD_CTRL_PORT |= (1 << GLCD_RESET);

  //Set Graphics Home Address
  writeData(_GH & 0xFF);
  writeData(_GH >> 8);
  writeCommand(T6963_SET_GRAPHIC_HOME_ADDRESS);

  //Set Graphics Area
  writeData(_GA);
  writeData(0x00);
  writeCommand(T6963_SET_GRAPHIC_AREA);

  //Set Text home address
  writeData(_TH & 0xFF);
  writeData(_TH >> 8);
  writeCommand(T6963_SET_TEXT_HOME_ADDRESS);

  //Set Text Area
  writeData(_TA);
  writeData(0x00);
  writeCommand(T6963_SET_TEXT_AREA);

  //Set Internal CGRAM address
  writeData(((_sizeMem/2)-1));
  writeData(0x00);
  writeCommand(T6963_SET_OFFSET_REGISTER);


  setDispMode(true,true,false,false);

  setMode('O','I');

  clearText();
  clearGraphic();
  delay(100);

}
