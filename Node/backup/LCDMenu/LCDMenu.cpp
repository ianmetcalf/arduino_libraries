/*
LCDMenu.h - Library for showing Menu on LCDs
Original Author: CWAL
License: Just leave this header, do anything else with it you want

*/


//#include "WProgram.h"
#include "LCDMenu.h"


int LCDMenu::gx(int which)
{
return (which%prow)*(charx/prow);
}

int LCDMenu::gy(int which)
{
return which/prow;
}


LCDMenu::LCDMenu(Menu &r,LiquidCrystal &d)
{
rootMenu=&r;
curMenu=rootMenu;
lcd=&d;
prow=2;
charx=20;
chary=4;
options(prow,1,1,1);
}

LCDMenu::LCDMenu(Menu &r,LiquidCrystal &d,int pr)
{
rootMenu=&r;
curMenu=rootMenu;
lcd=&d;
prow=pr;
charx=20;
chary=4;
options(prow,1,1,1);
}

LCDMenu::LCDMenu(Menu &r,LiquidCrystal &d,int pr,int cx,int cy)
{
rootMenu=&r;
curMenu=rootMenu;
lcd=&d;
prow=pr;
charx=cx;
chary=cy;
options(prow,1,1,1);
}

void LCDMenu::options(int pr,boolean o,boolean b,boolean c)
{
prow=pr;
offset=o;
showBack=b;
cursor=c;
}

void LCDMenu::setCursor()
{
if (cursor)
 {
 lcd->command(0x0F);
 }
else
 {
 lcd->command(0x0E);
 }
lcd->setCursor(gx(curloc),gy(curloc)-scroll);
}

void LCDMenu::doScroll()
{
if (curloc<0)
 {
 curloc=0;
 }
else
 {
 while (curloc>0&&!curMenu->getChild(curloc-showBack))//Only allow it to go up to Menu item (one more if back button enabled)
  {
  curloc--;
  }
 }
int wherey=gy(curloc);
if (wherey>=(chary+scroll))
 {
 scroll++;
 display();
 }
else if (wherey<(scroll))
 {
 scroll--;
 display();
 }
else
 {
 setCursor();
 }
}

void LCDMenu::goUp()
{
curloc-=prow;
doScroll();
}

void LCDMenu::goDown()
{
curloc+=prow;
doScroll();
}

void LCDMenu::goLeft()
{
curloc--;
doScroll();
}

void LCDMenu::goRight()
{
curloc++;
doScroll();
}

void LCDMenu::goBack()
{
goMenu(*curMenu->getParent());
}

void LCDMenu::goEnter()
{
Menu *tmp;
tmp=curMenu;
if ((tmp=tmp->getChild(curloc)))//The child exists
 {
 if (tmp->canEnter)//canEnter function is set
  {
  if (tmp->canEnter(*tmp))//It wants us to enter
   {
   goMenu(*tmp);
   }
  }
 else//canEnter function not set, assume entry allowed
  {
  goMenu(*tmp);
  }
 }
else//Child did not exist  The only time this should happen is one the back Menu item, so go back
 {
 goBack();
 }
}

void LCDMenu::goMenu(Menu &m)
{
curMenu=&m;
curloc=0;
display();
}

void LCDMenu::display()
{
Menu * tmp;
int i=scroll*prow;
int maxi=((chary+scroll)*prow);
lcd->clear();
if ((tmp=curMenu->getChild(i)))
 {
 do
  {
  lcd->setCursor(gx(i)+offset,gy(i)-scroll);
  lcd->print(tmp->name);
  i++;
  }
 while ((tmp=tmp->getSibling(1))&&i<maxi);
 if ((i<=maxi)&&showBack) //We stopped before the end of the lcd, so draw the back button if needed
  {
  lcd->setCursor(gx(i)+offset,gy(i)-scroll);
  lcd->print("Back");  
  }
 }
else //Menu has no children
 {
 lcd->setCursor(0,0);
 lcd->print("No Children");
 }
setCursor();
}


