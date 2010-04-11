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

#define FONT_WIDTH 6

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

T6963::T6963(uint16_t pixHoriz, uint16_t pixVert, uint16_t sizeMem)
{
  GLCD_NUMBER_OF_LINES = pixVert;
  GLCD_PIXELS_PER_LINE = pixHoriz;
  
  _GA = pixHoriz / FONT_WIDTH; //Supercedes GLCD_GRAPHIC_AREA
  _TA = pixHoriz / FONT_WIDTH; //Supercedes GLCD_TEXT_AREA
  //_FW = FONT_WIDTH;
  
  _sizeMem = constrain(sizeMem, 0, 64); //size of attached memory in kb.
  sizeGA = _GA * pixVert;		//Supercedes GLCD_GRAPHIC_SIZE
  sizeTA = (_TA * pixVert) >> 3;	        //Supercedes GLCD_TEXT_SIZE
  setTH(0);
  setGH(0);
}

void T6963::setTH(uint16_t addr)
{
  _TH=addr;
} //expose _TH

void T6963::setGH(uint16_t addr)
{
  if(addr == _TH){
    _GH = _TH+sizeTA;
  }
  else{
    _GH = addr;
  }
}

uint16_t T6963::getTH(void)
{
  return _TH;
}	//return _TH

uint16_t T6963::getGH(void)
{
  return _GH;
}

uint16_t T6963::getTA(void)
{
  return _TA;
}

uint16_t T6963::getGA(void)
{
  return _GA;
}

//-------------------------------------------------------------------------------------------------
//
// Ports intalization
//
//-------------------------------------------------------------------------------------------------

void T6963::initalizePorts(void)
{
  GLCD_DATA_DDR1 |= GLCD_DATA_MASK1;
  GLCD_DATA_DDR2 |= GLCD_DATA_MASK2;

  GLCD_CTRL_DDR |= ((1 << GLCD_RD) | (1 << GLCD_CE) | (1 << GLCD_CD));
  GLCD_CTRL_PORT |= ((1 << GLCD_RD) | (1 << GLCD_CE) | (1 << GLCD_CD));
}

//-------------------------------------------------------------------------------------------------
//
// Reads dispay status
//
//-------------------------------------------------------------------------------------------------

uint8_t T6963::checkStatus(void)
{
  uint8_t tmp;
  
  GLCD_DATA_DDR1 &= ~GLCD_DATA_MASK1;
  GLCD_DATA_DDR2 &= ~GLCD_DATA_MASK2;

  GLCD_CTRL_PORT &= ~((1 << GLCD_RD) | (1 << GLCD_CE));
  n_delay();
  tmp = (GLCD_DATA_PIN1 GLCD_DATA_RSHIFT1) | (GLCD_DATA_PIN2 GLCD_DATA_RSHIFT2);
  
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

void T6963::writeCommand(uint8_t command)
{
  while(!(checkStatus() & 0x03));
  
  GLCD_DATA_PORT1 &= ~GLCD_DATA_MASK1;
  GLCD_DATA_PORT1 |= (command GLCD_DATA_SHIFT1);
  GLCD_DATA_PORT2 &= ~GLCD_DATA_MASK2;
  GLCD_DATA_PORT2 |= (command GLCD_DATA_SHIFT2);

  GLCD_CTRL_PORT &= ~(1 << GLCD_CE);
  n_delay();
  GLCD_CTRL_PORT |= (1 << GLCD_CE);
}

//-------------------------------------------------------------------------------------------------
//
// Writes data
//
//-------------------------------------------------------------------------------------------------

void T6963::writeData(uint8_t data)
{
  while(!(checkStatus() & 0x03));
  
  GLCD_DATA_PORT1 &= ~GLCD_DATA_MASK1;
  GLCD_DATA_PORT1 |= (data GLCD_DATA_SHIFT1);
  GLCD_DATA_PORT2 &= ~GLCD_DATA_MASK2;
  GLCD_DATA_PORT2 |= (data GLCD_DATA_SHIFT2);

  GLCD_CTRL_PORT &= ~((1 << GLCD_CE) | (1 << GLCD_CD));
  n_delay();
  GLCD_CTRL_PORT |= ((1 << GLCD_CE) | (1 << GLCD_CD));
}

//-------------------------------------------------------------------------------------------------
//
// Reads data
//
//-------------------------------------------------------------------------------------------------

uint8_t T6963::readData(void)
{
  uint8_t tmp;
  
  while(!(checkStatus() & 0x03));
  
  GLCD_DATA_DDR1 &= ~GLCD_DATA_MASK1;
  GLCD_DATA_DDR2 &= ~GLCD_DATA_MASK2;
  
  GLCD_CTRL_PORT &= ~((1 << GLCD_RD) | (1 << GLCD_CE) | (1 << GLCD_CD));
  n_delay();
  tmp = ((GLCD_DATA_PIN1 & GLCD_DATA_MASK1) GLCD_DATA_RSHIFT1);
  tmp |= ((GLCD_DATA_PIN2 & GLCD_DATA_MASK2) GLCD_DATA_RSHIFT2);
  
  GLCD_CTRL_PORT |= ((1 << GLCD_RD) | (1 << GLCD_CE) | (1 << GLCD_CD));
  GLCD_DATA_DDR1 |= GLCD_DATA_MASK1;
  GLCD_DATA_DDR2 |= GLCD_DATA_MASK2;
  return tmp;
}

//-------------------------------------------------------------------------------------------------
//
// Set Display Mode
//
//-------------------------------------------------------------------------------------------------

uint8_t T6963::setMode(uint8_t _mode, uint8_t _CG)
{
  uint8_t tmp = T6963_MODE_SET;
  
  tmp |= _CG | _mode;
  
  writeCommand(tmp);
  return tmp;
}

uint8_t T6963::setDispMode(uint8_t settings)
{
  uint8_t tmp = T6963_DISPLAY_MODE | settings;
  
  writeCommand(tmp);
  return tmp;
}

uint8_t T6963::clearDispMode(void)
{
  writeCommand(T6963_DISPLAY_MODE);
}

//-------------------------------------------------------------------------------------------------
//
// Sets address pointer for display RAM memory
//
//-------------------------------------------------------------------------------------------------

void T6963::setAddressPointer(uint16_t address)
{
  writeData(address & 0xFF);
  writeData(address >> 8);
  writeCommand(T6963_SET_ADDRESS_POINTER);
}

void T6963::setCursorPointer(uint8_t _col, uint8_t _row)
{
  _col = constrain(_col, 0, (_TA - 1));
  _row = constrain(_row, 0, ((GLCD_NUMBER_OF_LINES>>3) - 1));
  writeData(_col);
  writeData(_row);
  writeCommand(T6963_SET_CURSOR_POINTER);
}

uint8_t T6963::setCursorPattern(uint8_t _b)
{
  uint8_t tmp = T6963_CURSOR_PATTERN_SELECT;
  
  _b = constrain(_b, 0, 7);
  tmp |= _b;
  
  writeCommand(tmp);
  return tmp;
}

uint8_t T6963::getTextRows(void)
{
  return (GLCD_NUMBER_OF_LINES>>3);
}

uint8_t T6963::getTextCols(void)
{
  return _TA;
}

//-------------------------------------------------------------------------------------------------
//
// Sets coordinates
//
//-------------------------------------------------------------------------------------------------

void T6963::graphicGoTo(uint8_t x, uint8_t y)
{
  uint16_t address = _GH + (x / FONT_WIDTH) + (_GA * y);
  setAddressPointer(address);
}

void T6963::textGoTo(uint8_t x, uint8_t y)
{
  uint16_t address = _TH +  x + (_TA * y);
  setAddressPointer(address);
}

//-------------------------------------------------------------------------------------------------
//
// Writes display data and increment address pointer
//
//-------------------------------------------------------------------------------------------------

void T6963::writeDispData(uint8_t data)
{
  writeData(data);
  writeCommand(T6963_DATA_WRITE_AND_INCREMENT);
}

//-------------------------------------------------------------------------------------------------
//
// Read display data
//
//-------------------------------------------------------------------------------------------------

uint8_t T6963::readDispData(void)
{
  uint8_t tmp;
  
  writeCommand(T6963_DATA_READ_AND_NONVARIABLE);
  tmp = readData();
  
  return tmp;
}

//-------------------------------------------------------------------------------------------------
//
// Write a block of data with or without a mask
//
//-------------------------------------------------------------------------------------------------

void T6963::writeBlock(uint8_t length, uint8_t data)
{
  while (length > 0)
  {
    writeDispData(data);
	length--;
  }
}

void T6963::writeBlockMask(uint8_t length, uint8_t mask)
{
  uint8_t tmp;
  
  while (length > 0)
  {
    tmp = readDispData();
    tmp |= mask;
    writeDispData(tmp);
    length--;
  }
}

void T6963::writeBlockMask(uint8_t length, uint8_t data, uint8_t mask)
{
  uint8_t tmp;
  
  while (length > 0)
  {
    tmp = readDispData();
    tmp = (data & mask) | (tmp & ~mask);
    writeDispData(tmp);
    length--;
  }
}

void T6963::writeLine(uint8_t width, uint8_t left, uint8_t left_mask, uint8_t fill, uint8_t rght, uint8_t rght_mask)
{
  writeBlockMask(1, left, left_mask);
  writeBlock(width - 2, fill);
  writeBlockMask(1, rght, rght_mask);
}

//-------------------------------------------------------------------------------------------------
//
// Writes a single character (ASCII code) to display RAM memory
//
//-------------------------------------------------------------------------------------------------

void T6963::writeChar(char charCode)
{
  writeDispData(charCode - 32);
}

//-------------------------------------------------------------------------------------------------
//
// Writes null-terminated string to display RAM memory
//
//-------------------------------------------------------------------------------------------------

void T6963::writeString(char * string)
{
  while(*string)
  {
    writeChar(*string++);
  }
}
//-------------------------------------------------------------------------------------------------
//
// Writes null-terminated string from program memory to display RAM memory
//
//-------------------------------------------------------------------------------------------------

void T6963::writeStringPgm(prog_char * string)
{
  char ch;
  while((ch = pgm_read_byte(string++)))
  {
    writeChar(ch);
  }
}

//-------------------------------------------------------------------------------------------------
//
// Writes single char pattern to character generator area of display RAM memory
//
//-------------------------------------------------------------------------------------------------

void T6963::defineCharacter(uint8_t charCode, uint8_t * defChar)
{
  uint16_t address = ((_sizeMem / 2) - 1) * 0x800;
  setAddressPointer(address);
  
  uint8_t i;
  for(i = 0; i < 8 ; i++)
  {
    writeDispData(*(defChar + i));
  }
}

//-------------------------------------------------------------------------------------------------
//
// Set a single pixel at x,y (in pixels) to 0 (off)
//
//-------------------------------------------------------------------------------------------------

uint8_t T6963::clearPixel(uint8_t x, uint8_t y)
{
  uint8_t tmp = T6963_BIT_RESET;
  
  tmp |= (FONT_WIDTH - 1) - (x % FONT_WIDTH); //LSB Direction Correction
  setAddressPointer((_GH + (x / FONT_WIDTH) + (_GA * y)));
  
  writeCommand(tmp);
  return tmp;
}

//-------------------------------------------------------------------------------------------------
//
// Set a single pixel at x,y (in pixels) to 1 (on)
//
//-------------------------------------------------------------------------------------------------

uint8_t T6963::setPixel(uint8_t x, uint8_t y)
{
  uint8_t tmp = T6963_BIT_SET;
  
  tmp |= (FONT_WIDTH - 1) - (x % FONT_WIDTH); //LSB Direction Correction
  setAddressPointer((_GH + (x / FONT_WIDTH) + (_GA * y)));
  
  writeCommand(tmp);
  return tmp;
}

//-------------------------------------------------------------------------------------------------
//
// Drawing functions
//
//-------------------------------------------------------------------------------------------------

void T6963::fillBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t * pattern, uint8_t size)
{
  uint8_t col_left = x / FONT_WIDTH;
  uint8_t col_rght = (x + w) / FONT_WIDTH;
  uint8_t cols = col_rght - col_left - 1;
  
  uint8_t mask_left = (1 << (((col_left + 1) * FONT_WIDTH) - x)) - 1;
  uint8_t mask_rght = ~((1 << (((col_rght + 1) * FONT_WIDTH) - x - w)) - 1);
  
  uint8_t i, p;
  for (i = 0; i < h; i++)
  {
    p = pattern[i % size];
    
    setAddressPointer(_GH + (_GA * (y + i)) + col_left);
    writeBlockMask(1, p, mask_left);
    writeBlock(cols, p);
    writeBlockMask(1, p, mask_rght);
  }
}

void T6963::clearBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
  uint8_t pattern[1] = {0};
  fillBox(x, y, w, h, pattern, 1);
}

void T6963::setBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
  uint8_t pattern[1] = {0xFF};
  fillBox(x, y, w, h, pattern, 1);
}

//-------------------------------------------------------------------------------------------------
//
// Clears graphics area of display RAM memory
//
//-------------------------------------------------------------------------------------------------

void T6963::clearGraphic(void)
{
  uint16_t i;
  
  setAddressPointer(_GH);
  
  for(i = 0; i < sizeGA; i++)
  {
    writeDispData(0);
  }
}


//-------------------------------------------------------------------------------------------------
//
// Clears text area of display RAM memory
//
//-------------------------------------------------------------------------------------------------

void T6963::clearText(void)
{
  uint16_t i;
  
  setAddressPointer(_TH);
  
  for(i = 0; i < sizeTA; i++)
  {
    writeDispData(0);
  }
}

//-------------------------------------------------------------------------------------------------
//
// Clears characters generator area of display RAM memory
//
//-------------------------------------------------------------------------------------------------

void T6963::clearCG(void)
{
  uint16_t i;
  uint16_t address = ((_sizeMem / 2) - 1) * 0x800;
  
  setAddressPointer(address);
  
  for(i = 0; i < 256 * 8; i++)
  {
    writeDispData(0);
  }
}

//-------------------------------------------------------------------------------------------------
//
// Display initalization
//
//-------------------------------------------------------------------------------------------------
void T6963::init(void)
{
  //Set up data and control ports
  initalizePorts();

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
  writeData(((_sizeMem / 2) - 1));
  writeData(0x00);
  writeCommand(T6963_SET_OFFSET_REGISTER);
  
  setDispMode((1<<T6963_DISPLAY_TEXT) | (1<<T6963_DISPLAY_GRAPHIC) | (0<<T6963_DISPLAY_CURSOR) | (0<<T6963_DISPLAY_BLINK));
  
  setMode(T6963_MODE_XOR, T6963_MODE_INTERNAL);
  
  clearGraphic();
  clearText();
  clearCG();
  delay(100);

}














/*
//-------------------------------------------------------------------------------------------------
//
// Set (if color==1) or clear (if color==0) pixel on screen
//
//-------------------------------------------------------------------------------------------------

void T6963::writePixel(byte x, byte y, byte color)
{
  unsigned char tmp;
  unsigned int address;
  address = _GH + (x / FONT_WIDTH) + (_GA * y);
  SetAddressPointer(address);
  writeCommand(T6963_DATA_READ_AND_NONVARIABLE);
  tmp = ReadData();
  if(color){
    tmp |= (1 <<  (FONT_WIDTH - 1 - (x % FONT_WIDTH)));
  }
  else{
    tmp &= ~(1 <<  (FONT_WIDTH - 1 - (x % FONT_WIDTH)));
  }
  WriteDispData(tmp);
}

//-------------------------------------------------------------------------------------------------
//
// Displays bitmap from program memory
//
//-------------------------------------------------------------------------------------------------

void T6963::Bitmap(uint8_t * bitmap, uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
  uint8_t i, j;
  
  for(j = 0; j < height; j++)
  {
    graphicGoTo(x, y + j);
    for(i = 0; i < width/FONT_WIDTH; i++)
    {
       writeDispData(pgm_read_byte(bitmap + i + (_GA * j)));
    }
  }
}

//------------------------------------------------------------------------
//Geometric shapes, scavenged from online resources.
//why re-invent the wheel

//BreshenhamLine algorithm - From wikipedia so it must be right
//http://en.wikipedia.org/wiki/Bresenham's_line_algorithm

void T6963::createLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
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

//------------------------------------------------------------------------------------------------
//Circles: http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
//------------------------------------------------------------------------------------------------

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

void T6963::createBox(int cx, int cy, int w, int h)
{
  int w2 = w >> 1;
  int h2 = h >> 1;
  int x = 0;
  int y = h2;
  
  while (x < w2)
  {
    plot4points(cx, cy, x, y);
    x++;
  }
  
  while (y >= 0)
  {
    plot4points(cx, cy, x, y);
    y--;
  }
}
*/

