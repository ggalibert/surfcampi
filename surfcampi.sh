#!/bin/bash
./collectPictures.sh
animatedFile=`./createAnimatedGif.sh`
lftp -e "put -O /surfcampi/ $animatedFile; bye" -u parliament,phtj4pdwj ftp.users.on.net
exit 0
