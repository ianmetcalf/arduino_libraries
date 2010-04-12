/*
  Grid - Library for arduino lcd gui.
  Created by Ian Metcalf, March 16, 2010.
  Released into the public domain.
*/

#include "Elem.h"

//#define COL_PIXELS 6
//#define ROW_PIXELS 8

#define TOGGLE_BIT 0
#define STATE_BIT 1

//uint8_t btn[2][3][3] = {{{0x07, 0x3F, 0x38},
//                          {0x08, 0x00, 0x04},
//                          {0x10, 0x00, 0x02}},
//                         {{0x07, 0x3F, 0x38},
//                          {0x0F, 0x3F, 0x3C},
//                          {0x1F, 0x3F, 0x3E}}};

// Constructors ////////////////////////////////////////////////////////////////

Elem::Elem(uint8_t _num, char *n)
{
  num = _num;
  name = n;
}

// Private Methods //////////////////////////////////////////////////////////////

void Elem::setParent(Elem &p)
{
  parent = &p;
}

void Elem::addSibling(Elem &s, Elem &p)
{
  if (sibling)
  {
    sibling->addSibling(s, p);
  }
  else
  {
    sibling = &s;
    sibling->setParent(p);
  }
}

// Public Methods //////////////////////////////////////////////////////////////

void Elem::addChild(Elem &c)
{
  if (child)
  {
    child->addSibling(c,*this);
  }
  else
  {
    child = &c;
    child->setParent(*this);
  }
}

Elem * Elem::getParent(void)
{
  if (parent)
  {
    return parent;
  }
  else //Top Menu
  {
    return this;
  }
}

Elem * Elem::getSibling(uint8_t which)
{
  if (which == 0)
  {
    return this;
  }
  else if (sibling)
  {
    return sibling->getSibling(which - 1);
  }
  else //Asking for a nonexistent sibling
  {
    return NULL;
  }
}

Elem * Elem::getChild(uint8_t which)
{
  if (child)
  {
    return child->getSibling(which);
  }
  else //This Menu item has no children
  {
    return NULL;
  }
}

void Elem::checkAction(uint8_t x, uint8_t y)
{
  if (x > (col * t6963_FW) && x < ((col + width) * t6963_FW) && y > (row * t6963_FH) && y < ((row + height) * t6963_FH))
  {
    if (settings & (1 << TOGGLE_BIT))
    {
      settings ^= (1 << STATE_BIT);
    }
    else
    {
      settings |= (1 << STATE_BIT);
    }
    
    display(0);
    
    if (callback)
    {
      callback(*this);
    }
    
    delay(100);
    
    if (!(settings & (1 << TOGGLE_BIT)))
    {
      settings &= ~(1 << STATE_BIT);
      display(0);
    }
  }
  else if (sibling)
  {
    sibling->checkAction(x, y);
  }
}

void Elem::setToggle(uint8_t toggle)
{
  if (toggle)
  {
    settings |= (1 << TOGGLE_BIT);
  }
  else
  {
    settings &= ~(1 << TOGGLE_BIT);
  }
}

void Elem::setCallBack(void (*function)(Elem&))
{
  callback = function;
}

void Elem::display(uint8_t cascade)
{
  /*
  uint16_t GA = lcd->getGA();
  uint16_t address = lcd->getGH() + col + (GA * row * ROW_PIXELS);
  uint8_t h = height * ROW_PIXELS;
  uint8_t t = width - 2;
  uint8_t state = (settings >> STATE_BIT) & 0x01;
  
  lcd->setAddressPointer(lcd->getTH() + text_col + (lcd->getTA() * text_row));
  lcd->writeString(name);
 
  lcd->setAddressPointer(address);
  lcd->writeLine(width, btn[state][0][0], btn[1][0][0], btn[state][0][1], btn[state][0][2], btn[1][0][2]);
  lcd->setAddressPointer(address + GA);
  lcd->writeLine(width, btn[state][1][0], btn[1][1][0], btn[state][1][1], btn[state][1][2], btn[1][1][2]);
  
  uint8_t i;
  for (i = 2; i < h - 2; i++)
  {
    lcd->setAddressPointer(address + (GA * i));
    lcd->writeLine(width, btn[state][2][0], btn[1][2][0], btn[state][2][1], btn[state][2][2], btn[1][2][2]);
  }
  
  lcd->setAddressPointer(address + (GA * (h - 2)));
  lcd->writeLine(width, btn[state][1][0], btn[1][1][0], btn[state][1][1], btn[state][1][2], btn[1][1][2]);
  lcd->setAddressPointer(address + (GA * (h - 1)));
  lcd->writeLine(width, btn[state][0][0], btn[1][0][0], btn[state][0][1], btn[state][0][2], btn[1][0][2]);
  */
  
  drawButton();
  
  if (sibling && cascade > 0)
  {
    sibling->display(cascade - 1);
  }
}

void Elem::init(uint8_t c, uint8_t r, uint8_t w, uint8_t h)
{
  uint8_t text_size = strlen(name);
  
  col = text_col = c;
  row = text_row = r;
  width = w;
  height = h;
  callback = NULL;
  
  if (width < (text_size + 2))
  {
    width = text_size + 2;
  }
  
  if (height < 3)
  {
    height = 3;
  }
  
  text_col += (width - text_size) >> 1;
  text_row += (height- 1) >> 1;
}


void Elem::drawButton(void)
{
  uint8_t r = row * t6963_FH;
  uint8_t h = height * t6963_FH;
  
  uint8_t left_byte = 0x00;
  uint8_t fill_byte = 0x00;
  uint8_t rght_byte = 0x00;
  
  uint8_t state = 0x00;
  if (settings & (1 << STATE_BIT))
  {
    state = 0x3F;
  }
  
  uint8_t i = 0;
  uint8_t j;
  while (i < h)
  {
    if (i < 2 || i > (h - 3))
    {
      left_byte = 0x00;
      fill_byte = 0x00;
      rght_byte = 0x00;
    }
    else if (i == 2 || i == (h - 3))
    {
      left_byte = 0x07;
      fill_byte = 0x3F;
      rght_byte = 0x38;
    }
    else if (i == 3 || i == (h - 4))
    {
      left_byte = 0x08;
      fill_byte = state;
      rght_byte = 0x04;
    }
    else
    {
      left_byte = 0x10;
      fill_byte = state;
      rght_byte = 0x02;
    }
    
    t6963_gotoGraphic(col, r + i);
    
    t6963_writeDisplay(left_byte);
    for (j = 0; j < width - 2; j++)
    {
      t6963_writeDisplay(fill_byte);
    }
    t6963_writeDisplay(rght_byte);
    
    i++;
  }
  
  t6963_gotoText(text_col, text_row);
  t6963_writeString(name);
}




