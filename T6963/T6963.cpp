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

T6963::T6963()
{
}

//-------------------------------------------------------------------------------------------------
//
// Reads dispay status
//
//-------------------------------------------------------------------------------------------------

uint8_t T6963::readStatus(void)
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
// Reads data
//
//-------------------------------------------------------------------------------------------------

uint8_t T6963::readData(void)
{
	uint8_t tmp;
	
	while(!(readStatus() & 0x03));
	
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
// Writes instruction 
//
//-------------------------------------------------------------------------------------------------

void T6963::writeCommand(uint8_t command)
{
	while(!(readStatus() & 0x03));
	
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
	while(!(readStatus() & 0x03));
	
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
// Set Display Mode
//
//-------------------------------------------------------------------------------------------------

void T6963::setMemMode(uint8_t mode, uint8_t CG)
{
	uint8_t tmp = T6963_MODE_SET;
	
	tmp |= CG | mode;
	writeCommand(tmp);
}

void T6963::setDispMode(uint8_t settings)
{
	uint8_t tmp = T6963_DISPLAY_MODE | settings;
	writeCommand(tmp);
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

void T6963::setCursorPointer(uint8_t col, uint8_t row)
{
	col = constrain(col, 0, (_TA - 1));
	row = constrain(row, 0, ((vertPix >> 3) - 1));
	writeData(col);
	writeData(row);
	writeCommand(T6963_SET_CURSOR_POINTER);
}

void T6963::setCursorPattern(uint8_t cursor)
{
	uint8_t tmp = T6963_CURSOR_PATTERN_SELECT;
	
	cursor = constrain(cursor, 0, 7);
	tmp |= cursor;
	
	writeCommand(tmp);
}

//-------------------------------------------------------------------------------------------------
//
// Sets coordinates
//
//-------------------------------------------------------------------------------------------------

void T6963::gotoText(uint8_t col, uint8_t row)
{
	setAddressPointer(_TH + col + (_TA * row));
}

void T6963::gotoGraphic(uint8_t col, uint8_t row)
{
	setAddressPointer(_GH + col + (_GA * row));
}
//-------------------------------------------------------------------------------------------------
//
// Writes display data and increment address pointer
//
//-------------------------------------------------------------------------------------------------

void T6963::writeDisplay(uint8_t data)
{
	writeData(data);
	writeCommand(T6963_DATA_WRITE_AND_INCREMENT);
}

void T6963::writeDispInc(uint8_t data)
{
	writeData(data);
	writeCommand(T6963_DATA_WRITE_AND_INCREMENT);
}

void T6963::writeDispDec(uint8_t data)
{
	writeData(data);
	writeCommand(T6963_DATA_WRITE_AND_DECREMENT);
}

//-------------------------------------------------------------------------------------------------
//
// Read display data
//
//-------------------------------------------------------------------------------------------------

uint8_t T6963::readDisplay(void)
{
	uint8_t tmp;
	
	writeCommand(T6963_DATA_READ_AND_NONVARIABLE);
	tmp = readData();
	
	return tmp;
}

uint8_t T6963::readDispInc(void)
{
	uint8_t tmp;
	
	writeCommand(T6963_DATA_READ_AND_INCREMENT);
	tmp = readData();
	
	return tmp;
}

uint8_t T6963::readDispDec(void)
{
	uint8_t tmp;
	
	writeCommand(T6963_DATA_READ_AND_DECREMENT);
	tmp = readData();
	
	return tmp;
}

//-------------------------------------------------------------------------------------------------
//
// Writes text (ASCII code) to display RAM memory
//
//-------------------------------------------------------------------------------------------------

void T6963::writeChar(char charCode)
{
	writeDisplay(charCode - 32);
}

void T6963::writeString(char * string)
{
	while(*string)
	{
		writeChar(*string++);  //seperate inc++ from writeChar command
	}
}

void T6963::writeString(char * string, uint8_t size)
{
	while(*string && size)
	{
		writeChar(*string++);
		size--;
	}
}

void T6963::writeStringPgm(prog_char * string)
{
	char charCode;
	while((charCode = pgm_read_byte(string++))) //same here
	{
		writeChar(charCode);
	}
}

//-------------------------------------------------------------------------------------------------
//
// Set a single pixel
//
//-------------------------------------------------------------------------------------------------

void T6963::writePixel(uint8_t data)
{
	uint8_t tmp = T6963_SET_PIXEL;
	
	tmp |= data;
	writeCommand(tmp);
}

//-------------------------------------------------------------------------------------------------
//
// Set a single pixel at x,y (in pixels) to 1 (on)
//
//-------------------------------------------------------------------------------------------------

void T6963::setPixel(uint8_t x, uint8_t y)
{
	uint8_t col = x / _FW;
	uint8_t tmp = T6963_BIT_SET;
	
	gotoGraphic(col, y);
	
	tmp |= (_FW * (col + 1)) - x - 1;
	writePixel(tmp);
}

void T6963::clearPixel(uint8_t x, uint8_t y)
{
	uint8_t col = x / _FW;
	uint8_t tmp = T6963_BIT_RESET;
	
	gotoGraphic(col, y);
	
	tmp |= (_FW * (col + 1)) - x - 1;
	writePixel(tmp);
}

//-------------------------------------------------------------------------------------------------
//
// Clears display RAM memory
//
//-------------------------------------------------------------------------------------------------

void T6963::clearText(void)
{
	uint16_t i;
	
	setAddressPointer(_TH);
	
	for(i = 0; i < sizeTA; i++)
	{
		writeDisplay(0);
	}
}

void T6963::clearGraphic(void)
{
	uint16_t i;
	
	setAddressPointer(_GH);
	
	for(i = 0; i < sizeGA; i++)
	{
		writeDisplay(0);
	}
}

void T6963::clearCG(void)
{
	uint16_t i;
	uint16_t address = ((memSize / 2) - 1) * 0x800;
	
	setAddressPointer(address);
	
	for(i = 0; i < 256 * 8; i++)
	{
		writeDisplay(0);
	}
}

//-------------------------------------------------------------------------------------------------
//
// Display initalization
//
//-------------------------------------------------------------------------------------------------

void T6963::init(uint8_t w, uint8_t h, uint8_t fw, uint8_t mem)
{
	horizPix = w;
	vertPix = h;
	memSize = constrain(mem, 0, 64);
	
	_FW = fw;
	_FH = 8;
	_TA = w / _FW;
	_GA = w / _FW;
	
	sizeTA = _TA * (h >> 3);
	sizeGA = _GA * h;
	
	_TH = 0;
	_GH = _TH + sizeTA;
	
	GLCD_DATA_DDR1 |= GLCD_DATA_MASK1;
	GLCD_DATA_DDR2 |= GLCD_DATA_MASK2;
	
	GLCD_CTRL_DDR |= ((1 << GLCD_RD) | (1 << GLCD_CE) | (1 << GLCD_CD));
	GLCD_CTRL_PORT |= ((1 << GLCD_RD) | (1 << GLCD_CE) | (1 << GLCD_CD));
	
	//Set Text home address
	writeData(_TH & 0xFF);
	writeData(_TH >> 8);
	writeCommand(T6963_SET_TEXT_HOME_ADDRESS);
	
	//Set Text Area
	writeData(_TA);
	writeData(0x00);
	writeCommand(T6963_SET_TEXT_AREA);
	
	//Set Graphics Home Address
	writeData(_GH & 0xFF);
	writeData(_GH >> 8);
	writeCommand(T6963_SET_GRAPHIC_HOME_ADDRESS);
	
	//Set Graphics Area
	writeData(_GA);
	writeData(0x00);
	writeCommand(T6963_SET_GRAPHIC_AREA);
	
	//Set Internal CGRAM address
	writeData(((memSize / 2) - 1));
	writeData(0x00);
	writeCommand(T6963_SET_OFFSET_REGISTER);
	
	setMemMode(T6963_MODE_XOR, T6963_MODE_INTERNAL);
	setDispMode((1<<T6963_DISPLAY_TEXT) | (1<<T6963_DISPLAY_GRAPHIC) | (0<<T6963_DISPLAY_CURSOR) | (0<<T6963_DISPLAY_BLINK));
	
	clearText();
	clearGraphic();
	clearCG();
	
	delay(100);
}

//-------------------------------------------------------------------------------------------------
//
// Preinstantiate Object
//
//-------------------------------------------------------------------------------------------------

T6963 LCD = T6963();
