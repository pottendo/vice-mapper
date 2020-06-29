#
# This file is part of vice-mapper.
#
# vice-mapper is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# vice-mapper is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with vice-mapper.  If not, see <https://www.gnu.org/licenses/>.
#

# Set project
cmake_minimum_required(VERSION 3.10)
project(mapper)

set (VER 1.5.0)

set(SRC main.cc VmTile.cc VmMap.cc VmMapControls.cc dialogs.cc)

if (DEFINED ENV{CROSS})
  set(ENV{PKG_CONFIG_PATH} "/mingw64/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
  set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)
  set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc-posix)
  set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++-posix)
  set(ARCH mingw64cross)
  message (STATUS "cross build ${ARCH}, $ENV{PKG_CONFIG_PATH}, ${CMAKE_CXX_COMPILER}, ${CMAKE_SOURCE_DIR}, ${CMAKE_CURRENT_BINARY_DIR}")
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(GTK REQUIRED gtk+-3.0 gtkmm-3.0)
  include_directories (${GTK_INCLUDE_DIRS})
  set (MAPPER_VERSION ${ARCH}-${VER})
  set (PROGRAMEXE mapper.exe)

  add_custom_target (bindist 
    COMMAND ${CMAKE_SOURCE_DIR}/make-bindist-win32-cross.sh vice-mapper-${MAPPER_VERSION} true ${CMAKE_CXX_COMPILER} ${CMAKE_SOURCE_DIR}
    COMMAND rm -f vice-mapper-${MAPPER_VERSION}.zip
    COMMAND zip -r vice-mapper-${MAPPER_VERSION}.zip vice-mapper-${MAPPER_VERSION}    )	
endif() # CROSS

if (NOT DEFINED ENV{MINGW_PREFIX} AND NOT DEFINED ENV{CROSS})
  # linux native build
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(GTK REQUIRED gtk+-3.0 gtkmm-3.0)
  include_directories (${GTK_INCLUDE_DIRS})
  set (ARCH unix)
  if (NOT DEFINED ENV{CROSS})
    set (ARCHLIBS -rdynamic)
  endif() 
  set (MAPPER_VERSION ${ARCH}-${VER})
  set (PROGRAMEXE mapper)
endif() # linux 

if(DEFINED ENV{MINGW_PREFIX})
  # msys2 build
  set (ARCH $ENV{MINGW_PREFIX})
  string (REPLACE "/" "" ARCH ${ARCH})
  set (MAPPER_VERSION ${ARCH}-${VER})

  if(${ARCH} STREQUAL "mingw32")
    # for some reason "include_directories (${GTK_INCLUDE_DIRS})" results in a wrong CXX_INCLUDES line
    # by preending each -I<some correct path> by -I/my/current/dir/<some correct path>
    #include_directories (${GTK_INCLUDE_DIRS})		# this unfortunately doesn't work in my setup
    set (CXX_INCLUDES -ID:/Programme/msys64/mingw32/include/gtkmm-3.0 -ID:/Programme/msys64/mingw32/lib/gtkmm-3.0/include -ID:/Programme/msys64/mingw32/include/atkmm-1.6 -ID:/Programme/msys64/mingw32/include/gdkmm-3.0 -ID:/Programme/msys64/mingw32/lib/gdkmm-3.0/include -ID:/Programme/msys64/mingw32/include/giomm-2.4 -ID:/Programme/msys64/mingw32/lib/giomm-2.4/include -ID:/Programme/msys64/mingw32/include/pangomm-1.4 -ID:/Programme/msys64/mingw32/lib/pangomm-1.4/include -ID:/Programme/msys64/mingw32/include/glibmm-2.4 -ID:/Programme/msys64/mingw32/lib/glibmm-2.4/include -ID:/Programme/msys64/mingw32/include/gtk-3.0 -ID:/Programme/msys64/mingw32/include/cairo -ID:/Programme/msys64/mingw32/include -ID:/Programme/msys64/mingw32/include/pango-1.0 -ID:/Programme/msys64/mingw32/include/fribidi -ID:/Programme/msys64/mingw32/include/atk-1.0 -ID:/Programme/msys64/mingw32/include/cairomm-1.0 -ID:/Programme/msys64/mingw32/lib/cairomm-1.0/include -ID:/Programme/msys64/mingw32/include/pixman-1 -ID:/Programme/msys64/mingw32/include/freetype2 -ID:/Programme/msys64/mingw32/include/harfbuzz -ID:/Programme/msys64/mingw32/include/libpng16 -ID:/Programme/msys64/mingw32/include/sigc++-2.0 -ID:/Programme/msys64/mingw32/lib/sigc++-2.0/include -ID:/Programme/msys64/mingw32/include/gdk-pixbuf-2.0 -ID:/Programme/msys64/mingw32/include/glib-2.0 -ID:/Programme/msys64/mingw32/lib/glib-2.0/include )

  else ()
    # include_directories (${GTK_INCLUDE_DIRS})		# this unfortunately doesn't work in my setup
    set (CXX_INCLUDES -ID:/Programme/msys64/mingw64/include/gtkmm-3.0 -ID:/Programme/msys64/mingw64/lib/gtkmm-3.0/include -ID:/Programme/msys64/mingw64/include/atkmm-1.6 -ID:/Programme/msys64/mingw64/include/gdkmm-3.0 -ID:/Programme/msys64/mingw64/lib/gdkmm-3.0/include -ID:/Programme/msys64/mingw64/include/giomm-2.4 -ID:/Programme/msys64/mingw64/lib/giomm-2.4/include -ID:/Programme/msys64/mingw64/include/pangomm-1.4 -ID:/Programme/msys64/mingw64/lib/pangomm-1.4/include -ID:/Programme/msys64/mingw64/include/glibmm-2.4 -ID:/Programme/msys64/mingw64/lib/glibmm-2.4/include -ID:/Programme/msys64/mingw64/include/gtk-3.0 -ID:/Programme/msys64/mingw64/include/cairo -ID:/Programme/msys64/mingw64/include -ID:/Programme/msys64/mingw64/include/pango-1.0 -ID:/Programme/msys64/mingw64/include/fribidi -ID:/Programme/msys64/mingw64/include/atk-1.0 -ID:/Programme/msys64/mingw64/include/cairomm-1.0 -ID:/Programme/msys64/mingw64/lib/cairomm-1.0/include -ID:/Programme/msys64/mingw64/include/pixman-1 -ID:/Programme/msys64/mingw64/include/freetype2 -ID:/Programme/msys64/mingw64/include/harfbuzz -ID:/Programme/msys64/mingw64/include/libpng16 -ID:/Programme/msys64/mingw64/include/sigc++-2.0 -ID:/Programme/msys64/mingw64/lib/sigc++-2.0/include -ID:/Programme/msys64/mingw64/include/gdk-pixbuf-2.0 -ID:/Programme/msys64/mingw64/include/glib-2.0 -ID:/Programme/msys64/mingw64/lib/glib-2.0/include )

  endif() # MINGW32 or 64

  # Bindist
  add_custom_target (bindist 
    COMMAND ./make-bindist-win32.sh vice-mapper-${MAPPER_VERSION} false
    COMMAND rm -f vice-mapper-${MAPPER_VERSION}.zip
    COMMAND zip -r vice-mapper-${MAPPER_VERSION}.zip vice-mapper-${MAPPER_VERSION}
    DEPENDS mapper.exe gui.glade
    )	
  set (PROGRAMEXE mapper.exe)
endif()

message (STATUS "building for ${ARCH}...")

add_compile_options(${CXX_INCLUDES} ${GTK_CFLAGS_OTHER} -DMAPPER_VERSION=\"${VER}\" -g -Wall -Wno-deprecated-declarations -I.)
link_directories (${GTK_LIBRARY_DIRS})

# Compile
add_executable(${PROGRAMEXE} ${SRC})
target_link_libraries(${PROGRAMEXE} ${GTK_LIBRARIES} ${ARCHLIBS})


