cmake_minimum_required(VERSION 2.4.4)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)

project(zlib C)

if(MSVC)
    set(CMAKE_DEBUG_POSTFIX "d")
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR})

#============================================================================
# zlib
#============================================================================

set(ZLIB_PUBLIC_HDRS
    zconf.h
    zlib.h
	unzip.h
	zip.h
)
set(ZLIB_PRIVATE_HDRS
    crc32.h
    deflate.h
    gzguts.h
    inffast.h
    inffixed.h
    inflate.h
    inftrees.h
	ioapi.h
	ioinbuffer.h
	iowin32.h
	mztools.h
    trees.h
    zutil.h
)
set(ZLIB_SRCS
    adler32.c
    compress.c
    crc32.c
    deflate.c
    gzclose.c
    gzlib.c
    gzread.c
    gzwrite.c
    inflate.c
    infback.c
    inftrees.c
    inffast.c
	ioapi.c
	ioinbuffer.c
	iowin32.c
	mztools.c
    trees.c
    uncompr.c
	unzip.c
    zutil.c
	zip.c
)


add_library(zlib ${ZLIB_SRCS} ${ZLIB_PUBLIC_HDRS})