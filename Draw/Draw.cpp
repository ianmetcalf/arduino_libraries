/*
  Draw - Library for a Graphic LCDs.
  Created by Ian Metcalf, March 26, 2010.
  Based on the API for processing
  Released into the public domain.
*/

#include "Draw.h"

#define STROKE_BIT       0
#define STROKE_COLOR_BIT 1
#define FILL_BIT         2
#define FILL_COLOR_BIT   3

#define CLOSE_SHAPE 1
#define OPEN_SHAPE  0

//-------------------------------------------------------------------------------------------------
//
// Reverse Bits Macro
//	
//-------------------------------------------------------------------------------------------------
/*
#define REVERSEBITS(b)  (BitReverseTable[b])
static uint8_t BitReverseTable[256] =
{
0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
};

#define RIGHTBITMASK(b)  (RightBitMaskTable[b])
static uint8_t RightBitMaskTable[8] = {0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F};

#define LEFTBITMASK(b)  (LeftBitMaskTable[b])
static uint8_t LeftBitMaskTable[8] = {0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE};
*/

//-------------------------------------------------------------------------------------------------
//
// Constructor
//	
//-------------------------------------------------------------------------------------------------

Draw::Draw()
{
}

//-------------------------------------------------------------------------------------------------
//
// Set drawing points
//	
//-------------------------------------------------------------------------------------------------

void Draw::setCurrent(uint16_t x1, uint16_t y1)
{
	curCol = x1 / colWidth;
	curOffset = (colWidth * (curCol + 1)) - x1;
	curY = y1;
}

void Draw::setEnd(uint16_t x1, uint16_t y1)
{
	endCol = x1 / colWidth;
	endOffset = (colWidth * (endCol + 1)) - x1;
	endY = y1;
}

//-------------------------------------------------------------------------------------------------
//
// Draw a horizontal line
//	
//-------------------------------------------------------------------------------------------------

void Draw::horizLine(void)
{
	if (curColor)
	{
		curColor = 0x08;
	}
	
	LCD.gotoGraphic(curCol, curY);
	
	if (curCol == endCol)
	{
		if (curOffset < endOffset)
		{
			curOffset--;
			while (curOffset != endOffset)
			{
				curOffset++;
				LCD.writePixel(curColor | (curOffset - 1));
			}
		}
		else
		{
			curOffset++;
			while (curOffset != endOffset)
			{
				curOffset--;
				LCD.writePixel(curColor | (curOffset - 1));
			}
		}
	}
	else if (curCol < endCol)
	{
		if (curOffset == colWidth)
		{
			if (curColor)
			{
				curColor = 0xFF;
			}
			
			while (curCol != endCol)
			{
				curCol++;
				LCD.writeDispInc(curColor);
			}
		}
		else
		{
			curOffset++;
			while (curOffset != 1)
			{
				curOffset--;
				LCD.writePixel(curColor | (curOffset - 1));
			}
			
			curOffset = colWidth;
			curCol++;
		}
		
		horizLine();
	}
	else
	{
		if (curOffset == 1)
		{
			if (curColor)
			{
				curColor = 0xFF;
			}
			
			while (curCol != endCol)
			{
				curCol--;
				LCD.writeDispDec(curColor);
			}
		}
		else
		{
			curOffset--;
			while (curOffset != colWidth)
			{
				curOffset++;
				LCD.writePixel(curColor | (curOffset - 1));
			}
			
			curOffset = 1;
			curCol--;
		}
		
		horizLine();
	}
}

//-------------------------------------------------------------------------------------------------
//
// Draw a vertical line
//	
//-------------------------------------------------------------------------------------------------

void Draw::vertLine(void)
{
	uint8_t tmp = 0;
	
	if (curColor)
	{
		tmp = 0x08;
	}
	
	tmp |= 0x07 & (curOffset - 1);
	
	if (curY < endY)
	{
		curY--;
		while (curY != endY)
		{
			curY++;
			LCD.gotoGraphic(curCol, curY);
			LCD.writePixel(tmp);
		}
	}
	else
	{
		curY++;
		while (curY != endY)
		{
			curY--;
			LCD.gotoGraphic(curCol, curY);
			LCD.writePixel(tmp);
		}
	}
}

//-------------------------------------------------------------------------------------------------
//
// Draw a diagonal line
//	
//-------------------------------------------------------------------------------------------------

void Draw::diagLine(void)
{
	int8_t inc = 1;
	
	if (curColor)
	{
		curColor = 0x08;
	}
	
	if (curY > endY)
	{
		inc = -1;
	}
	
	if (curCol < endCol || (curCol == endCol && curOffset >> endOffset))
	{
		curOffset++;
		curY -= inc;
		while (curY != endY)
		{
			curOffset--;
			if (curOffset < 1)
			{
				curOffset = colWidth;
				curCol++;
			}
			
			curY += inc;
			
			LCD.gotoGraphic(curCol, curY);
			LCD.writePixel(curColor | (curOffset - 1));
		}
	}
	else
	{
		curOffset--;
		curY -= inc;
		while (curY != endY)
		{
			curOffset++;
			if (curOffset > colWidth)
			{
				curOffset = 1;
				curCol--;
			}
			
			curY += inc;
			
			LCD.gotoGraphic(curCol, curY);
			LCD.writePixel(curColor | (curOffset - 1));
		}
	}
}

//-------------------------------------------------------------------------------------------------
//
// Draw a diagonal line using Bresenham's Line Algorithm
//	
//-------------------------------------------------------------------------------------------------

void Draw::bresenLine(int16_t dx, int16_t dy)
{
	uint16_t absDX = abs(dx);
	uint16_t absDY = abs(dy);
	
	int16_t error = 0;
	int8_t inc = 1;
	
	if (curColor)
	{
		curColor = 0x08;
	}
	
	LCD.gotoGraphic(curCol, curY);
	LCD.writePixel(curColor | (curOffset - 1));
	
	if (absDX > absDY)
	{
		error = (absDX >> 1);
		
		if (dy < 0)
		{
			inc = -1;
		}
		
		if (dx > 0)
		{
			while (curCol != endCol || curOffset != endOffset)
			{
				curOffset--;
				if (curOffset < 1)
				{
					curOffset = colWidth;
					curCol++;
				}
				
				error -= absDY;
				if (error < 0)
				{
					error += absDX;
					curY += inc;
				}
				
				LCD.gotoGraphic(curCol, curY);
				LCD.writePixel(curColor | (curOffset - 1));
			}
		}
		else
		{
			while (curCol != endCol || curOffset != endOffset)
			{
				curOffset++;
				if (curOffset > colWidth)
				{
					curOffset = 1;
					curCol--;
				}
				
				error -= absDY;
				if (error < 0)
				{
					error += absDX;
					curY += inc;
				}
				
				LCD.gotoGraphic(curCol, curY);
				LCD.writePixel(curColor | (curOffset - 1));
			}
		}
	}
	else
	{
		error = (absDY >> 1);
		
		if (dy < 0)
		{
			inc = -1;
		}
		
		if (dx > 0)
		{
			while (curY != endY)
			{
				curY += inc;
				
				error -= absDX;
				if (error < 0)
				{
					error += absDY;
					
					curOffset--;
					if (curOffset < 1)
					{
						curOffset = colWidth;
						curCol++;
					}
				}
				
				LCD.gotoGraphic(curCol, curY);
				LCD.writePixel(curColor | (curOffset - 1));
			}
		}
		else
		{
			while (curY != endY)
			{
				curY += inc;
				
				error -= absDX;
				if (error < 0)
				{
					error += absDY;
					
					curOffset++;
					if (curOffset > colWidth)
					{
						curOffset = 1;
						curCol--;
					}
				}
				
				LCD.gotoGraphic(curCol, curY);
				LCD.writePixel(curColor | (curOffset - 1));
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
//
// Draw a polygon
//
//-------------------------------------------------------------------------------------------------

void Draw::poly(uint8_t start, uint8_t end, uint8_t close)
{
	uint8_t point, tmp;
	int16_t dx, dy;
	
	curColor = (settings & (1 << STROKE_COLOR_BIT));
	
	setCurrent(points[start].x, points[start].y);
	
	point = start + 1;
	
	for (tmp = start; tmp < end; tmp++)
	{
		dx = points[point].x - points[tmp].x;
		dy = points[point].y - points[tmp].y;
		
		setEnd(points[point].x, points[point].y);
		
		if (dy == 0)
		{
			if (dx != 0)
			{
				horizLine();
			}
		}
		else if (dx == 0)
		{
			if (dy != 0)
			{
				vertLine();
			}
		}
		else if (dx == dy)
		{
			diagLine();
		}
		else
		{
			bresenLine(dx, dy);
		}
		
		point++;
		if (point == end)
		{
			if (close)
			{
				point = start;
			}
			else
			{
				tmp++;
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
//
// Fill a polygon
//
//-------------------------------------------------------------------------------------------------

void Draw::fillPoly(uint8_t start, uint8_t end)
{
	int8_t incA, incB;
	uint8_t startCol, startOffset, pointA, pointB, tmp;
	int16_t lastY, dxA, dyA, dxB, dyB, offset, errorA, errorB;
	
	curColor = (settings & (1 << FILL_COLOR_BIT));
	
	lastY = points[start].y;
	pointA = start;
	
	for (tmp = (start + 1); tmp < end; tmp++)
	{
		if (points[tmp].y < points[pointA].y)
		{
			pointA = tmp;
		}
		
		if (points[tmp].y > lastY)
		{
			lastY = points[tmp].y;
		}
	}
	
	pointB = pointA;
	
	setCurrent(points[pointA].x, points[pointA].y);
	setEnd(points[pointA].x, points[pointA].y);
	
	curY++;
	endY = curY;
	
	while (curY != lastY)
	{
		offset = 0;
		if (points[pointA].y < curY)
		{
			tmp = pointA;
			
			do
			{
				pointA++;
				if (pointA == end)
				{
					pointA = start;
				}
				
				dxA = points[pointA].x - points[tmp].x;
				dyA = points[pointA].y - points[tmp].y;
				
				if (dyA == 0)
				{
					offset -= dxA;
					tmp = pointA;
				}
			}
			while (dyA <= 0);
			
			if (dxA < 0)
			{
				dxA *= -1;
				incA = -1;
			}
			else
			{
				incA = 1;
			}
			
			errorA = (dxA > dyA) ? (dxA >> 1) : (dyA >> 1);
		}
		
		if (dxA != 0)
		{
			if (dxA == dyA)
			{
				offset -= incA;
			}
			else if (dxA > dyA)
			{
				while (errorA > 0)
				{
					errorA -= dyA;
					offset -= incA;
				}
				errorA += dxA;
			}
			else
			{
				errorA -= dxA;
				if (errorA < 0)
				{
					errorA += dyA;
					offset -= incA;
				}
			}
		}
		
		offset += curOffset;
		while (offset < 1)
		{
			offset += colWidth;
			curCol++;
		}
		while (offset > colWidth)
		{
			offset -= colWidth;
			curCol--;
		}
		curOffset = offset;
		
		offset = 0;
		if (points[pointB].y < curY)
		{
			tmp = pointB;
			
			do
			{
				if (pointB == start)
				{
					pointB = end;
				}
				pointB--;
				
				dxB = points[pointB].x - points[tmp].x;
				dyB = points[pointB].y - points[tmp].y;
				
				if (dyB == 0)
				{
					offset -= dxB;
					tmp = pointB;
				}
			}
			while (dyB <= 0);
			
			if (dxB < 0)
			{
				dxB *= -1;
				incB = -1;
			}
			else
			{
				incB = 1;
			}
			
			errorB = (dxB > dyB) ? (dxB >> 1) : (dyB >> 1);
		}
		
		if (dxB != 0)
		{
			if (dxB == dyB)
			{
				offset -= incB;
			}
			else if (dxB > dyB)
			{
				while (errorB > 0)
				{
					errorB -= dyB;
					offset -= incB;
				}
				errorB += dxB;
			}
			else
			{
				errorB -= dxB;
				if (errorB < 0)
				{
					errorB += dyB;
					offset -= incB;
				}
			}
		}
		
		offset += endOffset;
		while (offset < 1)
		{
			offset += colWidth;
			endCol++;
		}
		while (offset > colWidth)
		{
			offset -= colWidth;
			endCol--;
		}
		endOffset = offset;
		
		startCol = curCol;
		startOffset = curOffset;
		
		horizLine();
		
		curCol = startCol;
		curOffset = startOffset;
		
		curY++;
	}
}

//-------------------------------------------------------------------------------------------------
//
// Draw an ellipse
//	
//-------------------------------------------------------------------------------------------------

void Draw::midEllipse(int16_t xRadius, int16_t yRadius)
{
	uint8_t tmp, fillCircle, startCol, startOffset;
	int16_t dx, dy, centerY;
	int32_t a2, b2, crit1, crit2, crit3, t, dxt, dyt, d2xt, d2yt;
	
	centerY = (curY + endY) >> 1;
	
	fillCircle = (settings & (1 << FILL_BIT));
	
	if (fillCircle)
	{
		curColor = (settings & (1 << FILL_COLOR_BIT)) ? 0x08 : 0;
		
		startCol = curCol;
		startOffset = curOffset;
		
		curY = endY = centerY;
		horizLine();
		
		curCol = startCol;
		curOffset = startOffset;
	}
	
	if (settings & (1 << STROKE_BIT))
	{
		tmp = (settings & (1 << STROKE_COLOR_BIT)) ? 0x08 : 0;
	}
	else if (fillCircle)
	{
		tmp = curColor;
	}
	else
	{
		return;
	}
	
	dx = xRadius;
	dy = 0;
	
	a2 = (int32_t)xRadius * xRadius;
	b2 = (int32_t)yRadius * yRadius;
	
	crit1 = -(a2 / 4 + xRadius % 2);
	crit2 = crit1 - b2;
	crit3 = -(b2 / 4 + yRadius % 2 + a2);
	
	t = -a2 * dx;
	dxt = -2 * b2 * dx;
	dyt = 2 * a2 * dy;
	
	d2xt = 2 * b2;
	d2yt = 2 * a2;
	
	while (dx)
	{
		LCD.gotoGraphic(curCol, centerY + dy);
		LCD.writePixel(tmp | (curOffset - 1));
		
		LCD.gotoGraphic(endCol, centerY - dy);
		LCD.writePixel(tmp | (endOffset - 1));
		
		if ((t + b2 * dx) > crit1 && (t + a2 * dy) > crit3)
		{
			dx--;
			dxt += d2xt;
			t += dxt;
			
			curOffset--;
			if (curOffset < 1)
			{
				curOffset = colWidth;
				curCol++;
			}
			
			endOffset++;
			if (endOffset > colWidth)
			{
				endOffset = 1;
				endCol--;
			}
		}
		
		if ((t - b2 * dx) <= crit2)
		{
			dy++;
			dyt += d2yt;
			t += dyt;
			
			if (fillCircle)
			{
				startCol = curCol;
				startOffset = curOffset;
				
				curY = endY = centerY + dy;
				horizLine();
				
				curCol = startCol;
				curOffset = startOffset;
				
				curY = endY = centerY - dy;
				horizLine();
				
				curCol = startCol;
				curOffset = startOffset;
			}
			
		}
		
		LCD.gotoGraphic(curCol, centerY - dy);
		LCD.writePixel(tmp | (curOffset - 1));
		
		LCD.gotoGraphic(endCol, centerY + dy);
		LCD.writePixel(tmp | (endOffset - 1));
	}
}

//-------------------------------------------------------------------------------------------------
//
// Draw a circle
//	
//-------------------------------------------------------------------------------------------------

void Draw::midCircle(void)
{
	uint16_t radius, dx, dy;
	int16_t error;
	
	curColor = (settings & (1 << STROKE_COLOR_BIT)) ? 0x08 : 0;
	
	curY = ((curY + endY) >> 1);
	radius = abs(endY - curY);
	dx = radius;
	dy = 0;
	error = radius - 1;
	
	while (dx != dy)
	{
		LCD.gotoGraphic(curCol, curY + dy);
		LCD.writePixel(curColor | (curOffset - 1));
		
		LCD.gotoGraphic(curCol, curY - dy);
		LCD.writePixel(curColor | (curOffset - 1));
		
		LCD.gotoGraphic(endCol, curY + dy);
		LCD.writePixel(curColor | (endOffset - 1));
		
		LCD.gotoGraphic(endCol, curY - dy);
		LCD.writePixel(curColor | (endOffset - 1));
		
		error -= dy;
		dy++;
		error -= dy;
		
		while (error < 0)
		{
			dx--;
			error += dx;
			error += dx;
			
			curOffset--;
			if (curOffset < 1)
			{
				curOffset = colWidth;
				curCol++;
			}
			
			endOffset++;
			if (endOffset > colWidth)
			{
				endOffset = 1;
				endCol--;
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
//
// Set the pen position
//
//-------------------------------------------------------------------------------------------------

void Draw::setPen(int16_t x1, int16_t y1)
{
	x1 = constrain(x1, 0, screenWidth);
	y1 = constrain(y1, 0, screenHeight);
	
	setCurrent(x1, y1);
}

//-------------------------------------------------------------------------------------------------
//
// Draw point
//
//-------------------------------------------------------------------------------------------------

void Draw::point(void)
{
	curColor = (settings & (1 << STROKE_COLOR_BIT)) ? 0x08 : 0;
	
	LCD.gotoGraphic(curCol, curY);
	LCD.writePixel(curColor | (curOffset - 1));
}

void Draw::point(int16_t x1, int16_t y1)
{
	x1 = constrain(x1, 0, screenWidth);
	y1 = constrain(y1, 0, screenHeight);
	
	setCurrent(x1, y1);
	
	point();
}

//-------------------------------------------------------------------------------------------------
//
// Draw line
//
//-------------------------------------------------------------------------------------------------

void Draw::line(int16_t x1, int16_t y1)
{
	int16_t dx, dy;
	
	curColor = (settings & (1 << STROKE_COLOR_BIT));
	
	x1 = constrain(x1, 0, screenWidth);
	y1 = constrain(y1, 0, screenHeight);
	
	setEnd(x1, y1);
	
	dx = x1 - ((colWidth * (curCol + 1)) - curOffset);
	dy = y1 - curY;
	
	if (dy == 0)
	{
		horizLine();
	}
	else if (dx == 0)
	{
		vertLine();
	}
	else if (dx == dy)
	{
		diagLine();
	}
	else
	{
		bresenLine(dx, dy);
	}
}

void Draw::line(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
	int16_t dx, dy;
	
	curColor = (settings & (1 << STROKE_COLOR_BIT));
	
	x1 = constrain(x1, 0, screenWidth);
	y1 = constrain(y1, 0, screenHeight);
	x2 = constrain(x2, 0, screenWidth);
	y2 = constrain(y2, 0, screenHeight);
	
	setCurrent(x1, y1);
	setEnd(x2, y2);
	
	dx = x2 - x1;
	dy = y2 - y1;
	
	if (dy == 0)
	{
		horizLine();
	}
	else if (dx == 0)
	{
		vertLine();
	}
	else if (dx == dy)
	{
		diagLine();
	}
	else
	{
		bresenLine(dx, dy);
	}
}

//-------------------------------------------------------------------------------------------------
//
// Draw rectangle
//
//-------------------------------------------------------------------------------------------------

void Draw::rect(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
	int8_t inc;
	uint8_t startCol, startOffset;
	
	if (x1 == x2 || y1 == y2)
	{
		line(x1, y1, x2, y2);
		return;
	}
	
	x1 = constrain(x1, 0, screenWidth);
	y1 = constrain(y1, 0, screenHeight);
	x2 = constrain(x2, 0, screenWidth);
	y2 = constrain(y2, 0, screenHeight);
	
	if (settings & (1 << FILL_BIT))
	{
		curColor = (settings & (1 << FILL_COLOR_BIT));
		
		inc = (y1 > y2) ? -1 : 1;
		
		setCurrent(x1, y1);
		setEnd(x2, y1);
		
		startCol = curCol;
		startOffset = curOffset;
		
		while (curY != y2)
		{
			curCol = startCol;
			curOffset = startOffset;
			
			horizLine();
			curY += inc;
		}
	}
	
	if (settings & (1 << STROKE_BIT))
	{
		curColor = (settings & (1 << STROKE_COLOR_BIT));
		
		setCurrent(x1, y1);
		
		setEnd(x2, y1);
		horizLine();
		
		setEnd(x2, y2);
		vertLine();
		
		setEnd(x1, y2);
		horizLine();
		
		setEnd(x1, y1);
		vertLine();
	}
}

//-------------------------------------------------------------------------------------------------
//
// Draw a shape
//
//-------------------------------------------------------------------------------------------------

void Draw::beginShape(void)
{
	pointIndex = 0;
}

void Draw::vertex(int16_t x1, int16_t y1)
{
	if (pointIndex < POINT_BUFFER_SIZE)
	{
		points[pointIndex].x = constrain(x1, 0, screenWidth);
		points[pointIndex].y = constrain(y1, 0, screenHeight);
		
		pointIndex++;
	}
}

void Draw::endShape(uint8_t close)
{
	if (settings & (1 << FILL_BIT))
	{
		fillPoly(0, pointIndex);
	}
	
	if (settings & (1 << STROKE_BIT))
	{
		poly(0, pointIndex, close);
	}
}

void Draw::endShape(void)
{
	endShape(OPEN_SHAPE);
}

//-------------------------------------------------------------------------------------------------
//
// Draw a triangle
//
//-------------------------------------------------------------------------------------------------

void Draw::triangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3)
{
	beginShape();
	
	vertex(x1, y1);
	vertex(x2, y2);
	vertex(x3, y3);
	
	endShape(CLOSE_SHAPE);
}

//-------------------------------------------------------------------------------------------------
//
// Draw a quadrilateral
//
//-------------------------------------------------------------------------------------------------

void Draw::quad(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, int16_t x4, int16_t y4)
{
	beginShape();
	
	vertex(x1, y1);
	vertex(x2, y2);
	vertex(x3, y3);
	vertex(x4, y4);
	
	endShape(CLOSE_SHAPE);
}

//-------------------------------------------------------------------------------------------------
//
// Draw a circle
//
//-------------------------------------------------------------------------------------------------

void Draw::ellipse(int16_t cx, int16_t cy, int16_t width, int16_t height)
{
	width >>= 1;
	height >>= 1;
	
	if ((cx - width) >= 0 && (cy - height) >= 0 && (cx + width) < screenWidth && (cy + height) < screenHeight)
	{
		setCurrent(cx - width, cy - height);
		setEnd(cx + width, cy + height);
		
		midEllipse(width, height);
	}
}
	

//-------------------------------------------------------------------------------------------------
//
// Draw a circle
//
//-------------------------------------------------------------------------------------------------

void Draw::circle(int16_t cx, int16_t cy, int16_t radius)
{
	if ((cx - radius) >= 0 && (cy - radius) >= 0 && (cx + radius) < screenWidth && (cy + radius) < screenHeight)
	{
		if (settings & (1 << STROKE_BIT))
		{
			setCurrent(cx - radius, cy - radius);
			setEnd(cx + radius, cy + radius);
			
			midCircle();
		}
	}
}

//-------------------------------------------------------------------------------------------------
//
// Stroke on/off and color
//
//-------------------------------------------------------------------------------------------------

void Draw::stroke(uint8_t color)
{
	settings |= (1 << STROKE_BIT);
	
	if (color)
	{
		settings |= (1 << STROKE_COLOR_BIT);
	}
	else
	{
		settings &= ~(1 << STROKE_COLOR_BIT);
	}
}

void Draw::stroke(void)
{
	settings |= (1 << STROKE_BIT) | (1 << STROKE_COLOR_BIT);
}

void Draw::noStroke(void)
{
	settings &= ~(1 << STROKE_BIT);
}

//-------------------------------------------------------------------------------------------------
//
// Fill on/off and color
//
//-------------------------------------------------------------------------------------------------

void Draw::fill(uint8_t color)
{
	settings |= (1 << FILL_BIT);
	
	if (color)
	{
		settings |= (1 << FILL_COLOR_BIT);
	}
	else
	{
		settings &= ~(1 << FILL_COLOR_BIT);
	}
}

void Draw::fill(void)
{
	settings |= (1 << FILL_BIT) | (1 << FILL_COLOR_BIT);
}

void Draw::noFill(void)
{
	settings &= ~(1 << FILL_BIT);
}

//-------------------------------------------------------------------------------------------------
//
// Draw initalization
//
//-------------------------------------------------------------------------------------------------

void Draw::init(uint16_t w, uint16_t h, uint8_t fw, uint8_t mem)
{
	LCD.init(w, h, fw, mem);
	
	screenWidth = w;
	screenHeight = h;
	colWidth = fw;
	
	settings = (1 << STROKE_BIT) | (1 << STROKE_COLOR_BIT) | (0 << FILL_BIT) | (0 << FILL_COLOR_BIT);
	
	setCurrent(0, 0);
	setEnd(0, 0);
	
	curColor = 0;
}


//-------------------------------------------------------------------------------------------------
//
// Preinstantiate Object
//
//-------------------------------------------------------------------------------------------------

Draw draw = Draw();




