/*
Menu.h - Library for creating nested Menus with callback functions
Original Author: CWAL
License: Just leave this header, do anything else with it you want

*/
#ifndef Menu_h
#define Menu_h

#include "WProgram.h"


class Menu
{
private:
 Menu * parent;//Parent Menu, NULL if this is the top
 Menu * child;//First Child Menu, NULL if no children
 Menu * sibling;//Next Sibling Menu, NULL if this is the last sibling

 void setParent(Menu &p);//Sets the Menu's Parent to p
 void addSibling(Menu &s,Menu &p);//Adds a Sibling s with Parent p.  If the Menu already has a sibling, ask that sibling to add it
public:
 char *name;//Name of this Menu
 boolean (*canEnter)(Menu&);//Function to be called when this menu is 'used'

 Menu(char *n);//Constructs the Menu with a name and a NULL use function (be careful calling it)
 Menu(char *n,boolean (*c)(Menu&));//Constructs the Menu with a specified use function
 void addChild(Menu &c);//Adds the child c to the Menu.  If the menu already has a child, ask the child to add it as a sibling
 Menu * getChild(int which);//Returns a pointer to the which'th child of this Menu
 Menu * getSibling(int howfar);//Returns a pointer to the sibling howfar siblings away from this Menu
 Menu * getParent();//Returns this Menu's parent Menu.  If no parent, returns itself
};

#endif //Menu_h
