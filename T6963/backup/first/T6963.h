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
#define GLCD_WR				0	
#define GLCD_RD				1
#define GLCD_CE				2  //Should be able to XNOR this with WR and RD
#define GLCD_CD				3
//#define GLCD_RESET			4  //For some reason my LCD works with this pin resistored to +5
//#define GLCD_FS				5  //Use hardware solution not pin.

// display properties

const byte T6963_CURSOR_PATTERN_SELECT=0xA0; //cursor patter select command prefix or with desired size-1.
const byte T6963_DISPLAY_MODE=0x90; 


class T6963{
public:
  T6963(int pixHoriz,int pixVert,int fontWidth, int sizeMem); //Class
  //pixHoriz = Horizontal resolution
  //pixVert = Vertical Resolution
  //fontWidth = pixel width of font, determines number of columns, use hardware setting for LCD, no outputs linked to this variable.
  //sizeMem = size of memory attached to LCD in kb s/b 4,8,16,32,64 type number.
  
  void InitalizeInterface(void);
  void writeCommand(byte);
  void writeData(byte);
  byte ReadData(void);
  void clearText(void);
  void clearCG(void);
  void clearGraphic(void);
  void WriteChar(char ch);
  void WriteString(char * str);
  void WriteStringPgm(prog_char * str);
  void TextGoTo(byte, byte);
  void GraphicGoTo(byte x, byte y);
  void DefineCharacter(unsigned char, unsigned char *);
  void Initialize(void);
  void writePixel(byte, byte, byte);
  byte clearPixel(byte,byte);
  byte setPixel(byte,byte);
  void WriteDisplayData(byte);
  byte checkStatus(void);
  void SetAddressPointer(unsigned int address);
  byte setMode(char, char);
  byte clearDispMode();
  byte setDispMode(boolean _text,boolean _graphics, boolean _cursor, boolean _blink);
  byte setCursorPattern(byte _b);
  byte setCursorPointer(byte _col,byte _row);
  
  void createLine(int,int,int,int);
  void createCircle(int, int, int);
  
  void setTH(unsigned int addr);
  unsigned int getTH();
  void setGH(unsigned int addr);
  unsigned int getGH();
  unsigned int sizeGA;	//(GLCD_GRAPHIC_AREA * GLCD_NUMBER_OF_LINES)
  unsigned int sizeTA;	//(GLCD_TEXT_AREA * (GLCD_NUMBER_OF_LINES/8))
  byte getTextRows();
  byte getTextCols();
   
private:
  void plot8points(int, int, int, int);
  void plot4points(int, int, int, int);
  unsigned int GLCD_NUMBER_OF_LINES;	//Veritical Resolution, Lines
  unsigned int GLCD_PIXELS_PER_LINE;	//Horizontal Resolution, pixels per line
  unsigned int _FW;		//Font Width
  unsigned int _GH;		//Graphics home
  unsigned int _TH;		//Text Home
  unsigned int _GA;		//(GLCD_PIXELS_PER_LINE / GLCD_FONT_WIDTH)
  unsigned int _TA;		//(GLCD_PIXELS_PER_LINE / GLCD_FONT_WIDTH)
  byte _sizeMem;  //memory location for CGRAM (2kb required, 64kb max memory controlled by T6963, 32 possible locations)

};


#endif

