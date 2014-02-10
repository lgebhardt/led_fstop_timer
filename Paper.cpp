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

#include "Paper.h"

Paper::Paper() {
    sdready = false;
}

void Paper::init(bool ready)
{
    sdready = ready;
    if (sdready) {
        load(0);
    }
    else {
        initDefault();
    }
}

bool Paper::load(char paper) {
    if (!sdready) {
        initDefault();
        return false;
    }

    if(paper < FIRSTPAPER || paper > LASTPAPER)
        return false;

    String fileName = "/papers/";
    fileName += (int) paper;
    fileName += ".ppr";

    char buf[30];
    fileName.toCharArray(buf, 30);

    File f = SD.open(buf, FILE_READ);
        if (f) {
        initFromFile(f);
        f.close();
        return true;
    }
    else {
        initDefault();
    }
    return false;
}

bool Paper::initFromFile(File& dataFile) {
    //Line 1: name
    if (dataFile.available()) {
        name = readLine(dataFile);
    }
    String line;
    //Line 2: Max/focus brightness
    line = readLine(dataFile);
    int delim = line.indexOf(',');
    maxBrightnessSoft = parseLevel(line.substring(0, delim));
    maxBrightnessHard = parseLevel(line.substring(delim + 1));
    
    //Line 3: Min Grade
    minGrade = parseLevel(readLine(dataFile));
    //Line 4: Max Grade
    maxGrade = parseLevel(readLine(dataFile));

    //Line 5+: Brightness pairs
    for (int pos = 0; pos < GRADES; pos++) {
        line = readLine(dataFile);

        int delim = line.indexOf(',');
        amountsSoft[pos] = parseLevel(line.substring(0, delim));
        amountsHard[pos] = parseLevel(line.substring(delim + 1));
    }
    
    return true;
}

unsigned char Paper::parseLevel(String level) {
    level.trim();
    return level.toInt();
}

String Paper::readLine(File& dataFile) {
    String ln = "";
    while (dataFile.available()) {
        char character = dataFile.read();
        if (character == '\n') {
            return ln;
        }
        else {
            ln = ln + character;
        }
    }
    return ln;
}

unsigned char Paper::getAmountSoft(unsigned char grade) {
    return amountsSoft[(constrain(grade, MINGRADE, MAXGRADE) - MINGRADE) / 5];
}

unsigned char Paper::getAmountHard(unsigned char grade){
    return amountsHard[(constrain(grade, MINGRADE, MAXGRADE) - MINGRADE) / 5];
}

String& Paper::getName() { return name; }

void Paper::initDefault() {
    //Rough curve from first round of testing
    amountsSoft[0] = 255; //30
    amountsSoft[1] = 255; //35
    amountsSoft[2] = 255; //40
    amountsSoft[3] = 255; //45
    amountsSoft[4] = 255; //50
    amountsSoft[5] = 210; //55
    amountsSoft[6] = 184; //60
    amountsSoft[7] = 158; //65
    amountsSoft[8] = 141; //70
    amountsSoft[9] = 127; //75
    amountsSoft[10] = 118; //80
    amountsSoft[11] = 111; //85
    amountsSoft[12] = 105; //90
    amountsSoft[13] = 100; //95
    amountsSoft[14] = 95;  //100
    amountsSoft[15] = 90;  //105
    amountsSoft[16] = 85;  //110
    amountsSoft[17] = 80;  //115
    amountsSoft[18] = 75;  //120
    amountsSoft[19] = 71;  //125
    amountsSoft[20] = 66;  //130
    amountsSoft[21] = 58;  //135
    amountsSoft[22] = 51;  //140
    amountsSoft[23] = 45;  //145
    amountsSoft[24] = 39;  //150
    amountsSoft[25] = 34;  //155
    amountsSoft[26] = 28;  //160
    amountsSoft[27] = 22;  //165
    amountsSoft[28] = 16;  //170
    amountsSoft[29] = 10;  //175
    amountsSoft[30] = 9;   //180
    amountsSoft[31] = 8;   //Cheats to test high power green
    amountsSoft[32] = 7;
    amountsSoft[33] = 6;
    amountsSoft[34] = 5;
    
    amountsHard[30] = 255;
    amountsHard[31] = 255;
    amountsHard[32] = 255;
    amountsHard[33] = 255;
    amountsHard[34] = 255;
    for (unsigned char i = 6 ; i < 30; i++) {
        amountsHard[i] = 217 - amountsSoft[i];
    }
    
    name = "System Default Paper";
    maxBrightnessSoft = 217;
    maxBrightnessHard = 217;
    minGrade = 55;
    maxGrade = 180;
}

