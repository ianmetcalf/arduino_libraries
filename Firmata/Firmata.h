/*
  Firmata.h - Firmata library
  Copyright (C) 2006-2008 Hans-Christoph Steiner.  All rights reserved.
 
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.
*/

#ifndef Firmata_h
#define Firmata_h

#include <WProgram.h>
#include <inttypes.h>


/* Version numbers for the protocol.  The protocol is still changing, so these
 * version numbers are important.  This number can be queried so that host
 * software can test whether it will be compatible with the currently
 * installed firmware. */
#define FIRMATA_MAJOR_VERSION   2 // for non-compatible changes
#define FIRMATA_MINOR_VERSION   1 // for backwards compatible changes

#define MAX_DATA_BYTES 32 // max number of data bytes in non-Sysex messages

// message command bytes (128-255/0x80-0xFF)
#define DIGITAL_MESSAGE         0x90 // send data for a digital pin
#define ANALOG_MESSAGE          0xE0 // send data for an analog pin (or PWM)
#define REPORT_ANALOG           0xC0 // enable analog input by pin #
#define REPORT_DIGITAL          0xD0 // enable digital input by port pair
//
#define SET_PIN_MODE            0xF4 // set a pin to INPUT/OUTPUT/PWM/etc
//
#define REPORT_VERSION          0xF9 // report protocol version
#define SYSTEM_RESET            0xFF // reset from MIDI
//
#define START_SYSEX             0xF0 // start a MIDI Sysex message
#define END_SYSEX               0xF7 // end a MIDI Sysex message

// extended command set using sysex (0-127/0x00-0x7F)
/* 0x00-0x0F reserved for user-defined commands */
#define SERVO_CONFIG            0x70 // set max angle, minPulse, maxPulse, freq
#define STRING_DATA             0x71 // a string message with 14-bits per char
#define SHIFT_DATA              0x75 // a bitstream to/from a shift register
#define I2C_REQUEST             0x76 // send an I2C read/write request
#define I2C_REPLY               0x77 // a reply to an I2C read request
#define I2C_CONFIG				0x78 // config I2C settings such as delay times and power pins
#define REPORT_FIRMWARE         0x79 // report name and version of the firmware
#define SAMPLING_INTERVAL       0x7A // set the poll rate of the main loop
#define SYSEX_NON_REALTIME      0x7E // MIDI Reserved for non-realtime messages
#define SYSEX_REALTIME          0x7F // MIDI Reserved for realtime messages
// these are DEPRECATED to make the naming more consistent
#define FIRMATA_STRING          0x71 // same as STRING_DATA
#define SYSEX_I2C_REQUEST       0x76 // same as I2C_REQUEST
#define SYSEX_I2C_REPLY         0x77 // same as I2C_REPLY
#define SYSEX_SAMPLING_INTERVAL 0x7A // same as SAMPLING_INTERVAL

// pin modes
//#define INPUT                 0x00 // defined in wiring.h
//#define OUTPUT                0x01 // defined in wiring.h
#define ANALOG                  0x02 // analog pin in analogInput mode
#define PWM                     0x03 // digital pin in PWM output mode
#define SERVO                   0x04 // digital pin in Servo output mode
#define SHIFT                   0x05 // shiftIn/shiftOut mode
#define I2C                     0x06 // pin included in I2C setup

extern "C" {
// callback function types
    typedef void (*callbackFunction)(byte, int);
    typedef void (*systemResetCallbackFunction)(void);
    typedef void (*stringCallbackFunction)(char*);
    typedef void (*sysexCallbackFunction)(byte command, byte argc, byte*argv);
}


// TODO make it a subclass of a generic Serial/Stream base class
class FirmataClass
{
public:
	FirmataClass();
/* Arduino constructors */
    void begin();
    void begin(long);
/* querying functions */
	void printVersion(void);
    void blinkVersion(void);
    void printFirmwareVersion(void);
  //void setFirmwareVersion(byte major, byte minor);  // see macro below
    void setFirmwareNameAndVersion(const char *name, byte major, byte minor);
/* serial receive handling */
    int available(void);
    void processInput(void);
/* serial send handling */
	void sendAnalog(byte pin, int value);
	void sendDigital(byte pin, int value); // TODO implement this
	void sendDigitalPort(byte portNumber, int portData);
    void sendString(const char* string);
    void sendString(byte command, const char* string);
	void sendSysex(byte command, byte bytec, byte* bytev);
/* attach & detach callback functions to messages */
    void attach(byte command, callbackFunction newFunction);
    void attach(byte command, systemResetCallbackFunction newFunction);
    void attach(byte command, stringCallbackFunction newFunction);
    void attach(byte command, sysexCallbackFunction newFunction);
    void detach(byte command);

private:
/* firmware name and version */
    byte firmwareVersionCount;
    byte *firmwareVersionVector;
/* input message handling */
    byte waitForData; // this flag says the next serial input will be data
    byte executeMultiByteCommand; // execute this after getting multi-byte data
    byte multiByteChannel; // channel data for multiByteCommands
    byte storedInputData[MAX_DATA_BYTES]; // multi-byte data
/* sysex */
    boolean parsingSysex;
    int sysexBytesRead;
/* callback functions */
    callbackFunction currentAnalogCallback;
    callbackFunction currentDigitalCallback;
    callbackFunction currentReportAnalogCallback;
    callbackFunction currentReportDigitalCallback;
    callbackFunction currentPinModeCallback;
    systemResetCallbackFunction currentSystemResetCallback;
    stringCallbackFunction currentStringCallback;
    sysexCallbackFunction currentSysexCallback;

/* private methods ------------------------------ */
    void processSysexMessage(void);
	void systemReset(void);
    void pin13strobe(int count, int onInterval, int offInterval);
};

extern FirmataClass Firmata;

/*==============================================================================
 * MACROS
 *============================================================================*/

/* shortcut for setFirmwareNameAndVersion() that uses __FILE__ to set the
 * firmware name.  It needs to be a macro so that __FILE__ is included in the
 * firmware source file rather than the library source file.
 */
#define setFirmwareVersion(x, y)   setFirmwareNameAndVersion(__FILE__, x, y)

// total number of pins currently supported
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) // Arduino NG and Diecimila
#define TOTAL_ANALOG_PINS       8
#define TOTAL_DIGITAL_PINS      22 // 14 digital + 8 analog
#define TOTAL_PORTS             3 // total number of ports for the board
#define ANALOG_PORT             2 // port# of analog used as digital
#define FIRST_ANALOG_PIN        14 // pin# corresponding to analog 0
#define VERSION_BLINK_PIN       13 // digital pin to blink version on
#define FIRST_SERVO_PIN         2 // pin# of the first servo pin
#elif defined(__AVR_ATmega8__)  // old Arduinos
#define TOTAL_ANALOG_PINS       6
#define TOTAL_DIGITAL_PINS      20 // 14 digital + 6 analog
#define TOTAL_PORTS             3  // total number of ports for the board
#define ANALOG_PORT             2  // port# of analog used as digital
#define FIRST_ANALOG_PIN        14 // pin# corresponding to analog 0
#define VERSION_BLINK_PIN       13 // digital pin to blink version on
#define FIRST_SERVO_PIN         2 // pin# of the first servo pin
#elif defined(__AVR_ATmega1280__)// Arduino Mega
#define TOTAL_ANALOG_PINS       16
#define TOTAL_DIGITAL_PINS      70 // 54 digital + 16 analog
#define TOTAL_PORTS             9 // total number of ports for the board
#define ANALOG_PORT             8 // port# of analog used as digital
#define FIRST_ANALOG_PIN        54 // pin# corresponding to analog 0
#define VERSION_BLINK_PIN       13 // digital pin to blink version on
#define FIRST_SERVO_PIN         2 // pin# of the first servo pin
#elif defined(__AVR_ATmega128__)// Wiring
#define TOTAL_ANALOG_PINS       8
#define TOTAL_DIGITAL_PINS      51
#define TOTAL_PORTS             7 // total number of ports for the board
#define ANALOG_PORT             5 // port# of analog used as digital
#define FIRST_ANALOG_PIN        40 // pin# corresponding to analog 0
#define VERSION_BLINK_PIN       48 // digital pin to blink version on
#define FIRST_SERVO_PIN         8 // pin# of the first servo pin
#elif defined(__AVR_AT90USB162__) // Teensy
#define TOTAL_ANALOG_PINS       0
#define TOTAL_DIGITAL_PINS      21 // 21 digital + no analog
#define TOTAL_PORTS             4 // total number of ports for the board
#define ANALOG_PORT             3 // port# of analog used as digital
#define FIRST_ANALOG_PIN        21 // pin# corresponding to analog 0
#define VERSION_BLINK_PIN       6 // digital pin to blink version on
#elif defined(__AVR_ATmega32U4__) // Teensy
#define TOTAL_ANALOG_PINS       12
#define TOTAL_DIGITAL_PINS      25 // 11 digital + 12 analog
#define TOTAL_PORTS             4 // total number of ports for the board
#define ANALOG_PORT             3 // port# of analog used as digital
#define FIRST_ANALOG_PIN        11 // pin# corresponding to analog 0
#define VERSION_BLINK_PIN       11 // digital pin to blink version on
#elif defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB1286__) // Teensy++
#define TOTAL_ANALOG_PINS       8
#define TOTAL_DIGITAL_PINS      46 // 38 digital + 8 analog
#define TOTAL_PORTS             6 // total number of ports for the board
#define ANALOG_PORT             5 // port# of analog used as digital
#define FIRST_ANALOG_PIN        38 // pin# corresponding to analog 0
#define VERSION_BLINK_PIN       6 // digital pin to blink version on
#elif defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644__)  // Sanguino
#define TOTAL_ANALOG_PINS       8
#define TOTAL_DIGITAL_PINS      32 // 24 digital + 8 analog
#define TOTAL_PORTS             4 // total number of ports for the board
#define ANALOG_PORT             3 // port# of analog used as digital
#define FIRST_ANALOG_PIN        24 // pin# corresponding to analog 0
#define VERSION_BLINK_PIN       0 // digital pin to blink version on
#elif defined(__AVR_ATmega645__)  // Illuminato
#define TOTAL_ANALOG_PINS       6
#define TOTAL_DIGITAL_PINS      42 // 36 digital + 6 analog
#define TOTAL_PORTS             6 // total number of ports for the board
#define ANALOG_PORT             4 // port# of analog used as digital
#define FIRST_ANALOG_PIN        36 // pin# corresponding to analog 0
#define VERSION_BLINK_PIN       13 // digital pin to blink version on
#else // anything else
#define TOTAL_ANALOG_PINS       6
#define TOTAL_DIGITAL_PINS      14
#define TOTAL_PORTS             3 // total number of ports for the board
#define ANALOG_PORT             2 // port# of analog used as digital
#define FIRST_ANALOG_PIN        14 // pin# corresponding to analog 0
#define VERSION_BLINK_PIN       13 // digital pin to blink version on
#endif



#endif /* Firmata_h */

