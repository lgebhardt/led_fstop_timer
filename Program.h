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

#ifndef _PROGRAM_H_
#define _PROGRAM_H_

#include <Arduino.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include "Paper.h"
#include "LEDDriver.h"

/**
 * Definition of a program of exposures
 */
class Program {
  static const int INVALID=0x0000;
  static const int SLOTBITS=7; // Allocates 128 bytes per slot
  static const int SLOTBASE=0x80;
  static const int TEXTLEN=18;
  static const long MAXMS=999999L;    // ceiling of 1000s

public:

  static const int MAXSTEPS=8;
  static const int MAXEXPOSURES=MAXSTEPS;
  static const int FIRSTSLOT=1;
  static const int LASTSLOT=7;

  /// a single exposure step
  class Step {
	public:

      /// render the step settings to the screen (time and text)
      /// @param lin also display linear time (millis)
      void display(LiquidCrystal &disp, char *buf, bool lin);
      /// rended only the time line (bottom row);
      void displayTime(LiquidCrystal &disp, char *buf, bool lin);
      void displayGrade(LiquidCrystal &disp, char *buf, bool lin);

      int stops;               // fixed-point, 1/100ths of a stop
      unsigned char grade;     // grade, ISO Exposure Scale
      bool hard;               // perform the hard exposure
      bool soft;               // perform the soft exposure
      char text[TEXTLEN+1];    // description (only TEXTLEN(18) bytes written to EEPROM)
  };

  class Exposure {
    public:
      /// render the exposure settings to the screen (time and text)
      /// @param lin also display linear time (millis)
      void display(LiquidCrystal &disp, char *buf, bool lin);
      /// rended only the time line (bottom row);
      void displayTime(LiquidCrystal &disp, char *buf, bool lin);
      void displayGrade(LiquidCrystal &disp, char *buf, bool lin);
      unsigned long ms;        // milliseconds to expose (post-compilation, not saved)
      unsigned char hardpower; //power for hard step, 0 is full, 255 is off
      unsigned char softpower; //power for soft step, 0 is full, 255 is off
      Step* step; 
  };

  /// clear all entries, leaving base exposure of 0 stops
  void clear();
  void clearExposures();

  /// step accessor
  Step &getStep(int which);
  Exposure &getExposure(int which);

  /// determine number of valid exposure objects; first = base
  // unsigned char getCount();

  /// convert a program from stops to linear time so that it can be execed
  bool compile(char dd, Paper& p);

  /// save to EEPROM
  /// @param slot slot-number in 1..7
  void save(int slot);

  /// load from EEPROM
  /// @param slot slot-number in 1..7
  bool load(int slot);

  /// configure the program as a test strip;
  /// is assumed to compile after this.
  void configureStrip(int base, int step, bool cover, unsigned char grade, bool inithard, bool stephard, bool initsoft, bool stepsoft, Paper& p);

private:

  int slotAddr(int slot);
  void compileStripIndiv(char dd, Paper& p);
  void compileStripCover(char dd, Paper& p);
  bool compileNormal(char dd, Paper& p);
  void clipExposures();

  /// convert hundredths-of-stops to milliseconds
  static unsigned long hunToMillis(int hunst);

  // compilation settings
  bool isstrip, cover;

  /// first step is base, rest as dodges/burns
  Step steps[MAXSTEPS];
  Exposure exposures[MAXEXPOSURES];
};

#endif