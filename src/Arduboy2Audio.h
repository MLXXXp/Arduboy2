#ifndef ARDUBOY2_AUDIO_H
#define ARDUBOY2_AUDIO_H

#include <Arduino.h>
#include <EEPROM.h>

class Arduboy2Audio
{
 public:
  void static begin();
  void static on();
  void static off();
  void static saveOnOff();
  bool static enabled();

 protected:
  bool static audio_enabled;
};

#endif
