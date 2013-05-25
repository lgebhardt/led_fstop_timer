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

#include "Fstopcomms.h"

const char *FstopComms::BAD_WRITE=    "   Bad Write    ";
const char *FstopComms::BAD_READ=     "    Bad Read    ";
const char *FstopComms::CONNECTED=    " Host Connected ";
const char *FstopComms::CHECKSUM_FAIL=" Checksum Fail  ";

FstopComms::FstopComms(LiquidCrystal &l)
    : disp(l)
{
    lastlcd=lasttx=lastrx=micros();
    connected=false;
}

void FstopComms::begin()
{
    Serial.begin(COM_BAUD);
    reset();
}

void FstopComms::reset()
{
    connected=false;
    incmd=false;
    buflen=0;
    bufwant=1;
    lasttx=micros();
}

bool FstopComms::poll()
{
    while(Serial.available()){
        rx((char)Serial.read());        
    }

    unsigned long now=micros();

    // check for timeouts
    if(connected && now-lastrx > COM_TIMEOUT){
        reset();
    }
    if(incmd && now-lastrx > COM_CMDTIMEOUT){
        reset();
    }
    if(now-lasttx > COM_PERIOD)
        tx(COM_KEEPALIVE);
    if(now-lastlcd > COM_LCDTIMEOUT){
        disp.setCursor(0, 2);
        disp.print("                ");
        lastlcd=now;
    }

    // did we completely fail (timeout or bad command)
    return !connected;
}

void FstopComms::rx(char ch)
{
    lastrx=micros();

    // store byte
    if(buflen >= PKT_BUFFER)
        buflen=PKT_BUFFER-1;
    cmd[buflen++]=ch;
    incmd=true;

    // got the data we expected...
    if(buflen >= bufwant){
        if(!connected){
            connected=true;
            disp.clear();
            disp.print(CONNECTED);
            lastlcd=lastrx;
        }

        switch(cmd[0]){

            // keepalive; do nothing
        case COM_KEEPALIVE:
            bufwant=1;
            buflen=0;
            incmd=false;
            break;

            // request to read data
        case COM_READ:            
            if(bufwant == 1){
                // wait for the rest of the packet
                bufwant=PKT_HEADER;
            }
            else{
                respondRead();
            }
            break;

            // request to write data
        case COM_WRITE:
            if(bufwant == 1){
                // wait for rest of header
                bufwant=PKT_SHORTHDR;
            }
            else if(bufwant == PKT_SHORTHDR){
                // got length, decide how much packet to wait for
                char len=cmd[PKT_LEN];
                if(len < 1 || len > PKT_MAXREQ){
                    nak(BAD_WRITE);
                }
                else{
                    bufwant=PKT_HEADER+len;  // header+data+checksum
                }
            }
            else{
                // got whole packet
                respondWrite();
            }
            break;

            // disconnect
        default:
            reset();
        }
    }
}

void FstopComms::respondRead()
{
    if(!checkcheck())
        return;

    unsigned int addr=getAddr();
    char len=cmd[PKT_LEN];

    if(len < 1 || len > PKT_MAXREQ || addr < EEPROM_MIN_READ || addr+len > EEPROM_MAX_READ){
        nak(BAD_READ);
        return;
    }

    // read from EEPROM into buffer
    cmd[PKT_CMD]=COM_READACK;

    for(char i=0;i<len;++i){
        cmd[i+PKT_SHORTHDR]=EEPROM.read(addr++);
    }
    buflen=len+PKT_SHORTHDR;

    // append checksum and send.
    txCmd();
}

void FstopComms::respondWrite()
{
    if(!checkcheck())
        return;

    unsigned int addr=getAddr();
    char len=cmd[PKT_LEN];

    if(len < 1 || len > PKT_MAXREQ || addr < EEPROM_MIN_WRITE || addr+len > EEPROM_MAX_WRITE){
        nak(BAD_WRITE);
        return;
    }

    // copy from buffer to EEPROM
    char bp=PKT_SHORTHDR;
    for(char i=0;i<len;++i){
        EEPROM.write(addr++, cmd[bp++]);
    }

    cmd[PKT_CMD]=COM_WRITEACK;
    buflen=PKT_SHORTHDR;

    // recompute checksum, send without data
    txCmd();
}

unsigned int FstopComms::getAddr()
{
    unsigned char c1=cmd[PKT_ADDR], c0=cmd[PKT_ADDR+1];   // big-endian
    return (int(c1)<<8) + int(c0);
}

bool FstopComms::checkcheck()
{
    char chk=checksum();

    incmd=false;
    buflen=0;
    bufwant=1;

    if(chk != 0){
        error(CHECKSUM_FAIL);
        tx(COM_CHKFAIL);
        return false;
    }
    return true;
}

char FstopComms::checksum()
{
    char sum=0;
    for(int i=0;i<buflen;++i)
        sum ^= cmd[i];

    return sum;
}


void FstopComms::tx(char ch)
{
    lasttx=micros();
    Serial.write(ch);
}

void FstopComms::txCmd()
{
    // compute & append checksum if there is room
    if(buflen < PKT_BUFFER){
        cmd[buflen++]=checksum();
    }

    // transmit
    for(int i=0;i<buflen;++i){
        Serial.write(cmd[i]);
    }
    lasttx=micros();

    incmd=false;
    buflen=0;
    bufwant=1;
}

void FstopComms::nak(const char *s)
{
    error(s);
    tx(COM_NAK);
    incmd=false;
    buflen=0;
    bufwant=1;
}

void FstopComms::error(const char *s)
{
    disp.setCursor(0, 2);
    disp.print(s);
    lastlcd=micros();
}
