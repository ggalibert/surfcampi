#!/bin/bash

# set lftp otions
incomingDir=/surfcampi/
host=ftp.users.on.net

echo "uploading file to FTP"
lftp -e "put -O $incomingDir $1; bye" -u $USER,$PASS $host

exit 0
