/**
 * @file Arduboy2Audio.cpp
 * \brief
 * The Arduboy2Audio class for speaker and sound control.
 */

#include "Arduboy2.h"
#include "Arduboy2Audio.h"

bool Arduboy2Audio::audio_enabled = false;

void Arduboy2Audio::on()
{
  // fire up audio pins
#ifdef ARDUBOY_10
  pinMode(PIN_SPEAKER_1, OUTPUT);
  pinMode(PIN_SPEAKER_2, OUTPUT);
#else
  pinMode(PIN_SPEAKER_1, OUTPUT);
#endif
  audio_enabled = true;
}

void Arduboy2Audio::off()
{
  audio_enabled = false;
  // shut off audio pins
#ifdef ARDUBOY_10
  pinMode(PIN_SPEAKER_1, INPUT);
  pinMode(PIN_SPEAKER_2, INPUT);
#else
  pinMode(PIN_SPEAKER_1, INPUT);
#endif
}

void Arduboy2Audio::saveOnOff()
{
  EEPROM.update(EEPROM_AUDIO_ON_OFF, audio_enabled);
}

void Arduboy2Audio::begin()
{
  if (EEPROM.read(EEPROM_AUDIO_ON_OFF))
    on();
}

bool Arduboy2Audio::enabled()
{
  return audio_enabled;
}
