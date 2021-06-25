 /*
  Breakout
 Copyright (C) 2011 Sebastian Goscik
 All rights reserved.

 Modifications by Scott Allen 2016, 2018, 2020
 after previous changes by person(s) unknown.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 */

#include <Arduboy2.h>

// block in EEPROM to save high scores
#define EE_FILE 2

// EEPROM space used: 35 bytes (7*(3+2)) starting at
// EEPROM_STORAGE_SPACE_START + (EE_FILE * 35)

Arduboy2 arduboy;
BeepPin1 beep;

constexpr uint8_t frameRate = 40; // Frame rate in frames per second

// Tone frequencies. Converted to count values for the beep class
constexpr uint16_t tonePaddle = beep.freq(200); // Ball hits paddle
constexpr uint16_t toneBrick = beep.freq(261); // Ball hits a brick
constexpr uint16_t toneEdge = beep.freq(523); // Ball hits top or sides
constexpr uint16_t toneMiss = beep.freq(175); // Ball misses paddle, lose life
constexpr uint16_t toneInitialsChange = beep.freq(523); // Change initials
constexpr uint16_t toneInitialsMove = beep.freq(1046); // Select initials
// Tone durations
constexpr uint8_t toneTimeBeep = 250 / (1000 / frameRate); // Game (frames)
constexpr uint16_t toneTimeMiss = 500; // Miss paddle (milliseconds)
constexpr uint16_t toneTimeInitials = 80; // Initials entry (milliseconds)


constexpr unsigned int columns = 13; //Columns of bricks
constexpr unsigned int rows = 4;     //Rows of bricks

int dx = -1;        //Initial movement of ball
int dy = -1;        //Initial movement of ball
int xb;           //Ball's starting position
int yb;           //Ball's starting position
boolean released;     //If the ball has been released by the player
boolean paused = false;   //If the game has been paused
byte xPaddle;       //X position of paddle
boolean isHit[rows][columns];   //Array of if bricks are hit or not
boolean bounced=false;  //Used to fix double bounce glitch
byte lives = 3;       //Amount of lives
byte level = 1;       //Current level
unsigned int score=0;   //Score for the game
unsigned int brickCount;  //Amount of bricks hit
boolean pad, pad2, pad3;  //Button press buffer used to stop pause repeating
boolean oldpad, oldpad2, oldpad3;
char text_buffer[16];      //General string buffer
boolean start=false;    //If in menu or in game
boolean initialDraw=false;//If the inital draw has happened
char initials[3];     //Initials used in high score

//Ball Bounds used in collision detection
byte leftBall;
byte rightBall;
byte topBall;
byte bottomBall;

//Brick Bounds used in collision detection
byte leftBrick;
byte rightBrick;
byte topBrick;
byte bottomBrick;

byte tick;

void setup()
{
  arduboy.begin();
  beep.begin();
  arduboy.setFrameRate(frameRate);
  arduboy.initRandomSeed();
}

void loop()
{
  // pause render until it's time for the next frame
  if (!(arduboy.nextFrame()))
    return;

  // Handle the timing and stopping of tones
  beep.timer();

  //Title screen loop switches from title screen
  //and high scores until FIRE is pressed
  while (!start)
  {
    start = titleScreen();
    if (!start)
    {
      start = displayHighScores(EE_FILE);
    }
  }

  //Initial level draw
  if (!initialDraw)
  {
    //Clears the screen
    arduboy.clear();
    //Selects Font
    //Draws the new level
    level = 1;
    initialDraw=true;
    newLevel();
    score = 0;
  }

  if (lives>0)
  {
    drawPaddle();

    //Pause game if FIRE pressed
    pad = arduboy.pressed(A_BUTTON) || arduboy.pressed(B_BUTTON);

    if(pad == true && oldpad == false && released)
    {
      oldpad2 = false; //Forces pad loop 2 to run once
      pause();
    }

    oldpad = pad;
    drawBall();

    if(brickCount == rows * columns)
    {
      level++;
      newLevel();
    }
  }
  else
  {
    drawGameOver();
    if (score > 0)
    {
      enterHighScore(EE_FILE);
    }

    arduboy.clear();
    initialDraw=false;
    start=false;
    lives=3;
    newLevel();
  }

  arduboy.display();
}

void movePaddle()
{
  //Move right
  if(xPaddle < WIDTH - 12)
  {
    if (arduboy.pressed(RIGHT_BUTTON))
    {
      xPaddle+=2;
    }
  }

  //Move left
  if(xPaddle > 0)
  {
    if (arduboy.pressed(LEFT_BUTTON))
    {
      xPaddle-=2;
    }
  }
}

void moveBall()
{
  tick++;
  if(released)
  {
    //Move ball
    if (abs(dx)==2)
    {
      xb += dx/2;
      // 2x speed is really 1.5 speed
      if (tick%2==0)
        xb += dx/2;
    }
    else
    {
      xb += dx;
    }
    yb=yb + dy;

    //Set bounds
    leftBall = xb;
    rightBall = xb + 2;
    topBall = yb;
    bottomBall = yb + 2;

    //Bounce off top edge
    if (yb <= 0)
    {
      yb = 2;
      dy = -dy;
      playTone(toneEdge, toneTimeBeep);
    }

    //Lose a life if bottom edge hit
    if (yb >= 64)
    {
      arduboy.drawRect(xPaddle, 63, 11, 1, BLACK);
      xPaddle = 54;
      yb=60;
      released = false;
      lives--;
      playToneTimed(toneMiss, toneTimeMiss);
      if (random(0, 2) == 0)
      {
        dx = 1;
      }
      else
      {
        dx = -1;
      }
    }

    //Bounce off left side
    if (xb <= 0)
    {
      xb = 2;
      dx = -dx;
      playTone(toneEdge, toneTimeBeep);
    }

    //Bounce off right side
    if (xb >= WIDTH - 2)
    {
      xb = WIDTH - 4;
      dx = -dx;
      playTone(toneEdge, toneTimeBeep);
    }

    //Bounce off paddle
    if (xb+1>=xPaddle && xb<=xPaddle+12 && yb+2>=63 && yb<=64)
    {
      dy = -dy;
      dx = ((xb-(xPaddle+6))/3); //Applies spin on the ball
      // prevent straight bounce
      if (dx == 0)
      {
        dx = (random(0, 2) == 1) ? 1 : -1;
      }
      playTone(tonePaddle, toneTimeBeep);
    }

    //Bounce off Bricks
    for (byte row = 0; row < rows; row++)
    {
      for (byte column = 0; column < columns; column++)
      {
        if (!isHit[row][column])
        {
          //Sets Brick bounds
          leftBrick = 10 * column;
          rightBrick = 10 * column + 10;
          topBrick = 6 * row + 1;
          bottomBrick = 6 * row + 7;

          //If A collision has occurred
          if (topBall <= bottomBrick && bottomBall >= topBrick &&
              leftBall <= rightBrick && rightBall >= leftBrick)
          {
            Score();
            brickCount++;
            isHit[row][column] = true;
            arduboy.drawRect(10*column, 2+6*row, 8, 4, BLACK);

            //Vertical collision
            if (bottomBall > bottomBrick || topBall < topBrick)
            {
              //Only bounce once each ball move
              if(!bounced)
              {
                dy =- dy;
                yb += dy;
                bounced = true;
                playTone(toneBrick, toneTimeBeep);
              }
            }

            //Horizontal collision
            if (leftBall < leftBrick || rightBall > rightBrick)
            {
              //Only bounce once brick each ball move
              if(!bounced)
              {
                dx =- dx;
                xb += dx;
                bounced = true;
                playTone(toneBrick, toneTimeBeep);
              }
            }
          }
        }
      }
    }
    //Reset Bounce
    bounced = false;
  }
  else
  {
    //Ball follows paddle
    xb=xPaddle + 5;

    //Release ball if FIRE pressed
    pad3 = arduboy.pressed(A_BUTTON) || arduboy.pressed(B_BUTTON);
    if (pad3 == true && oldpad3 == false)
    {
      released = true;

      //Apply random direction to ball on release
      if (random(0, 2) == 0)
      {
        dx = 1;
      }
      else
      {
        dx = -1;
      }
      //Makes sure the ball heads upwards
      dy = -1;
    }
    oldpad3 = pad3;
  }
}

void drawBall()
{
  arduboy.drawPixel(xb,   yb,   BLACK);
  arduboy.drawPixel(xb+1, yb,   BLACK);
  arduboy.drawPixel(xb,   yb+1, BLACK);
  arduboy.drawPixel(xb+1, yb+1, BLACK);

  moveBall();

  arduboy.drawPixel(xb,   yb,   WHITE);
  arduboy.drawPixel(xb+1, yb,   WHITE);
  arduboy.drawPixel(xb,   yb+1, WHITE);
  arduboy.drawPixel(xb+1, yb+1, WHITE);
}

void drawPaddle()
{
  arduboy.drawRect(xPaddle, 63, 11, 1, BLACK);
  movePaddle();
  arduboy.drawRect(xPaddle, 63, 11, 1, WHITE);
}

void drawGameOver()
{
  arduboy.drawPixel(xb,   yb,   BLACK);
  arduboy.drawPixel(xb+1, yb,   BLACK);
  arduboy.drawPixel(xb,   yb+1, BLACK);
  arduboy.drawPixel(xb+1, yb+1, BLACK);
  arduboy.setCursor(37, 42);
  arduboy.print("Game Over");
  arduboy.setCursor(31, 56);
  arduboy.print("Score: ");
  arduboy.print(score);
  arduboy.display();
  arduboy.delayShort(4000);
}

void pause()
{
  paused = true;
  //Stop tone if playing
  beep.noTone();
  //Draw pause to the screen
  arduboy.setCursor(52, 45);
  arduboy.print("PAUSE");
  arduboy.display();
  while (paused)
  {
    arduboy.delayShort(150);
    //Unpause if FIRE is pressed
    pad2 = arduboy.pressed(A_BUTTON) || arduboy.pressed(B_BUTTON);
    if (pad2 == true && oldpad2 == false && released)
    {
        arduboy.fillRect(52, 45, 30, 11, BLACK);

        paused=false;
    }
    oldpad2 = pad2;
  }
}

void Score()
{
  score += (level*10);
}

void newLevel()
{
  //Undraw paddle
  arduboy.drawRect(xPaddle, 63, 11, 1, BLACK);

  //Undraw ball
  arduboy.drawPixel(xb,   yb,   BLACK);
  arduboy.drawPixel(xb+1, yb,   BLACK);
  arduboy.drawPixel(xb,   yb+1, BLACK);
  arduboy.drawPixel(xb+1, yb+1, BLACK);

  //Alter various variables to reset the game
  xPaddle = 54;
  yb = 60;
  brickCount = 0;
  released = false;

  //Draws new bricks and resets their values
  for (byte row = 0; row < 4; row++)
  {
    for (byte column = 0; column < 13; column++)
    {
      isHit[row][column] = false;
      arduboy.drawRect(10*column, 2+6*row, 8, 4, WHITE);
    }
  }

  if (!initialDraw)
  {
    arduboy.clear();
  }
  else
  {
    arduboy.display();
  }
}

//Used to delay images while reading button input
boolean pollFireButton(int n)
{
  for(int i = 0; i < n; i++)
  {
    arduboy.delayShort(15);
    pad = arduboy.pressed(A_BUTTON) || arduboy.pressed(B_BUTTON);
    if(pad == true && oldpad == false)
    {
      oldpad3 = true; //Forces pad loop 3 to run once
      return true;
    }
    oldpad = pad;
  }
  return false;
}

//Function by nootropic design to display high scores
boolean displayHighScores(byte file)
{
  byte y = 8;
  byte x = 24;
  // Each block of EEPROM has 7 high scores, and each high score entry
  // is 5 bytes long:  3 bytes for initials and two bytes for score.
  int address = file * 7 * 5 + EEPROM_STORAGE_SPACE_START;
  byte hi, lo;
  arduboy.clear();
  arduboy.setCursor(32, 0);
  arduboy.print("HIGH SCORES");
  arduboy.display();

  for(int i = 0; i < 7; i++)
  {
    sprintf(text_buffer, "%2d", i+1);
    arduboy.setCursor(x, y+(i*8));
    arduboy.print(text_buffer);
    arduboy.display();
    hi = EEPROM.read(address + (5*i));
    lo = EEPROM.read(address + (5*i) + 1);

    if ((hi == 0xFF) && (lo == 0xFF))
    {
      score = 0;
    }
    else
    {
      score = (hi << 8) | lo;
    }

    initials[0] = (char)EEPROM.read(address + (5*i) + 2);
    initials[1] = (char)EEPROM.read(address + (5*i) + 3);
    initials[2] = (char)EEPROM.read(address + (5*i) + 4);

    if (score > 0)
    {
      sprintf(text_buffer, "%c%c%c %u", initials[0], initials[1], initials[2], score);
      arduboy.setCursor(x + 24, y + (i*8));
      arduboy.print(text_buffer);
      arduboy.display();
    }
  }
  if (pollFireButton(300))
  {
    return true;
  }
  return false;
  arduboy.display();
}

boolean titleScreen()
{
  //Clears the screen
  arduboy.clear();
  arduboy.setCursor(16, 22);
  arduboy.setTextSize(2);
  arduboy.print("BREAKOUT");
  arduboy.setTextSize(1);
  arduboy.display();
  if (pollFireButton(25))
  {
    return true;
  }

  //Flash "Press FIRE" 5 times
  for(byte i = 0; i < 5; i++)
  {
    //Draws "Press FIRE"
    arduboy.setCursor(31, 53);
    arduboy.print("PRESS FIRE!");
    arduboy.display();

    if (pollFireButton(50))
    {
      return true;
    }

    //Removes "Press FIRE"
    arduboy.setCursor(31, 53);
    arduboy.print("           ");
    arduboy.display();

    if (pollFireButton(25))
    {
      return true;
    }
  }

  return false;
}

//Function by nootropic design to add high scores
void enterInitials()
{
  byte index = 0;

  arduboy.clear();

  initials[0] = ' ';
  initials[1] = ' ';
  initials[2] = ' ';

  while (true)
  {
    arduboy.display();
    arduboy.clear();

    arduboy.setCursor(16, 0);
    arduboy.print("HIGH SCORE");
    sprintf(text_buffer, "%u", score);
    arduboy.setCursor(88, 0);
    arduboy.print(text_buffer);
    arduboy.setCursor(56, 20);
    arduboy.print(initials[0]);
    arduboy.setCursor(64, 20);
    arduboy.print(initials[1]);
    arduboy.setCursor(72, 20);
    arduboy.print(initials[2]);
    for(byte i = 0; i < 3; i++)
    {
      arduboy.drawLine(56 + (i*8), 27, 56 + (i*8) + 6, 27, WHITE);
    }
    arduboy.drawLine(56, 28, 88, 28, BLACK);
    arduboy.drawLine(56 + (index*8), 28, 56 + (index*8) + 6, 28, WHITE);
    arduboy.delayShort(70);

    if (arduboy.pressed(LEFT_BUTTON) || arduboy.pressed(B_BUTTON))
    {
      if (index > 0)
      {
        index--;
        playToneTimed(toneInitialsMove, toneTimeInitials);
      }
    }

    if (arduboy.pressed(RIGHT_BUTTON))
    {
      if (index < 2)
      {
        index++;
        playToneTimed(toneInitialsMove, toneTimeInitials);
      }
    }

    if (arduboy.pressed(UP_BUTTON))
    {
      initials[index]++;
      playToneTimed(toneInitialsChange, toneTimeInitials);
      // A-Z 0-9 :-? !-/ ' '
      if (initials[index] == '0')
      {
        initials[index] = ' ';
      }
      if (initials[index] == '!')
      {
        initials[index] = 'A';
      }
      if (initials[index] == '[')
      {
        initials[index] = '0';
      }
      if (initials[index] == '@')
      {
        initials[index] = '!';
      }
    }

    if (arduboy.pressed(DOWN_BUTTON))
    {
      initials[index]--;
      playToneTimed(toneInitialsChange, toneTimeInitials);
      if (initials[index] == ' ')
      {
        initials[index] = '?';
      }
      if (initials[index] == '/')
      {
        initials[index] = 'Z';
      }
      if (initials[index] == 31)
      {
        initials[index] = '/';
      }
      if (initials[index] == '@')
      {
        initials[index] = ' ';
      }
    }

    if (arduboy.pressed(A_BUTTON))
    {
      playToneTimed(toneInitialsMove, toneTimeInitials);
      if (index < 2)
      {
        index++;
      }
      else
      {
        return;
      }
    }
  }

}

void enterHighScore(byte file)
{
  // Each block of EEPROM has 7 high scores, and each high score entry
  // is 5 bytes long:  3 bytes for initials and two bytes for score.
  int address = file * 7 * 5 + EEPROM_STORAGE_SPACE_START;
  byte hi, lo;
  char tmpInitials[3];
  unsigned int tmpScore = 0;

  // High score processing
  for(byte i = 0; i < 7; i++)
  {
    hi = EEPROM.read(address + (5*i));
    lo = EEPROM.read(address + (5*i) + 1);
    if ((hi == 0xFF) && (lo == 0xFF))
    {
      // The values are uninitialized, so treat this entry
      // as a score of 0.
      tmpScore = 0;
    }
    else
    {
      tmpScore = (hi << 8) | lo;
    }
    if (score > tmpScore)
    {
      enterInitials();
      for(byte j = i; j < 7; j++)
      {
        hi = EEPROM.read(address + (5*j));
        lo = EEPROM.read(address + (5*j) + 1);

        if ((hi == 0xFF) && (lo == 0xFF))
        {
        tmpScore = 0;
        }
        else
        {
          tmpScore = (hi << 8) | lo;
        }

        tmpInitials[0] = (char)EEPROM.read(address + (5*j) + 2);
        tmpInitials[1] = (char)EEPROM.read(address + (5*j) + 3);
        tmpInitials[2] = (char)EEPROM.read(address + (5*j) + 4);

        // write score and initials to current slot
        EEPROM.update(address + (5*j), ((score >> 8) & 0xFF));
        EEPROM.update(address + (5*j) + 1, (score & 0xFF));
        EEPROM.update(address + (5*j) + 2, initials[0]);
        EEPROM.update(address + (5*j) + 3, initials[1]);
        EEPROM.update(address + (5*j) + 4, initials[2]);

        // tmpScore and tmpInitials now hold what we want to
        //write in the next slot.
        score = tmpScore;
        initials[0] = tmpInitials[0];
        initials[1] = tmpInitials[1];
        initials[2] = tmpInitials[2];
      }

      score = 0;
      initials[0] = ' ';
      initials[1] = ' ';
      initials[2] = ' ';

      return;
    }
  }
}

// Play a tone at a frequency corresponding to the specified precomputed count,
// for the specified number of frames.
void playTone(uint16_t count, uint8_t frames)
{
  beep.tone(count, frames);
}

// Play a tone at a frequency corresponding to the specified precomputed count,
// for the specified duration in milliseconds, using a delay.
// Used when beep.timer() isn't being called.
void playToneTimed(uint16_t count, uint16_t duration)
{
  beep.tone(count);
  arduboy.delayShort(duration);
  beep.noTone();
}

