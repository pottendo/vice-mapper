# Set project
cmake_minimum_required(VERSION 3.0)
project(mapper)

# Configure project paths
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
#set(CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)

# Find dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0 gtkmm-3.0)
link_directories(${GTK3_LIBRARY_DIRS})
include_directories (${GTK3_INCLUDE_DIRS})
add_compile_options(${GTK3_CFLAGS_OTHER} -g -Wall -Wno-deprecated-declarations -I.)
link_directories (${GTK3_LIBRARY_DIRS})

#set(LIBRARIES ${LIBRARIES} ${GTK3_LIBRARIES})
#set(FLAGS "-I${GTK3_INCLUDE_DIRS}")
message(STATUS "Flags: ${GTK3_INCLUDE_DIRS} ${GTK3_CFLAGS_OTHER}")
#string(REPLACE ";" " -I" FLAGS "${FLAGS}")
#set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} ${GTK3_FLAGS})

# ${PROJECT_SOURCE_DIR}/dinphil-gtk.cc 
set(SRC main.cc myarea.cc map-window.cc map-controls.cc dialogs.cc)

# Compile
add_executable(mapper ${SRC})
target_link_libraries(mapper ${GTK3_LIBRARIES} -lstdc++fs)
