#ifndef _LED_DRIVER_H_
#define _LED_DRIVER_H_

#include <Arduino.h>

class LEDDriver {
public:
  LEDDriver(unsigned char p_e_center_hard, unsigned char p_e_center_soft, unsigned char p_e_corner_hard, unsigned char p_e_corner_soft);
  void begin();
  
/**
 * Scale the LEDS if you want to keep them from running at maximum current for longevity
 * The Hard LEDs are XTE Royal Blue and can run to 1500mA max
 * The Soft LEDs are XPE Green and can run to 1000mA max
 * The A009-D-V-1000 BuckBlock will provide up to 1000mA
 * For maximum life of the LEDs we should run them around 70% of the max, so we will need to scale the soft LEDs back.
 * Figure 9 of the BuckBlock manual indicates that 7 volts gives 70% power (255*.7) = 178
 * Figure 9 also indicates that the drivers are linear from about 2.3V to 9V (max),
   so set the lower bound to 60 for both, and 229 for the Hard. This will give us a fairly linear voltage response over
   a safe range. 
 * Both LEDs are slightly non linear in output vs current over these ranges, so look into this if we add a dimming function.
 */
  static const unsigned char LED_HARD_MAX=229;
  static const unsigned char LED_HARD_MIN=60;
  static const unsigned char LED_SOFT_MAX=178;
  static const unsigned char LED_SOFT_MIN=60;
  
/**
 * Set this according to the vagaries of your LEDs 
   false for test LEDs on the breadboard for testing
   true for A009-D-V-1000 BuckBlock
 */
  static const boolean LED_LAMP_INVERT = false;
  
  static unsigned char getLampOrientation(unsigned char in) { 
    return LED_LAMP_INVERT ? ~in : in;
  } 

  void focusOn();
  void exposeOn(unsigned char center_hard, unsigned char center_soft, unsigned char corner_hard, unsigned char corner_soft);
  void allOff();
  
private:
  char pin_expose_center_hard;
  char pin_expose_center_soft;
  char pin_expose_corner_hard;
  char pin_expose_corner_soft;
};

#endif