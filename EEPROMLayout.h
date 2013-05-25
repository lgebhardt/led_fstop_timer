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

#ifndef _EEPROMLAYOUT_H_
#define _EEPROMLAYOUT_H_

// EEPROM layout
#define EE_BACKLIGHT 0x00
#define EE_DRYDOWN 0x01
#define EE_DRYAPPLY 0x02
#define EE_ROTARY 0x03
#define EE_STRIPBASE 0x04
#define EE_STRIPSTEP 0x06
#define EE_STRIPCOV 0x08
#define EE_VERSION 0x09
#define EE_SPLITGRADE 0x0A
#define EE_CONFIGTOP 0x0B
#define EE_TOP 0x400

#endif