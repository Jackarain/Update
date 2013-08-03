// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "MainDlg.h"

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

   // 绑定控件.
   m_main_progress.Attach(GetDlgItem(IDC_PGS_ALL));
   m_single_progress.Attach(GetDlgItem(IDC_PGS_SINGLE));
   m_main_text.Attach(GetDlgItem(IDC_TEXT_MAIN));
   m_single_text.Attach(GetDlgItem(IDC_TEXT_SINGLE));

   // 设置控件.
   m_single_progress.SetRange(0, 1000);
   m_main_progress.SetRange(0, 1000);

   m_status = 0;

	return TRUE;
}

LRESULT CMainDlg::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CSimpleDialog<IDD_ABOUTBOX, FALSE> dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CMainDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (m_updater.result() == updater::st_updating)
		return 0;
	m_updater.start("http://codedoom.net/update/filelist.xml",
		boost::bind(&CMainDlg::check_files_callback, this, _1, _2, _3),
      boost::bind(&CMainDlg::down_load_callback, this, _1, _2, _3, _4, _5, _6, _7),
      boost::bind(&CMainDlg::check_files_callback, this, _1, _2, _3),
      boost::bind(&CMainDlg::update_files_process, this, _1, _2, _3),
      "C:\\setup_test");
	return 0;
}

LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_updater.stop();
	EndDialog(wID);
	return 0;
}

void CMainDlg::check_files_callback(std::string file, int count, int index)
{
   CString str = file.c_str();
   msg_info* info = new msg_info;

   m_status = WM_CHECK_PROGRESS;

   info->text.Format(_T("总计:%d, 当前:%d, 正在检测文件: \'%s\'"), count, index + 1, str.GetBuffer(0));
   if (count == index + 1)
      info->text = _T("检查完成.");

   info->main_pos = (double)(index + 1) / (double)count * 1000.0f;
   info->single_pos = 1000.0f;

   PostMessage(WM_CHECK_PROGRESS, (WPARAM)info);
}

void CMainDlg::down_load_callback(std::string file, int count, int index, 
   int total_size, int total_read_bytes, int file_size, int read_bytes)
{
   CString str = file.c_str();
   msg_info* info = new msg_info;

   m_status = WM_WORK_PROGRESS;

   info->text.Format(_T("当前: %d, 共: %d, 正在下载文件: \'%s\'"), index + 1, count, str.GetBuffer(0));

   if (count == index + 1)
      info->text = _T("下载完成.");

   info->main_pos = (double)total_read_bytes / (double)total_size * 1000.0f;
   info->single_pos = (double)read_bytes / (double)file_size * 1000.0f;

   PostMessage(WM_WORK_PROGRESS, (WPARAM)info);
}

void CMainDlg::update_files_process(std::string file, int count, int index)
{

}

LRESULT CMainDlg::OnCheckProgress(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
   msg_info* info = (msg_info*)wParam;
   if (!info)
      return 0;

   if (m_status != WM_CHECK_PROGRESS)
   {
      m_status = WM_CHECK_PROGRESS;
      m_single_progress.SetPos(0);
      m_main_progress.SetPos(0);
   }

   if (info->text != m_single_str)
   {
      m_single_str = info->text;
      m_single_text.SetWindowText(m_single_str);
   }

   m_single_progress.SetPos(info->single_pos);
   m_main_progress.SetPos(info->main_pos);

   delete info;

   return 0;
}

LRESULT CMainDlg::OnWorkProgress(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
   msg_info* info = (msg_info*)wParam;
   if (!info)
      return 0;

   if (m_status != WM_WORK_PROGRESS)
   {
      m_status = WM_WORK_PROGRESS;
      m_single_progress.SetPos(0);
      m_main_progress.SetPos(0);
   }

   if (info->text != m_single_str)
   {
      m_single_str = info->text;
      m_single_text.SetWindowText(m_single_str);
   }

   m_single_progress.SetPos(info->single_pos);
   m_main_progress.SetPos(info->main_pos);

   delete info;

   return 0;
}

LRESULT CMainDlg::OnUpdateProgress(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
   return 0;
}
