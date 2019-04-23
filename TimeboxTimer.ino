/* 
 * Copyright (c) 2019 Florian Mücke
 * 
 * Requires: 
 *   - Arduino
 *   - 1637 4-DigiLED Display
 *   - active buzzer
 *   - 2x micro switch
 * 
 * Arduino
 * +5V  -->  Buzzer: VCC
 * +5V  -->    1637: +5V
 * GND  -->  Buzzer: GND
 * GND  -->    1637: GND
 * GND  -->  S1: bottom right
 * GND  -->  S2: bottom right
 * A0   -->  S1: top left
 * A1   -->  S2: top left
 * A2   -->  Buzzer: SIG
 * D2   -->   1637: CLK
 * D3   -->   1637: DIO
 */

#include "TM1637.h" // https://github.com/Seeed-Studio/Seeed_Grove_4Digital_Display_g/blob/master/TM1637.h

// wired pins
const int IncrementPin = A0;
const int StartStopPin = A1;
const int BuzzerPin = A2;
const int CLK_pin = 2; //pins definitions for TM1637 and can be changed to other ports
const int DIO_pin = 3;

// constants 
const unsigned int incrementInterval = 60;

// global vars
unsigned int startTick = 0;
int initialTime = 120;
int elapsedTime = 0;
unsigned int startPressedTick = 0;
unsigned long incrementPressedTick = 0;
int incCount = 0;
bool isRunning = false;
int timerValue = 0;
TM1637 disp(CLK_pin, DIO_pin);

void Beep()
{
  for (int i = 0; i < 3; ++i)
  {
    digitalWrite(BuzzerPin, HIGH);
    delay(100);
    digitalWrite(BuzzerPin, LOW);
    delay(100);
  }
}

void setup()
{
  pinMode(IncrementPin, INPUT_PULLUP);
  pinMode(StartStopPin, INPUT_PULLUP);
  pinMode(BuzzerPin, OUTPUT);
  
  disp.set(BRIGHT_DARKEST);
  disp.init();
  disp.point(POINT_ON);
  //Beep();

  //Serial.begin(9600);   
}

unsigned int GetCurrentTick() { return millis()/1000; }

void Reset()
{
  Stop();
  elapsedTime = 0;
}

void Start()
{
  isRunning = true;
  disp.set(BRIGHT_TYPICAL);
  startTick = GetCurrentTick() - elapsedTime;
}

void Stop()
{
  isRunning = false;
  disp.set(BRIGHT_DARKEST);  
}

void loop()
{
  const auto tick = GetCurrentTick();
  
  if (digitalRead(StartStopPin) == LOW)
  {
    if (startPressedTick == 0)
    {
      startPressedTick = tick;
    
      if (!isRunning)
      {
        Start();
      }
      else
      {
        Stop();
      }
      //Serial.println("StartStop pressed");
    }
    else if (tick - startPressedTick > 3)
    {
      initialTime = 0;
    }
    else if (tick - startPressedTick > 1)
    {
      Reset();
    }

  }

  if (digitalRead(StartStopPin) == HIGH && startPressedTick != 0)
  {
    startPressedTick = 0;
  }

  const auto incTick = millis();
  
  if (digitalRead(IncrementPin) == LOW)
  {
    if (incrementPressedTick == 0)
    {
      initialTime += incrementInterval;
      //Serial.println("Increment pressed");
      incrementPressedTick = incTick;
      ++incCount;
    }
    else if (
      (incCount > 10 && incTick - incrementPressedTick >= 50) ||
      (incCount > 2 && incTick - incrementPressedTick >= 150) 
          || incTick - incrementPressedTick >= 500)
    {
      initialTime += incrementInterval;
      incrementPressedTick = incTick;
      ++incCount;
    }
  
    //Serial.println(incTick - incrementPressedTick);
  }
  
  if (digitalRead(IncrementPin) == HIGH && incrementPressedTick != 0)
  {
    incrementPressedTick = 0;
    incCount = 0;
  }

  //Serial.print("tick: "); Serial.println(GetCurrentTick());
  //Serial.print("elapsed: "); Serial.println(elapsedTime);
  auto remainingTime = GetTimeRemaining();
  DisplayTime(remainingTime);

  if (isRunning && elapsedTime >= initialTime)
  {
    Stop();
    Beep();
  }  
}

int GetTimeRemaining()
{
  if (isRunning)
  {
    int timePassed = GetCurrentTick() - startTick;
    //Serial.print("passed: "); Serial.println(timePassed);
    elapsedTime = timePassed;
  }

  return initialTime - elapsedTime;
}

void DisplayTime(int totalSecs)
{
  if (timerValue != totalSecs)
  {
    timerValue = totalSecs;
    int minutes = totalSecs / 60;
    int seconds = totalSecs % 60;
    
    int8_t TimeDisp[] = {0x00,0x00,0x00,0x00};
    TimeDisp[0] = minutes / 10;
    TimeDisp[1] = minutes % 10;
    TimeDisp[2] = seconds / 10;
    TimeDisp[3] = seconds % 10;
    disp.display(TimeDisp);
    
    //Serial.print(minutes);
    //Serial.print(":");
    //Serial.println(seconds);
  }
}
