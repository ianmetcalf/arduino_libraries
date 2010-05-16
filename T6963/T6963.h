/*
	Library for the T6963 LCD controller by Ian T Metcalf
		tested with the Arduino IDE v18 on a Duemilanova 328
	
	Configured for 240x128 lcd with 8k of memory
		http://www.crystalfontz.com/products/240128l/datasheets/2104/CFAG240128LYYHTZ_vPreliminary_3.0.pdf
	
	Based on the library written by rbrsidedn1
		http://code.google.com/p/rbrsidedn1
	
	Original description by rbrsidedn1:
		0.0- What these 2 guys built that I worked from
			Graphic LCD with Toshiba T6963 controller
			Copyright (c) Rados?aw Kwiecie?, 2007r
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
		r8 - Got 6bit font width (s/b any font width) working.
	
	Changes by ITM:
		2010/04/30	restructured code to ease understanding for myself
		2010/04/30	hard coded display properties, condensed init()
		2010/04/30	added primitive drawing functions for line drawing
						horizLine
						vertLine
						diagLine
						bresenLine
		2010/04/30	added macdraw like functions:
						move(dx, dy)
						moveTo(x, y)
						line(dx, dy)
						lineTo(x, y)
						rect(dx, dy)
						rectTo(x, y)
		2010/04/30	expanded text functions
						partial clear written forward or backward
						partial string written forward or backward
						similar text(x, y) textTo(dx, dy) to functions above
	
	All works by ITM are released under the creative commons attribution share alike license
		http://creativecommons.org/licenses/by-sa/3.0/
	
	I can be contacted at metcalfbuilt@gmail.com
*/


#ifndef T6963_H
#define T6963_H

extern "C"{
	#include <inttypes.h>
	#include <avr/pgmspace.h>
}

#define SCREEN_WIDTH	240
#define SCREEN_HEIGHT	128

#define FONT_WIDTH	6
#define FONT_HEIGHT	8

#define SCREEN_COLS			(SCREEN_WIDTH/FONT_WIDTH)
#define SCREEN_ROWS			(SCREEN_HEIGHT/FONT_HEIGHT)

#define MEM_SIZE	8

#define MEM_TEXT_START		0
#define MEM_TEXT_WIDTH		SCREEN_COLS
#define MEM_TEXT_HEIGHT		SCREEN_ROWS
#define MEM_TEXT_AREA		(MEM_TEXT_WIDTH*MEM_TEXT_HEIGHT)
#define MEM_TEXT_END		(MEM_TEXT_START+MEM_TEXT_AREA)

#define MEM_GRAPH_START		MEM_TEXT_END
#define MEM_GRAPH_WIDTH		(SCREEN_WIDTH/FONT_WIDTH)
#define MEM_GRAPH_HEIGHT	SCREEN_HEIGHT
#define MEM_GRAPH_AREA		(MEM_GRAPH_WIDTH*SCREEN_HEIGHT)
#define MEM_GRAPH_END		(MEM_GRAPH_START+MEM_GRAPH_AREA)

#define MEM_CG_OFFSET		((MEM_SIZE/2)-1)
#define MEM_CG_START		(MEM_CG_OFFSET*256*8)
#define MEM_CG_SIZE			(256*8)






class T6963
{
	public:
		T6963();
		
		void setMode(uint8_t, uint8_t);
		void setDisplay(uint8_t);
		
		void setAddress(void);
		void setText(void);
		void setCursorPointer(uint8_t, uint8_t);
		void setCursorPattern(uint8_t);
		
		uint8_t readByte(void);
		uint8_t readByteInc(void);
		uint8_t readByteDec(void);
		
		void writeByte(uint8_t);
		void writeByteInc(uint8_t);
		void writeByteDec(uint8_t);
		
		void writeBit(uint8_t);
		void writeBlock(uint8_t, uint16_t);
		//void writeBlock(uint8_t, uint8_t, uint8_t);
		
		void horizLine(int16_t);
		void vertLine(int16_t);
		void diagLine(int16_t, uint8_t);
		void bresenLine(int16_t, int16_t);
		
		void clear(void);
		void setColor(uint8_t);
		void move(int16_t, int16_t);
		void moveTo(uint8_t, uint8_t);
		void line(int16_t, int16_t);
		void lineTo(uint8_t, uint8_t);
		void rect(int16_t, int16_t);
		void rect(int16_t, int16_t, uint8_t);
		void rectTo(uint8_t, uint8_t);
		
		void clearText(void);
		void clearText(int16_t);
		void text(int16_t, int16_t);
		void textTo(uint8_t, uint8_t);
		void text(char*);
		void text(char*, int16_t);
		void textPgm(prog_char*);
		void textPgm(prog_char*, int16_t);
		
		void clearCG(void);
		
		void init(void);
		
	private:
		uint16_t _address;
		uint16_t _text;
		
		uint8_t _bit;
		uint8_t _color;
		
		uint8_t _lastX;
		uint8_t _lastY;
		
		uint8_t readStatus(void);
		uint8_t readData(void);
		
		void writeCommand(uint8_t);
		void writeData(uint8_t);
};

extern T6963 LCD;

#endif

