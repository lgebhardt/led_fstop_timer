#include "LEDDriver.h"

LEDDriver::LEDDriver(unsigned char p_e_center_hard, unsigned char p_e_center_soft, unsigned char p_e_corner_hard, unsigned char p_e_corner_soft) :
  pin_expose_center_hard(p_e_center_hard), pin_expose_center_soft(p_e_center_soft), pin_expose_corner_hard(p_e_corner_hard), pin_expose_corner_soft(p_e_corner_soft) {
  begin();
}  

void LEDDriver::begin() {
  allOff();
}

void LEDDriver::focusOn() {
  analogWrite(pin_expose_center_soft, getLampOrientation(255));
  analogWrite(pin_expose_corner_soft, getLampOrientation(255));
}

void LEDDriver::exposeOn(unsigned char center_hard, unsigned char center_soft, unsigned char corner_hard, unsigned char corner_soft) {
  analogWrite(pin_expose_center_hard, getLampOrientation((constrain(center_hard, 0, 100) * (LED_HARD_MAX - LED_HARD_MIN)) / 100));
  analogWrite(pin_expose_center_soft, getLampOrientation((constrain(center_soft, 0, 100) * (LED_SOFT_MAX - LED_SOFT_MIN)) / 100));
  analogWrite(pin_expose_corner_hard, getLampOrientation((constrain(corner_hard, 0, 100) * (LED_HARD_MAX - LED_HARD_MIN)) / 100));
  analogWrite(pin_expose_corner_soft, getLampOrientation((constrain(corner_soft, 0, 100) * (LED_SOFT_MAX - LED_SOFT_MIN)) / 100));
}

void LEDDriver::allOff() {
  analogWrite(pin_expose_center_hard, getLampOrientation(0));
  analogWrite(pin_expose_center_soft, getLampOrientation(0));
  analogWrite(pin_expose_corner_hard, getLampOrientation(0));
  analogWrite(pin_expose_corner_soft, getLampOrientation(0));
}