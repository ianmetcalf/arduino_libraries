/*
  Touch - Library for 7843 touch controller.
  Created by Jeruwit Supa, Dec 12, 2009
  www.circuitidea.com
*/

#ifndef Touch_h
#define Touch_h

extern "C"
{
  #include <inttypes.h>
}

#include "WProgram.h"

// Public Define ///////////////////////////////////////////////////////////////
typedef struct Point
{
  long x;
  long y;
} POINT;

typedef struct Matrix
{
  long An;
  long Bn;
  long Cn;
  long Dn;
  long En;
  long Fn;
  long Divider;
} MATRIX;

#define EE_MATRIX_ADDR 0
#define ADC_MIN_Y (50)
#define TOUCH_SAMPLE (250)
#define ACTION_SAMPLE (20)
#define ACTION_DURATION (5)


// Define Macro ////////////////////////////////////////////////////////////////

class Touch
{
  public:
    POINT screenPoint;
    POINT displayPoint;
    
    Touch();
    
    void getMatrix(void);
    void setMatrix(void);
    void calibrateMatrix(POINT * lcdPtr, POINT * touchPtr);
    
    void sampling(POINT * pScreen, uint8_t nTest);
    void readPoint(void);
    
    void action(void);
    void callBack(uint16_t intervl, void (*function)());
    
    void init(void);
    
  private:
    unsigned long previous;
    uint16_t interval;
    uint8_t duration;
    POINT samplePoint;
    
    MATRIX matrix;
    
    uint8_t readADC(uint8_t command);
    void readTouch(POINT * pADC_point);
    void screenToDisplay(void);
    void (*execute)();
};

extern Touch touch;

#endif