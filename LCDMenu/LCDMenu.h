/*
LCDMenu.h - Library for showing Menu on LCDs
Original Author: CWAL
License: Just leave this header, do anything else with it you want

*/
#ifndef LCDMenu_h
#define LCDMenu_h

#include "WProgram.h"
#include <../Menu/Menu.h>
#include <../LiquidCrystal/LiquidCrystal.h>


class LCDMenu
{
private:
LiquidCrystal * lcd;
Menu * rootMenu;
Menu * curMenu;

int charx;       //How many characters in a row;
int chary;       //How many characters in a column
int prow;        //How many items per row

int curloc;      //Which Menu item we are on in curMenu
int scroll;      //How far down in the Menu we are scrolled

boolean offset;  //Should the items be offset from the cursor?
boolean showBack;//Should a software back button be used?
boolean cursor;  //0=Underline,1=Block

int gx(int which);//Returns the x location of Menu item #which
int gy(int which);//Returns the y location of Menu item #which

public:
//Class assumes 20x4 lcd with 2 rows, software back button, block cursor
//Unless otherwise specified in constructor or by calling options
LCDMenu(Menu &r,LiquidCrystal &d);
LCDMenu(Menu &r,LiquidCrystal &d,int pr);
LCDMenu(Menu &r,LiquidCrystal &d,int pr,int cx,int cy);

void options(int pr,boolean o,boolean b,boolean c);//Sets PerRow, Offset, Software Back Button, and Cursor type

void setCursor();//Puts the cursor where it should be
void doScroll();//Makes sure we're scrolled to the right place
void goUp();//Move cursor up
void goDown();//Move cursor down
void goLeft();//Move cursor left
void goRight();//Move cursor right
void goBack();//Move to the parent Menu
void goEnter();//Activate the Menu under the cursor
void goMenu(Menu &m);//Go to Menu m
void display();//Display the current menu on the LCD

};

#endif //LCDMenu_h
