#!/bin/bash

echo "backup"
mkdir -p ./player-bckp/home/$USER/.config/upstart
mkdir -p ./player-bckp/storage/ca-st-ro/bin
mkdir -p ./player-bckp/storage/ca-st-ro/assets
cp /storage/ca-st-ro/bin/player ./player-bckp/storage/ca-st-ro/bin/
cp /home/$USER/.config/upstart/player.conf ./player-bckp/home/$USER/.config/upstart/
cp /storage/ca-st-ro/assets/* ./player-bckp/storage/ca-st-ro/assets/

echo "install"
killall player
tar xvf pkg.tar.gz
#cp -r pkg/storage/* /storage/
#cp -r pkg/ubuntuhome/* /home/rborisov/
rsync -av pkg/storage/ /storage/
rsync -av pkg/ubuntuhome/ /home/$USER/
rm -r pkg
killall player

echo "done"
