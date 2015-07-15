#!/bin/bash
hour=$1
outputFile="/tmp/surfcampi_$hour.gif"
convert -delay 30 -loop 0 /tmp/surfcampi_*.jpg $outputFile
echo $outputFile
exit 0
