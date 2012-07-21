// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "aboutdlg.h"
#include "MainDlg.h"

BOOL CListEx::InitListCtrlEx()
{
	m_dwSelected = -1;
	m_dwEditLine = CMainDlg::emUrl;
	return TRUE;
}

LRESULT CListEx::OnEditNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CString strText;

	if (m_ctlHideEdit.IsWindow())
	{
		m_ctlHideEdit.GetWindowText(strText);
		m_ctlHideEdit.ShowWindow(SW_HIDE);

		SetItemText(m_dwSelected, m_dwEditLine, strText);
	}

	bHandled = 0;
	return 0;
}

LRESULT CListEx::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (uMsg == WM_KEYDOWN)
	{
		if (wParam == VK_RETURN)
		{
			CRect rcRect;
			CString strText;

			if (!m_ctlHideEdit.Create(m_hWnd, NULL, NULL, WS_CHILD, ES_AUTOHSCROLL))
				return FALSE;

			m_ctlHideEdit.SetFont(GetFont());

			m_dwSelected = GetNextItem(-1, LVNI_SELECTED);
			if (m_dwSelected != -1)
			{
				GetItemText(m_dwSelected, m_dwEditLine, strText);
				GetSubItemRect(m_dwSelected, m_dwEditLine, 0, &rcRect);

				rcRect.left += 1;
				rcRect.bottom -= 1;

				m_ctlHideEdit.MoveWindow(&rcRect);
				m_ctlHideEdit.ShowWindow(SW_SHOW);
				m_ctlHideEdit.SetWindowText(strText);
				m_ctlHideEdit.SetFocus();
			}
		}
	}

	bHandled = 0;
	return 0;
}

void CMainDlg::AddFiles(ListXml& t, path const& p, path const& l, DWORD& s)
{
	if (l.leaf()[0] == '.')
		return ;
	path f(p / l);
	if (is_directory(f))
	{
		for (directory_iterator i(f), end; i != end; ++i)
			AddFiles(t, p, l / i->leaf(), s);
	}
	else
	{
		XMLNODEINFO xml;
		xml.dwSize = (DWORD)file_size(f);
		xml.strName = f.string().c_str();
		xml.strName.Replace(_T('/'), _T('\\'));

		char szMd5[33] = "";				
#ifdef UNICODE
		MDFile(CW2A(xml.strName.GetBuffer(0), CP_ACP), szMd5);
		OutputDebugString(xml.strName.GetBuffer(0));
		ATLTRACE("\n md5:%s\n", szMd5);
#else
		MDFile(name.GetBuffer(0), szMd5);
		OutputDebugString(xml.strName.GetBuffer(0));
		ATLTRACE("\n md5:%s\n", szMd5);
#endif // UNICODE

		xml.strMd5 = szMd5;
		xml.strMd5.Trim();
		xml.strCommand = _T("");
		// 是否需要注册.
		HINSTANCE hLib = LoadLibraryEx(xml.strName, NULL, DONT_RESOLVE_DLL_REFERENCES);
		if (hLib)
		{
			if (GetProcAddress(hLib, "DllRegisterServer"))
            xml.strCommand = _T("regsvr");
			FreeLibrary(hLib);
		}
		// 是否需要压缩.
		xml.strCompress = _T("gz");

		// 文件版本信息.
		xml.lVersion = 0;
		m_thCheck.GetFileVersion(xml.strName.GetBuffer(0), xml.lVersion);
	
		s++; // 统计文件总数.
		t.push_back(xml);
	}
}

BOOL CMainDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && 
		pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
	{
		m_ctlList.SendMessage(WM_KEYDOWN, pMsg->wParam, pMsg->lParam);
	}

	return CWindow::IsDialogMessage(pMsg);
}

BOOL CMainDlg::OnIdle()
{
	return FALSE;
}

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// center the dialog on the screen
	CenterWindow();

	// set icons
	HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	UIAddChildWindowContainer(m_hWnd);
	//////////////////////////////////////////////////////////////////////////
	// 控件操作.
	m_ctlInPath.SubclassWindow(GetDlgItem(IDC_IN_PATH));
	m_ctlOutPath.SubclassWindow(GetDlgItem(IDC_OUT_PATH));
	m_ctlInPath.Set(CEditEx::emOpen);
	m_ctlOutPath.Set(CEditEx::emSave);
	m_ctlInPath.SetWindowText(_T("<请双击选择位置>"));
	m_ctlOutPath.SetWindowText(_T("<请双击选择位置>"));
	m_ctlTextTip.Attach(GetDlgItem(IDC_TEXT_LIST_TIP));
	m_ctlList.SubclassWindow(GetDlgItem(IDC_LIST_HASH));
	m_ctlList.InitListCtrlEx();
	// m_ctlList.Attach(GetDlgItem(IDC_LIST_HASH));
	m_ctlBaseUrl.Attach(GetDlgItem(IDC_EDIT_URL));

	// 设置列表控件属性.
	m_ctlList.SetExtendedListViewStyle(m_ctlList.GetExtendedListViewStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES
#if (_WIN32_WINNT >= 0x501)
		| LVS_EX_DOUBLEBUFFER
#endif		
	);

	// 设置列.
	m_ctlList.InsertColumn(1, _T("文件名"), LVCFMT_LEFT, 240);		// emName
	m_ctlList.InsertColumn(2, _T("文件版本"), LVCFMT_LEFT, 100);	// emVer
	m_ctlList.InsertColumn(3, _T("文件MD5"), LVCFMT_LEFT, 210);	// emHash
	m_ctlList.InsertColumn(4, _T("压缩文件MD5"), LVCFMT_LEFT, 210);	// emUpdateHash
	m_ctlList.InsertColumn(5, _T("大小"), LVCFMT_LEFT, 140);		// emSize
	m_ctlList.InsertColumn(6, _T("COM注册"), LVCFMT_LEFT, 65);	// emRegsvr
	m_ctlList.InsertColumn(7, _T("压缩方式"), LVCFMT_LEFT, 100);	// emCompress
	m_ctlList.InsertColumn(8, _T("检查存在"), LVCFMT_LEFT, 100);	// emFileExist
	m_ctlList.InsertColumn(9, _T("自定义URL"), LVCFMT_LEFT, 600);	// emUrl
	m_ctlList.InsertColumn(10, _T("完整文件名"), LVCFMT_LEFT, 0);	// emFullPath

	// 设置控件位置.
	AddAnchor(GetDlgItem(IDC_TEXT_SEL_SETUP), TOP_LEFT, TOP_RIGHT);
	AddAnchor(GetDlgItem(IDC_TEXT_SEL_XML), TOP_LEFT, TOP_RIGHT);
	AddAnchor(GetDlgItem(IDC_TEXT_LIST_TIP), TOP_LEFT, TOP_RIGHT);
	AddAnchor(GetDlgItem(IDC_LIST_HASH), TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(GetDlgItem(IDC_CHECK), TOP_RIGHT);
	AddAnchor(GetDlgItem(IDC_BTN_GEN), TOP_RIGHT);
	AddAnchor(GetDlgItem(IDC_BTN_CLEAN), TOP_RIGHT);
	AddAnchor(GetDlgItem(IDCANCEL), BOTTOM_RIGHT);
	AddAnchor(GetDlgItem(ID_APP_ABOUT), BOTTOM_RIGHT);
	AddAnchor(GetDlgItem(IDC_IN_PATH), TOP_LEFT, TOP_RIGHT);
	AddAnchor(GetDlgItem(IDC_OUT_PATH), TOP_LEFT, TOP_RIGHT);
	AddAnchor(GetDlgItem(IDC_EDIT_URL), TOP_LEFT, TOP_RIGHT);

	// 设置窗口可以接收文件.
	DragAcceptFiles(TRUE);
	m_bDropFiles = FALSE;

	CString strOutPath;
	CString strInUrl;

	strOutPath.Preallocate(MAX_PATH);
	strInUrl.Preallocate(2048);

	// 读取配置文件.
	GetPrivateProfileString(INI_APP_NAME, INI_OUT_PATH, INI_ERROR_RET, strOutPath.GetBuffer(0), MAX_PATH, INI_FILE_NAME);
	strOutPath.ReleaseBuffer();
	if (strOutPath != INI_ERROR_RET)
	{
		m_ctlOutPath.SetWindowText(strOutPath);
		m_strOutPath = strOutPath;
	}
	GetPrivateProfileString(INI_APP_NAME, INI_URL_PATH, INI_ERROR_RET, strInUrl.GetBuffer(0), 2048, INI_FILE_NAME);
	strInUrl.ReleaseBuffer();
	if (strInUrl != INI_ERROR_RET)
	{
		m_ctlBaseUrl.SetWindowText(strInUrl);
		m_strBaseUrl = strInUrl;
	}

	return TRUE;
}

LRESULT CMainDlg::OnEditNotify(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	if (wParam == CEditEx::emOpen)
	{
		m_strInPath = (LPTSTR)lParam;
		// 开始检查文件...
		m_thCheck.SetFindPath(m_strInPath);
        boost::thread(boost::bind(&CCheckFile::Execute, &m_thCheck, (DWORD_PTR)m_hWnd, (HANDLE)NULL));
	}
	else if (wParam == CEditEx::emSave)
	{
		// 设置保存文件路径...
		m_strOutPath = (LPTSTR)lParam;
	}

	return 0;
}

LRESULT CMainDlg::OnCompressFileCount(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CString strText;

	m_dwSumFile = wParam;
	strText.Format(_T("共%d个文件,程序正在进行压缩..."), m_dwSumFile);
	m_ctlTextTip.SetWindowText(strText);

	return 0;
}

LRESULT CMainDlg::OnCompressFileProgress(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	CString strText;

	strText.Format(_T("共%d个文件,程序正在压缩第%d个文件的...\n"), m_dwSumFile, lParam);
	m_ctlTextTip.SetWindowText(strText);

	return 0;
}

LRESULT CMainDlg::OnCompressFileComplete(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	CString strText;
	CString strPath;

	m_dwSumFile = wParam;
	strPath = (LPTSTR)lParam;

	strText.Format(_T("共%d个文件,压缩完成."), wParam);
	m_ctlTextTip.SetWindowText(strText);

   MessageBox(_T("生成XML成功!"), _T("提示."), MB_OK | MB_ICONINFORMATION);

   // 打开文件夹.
	ShellExecute(NULL, _T("open"), _T("explorer.exe"), strPath, _T(""), SW_SHOW);

	return 0;
}

LRESULT CMainDlg::OnCheckFileCount(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CString strText;
	
	m_dwSumFile = wParam;
	strText.Format(_T("该目录有%d个文件,程序正在计算MD5 HASH..."), m_dwSumFile);
	m_ctlTextTip.SetWindowText(strText);

	return 0;
}

LRESULT CMainDlg::OnCheckFileProgress(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	CString strText;

	strText.Format(_T("该目录有%d个文件,程序正在计算第%d个文件的MD5 HASH...\n"), m_dwSumFile, lParam);
	m_ctlTextTip.SetWindowText(strText);

	return 0;
}

LRESULT CMainDlg::OnCheckFileComplete(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	CString strText;
	CString strPath;

	m_dwSumFile = wParam;
	strText.Format(_T("该目录有%d个文件,计算完成."), wParam);
	m_ctlTextTip.SetWindowText(strText);

	// 得到文件表,并添加到列表控件.
	if (!m_bDropFiles)
		m_thCheck.GetFileList(m_lstXmlInfo);

	if (m_strInPath.Right(1) != _T("\\"))
		strPath = m_strInPath + _T('\\');

	if (!m_bDropFiles)
		m_ctlList.DeleteAllItems();

	// 截出多余部分.
	for (ListXml::iterator it = m_lstXmlInfo.begin(); it != m_lstXmlInfo.end(); it++) {
		it->strFullPath = it->strName;
		if (strPath !=  _T("\\"))
			it->strName.Replace(strPath, _T(''));
		if (m_bDropFiles) {
			TCHAR szDir[MAX_PATH];  
			_tcscpy_s(szDir, it->strFullPath.GetBuffer(0));
			PathRemoveFileSpec(szDir);
			strPath = szDir;
			strPath.Right(1) != _T("\\") ? strPath = strPath + _T('\\') : CString("");
			it->strName.Replace(strPath, _T(""));
		}
		OutputDebugString(it->strName.GetBuffer(0));
		ATLTRACE("\n");

		// 插入一行.
		m_ctlList.InsertItem(0, it->strName.GetBuffer(0));
		// 设置文件版本号.
		if (m_thCheck.GetFileVersion(it->lVersion, strText))
			m_ctlList.SetItemText(0, emVer, strText);
		// 设置文件hash.
		m_ctlList.SetItemText(0, emHash, it->strMd5.GetBuffer(0));
		// 设置文件大小.
		strText.Format(_T("%d （字节）"), it->dwSize);
		m_ctlList.SetItemText(0, emSize, strText);
		// 文件是否注册.
		if (it->strCommand != _T(""))
			m_ctlList.SetItemText(0, emRegsvr, _T("注册"));
// 		// url位置.
// 		CString strUrl;
// 		m_ctlBaseUrl.GetWindowText(strUrl);
// 		strUrl != _T("") ? it->strUrl = strUrl, m_ctlList.SetItemText(0, emUrl, strUrl) : NULL;
		// 文件使用的压缩格式.
		if (it->strCompress != _T(""))
			m_ctlList.SetItemText(0, emCompress, _T("使用zip格式"));
		m_ctlList.SetItemText(0, emFullPath, it->strFullPath);
		// 设置为选择状态.
		m_ctlList.SetCheckState(0, TRUE);
	}

	strText.Format(_T("该目录有%d个文件,计算完成."), m_ctlList.GetItemCount());
	m_ctlTextTip.SetWindowText(strText);

	m_bDropFiles = FALSE;

	OnEnChangeEditUrl(0, 0, 0, m_bDropFiles);
	return 0;
}

LRESULT CMainDlg::OnDropfiles(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	HDROP hDrop = (HDROP)wParam;
	CString strName;
	DWORD dwFileCount = 0;
	int DropCount = DragQueryFile(hDrop, -1, NULL, 0);
	m_lstXmlInfo.clear();
	m_bDropFiles = TRUE;
	for(int i = 0; i < DropCount; i++)
	{
		int nNameSize = DragQueryFile(hDrop, i, NULL, 0) + 1;
		strName.Preallocate(nNameSize);		
		DragQueryFile(hDrop, i, strName.GetBuffer(0), nNameSize);
		strName.ReleaseBuffer();
		CFindFile finder;
		finder.FindFile(strName);
 		// 开始检查文件...
#ifdef UNICODE
		std::string str = CW2A(strName.GetBuffer(0), CP_ACP);
#else
		std::string str = strName.GetBuffer(0);
#endif // UNICODE
		path full_path = complete(path(str.c_str()));
		if (finder.IsDirectory())
		{
 			m_strInPath = strName;
			AddFiles(m_lstXmlInfo, full_path.branch_path(), full_path.leaf(), dwFileCount);
		}
		else
		{
			XMLNODEINFO xml;
			path f(full_path.branch_path() / full_path.leaf());
			xml.dwSize = (DWORD)file_size(f);
			xml.strName = f.string().c_str();
			xml.strName.Replace(_T('/'), _T('\\'));

			char szMd5[33] = "";				
#ifdef UNICODE
			MDFile(CW2A(xml.strName.GetBuffer(0), CP_ACP), szMd5);
			OutputDebugString(xml.strName.GetBuffer(0));
			ATLTRACE("\n md5:%s\n", szMd5);
#else
			MDFile(name.GetBuffer(0), szMd5);
			OutputDebugString(xml.strName.GetBuffer(0));
			ATLTRACE("\n md5:%s\n", szMd5);
#endif // UNICODE

			xml.strMd5 = szMd5;
			xml.strMd5.Trim();
			xml.strCommand = _T("");
			// 是否需要注册.
			HINSTANCE hLib = LoadLibrary(xml.strName);
			if (hLib)
			{
				if (GetProcAddress(hLib, "DllRegisterServer"))
					xml.strCommand = _T("regsvr");
				FreeLibrary(hLib);
			}
			// 是否需要压缩.
			xml.strCompress = _T("gz");

			// 文件版本信息.
			xml.lVersion = 0;
			m_thCheck.GetFileVersion(xml.strName.GetBuffer(0), xml.lVersion);

			dwFileCount++;
			m_lstXmlInfo.push_back(xml);
		}
	}

	SendMessage(WM_CHECK_COMPLETE, dwFileCount);

	return 0;
}

LRESULT CMainDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	return 0;
}

LRESULT CMainDlg::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CMainDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CloseDialog(wID);
	return 0;
}

void CMainDlg::CloseDialog(int nVal)
{
	// 读取配置文件.
	m_ctlOutPath.GetWindowText(m_strOutPath);
	WritePrivateProfileString(INI_APP_NAME, INI_OUT_PATH, m_strOutPath.GetBuffer(0), INI_FILE_NAME);
	m_ctlBaseUrl.GetWindowText(m_strBaseUrl);
	WritePrivateProfileString(INI_APP_NAME, INI_URL_PATH, m_strBaseUrl.GetBuffer(0), INI_FILE_NAME);

	DestroyWindow();
	::PostQuitMessage(nVal);
}

LRESULT CMainDlg::OnBnClickedBtnGen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// 获取输出路径.
	m_ctlOutPath.GetWindowText(m_strOutPath);

	if (m_strOutPath == _T(""))
	{
		MessageBox(_T("请选择目标文件夹!"), _T("提示"), MB_OK | MB_ICONWARNING);
		return 0;
	}
   else if (m_lstXmlInfo.size() == 0)
	{
		MessageBox(_T("请选择源文件夹."), _T("提示"), MB_OK | MB_ICONWARNING);
		return 0;
	}

   m_thr_gen = boost::thread(boost::bind(&CMainDlg::GenXmlFile, this));

	return 0;
}

LRESULT CMainDlg::OnNMDblclkListHash(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	CString strText;
	CPoint ptCursor;
	CRect rcRect;
	int nSel;
	
	// 判断双击位置.
	GetCursorPos(&ptCursor);
	nSel = m_ctlList.GetNextItem(-1, LVNI_SELECTED);
	if (nSel != -1)
	{
		m_ctlList.GetSubItemRect(nSel, emRegsvr, 0, &rcRect);
		m_ctlList.ClientToScreen(&rcRect);
		if (rcRect.PtInRect(ptCursor))				// 判断是否双击在注册列.
		{
			m_ctlList.GetItemText(nSel, emRegsvr, strText);
			if (strText == _T(""))
			{
				m_ctlList.SetItemText(m_ctlList.GetNextItem(-1, LVNI_SELECTED), emRegsvr, _T("注册"));
			}
			else
			{
				m_ctlList.SetItemText(m_ctlList.GetNextItem(-1, LVNI_SELECTED), emRegsvr, _T(""));
			}

			return 0;
		}

		m_ctlList.GetSubItemRect(nSel, emFileExist, 0, &rcRect);
		m_ctlList.ClientToScreen(&rcRect);
		if (rcRect.PtInRect(ptCursor))				// 判断是否双击在只检查文件列.
		{
			m_ctlList.GetItemText(nSel, emFileExist, strText);
			if (strText == _T(""))
			{
				m_ctlList.SetItemText(m_ctlList.GetNextItem(-1, LVNI_SELECTED), emFileExist, _T("是"));
			}
			else
			{
				m_ctlList.SetItemText(m_ctlList.GetNextItem(-1, LVNI_SELECTED), emFileExist, _T(""));
			}

			return 0;
		}

		m_ctlList.GetSubItemRect(nSel, emCompress, 0, &rcRect);
		m_ctlList.ClientToScreen(&rcRect);
      if (rcRect.PtInRect(ptCursor))				// 判断是否双击在压缩选择列.
      {
         m_ctlList.GetItemText(nSel, emCompress, strText);
         if (strText == _T(""))
            m_ctlList.SetItemText(m_ctlList.GetNextItem(-1, LVNI_SELECTED), emCompress, _T("使用gz格式"));
         else if (strText == _T("使用gz格式"))
            m_ctlList.SetItemText(m_ctlList.GetNextItem(-1, LVNI_SELECTED), emCompress, _T("使用zip格式"));
         else
            m_ctlList.SetItemText(m_ctlList.GetNextItem(-1, LVNI_SELECTED), emCompress, _T(""));

         return 0;
      }

		m_ctlList.GetSubItemRect(nSel, emUrl, 0, &rcRect);
		m_ctlList.ClientToScreen(&rcRect);

		if (rcRect.PtInRect(ptCursor))
		{
			m_ctlList.SendMessage(WM_KEYDOWN, VK_RETURN);
			return 0;
		}
	}

	return 0;
}

LRESULT CMainDlg::OnLvnKeydownListHash(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& bHandled)
{
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);

	if (pLVKeyDow->wVKey == VK_SPACE)
	{
		int nSelCount = m_ctlList.GetSelectedCount();
		if (nSelCount > 0)
		{
			int nSel = -1;
			BOOL bChk;
			for (int i = 0; i < nSelCount; i++)
			{
				nSel = m_ctlList.GetNextItem(nSel, LVNI_SELECTED);
				if (m_ctlList.GetItemState(nSel, LVIS_FOCUSED) == LVIS_FOCUSED) // 此处必须的.让默认程序处理.
					continue ;
				bChk = m_ctlList.GetCheckState(nSel);
				m_ctlList.SetCheckState(nSel, !bChk);
			}
		}
	}

	return 0;
}

LRESULT CMainDlg::OnEnChangeEditUrl(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CString strText;
	CString strBaseUrl;

	DWORD dwItemCount = m_ctlList.GetItemCount();
	for (DWORD i = 0; i < dwItemCount; i++)
	{
		m_ctlList.GetItemText(i, emUrl, strText);	// 得到第5列的URL.

		// 如果得到的文本中有编辑框里的内容,或者列是空的则添加默认url.
		if (strText.Find(m_strBaseUrl) != -1 || strText == _T(""))
		{
			CString strItemText;

			m_ctlBaseUrl.GetWindowText(strBaseUrl);
			m_ctlList.GetItemText(i, emName, strItemText);

			// 处理'/'符号.
			if (strBaseUrl.Right(1) == _T('/') || strItemText.Left(1) == _T('/'))
				strItemText = strBaseUrl + strItemText;
			else
				strItemText = strBaseUrl + _T("/") + strItemText;
			strItemText.Replace(_T('\\'), _T('/'));
			// 设置列.
			m_ctlList.SetItemText(i, emUrl, strItemText);
		}
	}

	m_ctlBaseUrl.GetWindowText(m_strBaseUrl);
	
	return 0;
}


LRESULT CMainDlg::OnBnClickedBtnClean(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_ctlList.DeleteAllItems();
	m_lstXmlInfo.clear();
	return 0;
}

void CMainDlg::GenXmlFile()
{
   CString strText;
   std::vector<int> SelItems;
   DWORD nSum = 0;
   CString strSaveDirectory;
   int nCount = m_ctlList.GetItemCount();

   // 得到当前工作目录.
   strSaveDirectory.Preallocate(MAX_PATH);
   GetCurrentDirectory(MAX_PATH, strSaveDirectory.GetBuffer(0));
   strSaveDirectory.ReleaseBuffer();

   // 输出路径添加'\'
   if (m_strOutPath.Right(1) != _T('\\'))
      m_strOutPath = m_strOutPath + _T('\\');

   // 设置压缩输出目录.
   m_thCompress.CreateDirectoryNested(m_strOutPath.GetBuffer(0));

   // 得到输入输出的ansi路径.
   std::string outPath, inFile, outFile;
#ifdef UNICODE
   outPath = CW2A(m_strOutPath.GetBuffer(0), CP_ACP);
#else
   outPath = m_strOutPath.GetBuffer(0);
#endif // UNICODE

   // 得到选择的文件个数.
   for (int i = 0; i < nCount; i++)
   {
      m_ctlList.GetItemText(i, emName, strText);
      if (m_ctlList.GetCheckState(i) &&
          strText != _T(""))
      {
         SelItems.push_back(i);
         nSum++;
      }
   }

   // 发送消息.
   PostMessage(WM_COMMPRESS_FILE_COUNT, nSum, 0);
   nSum = 0;

   // 创建xml文件.
   SYSTEMTIME sysTime;
   CString strTime;

   // 得到时间.
   GetLocalTime(&sysTime);
   strTime.Format(_T("%04d-%02d-%02d %02d:%02d:%02d"), sysTime.wYear, sysTime.wMonth, sysTime.wDay,
      sysTime.wHour, sysTime.wMinute, sysTime.wSecond);

   // 创建一个XML的文档对象.
   TiXmlDocument* xmlDocument = new TiXmlDocument();
   TiXmlElement* fileElement = NULL;
   TiXmlDeclaration* xmlDec = new TiXmlDeclaration("1.0", "GB2312", "");
   xmlDocument->LinkEndChild(xmlDec);
   TiXmlElement *rootElement = new TiXmlElement("update_root");
   xmlDocument->LinkEndChild(rootElement);
   rootElement->SetAttribute("createtime", CW2A(strTime.GetBuffer(0)));

   CString strCurDirectory;
   XMLNODEINFO xml;
   LONGLONG lVer = 0;
   unsigned int nSize = 0;
   int nPos;

   // 清空xml容器.
   m_lstXmlInfo.clear();

   for (std::vector<int>::iterator i = SelItems.begin();
      i != SelItems.end(); i++)
   {
      CString strFileDir, strFileName;
      std::string strInName, strOutName, strTemp;
      int n = *i;

      // 创建Element.
      fileElement = new TiXmlElement("file");
      rootElement->LinkEndChild(fileElement);

      // 得到文件名.
      m_ctlList.GetItemText(n, emName, strText);
      xml.strName = strText;
      fileElement->SetAttribute("name", CW2A(strText.GetBuffer(0), CP_ACP));

      // 得到完整文件名.
      m_ctlList.GetItemText(n, emFullPath, strText);
      xml.strFullPath = strText;

      // 得到是否需要压缩.
      m_ctlList.GetItemText(n, emCompress, strText);
      strText.Trim();
      nPos = strText.Find(_T("gz"));
      if (nPos != -1)
         xml.strCompress = _T("gz");
      nPos = strText.Find(_T("zip"));
      if (nPos != -1)
         xml.strCompress = _T("zip");

      // 压缩文件到目标文件夹.
      strFileDir = xml.strFullPath;
      // 得到最后一个反斜杠位置.
      nPos = strFileDir.ReverseFind(_T('\\'));
      // 最后一个反斜杠位置前面的字符串实际上就是完整路径.
      strCurDirectory = strFileDir.Left(nPos + 1);
      // 设置为当前文件夹.
      SetCurrentDirectory(strCurDirectory.GetBuffer(0));
      // 最后一个反斜杠位置后面的字符串实际上就是文件名.
      strFileName = strFileDir.Right(strFileDir.GetLength() - (nPos + 1));
      // 得到ansi字符串的输入文件名和输出文件名.
#ifdef UNICODE
      strInName = CW2A(strFileName.GetBuffer(0), CP_ACP);
      strOutName = CW2A(xml.strName.GetBuffer(0), CP_ACP);
#else
      strInName = strFileName.GetBuffer(0);
      strOutName = xml.strName.GetBuffer(0);
#endif // UNICODE

      inFile = strInName;
      outFile = outPath + strOutName;

      // 压缩文件.
      if (xml.strCompress == _T("zip"))      // 使用zip压缩.
      {
         outFile += ".zip";
         m_thCompress.CompressZip(inFile.c_str(), outFile.c_str());
         fileElement->SetAttribute("compress", "zip");
      }
      else if (xml.strCompress == _T("gz"))  // 使用gz压缩.
      {
         outFile += ".gz";
         m_thCompress.CompressGz(inFile.c_str(), outFile.c_str());
         fileElement->SetAttribute("compress", "gz");
      }
      else                                   // 不使用压缩时,直接复制过去.
      {
         CString strNewName = m_strOutPath + xml.strName;
         m_thCompress.CreateFileNested(strNewName);
         CopyFile(strFileName, strNewName, FALSE);
      }

		// 只检测文件存在.
		m_ctlList.GetItemText(n, emFileExist, strText);
		if (strText == _T(""))
		{
			xml.isCheckExist = FALSE;
		}
		else
		{
			xml.isCheckExist = TRUE;
			fileElement->SetAttribute("check", 1);
		}

      // 计算压缩文件的md5值.
      char szMd5[33] = "";
#ifdef UNICODE
      MDFile((char*)outFile.c_str(), szMd5);
      OutputDebugStringA(outFile.c_str());
      ATLTRACE("\n md5:%s\n", szMd5);
#else
      MDFile((char*)outFile.c_str(), szMd5);
      OutputDebugStringA(outFile.c_str());
      ATLTRACE("\n md5:%s\n", szMd5);
#endif // UNICODE
      xml.strUpdateMd5 = szMd5;

      // 设置Update Hash信息.
      m_ctlList.SetItemText(n, emUpdateHash, xml.strUpdateMd5.GetBuffer(0));
      fileElement->SetAttribute("filehash", CW2A(xml.strUpdateMd5.GetBuffer(0)));

      // HASH信息.
      m_ctlList.GetItemText(n, emHash, strText);
      xml.strMd5 = strText;
      fileElement->SetAttribute("md5", CW2A(strText.GetBuffer(0)));

      // 版本信息.
      m_ctlList.GetItemText(n, emVer, strText);
      lVer = 0;
      m_thCheck.GetFileVersion(strText.GetBuffer(0), lVer);
      xml.lVersion = lVer;
      fileElement->SetAttribute("version", CW2A(strText.GetBuffer(0), CP_ACP));

      // 文件大小.
      m_ctlList.GetItemText(n, emSize, strText);
      nSize = _tstol(strText.GetBuffer(0));
      xml.dwSize = nSize;
      fileElement->SetAttribute("size", nSize);

      // 是否需要注册.
      m_ctlList.GetItemText(n, emRegsvr, strText);
      xml.strCommand = strText;
      strText == _T("") ? NULL : fileElement->SetAttribute("command", "regsvr");

      // 指定url.
      m_ctlList.GetItemText(n, emUrl, strText);
      if (strText != _T(""))
      {
         if (xml.strCompress == _T("gz"))
         {
            if (strText.Right(2) != _T("gz"))
               strText = strText + _T(".gz");
         }
         else if (xml.strCompress == _T("zip"))
         {
            if (strText.Right(3) != _T("zip"))
               strText = strText + _T(".zip");
         }
      }
      xml.strUrl = strText;
#ifdef UNICODE
      strTemp = CW2A(strText.GetBuffer(0), CP_ACP);
#else
      strTemp = strText.GetBuffer(0);
#endif
      strText == _T("") ? NULL : fileElement->SetAttribute("url", strTemp.c_str());

      // 更新到容器.
      m_lstXmlInfo.push_back(xml);

      // 发送消息更新ui.
      nSum++;
      PostMessage(WM_COMMPRESS_PROGRESS, (WPARAM)xml.strName.GetBuffer(0), nSum);
   }
   rootElement->SetAttribute("count", nSum);

   // 创建路径.
   m_thCompress.CreateDirectoryNested(m_strOutPath.GetBuffer(0));

   CString strXmlFile;
   if (m_strOutPath.Right(1) == _T('\\'))
      strXmlFile = m_strOutPath + _T("filelist.xml");
   else
      strXmlFile = m_strOutPath + _T("\\filelist.xml");

   // 保存到XML文件.
   xmlDocument->SaveFile(CW2A(strXmlFile.GetBuffer(0), CP_ACP));
   delete xmlDocument;

   // 设置当前目录.
   SetCurrentDirectory(strSaveDirectory.GetBuffer(0));
   //  发送压缩完成消息.
   PostMessage(WM_COMMPRESS_COMPLETE, (WPARAM)nSum, (LPARAM)m_strOutPath.GetBuffer(0));
   // 清空容器.
   m_lstXmlInfo.clear();
}
