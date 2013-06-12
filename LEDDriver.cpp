#include "LEDDriver.h"

LEDDriver::LEDDriver(unsigned char p_e_center_hard, unsigned char p_e_center_soft, 
                     unsigned char p_e_corner_hard, unsigned char p_e_corner_soft, 
                     unsigned char p_e_safelight) :
                 pin_expose_center_hard(p_e_center_hard), 
                 pin_expose_center_soft(p_e_center_soft), 
                 pin_expose_corner_hard(p_e_corner_hard), 
                 pin_expose_corner_soft(p_e_corner_soft),
                 pin_safelight_relay(p_e_safelight) {
    begin();
}  

void LEDDriver::begin() {
    pinMode(pin_safelight_relay, OUTPUT);
    allOff();
}

void LEDDriver::focusOn(unsigned char center_hard, unsigned char center_soft, unsigned char corner_hard, unsigned char corner_soft) {
    // analogWrite(pin_expose_center_soft, LED_SOFT_MAX);
    // analogWrite(pin_expose_corner_soft, LED_SOFT_MAX);
    // digitalWrite(pin_safelight_relay, HIGH);
    exposeOn(center_hard, center_soft, corner_hard, corner_soft);
}

void LEDDriver::exposeOn(unsigned char center_hard, unsigned char center_soft, unsigned char corner_hard, unsigned char corner_soft) {
    //For Percents
    // analogWrite(pin_expose_center_hard, LED_HARD_MIN - (constrain(corner_hard, 0, 100) * ((LED_HARD_MIN - LED_HARD_MAX) / 100.0)));
    // analogWrite(pin_expose_center_soft, LED_SOFT_MIN - (constrain(corner_soft, 0, 100) * ((LED_SOFT_MIN - LED_SOFT_MAX) / 100.0)));
    // analogWrite(pin_expose_corner_hard, LED_HARD_MIN - (constrain(corner_hard, 0, 100) * ((LED_HARD_MIN - LED_HARD_MAX) / 100.0)));
    // analogWrite(pin_expose_corner_soft, LED_SOFT_MIN - (constrain(corner_soft, 0, 100) * ((LED_SOFT_MIN - LED_SOFT_MAX) / 100.0)));

    //For absolute values
    if (center_hard != LED_OFF) analogWrite(pin_expose_center_hard, constrain(center_hard, LED_HARD_MAX, LED_HARD_MIN));
    if (center_soft != LED_OFF) analogWrite(pin_expose_center_soft, constrain(center_soft, LED_SOFT_MAX, LED_SOFT_MIN));
    if (corner_hard != LED_OFF) analogWrite(pin_expose_corner_hard, constrain(corner_hard, LED_HARD_MAX, LED_HARD_MIN));
    if (corner_soft != LED_OFF) analogWrite(pin_expose_corner_soft, constrain(corner_soft, LED_SOFT_MAX, LED_SOFT_MIN));
    digitalWrite(pin_safelight_relay, HIGH);
}

void LEDDriver::calibrateOn(unsigned char center_hard, unsigned char center_soft, unsigned char corner_hard, unsigned char corner_soft) {
    analogWrite(pin_expose_center_hard, center_hard);
    analogWrite(pin_expose_center_soft, center_soft);
    analogWrite(pin_expose_corner_hard, corner_hard);
    analogWrite(pin_expose_corner_soft, corner_soft);
    digitalWrite(pin_safelight_relay, HIGH);
}

void LEDDriver::allOff() {
    analogWrite(pin_expose_center_hard, LED_OFF);
    analogWrite(pin_expose_center_soft, LED_OFF);
    analogWrite(pin_expose_corner_hard, LED_OFF);
    analogWrite(pin_expose_corner_soft, LED_OFF);
    digitalWrite(pin_safelight_relay, LOW);
}