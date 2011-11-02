#!/bin/bash
VERSION=`head -1 debian/changelog | sed 's/.*(\(.*\)).*/\1/'`

# Compile translations
./translate.sh

# Create Apple Disk iMaGe
cd dmg
./create-dmg --volname "Q Light Controller $VERSION" \
	     --background create-dmg/background.png \
	     --window-size 200 200 \
	     --icon-size 128 --icon "qlc" 96 64 \
	     QLC-$VERSION.dmg \
	     ~/QLC.app
cd ..
