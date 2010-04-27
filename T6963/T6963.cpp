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
#include <WProgram.h>

extern "C"{
	#include <inttypes.h>
	#include <avr/io.h>
	#include <avr/pgmspace.h>
	#include <util/delay.h>
}

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
//	Input	none
//
//	Output	none
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
//	Input	none
//
//	Output	none
//	
//-------------------------------------------------------------------------------------------------

T6963::T6963()
{
}









//*************************************************************************************************
//
//		Basic I/O Functions
//
//*************************************************************************************************

//-------------------------------------------------------------------------------------------------
//
// Read controller status
//
//	Input	none
//
//	Output	controller status
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
// Read data from the controller
//
//	Input	none
//
//	Output	data read
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
// Write a command to the controller
//
//	Input	command: the command to send
//
//	Output	none
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
// Write data to the controller
//
//	Input	data: the data to send
//
//	Output	none
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





















//*************************************************************************************************
//
//		Basic Controller Functions
//
//*************************************************************************************************

//-------------------------------------------------------------------------------------------------
//
// Set display mode
//
//	Input	mode: the display mode
//			CG: location of character graphics
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void T6963::setMode(uint8_t mode, uint8_t CG)
{
	writeCommand(T6963_MODE_SET | CG | mode);
}

//-------------------------------------------------------------------------------------------------
//
// Set display settings
//
//	Input	settings: the display settings
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void T6963::setDisplay(uint8_t settings)
{
	writeCommand(T6963_DISPLAY_MODE | settings);
}

//-------------------------------------------------------------------------------------------------
//
// Set the address pointer in display memory
//
//	Input	none
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void T6963::setAddress(void)
{
	writeData(_address & 0xFF);
	writeData(_address >> 8);
	writeCommand(T6963_SET_ADDRESS_POINTER);
}

//-------------------------------------------------------------------------------------------------
//
// Set the text address pointer in display memory
//
//	Input	none
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void T6963::setText(void)
{
	writeData(_text & 0xFF);
	writeData(_text >> 8);
	writeCommand(T6963_SET_ADDRESS_POINTER);
}

//-------------------------------------------------------------------------------------------------
//
// Set the screen cursor position
//
//	Input	col: cursor column 
//			row: cursor row
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void T6963::setCursorPointer(uint8_t col, uint8_t row)
{
	col = constrain(col, 0, (MEM_TEXT_WIDTH - 1));
	row = constrain(row, 0, (MEM_TEXT_HEIGHT - 1));
	writeData(col);
	writeData(row);
	writeCommand(T6963_SET_CURSOR_POINTER);
}

//-------------------------------------------------------------------------------------------------
//
// Sets cursor Pattern
//
//	Input	cursor: the cursor pattern
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void T6963::setCursorPattern(uint8_t cursor)
{
	cursor = constrain(cursor, 0, 7);
	writeCommand(T6963_CURSOR_PATTERN_SELECT | cursor);
}

//-------------------------------------------------------------------------------------------------
//
// Read display data
//
//	Input	none
//
//	Output	data read
//
//-------------------------------------------------------------------------------------------------

uint8_t T6963::readByte(void)
{
	writeCommand(T6963_DATA_READ_AND_NONVARIABLE);
	return readData();
}

//-------------------------------------------------------------------------------------------------
//
// Read display data and increment address pointer
//
//	Input	none
//
//	Output	data read
//
//-------------------------------------------------------------------------------------------------

uint8_t T6963::readByteInc(void)
{
	writeCommand(T6963_DATA_READ_AND_INCREMENT);
	return readData();
}

//-------------------------------------------------------------------------------------------------
//
// Read display data and decrement address pointer
//
//	Input	none
//
//	Output	data read
//
//-------------------------------------------------------------------------------------------------

uint8_t T6963::readByteDec(void)
{
	writeCommand(T6963_DATA_READ_AND_DECREMENT);
	return readData();
}

//-------------------------------------------------------------------------------------------------
//
// Write display data
//
//	Input	data: write data
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void T6963::writeByte(uint8_t data)
{
	writeData(data);
	writeCommand(T6963_DATA_WRITE_AND_NONVARIABLE);
}

//-------------------------------------------------------------------------------------------------
//
// Write display data and increment address pointer
//
//	Input	data: write data
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void T6963::writeByteInc(uint8_t data)
{
	writeData(data);
	writeCommand(T6963_DATA_WRITE_AND_INCREMENT);
}

//-------------------------------------------------------------------------------------------------
//
// Write display data and decrement address pointer
//
//	Input	data: write data
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void T6963::writeByteDec(uint8_t data)
{
	writeData(data);
	writeCommand(T6963_DATA_WRITE_AND_DECREMENT);
}

//-------------------------------------------------------------------------------------------------
//
// Write to a single bit
//
//	Input	data: write data
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void T6963::writeBit(uint8_t data)
{
	writeCommand(T6963_SET_PIXEL | data);
}


//-------------------------------------------------------------------------------------------------
//
// Write a byte of data to a block of memory
//
//	Input	data: byte of data
//			size: length of write (can be negative to write backward)
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void T6963::writeBlock(uint8_t data, uint16_t size)
{
	while (size > 0)
	{
		writeData(data);
		writeCommand(T6963_DATA_WRITE_AND_INCREMENT);
		size--;
	}
}




/*
//-------------------------------------------------------------------------------------------------
//
// Write a byte of data to a block of memory
//
//	Input	data: byte of data
//			width: width of write
//			height: height of write
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void T6963::writeBlock(uint8_t data, uint8_t width, uint8_t height)
{
	if (width <  MEM_GRAPH_WIDTH)
	{
		uint8_t mem = MEM_GRAPH_WIDTH - width;
		
		while (height > 0)
		{
			uint8_t size;
			
			for (size = width; size > 0; size--)
			{
				writeData(data);
				writeCommand(T6963_DATA_WRITE_AND_INCREMENT);
			}
			
			_address += mem;
			setAddress();
			
			height--;
		}
	}
}
*/



























//*************************************************************************************************
//
//		Primative Drawing Functions
//
//*************************************************************************************************

//-------------------------------------------------------------------------------------------------
//
// Draw a horizontal line
//
//	Input	length: line length
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void T6963::horizLine(int16_t length)
{
	uint8_t cmd, bit;
	
	cmd = T6963_SET_PIXEL | _color;
	
	if (length < 0)
	{
		length = -length;
		
		if (_bit + length > FONT_WIDTH - 1)
		{
			length -= FONT_WIDTH - _bit;
			
			while (_bit < FONT_WIDTH)
			{
				writeCommand(cmd | _bit);
				_bit++;
			}
			
			_bit = 0;
			_address--;
			setAddress();
			
			if (length > FONT_WIDTH - 1)
			{
				uint8_t col, data;
				
				col = length / FONT_WIDTH;
				_address -= col;
				length -= col * FONT_WIDTH;
				
				data = _color ? 0xFF : 0;
				
				while (col > 0)
				{
					writeData(data);
					writeCommand(T6963_DATA_WRITE_AND_DECREMENT);
					col--;
				}
				
				if (length == 0)
				{
					return;
				}
			}
			
			
		}
		
		bit = _bit + length;
		
		while (_bit < bit)
		{
			writeCommand(cmd | _bit);
			_bit++;
		}
	}
	else
	{
		if (_bit - length < 0)
		{
			_bit++;
			length -= _bit;
			
			while (_bit > 0)
			{
				_bit--;
				writeCommand(cmd | _bit);
				
			}
			
			_bit = FONT_WIDTH - 1;
			_address++;
			setAddress();
			
			if (length > FONT_WIDTH)
			{
				uint8_t col, data;
				
				col = length / FONT_WIDTH;
				_address += col;
				length -= col * FONT_WIDTH;
				
				data = _color ? 0xFF : 0;
				
				while (col > 0)
				{
					writeData(data);
					writeCommand(T6963_DATA_WRITE_AND_INCREMENT);
					col--;
				}
				
				if (length == 0)
				{
					return;
				}
			}
		}
		
		bit = _bit - length;
		
		while (_bit > bit)
		{
			writeCommand(cmd | _bit);
			_bit--;
		}
	}
}

//-------------------------------------------------------------------------------------------------
//
// Draw a vertical line
//
//	Input	length: line length
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void T6963::vertLine(int16_t length)
{
	uint8_t cmd;
	
	cmd = T6963_SET_PIXEL | _color | _bit;
	
	while (length > 0)
	{
		writeCommand(cmd);
		
		_address += MEM_GRAPH_WIDTH;
		setAddress();
		
		length--;
	}
	
	while (length < 0)
	{
		writeCommand(cmd);
		
		_address -= MEM_GRAPH_WIDTH;
		setAddress();
		
		length++;
	}
}

//-------------------------------------------------------------------------------------------------
//
// Draw a diagonal line
//
//	Input	dx: change in x
//			yIsNeg: 1 if y is negative
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void T6963::diagLine(int16_t dx, uint8_t yIsNeg)
{
	uint8_t cmd;
	int16_t dy;
	
	cmd = T6963_SET_PIXEL | _color;
	dy =  MEM_GRAPH_WIDTH;
	
	if (yIsNeg)
	{
		dy = -dy;
	}
	
	while (dx > 0)
	{
		writeCommand(cmd | _bit);
		
		if (_bit > 0)
		{
			_bit--;
		}
		else
		{
			_bit = FONT_WIDTH - 1;
			_address++;
		}
		
		_address += dy;
		setAddress();
		
		dx--;
	}
	
	while (dx < 0)
	{
		writeCommand(cmd | _bit);
		
		if (_bit < FONT_WIDTH - 1)
		{
			_bit++;
		}
		else
		{
			_bit = 0;
			_address--;
		}
		
		_address += dy;
		setAddress();
		
		dx++;
	}
}

//-------------------------------------------------------------------------------------------------
//
// Draw a bresenham run-slice line
//
//	Input	dx: change in x
//			dy: change in y
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void T6963::bresenLine(int16_t dx, int16_t dy)
{
	uint16_t errInc, errDec;
	int16_t mem, error;
	int8_t incX, incY, step;
	
	mem = MEM_GRAPH_WIDTH;
	incX = 1;
	incY = 1;
	
	if (dx < 0)
	{
		dx = -dx;
		incX = -1;
	}
	
	if (dy < 0)
	{
		dy = -dy;
		incY = -1;
		mem = -mem;
	}
	
	if (dx > dy)
	{
		error = (dx % dy);
		
		errInc = error * 2;
		errDec = dy * 2;
		
		step = (dx / dy) * incX;
		
		error -= ((step & 0x01) == 0) ? errDec : dy;
		
		horizLine(step / 2 + (errInc == 0 && (step & 0x01) == 0 ? 0 : incX));
		_address += mem;
		setAddress();
		
		dy -= 1;
		
		while (dy > 0)
		{
			error += errInc;
			
			if (error > 0)
			{
				error -= errDec;
				horizLine(step + incX);
			}
			else
			{
				horizLine(step);
			}
			
			_address += mem;
			setAddress();
			
			dy--;
		}
		
		horizLine(step / 2);
	}
	else
	{
		error = (dy % dx);
		
		errInc = error * 2;
		errDec = dx * 2;
		
		step = (dy / dx) * incY;
		
		error -= ((step & 0x01) == 0) ? errDec : dx;
		
		vertLine(step / 2 - (errInc == 0 && (step & 0x01) == 0 ? incY : 0));
		horizLine(incX);
		_address += mem;
		setAddress();
		
		dx -= 1;
		
		while (dx > 0)
		{
			error += errInc;
			
			if (error > 0)
			{
				error -= errDec;
				vertLine(step);
			}
			else
			{
				vertLine(step - incY);
			}
			
			horizLine(incX);
			_address += mem;
			setAddress();
			
			dx--;
		}
		
		vertLine(step / 2);
	}
}


























//*************************************************************************************************
//
//		Basic Drawing Functions
//
//*************************************************************************************************

//-------------------------------------------------------------------------------------------------
//
// Clear graphic memory
//
//	Input	none
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void T6963::clear(void)
{
	uint16_t tmp = _address;
	
	_address = MEM_GRAPH_START;
	setAddress();
	writeBlock(0, MEM_GRAPH_AREA);
	
	_address = tmp;
	setAddress();
}

//-------------------------------------------------------------------------------------------------
//
// Set the color to draw
//
//	Input	color: the color (1 = black; 0 = white)
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void T6963::setColor(uint8_t color)
{
	_color = (color > 0) ? T6963_BIT_SET : T6963_BIT_RESET;
}

//-------------------------------------------------------------------------------------------------
//
// Move to a relative point in graphic memory
//
//	Input	dx: change in x
//			dy: change in y
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void T6963::move(int16_t dx, int16_t dy)
{
	if (dx != 0 && dx < (SCREEN_WIDTH - _lastX) && dx >= (0 - _lastX))
	{
		int8_t col;
		
		_lastX += dx;
		dx -= _bit;
		col = dx / FONT_WIDTH;
		dx -= col * FONT_WIDTH;
		
		if (dx > 0)
		{
			col++;
			_bit = FONT_WIDTH - dx;
		}
		else
		{
			_bit = -dx;
		}
		
		_address += col;
	}
	
	if (dy != 0 && dy < (SCREEN_HEIGHT - _lastY) && dy >= (0 - _lastY))
	{
		_lastY += dy;
		
		_address += dy * MEM_GRAPH_WIDTH;
		
	}
	
	setAddress();
}

//-------------------------------------------------------------------------------------------------
//
// Move to a point in graphic memory
//
//	Input	x: new x
//			y: new y
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void T6963::moveTo(uint8_t x, uint8_t y)
{
	if (x < SCREEN_WIDTH && y < SCREEN_HEIGHT)
	{
		uint8_t col, bit;
		
		_lastX = x;
		_lastY = y;
		
		col = x / FONT_WIDTH;
		
		_address = MEM_GRAPH_START + MEM_GRAPH_WIDTH * y + col;
		_bit = (col + 1) * FONT_WIDTH - x - 1;
		
		setAddress();
	}
}

//-------------------------------------------------------------------------------------------------
//
// Draw a line to a relative point in graphic memory
//
//	Input	dx: change in x
//			dy: change in y
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void T6963::line(int16_t dx, int16_t dy)
{
	if (dy == 0)
	{
		if (dy < (SCREEN_HEIGHT - _lastY) && dy >= (0 - _lastY))
		{
			_lastX += dx;
			horizLine(dx);
		}
	}
	else if (dx == 0)
	{
		if (dy < (SCREEN_HEIGHT - _lastY) && dy >= (0 - _lastY))
		{
			_lastY += dy;
			vertLine(dy);
		}
	}
	else if (dx < (SCREEN_WIDTH - _lastX) && dx >= (0 - _lastX) && dy < (SCREEN_HEIGHT - _lastY) && dy >= (0 - _lastY))
	{
		_lastX += dx;
		_lastY += dy;
		
		if (dx == dy || dx == -dy)
		{
			diagLine(dx, (dy < 0));
		}
		else
		{
			bresenLine(dx, dy);
		}
	}
}

//-------------------------------------------------------------------------------------------------
//
// Draw a line to a point in graphic memory
//
//	Input	x: new x
//			y: new y
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void T6963::lineTo(uint8_t x, uint8_t y)
{
	if (y == _lastY)
	{
		if (x < SCREEN_WIDTH)
		{
			horizLine(x - _lastX);
			_lastX = x;
		}
	}
	else if (x == _lastX)
	{
		if (y < SCREEN_HEIGHT)
		{
			vertLine(y - _lastY);
			_lastY = y;
		}
	}
	else if (x < SCREEN_WIDTH && y < SCREEN_HEIGHT)
	{
		int16_t dx, dy;
		
		dx = x - _lastX;
		dy = y - _lastY;
		
		_lastX = x;
		_lastY = y;
		
		if (dx == dy || dx == -dy)
		{
			diagLine(dx, (dy < 0));
		}
		else
		{
			bresenLine(dx, dy);
		}
	}
}

//-------------------------------------------------------------------------------------------------
//
// Draw a rectangle relative to a point in graphic memory
//
//	Input	dx: width
//			dy: height
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void T6963::rect(int16_t dx, int16_t dy)
{
	if (dy == 0)
	{
		if (dy < (SCREEN_HEIGHT - _lastY) && dy >= (0 - _lastY))
		{
			horizLine(dx);
			move(-dx, 0);
		}
	}
	else if (dx == 0)
	{
		if (dy < (SCREEN_HEIGHT - _lastY) && dy >= (0 - _lastY))
		{
			vertLine(dy);
			move(0, -dy);
		}
	}
	else if (dx < (SCREEN_WIDTH - _lastX) && dx >= (0 - _lastX) && dy < (SCREEN_HEIGHT - _lastY) && dy >= (0 - _lastY))
	{
		horizLine(dx);
		vertLine(dy);
		horizLine(-dx);
		vertLine(-dy);
	}
}

//-------------------------------------------------------------------------------------------------
//
// Draw a rectangle to a point in graphic memory
//
//	Input	x: new x
//			y: new y
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void T6963::rectTo(uint8_t x, uint8_t y)
{
	int16_t dx, dy;
	
	dx = x - _lastX;
	dy = y - _lastY;
	
	if (dy == 0)
	{
		if (x < SCREEN_WIDTH)
		{
			horizLine(dx);
			move(-dx, 0);
		}
	}
	else if (dx == 0)
	{
		if (y < SCREEN_HEIGHT)
		{
			vertLine(dy);
			move(0, -dy);
		}
	}
	else if (x < SCREEN_WIDTH && y < SCREEN_HEIGHT)
	{
		horizLine(dx);
		vertLine(dy);
		horizLine(-dx);
		vertLine(-dy);
	}
}


















//*************************************************************************************************
//
//		Basic Text Functions
//
//*************************************************************************************************

//-------------------------------------------------------------------------------------------------
//
// Clear all text
//
//	Input	none
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void T6963::clearText(void)
{
	_text = MEM_TEXT_START;
	setText();
	writeBlock(0, MEM_TEXT_AREA);
	
	setAddress();
}

//-------------------------------------------------------------------------------------------------
//
// Clear a section of text
//
//	Input	none
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void T6963::clearText(int16_t size)
{
	setText();
	
	size = constrain(size, MEM_TEXT_START - _text, (MEM_TEXT_END - 1) - _text);
	_text += size;
	
	while (size > 0)
	{
		writeData(0);
		writeCommand(T6963_DATA_WRITE_AND_INCREMENT);
		size--;
	}
	
	while (size < 0)
	{
		writeData(0);
		writeCommand(T6963_DATA_WRITE_AND_DECREMENT);
		size++;
	}
	
	setAddress();
}

//-------------------------------------------------------------------------------------------------
//
// Set relative text location
//
//	Input	col: horizontal location
//			row: vertical location
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void T6963::text(int16_t col, int16_t row)
{
	_text = constrain(_text + MEM_TEXT_WIDTH * row + col, 0, MEM_TEXT_END - 1);
}

//-------------------------------------------------------------------------------------------------
//
// Set text location
//
//	Input	col: horizontal location
//			row: vertical location
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void T6963::textTo(uint8_t col, uint8_t row)
{
	_text = min(MEM_TEXT_START + MEM_TEXT_WIDTH * row + col, MEM_TEXT_END - 1);
}

//-------------------------------------------------------------------------------------------------
//
// Write a string of text
//
//	Input	*string: pointer to string
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void T6963::text(char *string)
{
	setText();
	
	while (*string && _text < MEM_TEXT_END)
	{
		writeData((*string) - 32);
		writeCommand(T6963_DATA_WRITE_AND_INCREMENT);
		string++;
		_text++;
	}
	
	setAddress();
}

//-------------------------------------------------------------------------------------------------
//
// Write part of a string of text
//
//	Input	*string: pointer to string
//			size: length of string (can be negative to write backward)
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void T6963::text(char *string, int16_t size)
{
	setText();
	
	size = constrain(size, MEM_TEXT_START - _text, (MEM_TEXT_END - 1) - _text);
	_text += size;
	
	while (*string && size > 0)
	{
		writeData((*string) - 32);
		writeCommand(T6963_DATA_WRITE_AND_INCREMENT);
		string++;
		size--;
	}
	
	while (*string && size < 0)
	{
		writeData((*string) - 32);
		writeCommand(T6963_DATA_WRITE_AND_DECREMENT);
		string++;
		size++;
	}
	
	setAddress();
}

//-------------------------------------------------------------------------------------------------
//
// Write a string of text from program memory
//
//	Input	col: horizontal location
//			row: vertical location
//			*string: pointer to program memory string
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void T6963::textPgm(prog_char *string)
{
	char charCode;
	
	setText();
	
	while (charCode = pgm_read_byte(string) && _text < MEM_TEXT_END)
	{
		writeData(charCode - 32);
		writeCommand(T6963_DATA_WRITE_AND_INCREMENT);
		string++;
		_text++;
	}
	
	setAddress();
}

//-------------------------------------------------------------------------------------------------
//
// Write part of a string of text from program memory
//
//	Input	*string: pointer to program memory string
//			size: length of string (can be negative to write backward)
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void T6963::textPgm(prog_char *string, int16_t size)
{
	char charCode;
	
	setText();
	
	size = constrain(size, MEM_TEXT_START - _text, (MEM_TEXT_END - 1) - _text);
	_text += size;
	
	while ((charCode = pgm_read_byte(string)) && size > 0)
	{
		writeData(charCode - 32);
		writeCommand(T6963_DATA_WRITE_AND_INCREMENT);
		string++;
		size--;
	}
	
	while ((charCode = pgm_read_byte(string)) && size < 0)
	{
		writeData(charCode - 32);
		writeCommand(T6963_DATA_WRITE_AND_DECREMENT);
		string++;
		size++;
	}
	
	setAddress();
}


















//*************************************************************************************************
//
//		Character Graphic Functions
//
//*************************************************************************************************

//-------------------------------------------------------------------------------------------------
//
// Clear character memory
//
//	Input	none
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------

void T6963::clearCG(void)
{
	uint16_t tmp = _address;
	
	_address = MEM_CG_START;
	setAddress();
	writeBlock(0, MEM_CG_SIZE);
	
	_address = tmp;
	setAddress();
}

























//-------------------------------------------------------------------------------------------------
//
// Display initalization
//
//-------------------------------------------------------------------------------------------------

void T6963::init(void)
{
	GLCD_DATA_DDR1 |= GLCD_DATA_MASK1;
	GLCD_DATA_DDR2 |= GLCD_DATA_MASK2;
	
	GLCD_CTRL_DDR |= ((1 << GLCD_RD) | (1 << GLCD_CE) | (1 << GLCD_CD));
	GLCD_CTRL_PORT |= ((1 << GLCD_RD) | (1 << GLCD_CE) | (1 << GLCD_CD));
	
	//Set text area home address
	writeData(MEM_TEXT_START & 0xFF);
	writeData(MEM_TEXT_START >> 8);
	writeCommand(T6963_SET_TEXT_HOME_ADDRESS);
	
	//Set text area width in mem
	writeData(MEM_TEXT_WIDTH);
	writeData(0x00);
	writeCommand(T6963_SET_TEXT_AREA);
	
	//Set graphic area home address
	writeData(MEM_GRAPH_START & 0xFF);
	writeData(MEM_GRAPH_START >> 8);
	writeCommand(T6963_SET_GRAPHIC_HOME_ADDRESS);
	
	//Set graphic area width in mem
	writeData(MEM_GRAPH_WIDTH);
	writeData(0x00);
	writeCommand(T6963_SET_GRAPHIC_AREA);
	
	//Set Internal CGRAM address
	writeData(MEM_CG_OFFSET);
	writeData(0x00);
	writeCommand(T6963_SET_OFFSET_REGISTER);
	
	setMode(T6963_MODE_XOR, T6963_MODE_INTERNAL);
	setDisplay((1<<T6963_DISPLAY_TEXT) | (1<<T6963_DISPLAY_GRAPHIC) | (0<<T6963_DISPLAY_CURSOR) | (0<<T6963_DISPLAY_BLINK));
	
	_address = 0;
	setAddress();
	writeBlock(0x00, MEM_SIZE * 0x400);
	_text = 0;
	
	_bit = 0;
	_color = T6963_BIT_SET;
	
	moveTo(0, 0);
	
	delay(100);
}

//-------------------------------------------------------------------------------------------------
//
// Preinstantiate Object
//
//-------------------------------------------------------------------------------------------------

T6963 LCD = T6963();
