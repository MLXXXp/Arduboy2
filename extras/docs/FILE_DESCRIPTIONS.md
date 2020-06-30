# File Descriptions

Documentation for files contained in this repository that aren't self explanatory.

### /library.properties

Provides information so that this library can be installed and updated in the Arduino IDE using the [Library Manager](https://www.arduino.cc/en/Guide/Libraries#toc3).

The value of *version* must be set to the latest stable tagged release. This should be changed and commited just before tagging the new release.

See the [Arduino IDE 1.5: Library specification](https://arduino.github.io/arduino-cli/library-specification/) for details.

### /library.json

This JSON file is a manifest used by the [PlatformIO IDE](https://platformio.org/) to make this library available in its [Library Manager](https://docs.platformio.org/en/latest/librarymanager/index.html).

The value of *version* must be set to the latest stable tagged release. This should be changed and commited just before tagging the new release.

See the [PlatformIO library.json](https://docs.platformio.org/en/latest/librarymanager/config.html) documentation for details.

### /extras/assets/arduboy_logo.png<br>/extras/assets/arduboy_screen.png

Templates used to create the ARDUBOY logo used in the *bootLogo()* function.

----------

