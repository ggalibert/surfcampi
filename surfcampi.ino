/*
* Program: surfcampi
* Copyright: Guillaume Galibert, LLC 2015+. All rights reserved.
* Author: Guillaume Galibert (guillaume.galibert@gmail.com)
* Licensed under the BSD License.
*
* Arduino program that controls surfcampi's Sleepy Pi.  It
* wakes up hourly the surfcampi's Raspberry Pi during daylight only.
*
* Requires the following external libraries:
* 1) DS1307RTC - http://www.pjrc.com/teensy/td_libs_DS1307RTC.html
* 2) Time - https://github.com/kachok/arduino-libraries/tree/master/Time
* 3) Low-Power - https://github.com/rocketscream/Low-Power
* 4) SleepPi - https://github.com/SpellFoundry/SleepyPi
*/

// ...Interval - the time between RPi Power-ups
#define TIME_INTERVAL_SECONDS    0
#define TIME_INTERVAL_MINUTES    0
#define TIME_INTERVAL_HOURS      1
#define TIME_INTERVAL_DAYS       0

// ...Failsafe time. If your RPi hasn't
// ...shutdown, we'll shut it down anyway
// ...Note: the Failsafe should be less than your
// ........ startup Interval!!
#define RPI_POWER_TIMEOUT_MS     180000    // in ms - so this is 3 min

// **** INCLUDES *****
#include "SleepyPi.h"
#include <Time.h>
#include <LowPower.h>
#include <DS1374RTC.h>
#include <Wire.h>

typedef enum {
   ePI_OFF = 0,
   ePI_BOOTING,
   ePI_ON,
   ePI_SHUTTING_DOWN,
   ePI_SHUTDOWN
}ePISTATE;

ePISTATE  rpiState;
const int LED_PIN = 13;
const int SUPPLY_PIN = A6;
int supplyReading = 0;
const char *monthName[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

const int startupHour = 6; // in local time
const int shutdownHour = 18;

void alarm_isr()
{
    // Just a handler for the alarm interrupt

}

void setup()
{ 
  tmElements_t powerUpInterval, compileTime;
  
  analogReference(DEFAULT);   // 3v3
  
  // Configure "Standard" LED pin
  pinMode(LED_PIN, OUTPUT);		
  digitalWrite(LED_PIN,LOW);		// Switch off LED

  SleepyPi.enablePiPower(false); 
  SleepyPi.enableExtPower(false);
  
  rpiState = ePI_OFF;

  // initialize serial communication: In Arduino IDE use "Serial Monitor"
  Serial.begin(9600);
  Serial.println("Starting, but I'm going to go to sleep for a while...");  
  
  // get the date and time the compiler was run
  if (getDate(__DATE__,compileTime) && getTime(__TIME__,compileTime)) 
  {
    // and configure the RTC with this info
    SleepyPi.setTime(compileTime);
  }
  
  powerUpInterval.Second = TIME_INTERVAL_SECONDS; 
  powerUpInterval.Minute = TIME_INTERVAL_MINUTES;
  powerUpInterval.Hour   = TIME_INTERVAL_HOURS;
  powerUpInterval.Day    = TIME_INTERVAL_DAYS;
  Serial.print("Alarm Interval: ");
  printTime(powerUpInterval,false);

  SleepyPi.setAlarm(powerUpInterval);
  SleepyPi.enableWakeupAlarm();
  delay(1000);  
}

void loop() 
{
    unsigned long timeNowMs, timeStartMs;
    tmElements_t  currentTime; 
    bool pi_running;
  
    // Allow wake up alarm to trigger interrupt on falling edge.
    attachInterrupt(0, alarm_isr, FALLING);		// Alarm pin
    SleepyPi.ackAlarm();
    
    // Enter power down state with ADC and BOD module disabled.
    // Wake up when wake up pin is low.
    SleepyPi.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
    
    // Disable external pin interrupt on wake up pin.
    detachInterrupt(0); 
    
    Serial.print("I've Just woken up: ");
    
    // check battery state
    // read the input on analog pin 6
    supplyReading = analogRead(SUPPLY_PIN);

    // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
    float batteryVoltage = supplyReading * 0.00322 * 1056 / 56; // resolution is 3.22mV, divider is 1 056 000
    Serial.print("Battery voltage is ");
    Serial.print(batteryVoltage);
    Serial.println("V");
    
    //SleepyPi.enablePiPower(true);   
    SleepyPi.readTime(currentTime);
    
    int crtHour = currentTime.Hour;
    int crtMin = currentTime.Minute;
    Serial.print("Current time is ");
    Serial.print(crtHour);
    Serial.print(":");
    Serial.print(crtMin);
    Serial.println(" Z");
    
    //Determine if the pi should be on or off
    if (crtHour >= startupHour && crtHour <= shutdownHour) {
      Serial.println("Time to take a picture!");
      
      SleepyPi.enablePiPower(true);
      
      rpiState = ePI_BOOTING;
      
      // The RPi is now awake. Wait for it to shutdown or
      // Force it off after a set timeout.  
      timeStartMs = timeNowMs = millis();
      while (((timeNowMs - timeStartMs) < RPI_POWER_TIMEOUT_MS) && rpiState != ePI_SHUTDOWN)
      {
           pi_running = SleepyPi.checkPiStatus(false);
           if(pi_running == true)
           {
              Serial.print("+");
              rpiState = ePI_ON;
              delay(200);      // milliseconds   
           }
           else
           {
             if(rpiState == ePI_BOOTING)
             { 
                Serial.print("."); 
                delay(500);      // milliseconds   
             }
             else {
                Serial.println(); 
                Serial.println("RPi not running...");
                rpiState = ePI_SHUTDOWN;               
             }
           }
           timeNowMs = millis();
      }
      // Did a timeout occur?
      if((timeNowMs - timeStartMs) >= RPI_POWER_TIMEOUT_MS)
      {
         Serial.println(); 
         Serial.print("TimeOut! At: ");
         if (SleepyPi.readTime(currentTime)) 
         {
            printTime(currentTime,false); 
         }
         // Manually Shutdown the Rpi
         SleepyPi.piShutdown(true);      
         SleepyPi.enableExtPower(false);
         rpiState = ePI_OFF; 
              
      }
      else
      {
         Serial.println(); 
         Serial.println("RPi Shutdown, I'm going back to sleep..."); 
         SleepyPi.piShutdown(true);      
         SleepyPi.enableExtPower(false);
         rpiState = ePI_OFF;       
      }
    }
    
    digitalWrite(LED_PIN,HIGH);		// Switch on LED
    delay(250);  
    digitalWrite(LED_PIN,LOW);		// Switch off LED 
}

bool printTime(tmElements_t tm, bool printDate)
{
    print2digits(tm.Hour);
    Serial.write(':');
    print2digits(tm.Minute);
    Serial.write(':');
    print2digits(tm.Second);
    if(printDate == true)
    {
      Serial.print(", Date (D/M/Y) = ");
      Serial.print(tm.Day);
      Serial.write('/');
      Serial.print(tm.Month);
      Serial.write('/');
      Serial.print(tmYearToCalendar(tm.Year));
    }
    Serial.println();   
}

bool getTime(const char *str, tmElements_t &time)
{
  int Hour, Min, Sec;

  if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3) return false;
  time.Hour = Hour;
  time.Minute = Min;
  time.Second = Sec;
  return true;
}

bool getDate(const char *str, tmElements_t &time)
{
  char Month[12];
  int Day, Year;
  uint8_t monthIndex;

  if (sscanf(str, "%s %d %d", Month, &Day, &Year) != 3) return false;
  for (monthIndex = 0; monthIndex < 12; monthIndex++) {
    if (strcmp(Month, monthName[monthIndex]) == 0) break;
  }
  if (monthIndex >= 12) return false;
  time.Day = Day;
  time.Month = monthIndex + 1;
  time.Year = CalendarYrToTm(Year);
  return true;
}

void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}
