/*
    Copyright (C) 2011-2012 William Brodie-Tyrrell
    william@brodie-tyrrell.org
  
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of   
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

#ifndef _FSTOPCOMMS_H_
#define _FSTOPCOMMS_H_

#include <Arduino.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include "EEPROMLayout.h"

/**
 * Serial communication state-machine
 */

class FstopComms {

  // comms settings & timing
  static const int COM_BAUD=9600;
  static const unsigned long COM_TIMEOUT=400e3;
  static const unsigned long COM_CMDTIMEOUT=50e3;
  static const unsigned long COM_LCDTIMEOUT=1e6;
  static const unsigned long COM_PERIOD=100e3;

  // communication protocol bytes
  static const char COM_KEEPALIVE=0x80;
  static const char COM_READ=0x81;
  static const char COM_WRITE=0x82;
  static const char COM_READACK=0x91;
  static const char COM_WRITEACK=0x92;
  static const char COM_NAK=0x9F;
  static const char COM_CHKFAIL=0x9E;

  static const int PKT_CMD=0;
  static const int PKT_LEN=1;
  static const int PKT_ADDR=2;

  static const int PKT_MAXREQ=64;
  static const int PKT_SHORTHDR=4;// cmd, len, addr*2
  static const int PKT_HEADER=5;  // cmd, len, addr*2, checksum
  static const int PKT_BUFFER=PKT_MAXREQ+PKT_HEADER;

  static const int EEPROM_MIN_READ=0x0000;
  static const int EEPROM_MIN_WRITE=EE_CONFIGTOP;
  static const int EEPROM_MAX_READ=EE_TOP;
  static const int EEPROM_MAX_WRITE=EE_TOP;

  static const char *BAD_WRITE;
  static const char *BAD_READ;
  static const char *CONNECTED;
  static const char *CHECKSUM_FAIL;

public:

  FstopComms(LiquidCrystal &l);

  /// initialise port
  void begin();
  /// reset communication state-machine
  void reset();
  /// inspect port and talk
  /// @return true if connection closed
  bool poll();

private:

  /// char received in poll()
  void rx(char ch);
  /// send a char
  void tx(char ch);
  /// send buffer
  void txCmd();
  /// reject request and reset state machine but no disconnect
  void nak(const char *s);
  /// show error message
  void error(const char *s);

  /// compute xor of cmd[0..buflen-1];
  char checksum();
  /// validate checksum and reset or disconnect
  /// @return true if checksum is OK
  bool checkcheck();

  /// get address (2-byte) from packet
  unsigned int getAddr();

  void respondRead();
  void respondWrite();

  LiquidCrystal &disp;
  unsigned long lastrx, lasttx, lastlcd;  ///< times of recent events
  bool incmd, connected;                  ///< connection state
  char cmd[PKT_BUFFER];                   ///< data buffer
  unsigned char bufwant;                  ///< how much we want in the buffer
  unsigned char buflen;                   ///< how much is in the buffer
};

#endif