#!/bin/bash

### Ubuntu
#sudo apt install libqt53drender5
#export QT_DIR=/usr/lib/x86_64-linux-gnu
./tools/premake/bin/linux/premake5 --file=code/premake5.lua gmake2
