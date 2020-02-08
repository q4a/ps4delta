#!/bin/bash

echo "Controller.prototype.ObligationsPageCallback = function()
{
    var page = gui.pageWidgetByObjectName(\"ObligationsPage\");
    page.obligationsAgreement.setChecked(true);
    page.completeChanged();
    gui.clickButton(buttons.NextButton);
}" >> inst.qs
wget http://master.qt.io/archive/online_installers/3.2/qt-unified-linux-x64-3.2.1-online.run -o qt.run
chmod +x qt.run
./qt.run --verbose --script inst.qs

sudo apt-get update && sudo apt install -y qtbase5-dev libxcursor-dev libxrandr-dev libxinerama-dev libxi-dev

chmod +x ./tools/premake/bin/linux/premake5
./tools/premake/bin/linux/premake5 --file=code/premake5.lua gmake2
