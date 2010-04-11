/*

Declare library authors


*/

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
#define GLCD_RD				0  // Inversed with WR in hardware
#define GLCD_CE				1  // Should be able to XNOR this with WR and RD
#define GLCD_CD				2

static uint8_t t6963_horizPix;
static uint8_t t6963_vertPix;
static uint8_t t6963_memSize;

static uint16_t t6963_TH;
static uint16_t t6963_GH;
static uint16_t t6963_TA;
static uint16_t t6963_GA;

static uint16_t t6963_sizeTA;
static uint16_t t6963_sizeGA;

//-------------------------------------------------------------------------------------------------
//
// Delay function
//	
//-------------------------------------------------------------------------------------------------
void n_delay(void)
{
  volatile uint8_t i;
  for(i = 0; i < (F_CPU/1000000); i++)
  {
    asm("nop");
  }
}

//-------------------------------------------------------------------------------------------------
//
// Reads dispay status
//
//-------------------------------------------------------------------------------------------------

uint8_t t6963_readStatus(void)
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

uint8_t t6963_readData(void)
{
  uint8_t tmp;
  
  while(!(t6963_readStatus() & 0x03));
  
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

void t6963_writeCommand(uint8_t command)
{
  while(!(t6963_readStatus() & 0x03));
  
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

void t6963_writeData(uint8_t data)
{
  while(!(t6963_readStatus() & 0x03));
  
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

void t6963_setMemMode(uint8_t mode, uint8_t CG)
{
  uint8_t tmp = T6963_MODE_SET;
  
  tmp |= CG | mode;
  t6963_writeCommand(tmp);
}

void t6963_setDispMode(uint8_t settings)
{
  uint8_t tmp = T6963_DISPLAY_MODE | settings;
  t6963_writeCommand(tmp);
}

//-------------------------------------------------------------------------------------------------
//
// Sets address pointer for display RAM memory
//
//-------------------------------------------------------------------------------------------------

void t6963_setAddressPointer(uint16_t address)
{
  t6963_writeData(address & 0xFF);
  t6963_writeData(address >> 8);
  t6963_writeCommand(T6963_SET_ADDRESS_POINTER);
}

void t6963_setCursorPointer(uint8_t col, uint8_t row)
{
  col = constrain(col, 0, (t6963_TA - 1));
  row = constrain(row, 0, ((t6963_vertPix >> 3) - 1));
  t6963_writeData(col);
  t6963_writeData(row);
  t6963_writeCommand(T6963_SET_CURSOR_POINTER);
}

void t6963_setCursorPattern(uint8_t cursor)
{
  uint8_t tmp = T6963_CURSOR_PATTERN_SELECT;
  
  cursor = constrain(cursor, 0, 7);
  tmp |= cursor;
  
  t6963_writeCommand(tmp);
}

//-------------------------------------------------------------------------------------------------
//
// Sets coordinates
//
//-------------------------------------------------------------------------------------------------

void t6963_gotoText(uint8_t col, uint8_t row)
{
  t6963_setAddressPointer(t6963_TH + col + (t6963_TA * row));
}

void t6963_gotoGraphic(uint8_t col, uint8_t row)
{
  t6963_setAddressPointer(t6963_GH + col + (t6963_GA * row));
}

//-------------------------------------------------------------------------------------------------
//
// Writes display data and increment address pointer
//
//-------------------------------------------------------------------------------------------------

void t6963_writeDisplay(uint8_t data)
{
  t6963_writeData(data);
  t6963_writeCommand(T6963_DATA_WRITE_AND_INCREMENT);
}

//-------------------------------------------------------------------------------------------------
//
// Read display data
//
//-------------------------------------------------------------------------------------------------

uint8_t t6963_readDisplay(void)
{
  uint8_t tmp;
  
  t6963_writeCommand(T6963_DATA_READ_AND_NONVARIABLE);
  tmp = t6963_readData();
  
  return tmp;
}

//-------------------------------------------------------------------------------------------------
//
// Writes text (ASCII code) to display RAM memory
//
//-------------------------------------------------------------------------------------------------

void t6963_writeChar(char charCode)
{
  t6963_writeDisplay(charCode - 32);
}

void t6963_writeString(char * string)
{
  while(*string)
  {
    t6963_writeChar(*string++);
  }
}

void t6963_writeStringPgm(prog_char * string)
{
  char charCode;
  while((charCode = pgm_read_byte(string++)))
  {
    t6963_writeChar(charCode);
  }
}

//-------------------------------------------------------------------------------------------------
//
// Set/clear a single pixel at x,y (in pixels)
//
//-------------------------------------------------------------------------------------------------

void t6963_setPixel(uint8_t x, uint8_t y)
{
  uint8_t col = x / t6963_FW;
  uint8_t tmp = T6963_BIT_SET;
  
  t6963_gotoGraphic(col, y);
  
  tmp |= ((col + 1) * t6963_FW) - x - 1;
  t6963_writeCommand(tmp);
}

void t6963_clearPixel(uint8_t x, uint8_t y)
{
  uint8_t col = x / t6963_FW;
  uint8_t tmp = T6963_BIT_RESET;
  
  t6963_gotoGraphic(col, y);
  
  tmp |= ((col + 1) * t6963_FW) - x - 1;
  t6963_writeCommand(tmp);
}

//-------------------------------------------------------------------------------------------------
//
// Clears display RAM memory
//
//-------------------------------------------------------------------------------------------------

void t6963_clearText(void)
{
  uint16_t i;
  
  t6963_setAddressPointer(t6963_TH);
  
  for(i = 0; i < t6963_sizeTA; i++)
  {
    t6963_writeDisplay(0);
  }
}

void t6963_clearGraphic(void)
{
  uint16_t i;
  
  t6963_setAddressPointer(t6963_GH);
  
  for(i = 0; i < t6963_sizeGA; i++)
  {
    t6963_writeDisplay(0);
  }
}

void t6963_clearCG(void)
{
  uint16_t i;
  uint16_t address = ((t6963_memSize / 2) - 1) * 0x800;
  
  t6963_setAddressPointer(address);
  
  for(i = 0; i < 256 * 8; i++)
  {
    t6963_writeDisplay(0);
  }
}

//-------------------------------------------------------------------------------------------------
//
// Display initalization
//
//-------------------------------------------------------------------------------------------------

void t6963_init(uint8_t w, uint8_t h, uint8_t fw, uint8_t mem)
{
  t6963_horizPix = w;
  t6963_vertPix = h;
  t6963_FW = fw;
  t6963_FH = 8;
  t6963_memSize = constrain(mem, 0, 64);
  
  GLCD_DATA_DDR1 |= GLCD_DATA_MASK1;
  GLCD_DATA_DDR2 |= GLCD_DATA_MASK2;

  GLCD_CTRL_DDR |= ((1 << GLCD_RD) | (1 << GLCD_CE) | (1 << GLCD_CD));
  GLCD_CTRL_PORT |= ((1 << GLCD_RD) | (1 << GLCD_CE) | (1 << GLCD_CD));
  
  t6963_TA = w / t6963_FW;
  t6963_GA = w / t6963_FW;
  
  t6963_sizeTA = t6963_TA * (h >> 3);
  t6963_sizeGA = t6963_GA * h;
  
  t6963_TH = 0;
  t6963_GH = t6963_TH + t6963_sizeTA;
  
  //Set Text home address
  t6963_writeData(t6963_TH & 0xFF);
  t6963_writeData(t6963_TH >> 8);
  t6963_writeCommand(T6963_SET_TEXT_HOME_ADDRESS);

  //Set Text Area
  t6963_writeData(t6963_TA);
  t6963_writeData(0x00);
  t6963_writeCommand(T6963_SET_TEXT_AREA);
  
  //Set Graphics Home Address
  t6963_writeData(t6963_GH & 0xFF);
  t6963_writeData(t6963_GH >> 8);
  t6963_writeCommand(T6963_SET_GRAPHIC_HOME_ADDRESS);

  //Set Graphics Area
  t6963_writeData(t6963_GA);
  t6963_writeData(0x00);
  t6963_writeCommand(T6963_SET_GRAPHIC_AREA);

  //Set Internal CGRAM address
  t6963_writeData(((t6963_memSize / 2) - 1));
  t6963_writeData(0x00);
  t6963_writeCommand(T6963_SET_OFFSET_REGISTER);
  
  t6963_setMemMode(T6963_MODE_XOR, T6963_MODE_INTERNAL);
  t6963_setDispMode((1<<T6963_DISPLAY_TEXT) | (1<<T6963_DISPLAY_GRAPHIC) | (0<<T6963_DISPLAY_CURSOR) | (0<<T6963_DISPLAY_BLINK));
  
  t6963_clearText();
  t6963_clearGraphic();
  t6963_clearCG();
  
  delay(100);
}

