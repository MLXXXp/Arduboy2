/*
BeepDemo.ino

This sketch provides an example of using the Arduboy2 library's BeepPin1 class
to play simple tones.
*/

/*
Written in 2018 by Scott Allen saydisp-git@yahoo.ca

To the extent possible under law, the author(s) have dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with
this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

// Comments are only provided for code dealing with tone generation or control.

#include <Arduboy2.h>
// There's no need to #include <Arduboy2Beep.h>
// It will be included in Arduboy2.h

Arduboy2 arduboy;

BeepPin1 beep; // Create a class instance for speaker pin 1
//BeepPin2 beep; // For speaker pin 2, use this line instead of the line above

int objectX = 0;

char displayText[60];

void setup() {
  arduboy.begin();
  arduboy.setFrameRate(25);

  beep.begin(); // Set up the hardware for playing tones

  commandText("none - Press buttons");
}

void loop() {
  if (!arduboy.nextFrame()) {
    return;
  }

  // The timer() function is called once per frame, so duration values will be
  // the number of frames that the tone plays for.
  // At 25 frames per second each frame will be 40ms.
  beep.timer(); // handle tone duration

  arduboy.pollButtons();

  arduboy.clear();

  if (arduboy.justPressed(LEFT_BUTTON)) {
    // Play a 523.251Hz tone (piano note C5) for 5 frames (200ms at 25 FPS)
    // beep.freq(523.251) is used to convert 523.251Hz to the required count
    beep.tone(beep.freq(523.251), 5);

    commandText("beep.tone(\n beep.freq(523.251),\n 5)");
  }

  if (arduboy.justPressed(UP_BUTTON)) {
    // Play a 587.330Hz tone (piano note D5) for 15 frames (600ms at 25 FPS)
    beep.tone(beep.freq(587.330), 15);

    commandText("beep.tone(\n beep.freq(587.330),\n 15)");
  }

  if (arduboy.justPressed(RIGHT_BUTTON)) {
    // Play a 659.255Hz tone (piano note E5) for 50 frames (2s at 25 FPS)
    beep.tone(beep.freq(659.255), 50);

    commandText("beep.tone(\n beep.freq(659.255),\n 50)");
  }

  if (arduboy.justPressed(DOWN_BUTTON)) {
    // Play a 698.456Hz tone (piano note F5) until stopped
    // or replaced by another tone
    beep.tone(beep.freq(698.456));

    commandText("beep.tone(\n beep.freq(698.456))");
  }

  if (arduboy.justPressed(A_BUTTON)) {
    // For short tones with a duration less than a frame time,
    // or when timer() isn't being used, such as in a menu,
    // a continuous tone can be played and then stopped after a delay
    // but note that no other work will be done during the delay.
    beep.tone(beep.freq(1000)); // Play a 1000Hz tone until stopped
    arduboy.delayShort(30); // Delay for 30ms
    beep.noTone(); // Stop the tone

    commandText("beep.tone(\n beep.freq(1000))\n(delay 30ms)\nbeep.noTone()");
  }

  if (arduboy.justPressed(B_BUTTON)) {
    beep.noTone(); // Stop the tone if one is playing

    commandText("beep.noTone()");
  }

  arduboy.println(F("Last command:"));
  arduboy.print(displayText);

  // The Arduboy2 class's audio subclass controls sound muting.
  // For this sketch, mute or unmute can be set using the "System Control"
  // start up feature. (Hold B while powering up then, while still holding B,
  // press UP to enable sound or DOWN for mute.)
  if (!arduboy.audio.enabled()) {
    arduboy.setCursor(22, 40);
    arduboy.print(F("Sound is MUTED"));
  }

  arduboy.setCursor(0, 48);
  // The "duration" variable can be tested for non-zero to determine if a
  // timed tone is currently playing.
  if (beep.duration != 0) {
    arduboy.print(F("A tone is playing"));
  }
  else {
    arduboy.print(F("Continuous tone or\nno tone playing"));
  }

  arduboy.drawRect(objectX, 40, 6, 6);
  if (++objectX == WIDTH - 6) {
    objectX = 0;
  }

  arduboy.display();
}

void commandText(const char* text) {
  strncpy(displayText, text, sizeof displayText);
  displayText[sizeof displayText - 1] = '\0';
}

