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
    initDefault();
}

unsigned char Paper::getAmountSoft(unsigned char grade) {
    return amountsSoft[(constrain(grade, MINGRADE, MAXGRADE) - MINGRADE) / 5];
}

unsigned char Paper::getAmountHard(unsigned char grade){
    return amountsHard[(constrain(grade, MINGRADE, MAXGRADE) - MINGRADE) / 5];
}

void Paper::load(char fileName) {
    
}

void Paper::initDefault() {
    // //ToDo: Fix this logic up. This is only for testing
    // for (unsigned char i = 0 ; i < GRADES ; i++) {
    //     amountsHard[i] = (200.0 / GRADES) * (GRADES - i);
    //     amountsSoft[i] = (193.0 / GRADES) * (i + 7);
    // }
    // // Overrides to turn off lamps fully at extremes
    // amountsSoft[0] = 7;
    // amountsSoft[GRADES - 1] = 255;


    //Rough curve from first round of testing
    amountsSoft[0] = 255;
    amountsSoft[1] = 255;
    amountsSoft[2] = 255;
    amountsSoft[3] = 255;
    amountsSoft[4] = 255;
    amountsSoft[5] = 210; //55
    amountsSoft[6] = 184; //60
    amountsSoft[7] = 158;
    amountsSoft[8] = 141;
    amountsSoft[9] = 127;
    amountsSoft[10] = 118;
    amountsSoft[11] = 111;
    amountsSoft[12] = 105;
    amountsSoft[13] = 100;
    amountsSoft[14] = 95;
    amountsSoft[15] = 90;
    amountsSoft[16] = 85;
    amountsSoft[17] = 80;
    amountsSoft[18] = 75;
    amountsSoft[19] = 71;
    amountsSoft[20] = 66;
    amountsSoft[21] = 58;
    amountsSoft[22] = 51;
    amountsSoft[23] = 45;
    amountsSoft[24] = 39;
    amountsSoft[25] = 34;
    amountsSoft[26] = 28;
    amountsSoft[27] = 22;
    amountsSoft[28] = 16;
    amountsSoft[29] = 10; //175
    amountsSoft[30] = 9;  //180
    amountsSoft[31] = 8;  //Cheats to test high power green
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
    
    strcpy(name, "Default Paper");
}

