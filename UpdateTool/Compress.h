#pragma once

// 压缩缓冲大小.
#define BUFLEN      16384

// 压缩文件进度,参数WPARAM表示检压缩件个数进度,LPARAM返回文件名.
#define WM_COMMPRESS_PROGRESS		(WM_USER + 2004)

// 压缩文件完成,参数WPARAM表示压缩文件个数,LPARAM为压缩后的路径.
#define WM_COMMPRESS_COMPLETE		(WM_USER + 2005)

// 压缩文件个数消息,参数WPARAM表示文件个数.
#define WM_COMMPRESS_FILE_COUNT		(WM_USER + 2006)


class CCompress
{
public:
    typedef enum { emGz, emZip, emAuto } outType;

public:
    CCompress(void);
    ~CCompress(void);

public:
    // 得到文件crc.
    int GetFileCrc(const char* filenameinzip, void*buf, unsigned long size_buf, unsigned long* result_crc);
    // 按路径创建文件夹函数.
    void ModifyPathSpec(TCHAR* szDst, BOOL bAddSpec);
    BOOL CreateDirectoryNested(LPCTSTR lpszDir);
    BOOL CreateFileNested(LPCTSTR lpszFile, BOOL bOverride = TRUE);

public:
    // 压缩一个文件成gz格式.
    bool CompressGz(const char* inFile, const char* outFile);

    // 压缩一个文件成zip格式.
    bool CompressZip(const char* inFile, const char* outFile);

    // 设置输入文件夹位置.
    void SetInPath(CString& strInPath) { m_strInPath = strInPath; }

    // 设置输出文件夹位置.
    void SetOutPath(CString& strOutPath) { m_strOutPath = strOutPath; }

    // 设置压缩文件列表.
    void SetFileList(ListXml& listFile) { m_lstFile = listFile; }

    // 压缩格式为zip时,可设置压缩密码.
    void SetZipPassword(std::string& strPwd) { m_strZipPwd = (m_outType == emZip ? strPwd : ""); }

    // 设置输出格式.
    void SetOutType(outType oType) { m_outType = oType; }

public:
    // 主执行程序.
    HRESULT Execute(DWORD_PTR dwParam, HANDLE hObject);

private:
    outType m_outType;		// 输出文件格式.
    CString m_strOutPath;	// 输出文件夹.
    CString m_strInPath;	// 输入文件夹.
    std::string m_strZipPwd;// zip压缩格式时的密码.	
    ListXml m_lstFile;		// 需要压缩的文件.
    DWORD m_dwFileCount;	// 文件计数.
    HWND m_hMsgWnd;			// 消息通知窗口句柄.
};
