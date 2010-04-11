/*

Declare library authors


*/

#ifndef T6963_h
#define T6963_h

#include "WProgram.h"
#include "inttypes.h"
#include "avr/io.h"
#include "avr/pgmspace.h"
#include "util/delay.h"
#include "T6963_Commands.h"

static uint8_t t6963_FW;
static uint8_t t6963_FH;

void n_delay(void);

uint8_t t6963_readStatus(void);
uint8_t t6963_readData(void);
void t6963_writeCommand(uint8_t);
void t6963_writeData(uint8_t);

void t6963_setMemMode(uint8_t, uint8_t);
void t6963_setDispMode(uint8_t);
void t6963_setAddressPointer(uint16_t);
void t6963_setCursorPointer(uint8_t, uint8_t);
void t6963_setCursorPattern(uint8_t);

void t6963_gotoText(uint8_t, uint8_t);
void t6963_gotoGraphic(uint8_t, uint8_t);

void t6963_writeDisplay(uint8_t);
uint8_t t6963_readDisplay(void);

void t6963_writeChar(char);
void t6963_writeString(char *);
void t6963_writeStringPgm(prog_char *);

void t6963_setPixel(uint8_t, uint8_t);
void t6963_clearPixel(uint8_t, uint8_t);

void t6963_clearText(void);
void t6963_clearGraphic(void);
void t6963_clearCG(void);

void t6963_init(uint8_t, uint8_t, uint8_t, uint8_t);

#endif