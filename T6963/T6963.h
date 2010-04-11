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

class T6963
{
	public:
		uint8_t _FW;
		uint8_t _FH;
		
		T6963();
		
		uint8_t readStatus(void);
		uint8_t readData(void);
		void writeCommand(uint8_t);
		void writeData(uint8_t);
		
		void setMemMode(uint8_t, uint8_t);
		void setDispMode(uint8_t);
		void setAddressPointer(uint16_t);
		void setCursorPointer(uint8_t, uint8_t);
		void setCursorPattern(uint8_t);
		
		void gotoText(uint8_t, uint8_t);
		void gotoGraphic(uint8_t, uint8_t);
		
		void writeDisplay(uint8_t);
		void writeDispInc(uint8_t);
		void writeDispDec(uint8_t);
		
		uint8_t readDisplay(void);
		uint8_t readDispInc(void);
		uint8_t readDispDec(void);
		
		void writeChar(char);
		void writeString(char *);
		void writeString(char *, uint8_t);
		void writeStringPgm(prog_char *);
		
		void writePixel(uint8_t);
		void setPixel(uint8_t, uint8_t);
		void clearPixel(uint8_t, uint8_t);
		
		void clearText(void);
		void clearGraphic(void);
		void clearCG(void);
		
		void init(uint8_t, uint8_t, uint8_t, uint8_t);
		
	private:
		uint8_t horizPix;
		uint8_t vertPix;
		uint8_t memSize;
		
		uint16_t _TH;
		uint16_t _GH;
		uint16_t _TA;
		uint16_t _GA;
		
		uint16_t sizeTA;
		uint16_t sizeGA;
};

extern T6963 LCD;

#endif

