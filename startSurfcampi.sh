#!/bin/bash

cd /home/pi/surfcampi/
git pull

./surfcampi.sh

sudo halt

exit 0
