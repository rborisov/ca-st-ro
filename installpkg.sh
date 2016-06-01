#!/bin/bash

killall player
tar xvf pkg.tar.gz
#cp -r pkg/storage/* /storage/
#cp -r pkg/ubuntuhome/* /home/rborisov/
rsync -av pkg/storage/ /storage/
rsync -av pkg/ubuntuhome/ /home/rborisov/
#/storage/ca-st-ro/bin/player &
