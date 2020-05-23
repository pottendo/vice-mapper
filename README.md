# vice-mapper
Welcome to a little GUI to manage maps out of emulator
screenshots. This was inspired by Overdoc's dream to have an automap
feature within retro-machine emulators, such as Vice, where a player
just navigates through a game and all 'new' screens are somehow
mapped.

Limitations:
- Automation was postponed until we find some clever idea to detect
'new' screens within a game.
- Scrolling maps would be nice but require even more cleverness...

However, for games like "Blinky's Scary School" or "Cauldron II" a
mapping feature can be very handy.

The map is composed ouf of individual screenshots which must follow a
specific naming convention:
  some-prefix-XXxYY.png
  
'XXxYY' provides the coordinates within the map. 

e.g.
  vice-screen-50x50.png
  
PNG file-format is preferred for now as some elements are hardcoded
within the current code.

# Integration

Usage:
$ mapper my-game-map/*.png

[Demo - Blinkey's Scary School Map](https://github.com/pottendo/vice-mapper/blob/master/doc/Demo1-BlinkeyMap.png)

Use drag&drop to move tiles around.


# build vice
../vice-emu-code/vice/configure -C --enable-native-tools --enable-native-gtk3ui --host=mingw32-gtk3 --enable-x64 2>&1 |tee mycfg-64.log
export MINGW_PREFIX=/mingw32
make -j12 2>&1 | tee make-32.log
make bindist

copy .DLLs for mapper - check with ntldd mapper.exe
