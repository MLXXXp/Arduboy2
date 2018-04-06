// ========================================
// Manage an Arduboy's system EEPROM area
// ========================================

/*
------------------------------------------------------------------------------
This sketch allows manipulation of the following values in system EEPROM:

- The Unit Name. This is a 6 character field that can be read by sketches
  using the readUnitName() function. Also, the Arduboy2 class will display
  the Unit Name at the end of the boot logo sequence, if possible, during
  start up.

- The Unit ID. This is a 16 bit value that can be read by sketches using the
  readUnitID() function.

- The "Show Unit Name" flag. This flag indicates whether or not the Unit Name
  should be displayed at the end of the boot logo sequence in circumstances
  where it's possible to do so.

- The "Show RGB LEDs with Boot Logo" flag. This flag indicates whether or not
  to flash the RGB LEDs while the boot logo is scrolling down.

- The "Show Boot Logo" flag. This flag indicates whether or not to display
  the boot logo sequence during start up.

This sketch also allows:

- The entire System EEPROM area to be reset back to default values.

- The entire User EEPROM area to be reset. This will clear the high scores
  and any other data saved by ALL sketches that have ever been installed.
------------------------------------------------------------------------------
*/

// Version 2.0

/*
------------------------------------------------------------------------------
Copyright (c) 2018, Scott Allen
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
3. Neither the name of the copyright holders nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS "AS IS" AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
------------------------------------------------------------------------------
*/

#include <Arduboy2.h>
#include <EEPROM.h>

// The frame rate determines the button auto-repeat rate for unit name entry
#define FRAME_RATE 10

// The unit ID auto-repeat rate is slowed, compared to the unit name rate, by
// repeating only once per the defined number of frames
#define ID_REPEAT_FRAMES 3

// Delay time before button auto-repeat starts, in milliseconds
#define REPEAT_DELAY 700

// All the constant stings
const char StrName[] PROGMEM = "NAME";
const char StrID[] PROGMEM = "ID";
const char StrFlags[] PROGMEM = "FLAGS";
const char StrYes[] PROGMEM = "YES";
const char StrNo[] PROGMEM = "NO";
const char StrSaveQ[] PROGMEM = "SAVE?";
const char StrSaved[] PROGMEM = "SAVED";
const char StrBtnChangeName[] PROGMEM = "UP:change Unit Name";
const char StrBtnChangeID[] PROGMEM = "DOWN:change Unit ID";
const char StrBtnFlags[] PROGMEM = "LEFT:flags";
const char StrBtnReset[] PROGMEM = "RIGHT:reset";
const char StrBtnMenu[] PROGMEM = "A:menu";
const char StrBtnSave[] PROGMEM = "B:save";
const char StrBtnYes[] PROGMEM = "A:yes";
const char StrBtnNo[] PROGMEM = "B:no";
const char StrShowLogoQ[] PROGMEM = "show boot logo?";
const char StrShowLEDsQ[] PROGMEM = "show boot LEDs?";
const char StrShowNameQ[] PROGMEM = "show unit name?";
const char StrBtnTestLogo[] PROGMEM = "UP+DOWN:test logo";
const char StrNoLogo1[] PROGMEM = "\"SHOW BOOT LOGO\"";
const char StrNoLogo2[] PROGMEM = "flag is OFF";
const char StrBtnResetSys[] PROGMEM = "UP:reset system";
const char StrBtnResetUser[] PROGMEM = "Down:reset user";
const char StrEEPROM[] PROGMEM = "EEPROM";
const char StrResetSys1[] PROGMEM = "EEPROM reserved for";
const char StrResetSys2[] PROGMEM = "system use will be";
const char StrResetSys3[] PROGMEM = "reset to defaults!";
const char StrResetUser1[] PROGMEM = "EEPROM containing";
const char StrResetUser2[] PROGMEM = "ALL saved sketch data";
const char StrResetUser3[] PROGMEM = "will be erased!";
const char StrAreYouSureQ[] PROGMEM = "ARE YOU SURE?";
const char StrBtnResetYes[] PROGMEM = "YES:hold A, press B";
const char StrBtnResetNo[] PROGMEM = "NO:any D-pad button";
const char StrWriting[] PROGMEM = "WRITING...";
const char StrSystem[] PROGMEM = "SYSTEM";
const char StrUser[] PROGMEM = "USER";
const char StrReset[] PROGMEM = "RESET";
const char StrHex[] PROGMEM = "hex";
const char StrDecimal[] PROGMEM = "decimal";

#define CHAR_WIDTH 6
#define CHAR_HEIGHT 8
#define SMALL_SPACE 4 // The number of pixels for a small space between groups

// Defines for text and field locations
#define MENU_BTN_CHANGE_NAME_X centerStr_P(StrBtnChangeName)
#define MENU_BTN_CHANGE_NAME_Y 0
#define MENU_NAME_X centerStrLen(ARDUBOY_UNIT_NAME_LEN)
#define MENU_NAME_Y (MENU_BTN_CHANGE_NAME_Y + CHAR_HEIGHT + 3)

#define MENU_BTN_CHANGE_ID_X centerStr_P(StrBtnChangeID)
#define MENU_BTN_CHANGE_ID_Y 26
#define MENU_HEADING_HEX_X (centerStr_P(StrHex) - (WIDTH / 4))
#define MENU_HEADING_DECIMAL_X (centerStr_P(StrDecimal) + (WIDTH / 4))
#define MENU_HEADINGS_Y (MENU_BTN_CHANGE_ID_Y + CHAR_HEIGHT + 1)
#define MENU_ID_HEX_X (centerStrLen(5) - (WIDTH / 4))
#define MENU_ID_DECIMAL_X (centerStrLen(5) + (WIDTH / 4))
#define MENU_ID_Y (MENU_HEADINGS_Y + CHAR_HEIGHT + 1)

#define MENU_BTN_FLAGS_X 0
#define MENU_BTN_FLAGS_Y 56
#define MENU_BTN_RESET_X rightStr_P(StrBtnReset)
#define MENU_BTN_RESET_Y MENU_BTN_FLAGS_Y


#define NAME_TITLE_X centerStr_P(StrName)
#define NAME_TITLE_Y 0

#define NAME_BTN_MENU_X 0
#define NAME_BTN_MENU_Y 0
#define NAME_BTN_SAVE_X rightStr_P(StrBtnSave)
#define NAME_BTN_SAVE_Y NAME_BTN_MENU_Y

#define NAME_LARGE_X centerStrLen(ARDUBOY_UNIT_NAME_LEN * 2)
#define NAME_LARGE_Y 11

#define NAME_HEX_X 0
#define NAME_HEX_Y 40
#define NAME_DECIMAL_X 0
#define NAME_DECIMAL_Y 54

#define NAME_BTN_YES_X 0
#define NAME_BTN_YES_Y 0
#define NAME_BTN_NO_X rightStr_P(StrBtnNo)
#define NAME_BTN_NO_Y NAME_BTN_YES_Y

#define NAME_SAVE_Q_X (centerStr_P(StrSaveQ) - ((6 * CHAR_WIDTH) + (CHAR_WIDTH / 2)))
#define NAME_SAVE_Q_Y (NAME_LARGE_Y + (CHAR_HEIGHT / 2) + 3)
#define NAME_SAVE_X (NAME_SAVE_Q_X + ((strlen_P(StrSaveQ) * CHAR_WIDTH) + CHAR_WIDTH))
#define NAME_SAVE_Y (NAME_LARGE_Y + 3)


#define ID_TITLE_X centerStr_P(StrID)
#define ID_TITLE_Y 0

#define ID_BTN_MENU_X 0
#define ID_BTN_MENU_Y 0
#define ID_BTN_SAVE_X rightStr_P(StrBtnSave)
#define ID_BTN_SAVE_Y ID_BTN_MENU_Y

#define ID_LARGE_X centerStrLen(5 * 2)
#define ID_LARGE_Y 13

#define ID_2_DECIMAL_X 12
#define ID_2_DECIMAL_Y 38
#define ID_DECIMAL_X (WIDTH - (CHAR_WIDTH * 5 + 12))
#define ID_DECIMAL_Y ID_2_DECIMAL_Y

#define ID_BINARY_X 0
#define ID_BINARY_Y 54

#define ID_BTN_YES_X 0
#define ID_BTN_YES_Y 0
#define ID_BTN_NO_X rightStr_P(StrBtnNo)
#define ID_BTN_NO_Y ID_BTN_YES_Y

#define ID_SAVE_Q_X (centerStr_P(StrSaveQ) - ((5 * CHAR_WIDTH) + (CHAR_WIDTH / 2)))
#define ID_SAVE_Q_Y (ID_LARGE_Y + (CHAR_HEIGHT / 2) + 1)
#define ID_SAVE_X (ID_SAVE_Q_X + ((strlen_P(StrSaveQ) * CHAR_WIDTH) + CHAR_WIDTH))
#define ID_SAVE_Y (ID_LARGE_Y + 1)

#define FLAGS_TITLE_X centerStr_P(StrFlags)
#define FLAGS_TITLE_Y 0

#define FLAGS_BTN_MENU_X 0
#define FLAGS_BTN_MENU_Y 0
#define FLAGS_BTN_SAVE_X rightStr_P(StrBtnSave)
#define FLAGS_BTN_SAVE_Y FLAGS_BTN_MENU_Y

#define FLAGS_LOGO_Y 14
#define FLAGS_LEDS_Y 27
#define FLAGS_NAME_Y 40

#define FLAGS_Q_X  (CHAR_WIDTH * 2)
#define FLAGS_SET_X rightStr_P(StrYes)
#define FLAGS_CURSOR_X 0

#define FLAGS_TEST_X centerStr_P(StrBtnTestLogo)
#define FLAGS_TEST_Y 56

#define FLAGS_SAVED_X centerStr2_P(StrSaved)
#define FLAGS_SAVED_Y ((HEIGHT / 2) - CHAR_HEIGHT)

#define FLAGS_NO_LOGO_1_X centerStr_P(StrNoLogo1)
#define FLAGS_NO_LOGO_1_Y ((HEIGHT / 2) - CHAR_HEIGHT - 1)
#define FLAGS_NO_LOGO_2_X centerStr_P(StrNoLogo2)
#define FLAGS_NO_LOGO_2_Y (FLAGS_NO_LOGO_1_Y + CHAR_HEIGHT + 2)

#define RESET_BTN_MENU_X centerStr_P(StrBtnMenu)
#define RESET_BTN_MENU_Y 0
#define RESET_BTN_SYS_X centerStr_P(StrBtnResetSys)
#define RESET_BTN_SYS_Y 16
#define RESET_BTN_SYS_EEPROM_X centerStr_P(StrEEPROM)
#define RESET_BTN_SYS_EEPROM_Y (RESET_BTN_SYS_Y + CHAR_HEIGHT)
#define RESET_BTN_USER_X centerStr_P(StrBtnResetUser)
#define RESET_BTN_USER_Y 40
#define RESET_BTN_USER_EEPROM_X centerStr_P(StrEEPROM)
#define RESET_BTN_USER_EEPROM_Y (RESET_BTN_USER_Y + CHAR_HEIGHT)

#define RESET_SYS_TEXT_1_X centerStr_P(StrResetSys1)
#define RESET_SYS_TEXT_1_Y 0
#define RESET_SYS_TEXT_2_X centerStr_P(StrResetSys2)
#define RESET_SYS_TEXT_2_Y (RESET_SYS_TEXT_1_Y + CHAR_HEIGHT)
#define RESET_SYS_TEXT_3_X centerStr_P(StrResetSys3)
#define RESET_SYS_TEXT_3_Y (RESET_SYS_TEXT_2_Y + CHAR_HEIGHT)
#define RESET_SYS_SURE_Q_X centerStr_P(StrAreYouSureQ)
#define RESET_SYS_SURE_Q_Y 32
#define RESET_SYS_BTN_YES_X centerStr_P(StrBtnResetYes)
#define RESET_SYS_BTN_YES_Y 48
#define RESET_SYS_BTN_NO_X centerStr_P(StrBtnResetYes)
#define RESET_SYS_BTN_NO_Y (RESET_SYS_BTN_YES_Y + CHAR_HEIGHT)

#define RESET_SYS_CONFIRMED_1_X centerStr2_P(StrSystem)
#define RESET_SYS_CONFIRMED_1_Y 7
#define RESET_SYS_CONFIRMED_2_X centerStr2_P(StrEEPROM)
#define RESET_SYS_CONFIRMED_2_Y (RESET_SYS_CONFIRMED_1_Y + (CHAR_HEIGHT * 2) + 2)
#define RESET_SYS_CONFIRMED_3_X centerStr2_P(StrReset)
#define RESET_SYS_CONFIRMED_3_Y (RESET_SYS_CONFIRMED_2_Y + (CHAR_HEIGHT * 2) + 2)

#define RESET_USER_TEXT_1_X centerStr_P(StrResetUser1)
#define RESET_USER_TEXT_1_Y 0
#define RESET_USER_TEXT_2_X centerStr_P(StrResetUser2)
#define RESET_USER_TEXT_2_Y (RESET_USER_TEXT_1_Y + CHAR_HEIGHT)
#define RESET_USER_TEXT_3_X centerStr_P(StrResetUser3)
#define RESET_USER_TEXT_3_Y (RESET_USER_TEXT_2_Y + CHAR_HEIGHT)
#define RESET_USER_SURE_Q_X centerStr_P(StrAreYouSureQ)
#define RESET_USER_SURE_Q_Y 32
#define RESET_USER_BTN_YES_X centerStr_P(StrBtnResetYes)
#define RESET_USER_BTN_YES_Y 48
#define RESET_USER_BTN_NO_X centerStr_P(StrBtnResetYes)
#define RESET_USER_BTN_NO_Y (RESET_USER_BTN_YES_Y + CHAR_HEIGHT)

#define RESET_USER_CONFIRMED_1_X centerStr2_P(StrUser)
#define RESET_USER_CONFIRMED_1_Y 7
#define RESET_USER_CONFIRMED_2_X centerStr2_P(StrEEPROM)
#define RESET_USER_CONFIRMED_2_Y (RESET_USER_CONFIRMED_1_Y + (CHAR_HEIGHT * 2) + 2)
#define RESET_USER_CONFIRMED_3_X centerStr2_P(StrReset)
#define RESET_USER_CONFIRMED_3_Y (RESET_USER_CONFIRMED_2_Y + (CHAR_HEIGHT * 2) + 2)

#define RESET_USER_WRITING_X centerStr2_P(StrWriting)
#define RESET_USER_WRITING_Y ((HEIGHT / 2) - (CHAR_HEIGHT - 1))

// EEPROM addresses
#define EEPROM_START     (0x0000)
#define EEPROM_SIZE      (1024)
#define EEPROM_END       (EEPROM_START + EEPROM_SIZE - 1)

// Calculation of the number of frames to wait before button auto-repeat starts
#define DELAY_FRAMES (REPEAT_DELAY / (1000 / FRAME_RATE))

// The Arduino "magic" has trouble creating prototypes for functions called
// by pointers, so they're declared here manually
void stateMain(), stateName(), stateID(), stateFlags(), stateReset();
void stateSaveName(), stateSaveID(), stateResetSys(), stateResetUser();
void screenMain(), screenName(), screenID(), screenFlags(), screenReset();
void screenSaveName(), screenSaveID(), screenResetSys(), screenResetUser();

Arduboy2 arduboy;

char unitName[ARDUBOY_UNIT_NAME_LEN + 1];
byte nameIndex;

uint16_t unitID;
byte idIndex;

boolean showLogoFlag;
boolean showLEDsFlag;
boolean showNameFlag;

// Selected flag
enum SelectedFlag : byte {
  selFlagLogo,
  selFlagLEDs,
  selFlagName
};

byte currentFlag;

// Assign numbers for each state/screen
enum State : byte {
  sMain,
  sName,
  sID,
  sFlags,
  sReset,
  sSaveName,
  sSaveID,
  sResetSys,
  sResetUser,
  sMAX = sResetUser
};

byte currentState;

// Function pointer array for button handling
void (*stateFunc[sMAX + 1])() = {
  stateMain,
  stateName,
  stateID,
  stateFlags,
  stateReset,
  stateSaveName,
  stateSaveID,
  stateResetSys,
  stateResetUser
};

// Function pointer array for screen drawing
void (*screenFunc[sMAX + 1])() = {
  screenMain,
  screenName,
  screenID,
  screenFlags,
  screenReset,
  screenSaveName,
  screenSaveID,
  screenResetSys,
  screenResetUser
};

unsigned int delayCount = 0;
boolean repeating = false;


// ============================= SETUP ===================================
void setup() {
  arduboy.begin();
  arduboy.setFrameRate(FRAME_RATE);
  setState(sMain);
}
// =======================================================================


// =========================== MAIN LOOP =================================
void loop() {
  if (!arduboy.nextFrame()) {
    return;
  }

  arduboy.pollButtons();

  (*stateFunc[currentState])();

  if ((delayCount != 0) && (--delayCount == 0)) {
    repeating = true;
  }
}
// =======================================================================


// ------------------------- Program States ------------------------------

// Set to the given state and display the screen for that state
// Can be called with the current state to update the current screen
void setState(byte newState) {
  currentState = newState;
  stopButtonRepeat();
  drawScreen();
}

// STATE: Main selection screen
void stateMain() {
  if (arduboy.justPressed(UP_BUTTON)) {
    setState(sName);
  }
  else if (arduboy.justPressed(DOWN_BUTTON)) {
    setState(sID);
  }
  else if (arduboy.justPressed(LEFT_BUTTON)) {
    setState(sFlags);
  }
  else if (arduboy.justPressed(RIGHT_BUTTON)) {
    setState(sReset);
  }
}

// STATE: Change unit name
void stateName() {
  if (arduboy.justPressed(UP_BUTTON)) {
    nameCharInc();
    startButtonDelay();
  }
  else if (arduboy.justPressed(DOWN_BUTTON)) {
    nameCharDec();
    startButtonDelay();
  }
  else if (repeating && arduboy.pressed(UP_BUTTON)) {
    nameCharInc();
  }
  else if (repeating && arduboy.pressed(DOWN_BUTTON)) {
    nameCharDec();
  }
  else if (arduboy.justPressed(RIGHT_BUTTON)) {
    nameCursorRight();
  }
  else if (arduboy.justPressed(LEFT_BUTTON)) {
    nameCursorLeft();
  }
  else if (arduboy.justPressed(A_BUTTON)) {
    setState(sMain);
  }
  else if (arduboy.justPressed(B_BUTTON)) {
    setState(sSaveName);
  }
  else if (repeating) {
    stopButtonRepeat();
  }
}

// STATE: Change unit ID
void stateID() {
  if (arduboy.justPressed(UP_BUTTON)) {
    idDigitInc();
    startButtonDelay();
  }
  else if (arduboy.justPressed(DOWN_BUTTON)) {
    idDigitDec();
    startButtonDelay();
  }
  else if (repeating && arduboy.pressed(UP_BUTTON)) {
    if (arduboy.everyXFrames(ID_REPEAT_FRAMES)) {
      idDigitInc();
    }
  }
  else if (repeating && arduboy.pressed(DOWN_BUTTON)) {
    if (arduboy.everyXFrames(ID_REPEAT_FRAMES)) {
      idDigitDec();
    }
  }
  else if (arduboy.justPressed(RIGHT_BUTTON)) {
    idCursorRight();
  }
  else if (arduboy.justPressed(LEFT_BUTTON)) {
    idCursorLeft();
  }
  else if (arduboy.justPressed(A_BUTTON)) {
    setState(sMain);
  }
  else if (arduboy.justPressed(B_BUTTON)) {
    setState(sSaveID);
  }
  else if (repeating) {
    stopButtonRepeat();
  }
}

// STATE: Set system flags
void stateFlags() {
  if (arduboy.pressed(UP_BUTTON + DOWN_BUTTON)) {
    showNameFlag = arduboy.readShowUnitNameFlag();
    showLEDsFlag = arduboy.readShowBootLogoLEDsFlag();
    if ((showLogoFlag = arduboy.readShowBootLogoFlag()) == true) {
      arduboy.bootLogo();
    }
    else {
      displayNoLogo();
    }
    currentFlag = selFlagLogo;
    setState(sFlags);
  }
  else if (arduboy.justPressed(UP_BUTTON)) {
    flagsCursorUp();
  }
  else if (arduboy.justPressed(DOWN_BUTTON)) {
    flagsCursorDown();
  }
  else if (arduboy.justPressed(RIGHT_BUTTON) ||
           arduboy.justPressed(LEFT_BUTTON)) {
    flagToggle();
  }
  else if (arduboy.justPressed(A_BUTTON)) {
    setState(sMain);
  }
  else if (arduboy.justPressed(B_BUTTON)) {
    saveFlags();
    setState(sFlags);
  }
}

// STATE: Reset EEPROM areas
void stateReset() {
  if (arduboy.justPressed(UP_BUTTON)) {
    setState(sResetSys);
  }
  else if (arduboy.justPressed(DOWN_BUTTON)) {
    setState(sResetUser);
  }
  else if (arduboy.justPressed(A_BUTTON)) {
    setState(sMain);
  }
}

// STATE: Prompt to save the unit name
void stateSaveName() {
  if (arduboy.justPressed(A_BUTTON)) {
    arduboy.writeUnitName(unitName);
    setState(sMain);
  }
  else if (arduboy.justPressed(B_BUTTON)) {
    setState(sName);
  }
}

// STATE: Prompt to save the unit ID
void stateSaveID() {
  if (arduboy.justPressed(A_BUTTON)) {
    arduboy.writeUnitID(unitID);
    setState(sMain);
  }
  else if (arduboy.justPressed(B_BUTTON)) {
    setState(sID);
  }
}

// STATE: Prompt to reset system EEPROM
void stateResetSys() {
  if (arduboy.justPressed(B_BUTTON) && arduboy.pressed(A_BUTTON)) {
    resetSysEEPROM();
    setState(sReset);
  }
  else if (arduboy.justPressed(UP_BUTTON) ||
           arduboy.justPressed(DOWN_BUTTON) ||
           arduboy.justPressed(RIGHT_BUTTON) ||
           arduboy.justPressed(LEFT_BUTTON)) {
    setState(sReset);
  }
}

// STATE: Prompt to reset user EEPROM
void stateResetUser() {
  if (arduboy.justPressed(B_BUTTON) && arduboy.pressed(A_BUTTON)) {
    resetUserEEPROM();
    setState(sReset);
  }
  else if (arduboy.justPressed(UP_BUTTON) ||
           arduboy.justPressed(DOWN_BUTTON) ||
           arduboy.justPressed(RIGHT_BUTTON) ||
           arduboy.justPressed(LEFT_BUTTON)) {
    setState(sReset);
  }
}

// ------------------------- Screen Display ------------------------------

// Display the screen for the current state
void drawScreen() {
  arduboy.clear();
  (*screenFunc[currentState])();
  arduboy.display();
}

// DISPLAY: Main screen
void screenMain() {
  readEEPROM();
  nameIndex = idIndex = 0;
  currentFlag = selFlagLogo;

  printStr_P(MENU_BTN_CHANGE_NAME_X, MENU_BTN_CHANGE_NAME_Y, StrBtnChangeName);
  printName(MENU_NAME_X, MENU_NAME_Y);

  printStr_P(MENU_BTN_CHANGE_ID_X, MENU_BTN_CHANGE_ID_Y, StrBtnChangeID);
  printStr_P(MENU_HEADING_HEX_X, MENU_HEADINGS_Y, StrHex);
  printStr_P(MENU_HEADING_DECIMAL_X, MENU_HEADINGS_Y, StrDecimal);
  printIDHex(MENU_ID_HEX_X, MENU_ID_Y);
  printIDDecimal(MENU_ID_DECIMAL_X, MENU_ID_Y);

  printStr_P(MENU_BTN_FLAGS_X, MENU_BTN_FLAGS_Y, StrBtnFlags);
  printStr_P(MENU_BTN_RESET_X, MENU_BTN_RESET_Y, StrBtnReset);
}

// DISPLAY: Change unit name
void screenName() {
  printNameScreenCommon();
  printStr_P(NAME_BTN_MENU_X, NAME_BTN_MENU_Y, StrBtnMenu);
  printStr_P(NAME_BTN_SAVE_X, NAME_BTN_SAVE_Y, StrBtnSave);
  printNameLarge(NAME_LARGE_X, NAME_LARGE_Y);
  printNameUnderline(NAME_LARGE_X, NAME_LARGE_Y);
  printNameCursors();
}

// DISPLAY: Change unit ID
void screenID() {
  printIDScreenCommon();
  printStr_P(ID_BTN_MENU_X, ID_BTN_MENU_Y, StrBtnMenu);
  printStr_P(ID_BTN_SAVE_X, ID_BTN_SAVE_Y, StrBtnSave);
  printIDLarge(ID_LARGE_X, ID_LARGE_Y);
  printIDCursors();
}

// DISPLAY: Set system flags
void screenFlags() {
  printStr_P(FLAGS_BTN_MENU_X, FLAGS_BTN_MENU_Y, StrBtnMenu);
  printStr_P(FLAGS_BTN_SAVE_X, FLAGS_BTN_SAVE_Y, StrBtnSave);
  printStr_P(FLAGS_TITLE_X, FLAGS_TITLE_Y, StrFlags);
  printStr_P(FLAGS_Q_X, FLAGS_LOGO_Y, StrShowLogoQ);
  printStr_P(FLAGS_Q_X, FLAGS_LEDS_Y, StrShowLEDsQ);
  printStr_P(FLAGS_Q_X, FLAGS_NAME_Y, StrShowNameQ);
  printFlagSettings();
  printStr_P(FLAGS_TEST_X, FLAGS_TEST_Y, StrBtnTestLogo);
}

// DISPLAY: Reset EEPROM areas
void screenReset() {
  printStr_P(RESET_BTN_MENU_X, RESET_BTN_MENU_Y, StrBtnMenu);
  printStr_P(RESET_BTN_SYS_X, RESET_BTN_SYS_Y, StrBtnResetSys);
  printStr_P(RESET_BTN_SYS_EEPROM_X, RESET_BTN_SYS_EEPROM_Y, StrEEPROM);
  printStr_P(RESET_BTN_USER_X, RESET_BTN_USER_Y, StrBtnResetUser);
  printStr_P(RESET_BTN_USER_EEPROM_X, RESET_BTN_USER_EEPROM_Y, StrEEPROM);
}

// DISPLAY: Prompt to save the unit name
void screenSaveName() {
  printNameScreenCommon();
  printStr_P(NAME_BTN_YES_X, NAME_BTN_YES_Y, StrBtnYes);
  printStr_P(NAME_BTN_NO_X, NAME_BTN_NO_Y, StrBtnNo);
  printSavePrompt(NAME_SAVE_Q_X, NAME_SAVE_Q_Y);
  printNameLarge(NAME_SAVE_X, NAME_SAVE_Y);
}

// DISPLAY: Prompt to save the unit ID
void screenSaveID() {
  printIDScreenCommon();
  printStr_P(ID_BTN_YES_X, ID_BTN_YES_Y, StrBtnYes);
  printStr_P(ID_BTN_NO_X, ID_BTN_NO_Y, StrBtnNo);
  printSavePrompt(ID_SAVE_Q_X, ID_SAVE_Q_Y);
  printIDLarge(ID_SAVE_X, ID_SAVE_Y);
}

// DISPLAY: Propmt to reset the system EEPROM area
void screenResetSys() {
  printStr_P(RESET_SYS_TEXT_1_X, RESET_SYS_TEXT_1_Y, StrResetSys1);
  printStr_P(RESET_SYS_TEXT_2_X, RESET_SYS_TEXT_2_Y, StrResetSys2);
  printStr_P(RESET_SYS_TEXT_3_X, RESET_SYS_TEXT_3_Y, StrResetSys3);
  printStr_P(RESET_SYS_SURE_Q_X, RESET_SYS_SURE_Q_Y, StrAreYouSureQ);
  printStr_P(RESET_SYS_BTN_YES_X, RESET_SYS_BTN_YES_Y, StrBtnResetYes);
  printStr_P(RESET_SYS_BTN_NO_X, RESET_SYS_BTN_NO_Y, StrBtnResetNo);
}

// DISPLAY: Propmt to reset the user EEPROM area
void screenResetUser() {
  printStr_P(RESET_USER_TEXT_1_X, RESET_USER_TEXT_1_Y, StrResetUser1);
  printStr_P(RESET_USER_TEXT_2_X, RESET_USER_TEXT_2_Y, StrResetUser2);
  printStr_P(RESET_USER_TEXT_3_X, RESET_USER_TEXT_3_Y, StrResetUser3);
  printStr_P(RESET_USER_SURE_Q_X, RESET_USER_SURE_Q_Y, StrAreYouSureQ);
  printStr_P(RESET_USER_BTN_YES_X, RESET_USER_BTN_YES_Y, StrBtnResetYes);
  printStr_P(RESET_USER_BTN_NO_X, RESET_USER_BTN_NO_Y, StrBtnResetNo);
}

// Display a message indicating "Show boot logo" flag is off
void displayNoLogo() {
  arduboy.clear();
  printStr_P(FLAGS_NO_LOGO_1_X, FLAGS_NO_LOGO_1_Y, StrNoLogo1);
  printStr_P(FLAGS_NO_LOGO_2_X, FLAGS_NO_LOGO_2_Y, StrNoLogo2);
  arduboy.display();
  arduboy.delayShort(2000);
}

// Save the system flags and overlay the "SAVED" message on the screen
void saveFlags() {
  arduboy.writeShowUnitNameFlag(showNameFlag);
  arduboy.writeShowBootLogoFlag(showLogoFlag);
  arduboy.writeShowBootLogoLEDsFlag(showLEDsFlag);
  printStrLargeRev_P(FLAGS_SAVED_X, FLAGS_SAVED_Y, StrSaved);
  arduboy.display();
  arduboy.delayShort(1500);
}

// Reset the system EEPROM area and display the confirmation message
void resetSysEEPROM() {
  for (unsigned int i = EEPROM_START; i < EEPROM_STORAGE_SPACE_START; i++) {
    EEPROM.update(i, 0xFF);
  }
  arduboy.clear();
  printStrLargeRev_P(RESET_SYS_CONFIRMED_1_X, RESET_SYS_CONFIRMED_1_Y, StrSystem);
  printStrLargeRev_P(RESET_SYS_CONFIRMED_2_X, RESET_SYS_CONFIRMED_2_Y, StrEEPROM);
  printStrLargeRev_P(RESET_SYS_CONFIRMED_3_X, RESET_SYS_CONFIRMED_3_Y, StrReset);
  arduboy.display();
  arduboy.delayShort(2000);
}

// Reset the user EEPROM area and display the confirmation message
void resetUserEEPROM() {
  arduboy.clear();
  arduboy.setTextSize(2);
  printStr_P(RESET_USER_WRITING_X, RESET_USER_WRITING_Y, StrWriting);
  arduboy.setTextSize(1);
  arduboy.display(CLEAR_BUFFER);
  for (unsigned int i = EEPROM_STORAGE_SPACE_START; i <= EEPROM_END; i++) {
    EEPROM.update(i, 0xFF);
  }
  printStrLargeRev_P(RESET_USER_CONFIRMED_1_X, RESET_USER_CONFIRMED_1_Y, StrUser);
  printStrLargeRev_P(RESET_USER_CONFIRMED_2_X, RESET_USER_CONFIRMED_2_Y, StrEEPROM);
  printStrLargeRev_P(RESET_USER_CONFIRMED_3_X, RESET_USER_CONFIRMED_3_Y, StrReset);
  arduboy.display();
  arduboy.delayShort(2000);
}

// --------------------- Printing Functions ------------------------------

// Print the name entry screen common information
void printNameScreenCommon() {
  printStr_P(NAME_TITLE_X, NAME_TITLE_Y, StrName);
  printNameHex(NAME_HEX_X, NAME_HEX_Y);
  printNameDecimal(NAME_DECIMAL_X, NAME_DECIMAL_Y);
}

// Print the name entry screen common information
void printIDScreenCommon() {
  printStr_P(ID_TITLE_X, ID_TITLE_Y, StrID);
  printIDDecimalBytes(ID_2_DECIMAL_X, ID_2_DECIMAL_Y);
  printIDDecimal(ID_DECIMAL_X, ID_DECIMAL_Y);
  printIDBinary(ID_BINARY_X, ID_BINARY_Y);
}

// Print the name screen cursors
void printNameCursors() {
  arduboy.fillRect(NAME_LARGE_X + (nameIndex * CHAR_WIDTH * 2),
                   NAME_LARGE_Y + (CHAR_HEIGHT * 2) + 2,
                   (CHAR_WIDTH * 2) - 2, 2);

  arduboy.drawFastHLine(NAME_HEX_X +
                         (nameIndex * (CHAR_WIDTH * 3 + SMALL_SPACE)),
                        NAME_HEX_Y + CHAR_HEIGHT + 1, CHAR_WIDTH * 3 - 1);

  arduboy.drawFastHLine(NAME_DECIMAL_X +
                         (nameIndex * (CHAR_WIDTH * 3 + SMALL_SPACE)),
                        NAME_DECIMAL_Y + CHAR_HEIGHT + 1, CHAR_WIDTH * 3 - 1);
}

// Print the ID screen cursors
void printIDCursors() {
  arduboy.fillRect(ID_LARGE_X + ((idIndex + 1) * (CHAR_WIDTH * 2)),
                   ID_LARGE_Y + (CHAR_HEIGHT * 2),
                   (CHAR_WIDTH * 2) - 2, 2);

  arduboy.drawFastHLine(ID_2_DECIMAL_X +
                         ((idIndex / 2) * (CHAR_WIDTH * 3 + SMALL_SPACE)),
                        ID_2_DECIMAL_Y + CHAR_HEIGHT + 1, CHAR_WIDTH * 3 - 1);

  arduboy.drawFastHLine(ID_DECIMAL_X, ID_DECIMAL_Y + CHAR_HEIGHT + 1,
                        CHAR_WIDTH * 5 - 1);

  arduboy.drawFastHLine((ID_BINARY_X + CHAR_WIDTH + SMALL_SPACE) +
                         (idIndex * (CHAR_WIDTH * 4 + SMALL_SPACE)),
                        ID_BINARY_Y + CHAR_HEIGHT + 1, CHAR_WIDTH * 4 - 1);
}

// Print the values and cursor for the flags
void printFlagSettings() {
  int cursorY;
  byte cursorLen = strlen_P(StrYes) * CHAR_WIDTH - 1 ;

  if (showLogoFlag) {
    printStr_P(FLAGS_SET_X, FLAGS_LOGO_Y, StrYes);
  }
  else {
    printStr_P(FLAGS_SET_X, FLAGS_LOGO_Y, StrNo);
  }

  if (showLEDsFlag) {
    printStr_P(FLAGS_SET_X, FLAGS_LEDS_Y, StrYes);
  }
  else {
    printStr_P(FLAGS_SET_X, FLAGS_LEDS_Y, StrNo);
  }

  if (showNameFlag) {
    printStr_P(FLAGS_SET_X, FLAGS_NAME_Y, StrYes);
  }
  else {
    printStr_P(FLAGS_SET_X, FLAGS_NAME_Y, StrNo);
  }

  switch (currentFlag) {
   case selFlagLEDs:
    cursorY = FLAGS_LEDS_Y;
    if (!showLEDsFlag) {
      cursorLen = strlen_P(StrNo) * CHAR_WIDTH - 1;
    }
    break;
   case selFlagName:
    cursorY = FLAGS_NAME_Y;
    if (!showNameFlag) {
      cursorLen = strlen_P(StrNo) * CHAR_WIDTH - 1;
    }
    break;
   default: // selFlagLogo
    cursorY = FLAGS_LOGO_Y;
    if (!showLogoFlag) {
      cursorLen = strlen_P(StrNo) * CHAR_WIDTH - 1;
    }
    break;
}

  arduboy.setCursor(FLAGS_CURSOR_X, cursorY);
  arduboy.print('\x10');

  arduboy.drawFastHLine(FLAGS_SET_X, cursorY + CHAR_HEIGHT, cursorLen);
}

// Print the unit name in normal size including an extent underline
// at the given location
void printName(int x, int y) {
  printStr(x, y, unitName);

  y += (CHAR_HEIGHT + 1);
  for (byte i = 0; i < ARDUBOY_UNIT_NAME_LEN; i++, x += CHAR_WIDTH) {
    arduboy.drawFastHLine(x, y, CHAR_WIDTH - 1);
  }
}

// Print the unit name in large size at the given location
void printNameLarge(int x, int y) {
  arduboy.setTextSize(2);
  printStr(x, y, unitName);
  arduboy.setTextSize(1);
}

// Add a line below the large name showing the current length
// Coordinates are for the name itself
void printNameUnderline(int x, int y) {
  int lWidth;

  if (unitName[0] != 0) {
    x -= 1;
    y += ((CHAR_HEIGHT * 2) + 6);
    lWidth = (strlen(unitName) * (CHAR_WIDTH * 2));
    arduboy.drawPixel(x, y);
    arduboy.drawPixel(x + lWidth - 1, y);
    arduboy.drawFastHLine(x, y + 1, lWidth);
  }
}

// Print the unit name in hex at the given location
void printNameHex(int x, int y) {
  for (byte i = 0; i < ARDUBOY_UNIT_NAME_LEN; i++) {
    printHex8(x, y, unitName[i]);
    x += CHAR_WIDTH * 3 + SMALL_SPACE;
  }
}

// Print the unit name in decimal at the given location
void printNameDecimal(int x, int y) {
  for (byte i = 0; i < ARDUBOY_UNIT_NAME_LEN; i++) {
    printDecimal8(x, y, unitName[i]);
    x += CHAR_WIDTH * 3 + SMALL_SPACE;
  }
}

// Print the unit ID in large size at the given location
void printIDLarge(int x, int y) {
  arduboy.setTextSize(2);
  printIDHex(x, y);
  arduboy.setTextSize(1);
}

// Print the unit ID in normal size at the given location
void printIDHex(int x, int y) {
  printHex16(x, y, unitID);
}

// Print the unit ID as 2 decimal bytes at the given location
void printIDDecimalBytes(int x, int y) {
  printDecimal8(x, y, unitID >> 8);
  printDecimal8(x + CHAR_WIDTH * 3 + SMALL_SPACE, y, unitID & 0x00FF);
}

// Print the unit ID in decimal at the given location
void printIDDecimal(int x, int y) {
  printDecimal16(x, y, unitID);
}

// print the unit ID in binary at the given location as 4 nybbles
// with a leading 'b'
void printIDBinary(int x, int y) {
  arduboy.setCursor(x, y);
  arduboy.print('b');
  x += CHAR_WIDTH + SMALL_SPACE;
  for (char i = 3 * 4; i >= 0; i -= 4) {
    printBinaryNybble(x, y, (byte)(unitID >> i));
    x += CHAR_WIDTH * 4 + SMALL_SPACE;
  }
}

// Print the save prompt in reverse at the given location
void printSavePrompt(int x, int y) {
  arduboy.fillRect(x - 2, y - 2,
                   strlen_P(StrSaveQ) * CHAR_WIDTH + 3, CHAR_HEIGHT + 3);
  arduboy.setTextColor(BLACK);
  arduboy.setTextBackground(WHITE);
  printStr_P(x, y, StrSaveQ);
  arduboy.setTextColor(WHITE);
  arduboy.setTextBackground(BLACK);
}

// Print a string at the given location
void printStr(int x, int y, char* str) {
  arduboy.setCursor(x, y);
  arduboy.print(str);
}

// Print a string in program memory at the given location
void printStr_P(int x, int y, const char* str) {
  arduboy.setCursor(x, y);
  arduboy.print((__FlashStringHelper*)(str));
}

// Print an 8 bit number in decimal, right justified with leading spaces
void printDecimal8(int x, int y, byte val) {
  printDecimalHelper(x, y, 2, 100, val);
}

// Print a 16 bit number in decimal, right justified with leading spaces
void printDecimal16(int x, int y, unsigned int val) {
  printDecimalHelper(x, y, 4, 10000, val);
}

// Print a right justified decimal number, given width-1 and (width-1)^10
void printDecimalHelper(int x, int y, byte width, unsigned int pwr10,
                        unsigned int val) {
  arduboy.setCursor(x, y);
  while (width > 0) {
    if (val >= pwr10) {
      break;
    }
    arduboy.print(' ');
    pwr10 /= 10;
    width--;
  }
  arduboy.print(val);
}

// Print an 8 bit hex number with leading x and zeros
void printHex8(int x, int y, byte val) {
  arduboy.setCursor(x, y);
  arduboy.print('x');
  if (val < 16) {
      arduboy.print('0');
  }
  arduboy.print(val, HEX);
}

// Print a 16 bit hex number with leading x and zeros
void printHex16(int x, int y, unsigned int val) {
  arduboy.setCursor(x, y);
  arduboy.print('x');
  for (char i = 3 * 4; i >= 0; i -= 4) {
    arduboy.print((val >> i) & 0x000F, HEX);
  }
}

// Print a nybble in binary from the lowest 4 bits of the provided byte
void printBinaryNybble(int x, int y, byte val) {
  arduboy.setCursor(x, y);

  for (char i = 3; i >= 0; i--) {
    arduboy.print((val >> i) & 0x01);
  }
}

// Print a constant string in large size and reversed
void printStrLargeRev_P(int x, int y, const char* string) {
  arduboy.fillRect(x - 4, y - 4,
                   strlen_P(string) * CHAR_WIDTH * 2 + 6, CHAR_HEIGHT * 2 + 6);
  arduboy.setTextColor(BLACK);
  arduboy.setTextBackground(WHITE);
  arduboy.setTextSize(2);
  printStr_P(x, y, string);
  arduboy.setTextSize(1);
  arduboy.setTextColor(WHITE);
  arduboy.setTextBackground(BLACK);
}

// ---------------- Control and Utility Functions ------------------------

// Get the current unit name and ID, and the system flags, from EEPROM
void readEEPROM() {
  memset(unitName, 0, sizeof(unitName));
  arduboy.readUnitName(unitName);
  unitID = arduboy.readUnitID();
  showLogoFlag = arduboy.readShowBootLogoFlag();
  showLEDsFlag = arduboy.readShowBootLogoLEDsFlag();
  showNameFlag = arduboy.readShowUnitNameFlag();
}

// Increment the name character at the cursor position
void nameCharInc() {
  while (invalidChar(++unitName[nameIndex])) { }
  drawScreen();
}

// Decrement the name character at the cursor position
void nameCharDec() {
  while (invalidChar(--unitName[nameIndex])) { }
  drawScreen();
}

// Return true if the given character is not allowed
boolean invalidChar(char c) {
  return (c == '\n') || (c == '\r') || ((byte)c == 0xFF);
}

// Move the name cursor right
void nameCursorRight() {
  if (++nameIndex == ARDUBOY_UNIT_NAME_LEN) {
    nameIndex = 0;
  }
  drawScreen();
}

// Move the name cursor left
void nameCursorLeft() {
  if (nameIndex == 0) {
    nameIndex = ARDUBOY_UNIT_NAME_LEN - 1;
  }
  else {
    nameIndex--;
  }
  drawScreen();
}

// Increment the ID digit at the cursor position
void idDigitInc() {
  uint16_t mask = 0xF000 >> (idIndex * 4);
  uint16_t val = 0x1000 >> (idIndex * 4);

  unitID = (unitID & ~mask) | ((unitID + val) & mask);
  drawScreen();
}

// Decrement the ID digit at the cursor position
void idDigitDec() {
  uint16_t mask = 0xF000 >> (idIndex * 4);
  uint16_t val = 0x1000 >> (idIndex * 4);

  unitID = (unitID & ~mask) | ((unitID - val) & mask);
  drawScreen();
}

// Move the ID cursor right
void idCursorRight() {
  if (++idIndex == sizeof(unitID) * 2) {
    idIndex = 0;
  }
  drawScreen();
}

// Move the ID cursor left
void idCursorLeft() {
  if (idIndex == 0) {
    idIndex = sizeof(unitID) * 2 - 1;
  }
  else {
    idIndex--;
  }
  drawScreen();
}

// Move the Flags cursor down
void flagsCursorDown() {
  switch (currentFlag) {
   case selFlagLogo:
    currentFlag = selFlagLEDs;
    break;
   case selFlagLEDs:
    currentFlag = selFlagName;
    break;
   case selFlagName:
    currentFlag = selFlagLogo;
    break;
  }
  drawScreen();
}

// Move the Flags cursor up
void flagsCursorUp() {
  switch (currentFlag) {
   case selFlagName:
    currentFlag = selFlagLEDs;
    break;
   case selFlagLEDs:
    currentFlag = selFlagLogo;
    break;
   case selFlagLogo:
    currentFlag = selFlagName;
    break;
  }
  drawScreen();
}

// Toggle the currently selected flag
void flagToggle() {
  switch (currentFlag) {
   case selFlagLogo:
    showLogoFlag = !showLogoFlag;
    break;
   case selFlagLEDs:
    showLEDsFlag = !showLEDsFlag;
    break;
   case selFlagName:
    showNameFlag = !showNameFlag;
    break;
  }
  drawScreen();
}

// Start the button auto-repeat delay
void startButtonDelay() {
  delayCount = DELAY_FRAMES;
  repeating = false;
}

// Stop the button auto-repeat or delay
void stopButtonRepeat() {
  delayCount = 0;
  repeating = false;
}

// Calculate the X coordinate to center a string of the given length
int centerStrLen(unsigned int len) {
  return (WIDTH / 2) - (len * CHAR_WIDTH / 2);
}

// Calculate the X coordinate to center a string located in program memory
int centerStr_P(const char* str) {
  return (WIDTH / 2) - (strlen_P(str) * CHAR_WIDTH / 2);
}

// Calculate the X coordinate to center a size 2 string located in
// program memory
int centerStr2_P(const char* str) {
  return (WIDTH / 2) - (strlen_P(str) * CHAR_WIDTH);
}

// Calculate the X coordinate to right justify a string in program memory
int rightStr_P(const char* str) {
  return WIDTH - (strlen_P(str) * CHAR_WIDTH) + 1;
}

