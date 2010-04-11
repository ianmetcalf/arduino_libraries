#include <LiquidCrystal.h>
#include <Menu.h>
#include <LCDMenu.h>

LiquidCrystal lcd(2,-1,3,4,5,6,7);

Menu top("Root");
LCDMenu Root(top,lcd);

Menu Item1("Something");
Menu Item11("Stuff");
Menu Item12("More");
Menu Item121("Deeper");
Menu Item2("Other");
Menu Item3("Etc");
Menu Item31("So On");

void menuinit()
{
top.addChild(Item1);
top.addChild(Item2);
top.addChild(Item3);
Item1.addChild(Item11);
Item1.addChild(Item12);
Item12.addChild(Item121);
Item3.addChild(Item31);
Root.display();
}

//UP,DOWN,LEFT,RIGHT,ENTER,BACK
int but[6]={8,9,10,11,12,13};
//Previous States of buttons
boolean pbut[6]={0,0,0,0,0,0};

void buttoncheck()
{
for (int i=0;i<5;i++)
 {
 if (digitalRead(but[i]))
  {
  if (pbut[i]==0)
   {
   button(i);
   pbut[i]=1;
   }
  }
 else
  {
  pbut[i]=0;
  }
 }
}

void button(int which)
{
switch (which)
 {
 case 0://UP
  Root.goUp();
 break;
 case 1://DOWN
  Root.goDown();
 break;
 case 2://LEFT
  Root.goLeft();
 break;
 case 3://RIGHT
  Root.goRight();
 break;
 case 4://ENTER
  Root.goEnter();
 break;
 case 5://BACK
  Root.goBack();
 break;
 }
}

void setup()
{
menuinit();
}

void loop()
{
buttoncheck();
}
