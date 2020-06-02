# Set project
cmake_minimum_required(VERSION 3.10)
project(mapper)

# Configure project paths
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
#set(CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)

set (VER 1.0)
# Find dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED gtk+-3.0 gtkmm-3.0)
link_directories(${GTK_LIBRARY_DIRS})

if(NOT DEFINED ENV{MINGW_PREFIX})

include_directories (${GTK_INCLUDE_DIRS})
set (ARCH unix)
set (ARCHLIBS -rdynamic)

else()
# msys2 build
set (ARCH $ENV{MINGW_PREFIX})
string (REPLACE "/" "" ARCH ${ARCH})
set (MAPPER_VERSION ${ARCH}-1.0)

# for some reason "include_directories (${GTK_INCLUDE_DIRS})" results in a wrong CXX_INCLUDES line
# by preending each -I<some correct path> by -I/my/current/dir/<some correct path>

if(${ARCH} STREQUAL "mingw32")
include_directories (${GTK_INCLUDE_DIRS})		# this unfortunately doesn't work in my setup
set (CXX_INCLUDES -ID:/Programme/msys64/mingw32/include/gtkmm-3.0 -ID:/Programme/msys64/mingw32/lib/gtkmm-3.0/include -ID:/Programme/msys64/mingw32/include/atkmm-1.6 -ID:/Programme/msys64/mingw32/include/gdkmm-3.0 -ID:/Programme/msys64/mingw32/lib/gdkmm-3.0/include -ID:/Programme/msys64/mingw32/include/giomm-2.4 -ID:/Programme/msys64/mingw32/lib/giomm-2.4/include -ID:/Programme/msys64/mingw32/include/pangomm-1.4 -ID:/Programme/msys64/mingw32/lib/pangomm-1.4/include -ID:/Programme/msys64/mingw32/include/glibmm-2.4 -ID:/Programme/msys64/mingw32/lib/glibmm-2.4/include -ID:/Programme/msys64/mingw32/include/gtk-3.0 -ID:/Programme/msys64/mingw32/include/cairo -ID:/Programme/msys64/mingw32/include -ID:/Programme/msys64/mingw32/include/pango-1.0 -ID:/Programme/msys64/mingw32/include/fribidi -ID:/Programme/msys64/mingw32/include/atk-1.0 -ID:/Programme/msys64/mingw32/include/cairomm-1.0 -ID:/Programme/msys64/mingw32/lib/cairomm-1.0/include -ID:/Programme/msys64/mingw32/include/pixman-1 -ID:/Programme/msys64/mingw32/include/freetype2 -ID:/Programme/msys64/mingw32/include/harfbuzz -ID:/Programme/msys64/mingw32/include/libpng16 -ID:/Programme/msys64/mingw32/include/sigc++-2.0 -ID:/Programme/msys64/mingw32/lib/sigc++-2.0/include -ID:/Programme/msys64/mingw32/include/gdk-pixbuf-2.0 -ID:/Programme/msys64/mingw32/include/glib-2.0 -ID:/Programme/msys64/mingw32/lib/glib-2.0/include )

else ()
# include_directories (${GTK_INCLUDE_DIRS})		# this unfortunately doesn't work in my setup
set (CXX_INCLUDES -ID:/Programme/msys64/mingw64/include/gtkmm-3.0 -ID:/Programme/msys64/mingw64/lib/gtkmm-3.0/include -ID:/Programme/msys64/mingw64/include/atkmm-1.6 -ID:/Programme/msys64/mingw64/include/gdkmm-3.0 -ID:/Programme/msys64/mingw64/lib/gdkmm-3.0/include -ID:/Programme/msys64/mingw64/include/giomm-2.4 -ID:/Programme/msys64/mingw64/lib/giomm-2.4/include -ID:/Programme/msys64/mingw64/include/pangomm-1.4 -ID:/Programme/msys64/mingw64/lib/pangomm-1.4/include -ID:/Programme/msys64/mingw64/include/glibmm-2.4 -ID:/Programme/msys64/mingw64/lib/glibmm-2.4/include -ID:/Programme/msys64/mingw64/include/gtk-3.0 -ID:/Programme/msys64/mingw64/include/cairo -ID:/Programme/msys64/mingw64/include -ID:/Programme/msys64/mingw64/include/pango-1.0 -ID:/Programme/msys64/mingw64/include/fribidi -ID:/Programme/msys64/mingw64/include/atk-1.0 -ID:/Programme/msys64/mingw64/include/cairomm-1.0 -ID:/Programme/msys64/mingw64/lib/cairomm-1.0/include -ID:/Programme/msys64/mingw64/include/pixman-1 -ID:/Programme/msys64/mingw64/include/freetype2 -ID:/Programme/msys64/mingw64/include/harfbuzz -ID:/Programme/msys64/mingw64/include/libpng16 -ID:/Programme/msys64/mingw64/include/sigc++-2.0 -ID:/Programme/msys64/mingw64/lib/sigc++-2.0/include -ID:/Programme/msys64/mingw64/include/gdk-pixbuf-2.0 -ID:/Programme/msys64/mingw64/include/glib-2.0 -ID:/Programme/msys64/mingw64/lib/glib-2.0/include )

endif() # MINGW32 or 64

# Bindist
add_custom_target (bindist 
COMMAND ./make-bindist-win32.sh vice-mapper-${MAPPER_VERSION}
COMMAND rm -f vice-mapper-${MAPPER_VERSION}.zip
COMMAND zip -r vice-mapper-${MAPPER_VERSION}.zip vice-mapper-${MAPPER_VERSION}
DEPENDS mapper.exe gui.glade
)	

endif()

message (STATUS "building for ${ARCH}...")

string(REPLACE "." "_" VERSION_CSTR ${VER})
add_compile_options(${CXX_INCLUDES} ${GTK_CFLAGS_OTHER} -DMAPPER_VERSION=\"${VERSION_CSTR}\" -g -Wall -Wno-deprecated-declarations -I.)
link_directories (${GTK_LIBRARY_DIRS})
#set(LIBRARIES ${LIBRARIES} ${GTK3_LIBRARIES})
#set(FLAGS "-I${GTK3_INCLUDE_DIRS}")
#message(STATUS "Flags: ${GTK_INCLUDE_DIRS} ${GTK_CFLAGS_OTHER}")
#string(REPLACE ";" " -I" FLAGS "${FLAGS}")
#set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} ${GTK3_FLAGS})

# ${PROJECT_SOURCE_DIR}/dinphil-gtk.cc 
set(SRC main.cc myarea.cc map-window.cc map-controls.cc dialogs.cc)

# Compile
add_executable(mapper ${SRC})
target_link_libraries(mapper ${GTK_LIBRARIES} ${ARCHLIBS})


