#!/bin/bash

chmod +x ./tools/premake/bin/linux/premake5
./tools/premake/bin/linux/premake5 --file=code/premake5.lua gmake2
