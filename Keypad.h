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

#ifndef _KEYPAD_H_
#define _KEYPAD_H_

#include <Arduino.h>
#include <LiquidCrystal.h>

/**
 * Debouncing of a single button to generate keypress events.
 */
class ButtonDebounce {
public:

    ButtonDebounce(char p, bool inv=true);

    /// inspect pin
    void scan();

    /// return current pin state
    bool getCurrentState() const;

    /// return true ONCE for each keydown
    bool hadPress();

private:
    char pin;
    bool state, event, invert;
    unsigned long changedat;

    // 20 ms must elapse since keyup before a new keydown will register
    static const unsigned long TIMEOUT=20000;
};


/**
 * 4x4 keypad scan with debouncing & conversion to ASCII
 */
class Keypad {
public:

    /// constructor initialises IO pins
    /// columns are used as outputs, rows are used as inputs
    Keypad(char c0, char c1, char c2, char c3, char r0, char r1, char r2, char r3);

    // raw keycodes
    enum {
        KP_1,
        KP_2,
        KP_3,
        KP_A,
        KP_4,
        KP_5,
        KP_6,
        KP_B,
        KP_7,
        KP_8,
        KP_9,
        KP_C,
        KP_ASTR,
        KP_0,
        KP_HASH,
        KP_D,
        KP_KEYCOUNT,
        KP_INVALID=-1
    };

    // setup IO
    void begin();

    /// scan the keyboard
    void scan();
    /// is there a keypress available?
    bool available() const;
    /// return raw 4-bit code
    char readRaw();
    /// return ASCII representation
    char readAscii();
    /// wait for keypress and return raw code
    char readBlocking();
    
    /// get current keypress state (no debouncing)
    char getCurrentState() const;

    /// run a raw code through the ASCII charmap
    static char convertToAscii(char r);

private:

    // 20 ms must elapse since keyup before a new keydown will register
    static const unsigned long TIMEOUT=20000;

    char code, seeing;
    char pins[8];
    unsigned long keyup;
  
    /// conversion from keycode to ASCII chars
    static const char *ASCII;
};

/**
 * 4x4 keypad that does mobile-phone-style alphanumeric entry with
 * multiple keypresses to select each character.
 * 
 * API designed to work within a state-machine, i.e. has a poll method
 * that doesn't block; returns true when the user has completed
 * entry.  Needs a pointer to a LiquidCrystal so that it can manage
 * display of chars as they're entered.
 *
 * Numeric keys do alphanumerics, 1 has most of the symbols, A is
 * backspace, B-D are "exit"
 *
 * '*' toggles alpha/numeric, '#' toggles case
 */
class SMSKeypad : public Keypad {
public:

    /**
     * All of the state required to perform string entry
     */
    class Context {
    public:
        /// define new input context (buffer, size, LCD device)
        /// @param b pointer to output buffer
        /// @param ml max length of string (requires sizeof(buffer)=ml+1)
        /// @param l output LCD
        /// @param c output column on LCD
        /// @Param r output row on LCD
        Context(char *b, unsigned ml, LiquidCrystal *l, unsigned char c, unsigned char r);

        /// reset the context (called by setContext())
        void clear();
  
        char * const buffer;        ///< pointer to character storage; alloced elsewhere
        const unsigned maxlen;     ///< max string length (buffer must contain maxlen+1)
        LiquidCrystal * const lcd;  ///< where to render entered chars
        const unsigned lcdc, lcdr; ///< cursor position on LCD to render at
        char exitcode;       ///< KP_B, KP_C or KP_D that terminated the read
    };

    SMSKeypad(char c0, char c1, char c2, char c3, char r0, char r1, char r2, char r3);

    /// define the context in which to operate
    void setContext(Context *c);
  
    /// poll the keypad using current context
    /// @return true if user presses B-D
    bool poll();

private:
    Context *ctx;        ///< current input context
    unsigned char upto;  ///< which char are we up to on this keypress
    bool alpha;       ///< alphanumeric mode
    bool ucase;       ///< upper/lower case
    unsigned char len;   ///< length of string entered
    char lastcode;       ///< code of previous keypress
    unsigned long t_last;///< time of last keypress
  
    /// user-input event requiring string update
    void onKeypress(char ch);
  
    /// determine next alphabetic character
    char nextAlpha(char digit, unsigned char upto);
  
    /// display current context
    void show();
    /// show where next char will be written
    void showCursor();
  
    /// time to move onto next char
    static const unsigned long CHAR_TIMEOUT=500000;

    /// chars available for each key
    static const char *CH_0, *CH_1, *CH_2, *CH_3, *CH_4, *CH_5, *CH_6, *CH_7, *CH_8, *CH_9;
    static const char *ALPHABET[];
    static const unsigned char CH_LENS[];
};

/**
 * Input processor that takes either a Keypad input device and uses 
 * it to manage the input of decimal numbers to a fixed precision.
 *
 * Numbers are entered L2R but precision is fixed, so the number will
 * appear to grow from the right.
 *
 * API is a polled operation with a defined context object, similar
 * to that used by SMSKeypad.
 *
 * As per SMSKeypad, A is backspace; B-D are "exit"; 
 * # and * change sign.
 */
class DecimalKeypad {
public:

    /**
     * State defining a particular input request
     */
    class Context {
    public:
        /// define new input context
        /// @param b output buffer (length must be at least mag+prec)
        /// @param m max magnitude (digits left of dec point)
        /// @param p precision (digits right of dec point)
        /// @param l output LCD
        /// @param c output column on LCD
        /// @param r output row on LCD
        /// @param si permit signed numbers
        Context(char *b, unsigned char m, unsigned char p, 
                LiquidCrystal *l, unsigned char c, unsigned char r,
                bool si);

        /// reset the context (called by setContext())
        void clear();
  
        char *buffer;        ///< pointer to character storage; alloced elsewhere
        const unsigned char mag, prec;
        LiquidCrystal * const lcd;  ///< where to render entered chars
        const unsigned lcdc, lcdr; ///< cursor position on LCD to render at
        char exitcode;       ///< KP_B, KP_C or KP_D that terminated the read
        long result;         ///< fixed-point result, in LSD-counts
        bool sign;        ///< do we allow negative inputs?
    };

    DecimalKeypad(Keypad &k);

    /// start performing input
    void setContext(Context *c);

    /// poll keypad
    /// @return true if completed
    bool poll();

private:

    Keypad &kp;
    Context *ctx;

    int count;              ///< how many digits entered
    bool negative;

    /// process/append a new digit
    void onDigit(char ch);

    /// display currently-entered number
    void show();

    /// compute final value
    void readComplete(char ch);
};


#endif // _KEYPAD_H_
