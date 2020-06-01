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
When saving a map tiles (ie. screenshot) are renamed according to
their coordinates otherwise are kept unmodified, so a the map finally
consists of a set of properly named screenshots in one directory. 

![Cybernoid](https://github.com/pottendo/vice-mapper/blob/master/doc/Demo4-Cybernoid.png)

# Integration with Vice & Usage

So far a tiny patch is needed to enable vice to feature a
'one-keystroke' screenshot. For now this is key is *Pause* on normal
keyboards. If this key is pressed, vice generated a screenshot of the
actual screen in the working directory of Vice. A specific name
convention is used: *vice-screen--1xYYY.png*
where YY is a running number starting from 01.
`-1' is the hint for the mapper that this tile is not placed yet.
Numbering starts from 01 every time, vice starts.

Here the patch: [vice-mapper patch for hotkey 'Pause'](https://github.com/pottendo/vice-mapper/blob/master/vice-mapper.patch)
to be applied in <vice-src>/src/arch/gtk3
$ patch -p0 < vice-mapper.patch

Vice features a screenshot hotkey by default `Alt-Shift-F12' already,
but I found it to be a bit unhandy during gaming; and the mapper
naming convention is not (yet) followed.

Usage:
$ mapper [game-map directory]

[Demo - Blinkey's Scary School
Map](https://github.com/pottendo/vice-mapper/blob/master/doc/Demo1-BlinkeyMap.png)<br>

[Cybernoid](https://github.com/pottendo/vice-mapper/blob/master/doc/Demo3-Cybernoid.png)<br>

Use drag&drop to move tiles around. <br>
Use SPACE+Mousemove within the map to pan around the map. 

# Build

Make sure you have the respective gtk+* and the gtkmm* development
packages installed.

$ cmake .<br>
$ make

For windows builds (msys2) make sure to set your PATH: 
e.g. $ export PATH=/mingw32/bin:$PATH

# Limits

- Currently the design is not optimized for speed & memory footprint
- Maps dimension limits are hard-coded to 100x100.
- memory limit is not handled at all and just kills the app when reached<br>
  Especially on 32bit systems the map may only support 50x50 (used) tiles.
  Tiles beyond/below the current min/max coordinates aren't managed, so won't need space.

# TODOs

UI-stuff<br>
- insert row/columns more easily
- support moving around tiles on map to make space in case a border is reached 
- maybe allow individual crop vals for tiles
- allow other name than def_basename (i.e. vice-screen-)
- support multiple maps in parallel (notpads?), encapsule some globals
- highlight tiles when cursor is hovering over.

Internals:
- log-window
- adjust maximum map dynamically depending on 32/64 bit
- refactor code to follow consistent conventions
- put in GPL license

# Build vice for Windoze

This is just a brief reminder how to build Vice/Gtk+ on windoze using
the msys2 packages (https://www.msys2.org/)

$ ../vice-emu-code/vice/configure -C --enable-native-tools
--enable-native-gtk3ui --host=mingw32-gtk3 --enable-x64 2>&1 |tee
mycfg-64.log<br>
$ export MINGW_PREFIX=/mingw32<br>
$ make -j12 2>&1 | tee make-32.log<br>
$ make bindist<br>


