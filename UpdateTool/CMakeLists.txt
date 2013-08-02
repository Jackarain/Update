include_directories(ResizableLib)

add_definitions(-DUNICODE)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS system thread date_time filesystem chrono locale regex)
include_directories(${Boost_INCLUDE_DIRS})

find_package(WTL REQUIRED)
include_directories(${WTL_INCLUDE_DIRS})

set(UPDATE_TOOL_SRC UpdateTool.cpp MainDlg.cpp stdafx.cpp Compress.cpp CheckFile.cpp AboutDlg.cpp)
set(UPDATE_TOOL_HDR AboutDlg.h CheckFile.h Compress.h MainDlg.h resource.h stdafx.h)
set(UPDATE_TOOL_RSC UpdateTools.rc)

add_executable(UpdateTool WIN32 ${UPDATE_TOOL_SRC} ${UPDATE_TOOL_HDR} ${UPDATE_TOOL_RSC})
target_link_libraries(UpdateTool ${libupdate} ${Boost_LIBRARIES} zlib tinyxml md5)