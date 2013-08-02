find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_library(libupdate STATIC parser_http_date.cpp uncompress.cpp updater.cpp url.cpp)