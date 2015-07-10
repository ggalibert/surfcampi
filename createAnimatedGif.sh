#!/bin/bash
hour=`date +%H`
convert -delay 10 -loop 0 /tmp/surfcampi_*.jpg /tmp/surfcampi_$hour.gif
echo /tmp/surfcampi_$hour.gif
exit 0
