#ifndef __INC_H__
#define __INC_H__

// C++头文件.
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <algorithm>

// boost头文件.
#include <boost/regex.hpp>
#include <boost/cstdint.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

using boost::asio::ip::tcp;
using namespace boost::filesystem;

// XML头文件.
#include <tinyxml.h>
#include <tinystr.h>

// md5头文件.
#include <md5.h>

// 引入库文件.
#ifdef _DEBUG
#pragma comment(lib, "tinyxmld.lib")
#else
#pragma comment(lib, "tinyxml.lib")
#endif // _DEBUG

#endif // __INC_H__
