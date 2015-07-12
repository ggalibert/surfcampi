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

#include "SleepyPi.h"
#include <Time.h>
#include <LowPower.h>
#include <DS1374RTC.h>
#include <Wire.h>

#define ALRM1_MATCH_SEC        0b1110  // when seconds match
#define ALRM1_MATCH_MIN_SEC    0b1100  // when minutes and seconds match

#define ALRM2_ONCE_PER_MIN     0b111   // once per minute (00 seconds of every minute)
#define ALRM2_MATCH_MIN        0b110   // when minutes match

//Variables for dealing with date conversions
tmElements_t tm;
const int utcOffsetHoursDT = 9;
const int utcOffsetHoursST = 10;
const int dtStartMonth = 4;
const int dtStartDay = 5;
const int dtEndMonth = 10;
const int dtEndDay = 4;

const int startupHour = 7; //sunrise 7
const int shutdownHour = 19; //sunset 19
boolean piShouldBeOn;
boolean piIsRunning;

const int LED_PIN = 13;

void alarm_isr()
{
    // Just a handler for the alarm interrupt.
    // You could do something here

}

void setup() {
  // Configure "Standard" LED pin
  pinMode(LED_PIN, OUTPUT);		
  digitalWrite(LED_PIN,LOW);		// Switch off LED
  
  // initialize serial communication: In Arduino IDE use "Serial Monitor"
  Serial.begin(9600);
  
  //Set the initial Power to be false
  SleepyPi.enablePiPower(false);  
  SleepyPi.enableExtPower(false); //expansion pins
}

void loop() {
  // Allow wake up alarm to trigger interrupt on falling edge.
  attachInterrupt(0, alarm_isr, FALLING);		// Alarm pin

  SleepyPi.enableWakeupAlarm();
  //SleepyPi.setAlarm(60);              // in seconds
  SleepyPi.setAlarm(ALRM1_MATCH_SEC, 0); 
  // Enter power down state with ADC and BOD module disabled.
  // Wake up when wake up pin is low.
  SleepyPi.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
    
  // Disable external pin interrupt on wake up pin.
  detachInterrupt(0); 
    
  // Do something here
  // Example: Read sensor, data logging, data transmission.
  if (RTC.readTime(tm)) {
    piIsRunning = SleepyPi.checkPiStatus(false);
    int crtHour = getCrtHourUsingOffset();
    
    Serial.print("Current time is ");
    Serial.println(crtHour);
    
    //Determine if the pi should be on or off
    if (crtHour >= startupHour && crtHour < shutdownHour) {
      piShouldBeOn = true;
    } else {
      piShouldBeOn = false;
    }
    
    //Helps with debugging
    if (piIsRunning == true) {
      Serial.println("Pi is running!");
    } else {
      Serial.println("Pi is NOT running!");
    }
    
    if (piIsRunning == true && piShouldBeOn == false) {
      //Time to turn off
      Serial.println("Turning Pi off!");
      delay(200); //Give it time to print before going down
      SleepyPi.piShutdown(true);
    } else if (piIsRunning == false && piShouldBeOn == true) {
      //Time to turn on
      Serial.println("Turning Pi on!"); 
      SleepyPi.enablePiPower(true);
      delay(60000); //hold to boot in ms
      while(SleepyPi.checkPiStatus(false)){
        delay(10000); //hold to shutdown in ms
      }
      Serial.println("Turning power off!");
      delay(200); //Give it time to print before going down
      SleepyPi.enablePiPower(false);
    } else if (piIsRunning == false && piShouldBeOn == false) {
      //powerDown saves more juice than idle, but we have to leave
      //ADC_ON or else the wakeup will cause a startup of the Pi if off.
      Serial.println("All good, back into deep sleep!");
    } else {
      //Pi is operating as it should be, sleep and check again shortly
      Serial.println("All good, back into deep sleep!");
    }
  } else {
    if (RTC.chipPresent()) {
      Serial.println("The DS1374 RTC is stopped. Please set time!");
      Serial.println();
    } else {
      Serial.println("DS1374 read error!  Please check the circuitry.");
      Serial.println();
    }
    delay(10000); //Give it a few secs before trying again
  }
}

//Try to get the right offset based on what we know about daylight vs standard time.
//Note: not perfect, but pretty close.  Might be slightly off the day of time changes.
unsigned int getCrtOffset() {
  int crtMonth = tm.Month;
  int crtDay = tm.Day;
  unsigned int offset = 0;
  if (crtMonth > dtStartMonth && crtMonth < dtEndMonth) {
    //We are in between months so no need to check the day
    offset = utcOffsetHoursST;
  } else if (crtMonth == dtStartMonth) {
    //We have matched the start month, check the day
    if (crtDay >= dtStartDay) {
      offset = utcOffsetHoursST;
    } else {
      offset = utcOffsetHoursDT;
    }
  } else if (crtMonth == dtEndMonth) {
    //We have matched the end month, check the day
    if (crtDay <= dtEndDay) {
      offset = utcOffsetHoursST;
    } else {
      offset = utcOffsetHoursDT;
    }
  } else {
    offset = utcOffsetHoursDT;
  }
  
  return offset;
}

int getCrtHourUsingOffset() {
  //Time is returned in UTC so we must convert
  int crtHour;
  int utcHour = tm.Hour; //tm.Hour or manual for debugging
  int crtOffset = getCrtOffset();
  int crtUtcHour = utcHour + (crtOffset);
  
  //We must account for negatives
  if (crtUtcHour <= 0) {
    crtHour = 24 - abs(crtUtcHour);
  } else {
    crtHour = crtUtcHour;
  }
  
  return crtHour;
}
