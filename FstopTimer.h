/* -*- C++ -*- */

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

#ifndef _FSTOP_TIMER_H_
#define _FSTOP_TIMER_H_

#include <Arduino.h>
#include "RotaryEncoder.h"
#include "Keypad.h"
#include "Fstopcomms.h"
#include "Executor.h"

/**
 * State-machine implementing fstop timer
 *
 */
class FstopTimer {
  // bounds of exposure
  static const int MAXSTOP=999;
  static const int MINSTOP=-800;

  // bounds of grade
  static const int MAXGRADE=50;
  static const int MINGRADE=-10;

public:

  static const char *VERSION;
  static const char VERSIONCODE=4;

  enum {
    ST_SPLASH,
    ST_MAIN,
    ST_EDIT,
    ST_EDIT_EV,
    ST_EDIT_TEXT,
    ST_EXEC,
    ST_FOCUS,
    ST_IO,
    ST_IO_LOAD,
    ST_IO_SAVE,
    ST_COMMS,
    ST_TEST,
    ST_TEST_CHANGEB,
    ST_TEST_CHANGES,
    ST_CONFIG,
    ST_CONFIG_DRY,
    ST_CONFIG_ROTARY,
    ST_COUNT
  };

  /// constructor
  /// @param l display to write to
  /// @param k keypad to read from
  /// @param r rotary encoder for exposure changes
  /// @param b button to start/stop exposures
  /// @param p_e exposure pin (high = on)
  /// @param p_p beep pin (not used much)
  /// @param p_bl backlight pin, connect via BC548
  FstopTimer(LiquidCrystal &l, SMSKeypad &k, RotaryEncoder &r,
  ButtonDebounce &b,
  LEDDriver &led,
  char p_b, char p_bi);

  /// setup IO
  void begin();

  /// continue exucution of current state, ponder input
  void poll();

private:

  char inbuf[21];
  char dispbuf[21];

  LiquidCrystal &disp;
  SMSKeypad &keys;
  RotaryEncoder &rotary;
  ButtonDebounce &button;
  SMSKeypad::Context smsctx;
  DecimalKeypad deckey;
  FstopComms comms;
  DecimalKeypad::Context expctx;
  DecimalKeypad::Context stepctx;
  DecimalKeypad::Context dryctx;
  DecimalKeypad::Context intctx;

  /// programs to execute
  Program current, strip;
  /// current config for test strips
  int stripbase, stripstep;
  bool stripcover;

  Executor exec;

  /// LCD PWM factor
  int brightness;
  /// whether drydown correction is currently applied
  bool drydown_apply;
  /// drydown factor
  char drydown; 
  /// use split grade exposures
  bool splitgrade;
  /// exposure that is being edited
  int expnum;
  /// exposure change using rotary encoder
  int rotexp;
  /// where we're up to in a program-exec when focusing
  char focusphase;

  /// hardware pin numbers
  char pin_exposebtn, pin_beep, pin_backlight;

  LEDDriver leddriver;

  /// all state-machine functions have this signature
  typedef void (FstopTimer::* voidfunc)();
  static voidfunc sm_enter[ST_COUNT];
  static voidfunc sm_poll[ST_COUNT];

  /// current state
  int curstate;
  int prevstate;

  void errorBeep();

  /// modify a value and clamp it to [MINSTOP, MAXSTOP]
  void clampExposure(int &orig, int delta);

  /**
   * Enter a new state in the state machine
   */
  void changeState(int st);

  /// invert and save the drydown-application bit
  void toggleDrydown();

  /// invert and save the splitgrade bit
  void toggleSplitgrade();

  /// exec the current program
  void execCurrent();

  /// exec the test strip
  void execTest();

  /// change backlight intensity
  void setBacklight();

  /// state-machine body
  void st_splash_enter();
  void st_splash_poll();
  void st_main_enter();
  void st_main_poll();
  void st_exec_enter();
  void st_exec_poll();
  void st_focus_enter();
  void st_focus_poll();
  void st_edit_enter();
  void st_edit_poll();
  void st_edit_ev_enter();
  void st_edit_ev_poll();
  void st_edit_text_enter();
  void st_edit_text_poll();
  void st_io_enter();
  void st_io_poll();
  void st_io_load_enter();
  void st_io_load_poll();
  void st_io_save_enter();
  void st_io_save_poll();
  void st_comms_enter();
  void st_comms_poll();
  void st_test_enter();
  void st_test_poll();
  void st_test_changeb_enter();
  void st_test_changeb_poll();
  void st_test_changes_enter();
  void st_test_changes_poll();
  void st_config_enter();
  void st_config_poll();
  void st_config_dry_enter();
  void st_config_dry_poll();
  void st_config_rotary_enter();
  void st_config_rotary_poll();

  // backlight bounds
  static const char BL_MIN=0;
  static const char BL_MAX=8;
};


#endif // _FSTOP_TIMER_H_

