#!/bin/bash
./collectPictures.sh
animatedFile=`./createAnimatedGif.sh`
lftp -e "put -O /surfcampi/ $animatedFile; bye" -u $USER,$PASS ftp.users.on.net
exit 0
