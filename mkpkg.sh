#!/bin/bash

mkdir -p pkg/ca-st-ro/assets
mkdir -p pkg/ca-st-ro/bin

cp assets/bkbg.jpg pkg/ca-st-ro/assets/
cp assets/player.glade pkg/ca-st-ro/assets
cp build/player pkg/ca-st-ro/bin/

tar czf pkg.tar.gz pkg
