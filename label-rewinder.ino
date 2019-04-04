/*
  label-rewinder
  Controls an ATtiny 84 for rewinding labels on roll.
  Parts: 1xATtiny84, 2x74hc595, 1xl293d, 4-Digit Seven-Segment-Display, 3x MicroPushButtons, 4x 330Ohm Resistor

  This C code is in the public domain.

  modified 03 April 2019
  by Mahmut Keygubatli
*/

//Pins for Buttons
int runBtn = 8;
int stopBtn = 7;
int changeModeBtn = 3;

//Pins for motor control
int motorSpinLeft = 10;
int motorSpinRight = 9;

//Pins for 4-digit seven-segment display
//Pin connected to ST_CP of 74HC595
int latchPin = 1;
//Pin connected to SH_CP of 74HC595
int clockPin = 0;
////Pin connected to DS of 74HC595
int dataPin = 2;

//Motor mode
int spinleft = LOW;
int spinRight = LOW;

//Display Data
byte displayDataRollLeft[] = {4 + 64, 1 + 128, 16 + 2, 64 + 4, 128 + 1, 2 + 16}; // 6
byte displayModeStep[] = {
    0,
    32,
    0,
    32,
    1,
    1 + 4,
    1 + 4 + 2,
    1 + 4 + 2 + 128,
    1 + 4 + 2 + 128 + 64,
    1 + 4 + 2 + 128 + 64 + 16,
    1 + 4 + 2 + 128 + 64 + 16 + 8,
    1 + 4 + 2 + 128 + 64 + 16,
    1 + 4 + 2 + 128 + 64,
    1 + 4 + 2 + 128,
    1 + 4 + 2,
    1 + 4,
    1,
    0}; // 18

// controler mode
// this mode can used for spinning motor in single step
// to left with run-button or to right with stopBtn
#define SINGLE_STEP 0

// this mode can used for rewindig label with the control arm.
// If arm is on up-position motor will stop, on down position motor will run
#define REWIND_LABEL 1

void showDisplay(byte seven, byte digitPart)
{
  shiftOut(dataPin, clockPin, MSBFIRST, digitPart);
  shiftOut(dataPin, clockPin, MSBFIRST, seven);
  digitalWrite(latchPin, HIGH);
  digitalWrite(latchPin, LOW);
}

int rewindDiplayCounter = 0;
void rewindLabel()
{
  if (digitalRead(runBtn) == LOW)
  {
    spinleft = HIGH;
  }
  if (digitalRead(stopBtn) == LOW)
  {
    spinleft = LOW;
  }
  digitalWrite(motorSpinLeft, spinleft);
  digitalWrite(motorSpinRight, LOW);

  //increase Display counter
  (spinleft == HIGH) && rewindDiplayCounter++;

  //show display
  byte seven = displayDataRollLeft[(rewindDiplayCounter) % 6];
  showDisplay(seven, 0);

  if (rewindDiplayCounter >= 6 * 100)
  {
    rewindDiplayCounter = 0;
  }
}

int singleStepCounter = 0;
void singleStep()
{
  if (digitalRead(runBtn) == LOW)
  {
    spinleft = HIGH;
    spinRight = LOW;
    singleStepCounter = 1;
  }
  else if (digitalRead(stopBtn) == LOW)
  {
    spinRight = HIGH;
    spinleft = LOW;
    singleStepCounter = 1;
  }
  if (singleStepCounter > 0)
  {
    singleStepCounter--;
  }
  else
  {
    spinRight = LOW;
    spinleft = LOW;
  }
  digitalWrite(motorSpinLeft, spinleft);
  digitalWrite(motorSpinRight, spinRight);

  //show display
  byte seven;
  if (spinleft == HIGH)
  {
    seven = 2 + 128 + 64; //L
  }
  else if (spinRight == HIGH)
  {
    seven = 4 + 2 + 128; //R
  }
  else
  {
    seven = 2 + 1 + 8 + 128 + 16; //H
  }
  showDisplay(seven, 0);
}

int mode = REWIND_LABEL;
int modeCounter = 0;
boolean onModeChange = false;
void checkModeChange()
{
  if (digitalRead(changeModeBtn) == LOW)
  {
    onModeChange = true;
    if (modeCounter == 0)
    {
      modeCounter = 18;
    }
    else if (modeCounter > 0)
    {
      modeCounter--;
    }
  }
  else
  {
    if (onModeChange == true)
    {
      showDisplay(0, 0);
    }

    modeCounter = 0;
    onModeChange = false;
  }
  if (onModeChange && modeCounter == 1)
  {
    modeCounter = -1;
    if (mode == SINGLE_STEP)
    {
      mode = REWIND_LABEL;
    }
    else
    {
      mode = SINGLE_STEP;
    }
  }
  if (onModeChange)
  {
    byte seven;
    int i = 18 - modeCounter;
    if (modeCounter == -1)
    {
      seven = 1 + 4 + 2 + 8 + 128 + 16; // A
    }
    else
    {
      seven = displayModeStep[i];
    }
    showDisplay(seven, 0);

    digitalWrite(motorSpinLeft, LOW);
    digitalWrite(motorSpinRight, LOW);
    spinleft = LOW;
    spinRight = LOW;
    delay(100);
  }
}

void setup()
{
  //input pins
  pinMode(runBtn, INPUT_PULLUP);
  pinMode(stopBtn, INPUT_PULLUP);
  pinMode(changeModeBtn, INPUT_PULLUP);

  //output pins for motor
  pinMode(motorSpinLeft, OUTPUT);
  pinMode(motorSpinRight, OUTPUT);
  digitalWrite(motorSpinLeft, LOW);
  digitalWrite(motorSpinRight, LOW);

  //output pins for display
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
}

void loop()
{
  checkModeChange();

  if (!onModeChange && mode == REWIND_LABEL)
  {
    rewindLabel();
  }
  else if (!onModeChange && mode == SINGLE_STEP)
  {
    singleStep();
  }
  delay(100);
}
