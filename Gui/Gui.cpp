/*
	Library for a gui interface by Ian T Metcalf
		tested with the Arduino IDE v18 on a Duemilanova 328
	
	All works by ITM are released under the creative commons attribution share alike license
		http://creativecommons.org/licenses/by-sa/3.0/
	
	I can be contacted at metcalfbuilt@gmail.com
*/



//*************************************************************************************************
//	Libraries
//*************************************************************************************************

#include "Gui.h"


// Calibration Points
const uint8_t calibPts[3][2] = {{24, 13}, {120, 114}, {215, 63}};









//-------------------------------------------------------------------------------------------------
//
// Set the current scheme
//
//	Input	*scheme: pointer to an array of elements
//			size: number of elements in scheme
//
//	Output	none
//	
//-------------------------------------------------------------------------------------------------

void Gui::setScheme(Element *scheme, uint8_t size)
{
	_scheme = scheme;
	_size = size;
}

//-------------------------------------------------------------------------------------------------
//
// Draw the current scheme
//
//	Input	none
//
//	Output	none
//	
//-------------------------------------------------------------------------------------------------

void Gui::draw(void)
{
	uint8_t count;
	
	for (count = 0; count < _size; count++)
	{
		uint8_t w, h;
		
		switch (_scheme[count].type)
		{
			default:
			case GUI_LABEL:
				LCD.textTo(_scheme[count].col, _scheme[count].row);
				LCD.text(_scheme[count].string);
				break;
				
			case GUI_BUTTON:
				w = strlen(_scheme[count].string) * FONT_WIDTH;
				h = FONT_HEIGHT;
				
				LCD.moveTo(_scheme[count].col * FONT_WIDTH, _scheme[count].row * FONT_HEIGHT + 2);
				LCD.rect(w + 10, h + 10, 2);
				
				LCD.textTo(_scheme[count].col + 1, _scheme[count].row + 1);
				LCD.text(_scheme[count].string);
				
				break;
				
			case GUI_ARROW:
				LCD.moveTo(_scheme[count].col * FONT_WIDTH + 24, _scheme[count].row * FONT_HEIGHT + 2);
				LCD.textTo(_scheme[count].col + 5, _scheme[count].row + 1);
				LCD.rect(22, 18, 4);
				LCD.text("UP");
				
				LCD.move(-22, 16);
				LCD.rect(22, 18, 4);
				LCD.text(-5, 2);
				LCD.text("<");
				
				LCD.move(41, 0);
				LCD.rect(22, 18, 4);
				LCD.text(6, 0);
				LCD.text(">");
				
				LCD.move(-22, 16);
				LCD.rect(22, 18, 4);
				LCD.text(-5, 2);
				LCD.text("DN");
				
				break;
		}
	}
}

//-------------------------------------------------------------------------------------------------
//
// Display a message on the screen
//
//	Input	*string: message to display
//
//	Output	none
//	
//-------------------------------------------------------------------------------------------------

void Gui::message(char *string)
{
	uint8_t length, col, row;
	
	length = strlen(string);
	
	if (length & 0x01)
	{
		length++;
	}
	
	col = (SCREEN_COLS - length) / 2;
	row = SCREEN_ROWS / 2;
	
	LCD.moveTo((col - 1) * FONT_WIDTH - 2, (row - 1) * FONT_HEIGHT);
	LCD.rect(length * FONT_WIDTH + 16, FONT_WIDTH + 16);
	
	LCD.textTo(col, row);
	LCD.text(string);
}














//-------------------------------------------------------------------------------------------------
//
// Buffer the current touch value and get flags
//
//	Input	flags: the flags to check
//
//	Output	the flags checked
//	
//-------------------------------------------------------------------------------------------------

uint8_t Gui::touch(uint8_t flags)
{
	cli();
	
	_touchBuffer = Touchscreen_Data;
	_touchBuffer.Flag_Register &= flags;
	Touchscreen_Data.Flag_Register ^= _touchBuffer.Flag_Register;
	
	sei();
	
	return _touchBuffer.Flag_Register;
}

//-------------------------------------------------------------------------------------------------
//
// Calibrate the the touch screen
//
//	Input	none
//
//	Output	none
//	
//-------------------------------------------------------------------------------------------------

void Gui::calibrate(void)
{
	uint8_t count;
	uint16_t touchPts[3][2];
	int32_t divid;
	
	LCD.clearText();
	LCD.clearGraph();
	message("Calibrate");
	
	for (count = 0; count < 3; count++)
	{
		LCD.moveTo(calibPts[count][0] - 5, calibPts[count][1] - 5);
		LCD.rect(10, 10);
		
		while (!touch(1 << FLAG_REGISTER_COORDINATES));
		while (!touch(1 << FLAG_REGISTER_END));
		
		touchPts[count][0] = _touchBuffer.x_pos;
		touchPts[count][1] = _touchBuffer.y_pos;
		
		LCD.moveTo(calibPts[count][0] - 5, calibPts[count][1] - 5);
		LCD.setColor(0);
		LCD.rect(10, 10);
		LCD.setColor(1);
		
		_delay_ms(100);
	}
	
	/*
	
	K = (TX0 - TX2) * (TY1 - TY2) - (TX1 - TX2) * (TY0 - TY2)
	
	A = (DX0 - DX2) * (TY1 - TY2) - (DX1 - DX2) * (TY0 - TY2)
	
	B = (TX0 - TX2) * (DX1 - DX2) - (DX0 - DX2) * (TX1 - TX2)
	
	C = (TX2 * DX1 - TX1 * DX2) * TY0 + (TX0 * DX2 - TX2 * DX0) * TY1 + (TX1 * DX0 - TX0 * DX1) * TY2
	
	D = (DY0 - DY2) * (TY1 - TY2) - (DY1 - DY2) * (TY0 - TY2)
	
	E = (TX0 - TX2) * (DY1 - DY2) - (DY0 - DY2) * (TX1 - TX2)
	
	F = (TX2 * DY1 - TX1 * DY2) * TY0 + (TX0 * DY2 - TX2 * DY0) * TY1 + (TX1 * DY0 - TX0 * DY1) * TY2
	
	*/
	
	divid =	(((int32_t)touchPts[0][0] - touchPts[2][0]) * ((int32_t)touchPts[1][1] - touchPts[2][1])) -
			(((int32_t)touchPts[1][0] - touchPts[2][0]) * ((int32_t)touchPts[0][1] - touchPts[2][1]));
	
	if (divid != 0)
	{
		_calib.An =	(((int32_t)calibPts[0][0] - calibPts[2][0]) * ((int32_t)touchPts[1][1] - touchPts[2][1])) -
					(((int32_t)calibPts[1][0] - calibPts[2][0]) * ((int32_t)touchPts[0][1] - touchPts[2][1]));
		
		_calib.Bn =	(((int32_t)touchPts[0][0] - touchPts[2][0]) * ((int32_t)calibPts[1][0] - calibPts[2][0])) -
					(((int32_t)calibPts[0][0] - calibPts[2][0]) * ((int32_t)touchPts[1][0] - touchPts[2][0]));
		
		_calib.Cn =	(((int32_t)touchPts[2][0] * calibPts[1][0]) - ((int32_t)touchPts[1][0] * calibPts[2][0])) * touchPts[0][1] +
					(((int32_t)touchPts[0][0] * calibPts[2][0]) - ((int32_t)touchPts[2][0] * calibPts[0][0])) * touchPts[1][1] +
					(((int32_t)touchPts[1][0] * calibPts[0][0]) - ((int32_t)touchPts[0][0] * calibPts[1][0])) * touchPts[2][1];
		
		_calib.Dn =	(((int32_t)calibPts[0][1] - calibPts[2][1]) * ((int32_t)touchPts[1][1] - touchPts[2][1])) -
					(((int32_t)calibPts[1][1] - calibPts[2][1]) * ((int32_t)touchPts[0][1] - touchPts[2][1]));
		
		_calib.En =	(((int32_t)touchPts[0][0] - touchPts[2][0]) * ((int32_t)calibPts[1][1] - calibPts[2][1])) -
					(((int32_t)calibPts[0][1] - calibPts[2][1]) * ((int32_t)touchPts[1][0] - touchPts[2][0]));
		
		_calib.Fn =	(((int32_t)touchPts[2][0] * calibPts[1][1]) - ((int32_t)touchPts[1][0] * calibPts[2][1])) * touchPts[0][1] +
					(((int32_t)touchPts[0][0] * calibPts[2][1]) - ((int32_t)touchPts[2][0] * calibPts[0][1])) * touchPts[1][1] +
					(((int32_t)touchPts[1][0] * calibPts[0][1]) - ((int32_t)touchPts[0][0] * calibPts[1][1])) * touchPts[2][1];
		
		_calib.divider = divid;
		
		eeprom_write_block((const void*)&_calib, (void*)GUI_CALIB_EEPROM_ADDR, sizeof(CALIBRATE));
	}
	
	LCD.clearText();
	LCD.clearGraph();
}

//-------------------------------------------------------------------------------------------------
//
// Check for a touch event
//
//	Input	none
//
//	Output	0 no event
//			1 press
//	
//-------------------------------------------------------------------------------------------------

uint8_t Gui::getTouch(void)
{
	touch((1 << FLAG_REGISTER_END) | (1 << FLAG_REGISTER_COORDINATES));
	
	if (TestBit(_touchBuffer.Flag_Register, FLAG_REGISTER_END))
	{
		drag = 0;
	}
	else if (TestBit(_touchBuffer.Flag_Register, FLAG_REGISTER_COORDINATES))
	{
		drag = 1;
	}
	
	if (TestBit(_touchBuffer.Flag_Register, FLAG_REGISTER_COORDINATES))
	{
		touchX = ((_calib.An * _touchBuffer.x_pos) + (_calib.Bn * _touchBuffer.y_pos) + _calib.Cn) / _calib.divider;
		touchY = ((_calib.Dn * _touchBuffer.x_pos) + (_calib.En * _touchBuffer.y_pos) + _calib.Fn) / _calib.divider;
		
		return 1;
	}
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
// Check if the touch point is within the box
//
//	Input	bx, by: top left corner of the box
//			bw, bh: width and height of the box
//
//	Output	element number or 0
//	
//-------------------------------------------------------------------------------------------------

uint8_t Gui::checkBox(uint8_t bx, uint8_t by, uint8_t bw, uint8_t bh)
{
	return (touchX > bx && touchX < (bx + bw) && touchY > by && touchY < (by + bh)) ? 1 : 0;
}


//-------------------------------------------------------------------------------------------------
//
// Check if touch point is within an element of the current scheme
//
//	Input	none
//
//	Output	element number or 0
//	
//-------------------------------------------------------------------------------------------------

uint8_t Gui::checkScheme(void)
{
	uint8_t ex, ey, count, current;
	
	current = 0;
	
	for (count = 0; count < _size; count++)
	{
		ex = _scheme[count].col * FONT_WIDTH;
		ey = _scheme[count].row * FONT_HEIGHT;
		
		switch (_scheme[count].type)
		{
			case GUI_BUTTON:
				if (checkBox(ex, ey + 2, strlen(_scheme[count].string) * FONT_WIDTH + 11, FONT_HEIGHT + 9))
				{
					return current + 1;
				}
				
				current++;
				break;
				
			case GUI_ARROW:
				if (checkBox(ex + 24, ey + 2, 23, 19))
				{
					return current + 1;
				}
				else if (checkBox(ex + 3, ey + 18, 23, 19))
				{
					return current + 2;
				}
				else if (checkBox(ex + 45, ey + 18, 23, 19))
				{
					return current + 3;
				}
				else if (checkBox(ex + 24, ey + 34, 23, 19))
				{
					return current + 4;
				}
				current += 4;
				break;
		}
	}
	
	return 0;
}


























//-------------------------------------------------------------------------------------------------
//
// Gui initalization
//
//	Input	none
//
//	Output	none
//
//-------------------------------------------------------------------------------------------------


void Gui::init(Element *scheme, uint8_t size)
{
	_scheme = scheme;
	_size = size;
	
	drag = 0;
	touchX = 0;
	touchY = 0;
	
	eeprom_read_block((void*)&_calib, (const void*)GUI_CALIB_EEPROM_ADDR, sizeof(CALIBRATE));
	
	Touchscreen_Init();
	sei();
	
	touch(0xFF);
}



















//*************************************************************************************************
//	Constructor
//*************************************************************************************************

Gui::Gui()
{
}


//*************************************************************************************************
//	Preinstantiate object
//*************************************************************************************************

Gui GUI = Gui();






