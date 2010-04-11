/*
  Node - Library for a Linked List.
  Created by Ian Metcalf, March 16, 2010.
  Based on the Menu Library created by CWAL
  Released into the public domain.
*/

#ifndef Node_h
#define Node_h

#include "WProgram.h"

#define TOGGLE_BIT 0 // Toggle or instant button
#define STATE_BIT 1  // Current state of button
#define GRID_BIT 4   // Indicates whether measurments are in pix/col&row

class Node
{
  public:
    uint8_t num;
    uint8_t pos_x;
    uint8_t pos_y;
    uint8_t width;
    uint8_t height;
    uint8_t flags;
    
    Node(char *);
    
    void addChild(Node&);
    Node * getParent(void);
    Node * getSibling(uint8_t);
    Node * getChild(uint8_t);
    
    char * getName(void);
    uint8_t getLength(void);
    
    void setCallBack(void (*function)(Node&));
    void setDisplay(void (*function)(Node&));
    
    void checkAction(uint8_t, uint8_t);
    void showNodes(uint8_t);
    void showNodes(void);
    
    void init(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
    
  private:
    char * name;
    uint8_t length;
    
    Node * parent;
    Node * sibling;
    Node * child;
    
    void (*callback)(Node&);
    void (*display)(Node&);
    
    void setParent(Node&);
    void addSibling(Node&, Node&);
};

#endif