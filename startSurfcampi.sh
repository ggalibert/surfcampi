#!/bin/bash
cd /home/pi/surfcampi/
git pull &> startSurfcampi.log
#./surfcampi.sh &>> startSurfcampi.log 
exit 0
