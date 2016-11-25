#!/bin/bash

echo "backup"
mkdir -p ./gps-bckp/home/$USER/.config/upstart
mkdir -p ./gps-bckp/storage/ca-st-ro/bin
cp /storage/ca-st-ro/bin/gpsmon ./gps-bckp/storage/ca-st-ro/bin/
cp /home/$USER/.config/upstart/gpsmon.conf ./gps-bckp/home/$USER/.config/upstart/

echo "remove"
killall gpsmon

rm /storage/ca-st-ro/bin/gpsmon
rm /home/$USER/.config/upstart/gpsmon.conf

echo "done"
