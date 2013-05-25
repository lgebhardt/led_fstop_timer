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

#include "Keypad.h"

/**
 * ButtonDebounce
 */

ButtonDebounce::ButtonDebounce(char p, bool inv)
    : pin(p), invert(inv)
{
    pinMode(pin, INPUT);
    digitalWrite(pin, 1);
    state=false;
    event=false;
    changedat=0;
}

void ButtonDebounce::scan()
{
    bool newstate=digitalRead(pin) ^ invert;

    if(newstate != state){
        unsigned long now=micros();

        // changed after long-enough time
        if(now - changedat > TIMEOUT){
            changedat=now;
            state=newstate;
            if(state)
                event=true;
        }
    }
}

bool ButtonDebounce::getCurrentState() const
{
    return state;
}

bool ButtonDebounce::hadPress()
{
    bool res=event;
    event=false;
    return res;
}

/**
 * Keypad
 */
const char *Keypad::ASCII="123A456B789C*0#D";

Keypad::Keypad(char c0, char c1, char c2, char c3, char r0, char r1, char r2, char r3)
{
    pins[0]=c0;
    pins[1]=c1;
    pins[2]=c2;
    pins[3]=c3;
    pins[4]=r0;
    pins[5]=r1;
    pins[6]=r2;
    pins[7]=r3;

    code=seeing=KP_INVALID;
    keyup=0;

}

void Keypad::begin()
{
    keyup=micros();

    for(char i=0;i<4;++i){
        pinMode(pins[i], OUTPUT);
        digitalWrite(pins[i], HIGH);
     
        pinMode(pins[4+i], INPUT);
        digitalWrite(pins[4+i], HIGH);  // set pullups
    }

}

void Keypad::scan()
{
    unsigned long now=micros();
  
    // look at all the combinations
    char newsee=KP_INVALID;
    for(char row=0;row<4;++row){
        digitalWrite(pins[row], LOW);
        for(char col=0;col<4;++col){
            if(!digitalRead(pins[4+col])){
                newsee=(row<<2)|col;
            }
        }
        digitalWrite(pins[row], HIGH);
    }
  
    // state change and not within timeout period
    if(newsee != seeing && now-keyup > TIMEOUT){
        keyup=now;
        seeing=newsee;

        // store the press on key-down
        if(seeing != KP_INVALID)
            code=seeing;
    }
}

bool Keypad::available() const
{
    return code != KP_INVALID;
}

char Keypad::readRaw()
{
    char res=code;
    code=KP_INVALID;
    return res;
}

char Keypad::readAscii()
{
    return convertToAscii(readRaw());
}

char Keypad::readBlocking()
{
    do {
        scan();
    } while(!available());

    return readRaw();
}

char Keypad::convertToAscii(char c)
{
    if(c < 0 || c >= KP_KEYCOUNT)
        return '\0';
    
    return ASCII[c];
}

char Keypad::getCurrentState() const
{
    return seeing; 
}


/**
 * SMSKeypad
 */

const char *SMSKeypad::CH_0=" ";
const char *SMSKeypad::CH_1="-,.!?+/@#$%^&*()";
const char *SMSKeypad::CH_2="ABC";
const char *SMSKeypad::CH_3="DEF";
const char *SMSKeypad::CH_4="GHI";
const char *SMSKeypad::CH_5="JKL";
const char *SMSKeypad::CH_6="MNO";
const char *SMSKeypad::CH_7="PQRS";
const char *SMSKeypad::CH_8="TUV";
const char *SMSKeypad::CH_9="WXYZ";
const char *SMSKeypad::ALPHABET[]={SMSKeypad::CH_0, SMSKeypad::CH_1, SMSKeypad::CH_2,
                                   SMSKeypad::CH_3, SMSKeypad::CH_4, SMSKeypad::CH_5, 
                                   SMSKeypad::CH_6, SMSKeypad::CH_7, SMSKeypad::CH_8, 
                                   SMSKeypad::CH_9, };
const unsigned char SMSKeypad::CH_LENS[]={1, 16, 3, 3, 3, 3, 3, 4, 3, 4 };


SMSKeypad::SMSKeypad(char c0, char c1, char c2, char c3, char r0, char r1, char r2, char r3)
    : Keypad(c0, c1, c2, c3, r0, r1, r2, r3)
{
}

void SMSKeypad::setContext(Context *c)
{
    ctx=c;
    if(NULL == ctx)
        return;
    
    len=0;
    upto=0;
    alpha=true;
    ucase=true;
    t_last=micros()-CHAR_TIMEOUT-10;
    ctx->clear();

    show();
}
  
bool SMSKeypad::poll()
{
    scan(); 
    if(NULL == ctx)
        return false;
    if(!available()){
        showCursor();  // will update cursor position on timeout
        return false;  
    }

    char ch=readRaw();
    switch(ch){
    case KP_A:
        // backspace
        if(len > 0){
            ctx->buffer[--len]='\0'; 
        }
        break;
    case KP_B:
    case KP_C:
    case KP_D:
        // exit
        ctx->exitcode=ch;
        ctx->lcd->noCursor();
        return true;
    case KP_ASTR:
        // toggle alpha
        alpha=!alpha;
        break;
    case KP_HASH:
        // toggle case
        ucase=!ucase;
        break;
    default:
        // write something!
        onKeypress(ch);
    }
  
    lastcode=ch;
    
    show();
    return false;
}

void SMSKeypad::onKeypress(char ch)
{
    char asc=convertToAscii(ch);
  
    if(alpha){
        // convert ASCII char to a number
        char digit=asc-'0';
    
        // alpha: post-timeout or different key pressed => append
        if(micros()-t_last > CHAR_TIMEOUT || ch != lastcode){
            if(len < ctx->maxlen){
                upto=0;
                ctx->buffer[len++]=nextAlpha(digit, upto);
            }
            // else cannot append; do nothing
        }
        else{
            // alpha within-timeout & same key; overwrite last char with new option 
            if(++upto >= CH_LENS[digit])
                upto=0;
            ctx->buffer[len-1]=nextAlpha(digit, upto);
        }
    }
    else{
        // numeric input, no timeouts
        if(len < ctx->maxlen){
            ctx->buffer[len++]=asc;
        }
    }
  
    t_last=micros();
}

char SMSKeypad::nextAlpha(char digit, unsigned char upto)
{
    char asc=ALPHABET[digit][upto];
    return ucase ? toupper(asc) : tolower(asc);
}

void SMSKeypad::show()
{
    ctx->lcd->setCursor(ctx->lcdc, ctx->lcdr);
    ctx->lcd->print(ctx->buffer);

    // erase following chars
    for(int i=len;i<ctx->maxlen;++i)
        ctx->lcd->print(" ");
    
    showCursor();
}

void SMSKeypad::showCursor()
{
    // show cursor at current position
    int col=ctx->lcdc+len;
    if(micros()-t_last < CHAR_TIMEOUT && len > 0)
        --col;
    ctx->lcd->setCursor(col, ctx->lcdr);
    ctx->lcd->cursor();  
}

SMSKeypad::Context::Context(char *b, unsigned ml, LiquidCrystal *l, 
                            unsigned char c, unsigned char r)
    : buffer(b), maxlen(ml), lcd(l), lcdc(c), lcdr(r)
{
    clear();
}

void SMSKeypad::Context::clear()
{
    memset(buffer, '\0', maxlen+1);
    exitcode=KP_INVALID; 
}


/**
 * DecimalKeypad
 */


DecimalKeypad::Context::Context(char *b, unsigned char m, unsigned char p,
                                LiquidCrystal *l, unsigned char c, unsigned char r,
                                bool si)
    : buffer(b), mag(m), prec(p), lcd(l), lcdc(c), lcdr(r), result(0L), sign(si)
{
    clear();
}

void DecimalKeypad::Context::clear()
{
    exitcode=Keypad::KP_INVALID;
    // buffer[mag+prec]='\0';
    memset(buffer, mag+prec, '\0');
}

DecimalKeypad::DecimalKeypad(Keypad &k)
    : kp(k)
{
    ctx=NULL;
    count=0;
}

void DecimalKeypad::setContext(Context *c)
{
    ctx=c;
    if(NULL == ctx)
        return;

    ctx->clear();
    count=0;
    negative=false;
    show();
}

bool DecimalKeypad::poll()
{
    kp.scan();
    if(NULL == ctx)
        return false;

    if(kp.available()){
        char ch=kp.readRaw();
        switch(ch){
        case Keypad::KP_A:
            if(count > 0){
                --count;
            }
            // backspace
            break;
        case Keypad::KP_B:
        case Keypad::KP_C:
        case Keypad::KP_D:
            // exit
            readComplete(ch);
            return true;
        case Keypad::KP_ASTR:
        case Keypad::KP_HASH:
            // change sign
            if(ctx->sign)
                negative=!negative;
            break;
        default:
            onDigit(ch);
        }
    }

    show();
    return false;
}

void DecimalKeypad::onDigit(char ch)
{
    ch=Keypad::convertToAscii(ch);
    if(count < ctx->mag+ctx->prec){
        ctx->buffer[count++]=ch;
    }
    // else full, ignore
}

void DecimalKeypad::show()
{
    ctx->lcd->noCursor();
    ctx->lcd->setCursor(ctx->lcdc, ctx->lcdr);

    char ptr=0;
    char sigfig=count-ctx->prec;
    if(sigfig < 1)
        sigfig=1;

    // print leading spaces
    for(int i=sigfig;i<ctx->mag;++i)
        ctx->lcd->print(" ");

    // show sign
    if(ctx->sign){
        if(negative)
            ctx->lcd->print("-");
        else
            ctx->lcd->print(" ");
    }

    // print digits before the decimal point
    sigfig=count-ctx->prec;
    if(sigfig < 1){
        ctx->lcd->print("0");
    }
    else{
        for(int i=0;i<sigfig;++i){
            ctx->lcd->print(ctx->buffer[ptr++]);           
        }
    }

    // asking for non-integer, show dec place
    if(ctx->prec > 0)
        ctx->lcd->print(".");

    // do post-decimal zero-fill
    for(int i=0;i<ctx->prec-count;++i)
        ctx->lcd->print("0");

    // do post-decimal prints
    while(ptr < count)
        ctx->lcd->print(ctx->buffer[ptr++]);
  
    // *phew*
    ctx->lcd->setCursor(ctx->lcdc+ctx->mag+ctx->prec, ctx->lcdr);
    ctx->lcd->cursor();
}

void DecimalKeypad::readComplete(char ch)
{
    long val=0;
    for(int i=0;i<count;++i){
        val*=10;
        val+=ctx->buffer[i]-'0';
    }

    if(negative)
        val=-val;

    ctx->result=val;
    ctx->exitcode=ch;
    ctx->lcd->noCursor();
}
