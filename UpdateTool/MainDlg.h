// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "CheckFile.h"
#include "Compress.h"

#pragma once

#define WM_EDIT_NOTIFY (WM_USER + 101)	// 通知消息.

class CEditEx : public CWindowImpl<CEditEx, CEdit>
{
public:
	typedef enum { emNoSet, emSet, emDisable, emEnable, emSave, emOpen } emStatus;
	emStatus m_emFlag;		// 对话框标志.
	CString  m_strPath;		// 文件路径.

protected:
	CString m_strInfo;		// 提示信息.

public:
	BEGIN_MSG_MAP(CEditEx)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnDbClick)
	END_MSG_MAP()

	CEditEx(HWND hWnd = NULL) 
	{ 
		m_emFlag = emOpen;
		m_strPath = _T("");
		m_strInfo = _T("<请双击选择位置>");
	}

	CEditEx& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	emStatus Set(emStatus status)
	{
		emStatus oldStatus = m_emFlag;
		m_emFlag = status;
		return oldStatus;
	}

	emStatus Get()
	{
		return m_emFlag;
	}

	void InitEdit()
	{
		SetWindowText(m_strInfo);
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return 0;
	}

	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled)
	{
		CString strText;
		GetWindowText(strText);

		if (strText == m_strInfo)
		{
			SetWindowText(_T(""));
		}

		bHandled = 0;
		return 0;
	}

	LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		CString strText;
		GetWindowText(strText);

		if (strText == m_strInfo)
		{
			SetWindowText(_T(""));
		}

		if (m_emFlag == emEnable)
		{
			SetReadOnly(FALSE);
		}
		else if (m_emFlag == emDisable)
		{
			SetReadOnly(TRUE);
		}

		bHandled = 0;
		return 0;
	}

	LRESULT OnKillFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		CString strText;
		GetWindowText(strText);

		if (strText == _T(""))
		{
			SetWindowText(m_strInfo);
		}

		if (m_emFlag == emEnable)
		{
			SetReadOnly(FALSE);
		}
		else if (m_emFlag == emDisable)
		{
			SetReadOnly(TRUE);
		}

		bHandled = 0;
		return 0;
	}

	LRESULT OnDbClick(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		// 打开文件对话框.
		CString strPath;

		if (m_emFlag == emOpen)
		{
			CFolderDialog dlg(m_hWnd, _T("请选择应用程序所在的文件夹:"), BIF_EDITBOX | BIF_VALIDATE | BIF_RETURNONLYFSDIRS);
			if (dlg.DoModal() == IDOK)
			{
				m_strPath = dlg.m_szFolderPath;
				SetWindowText(m_strPath);
				// 发送通知到父窗口.
				GetParent().SendMessage(WM_EDIT_NOTIFY, emOpen, (LPARAM)m_strPath.GetBuffer(0));
			}
		}
		else if (m_emFlag == emSave)
		{
			CFolderDialog dlg(m_hWnd, _T("请选择保存位置:"), BIF_EDITBOX | BIF_VALIDATE | BIF_RETURNONLYFSDIRS);
			if (dlg.DoModal() == IDOK)
			{
				m_strPath = dlg.m_szFolderPath;
				SetWindowText(m_strPath);
				// 发送通知到父窗口.
				GetParent().SendMessage(WM_EDIT_NOTIFY, emSave, (LPARAM)m_strPath.GetBuffer(0));
			}
// 			CFileDialog dlg(FALSE, NULL, NULL, OFN_OVERWRITEPROMPT, _T("XML文件 (*.xml)\0*.xml\0"), m_hWnd);
// 			if (dlg.DoModal() == IDOK)
// 			{
// 				m_strPath = dlg.m_szFileName;
// 				if (m_strPath.Right(4) != _T(".xml"))
// 					m_strPath = m_strPath + _T(".xml");
// 				SetWindowText(m_strPath);
// 				// 发送通知到父窗口.
// 				GetParent().SendMessage(WM_EDIT_NOTIFY, emSave, (LPARAM)m_strPath.GetBuffer(0));
// 			}
		}

		return 0;
	}
};

class CListEx : public CWindowImpl<CListEx, CListViewCtrl>
{
	// 列表框中的编辑框.
	class CListEdit : public CWindowImpl<CListEdit, CEdit>
	{
	public:
		BEGIN_MSG_MAP(CListEdit)
			MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
		END_MSG_MAP()

	public:
		HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
		{
			if (IsWindow())
			{
				DestroyWindow();
			}

			return __super::Create(hWndParent, rect, szWindowName, dwStyle, dwExStyle, MenuOrID, lpCreateParam);
		}

		LRESULT OnKillFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
		{
			CWindow wndParent(GetParent());
			CString strText;

			GetWindowText(strText);
			wndParent.SendMessage(WM_EDIT_NOTIFY, (WPARAM)strText.GetBuffer(0));

			return 0;
		}
	};

public:
	DWORD m_dwSelected;		// 当前选择行.
	BOOL m_bEditShow;		// 当前显示状态,如果被编辑,那么则为显示状态.
	CListEdit m_ctlHideEdit;	// 编辑控件.
	DWORD m_dwTimerID;		// 定时器的ID.
	DWORD m_dwEditLine;		// 编辑行.

public:
	BEGIN_MSG_MAP(CListEx)
		MESSAGE_HANDLER(WM_EDIT_NOTIFY, OnEditNotify)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnEditNotify)
		MESSAGE_HANDLER(WM_HSCROLL, OnEditNotify)
		MESSAGE_HANDLER(WM_VSCROLL, OnEditNotify)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
	END_MSG_MAP()

public:
	// 初始化控件.
	BOOL InitListCtrlEx();

public:
	// 编辑框焦点失去消息.
 	LRESULT OnEditNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	// 按键消息处理...
 	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

class CMainDlg : public CResizableDialogImpl<CMainDlg>, 
				 public CUpdateUI<CMainDlg>,
				 public CMessageFilter, public CIdleHandler
{
	typedef CResizableDialogImpl<CMainDlg> baseClass;
	typedef enum {emName, emVer, emHash, emUpdateHash, emSize, emRegsvr, emCompress, emUrl, emFullPath} listColumn;
	friend class CListEx;
public:
	CMainDlg() : m_thCheck(), m_thCompress() 
		{ m_thCompress.SetOutType(m_thCompress.emAuto); }

public:
	enum { IDD = IDD_MAINDLG };

public:
	CEditEx m_ctlInPath;
	CEditEx m_ctlOutPath;
	CListEx m_ctlList;
	CWindow m_ctlTextTip;
	ListXml m_lstXmlInfo;			// xml信息.
	CString m_strInPath;
	CString m_strOutPath;
	DWORD m_dwSumFile;
	CEdit m_ctlBaseUrl;
	CString m_strBaseUrl;
	BOOL m_bDropFiles;

	CCheckFile m_thCheck;		// 检查文件并更新组件.
	CCompress m_thCompress;		// 压缩文件组件.
   boost::thread m_thr_gen;   // 生成xml文件.

	// 添加文件到ListXml.
	void AddFiles(ListXml& t, path const& p, path const& l, DWORD& s);
   // 生成xml.
   void GenXmlFile();

public:

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	BEGIN_UPDATE_UI_MAP(CMainDlg)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainDlg)
		CHAIN_MSG_MAP(baseClass)
		NOTIFY_HANDLER(IDC_LIST_HASH, LVN_KEYDOWN, OnLvnKeydownListHash)
		NOTIFY_HANDLER(IDC_LIST_HASH, NM_DBLCLK, OnNMDblclkListHash)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_DROPFILES, OnDropfiles)
		MESSAGE_HANDLER(WM_EDIT_NOTIFY, OnEditNotify)
		MESSAGE_HANDLER(WM_CHECK_FILE_COUNT, OnCheckFileCount)
		MESSAGE_HANDLER(WM_CHECK_COMPLETE, OnCheckFileComplete)
		MESSAGE_HANDLER(WM_CHECK_PROGRESS, OnCheckFileProgress)
		MESSAGE_HANDLER(WM_COMMPRESS_FILE_COUNT, OnCompressFileCount)
		MESSAGE_HANDLER(WM_COMMPRESS_COMPLETE, OnCompressFileComplete)
		MESSAGE_HANDLER(WM_COMMPRESS_PROGRESS, OnCompressFileProgress)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(IDC_CHECK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_HANDLER(IDC_BTN_GEN, BN_CLICKED, OnBnClickedBtnGen)
		COMMAND_HANDLER(IDC_EDIT_URL, EN_CHANGE, OnEnChangeEditUrl)
		COMMAND_HANDLER(IDC_BTN_CLEAN, BN_CLICKED, OnBnClickedBtnClean)
	END_MSG_MAP()

	void CloseDialog(int nVal);

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDropfiles(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnEditNotify(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCheckFileCount(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCheckFileComplete(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCheckFileProgress(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCompressFileCount(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCompressFileComplete(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCompressFileProgress(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBtnGen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnNMDblclkListHash(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnLvnKeydownListHash(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnEnChangeEditUrl(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedBtnClean(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};
