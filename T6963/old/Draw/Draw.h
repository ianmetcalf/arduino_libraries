/*
	Draw - Library for a Graphic LCDs.
	Created by Ian Metcalf, March 26, 2010.
	Based on the API for processing
	Released into the public domain.
*/

#ifndef Draw_h
#define Draw_h

#include "WProgram.h"
#include "../T6963/T6963.h"

typedef struct gPoint
{
  int16_t x;
  int16_t y;
} gPOINT;

#define POINT_BUFFER_SIZE 16

class Draw
{
	public:
		Draw();
		
		void setPen(int16_t, int16_t);
		
		void point(void);
		void point(int16_t, int16_t);
		
		void line(int16_t, int16_t);
		void line(int16_t, int16_t, int16_t, int16_t);
		void rect(int16_t, int16_t, int16_t, int16_t);
		
		void beginShape(void);
		void vertex(int16_t, int16_t);
		void endShape(uint8_t);
		void endShape(void);
		
		void triangle(int16_t, int16_t, int16_t, int16_t, int16_t, int16_t);
		void quad(int16_t, int16_t, int16_t, int16_t, int16_t, int16_t, int16_t, int16_t);
		
		void ellipse(int16_t, int16_t, int16_t, int16_t);
		void circle(int16_t, int16_t, int16_t);
		
		//void elipseMode(uint8_t);
		//void rectMode(uint8_t);
		//void arc(uint16_t, uint16_t, uint16_t, uint16_t, uint8_t, uit8_t);
		//void ellipse(uint16_t, uint16_t, uint16_t, uint16_t);
		//void bezier(uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);
		//void curve(uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);
		
		void stroke(uint8_t);
		void stroke(void);
		void noStroke(void);
		
		void fill(uint8_t);
		void fill(void);
		void noFill(void);
		
		void init(uint16_t, uint16_t, uint8_t, uint8_t);
		
	private:
		uint16_t screenWidth;
		uint16_t screenHeight;
		uint8_t colWidth;
		uint8_t settings;
		
		uint8_t curColor;
		
		uint8_t curCol;
		uint8_t curOffset;
		uint16_t curY;
		
		uint8_t endCol;
		uint8_t endOffset;
		uint16_t endY;
		
		gPOINT points[POINT_BUFFER_SIZE];
		uint8_t pointIndex;
		
		void setCurrent(uint16_t, uint16_t);
		void setEnd(uint16_t, uint16_t);
		
		void horizLine(void);
		void vertLine(void);
		void diagLine(void);
		void bresenLine(int16_t, int16_t);
		
		void poly(uint8_t, uint8_t, uint8_t);
		void fillPoly(uint8_t, uint8_t);
		
		void midEllipse(int16_t, int16_t);
		void midCircle(void);
};

extern Draw draw;

#endif