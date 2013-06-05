#ifndef _LED_DRIVER_H_
#define _LED_DRIVER_H_

#include <Arduino.h>

class LEDDriver {
public:
    LEDDriver(unsigned char p_e_center_hard, unsigned char p_e_center_soft, unsigned char p_e_corner_hard, unsigned char p_e_corner_soft, unsigned char p_e_safelight);
    void begin();
  
/**
 * Scale the LEDS if you want to keep them from running at maximum current for longevity
 * The Hard LEDs are XTE Royal Blue and can run to 1500mA max
 * The Soft LEDs are XPE Green and can run to 1000mA max
 * The A009-D-V-1000 BuckBlock will provide up to 1000mA
 * For maximum life of the LEDs we should run them around 70% of the max, so we will need to scale the soft LEDs back.
 * Testing indicates that 7 results in .72Amps, which is close enough
 * Both LEDs get a bit unstable at low currents. So a minimum brightness needs to be established. 200 is a good starting point
 * Both LEDs are non linear in output vs current over these ranges, so look into this if we add a dimming function.
 */
    static const unsigned char LED_HARD_MAX=0;
    static const unsigned char LED_HARD_MIN=200;
    static const unsigned char LED_SOFT_MAX=7;
    static const unsigned char LED_SOFT_MIN=200;
    static const unsigned char LED_OFF=255;

    void focusOn();
    void exposeOn(unsigned char center_hard, unsigned char center_soft, unsigned char corner_hard, unsigned char corner_soft);
    void calibrateOn(unsigned char center_hard, unsigned char center_soft, unsigned char corner_hard, unsigned char corner_soft);
    void allOff();
  
private:
    char pin_expose_center_hard;
    char pin_expose_center_soft;
    char pin_expose_corner_hard;
    char pin_expose_corner_soft;
    char pin_safelight_relay; //Can also be used for a regular enlarger if the relay supports two modes
};

#endif