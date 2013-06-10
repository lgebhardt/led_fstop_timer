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

#include "FstopTimer.h"

const char *FstopTimer::VERSION="LED F/Stop Timer ";

/// functions to exec on entering each state
FstopTimer::voidfunc FstopTimer::sm_enter[]
   ={ &FstopTimer::st_splash_enter,
      &FstopTimer::st_main_enter,
      &FstopTimer::st_edit_enter, 
      &FstopTimer::st_edit_ev_enter, 
      &FstopTimer::st_edit_grade_enter, 
      &FstopTimer::st_edit_text_enter, 
      &FstopTimer::st_exec_enter,
      &FstopTimer::st_focus_enter,
      &FstopTimer::st_io_enter, 
      &FstopTimer::st_io_load_enter, 
      &FstopTimer::st_io_save_enter,
      &FstopTimer::st_comms_enter,
      &FstopTimer::st_test_enter, 
      &FstopTimer::st_test_changeb_enter, 
      &FstopTimer::st_test_changes_enter, 
      &FstopTimer::st_config_enter,
      &FstopTimer::st_config_dry_enter,
      &FstopTimer::st_config_rotary_enter,
      &FstopTimer::st_calibrate_light_enter
 };
/// functions to exec when polling within each state
FstopTimer::voidfunc FstopTimer::sm_poll[]
   ={ &FstopTimer::st_splash_poll, 
      &FstopTimer::st_main_poll, 
      &FstopTimer::st_edit_poll,
      &FstopTimer::st_edit_ev_poll,
      &FstopTimer::st_edit_grade_poll,
      &FstopTimer::st_edit_text_poll,
      &FstopTimer::st_exec_poll, 
      &FstopTimer::st_focus_poll, 
      &FstopTimer::st_io_poll, 
      &FstopTimer::st_io_load_poll, 
      &FstopTimer::st_io_save_poll,
      &FstopTimer::st_comms_poll,
      &FstopTimer::st_test_poll,
      &FstopTimer::st_test_changeb_poll,
      &FstopTimer::st_test_changes_poll,
      &FstopTimer::st_config_poll,
      &FstopTimer::st_config_dry_poll,
      &FstopTimer::st_config_rotary_poll,
      &FstopTimer::st_calibrate_light_poll
};

FstopTimer::FstopTimer(LiquidCrystal &l, SMSKeypad &k, RotaryEncoder &r,
                       ButtonDebounce &b,
                       ButtonDebounce &fs,
                       LEDDriver &led, 
                       TSL2561 &t, char p_b, char p_bl)
    : disp(l), keys(k), rotary(r), button(b), footswitch(fs), leddriver(led), tsl(t),
      smsctx(&inbuf[0], 14, &disp, 0, 0),
      deckey(keys), comms(l),
      expctx(&inbuf[0], 1, 2, &disp, 0, 2, true),
      gradectx(&inbuf[0], 3, 0, &disp, 7, 1, false),
      stepctx(&inbuf[0], 1, 2, &disp, 0, 1, false),
      dryctx(&inbuf[0], 0, 2, &disp, 0, 1, false),
      intctx(&inbuf[0], 1, 0, &disp, 0, 1, false),
      exec(l, keys, button, footswitch, led),
      
       
      pin_beep(p_b), pin_backlight(p_bl)
{
    // init libraries
    prevstate=curstate=ST_MAIN;
    focusphase=-1;
}

void FstopTimer::setBacklight()
{
    brightness=EEPROM.read(EE_BACKLIGHT);
    if(brightness > BL_MAX)
        brightness=BL_MAX;
    if(brightness < BL_MIN)
        brightness=BL_MIN;

    analogWrite(pin_backlight, (1<<brightness)-1);
}

void FstopTimer::begin()
{
    pinMode(pin_beep, OUTPUT);
    pinMode(pin_backlight, OUTPUT);
    pinMode(pin_exposebtn, INPUT);

    leddriver.allOff();

    // load & apply backlight settings
    setBacklight();
   
    drydown=EEPROM.read(EE_DRYDOWN);
//    drydown_apply=EEPROM.read(EE_DRYAPPLY);
    drydown_apply = false;
    
    splitgrade = EEPROM.read(EE_SPLITGRADE);
    
    current.clear();
    stripbase=(EEPROM.read(EE_STRIPBASE)<<8) | EEPROM.read(EE_STRIPBASE+1);
    stripstep=(EEPROM.read(EE_STRIPSTEP)<<8) | EEPROM.read(EE_STRIPSTEP+1);
    stripcover=EEPROM.read(EE_STRIPCOV);

    // prevent client-overwrite shenanigans
    EEPROM.write(EE_VERSION, VERSIONCODE);

    rotexp=EEPROM.read(EE_ROTARY);

    comms.begin();
    exec.begin();

    // boot the state machine
    changeState(ST_SPLASH);
}

void FstopTimer::toggleDrydown()
{
    drydown_apply=!drydown_apply;
//    EEPROM.write(EE_DRYAPPLY, drydown_apply);
}

void FstopTimer::toggleSplitgrade()
{
    splitgrade=!splitgrade;
    EEPROM.write(EE_SPLITGRADE, splitgrade);
}

void FstopTimer::st_splash_enter()
{
    disp.clear();
    disp.print(VERSION);
    dtostrf(EEPROM.read(EE_VERSION)*0.1, 3, 1, dispbuf);
    disp.print(dispbuf);
    disp.setCursor(0, 1);
    disp.print("W Brodie-Tyrrell");
    disp.setCursor(0, 2);
    disp.print("Larry Gebhardt");
}

void FstopTimer::st_splash_poll()
{
    if(Serial.available()){
        changeState(ST_COMMS);
    }
    if(keys.available()){
        keys.readAscii();
        changeState(ST_MAIN);
    }
    if(button.hadPress() || footswitch.hadPress()){
        changeState(ST_MAIN);
    }    
}

/// begin the main-menu state; write to display
void FstopTimer::st_main_enter()
{
    disp.clear();
    disp.setCursor(0,0);
    disp.print("A:Edit   B:IO");
    disp.setCursor(0,1);
    disp.print("C:Config D:Test");
    disp.setCursor(0,2);
    disp.print("*:Focus  #:Expose");
}

/// polling function executed while in the Main state
void FstopTimer::st_main_poll()
{   
    if(button.hadPress()){
        changeState(ST_FOCUS);
        return;
    }
    if(footswitch.hadPress()){
        changeState(ST_FOCUS);
        return;
    }
    if(keys.available()){
        char ch=keys.readAscii();
        if(isspace(ch))
            return;
        switch(ch){
        case 'A':
            // exec.current.clear();
            changeState(ST_EDIT);
            break;
        case 'B':
            changeState(ST_IO);        
            break;
        case 'C':
            changeState(ST_CONFIG);
            break;
        case 'D':
            changeState(ST_TEST);
            break;
        case '#':
            exec.setProgram(&current);
            changeState(ST_EXEC);
            break;
        case '*':
            changeState(ST_FOCUS);
            break;
        default:
            errorBeep();
        }
    }
    if(Serial.available()){
        changeState(ST_COMMS);
    }   
}

void FstopTimer::execCurrent()
{
    if(!current.compile(drydown_apply ? drydown : 0, splitgrade, currentPaper)){
        disp.print("Cannot Print");
        disp.setCursor(0, 1);
        disp.print("Dodges > Base");
        exec.setProgram(NULL);
        errorBeep();
        delay(2000);
        changeState(ST_EDIT);
    }
    else{
        exec.setProgram(&current);
        changeState(ST_EXEC);
    }
}

void FstopTimer::execTest()
{
    strip.configureStrip(stripbase, stripstep, stripcover);
    exec.setProgram(&strip);
    changeState(ST_EXEC);
}

void FstopTimer::st_exec_enter()
{
    Program *p=exec.getProgram();
    // we assume it compiles if we're in this state
    p->compile(drydown_apply ? drydown : 0, splitgrade, currentPaper);
    disp.clear();
    exec.setDrydown(drydown_apply);
    exec.setSplitgrade(splitgrade);

    if(focusphase >= 0){
        exec.changePhase(focusphase);
    }
    focusphase=-1;
}

void FstopTimer::st_exec_poll()
{
    if(button.hadPress() || footswitch.hadPress()){
        exec.expose();
        return;
    }   

    if(keys.available()){
        char ch=keys.readAscii();
        if(isspace(ch))
            return;
        switch(ch){
        case '#':
            // perform exposure!
            exec.expose();
            break;
        case '*':
            // focus
            focusphase=exec.getPhase();
            changeState(ST_FOCUS);
            break;
        case 'A':
            // abort/restart
            disp.clear();
            disp.print("Restart Exposure");
            delay(1000);
            exec.changePhase(0);
            break;
        case 'B':
            // skip exposure
            exec.nextPhase();
            break;
        case 'C':
            // main menu
            changeState(ST_MAIN);
            break;
        case '7':
            // toggle splitgrade
            toggleSplitgrade();
            if(exec.getPhase() != 0){
                disp.clear();
                disp.print("Restart Exposure");
                disp.setCursor(0,1);
                disp.print("For Splitgrade Chg");
                delay(1000);
            }
            // forces recompile -> apply splitgrade
            changeState(ST_EXEC);
            break;
        case 'D':
            // toggle drydown & recompile
            toggleDrydown();
            if(exec.getPhase() != 0){
                disp.clear();
                disp.print("Restart Exposure");
                disp.setCursor(0,1);
                disp.print("For Drydown Chg");
                delay(1000);
            }
            // forces recompile -> apply drydown
            changeState(ST_EXEC);
            break;
        } 
    }
}

void FstopTimer::st_focus_enter()
{
    disp.clear();
    disp.print("       Focus!");    
    leddriver.focusOn();
}

void FstopTimer::st_focus_poll()
{
    bool done=false;
    
    if (button.hadPress() || footswitch.hadPress()){
        done=true;
    }

    // return to prev state?
    if(keys.available()){
        keys.readRaw();
        done=true;
    }

    if(done){
      leddriver.allOff();
      changeState(prevstate);
    }
}

void FstopTimer::st_edit_enter()
{
    // don't print help
/*
    disp.clear();
    disp.setCursor(0,0);
    disp.print("Edit Program");
    disp.setCursor(0,1);
    disp.print("A:TX B:EV n:SLOT");
    delay(1000);
*/

    rotary.getDelta();       // clear any accum'd motion
    expnum=0;
    current.getStep(expnum).display(disp, dispbuf, false);
}

void FstopTimer::st_edit_poll()
{
    if(button.hadPress() || footswitch.hadPress()){
        exec.setProgram(&current);
        changeState(ST_EXEC);
        return;
    }   

    // keypad events?
    if(keys.available()){
        char ch=keys.readAscii();
        if(isspace(ch))
            return;
        switch(ch){
        case 'A':
            // edit text
            changeState(ST_EDIT_TEXT);
            break;
        case 'B':
            // edit EV
            changeState(ST_EDIT_EV);
            break;
        case 'C':
            changeState(ST_MAIN);
            break;
        case 'D':
            changeState(ST_EDIT_GRADE);
            break;
        case '#':
        case '*':
            exec.setProgram(&current);
            changeState(ST_EXEC);
            break;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
            // change to different step for editing
            expnum=ch-'1';
            current.getStep(expnum).display(disp, dispbuf, false);
            break;
        default:
            errorBeep();
        }
    }

    // knob events?
    int rot=rotary.getDelta();
    if(rot != 0){
        clampExposure(current.getStep(expnum).stops, rot*rotexp);
        current.getStep(expnum).display(disp, dispbuf, false);
    }
}

void FstopTimer::st_edit_ev_enter()
{
    deckey.setContext(&expctx);
}

void FstopTimer::st_edit_ev_poll()
{
    if(deckey.poll()){
        if(expctx.exitcode != Keypad::KP_C){
            current.getStep(expnum).stops=expctx.result;
        }
        current.getStep(expnum).display(disp, dispbuf, false);
        // sly state change without expnum=0
        curstate=ST_EDIT;
    }
}

void FstopTimer::st_edit_grade_enter()
{
    deckey.setContext(&gradectx);
}

void FstopTimer::st_edit_grade_poll()
{
    if(deckey.poll()){
        if(expctx.exitcode != Keypad::KP_C){
            // current.getStep(expnum).grade=constrain(gradectx.result, MINGRADE, MAXGRADE);
            // Round to units of 5. Always rounds down, but that's ok.
            unsigned char temp = (gradectx.result / 5)*5;
            current.getStep(expnum).grade=constrain(temp, MINGRADE, MAXGRADE);
        }
        current.getStep(expnum).display(disp, dispbuf, false);
        // sly state change without expnum=0
        curstate=ST_EDIT;
    }
}

void FstopTimer::st_edit_text_enter()
{
    keys.setContext(&smsctx);
}

void FstopTimer::st_edit_text_poll()
{
    if(keys.poll()){
        if(smsctx.exitcode != Keypad::KP_C){
            strcpy(current.getStep(expnum).text, smsctx.buffer);
        }
        current.getStep(expnum).display(disp, dispbuf, false);
        curstate=ST_EDIT;        
    }
}

void FstopTimer::st_io_enter()
{
    disp.clear();
    disp.print("A: New  B: Load");
    disp.setCursor(0, 1);
    disp.print("C: Save D: Main");
}

void FstopTimer::st_io_poll()
{
    if(keys.available()){
        char ch=keys.readAscii();
        switch(ch){
        case 'A':
            current.clear();
            changeState(ST_EDIT);
            break;
        case 'B':
            changeState(ST_IO_LOAD);
            break;
        case 'C':
            changeState(ST_IO_SAVE);
            break;
        case 'D':
            changeState(ST_MAIN);
            break;
        default:
            errorBeep();
        }
    }
}

void FstopTimer::st_io_load_enter()
{
    disp.clear();
    disp.print("Select Load Slot");
    deckey.setContext(&intctx);
}

void FstopTimer::st_io_load_poll()
{
    if(deckey.poll()){
        disp.clear();
        char slot=intctx.result;

        if(current.load(slot)){ 
            disp.print("Program Loaded");
        }
        else{
            disp.print("Slot not in 1..7");
            errorBeep();
        }
        delay(1000);

        changeState(ST_EDIT);
    }
}

void FstopTimer::st_io_save_enter()
{
    disp.clear();
    disp.print("Select Save Slot");
    deckey.setContext(&intctx);
}

void FstopTimer::st_io_save_poll()
{
    if(deckey.poll()){
        disp.clear();
        char slot=intctx.result;
        if(slot >= Program::FIRSTSLOT && slot <= Program::LASTSLOT){
            current.save(slot);
            disp.print("Program Saved");
        }
        else{
            disp.print("Slot not in 1..7");
            errorBeep();
        }
        delay(1000);

        changeState(ST_MAIN);
    }
}

void FstopTimer::st_comms_enter()
{
    comms.reset();
}

void FstopTimer::st_comms_poll()
{
    if(comms.poll()){
        changeState(ST_MAIN);
    }
}

void FstopTimer::st_test_enter()
{
    disp.clear();
    disp.print("A:");
    disp.print(stripcover ? "Cover" : "Indiv");
    disp.print(" B:Change");
    disp.setCursor(0, 1);
    dtostrf(0.01f*stripbase, 0, 2, dispbuf);
    disp.print(dispbuf);
    disp.print(" by ");
    dtostrf(0.01f*stripstep, 0, 2, dispbuf);
    disp.print(dispbuf);
    disp.setCursor(19, 2);
    disp.print(drydown_apply ? "D" : " ");
    rotary.getDelta();
}

void FstopTimer::st_test_poll()
{
    bool go=false;
    
    if (button.hadPress() || footswitch.hadPress()){
        go = true;
    }

    if(keys.available()){
        char ch=keys.readAscii();
        switch(ch){
        case 'A':
            // toggle type
            stripcover=!stripcover;
            EEPROM.write(EE_STRIPCOV, stripcover);
            changeState(ST_TEST);
            break;
        case 'B':
            // change exposures
            changeState(ST_TEST_CHANGEB);
            break;
        case 'C':
            // main menu
            changeState(ST_MAIN);
            break;
        case 'D':
            // toggle drydown
            toggleDrydown();
            changeState(ST_TEST);
            break;
        case '#':
            // perform exposure
            go=true;
            break;
        }
    }

    int rot=rotary.getDelta();
    if(rot != 0){
        clampExposure(stripbase, rot*rotexp);
        changeState(ST_TEST);
    }

    if(go){
        strip.configureStrip(stripbase, stripstep, stripcover);
        exec.setProgram(&strip);
        changeState(ST_EXEC);
    }
}

void FstopTimer::st_test_changeb_enter()
{
    disp.clear();
    disp.print("Test Strip Base:");
    deckey.setContext(&expctx);
}

void FstopTimer::st_test_changeb_poll()
{
    if(deckey.poll()){
        if(expctx.exitcode != Keypad::KP_C){
            stripbase=expctx.result;
            EEPROM.write(EE_STRIPBASE, (stripbase >> 8) & 0xFF);
            EEPROM.write(EE_STRIPBASE+1, stripbase & 0xFF);
        }
        changeState(ST_TEST_CHANGES);
    }
}

void FstopTimer::st_test_changes_enter()
{
    disp.clear();
    disp.print("Test Strip Step:");
    deckey.setContext(&stepctx);
}

void FstopTimer::st_test_changes_poll()
{
    if(deckey.poll()){
        if(stepctx.exitcode != Keypad::KP_C){
            stripstep=stepctx.result;
            EEPROM.write(EE_STRIPSTEP, (stripstep >> 8) & 0xFF);
            EEPROM.write(EE_STRIPSTEP+1, stripstep & 0xFF);
        }
        changeState(ST_TEST);
    }
}

void FstopTimer::st_config_enter()
{
    disp.clear();
    disp.setCursor(0,0);
    disp.print("Config  A:Rotary");
    disp.setCursor(0,1);
    disp.print("B:Brite D:Drydn"); 
    disp.setCursor(0,2);
    disp.print("0:Cal Light");
}

void FstopTimer::st_config_poll()
{
    if(keys.available()){
        char ch=keys.readAscii();
        switch(ch){
        case 'A':
            changeState(ST_CONFIG_ROTARY);
            break;
        case 'B': 
            // change brightness
            ++brightness;
            if(brightness > BL_MAX)
                brightness=BL_MIN;
            EEPROM.write(EE_BACKLIGHT, brightness);
            setBacklight();
            break;
        case 'D':
            // change drydown
            changeState(ST_CONFIG_DRY);
            break;
        case '0':
            changeState(ST_CALIBRATE_LIGHT);
            break;
        default:
            // main menu
            changeState(ST_MAIN);
        }
    }
}
void FstopTimer::st_calibrate_light_enter()
{
    disp.clear();
    disp.setCursor(0,0);
    disp.print("#:Start");
}

void FstopTimer::st_calibrate_light_poll()
{
    if(keys.available()){
        char ch=keys.readAscii();
        switch(ch){
            case '#':
                calibrateLightSource(SOFT);
                calibrateLightSource(HARD);
                delay(1000);
                changeState(ST_CALIBRATE_LIGHT);
                break;
            default:
                // main menu
                changeState(ST_MAIN);
        }
    }
}

void FstopTimer::calibrateLightSource(Contrast_Enum source)
{
    char output_buffer[20];
    File dataFile;
    SD.mkdir("/cal/");
    if (source == SOFT) {
        if (SD.exists("/cal/soft.txt")) {
            SD.remove("/cal/soft.txt");
        }
        dataFile = SD.open("/cal/soft.txt", FILE_WRITE);
    } else {
        if (SD.exists("/cal/hard.txt")) {
            SD.remove("/cal/hard.txt");
        }
        dataFile = SD.open("/cal/hard.txt", FILE_WRITE);
    }

    if (dataFile) {
        disp.setCursor(0, 0);
        if (source == SOFT){
            disp.print("Reading Soft Light");
        } else {
            disp.print("Reading Hard Light");
        }
        dataFile.println("i, ir_spectrum, full_spectrum");
        for (int i = 0; i <= 255 ; i++){
            disp.setCursor(0, 2);
            if (source == SOFT){
                leddriver.calibrateOn(255, i, 255, i);      
            } else {
                leddriver.calibrateOn(i, 255, i, 255);      
            }

            delay(500);
            uint32_t full_luminosity = tsl.getFullLuminosity();
            uint16_t ir_spectrum = full_luminosity >> 16;
            uint16_t full_spectrum = full_luminosity & 0xFFFF;    
            snprintf_P(output_buffer, 20, PSTR("%3d %6d %6d"), i, ir_spectrum, full_spectrum);
            disp.print(output_buffer);
    
            dataFile.print(i);
            dataFile.print(",");
            dataFile.print(ir_spectrum);
            dataFile.print(",");
            dataFile.println(full_spectrum);
        }
        leddriver.allOff();
        dataFile.close();
    } else {
        disp.setCursor(0, 0);
        disp.print("Error opening file");
    }
}

void FstopTimer::st_config_dry_enter()
{
    disp.clear();
    disp.print("Drydown Factor:");
    deckey.setContext(&dryctx);
}

void FstopTimer::st_config_dry_poll()
{
    if(deckey.poll()){
        disp.clear();
        if(dryctx.exitcode == Keypad::KP_C){
            disp.print("Cancelled");
        }
        else{
            drydown=abs(dryctx.result);
            EEPROM.write(EE_DRYDOWN, drydown);
            disp.print("Changed");
        }
        disp.setCursor(0, 1);
        disp.print("Drydown = ");
        dtostrf(0.01f*drydown, 0, 2, dispbuf);
        disp.print(dispbuf);
        delay(1000);
    
        changeState(ST_MAIN);
    }
}


void FstopTimer::st_config_rotary_enter()
{
    disp.clear();
    disp.print("Rotary Step:");
    deckey.setContext(&dryctx);
}

void FstopTimer::st_config_rotary_poll()
{
    if(deckey.poll()){
        disp.clear();
        if(dryctx.exitcode == Keypad::KP_C){
            disp.print("Cancelled");
        }
        else{
            rotexp=abs(dryctx.result);
            EEPROM.write(EE_ROTARY, rotexp);
            disp.print("Changed");
        }
        disp.setCursor(0, 1);
        disp.print("Step = ");
        dtostrf(0.01f*rotexp, 0, 2, dispbuf);
        disp.print(dispbuf);
        delay(1000);
    
        changeState(ST_MAIN);
    }
}

void FstopTimer::changeState(int st)
{
    prevstate=curstate;
    curstate=st;
    button.hadPress();  // clear any press that might interfere in next state
    footswitch.hadPress();
    (this->*sm_enter[curstate])();
}


void FstopTimer::poll()
{
    // scan keypad
    keys.scan();
    button.scan();
    footswitch.scan();
  
    // attend to whatever the state requires
    (this->*sm_poll[curstate])(); 
}

void FstopTimer::clampExposure(int &expos, int delta)
{
    expos+=delta;

    if(expos > MAXSTOP)
        expos=MAXSTOP;
    if(expos < MINSTOP)
        expos=MINSTOP;
}

void FstopTimer::errorBeep()
{
    tone(pin_beep, 2000, 100); 
}

