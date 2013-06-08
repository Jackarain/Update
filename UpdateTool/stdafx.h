// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once

// Change these values to use different versions
#define WINVER		0x0500
#define _WIN32_WINNT	0x0501
#define _WIN32_IE	0x0501
#define _RICHEDIT_VER	0x0200

#define _WTL_NO_UNION_CLASSES
#define _WTL_NO_CSTRING
#define _WTL_NO_WTYPES
#define _WTL_NEW_PAGE_NOTIFY_HANDLERS

#include <atlbase.h>
#include <atltypes.h>
#include <atlwin.h>
#include <atlcoll.h>
#include <atlstr.h>

#include <atlapp.h>

extern CAppModule _Module;

#include <atlcom.h>
#include <atlhost.h>
#include <atlwin.h>
#include <atlctl.h>

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>
#include <atlsplit.h>
#include <atlctrlx.h>
#include <atlstr.h>
#include <atlscrl.h>
#include <atlddx.h>
#include <atlmisc.h>
#include <atlgdi.h>
#include <atlhttp.h>

#include <winver.h>
#pragma comment(lib, "Version.lib")

#if defined _M_IX86
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#define EXPORTS_STATIC


#include <vector>
#include <string>
#include <algorithm>


#include <iostream>
#include <boost/thread/xtime.hpp>
#include <boost/thread/once.hpp>
#include <boost/utility.hpp>
#include <cassert>

// #include <boost/throw_exception.hpp>
// #define BOOST_FILESYSTEM_VERSION 2
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>

#include <boost/thread/thread.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/utility.hpp>
#include <boost/test/unit_test.hpp>
#define DEFAULT_EXECUTION_MONITOR_TYPE execution_monitor::use_sleep_only
#include <libs/thread/test/util.inl>

// #include <Singleton.h>
// #pragma comment(lib, "loki.lib")

#include "ResizableLib/ResizableLib.h"

#include <md5.hpp>

#include <zlib.h>
#include <zip.h>

// #ifdef WIN32
// #define USEWIN32IOAPI
// #include "iowin32.h"
// #endif

// #define USE_BOOST_THREAD

// XML头文件.
#include <tinyxml.h>
#include <tinystr.h>

// using namespace Loki;
using namespace boost;
using namespace boost::filesystem;

// #define USE_BOOST_THREAD

// XML结点信息.
typedef struct
{
	CString strFullPath;    // 全路径.
	CString strName;        // 文件名.
	CString strMd5;         // 文件的MD5值.
   CString strUpdateMd5;   // 更新文件的md5值.
	CString strUrl;         // 下载的url,默认为空.
	CString strCommand;     // 命令.
	CString strCompress;    // 压缩选项.
	BOOL isCheckExist;		// 更新时不检查文件MD5, 只检查文件是否存在, 如果不存在才更新.
	DWORD dwSize;           // 大小.
	LONGLONG lVersion;      // 文件版本.
} XMLNODEINFO, *PXMLNODEINFO;

typedef std::vector<XMLNODEINFO> ListXml;

// 配置文件名.
#define INI_FILE_NAME  _T(".\\load.ini")
#define INI_APP_NAME   _T("Initialize")
#define INI_OUT_PATH   _T("OutPath")
#define INI_URL_PATH   _T("Url")
#define INI_ERROR_RET  _T("0")