
#include "RotaryEncoder.h"

RotaryEncoder *RotaryEncoder::enc=NULL;
char RotaryEncoder::ENCPIN0=3;
char RotaryEncoder::ENCPIN1=2;

RotaryEncoder::RotaryEncoder(bool rev)
{
    enc=this;

    delta=0;
    laststate=0;
    trans=0;

    // pin-swap will revese operation
    if(rev){
        int tmp=ENCPIN0;
        ENCPIN0=ENCPIN1;
        ENCPIN1=tmp;
    }
}

void RotaryEncoder::begin()
{
    // inputs with pullup resistors
    pinMode(ENCPIN0, INPUT);
    pinMode(ENCPIN1, INPUT);
    digitalWrite(ENCPIN0, HIGH);
    digitalWrite(ENCPIN1, HIGH);

    // init state
    laststate=(digitalRead(ENCPIN1) << 1) | digitalRead(ENCPIN0);
    lastdetent=laststate;
    delta=0;
    trans=0;

    // attach interrupt handler
    attachInterrupt(ENCINT0, &RotaryEncoder::changeInterrupt, CHANGE);
    attachInterrupt(ENCINT1, &RotaryEncoder::changeInterrupt, CHANGE);
}

void RotaryEncoder::changeInterrupt()
{
    if(NULL == enc)
        return;

    char p0=digitalRead(ENCPIN0);
    char p1=digitalRead(ENCPIN1);

    char newstate=(p1<<1) | p0;

    if(newstate != enc->laststate){

        if(p0 == p1){
            // in detent, make sure we only change if we're
            // going to different detent, not bouncing back to
            // previous one
            if(newstate != enc->lastdetent)
                enc->delta+=enc->trans;

            enc->lastdetent=newstate;
            enc->trans=0;
        }
        else{
            // leaving detent
            enc->trans=(((newstate ^ enc->laststate) & 1)<<1)-1;
        }

        // remember state
        enc->laststate=newstate;
    }
    // else wtf?
}

int RotaryEncoder::getDelta()
{
    int res=delta;
    delta=0;
    return res;
}
