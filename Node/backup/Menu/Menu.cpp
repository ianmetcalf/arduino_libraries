/*
Menu.cpp - Library for creating nested Menus with callback functions
Original Author: CWAL
License: Just leave this header, do anything else with it you want

*/

//#include "WProgram.h"
#include "Menu.h"

void Menu::setParent(Menu &p)
{
parent=&p;
}

void Menu::addSibling(Menu &s,Menu &p)
{
if (sibling)
 {
 sibling->addSibling(s,p);
}
else
 {
 sibling=&s;
 sibling->setParent(p);
 }
}

Menu::Menu(char *n)
{
name=n;
canEnter=NULL;
}

Menu::Menu(char *n,boolean (*c)(Menu&))
{
name=n;
canEnter=c;
}

void Menu::addChild(Menu &c)
{
if (child)
 {
 child->addSibling(c,*this);
 }
else
 {
 child=&c;
 child->setParent(*this);
 }
}

Menu * Menu::getChild(int which)
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

Menu * Menu::getSibling(int howfar)
{
if (howfar==0)
 {
 return this;
 }
else if (sibling)
 {
 return sibling->getSibling(howfar-1);
 }
else //Asking for a nonexistent sibling
 {
 return NULL;
 }
}

Menu * Menu::getParent()
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

