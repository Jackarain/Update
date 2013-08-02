set(Boost_USE_STATIC_LIBS ON)

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIR})

find_package(WTL REQUIRED)
include_directories(${WTL_INCLUDE_DIRS})

include_directories(../libupdate)

add_executable(Updater WIN32 main.cpp MainDlg.cpp stdafx.cpp MainDlg.h resource.h stdafx.h Update.h Update.rc)
target_link_libraries(Updater libupdate zlib tinyxml md5)