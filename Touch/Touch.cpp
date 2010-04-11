/*
  Touch - Library for 7843 touch controller.
  Created by Jeruwit Supa, Dec 12, 2009
  www.circuitidea.com
*/

extern "C"
{
  #include <inttypes.h>
  #include <avr/io.h>
  #include <avr/eeprom.h>
}

#include "WProgram.h"
#include "Touch.h"

// SPI Speed:
// 3 = every /128 clock
// 2 = every /64 clock
// 1 = every /16 clock
// 0 = every /4 clock
#define TS_SPI_SPEED 2
#define TS_SPI_DOUBLE 0 //Double SPI Speed: 0 = disable; 1 = enable

#define TS_SPI_DDR  DDRB
#define TS_SPI_PORT PORTB
#define TS_SPI_PIN  PINB

#define TS_SS_BIT   2
#define TS_MOSI_BIT 3
#define TS_MISO_BIT 4
#define TS_SCK_BIT  5

#define TS_CS_DDR   DDRD
#define TS_CS_PORT  PORTD
#define TS_CS_PIN   PIND
#define TS_CS_BIT   2


// Constructors ////////////////////////////////////////////////////////////////

Touch::Touch(void)
{
  execute = 0;
  getMatrix();
}


// Private Methods //////////////////////////////////////////////////////////////

uint8_t Touch::readADC(uint8_t command)
{
  SPDR = command;
  while(!(SPSR & (1<<SPIF)));
  return (SPDR);
}

void Touch::readTouch(POINT * pADC_point)
{
  uint16_t x,y;
  uint8_t buf_data[4];
  
  TS_CS_PORT &= ~(1<<TS_CS_BIT);
  
  buf_data[0] = readADC(0xD0); // Write command to measure x-position
  buf_data[0] = readADC(0x00); // Read ADC data (7-bit high byte)
  buf_data[1] = readADC(0x00); // Read ADC data (5-bit low byte)
  
  buf_data[2] = readADC(0x90); // Write command to measure y-position
  buf_data[2] = readADC(0x00); // Read ADC data (7-bit high byte)
  buf_data[3] = readADC(0x00); // Read ADC data (5-bit low byte)
  
  TS_CS_PORT |= (1<<TS_CS_BIT);
  
  x = (uint16_t)buf_data[0] << 5;
  x |= (uint16_t)buf_data[1] >> 3;
  
  y = (uint16_t)buf_data[2] << 5;
  y |= (uint16_t)buf_data[3] >> 3;
  
  pADC_point->x = x >> 2;
  pADC_point->y = (4095L-y) >> 2;
}

void Touch::sampling(POINT * pScreen, uint8_t nTest)
{
  POINT adcPoint;
  
  long avgX, avgY;
  
  uint8_t i;
  uint8_t nInvalid;
  
  uint8_t regSPCR;
  uint8_t regSPSR;
  
  SPCR = (1<<SPE) | (1<<MSTR) | ((TS_SPI_SPEED>>1)<<SPR1) | ((TS_SPI_SPEED & 0x01)<<SPR0);
  SPSR = (TS_SPI_DOUBLE<<SPI2X);
  
  pScreen->x = 0;
  pScreen->y = 0;
  
  readTouch(&adcPoint);
  if (adcPoint.y < ADC_MIN_Y)
  {
    SPCR = regSPCR;
    SPSR = regSPSR;
    return;
  }
  
  avgX = adcPoint.x;
  avgY = adcPoint.y;
  
  nInvalid = 0;
  i = nTest;
  do
  {
    if (++nInvalid > 20)
    {
      break;
    }
    
    readTouch(&adcPoint);
    if (adcPoint.y < ADC_MIN_Y)
    {
      continue;
    }
    
    avgX += adcPoint.x;
    avgX >>= 1;
    avgY += adcPoint.y;
    avgY >>= 1;
    
    nInvalid = 0;
  } while (--i);
  
  SPCR = regSPCR;
  SPSR = regSPSR;
  
  if (nInvalid)
  {
    return;
  }
  
  pScreen->x = avgX;
  pScreen->y = avgY;
}

void Touch::screenToDisplay(void)
{
  displayPoint.x = ((matrix.An * screenPoint.x) + (matrix.Bn * screenPoint.y) + matrix.Cn) / matrix.Divider;
  displayPoint.y = ((matrix.Dn * screenPoint.x) + (matrix.En * screenPoint.y) + matrix.Fn) / matrix.Divider;
}


// Public Methods //////////////////////////////////////////////////////////////

void Touch::getMatrix(void)
{
  eeprom_read_block((void*)&matrix, (const void*)EE_MATRIX_ADDR, sizeof(MATRIX));
}

void Touch::setMatrix(void)
{
  eeprom_write_block((const void*)&matrix, (void*)EE_MATRIX_ADDR, sizeof(MATRIX));
}

void Touch::calibrateMatrix(POINT * lcdPtr, POINT * touchPtr)
{
  long tmp = ((touchPtr[0].x - touchPtr[2].x) * (touchPtr[1].y - touchPtr[2].y)) -
             ((touchPtr[1].x - touchPtr[2].x) * (touchPtr[0].y - touchPtr[2].y));
  
  if (tmp)
  {
    matrix.An = ((lcdPtr[0].x - lcdPtr[2].x) * (touchPtr[1].y - touchPtr[2].y)) -
				((lcdPtr[1].x - lcdPtr[2].x) * (touchPtr[0].y - touchPtr[2].y));
    
    matrix.Bn = ((touchPtr[0].x - touchPtr[2].x) * (lcdPtr[1].x - lcdPtr[2].x)) -
				((lcdPtr[0].x - lcdPtr[2].x) * (touchPtr[1].x - touchPtr[2].x));
    
    matrix.Cn = (touchPtr[2].x * lcdPtr[1].x - touchPtr[1].x * lcdPtr[2].x) * touchPtr[0].y +
				(touchPtr[0].x * lcdPtr[2].x - touchPtr[2].x * lcdPtr[0].x) * touchPtr[1].y +
				(touchPtr[1].x * lcdPtr[0].x - touchPtr[0].x * lcdPtr[1].x) * touchPtr[2].y;
    
    matrix.Dn = ((lcdPtr[0].y - lcdPtr[2].y) * (touchPtr[1].y - touchPtr[2].y)) -
				((lcdPtr[1].y - lcdPtr[2].y) * (touchPtr[0].y - touchPtr[2].y));
    
    matrix.En = ((touchPtr[0].x - touchPtr[2].x) * (lcdPtr[1].y - lcdPtr[2].y)) -
                ((lcdPtr[0].y - lcdPtr[2].y) * (touchPtr[1].x - touchPtr[2].x));
    
    matrix.Fn = (touchPtr[2].x * lcdPtr[1].y - touchPtr[1].x * lcdPtr[2].y) * touchPtr[0].y +
                (touchPtr[0].x * lcdPtr[2].y - touchPtr[2].x * lcdPtr[0].y) * touchPtr[1].y +
                (touchPtr[1].x * lcdPtr[0].y - touchPtr[0].x * lcdPtr[1].y) * touchPtr[2].y;
    
    matrix.Divider = tmp;
    setMatrix();
  }
}

void Touch::readPoint(void)
{
  sampling(&screenPoint, TOUCH_SAMPLE);
  if ((screenPoint.x == 0) || (screenPoint.y == 0))
  {
    displayPoint.x = displayPoint.y = 0;
  }
  else
  {
    screenToDisplay();
  }
  
  if (displayPoint.x<0)
  {
    displayPoint.x = 0;
  }
  if (displayPoint.y<0)
  {
    displayPoint.y = 0;
  }  
}

void Touch::action(void)
{
  if (millis()-previous >= interval)
  {
    previous = millis();
    
    sampling(&screenPoint, ACTION_SAMPLE);
    
    if (screenPoint.x > 0)
    {
      screenToDisplay();
      
      if (duration)
      {
        samplePoint.x = (samplePoint.x + displayPoint.x) >> 1;
        samplePoint.y = (samplePoint.y + displayPoint.y) >> 1;
      }
      else
      {
        samplePoint.x = displayPoint.x;
        samplePoint.y = displayPoint.y;
      }
      duration++;
      
      if (duration > ACTION_DURATION)
      {
        duration = ACTION_DURATION;
        displayPoint.x = samplePoint.x;
        displayPoint.y = samplePoint.y;
        
        if (execute)
        {
          execute();
        }
        return;
      }
      return;
    }
    duration = 0;
  }
}

void Touch::callBack(uint16_t intervl, void (*function)())
{
  duration = 0;
  previous = 0;
  interval = intervl;
  execute = function;
}

void Touch::init(void)
{
  uint8_t temp;
  
  TS_CS_PORT |= (1<<TS_CS_BIT);
  TS_CS_DDR |= (1<<TS_CS_BIT);
  
  TS_SPI_DDR |= (1<<TS_SCK_BIT) | (1<<TS_MOSI_BIT);
  if (!(TS_SPI_DDR & (1<<TS_SS_BIT)))
  {
    TS_SPI_DDR |= (1<<TS_SS_BIT);
  }
  TS_SPI_PORT |= (1<<TS_MISO_BIT);
  
  SPCR = (1<<SPE) | (1<<MSTR) | ((TS_SPI_SPEED>>1)<<SPR1) | ((TS_SPI_SPEED & 0x01)<<SPR0);
  SPSR = (TS_SPI_DOUBLE<<SPI2X);
  temp = SPSR;
}

//-------------------------------------------------------------------------------------------------
//
// Preinstantiate Object
//
//-------------------------------------------------------------------------------------------------

Touch touch = Touch();






