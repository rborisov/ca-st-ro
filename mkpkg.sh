#!/bin/bash

mkdir -p pkg/storage/ca-st-ro/assets
mkdir -p pkg/storage/ca-st-ro/bin

cp -r etc/* pkg/
cp assets/ajax-throbber.gif pkg/storage/ca-st-ro/assets/
cp assets/bkbg.jpg pkg/storage/ca-st-ro/assets/
cp assets/player.glade pkg/storage/ca-st-ro/assets
cp build/player pkg/storage/ca-st-ro/bin/

tar czf pkg.tar.gz pkg
