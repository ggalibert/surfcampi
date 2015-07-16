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
#include <Timezone.h>
#include <LowPower.h>
#include <DS1374RTC.h>
#include <Wire.h>

//Variables for dealing with time
tmElements_t tm;
TimeChangeRule myEDT = {"EDT", First, Sun, Oct, 2, 540};  //UTC + 9 hours
TimeChangeRule myEST = {"EST", First, Sun, Apr, 2, 600};   //UTC + 10 hours
Timezone myTz(myEDT, myEST);

const int startupHour = 6; // in local time
const int shutdownHour = 18;
boolean piShouldBeOn;
boolean piIsRunning;

void setup() {
  Serial.begin(9600);
  
  //Set the initial Power to be off
  SleepyPi.enablePiPower(false);  
  SleepyPi.enableExtPower(false); //expansion pins
}

void loop() {
  if (RTC.readTime(tm)) {
    piIsRunning = SleepyPi.checkPiStatus(false);
    int utcHour = tm.Hour;
    int crtMin = tm.Minute;
    
    Serial.print("Current time is ");
    Serial.print(utcHour);
    Serial.print(":");
    Serial.print(crtMin);
    Serial.println(" UTC");
    
    time_t utc = makeTime(tm);
    time_t local = myTz.toLocal(utc);
    int crtHour = hour(local);

    Serial.print("Current time is ");
    Serial.print(crtHour);
    Serial.print(":");
    Serial.print(crtMin);
    Serial.println(" Z");
    
    //Determine if the pi should be on or off
    if (crtHour >= startupHour && crtHour < shutdownHour && crtMin == 0) {
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
    
    if (piIsRunning == false && piShouldBeOn == true) {
      //Time to turn on
      Serial.println("Turning Pi on!"); 
      SleepyPi.enablePiPower(true);
      delay(60000); //hold for boot
      while(SleepyPi.checkPiStatus(false)){
        delay(10000); //checking status every 10sec
      }
      Serial.println("Turning power off!");
      delay(10000); //Give it time to shut down before cutting power
      SleepyPi.enablePiPower(false);
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
