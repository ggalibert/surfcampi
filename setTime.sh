#!/bin/sh -e
sudo /bin/bash "echo ds1374 0x68 > /sys/class/i2c-adapter/i2c-1/new_device"
 
# Check for an IP address
_IP=$(hostname -I) || true
_HOSTNAME=$(hostname) || true
 
if [ "$_IP" ]; then
    # We have a network, set the RTC from the system time.
    printf "\nSetting hardware clock from system time\n"
    /sbin/hwclock -wu
 
    printf "\n%s IP address is %s\n" "$_HOSTNAME" "$_IP"
else
    # No network, set the system time from the RTC
    printf "\nSetting system time from hardware clock\n"
    /sbin/hwclock -s
fi

exit 0
