/* -*- C++ -*- */

#ifndef _ROTARY_ENCODER_H_
#define _ROTARY_ENCODER_H_

#include <Arduino.h>

/**
 * Interrupt access to a crappy DSE quadrature encoder where the detents
 * are 00 and 11.  Change interrupts are used to catch the transitions,
 * of which there are TWO per click, hence the desire to avoid polling.
 *
 * Expects each axis to be wired with common low and uses internal
 * pullup on a pair of input pins to see when the each 
 * axis is shorted.
 *
 * Supports only one encoder as it uses a static pointer to the
 * encoder object to access it from a change-interrupt handler.
 *
 * Because the change interrupts only exist on pins 2 & 3, the encoder
 * must be attached to those two pins!
 */
class RotaryEncoder {
public:

    /// constructor
    RotaryEncoder(bool rev);

    /// setup IO pins, initialise state
    void begin();

    /// get (and clear) the number of counts since this was last called
    int getDelta();

private:    

    /// this gets bound to the two change interrupts (0, 1)
    /// which requires that the pins be 2 and 3.
    static void changeInterrupt();

    // these two used only from interrupt
    volatile char laststate, lastdetent;
    volatile char trans;
    // this one shared between interrupt and getDelta
    volatile int delta;

    static RotaryEncoder *enc;
    static char ENCPIN0, ENCPIN1;
    static const char ENCINT0=0, ENCINT1=1;
};

#endif // _ROTARY_ENCODER_H_
