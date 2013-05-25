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

#include "Executor.h"

Executor::Executor(LiquidCrystal &l, Keypad &k, ButtonDebounce &b, LEDDriver &led)
    : disp(l), keys(k), button(b), leddriver(led)
{
    current=NULL;
}

void Executor::begin()
{
    execphase=0;
    dd=false;
}

void Executor::setProgram(Program *p)
{
    current=p;
    changePhase(0);
}

void Executor::setDrydown(bool d)
{
    dd=d;
    changePhase(0);
}

void Executor::setSplitgrade(bool s)
{
    sg=s;
    changePhase(0);
}

/// specify that program is up to a particular exposure; display it
void Executor::changePhase(unsigned char ph)
{
    if(NULL == current)
        return;

	execphase = ph;

    (*current).getExposure(execphase).display(disp, dispbuf, true);
    disp.setCursor(18, 2);
    disp.print(sg ? "S" : " ");
    disp.setCursor(19, 2);
    disp.print(dd ? "D" : " ");
}

void Executor::expose()
{
    if(NULL == current)
        return;

    // backup the duration; it will get overwritten for display purposes
    Program::Exposure &expo=(*current).getExposure(execphase);
    unsigned long msbackup=expo.ms;
    unsigned long start=micros();
    unsigned long now=start, dt=0, lastupdate=start;

    // begin
    leddriver.exposeOn(expo.hardpower, expo.softpower, expo.hardpower, expo.softpower);

    // polling loop
    bool cancelled=false, skipped=false;
    while(!skipped){
        now=micros();
        dt=(now-start)/1000;    // works with overflows

        // time is elapsed
        if(dt >= msbackup){
            break;
        }

        // time for some IO?  otherwise tighten the loop a bit
        if((msbackup-dt) > 50){
            if((now-lastupdate) > 100000){
                // re-display with reduced time remaining
                expo.ms=msbackup-dt;
                expo.displayTime(disp, dispbuf, true);
                lastupdate=now;
            }

            // pause!
            keys.scan();
            if(keys.available()){
                if(keys.readRaw() == Keypad::KP_HASH){
                    leddriver.allOff();
                    unsigned long pausestart=micros();

                    // cancel on anuthing but Expose button
                    char ch=keys.readBlocking();
                    switch(ch){
                    case Keypad::KP_HASH:
                        // adjust clock and resume exposing
                        // !! should account for enlarger warmup here
                        start+=micros()-pausestart;
					    leddriver.exposeOn(expo.hardpower, expo.softpower, expo.hardpower, expo.softpower);
                        break;
                    case Keypad::KP_B:
                        // halt and this exposure
                        skipped=true;
                        break;
                    default:
                        // halt and cancel
                        skipped=true;
                        cancelled=true;
                    }
                }
            }
        }
    }

    // cease
	leddriver.allOff();

    // restore
    expo.ms=msbackup;

    if(cancelled){
        // tell user to go home
        disp.clear();
        disp.print("Prog Cancelled");
        delay(1000);
        changePhase(0);
    }
    else{
        nextPhase();
    }
}

void Executor::nextPhase()
{
    // decide on next exposure or reset to beginning
    for(int newphase=execphase+1; newphase < Program::MAXEXPOSURES;++newphase){
        if((*current).getExposure(newphase).ms != 0){
            changePhase(newphase);
            return;
        }
    }

    disp.clear();
    disp.print("Program Complete");
    delay(1000);
    changePhase(0);
}
