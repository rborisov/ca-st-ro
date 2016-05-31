#!/bin/bash

killall player
tar xvf pkg.tar.gz
cp -r pkg/* /storage/
/storage/ca-st-ro/bin/player &
