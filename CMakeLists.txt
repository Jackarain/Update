project(AVUpdate)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

set(BUILD_SHARED_LIBS OFF)
include_directories(common/md5)
add_subdirectory(common/md5)

## tinyxml
include_directories(common/tinyxml)
add_subdirectory(common/tinyxml)

## zlib
include_directories(common/zlib-1.2.5)
add_subdirectory(common/zlib-1.2.5)
#for zconf.h
include_directories(${zlib_BINARY_DIR})

add_subdirectory(libupdate)
add_subdirectory(Updater)
add_subdirectory(UpdateTool)