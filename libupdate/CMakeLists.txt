find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(LIBUPDATE_SRC parser_http_date.cpp uncompress.cpp updater.cpp)
set(LIBUPDATE_HDR uncompress.hpp updater.hpp)
add_library(libupdate STATIC ${LIBUPDATE_SRC} ${LIBUPDATE_HDR})
