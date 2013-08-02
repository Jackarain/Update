find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(LIBUPDATE_SRC parser_http_date.cpp uncompress.cpp updater.cpp url.cpp)
set(LIBUPDATE_HDR uncompress.hpp updater.hpp url.hpp)
add_library(libupdate STATIC ${LIBUPDATE_SRC} ${LIBUPDATE_HDR})