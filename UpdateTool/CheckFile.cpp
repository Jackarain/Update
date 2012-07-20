#include "stdafx.h"
#include "CheckFile.h"
#include <io.h>

CCheckFile::CCheckFile(void)
: m_dwFileCount(0)
{
}

CCheckFile::~CCheckFile(void)
{
}

void CCheckFile::GetFileList(ListXml& mFileList)
{
    mFileList = m_lstXmlInfo;
}

BOOL CCheckFile::GetFileVersion(LPCTSTR szFilename, LONGLONG& lVer)
{
    DWORD dwDummy;
    DWORD dwVerInfoSize;
    LPBYTE lpVersionInfo;
    VS_FIXEDFILEINFO *lpFileInfo;
    DWORD dwFileVersionMS;
    DWORD dwFileVersionLS;
    LONGLONG lRet;
    UINT uLen;

    // 获取文件版本信息大小.
    dwVerInfoSize = GetFileVersionInfoSize(szFilename, &dwDummy);
    if (dwVerInfoSize == 0)
        return FALSE;
    lpVersionInfo = new BYTE[dwVerInfoSize];
    if (!lpVersionInfo)
        return FALSE;
    // 获取文件版本信息.
    if (!GetFileVersionInfo(szFilename, 0, dwVerInfoSize, lpVersionInfo))
    {
        delete[] lpVersionInfo;
        return FALSE;		
    }
    // 查询文件版本信息.
    if (!VerQueryValue(lpVersionInfo, _T("\\"), (LPVOID *)&lpFileInfo, &uLen))
    {
        delete[] lpVersionInfo;
        return FALSE;
    }
    dwFileVersionMS = lpFileInfo->dwFileVersionMS;
    dwFileVersionLS = lpFileInfo->dwFileVersionLS;
    ATLTRACE("Higher: %x, Lower: %x\n" , dwFileVersionMS, dwFileVersionLS);
    delete[] lpVersionInfo;
    // 合并成LONGLONG.
    lRet = dwFileVersionMS;
    lRet <<= 32;
    lRet |= dwFileVersionLS;
    lVer = lRet;

    return TRUE;
}

BOOL CCheckFile::GetFileVersion(LPCTSTR szFilename, CString& strVer)
{
    LONGLONG lVer;

    // 获取版本信息.
    if (!GetFileVersion((LPCTSTR)szFilename, lVer))
        return FALSE;
    // 格式化为字符串.
    DWORD dwFileVersionMS = lVer >> 32;
    DWORD dwFileVersionLS = lVer & 0x00000000FFFFFFFF;
    DWORD dwVer1 = HIWORD(dwFileVersionMS); 
    DWORD dwVer2 = LOWORD(dwFileVersionMS); 
    DWORD dwVer3 = HIWORD(dwFileVersionLS); 
    DWORD dwVer4 = LOWORD(dwFileVersionLS);		
    strVer.Format(_T("%d.%d.%d.%d"), dwVer1, dwVer2, dwVer3, dwVer4);

    return TRUE;
}

BOOL CCheckFile::GetFileVersion(CString& strVer, LONGLONG& lVer)
{
    DWORD dwVer[4];

    if (strVer == _T(""))
        return FALSE;

    // 解析字条串.
    _stscanf(strVer.GetBuffer(0), _T("%d.%d.%d.%d"), &dwVer[0], &dwVer[1], &dwVer[2], &dwVer[3]);
    DWORD dwMver = MAKELONG(dwVer[1], dwVer[0]);
    DWORD dwSver = MAKELONG(dwVer[2], dwVer[3]);
    lVer = dwMver;
    lVer <<= 32;
    lVer |= dwSver;

    return TRUE;
}

BOOL CCheckFile::GetFileVersion(LONGLONG& lVer, CString& strVer)
{
    DWORD dwFileVersionMS = lVer >> 32;
    DWORD dwFileVersionLS = lVer & 0x00000000FFFFFFFF;
    DWORD dwVer1 = HIWORD(dwFileVersionMS); 
    DWORD dwVer2 = LOWORD(dwFileVersionMS); 
    DWORD dwVer3 = HIWORD(dwFileVersionLS); 
    DWORD dwVer4 = LOWORD(dwFileVersionLS);
    strVer.Format(_T("%d.%d.%d.%d"), dwVer1, dwVer2, dwVer3, dwVer4);

    return TRUE;
}

int CCheckFile::CompareVersion(LPCTSTR szFile1, LPCTSTR szFile2)
{
    LONGLONG lVer1, lVer2;
    int nRet = -1;

    // 获取两个文件的版本.
    if (!GetFileVersion(szFile1, lVer1))
        return nRet;
    if (!GetFileVersion(szFile2, lVer2))
        return nRet;		

    // 计算.
    nRet = (int)(lVer1 - lVer2);

    return nRet;
}

void CCheckFile::AddFiles(ListXml& t, path const& p, path const& l)
{
    if (l.leaf()[0] == '.')
        return ;
    path f(p / l);
    if (is_directory(f))
    {
        for (directory_iterator i(f), end; i != end; ++i)
            AddFiles(t, p, l / i->leaf());
    }
    else
    {
        m_xmlTemp.dwSize = (DWORD)file_size(f);
        m_xmlTemp.strName = f.string().c_str();
        m_xmlTemp.strName.Replace(_T('/'), _T('\\'));

        char szMd5[33] = "";
#ifdef UNICODE
        MDFile(CW2A(m_xmlTemp.strName.GetBuffer(0), CP_ACP), szMd5);
        OutputDebugString(m_xmlTemp.strName.GetBuffer(0));
        ATLTRACE("\n md5:%s\n", szMd5);
#else
        MDFile(name.GetBuffer(0), szMd5);
        OutputDebugString(m_xmlTemp.strName.GetBuffer(0));
        ATLTRACE("\n md5:%s\n", szMd5);
#endif // UNICODE

        m_xmlTemp.strMd5 = szMd5;
        m_xmlTemp.strMd5.Trim();
        m_xmlTemp.strCommand = _T("");
        // 是否需要注册.
        HINSTANCE hLib = LoadLibraryEx(m_xmlTemp.strName, NULL, DONT_RESOLVE_DLL_REFERENCES);
        if (hLib) {
           if (GetProcAddress(hLib, "DllRegisterServer"))
              m_xmlTemp.strCommand = _T("regsvr");
           FreeLibrary(hLib);
        }
        // 是否需要压缩.
        m_xmlTemp.strCompress = _T("gz");

        // 文件版本信息.
        m_xmlTemp.lVersion = 0;
        GetFileVersion(m_xmlTemp.strName.GetBuffer(0), m_xmlTemp.lVersion);

        t.push_back(m_xmlTemp);
        m_dwFileCount++;
        // 此时文件总数已计算得出.
        ::PostMessage(m_hMsgWnd, WM_CHECK_PROGRESS, (WPARAM)&m_xmlTemp, m_dwFileCount);
    }
}

void CCheckFile::CountFileSum(path const& p, path const& l)
{
    if (l.leaf()[0] == '.')
        return ;
    path f(p / l);
    if (is_directory(f))
    {
        for (directory_iterator i(f), end; i != end; ++i)
            CountFileSum(p, l / i->leaf());
    }
    else
    {
        m_dwFileCount++;
    }
}

HRESULT CCheckFile::Execute(DWORD_PTR dwParam, HANDLE hObject)
{
    m_hMsgWnd = (HWND)dwParam;
    // 先得到文件总数.
    m_dwFileCount = 0;
    if (PathFileExists(m_strPath))
    {
        // 清空表.
        m_lstXmlInfo.clear();

        CString strPath = m_strPath;
        if (strPath.Right(1) == _T("\\"))
        {
            strPath = strPath.Left(strPath.GetLength() - 1);
        }
        if (strPath.Right(1) == _T(":"))
        {
            strPath = strPath + _T("\\");
        }
#ifdef UNICODE
        std::string str = CW2A(strPath.GetBuffer(0), CP_ACP);
#else
        std::string str = strPath.GetBuffer(0);
#endif // UNICODE
        path full_path = complete(path(str.c_str()));
        try
        {
            CountFileSum(full_path.branch_path(), full_path.leaf());
        }
        catch (std::exception& e)
        {
            ATLTRACE("%s\n", e.what());
        }
        // 此时文件总数已计算得出.
        ::PostMessage(m_hMsgWnd, WM_CHECK_FILE_COUNT, m_dwFileCount, 0);
        // 开始遍历文件,计算MD5值.
        m_dwFileCount = 0;
        try
        {
            AddFiles(m_lstXmlInfo, full_path.branch_path(), full_path.leaf());
        }
        catch (std::exception& e)
        {
            ATLTRACE("%s\n", e.what());
        }
        // 检查文件完成.
        ::PostMessage(m_hMsgWnd, WM_CHECK_COMPLETE, m_dwFileCount, 0);
    }

    return S_FALSE;
}