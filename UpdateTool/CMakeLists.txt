include_directories(ResizableLib)

add_definitions(-DUNICODE)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS system thread date_time filesystem chrono locale regex)
include_directories(${Boost_INCLUDE_DIRS})

find_package(WTL)
include_directories(${WTL_INCLUDE_DIRS})

add_executable(UpdateTool WIN32 UpdateTool.cpp MainDlg.cpp stdafx.cpp Compress.cpp CheckFile.cpp AboutDlg.cpp)
target_link_libraries(UpdateTool ${libupdate} ${Boost_LIBRARIES} zlib tinyxml md5)