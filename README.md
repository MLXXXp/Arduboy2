# Arduboy2 Library

The Arduboy2 library is maintained in a git repository hosted on [GitHub](https://github.com/) at:

https://github.com/MLXXXp/Arduboy2

The **Arduboy2** library is a fork of the [Arduboy library](https://github.com/Arduboy/Arduboy), which provides a standard *application programming interface* (API) to the display, buttons and other hardware of the Arduino based [Arduboy miniature game system](https://www.arduboy.com/). The original *Arduboy* library is no longer being maintained.

The name *Arduboy2* doesn't indicate that it's for a new "next generation" of the Arduboy hardware. The name was changed so it can coexist in the Arduino IDE with the current *Arduboy* library, without conflict. This way, existing sketches can continue to use the *Arduboy* library and class, without changes, while new sketches can be written (or old ones modified) to use and take advantage of the capabilities of the *Arduboy2* class and library.

For notes on the differences between the *Arduboy2* library and the original *Arduboy* library, and for information on migrating a sketch currently using the *Arduboy* library, see the sections at the end of this document.

## Library documentation

Comments in the library header files are formatted for the [Doxygen](https://www.doxygen.nl/) document generation system. The HTML files generated using the configuration file _extras/Doxyfile_ can be found at:

https://MLXXXp.github.io/documents/Arduino/libraries/Arduboy2/Doxygen/html/index.html

A generated PDF file can be found at:

https://MLXXXp.github.io/documents/Arduino/libraries/Arduboy2/Doxygen/pdf/Arduboy2.pdf

## Installation

The Arduboy2 library can be installed using the Arduino IDE Library Manager:

- In the Arduino IDE select from the menus: `Sketch > Include Library > Manage Libraries...`
- In the Library Manager *Filter your search...* field enter *arduboy2*.
- Click somewhere within the Arduboy2 entry.
- Click on the *Install* button.

For more library installation information see

[Installing Additional Arduino Libraries - Using the Library Manager](https://www.arduino.cc/en/Guide/Libraries#toc3)

## Start up features

The *begin()* function, used to initialize the library, includes features that are intended to be available to all sketches using the library (unless the sketch developer has chosen to disable one or more of them to free up some code space):

### The boot logo

At the start of the sketch, the **ARDUBOY** logo scrolls down from the top of the screen to the center.

The RGB LED lights red then green then blue while the logo is scrolling. (If your Arduboy is one of those that has the RGB LED installed incorrectly, then it will light blue then off then red). For users who do not wish to have the RGB LED flash during the boot logo sequence, a flag can be set in system EEPROM to have it remain off. The included *SetSystemEEPROM* example sketch can be used to set this flag.

A user settable *unit name* can be saved in system EEPROM memory. If set, this name will be briefly displayed at the bottom of the boot logo screen, after the logo stops scrolling down. This feature is only available if the *Arduboy2* class is used, not the *Arduboy2Base* class. This is because it requires the text display functions, which are only available in the *Arduboy2* class. A flag in system EEPROM controls whether or not the *unit name* is displayed on the boot logo screen, regardless of whether the *unit name* itself has been set. The included *SetSystemEEPROM* example sketch can be used to set both the *unit name* and this flag.

Once the logo display sequence completes, the sketch continues.

**Note:**

- For developers who wish to quickly begin testing, or impatient users who want to go strait to playing their game, the boot logo sequence can be bypassed by holding the *RIGHT* button while powering up, and then releasing it. Alternatively, the *RIGHT* button can be pressed while the logo is scrolling down.
- For users who wish to always disable the displaying of the boot logo sequence on boot up, a flag in system EEPROM is available for this. The included *SetSystemEEPROM* example sketch can be used to set this flag.

### "Flashlight" mode

If the *UP* button is pressed and held when the Arduboy is powered on, it enters *flashlight* mode. This turns the RGB LED fully on, and all the pixels of the screen are lit, resulting in a bright white light suitable as a small flashlight. (For an incorrect RGB LED, only the screen will light). To exit *flashlight* mode the Arduboy must be restarted.

*Flashlight* mode is also sometimes useful to allow uploading of new sketches, in case the sketch currently loaded uses a large amount of RAM which creates a bootloader problem.

### Audio mute control

Pressing and holding the *B* button when powering on will enter *System Control* mode. The RGB LED will light blue (red for an incorrect LED) to indicate that you are in *system control* mode. You must continue to hold the *B* button to remain in this mode. The only *system control* function currently implemented is *audio mute control*.

Pressing the *UP* button (while still holding *B*) will set a flag in system EEPROM indicating *audio enabled*. The RGB LED will flash green once (off for an incorrect LED) to indicate this action.

Pressing the *DOWN* button (while still holding *B*) will set the flag to *audio disabled* (muted). The RGB LED will flash red once (blue for an incorrect LED) to indicate this action.

Releasing the *B* button will exit *system control* mode and the sketch will continue.

Note that the audio control feature only sets a flag in EEPROM. Whatever code actually produces the sound must use the *audio.enabled()* function to check and honor the mute state. Audio libraries written with the Arduboy system in mind, such as the available *ArduboyPlaytune* and *ArduboyTones*, should do this. However, be aware that for some sketches, which don't use the Arduboy2 or other compliant library and generate sounds in their own way, this method of muting sound may not work.

## Using the library in a sketch

As with most libraries, to use Arduboy2 in your sketch you must include its header file at the start:

```cpp
#include <Arduboy2.h>
```

You must then create an Arduboy2 class object:

```cpp
Arduboy2 arduboy;
```

Naming the object *arduboy* has become somewhat of a standard, but you can use a different name if you wish.

To initialize the library, you must call its *begin()* function. This is usually done at the start of the sketch's *setup()* function:

```cpp
void setup()
{
  arduboy.begin();
  // more setup code follows, if required
}
```

The rest of the Arduboy2 functions will now be available for use.

If you wish to use the Sprites class functions you must create a Sprites object:

```cpp
Sprites sprites;
```

Sample sketches have been included with the library as examples of how to use it. To load an example, for examination and uploading to the Arduboy, using the Arduino IDE menus select:

`File > Examples > Arduboy2`

More information on writing sketches for the Arduboy can be found in the [Arduboy Community Forum](https://community.arduboy.com/).

### Using EEPROM in a sketch <- THIS IS IMPORTANT!

The Arduboy2 library reserves an area at the start of EEPROM for storing system information, such as the current audio mute state and the Unit Name and Unit ID. A sketch **MUST NOT** use this reserved area for its own purposes. A sketch may use any EEPROM past this reserved area. The first EEPROM address available for sketch use is given as the defined value *EEPROM_STORAGE_SPACE_START*

### Audio control functions

The library includes an Arduboy2Audio class. This class provides functions to enable and disable (mute) sound and also save the current mute state so that it remains in effect over power cycles and after loading a different sketch. It doesn't contain anything to actually produce sound.

The Arduboy2Base class, and thus the Arduboy2 class, creates an Arduboy2Audio class object named *audio*, so a sketch doesn't need to create its own Arduboy2Audio object.

Example:

```cpp
#include <Arduboy2.h>

Arduboy2 arduboy;

// Arduboy2Audio functions can be called as follows:
  arduboy.audio.on();
  arduboy.audio.off();
```

### Simple tone generation

The *BeepPin1* and *BeepPin2* classes are available to generate simple square wave tones using speaker pin 1 and speaker pin 2 respectively. These classes are documented in file *Arduboy2Beep.h*. Also, *BeepDemo* is included as one of the example sketches, which demonstrates basic use.

NOTE: These functions will not work with a DevKit Arduboy because the speaker pins used cannot be directly controlled by a timer/counter. "Dummy" functions are provided so a sketch will compile and work properly but no sound will be produced.

### Ways to make more code space available to sketches

#### Sound effects and music

If all you want is to play single tones, using the built in *BeepPin1* or *BeepPin2* classes will be very efficient.

If you want to be able to play sequences of tones or background music, using the [*ArduboyTones*](https://github.com/MLXXXp/ArduboyTones) library will be more code efficient than using [*ArduboyPlaytune*](https://github.com/Arduboy/ArduboyPlayTune) or most other sound libraries compatible with the Arduboy. *ArduboyTones* even produces less code than the [Arduino built in *tone()* function](https://www.arduino.cc/en/Reference/Tone). You'll have to decide on the appropriate library or functions you use to generate sound, based on the features required and how much memory you want it to use.

#### Remove the text functions

If your sketch doesn't use any of the functions for displaying text, such as *setCursor()* and *print()*, you can remove them. You could do this if your sketch generates whatever text it requires by some other means. Removing the text functions frees up code by not including the font table and some code that is always pulled in by inheriting the [Arduino *Print* class](https://playground.arduino.cc/Code/Printclass/).

To eliminate text capability in your sketch, when creating the library object simply use the *Arduboy2Base* class instead of *Arduboy2*:

For example, if the object will be named *arduboy*:

Replace

```cpp
Arduboy2 arduboy;
```

with

```cpp
Arduboy2Base arduboy;
```

#### Substitute or remove boot up features

As previously described in the _Start up features_ section, the *begin()* function includes features that are intended to be available to all sketches during boot up. However, if you're looking to gain some code space, you can call *boot()* instead of *begin()*. This will initialize the system but not include any of the extra boot up features. You can then add back in any of these features by calling the functions that perform them. You will have to trade off between the desirability of having a feature and how much memory you can recover by not including it.

You should at least call either *flashlight()* or *safeMode()* as a safeguard to allow uploading a new sketch when the bootloader "magic key" problem is an issue.

Here is a template that provides the equivalent of *begin()*

```cpp
void setup()
{
  // Required to initialize the hardware.
  arduboy.boot();

  // This clears the display. (The screen buffer will be all zeros)
  // It may not be needed if something clears the display later on but
  // "garbage" will be displayed if systemButtons() is used without it.
  arduboy.display();

  // flashlight() or safeMode() should always be included to provide
  // a method of recovering from the bootloader "magic key" problem.
  arduboy.flashlight();
//  arduboy.safeMode();

  // This allows sound to be turned on or muted. If the sketch provides
  // its own way of toggling sound, or doesn't produce any sound, this
  // function may not be required.
  arduboy.systemButtons();

  // This is required to initialize the speaker. It's not needed if
  // the sketch doesn't produce any sounds.
  arduboy.audio.begin();

  // This displays the boot logo sequence but note that the logo can
  // be suppressed by the user, by pressing the RIGHT button or using
  // a system EEPROM setting. If not removed entirely, an alternative
  // bootLogo...() function may save some memory.
  arduboy.bootLogo();
//  arduboy.bootLogoCompressed();
//  arduboy.bootLogoSpritesSelfMasked();
//  arduboy.bootLogoSpritesOverwrite();
//  arduboy.bootLogoSpritesBSelfMasked();
//  arduboy.bootLogoSpritesBOverwrite();
//  arduboy.bootLogoText();

  // Wait for all buttons to be released, in case a pressed one might
  // cause problems by being acted upon when the actual sketch code
  // starts. If neither systemButtons() nor bootLogo() is kept, this
  // function isn't required.
  arduboy.waitNoButtons();

// Additional setup code...

}
```

There are a few functions provided that are roughly equivalent to the standard functions used by *begin()* but which use less code space.

- *bootLogoCompressed()*, *bootLogoSpritesSelfMasked()*, *bootLogoSpritesOverwrite()*, *bootLogoSpritesBSelfMasked()* and *bootLogoSpritesBOverwrite()* will do the same as *bootLogo()* but will use *drawCompressed()*, or *Sprites* / *SpritesB* class *drawSelfMasked()* or *drawOverwrite()* functions respectively, instead of *drawBitmap()*, to render the logo. If the sketch uses one of these functions, then using the boot logo function that also uses it may reduce code size. It's best to try each of them to see which one produces the smallest size.
- *bootLogoText()* can be used in place *bootLogo()* in the case where the sketch uses text functions. It renders the logo as text instead of as a bitmap (so doesn't look as good).
- *safeMode()* can be used in place of *flashlight()* as a safeguard to allow uploading a new sketch when the bootloader "magic key" problem is an issue. It only lights the red RGB LED, so you don't get the bright light that is the primary purpose of *flashlight()*.

It is also possible to replace the boot logo drawing function with one that uses a different bitmap rendering function used elsewhere in your sketch. This may save memory by using this bitmap function for the logo, instead of the logo using a separate function that only ends up being used once. For example, if you use the *ArdBitmap* library's *drawCompressed()* function in your sketch, you could convert the **ARDUBOY** logo to *Ardbitmap* compressed format, and create *drawLogoArdCompressed()* and *bootLogoArdCompressed()* functions:

```cpp
void drawLogoArdCompressed(int16_t y)
{
  ardbitmap.drawCompressed(20, y, arduboy_logo_ardbitmap,
                           WHITE, ALIGN_CENTER, MIRROR_NONE);
}

void bootLogoArdCompressed()
{
  if (arduboy.bootLogoShell(drawLogoArdCompressed))
  {
    arduboy.bootLogoExtra();
  }
}

void setup()
{
  arduboy.beginDoFirst();
  bootLogoArdCompressed();
  arduboy.waitNoButtons();

// Additional setup code...

}
```

The **ARDUBOY** logo, in PNG format, is included in the library repository as file:

`extras/assets/arduboy_logo.png`

#### Use the SpritesB class instead of Sprites

The *SpritesB* class has functions identical to the *Sprites* class. The difference is that *SpritesB* is optimized for small code size rather than execution speed. If you want to use the sprites functions, and the slower speed of *SpritesB* doesn't affect your sketch, you may be able to use it to gain some code space.

Even if the speed is acceptable when using *SpritesB*, you should still try using *Sprites*. In some cases *Sprites* will produce less code than *SpritesB*, notably when only one of the functions is used.

You can easily switch between using *Sprites* or *SpritesB* by using one or the other to create an object instance:

```cpp
Sprites sprites;  // Use this to optimize for execution speed
SpritesB sprites; // Use this to (likely) optimize for code size
```

#### Eliminate the USB stack code

**Warning:** Although this will free up a fair amount of code and some RAM space, without an active USB interface uploader programs will be unable to automatically force a reset to invoke the bootloader. This means the user will have to manually initiate a reset in order to upload a new sketch. This can be an inconvenience or even frustrating for a user, due to the fact that timing the sequence can sometimes be tricky. Therefore, using this technique should be considered as a last resort. If it is used, the sketch documentation should state clearly what will be involved to upload a new sketch.

The *ARDUBOY_NO_USB* macro is used to eliminate the USB code. The *exitToBootloader()* function is available to make it easier for a user to invoke the bootloader. For more details, see the documentation provided for these.

----------

## What's different from Arduboy library V1.1

(These notes apply to when the *Arduboy2* library was first released. There will have been many additional changes, enhancements and features added to *Arduboy2* since then.)

A main goal of Arduboy2 is to provide ways in which more code space can be freed for use by large sketches. Another goal is to allow methods other than the *tunes* functions to be used to produce sounds. Arduboy2 remains substantially compatible with [Arduboy library V1.1](https://github.com/Arduboy/Arduboy/releases/tag/v1.1), which was the latest stable release at the time of the fork. Arduboy2 is based on the code targeted for Arduboy library V1.2, which was still in development and unreleased at the time it was forked.

Main differences between Arduboy2 and Arduboy V1.1 are:

- The *ArduboyTunes* subclass, which provided the *tunes.xxx()* functions, has been removed. It's functionality is available in a separate [*ArduboyPlaytune* library](https://github.com/Arduboy/ArduboyPlayTune). By removing these functions, more code space may become available because interrupt routines and other support code was being compiled in even if a sketch didn't make use them. Another benefit is that without the automatic installation of timer interrupt service routines, other audio generating functions and libraries, that need access to the same interrupts, can now be used. Removal of the *tunes* functions is the main API incompatibility with Arduboy V1.1. Sketches written to use *tunes* functions will need some minor modifications in order to make them work with Arduboy2 plus ArduboyPlaytune, [ArduboyTones](https://github.com/MLXXXp/ArduboyTones), or some other audio library.
- Arduboy library V1.1 uses timer 1 for the *tunes* functions. This causes problems when attempting to control the Arduboy's RGB LED using PWM, such as with *setRGBled()*, because it also requires timer 1. Since the *tunes* functionality has been removed from Arduboy2, there are no problems with using the RGB LED (except those caused by the RGB LED being incorrectly installed). Of course, using an external library that uses timer 1, such as *ArduboyPlaytune*, may reintroduce the problems. However, using a library that doesn't use timer 1, such as *ArduboyTones*, is now an option.
- The code to generate text output, using *setCursor()*, *print()*, etc., can be removed to free up code space, if a sketch doesn't use any text functions. The *Arduboy2* class includes the text functions but using the *Arduboy2Base* class instead will eliminate them. With text functions included, the font table and some support functions are always compiled in even if not used. The API for using text functions is the same as Arduboy V1.1 with some additional functions added:
 - *setTextColor()* and *setTextBackground()* allow for printing black text on a white background.
 - *getCursorX()* and *getCursorY()* allow for determining the current text cursor position.
 - The *clear()* function will now reset the text cursor to home position 0, 0.
- A new feature has been added which allows the *audio on/off* flag in system EEPROM to be configured by the user when the sketch starts. The flag is used by the Arduboy and Arduboy2 *audio* subclass, along with external sound functions and libraries, to provide a standardized sound mute capability. See the information above, under the heading *Audio mute control*, for more details.
- The *color* parameter, which is the last parameter for most of the drawing functions, has been made optional and will default to WHITE if not included in the call. This doesn't save any code but has been added as a convenience, since most drawing functions are called with WHITE specified.
- A new function *digitalWriteRGB()* has been added to control the RGB LED digitally instead of using PWM. This uses less code if just turning the RGB LEDs fully on or off is all that's required.
- The *beginNoLogo()* function is not included. This function could be used in Arduboy V1.1 in place of *begin()* to suppress the displaying of the ARDUBOY logo and thus free up the code that it required. Instead, Arduboy2 allows a sketch to call *boot()* and then add in any extra features that *begin()* provides by calling their functions directly after *boot()*, if desired.
- The *ArduboyCore* and *ArduboyAudio* base classes, previously only available to, and used to derive, the *Arduboy* class, have been made publicly available for the benefit of developers who may wish to use them as the base of an entirely new library. This change doesn't affect the existing API.

As of version 2.1.0 functionality from the Team A.R.G. *Arglib* library has been added:

- The sprite drawing functions, collision detection functions, and button handling functions that Team A.R.G. incorporated from the [ArduboyExtra](https://github.com/yyyc514/ArduboyExtra) project. The *poll()* function was renamed *pollButtons()* for clarity. The *Sprites* class doesn't require a parameter for the constructor, whereas in *Arglib* a pointer to an Arduboy class object is required.
- The *drawCompressed()* function, which allows compressed bitmaps to be drawn. Saving bitmaps in compressed form may reduce overall sketch size.

Team A.R.G. has now migrated all of their games and demos to use the Arduboy2 library.

## Migrating a sketch from Arduboy library V1.1 to Arduboy2

Since the Arduboy2 library can coexist in the Arduino IDE alongside the Arduboy library V1.1, a currently working sketch that uses Arduboy V1.1 doesn't have to be migrated to Arduboy2. However, if you want to switch a sketch to Arduboy2 for further development, in order to take advantage of any of the changes and enhancements, it's generally relatively easy.

The Arduboy2 library, for the most part, is compatible with Arduboy library V1.1 but migrating a sketch to Arduboy2 will require some small changes, and more so if it uses the *tunes* functions, such as *tunes.tone()* or *tunes.playScore()*.

### Required changes

The first thing to do is change the `include` for the library header file:

```cpp
#include <Arduboy.h>
```

becomes

```cpp
#include <Arduboy2.h>
```

If it was "Arduboy.h" (in quotes), it's still better to change it to &lt;Arduboy2.h&gt; (in angle brackets).

The same thing has to be done with creating the library object. (If the object name isn't *arduboy*, keep whatever name is used.):

```cpp
Arduboy arduboy;
```

becomes

```cpp
Arduboy2 arduboy;
```

If the sketch doesn't use any *tunes* functions, there's a good chance this is all that has to be done to make it compile.

### Sketch uses only tunes.tone() for sound

If the sketch has sound but only uses *tunes.tone()*, solutions are:

#### Solution 1: Switch to using Arduino tone()

An easy change is to use the Arduino built in *tone()* function. You can add a function to the sketch that wraps *tone()* so that it works like *tunes.tone()*, like so:

```cpp
// Wrap the Arduino tone() function so that the pin doesn't have to be
// specified each time. Also, don't play if audio is set to off.
void playTone(unsigned int frequency, unsigned long duration)
{
  if (arduboy.audio.enabled() == true)
  {
    tone(PIN_SPEAKER_1, frequency, duration);
  }
}
```

You then change all *tunes.tone()* calls to *playTone()* calls using the same parameter values. For example:

```cpp
  arduboy.tunes.tone(1000, 250);
```

becomes

```cpp
  playTone(1000, 250);
```

#### Solution 2: Switch to using the ArduboyTones library

Changing to the *ArduboyTones* library is slightly more complicated. The advantage is that it will generate less code than using *tone()* and will also allow you to easily enhance the sketch to play tone sequences instead of just single tones. ArduboyTones can also play each tone at either normal or a higher volume.

You have to add an include for the ArduboyTones header file:

```cpp
#include <ArduboyTones.h>
```

You then have to create an object for the *ArduboyTones* class and pass it a pointer to the Arduboy2 *audio.enabled()* function. This must go after the creation of the Arduboy2 object, like so:

```cpp
Arduboy2 arduboy;
ArduboyTones sound(arduboy.audio.enabled);
```

You then change all Arduboy *tunes.tone()* calls to ArduboyTones *tone()* calls using the same parameter values. For example:

```cpp
  arduboy.tunes.tone(1000, 250);
```

becomes

```cpp
  sound.tone(1000, 250);
```

See the [ArduboyTones](https://github.com/MLXXXp/ArduboyTones) README file for more information on installing and using it.

#### Solution 3: Switch to using the ArduboyPlaytune library.

See the following for how to do this:

### Sketch uses tunes.playScore()

If the sketch uses *tunes.playScore()*, probably the easiest solution is to use the *ArduboyPlaytune* library. *ArduboyPlaytune* is essentially the code that was in the Arduboy V1.1 *tunes* subclass, which has been removed from Arduboy2. It's been cleaned up and a few enhancements have been added, but all the Arduboy V1.1 *tunes* functions are available.

You have to add an include for the ArduboyPlaytune header file:

```cpp
#include <ArduboyPlaytune.h>
```

You then have to create an object for the *ArduboyPlaytune* class and pass it a pointer to the Arduboy2 *audio.enabled()* function. This must go after the creation of the Arduboy2 object, like so:

```cpp
Arduboy2 arduboy;
ArduboyPlaytune tunes(arduboy.audio.enabled);
```

The sound channels must then be initialzed and assigned to the speaker pins. This code would go in the *setup()* function:

```cpp
  // audio setup
  tunes.initChannel(PIN_SPEAKER_1);
#ifndef AB_DEVKIT
  // if not a DevKit
  tunes.initChannel(PIN_SPEAKER_2);
#else
  // if it's a DevKit
  tunes.initChannel(PIN_SPEAKER_1); // use the same pin for both channels
  tunes.toneMutesScore(true);       // mute the score when a tone is sounding
#endif
```

If you name the ArduboyPlaytune object *tunes* as shown above, then you just have to remove the Arduboy object name from any *tunes* calls. For example:

```cpp
  arduboy.tunes.playScore(mySong);
```

becomes

```cpp
  tunes.playScore(mySong);
```

See the [*ArduboyPlaytune* library](https://github.com/Arduboy/ArduboyPlayTune) documentation for more information.

If you don't need to play scores containing two parts, and don't require tones to be played in parallel with a score that's playing, then as an alternative to using *ArduboyPlaytune* you may wish to consider switching to *ArduboyTones*. This may require a bit of work because any *ArduboyPlaytune* scores would have to be converted to *ArduboyTones* format. It would involve changing note numbers to frequencies. This could be simplified by using the provided *NOTE_* defines. Also, durations would have to be converted, including adding silent "rest" tones as necessary.

The benefit of using *ArduboyTones* would be reduced code size and possibly easier addition of new sequences without the need of a MIDI to Playtune format converter.

### Sketch uses the beginNoLogo() function instead of begin()

The *beginNoLogo()* function has been removed. *beginNoLogo()* can be replaced with *begin()*, since users can choose to suppress the logo sequence using the *RIGHT* button or by setting a flag in system EEPROM.

If using *begin()* results in the sketch program memory size being too large, *beginDoFirst()* or *boot()* can be used with additional functions following it to add back in desired boot functionality. See the information above, under the heading *Substitute or remove boot up features*, for more details. Assuming the object is named *arduboy*, an equivalent replacement for *beginNoLogo()* would be:

```cpp
  arduboy.beginDoFirst();
  arduboy.waitNoButtons();
```

----------

