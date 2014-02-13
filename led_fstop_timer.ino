/*
    Based on work Copyright (C) 2011 William Brodie-Tyrrell
    william@brodie-tyrrell.org

	Portions Copyright (C) 2013 Larry Gebhardt
  
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

/*
 * This sketch implements an f/stop enlarger timer and LED light source controller;
 * it has been tested on an Arduino Mega 2560; it will not fit in an UNO or smaller
 */

#ifndef _LED_FSTOP_TIMER
#define _LED_FSTOP_TIMER

#include "Wire.h"
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include "Keypad.h"
#include "RotaryEncoder.h"
#include "FstopTimer.h"
#include <SD.h>
#include "TSL2561.h"

/**
 * Set this according to the vagaries of your rotary encoder
 */
#define ROTARY_REVERSE 1

/**
 * IO pin assignments; change to suit your PCB
 */
#define EXPOSE_CENTER_HARD 12
#define EXPOSE_CENTER_SOFT 11
#define EXPOSE_CORNER_HARD 10
#define EXPOSE_CORNER_SOFT 9
#define SAFELIGHT_RELAY 8


#define EXPOSEBTN 14  // low=depressed (internal pullup); not currently in use
#define FOOTSWITCH 15 // low=depressed (internal pullup); not currently in use
#define BEEP 6        // requires PWM
#define BACKLIGHT 8   // requires PWM; high=on

#define SD_CARD 53

// keypad is 4x4: 3x3 numeric upper left with *0# below that and A-D down the right
/*
// separate pins
#define SCANCOL0 A1    // left column 147*
#define SCANCOL1 A0
#define SCANCOL2 8
#define SCANCOL3 9     // right column ABCD
*/

// for physical keypad pinout, consider LHS (viewed from front) as pin 1
// 1-4 = rows starting at top
// 5-8 = columns starting at left
#define SCANCOL0 38    // left column 147*
#define SCANCOL1 24
#define SCANCOL2 26
#define SCANCOL3 30     // right column ABCD

#define SCANROW0 28    // top row 123A
#define SCANROW1 32
#define SCANROW2 34
#define SCANROW3 36    // bottom row *0#D

// pins for HD44780 in 4-bit mode; RW grounded.
// #define LCDD7 7
// #define LCDD6 6
// #define LCDD5 5
// #define LCDD4 4
// #define LCDEN 9
// #define LCDRS 8

// LCD PINS Using SPI
#define LCD_DATA 42
#define LCD_CLK 40
#define LCD_LATCH 44

// 2 and 3 are taken by the rotary encoder - fixed assignment as they
// require interrupts

/**
 * Instances of static interface objects
 */
// LiquidCrystal disp(0); //I2C
LiquidCrystal disp(LCD_DATA, LCD_CLK, LCD_LATCH); //SPI

SMSKeypad keys(SCANCOL0, SCANCOL1, SCANCOL2, SCANCOL3, SCANROW0, SCANROW1, SCANROW2, SCANROW3);
ButtonDebounce expbtn(EXPOSEBTN);
ButtonDebounce footswitch(FOOTSWITCH);
RotaryEncoder rotary(ROTARY_REVERSE);
LEDDriver leddriver(EXPOSE_CENTER_HARD, EXPOSE_CENTER_SOFT, EXPOSE_CORNER_HARD, EXPOSE_CORNER_SOFT, SAFELIGHT_RELAY);
// all the guts are in this object
TSL2561 tsl(TSL2561_ADDR_FLOAT); 

FstopTimer fst(disp, keys, rotary, expbtn, footswitch, leddriver, tsl, BEEP, BACKLIGHT, SD_CARD);

/**
 * Arduino boot/init function
 */
void setup()
{
  Serial.begin(9600);

   disp.begin(20, 4);
   leddriver.begin();
   keys.begin();
   rotary.begin();
   fst.begin();
   
   tsl.setGain(TSL2561_GAIN_16X);
   tsl.setTiming(TSL2561_INTEGRATIONTIME_402MS);
}

/**
 * Arduino main-program loop.
 * Processes the current state, looks for transitions to other states.
 */
void loop()
{
  fst.poll();
}

#endif
