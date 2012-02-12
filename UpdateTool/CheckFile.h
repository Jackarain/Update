#pragma once

// 检查文件进度,参数WPARAM表示检查文件个数进度,LPARAM返回文件名.
#define WM_CHECK_PROGRESS		(WM_USER + 2001)

// 检查文件完成,参数WPARAM表示检查文件个数,LPARAM是否文件完整,完整为0,不完整为缺失文件数.
#define WM_CHECK_COMPLETE		(WM_USER + 2002)

// 发送文件个数消息,参数WPARAM表示文件个数.
#define WM_CHECK_FILE_COUNT		(WM_USER + 2003)


class CCheckFile
{
public:
    CCheckFile(void);
    ~CCheckFile(void);

public:
    // 设置文件列表.
    void GetFileList(ListXml& mFileList);

    // 统计文件数.
    void CountFileSum(path const& p, path const& l);

    // 添加文件到列表.
    void AddFiles(ListXml& t, path const& p, path const& l);

    // 返回文件总数.
    DWORD GetListCount() { return m_dwFileCount; }

    // 设置文件路径.
    void SetFindPath(CString& strPath) { m_strPath = strPath; }

    // 清理列表.
    void CleanList() { m_lstXmlInfo.clear(); }

    // 文件版本号处理函数.
    BOOL GetFileVersion(LPCTSTR szFilename, LONGLONG& lVer);
    BOOL GetFileVersion(LPCTSTR szFilename, CString& strVer);
    BOOL GetFileVersion(CString& strVer, LONGLONG& lVer);
    BOOL GetFileVersion(LONGLONG& lVer, CString& strVer);
    int CompareVersion(LPCTSTR szFile1, LPCTSTR szFile2); // 版本信息比较,file1版本大于file2时返回大于0,等于时返回0,小于时返回小于0,返回-1为比较失败.

public:
    // 主执行程序.
    HRESULT Execute(DWORD_PTR dwParam, HANDLE hObject);

private:
    CString m_strPath;
    DWORD m_dwFileCount;
    XMLNODEINFO m_xmlTemp;
    ListXml m_lstXmlInfo;						// xml信息.
    HWND m_hMsgWnd;								// 消息通知窗口句柄.
};
