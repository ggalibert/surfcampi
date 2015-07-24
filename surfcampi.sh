#!/bin/bash

echo " "

# connect to 3G
#echo "connecting to 3G"
#sudo ./sakis3g connect

# print current date time
date

# update scripts from github repo
git pull

# define hours for when surfcampi is working
workingHours=(`cat ./workingHours.txt`)
hour=`date +%H`

# find out whether surfcampi should be working or not
isWorkingHour=0
for wHour in "${workingHours[@]}"
do
  if [ $wHour = $hour ]; then
    isWorkingHour=1
  fi
done

if [ $isWorkingHour = 1 ]; then
  # surfcampi is working

  # run script to collect pictures
  ./collectPictures.sh

  # run script to create an animated gif from the collected pictures
  echo "creating animated gif"
  animatedFile=`./createAnimatedGif.sh $hour`

  # upload animated gif to FTP
  ./uploadToFTP.sh $animatedFile
else
  # surfcampi does nothing for 1min in case we want to ssh it
  echo "surfcampi does not have anything to do. Waiting doing nothing for a 1min."
  sleep 60
fi

# disconnect from 3G
#echo "disconnecting from 3G"
#sudo ./sakis3g disconnect

# shutdown surfcampi
sudo halt

exit 0
