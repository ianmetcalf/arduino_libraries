/*
  Node - Library for a Linked List.
  Created by Ian Metcalf, March 16, 2010.
  Based on the Menu Library created by CWAL
  Released into the public domain.
*/

#include "Node.h"

// Constructors ////////////////////////////////////////////////////////////////

Node::Node(char *n)
{
  name = n;
  length = strlen(n);
  
  callback = NULL;
  display = NULL;
}

// Private Methods //////////////////////////////////////////////////////////////

void Node::setParent(Node &p)
{
  parent = &p;
}

void Node::addSibling(Node &s, Node &p)
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

void Node::addChild(Node &c)
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

Node * Node::getParent(void)
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

Node * Node::getSibling(uint8_t which)
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

Node * Node::getChild(uint8_t which)
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

char * Node::getName(void)
{
  return name;
}

uint8_t Node::getLength(void)
{
  return length;
}

void Node::setCallBack(void (*function)(Node&))
{
  callback = function;
}

void Node::setDisplay(void (*function)(Node&))
{
  display = function;
}


void Node::checkAction(uint8_t x, uint8_t y)
{
  if (x > pos_x && x < (pos_x + width) && y > pos_y && y < (pos_y + height))
  {
    if (flags & (1 << TOGGLE_BIT))
    {
      flags ^= (1 << STATE_BIT);
    }
    else
    {
      flags |= (1 << STATE_BIT);
    }
    showNodes(0);
    
    if (callback)
    {
      callback(*this);
    }
    delay(100);
    
    if (!(flags & (1 << TOGGLE_BIT)))
    {
      flags &= ~(1 << STATE_BIT);
      showNodes(0);
    }
  }
  else if (sibling)
  {
    sibling->checkAction(x, y);
  }
}

void Node::showNodes(uint8_t cascade)
{
  if (display)
  {
    display(*this);
  }
  
  if (sibling && cascade > 0)
  {
    sibling->showNodes(cascade - 1);
  }
}

void Node::showNodes(void)
{
  if (display)
  {
    display(*this);
  }
  
  if (sibling)
  {
    sibling->showNodes();
  }
}

void Node::init(uint8_t n, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t f)
{
  num = n;
  pos_x = x;
  pos_y = y;
  width = w;
  height = h;
  flags = f;
}


