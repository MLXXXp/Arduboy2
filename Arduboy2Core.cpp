/**
 * @file Arduboy2Core.cpp
 * \brief
 * The Arduboy2Core class for Arduboy hardware initilization and control.
 */

#include "Arduboy2Core.h"
#include <avr/wdt.h>

const uint8_t PROGMEM lcdBootProgram[] = {
  // boot defaults are commented out but left here in case they
  // might prove useful for reference
  // Further reading: https://www.adafruit.com/datasheets/SSD1306.pdf

#ifdef OLED_SH1106
  0x8D, 0x14,                   // Charge Pump Setting v = enable (0x14)
  0xA1,                         // Set Segment Re-map
  0xC8,                         // Set COM Output Scan Direction
  0x81, 0xCF,                   // Set Contrast v = 0xCF
  0xD9, 0xF1,                   // Set Precharge = 0xF1
  OLED_SET_COLUMN_ADDRESS_LO,   //Set column address for left most pixel 
  0xAF                          // Display On
#elif defined(OLED_96X96)
  0x15, 0x10, 0x3f, //Set column start and end address
  0x75, 0x00, 0x5f, //Set row start and end address
  0xA0, 0x55,       //set re-map: split odd-even COM signals|COM remap|vertical address increment|column address remap
  0xA1, 0x00,       //set display start line
  0xA2, 0x60,       //set display offset
  //0xA4,           //Normal display
  0xA8, 0x5F,       //Set MUX ratio 96MUX
  //0xB2, 0x23,
  //0xB3, 0xF0,     //set devider clock | oscillator frequency
  //0x81, 0x0F,     //Set contrast
  //0xBC, 0x1F,     //set precharge voltage
  //0x82, 0xFE,     //set second Precharge speed
  0xB1, 0x21,       //reset and 1st precharge phase length  phase 2:2 DCLKs, Phase 1: 1 DCLKs
  //0xBB, 0x0F,     //set 2nd precharge period: 15 DCLKs
  //0xbe, 0x1F,     //output level high voltage com signal
  //0xB8, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E, 0x20, //set gray scale table
  0xAF              //Display on
#else
  // for SSD1306 and compatible displays
  //
  // Display Off
  // 0xAE,

  // Set Display Clock Divisor v = 0xF0
  // default is 0x80
  0xD5, 0xF0,

  // Set Multiplex Ratio v = 0x3F
  // 0xA8, 0x3F,

  // Set Display Offset v = 0
  // 0xD3, 0x00,

  // Set Start Line (0)
  // 0x40,

  // Charge Pump Setting v = enable (0x14)
  // default is disabled
  0x8D, 0x14,

  // Set Segment Re-map (A0) | (b0001)
  // default is (b0000)
  0xA1,

  // Set COM Output Scan Direction
  0xC8,

  // Set COM Pins v
  // 0xDA, 0x12,

  // Set Contrast v = 0xCF
  0x81, 0xCF,

  // Set Precharge = 0xF1
  0xD9, 0xF1,

  // Set VCom Detect
  // 0xDB, 0x40,

  // Entire Display ON
  // 0xA4,

  // Set normal/inverse display
  // 0xA6,

  // Display On
  0xAF,

  // set display mode = horizontal addressing mode (0x00)
  0x20, 0x00,

  // set col address range
  // 0x21, 0x00, COLUMN_ADDRESS_END,

  // set page address range
  // 0x22, 0x00, PAGE_ADDRESS_END
#endif

#if defined OLED_SSD1309 //required additionally for SSD1309
  0x21, 0x00, COLUMN_ADDRESS_END
#endif
};

Arduboy2Core::Arduboy2Core() { }

void Arduboy2Core::boot()
{
  #ifdef ARDUBOY_SET_CPU_8MHZ
  // ARDUBOY_SET_CPU_8MHZ will be set by the IDE using boards.txt
  setCPUSpeed8MHz();
  #endif

  // Select the ADC input here so a delay isn't required in initRandomSeed()
  ADMUX = RAND_SEED_IN_ADMUX;

  bootPins();
  bootSPI();
  bootOLED();
  bootPowerSaving();
}

#ifdef ARDUBOY_SET_CPU_8MHZ
// If we're compiling for 8MHz we need to slow the CPU down because the
// hardware clock on the Arduboy is 16MHz.
// We also need to readjust the PLL prescaler because the Arduino USB code
// likely will have incorrectly set it for an 8MHz hardware clock.
void Arduboy2Core::setCPUSpeed8MHz()
{
  uint8_t oldSREG = SREG;
  cli();                // suspend interrupts
  PLLCSR = _BV(PINDIV); // dissable the PLL and set prescale for 16MHz)
  CLKPR = _BV(CLKPCE);  // allow reprogramming clock
  CLKPR = 1;            // set clock divisor to 2 (0b0001)
  PLLCSR = _BV(PLLE) | _BV(PINDIV); // enable the PLL (with 16MHz prescale)
  SREG = oldSREG;       // restore interrupts
}
#endif

// Pins are set to the proper modes and levels for the specific hardware.
// This routine must be modified if any pins are moved to a different port
void Arduboy2Core::bootPins()
{
#ifdef ARDUBOY_10

  // Port B INPUT_PULLUP or HIGH
  PORTB |= _BV(RED_LED_BIT) |
  #ifndef ARDUINO_AVR_PROMICRO
           _BV(GREEN_LED_BIT) |
  #endif
           _BV(BLUE_LED_BIT) | _BV(B_BUTTON_BIT);
  // Port B INPUT or LOW (none)
  // Port B inputs
  DDRB &= ~(_BV(B_BUTTON_BIT));
  // Port B outputs
  DDRB |= _BV(RED_LED_BIT) | 
  #ifndef ARDUINO_AVR_PROMICRO
          _BV(GREEN_LED_BIT) | 
  #endif
          _BV(BLUE_LED_BIT) | _BV(SPI_MOSI_BIT) | _BV(SPI_SCK_BIT);

  // Port C
  // Speaker: Not set here. Controlled by audio class

  // Port D INPUT_PULLUP or HIGH
  #ifdef ARDUINO_AVR_PROMICRO
    PORTD |= _BV(CS_BIT) | _BV(GREEN_LED_BIT);
  #else
    PORTD |= _BV(CS_BIT);
  #endif
  // Port D INPUT or LOW
  PORTD &= ~(_BV(RST_BIT));
  // Port D inputs (none)
  // Port D outputs
  DDRD |= _BV(RST_BIT) | _BV(CS_BIT) |
  #ifdef ARDUINO_AVR_PROMICRO
    _BV(GREEN_LED_BIT) |
  #endif
  _BV(DC_BIT);

  // Port E INPUT_PULLUP or HIGH
  PORTE |= _BV(A_BUTTON_BIT);
  // Port E INPUT or LOW (none)
  // Port E inputs
  DDRE &= ~(_BV(A_BUTTON_BIT));
  // Port E outputs (none)

  // Port F INPUT_PULLUP or HIGH
  PORTF |= _BV(LEFT_BUTTON_BIT) | _BV(RIGHT_BUTTON_BIT) |
           _BV(UP_BUTTON_BIT) | _BV(DOWN_BUTTON_BIT);
  // Port F INPUT or LOW
  PORTF &= ~(_BV(RAND_SEED_IN_BIT));
  // Port F inputs
  DDRF &= ~(_BV(LEFT_BUTTON_BIT) | _BV(RIGHT_BUTTON_BIT) |
            _BV(UP_BUTTON_BIT) | _BV(DOWN_BUTTON_BIT) |
            _BV(RAND_SEED_IN_BIT));
  // Port F outputs (none)

#elif defined(AB_DEVKIT)

  // Port B INPUT_PULLUP or HIGH
  PORTB |= _BV(LEFT_BUTTON_BIT) | _BV(UP_BUTTON_BIT) | _BV(DOWN_BUTTON_BIT) |
           _BV(BLUE_LED_BIT);
  // Port B INPUT or LOW (none)
  // Port B inputs
  DDRB &= ~(_BV(LEFT_BUTTON_BIT) | _BV(UP_BUTTON_BIT) | _BV(DOWN_BUTTON_BIT));
  // Port B outputs
  DDRB |= _BV(BLUE_LED_BIT) | _BV(SPI_MOSI_BIT) | _BV(SPI_SCK_BIT);

  // Port C INPUT_PULLUP or HIGH
  PORTC |= _BV(RIGHT_BUTTON_BIT);
  // Port C INPUT or LOW (none)
  // Port C inputs
  DDRC &= ~(_BV(RIGHT_BUTTON_BIT));
  // Port C outputs (none)

  // Port D INPUT_PULLUP or HIGH
  PORTD |= _BV(CS_BIT);
  // Port D INPUT or LOW
  PORTD &= ~(_BV(RST_BIT));
  // Port D inputs (none)
  // Port D outputs
  DDRD |= _BV(RST_BIT) | _BV(CS_BIT) | _BV(DC_BIT);

  // Port E (none)

  // Port F INPUT_PULLUP or HIGH
  PORTF |= _BV(A_BUTTON_BIT) | _BV(B_BUTTON_BIT);
  // Port F INPUT or LOW
  PORTF &= ~(_BV(RAND_SEED_IN_BIT));
  // Port F inputs
  DDRF &= ~(_BV(A_BUTTON_BIT) | _BV(B_BUTTON_BIT) | _BV(RAND_SEED_IN_BIT));
  // Port F outputs (none)
  // Speaker: Not set here. Controlled by audio class

#endif
}

void Arduboy2Core::bootOLED()
{
  // reset the display
  delayShort(5); // reset pin should be low here. let it stay low a while
  bitSet(RST_PORT, RST_BIT); // set high to come out of reset
  delayShort(5); // wait a while

  // select the display (permanently, since nothing else is using SPI)
  bitClear(CS_PORT, CS_BIT);

  // run our customized boot-up command sequence against the
  // OLED to initialize it properly for Arduboy
  LCDCommandMode();
  for (uint8_t i = 0; i < sizeof(lcdBootProgram); i++) {
    SPItransfer(pgm_read_byte(lcdBootProgram + i));
  }
  LCDDataMode();
}

void Arduboy2Core::LCDDataMode()
{
  bitSet(DC_PORT, DC_BIT);
}

void Arduboy2Core::LCDCommandMode()
{
  bitClear(DC_PORT, DC_BIT);
}

// Initialize the SPI interface for the display
void Arduboy2Core::bootSPI()
{
// master, mode 0, MSB first, CPU clock / 2 (8MHz)
  SPCR = _BV(SPE) | _BV(MSTR);
  SPSR = _BV(SPI2X);
}

// Write to the SPI bus (MOSI pin)
void Arduboy2Core::SPItransfer(uint8_t data)
{
  SPDR = data;
  /*
   * The following NOP introduces a small delay that can prevent the wait
   * loop form iterating when running at the maximum speed. This gives
   * about 10% more speed, even if it seems counter-intuitive. At lower
   * speeds it is unnoticed.
   */
  asm volatile("nop");
  while (!(SPSR & _BV(SPIF))) { } // wait
}

void Arduboy2Core::safeMode()
{
  if (buttonsState() == UP_BUTTON)
  {
    digitalWriteRGB(RED_LED, RGB_ON);

    // prevent the bootloader magic number from being overwritten by timer 0
    // when a timer variable overlaps the magic number location
    power_timer0_disable();

    while (true) { }
  }
}


/* Power Management */

void Arduboy2Core::idle()
{
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_mode();
}

void Arduboy2Core::bootPowerSaving()
{
  // disable Two Wire Interface (I2C) and the ADC
  PRR0 = _BV(PRTWI) | _BV(PRADC);
  // disable USART1
  PRR1 = _BV(PRUSART1);
  // All other bits will be written with 0 so will be enabled
}

// Shut down the display
void Arduboy2Core::displayOff()
{
  LCDCommandMode();
  SPItransfer(0xAE); // display off
  SPItransfer(0x8D); // charge pump:
  SPItransfer(0x10); //   disable
  delayShort(250);
  bitClear(RST_PORT, RST_BIT); // set display reset pin low (reset state)
}

// Restart the display after a displayOff()
void Arduboy2Core::displayOn()
{
  bootOLED();
}

uint8_t Arduboy2Core::width() { return WIDTH; }

uint8_t Arduboy2Core::height() { return HEIGHT; }


/* Drawing */

void Arduboy2Core::paint8Pixels(uint8_t pixels)
{
  SPItransfer(pixels);
}

void Arduboy2Core::paintScreen(const uint8_t *image)
{
#ifdef OLED_SH1106 
  for (uint8_t i = 0; i < HEIGHT / 8; i++)
  {
    LCDCommandMode();
    SPDR = (OLED_SET_PAGE_ADDRESS + i);
    while (!(SPSR & _BV(SPIF)));
    SPDR = (OLED_SET_COLUMN_ADDRESS_HI); // only reset hi nibble to zero
    while (!(SPSR & _BV(SPIF)));
    LCDDataMode();
    for (uint8_t j = WIDTH; j > 0; j--)
      {
        SPDR = pgm_read_byte(image++);
        while (!(SPSR & _BV(SPIF)));
      }
  }
#elif defined(OLED_96X96)
  uint16_t i = 0;
  for (uint8_t col = 0; col < WIDTH / 2; col++)
  {
    for (uint8_t row = 0; row < HEIGHT / 8; row++)
    {
      uint8_t b1 = pgm_read_byte(image + i);
      uint8_t b2 = pgm_read_byte(image + i + 1);
      for (uint8_t shift = 0; shift < 8; shift++)
      {
        uint8_t c = 0xFF;
        if ((b1 & 1) == 0) c &= 0x0F;
        if ((b2 & 1) == 0) c &= 0xF0;
        SPDR = c;
        b1 = b1 >> 1;
        b2 = b2 >> 1;
        while (!(SPSR & _BV(SPIF)));
      }
      i += WIDTH;
    }
    i -= HEIGHT / 8 * WIDTH - 2;
  }

#else 
  //OLED SSD1306 and compatibles
  for (int i = 0; i < (HEIGHT*WIDTH)/8; i++)
  {
    SPItransfer(pgm_read_byte(image + i));
  }
#endif
}

// paint from a memory buffer, this should be FAST as it's likely what
// will be used by any buffer based subclass
void Arduboy2Core::paintScreen(uint8_t image[], bool clear)
{
#ifdef OLED_SH1106 
  //Assembly optimized page mode display code with clear support.
  //Each byte transfer takes 18 cycles
  asm volatile (
    "     ldi  r25, %[page_cmd]                     \n\t"
    ".l1:                                           \n\t"
    "     ldi  r24, %[width]          ;1            \n\t"         
    "     ldi  r20,5                  ;1            \n\t"         
    "     cbi  %[dc_port], %[dc_bit]  ;2 cmd mode   \n\t"         
    "     out  %[spdr], r25           ;1            \n\t"         
    ".l2: subi r20,1                  ;r20*3-1 : 14 \n\t"         
    "     brne .l2                                  \n\t"         
    "     rjmp .+0                    ;2            \n\t"
    "     ldi  r20,%[col_cmd]         ;1            \n\t"
    "     out  %[spdr], r20           ;1            \n\t"
    ".l3: rjmp .l7                    ;2            \n\t"
    ".l4: ld   r20, Z                 ;2            \n\t"
    "     cp   %[clear], __zero_reg__ ;1            \n\t" 
    "     brne .l5                    ;1/2          \n\t"
    "     nop                         ;1            \n\t"
    "     rjmp .l6                    ;2            \n\t"   
    ".l5: st   Z, __zero_reg__        ;2 : 7        \n\t"
    ".l6: sbi  %[dc_port], %[dc_bit]  ;2 data mode  \n\t"
    "     out  %[spdr], r20           ;1            \n\t" 
    "     adiw r30, 1                 ;2            \n\t"
    ".l7: rjmp .+0                    ;2            \n\t"
    "     nop                         ;1            \n\t"
    "     subi r24, 1                 ;1            \n\t"
    "     brne .l4                    ;1/2 : 5/6    \n\t"
    "     rjmp .+0                    ;2            \n\t"
    "     subi r25, -1                ;1            \n\t"
    "     cpi  r25,%[page_end]        ;1            \n\t"
    "     brne .l1                    ;1/2 : 5/6    \n\t"
    :
    : [ptr]      "z" (image),
      [page_cmd] "M" (OLED_SET_PAGE_ADDRESS),
      [page_end] "M" (OLED_SET_PAGE_ADDRESS + (HEIGHT / 8)),
      [dc_port]  "I" (_SFR_IO_ADDR(DC_PORT)),
      [dc_bit]   "I" (DC_BIT),
      [spdr]     "I" (_SFR_IO_ADDR(SPDR)),
      [col_cmd]  "M" (OLED_SET_COLUMN_ADDRESS_HI),
      [width]    "M" (WIDTH + 1),
      [clear]    "a" (clear)
    : "r20", "r24", "r25"
  );
#elif defined(OLED_96X96)
  // 1 bit to 4-bit display code with clear support.
  // Each transfer takes 18 cycles with additional 4 cycles for a column change.
  asm volatile(
    "  ldi     r25, %[col]                      \n\t"          
    ".lcolumn:                                  \n\t"         
    "   ldi     r24, %[row]         ;1          \n\t"
    ".lrow:                                     \n\t"
    "   ldi     r21, 7              ;1          \n\t"
    "   ld      r22, z              ;2          \n\t"
    "   ldd     r23, z+1            ;2          \n\t"
    ".lshiftstart:                              \n\t"
    "   ldi     r20, 0xFF           ;1          \n\t"
    "   sbrs    r22, 0              ;1          \n\t"
    "   andi    r20, 0x0f           ;1          \n\t"
    "   sbrs    r23, 0              ;1          \n\t"
    "   andi    r20,0xf0            ;1          \n\t"
    "   out     %[spdr], r20        ;1          \n\t"
    "                                           \n\t"
    "   cp   %[clear], __zero_reg__ ;1          \n\t"
    "   brne .lclear1               ;1/2        \n\t"
    ".lshiftothers:                             \n\t"
    "   movw    r18, %A[ptr]        ;1          \n\t"
    "   rjmp .+0                    ;2          \n\t"
    "   rjmp .lshiftnext            ;2          \n\t"
    ".lclear1:                                  \n\t"
    "   st      z, __zero_reg__     ;2          \n\t" 
    "   std     z+1, __zero_reg__   ;2          \n\t"
    ".lshiftnext:                               \n\t"
    "                                           \n\t"
    "   lsr     r22                 ;1          \n\t"
    "   lsr     r23                 ;1          \n\t"
    "                                           \n\t"
    "   ldi     r20, 0xFF           ;1          \n\t"
    "   sbrs    r22, 0              ;1/2        \n\t"
    "   andi    r20, 0x0f           ;1          \n\t"
    "   sbrs    r23, 0              ;1/2        \n\t"
    "   andi    r20,0xf0            ;1          \n\t"
    "                                           \n\t"
    "   subi    r18, %[top_lsb]     ;1          \n\t" //image - (HEIGHT / 8) * ((WIDTH / 8) - 1) + 2
    "   sbci    r19, %[top_msb]     ;1          \n\t"
    "   subi    r21, 1              ;1          \n\t"
    "   out     %[spdr], r20        ;1          \n\t"
    "   brne    .lshiftothers       ;1/2        \n\t"
    "                                           \n\t"
    "   nop                         ;1          \n\t"
    "   subi    %A[ptr], %[width]   ;1          \n\t" //image + width (negated addition)
    "   sbci    %B[ptr], -1         ;1          \n\t"
    "   subi    r24, 1              ;1          \n\t"
    "   brne    .lrow               ;1/2        \n\t"
    "                                           \n\t"
    "   movw    %A[ptr], r18        ;1          \n\t"
    "   subi    r25, 1              ;1          \n\t"
    "   brne    .lcolumn            ;1/2        \n\t"
    :
    : [ptr]     "z" (image),
      [spdr]    "I" (_SFR_IO_ADDR(SPDR)),
      [row]     "M" (HEIGHT / 8),
      [col]     "M" (WIDTH / 2),
      [width]   "M" (256 - WIDTH),
      [top_lsb] "M" ((WIDTH * ((HEIGHT / 8) - 1) - 2) & 0xFF),
      [top_msb] "M" ((WIDTH * ((HEIGHT / 8) - 1) - 2) >> 8),
      [clear]   "a" (clear)
    : "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25"
  );
#else
  //OLED SSD1306 and compatibles
  //data only transfer with clear support at 18 cycles per transfer
  asm volatile (
    "     ldi  r24,%[len_lsb]                     \n\t"
    "     ldi  r25,%[len_msb]                     \n\t"
    ".l1: ld   r20, Z                 ;2          \n\t"
    "     out  %[spdr], r20           ;1          \n\t" 
    "     cp   %[clear], __zero_reg__ ;1          \n\t" //if (clear) *(image++) = 0 
    "     breq .l2                    ;1/2 : 5/6  \n\t"
    "     st   Z+, __zero_reg__       ;2          \n\t" 
    "     rjmp .l3                    ;2          \n\t" 
    ".l2:                                         \n\t"
    "     adiw r30, 1                 ;2          \n\t" // else *(image++)
    "     nop                         ;1          \n\t" 
    ".l3:                                         \n\t"
    "     rjmp .+0                    ;2          \n\t" 
    "     rjmp .+0                    ;2          \n\t" 
    "     rjmp .+0                    ;2          \n\t" 
    "     sbiw  r24, 1                ;1          \n\t"
    "     brne .l1                    ;1/2 : 18   \n\t"
    :
    : [ptr]     "z" (image),
      [spdr]    "I" (_SFR_IO_ADDR(SPDR)),
      [len_msb] "M" (WIDTH * (HEIGHT / 8) >> 8),
      [len_lsb] "M" (WIDTH * (HEIGHT / 8) & 0xFF),
      [clear]   "a" (clear)
    : "r20", "r24", "r25"
  );
#endif  
  while (!(SPSR & _BV(SPIF))); // wait for the last transfer to finish
}

void Arduboy2Core::blank()
{
#ifdef OLED_SH1106 
  for (int i = 0; i < (HEIGHT*132)/8; i++)
    SPItransfer(0x00);
#elif defined(OLED_96X96)
  for (int i = 0; i < (HEIGHT*WIDTH)/2; i++)
    SPItransfer(0x00);
#else //OLED SSD1306 and compatibles
  for (int i = 0; i < (HEIGHT*WIDTH)/8; i++)
    SPItransfer(0x00);
#endif
}

void Arduboy2Core::sendLCDCommand(uint8_t command)
{
  LCDCommandMode();
  SPItransfer(command);
  LCDDataMode();
}

// invert the display or set to normal
// when inverted, a pixel set to 0 will be on
void Arduboy2Core::invert(bool inverse)
{
  sendLCDCommand(inverse ? OLED_PIXELS_INVERTED : OLED_PIXELS_NORMAL);
}

// turn all display pixels on, ignoring buffer contents
// or set to normal buffer display
void Arduboy2Core::allPixelsOn(bool on)
{
  sendLCDCommand(on ? OLED_ALL_PIXELS_ON : OLED_PIXELS_FROM_RAM);
}

// flip the display vertically or set to normal
void Arduboy2Core::flipVertical(bool flipped)
{
  sendLCDCommand(flipped ? OLED_VERTICAL_FLIPPED : OLED_VERTICAL_NORMAL);
}

// flip the display horizontally or set to normal
void Arduboy2Core::flipHorizontal(bool flipped)
{
  sendLCDCommand(flipped ? OLED_HORIZ_FLIPPED : OLED_HORIZ_NORMAL);
}

/* RGB LED */

void Arduboy2Core::setRGBled(uint8_t red, uint8_t green, uint8_t blue)
{
#ifdef ARDUBOY_10 // RGB, all the pretty colors
  // inversion is necessary because these are common annode LEDs
  analogWrite(RED_LED, 255 - red);
  analogWrite(GREEN_LED, 255 - green);
  analogWrite(BLUE_LED, 255 - blue);
#elif defined(AB_DEVKIT)
  // only blue on DevKit, which is not PWM capable
  (void)red;    // parameter unused
  (void)green;  // parameter unused
  bitWrite(BLUE_LED_PORT, BLUE_LED_BIT, blue ? RGB_ON : RGB_OFF);
#endif
}

void Arduboy2Core::digitalWriteRGB(uint8_t red, uint8_t green, uint8_t blue)
{
#ifdef ARDUBOY_10
  bitWrite(RED_LED_PORT, RED_LED_BIT, red);
  bitWrite(GREEN_LED_PORT, GREEN_LED_BIT, green);
  bitWrite(BLUE_LED_PORT, BLUE_LED_BIT, blue);
#elif defined(AB_DEVKIT)
  // only blue on DevKit
  (void)red;    // parameter unused
  (void)green;  // parameter unused
  bitWrite(BLUE_LED_PORT, BLUE_LED_BIT, blue);
#endif
}

void Arduboy2Core::digitalWriteRGB(uint8_t color, uint8_t val)
{
#ifdef ARDUBOY_10
  if (color == RED_LED)
  {
    bitWrite(RED_LED_PORT, RED_LED_BIT, val);
  }
  else if (color == GREEN_LED)
  {
    bitWrite(GREEN_LED_PORT, GREEN_LED_BIT, val);
  }
  else if (color == BLUE_LED)
  {
    bitWrite(BLUE_LED_PORT, BLUE_LED_BIT, val);
  }
#elif defined(AB_DEVKIT)
  // only blue on DevKit
  if (color == BLUE_LED)
  {
    bitWrite(BLUE_LED_PORT, BLUE_LED_BIT, val);
  }
#endif
}

/* Buttons */

uint8_t Arduboy2Core::buttonsState()
{
  uint8_t buttons;

  // using ports here is ~100 bytes smaller than digitalRead()
#ifdef AB_DEVKIT
  // down, left, up
  buttons = ((~PINB) & B01110000);
  // right button
  if ((PINC & _BV(6)) == 0) buttons |= RIGHT_BUTTON; //compiles to shorter and faster code

  // A and B
  if ((PINF & _BV(7)) == 0) buttons |= A_BUTTON; 
  if ((PINF & _BV(6)) == 0) buttons |= B_BUTTON; 
#elif defined(ARDUBOY_10)
  // down, up, left right
  buttons = ((~PINF) & B11110000);
  // A (left)
if ((PINE & _BV(6)) == 0) {buttons |= A_BUTTON;}
  // B (right)
  if ((PINB & _BV(4)) == 0) {buttons |= B_BUTTON;}
#endif
#ifdef ENABLE_BOOTLOADER_KEYS
  //bootloader button combo
  if (buttons == (LEFT_BUTTON | UP_BUTTON | A_BUTTON | B_BUTTON))
  { cli();
    //set magic boot key
    *(uint8_t *)0x0800 = 0x77;//using uint8_t saves an instruction
    *(uint8_t *)0x0801 = 0x77;
    //enable and trigger watchdog by timeout
    wdt_enable(WDTO_15MS); 
    while (true);
  }
#endif

  return buttons;
}

// delay in ms with 16 bit duration
void Arduboy2Core::delayShort(uint16_t ms)
{
  delay((unsigned long) ms);
}

