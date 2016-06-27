#ifndef ArduboyAudio_h
#define ArduboyAudio_h

#include <Arduino.h>
#include <EEPROM.h>

class ArduboyAudio
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
