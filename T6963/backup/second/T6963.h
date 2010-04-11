/*-------------------------------------------------------------------------------------------------
0.0- What these 2 guys built that I worked from
      Graphic LCD with Toshiba T6963 controller
      Copyright (c) Rados³aw Kwiecieñ, 2007r
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
#ifndef T6963_h
#define T6963_h

#include "WProgram.h"
#include "inttypes.h"
#include "avr/io.h"
#include "avr/pgmspace.h"
#include "util/delay.h"
#include "T6963_Commands.h"

// data port
#define GLCD_DATA_PORT1 	PORTC
#define GLCD_DATA_PIN1		PINC
#define GLCD_DATA_DDR1		DDRC
#define GLCD_DATA_SHIFT1	<<0
#define GLCD_DATA_RSHIFT1	>>0
#define GLCD_DATA_MASK1		0x0F

#define GLCD_DATA_PORT2 	PORTD
#define GLCD_DATA_PIN2		PIND
#define GLCD_DATA_DDR2		DDRD
#define GLCD_DATA_SHIFT2	>>0
#define GLCD_DATA_RSHIFT2	<<0
#define GLCD_DATA_MASK2		0xF0

// control port
#define GLCD_CTRL_PORT		PORTB
#define GLCD_CTRL_PIN		PINB
#define GLCD_CTRL_DDR		DDRB

// control signals
//#define GLCD_WR				0
#define GLCD_RD				0
#define GLCD_CE				1  //Should be able to XNOR this with WR and RD
#define GLCD_CD				2
//#define GLCD_RESET			4  //For some reason my LCD works with this pin resistored to +5
//#define GLCD_FS				5  //Use hardware solution not pin.

//const byte T6963_CURSOR_PATTERN_SELECT=0xA0; //cursor patter select command prefix or with desired size-1.
//const byte T6963_DISPLAY_MODE=0x90; 

class T6963
{
public:
  uint16_t sizeGA;	//(GLCD_GRAPHIC_AREA * GLCD_NUMBER_OF_LINES)
  uint16_t sizeTA;	//(GLCD_TEXT_AREA * (GLCD_NUMBER_OF_LINES/8))
  
  T6963(uint16_t pixHoriz, uint16_t pixVert, uint16_t sizeMem);
  //pixHoriz = Horizontal resolution
  //pixVert = Vertical Resolution
  //sizeMem = size of memory attached to LCD in kb s/b 4,8,16,32,64 type number.
  
  void setTH(uint16_t addr);
  void setGH(uint16_t addr);
  
  uint16_t getTH(void);
  uint16_t getGH(void);
  uint16_t getTA(void);
  uint16_t getGA(void);
  
  uint8_t setMode(uint8_t _mode, uint8_t _CG);
  uint8_t setDispMode(uint8_t settings);
  uint8_t clearDispMode(void);
  
  void setAddressPointer(uint16_t address);
  void setCursorPointer(uint8_t _col, uint8_t _row);
  uint8_t setCursorPattern(uint8_t _b);
  uint8_t getTextRows(void);
  uint8_t getTextCols(void);
  
  void graphicGoTo(uint8_t x, uint8_t y);
  void textGoTo(uint8_t x, uint8_t y);
  
  void writeDispData(uint8_t data);
  uint8_t readDispData(void);
  
  void writeBlock(uint8_t length, uint8_t data);
  void writeBlockMask(uint8_t length, uint8_t mask);
  void writeBlockMask(uint8_t length, uint8_t data, uint8_t mask);
  void writeLine(uint8_t width, uint8_t left, uint8_t left_mask, uint8_t fill, uint8_t rght, uint8_t rght_mask);
  
  void writeChar(char charCode);
  void writeString(char * string);
  void writeStringPgm(prog_char * string);
  
  void defineCharacter(uint8_t charCode, uint8_t * defChar);
  
  uint8_t clearPixel(uint8_t x, uint8_t y);
  uint8_t setPixel(uint8_t x, uint8_t y);
  
  void fillBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t * pattern, uint8_t size);
  void clearBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
  void setBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
  
  void clearGraphic(void);
  void clearText(void);
  void clearCG(void);
  
  void init(void);
  
  //void writePixel(byte, byte, byte);
  //void createLine(int,int,int,int);
  //void createCircle(int, int, int);
  //void createBox(int, int, int, int);
  
private:
  uint16_t GLCD_NUMBER_OF_LINES;	//Veritical Resolution, Lines
  uint16_t GLCD_PIXELS_PER_LINE;	//Horizontal Resolution, pixels per line
  uint16_t _GH;		//Graphics home
  uint16_t _TH;		//Text Home
  uint16_t _GA;		//(GLCD_PIXELS_PER_LINE / GLCD_FONT_WIDTH)
  uint16_t _TA;		//(GLCD_PIXELS_PER_LINE / GLCD_FONT_WIDTH)
  uint8_t _sizeMem;  //memory location for CGRAM (2kb required, 64kb max memory controlled by T6963, 32 possible locations)
  
  void initalizePorts(void);
  uint8_t checkStatus(void);
  
  void writeCommand(uint8_t _command);
  void writeData(uint8_t data);
  uint8_t readData(void);
  
  //void plot8points(int, int, int, int);
  //void plot4points(int, int, int, int);
};


#endif

