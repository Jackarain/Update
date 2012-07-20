//
// uncompress.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2011 Jack (jack.wgm@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// path LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// * $Id: uncompress.hpp 49 2011-07-15 03:00:34Z jack $
//

#ifndef __UNCOMPRESS_H__
#define __UNCOMPRESS_H__

#include <stdio.h>
#include <zlib.h>
#include <unzip.h>

#pragma once

#ifdef WIN32
void change_file_date(const char *filename, uLong dosdate, tm_unz tmu_date);
#endif // WIN32

int mymkdir(const char* dirname);
int makedir(char *newdir);
int do_extract_currentfile(unzFile uf, const int* popt_extract_without_path,
   int* popt_overwrite, const char* extract_path, const char* password, int opt_close = 1);
int do_extract(unzFile uf, int opt_extract_without_path, 
   int opt_overwrite, const char* extract_path, const char* password);
int do_extract_onefile(unzFile uf, const char* filename, 
   int opt_extract_without_path, int opt_overwrite, const char* extract_path, const char* password);
int do_extract_gz(const char* file);
int do_extract_zip(const char* file, const char* extract_path = NULL);

#endif // __UNCOMPRESS_H__
