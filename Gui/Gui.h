/*
	Library for a gui interface by Ian T Metcalf
		tested with the Arduino IDE v18 on a Duemilanova 328
	
	All works by ITM are released under the creative commons attribution share alike license
		http://creativecommons.org/licenses/by-sa/3.0/
	
	I can be contacted at metcalfbuilt@gmail.com
*/


#ifndef GUI_h
#define GUI_h

//*************************************************************************************************
//	Libraries
//*************************************************************************************************

extern "C"
{
	#include <inttypes.h>
	#include <string.h>
	#include <avr/eeprom.h>
	#include <util/delay.h>
	#include "utility/Touchscreen.h"
}

#include <T6963.h>


//*************************************************************************************************
//	Global Definitions
//*************************************************************************************************

#define GUI_CALIB_EEPROM_ADDR	10

// gui element types
#define GUI_LABEL	0
#define GUI_BUTTON	1
#define GUI_ARROW	2



//*************************************************************************************************
//	Global Types
//*************************************************************************************************

typedef struct Calibrate
{
	int32_t An;
	int32_t Bn;
	int32_t Cn;
	int32_t Dn;
	int32_t En;
	int32_t Fn;
	int32_t divider;
}
CALIBRATE;

typedef struct Element
{
	uint8_t type;
	uint8_t col;
	uint8_t row;
	char *string;
}
ELEMENT;



//*************************************************************************************************
//	Class Definition
//*************************************************************************************************

class Gui
{
	public:
		Gui();
		
		uint8_t drag;
		uint8_t touchX;
		uint8_t touchY;
		
		void setScheme(Element*, uint8_t);
		void draw(void);
		void message(char*);
		
		void calibrate(void);
		uint8_t getTouch(void);
		
		uint8_t checkBox(uint8_t, uint8_t, uint8_t, uint8_t);
		uint8_t checkScheme(void);
		
		void init(Element*, uint8_t);
		
	private:
		Element *_scheme;
		uint8_t _size;
		
		Interface _touchBuffer;
		Calibrate _calib;
		
		uint8_t touch(uint8_t);
		
};

extern Gui GUI;

#endif