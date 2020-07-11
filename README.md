# vice-mapper
Welcome to a little GUI to create graphical maps out of emulator
screenshots - tested with Vice. This was inspired by Overdoc's dream
to have an automap feature within retro-machine emulators, such as
Vice, where a player just navigates through a game and all 'new'
screens are somehow mapped.

Limitations:
- Automation was postponed until we find some clever idea to detect
'new' screens within a game.
- Scrolling maps would be nice but require even more cleverness...

However, for games like "Blinky's Scary School" or "Cauldron II" a
mapping feature can be very handy.

The map is composed ouf of individual screenshots which follow a
specific naming convention:
  vice-screen--XXxYY.png
'XXxYY' provides the coordinates within the map. 

e.g.
  vice-screen-50x50.png
  
bitmaps not following this convention are *unplaced* and can be
placed. Once a map is saved, the filenames are renamed.

Exported maps should follow the convention to be named as the
directory holding all individual tiles:
  .../barnsley-badger/barnsley-badger.png
These files are **ignored** and not shot shown as *unplaced* to avoid
cluttering up the list of unplaced tiles.
  
PNG file-format is preferred for now as some elements are hardcoded
within the current code.
When saving a map tiles (ie. screenshot) are renamed according to
their coordinates otherwise are kept unmodified, so a the map finally
consists of a set of properly named screenshots in one directory. 

![Cybernoid](https://github.com/pottendo/vice-mapper/blob/master/doc/Demo4-Cybernoid.png)

# Integration with Vice & Usage

Vice (current trunk) supports a one-keypress screenshot by hitting *Pause*.
(equivalent to *Alt-Shift-F12*)

Usage:
$ mapper [game-map directory]

[Demo - Blinkey's Scary School
Map](https://github.com/pottendo/vice-mapper/blob/master/doc/Demo1-BlinkeyMap.png)<br>

[Cybernoid](https://github.com/pottendo/vice-mapper/blob/master/doc/Demo3-Cybernoid.png)<br>

Use drag&drop to move tiles around. <br>
Use SPACE+mousemove within the map to pan around the map. 
Use right mouse button to show popup menu on tiles.
A debug log is collected and saved at exit in file vice-mapper.log
(Backup file vice-mapper.log~).

A map is stored in one directory using a standard filename convention
for each placed (and even unplaced) tiles.
Saving a map therefore renames tiles to code the coordinates into the
file-names starting with a standard prefix *vice-screen-*. Even
temporary filenames are created to save unplaced tiles if needed.
A tiny configuration file is generated *vice-screen-.vsm* to store
zoom and crop parameters.

Deleted tiles are moved to the trash-bin (if supported).

# Build

Make sure you have the respective gtk+* and the gtkmm* development
packages installed.

$ cmake .<br>
$ make

For windows builds (msys2) make sure to set your PATH: 
e.g. $ export PATH=/mingw64/bin:$PATH

# Limits

- The simple map file format using standard names implies that there
  is only **one map per directory** possible.
- Note that some conventions for the file-names exist! see above!
- Currently the design is not optimized for speed & memory footprint
- Maps dimension limits are hard-coded to 100x100.
- memory limit is not handled at all and just kills the app when reached<br>
  Especially on 32bit systems the map may only support 50x50 (used) tiles.
  Tiles beyond/below the current min/max coordinates aren't managed,
  so won't need space.
- Handling of different tile-sizes in one map has some flaws when
  exporting
- Printing always scales to one page, may lead to distortions.
  Use export and your favorite printing program to print .png files.


# TODOs

UI-stuff<br>
- insert row/columns more easily
- better support moving around tiles on map to make space in case a border is reached 
- maybe allow individual crop vals for tiles

Internals:<br>
- adjust maximum map dynamically depending on 32/64 bit
- Properly manage 'gui.glade' path.
- encapsule some globals
- refactor code to follow consistent conventions
- fix bugs

Know issues:<br>
- If trash-bin is unsupported deleted tiles remain -> will cause map
  inconsistencies
- If memory limit is reached (32bit system), mapper crashes without
  warning.
- Potential, maybe GTK+ related memory leaks detected with valgrind.
- Printer dialog on Linux won't let one set page format, etc. (Ubuntu
  19.10), Print Preview behaves strange (print button won't do
  anything from the preview popup) -> use *print button* directly from
  print dialog instead.
  any tips are welcome!

# Copyright notice

(C) 2020 by pottendo

vice-mapper is release under the terms of GPL V3
Refer to gpl-3.0.txt.

# Build vice for Windoze

This is just a brief reminder how to build Vice/Gtk+ on windoze using
the msys2 packages (https://www.msys2.org/)

$ ../vice-emu-code/vice/configure -C --enable-native-tools
--enable-native-gtk3ui --host=mingw32-gtk3 --enable-x64 2>&1 |tee
mycfg-64.log<br>
$ export MINGW_PREFIX=/mingw64<br>
$ make -j12 2>&1 | tee make-32.log<br>
$ make bindist<br>


