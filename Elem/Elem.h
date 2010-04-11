/*
  Elem - Library for arduino lcd gui.
  Created by Ian Metcalf, March 16, 2010.
  Based on the Menu Library created by CWAL
  Released into the public domain.
*/

#ifndef Elem_h
#define Elem_h

#include "WProgram.h"

class Elem
{
  private:
    Elem * parent;
    Elem * sibling;
    Elem * child;
    
    uint8_t width;
    uint8_t height;
    uint8_t settings;
    void (*callback)(Elem&);
    
    void setParent(Elem &p);
    void addSibling(Elem &s, Elem &p);
    
  public:
    char * name;
	uint8_t num;
    uint8_t col;
    uint8_t row;
	uint8_t text_col;
	uint8_t text_row;
    
    Elem(uint8_t _num, char *n);
    
    void addChild(Elem &c);
    Elem * getParent(void);
    Elem * getSibling(uint8_t which);
    Elem * getChild(uint8_t which);
	
	void setToggle(uint8_t toggle);
    
    void checkAction(uint8_t x, uint8_t y);
    void setCallBack(void (*function)(Elem&));
    void display(uint8_t cascade);
    
    void init(uint8_t _col, uint8_t _row, uint8_t w, uint8_t h);
    
    void drawButton(void);
};

#endif