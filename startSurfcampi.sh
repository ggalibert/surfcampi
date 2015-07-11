#!/bin/bash
cd /home/pi/surfcampi/
git pull &> startSurfcampi.log
./surfcampi.sh &>> startSurfcampi.log 
sudo halt  &>> startSurfcampi.log
exit 0
