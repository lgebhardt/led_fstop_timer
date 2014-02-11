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

#ifndef _EXECUTOR_H_
#define _EXECUTOR_H_

#include <Arduino.h>
#include <LiquidCrystal.h>
#include "Keypad.h"
#include "LEDDriver.h"
#include "Program.h"

class Executor {
public:
  Executor(LiquidCrystal &d, Keypad &k, ButtonDebounce &b, ButtonDebounce &fs, LEDDriver &led);

  void begin();

  /// must call this to define what will be execed
  /// @param p the current program
  void setProgram(Program *p);

  /// set drydown indication
  /// @param d whether to indicate that drydown is applied
  void setDrydown(bool d);

  Program *getProgram() const { 
    return current; 
  }

  /// define which exposure step to do
  void changePhase(unsigned char ph);
  
  unsigned char getPhase() const { 
    return execphase; 
  }

  /// move onto next phase
  void nextPhase();

  /// do a controlled exposure
  void expose();

private:    

  /// program we're working on
  Program *current;

  LiquidCrystal &disp;
  Keypad &keys;
  ButtonDebounce &button;
  ButtonDebounce &footswitch;
  LEDDriver &leddriver;
  char dispbuf[21];

  bool dd;
  bool sg;

  /// program phase about to be executed
  unsigned char execphase;
};

#endif