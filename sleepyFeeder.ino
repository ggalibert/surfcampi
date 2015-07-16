/*
* Program: SleepyFeeder (http://feedertweeter.net/)
* Copyright: Manifold, LLC 2014+. All rights reserved.
* Author: Chad Auld (cauld@wearemanifold.com)
* Licensed under the BSD License.
* https://bitbucket.org/cauld/sleepyfeeder/
*
* Arduino program that controls Feeder Tweeter's Sleepy Pi.  It
* starts and stops the Feeder's Raspberry Pi during non-daylight hours.
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

//Variables for dealing with date conversions
//Can get this date information from timeanddate.com
tmElements_t tm;
const int utcOffsetHoursDT = -7;
const int utcOffsetHoursST = -6;
const int dtStartMonth = 3;
const int dtStartDay = 9;
const int dtEndMonth = 11;
const int dtEndDay = 2;

const int startupHour = 7; //sunrise 7
const int shutdownHour = 19; //sunset 19
boolean piShouldBeOn;
boolean piIsRunning;

void setup() {
  Serial.begin(9600);
  
  //Set the initial Power to be off
  SleepyPi.enablePiPower(true);  
  SleepyPi.enableExtPower(false); //expansion pins
  
  //This delay gives us a minute to login to the box
  //right after boot so that we can kill the shutdown script
  //if we need to debug before the box shuts right back down.
  Serial.println("Entering startup debug delay...");
  delay(20000);
  Serial.println("Exiting startup debug delay...");
}

void loop() {
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
      delay(120000); //hold for boot
    } else if (piIsRunning == false && piShouldBeOn == false) {
      //powerDown saves more juice than idle, but we have to leave
      //ADC_ON or else the wakeup will cause a startup of the Pi if off.
      Serial.println("All good, back into deep sleep!");
      delay(200); //Give it time to print before going idle
      SleepyPi.powerDown(SLEEP_8S, ADC_ON, BOD_OFF);
    } else {
      //Pi is operating as it should be, sleep and check again shortly
      Serial.println("All good, taking an 8s nap!");
      delay(200); //Give it time to print before going idle
      SleepyPi.idle(SLEEP_8S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART0_OFF, TWI_OFF);
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

