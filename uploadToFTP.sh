#!/bin/bash

# set lftp otions
incomingDir=/surfcampi/
host=ftp.users.on.net

lftp -e "put -O $incomingDir $1; bye" -u $USER,$PASS $host

exit 0
