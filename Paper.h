/*
    Copyright (C) 2013 Larry Gebhardt
    www.trippingthroughthedark.com
  
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

#ifndef _PAPER_H_
#define _PAPER_H_

#include <Arduino.h>
#include <SD.h>

class Paper {

public:    
    Paper();

private:
    // bounds of grade, 30 to 200 in steps of 5
    static const int MAXGRADE=200;
    static const int MINGRADE=30;
    static const int GRADES=35; 

    char name[21];
    unsigned char amountsSoft[GRADES];
    unsigned char amountsHard[GRADES];

public:
    void load(char fileName);
    void initDefault();
    unsigned char getAmountSoft(unsigned char grade);
    unsigned char getAmountHard(unsigned char grade);
    char* getName();
};

#endif

