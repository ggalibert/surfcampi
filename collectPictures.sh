#!/bin/bash

# set options for raspistill
pictureWidth=800
pictureHeight=600
jpegQuality=75
outputFile="/tmp/surfcampi_%02d.jpg"
timeLapseRate=10000
timeLapseLength=50000

# perform a time lapse of 6 pictures every 10sec with no preview
raspistill -n -w $pictureWidth -h $pictureHeight -q $jpegQuality -o $outputFile -tl $timeLapseRate -t $timeLapseLength

# add annotation to pictures
outputFiles=(`echo /tmp/surfcampi_*.jpg`)
for file in "${outputFiles[@]}"
do
  convert $file -fill white  -undercolor '#00000080'  -gravity NorthWest -pointsize 24 -annotate +5+5 " $file " $file
done

exit 0
