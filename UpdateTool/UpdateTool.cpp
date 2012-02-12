// Tester.cpp : main source file for Tester.exe
//

#include "stdafx.h"

#include "resource.h"

#include "aboutdlg.h"
#include "MainDlg.h"

CAppModule _Module;

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMainDlg dlgMain;

	if(dlgMain.Create(NULL) == NULL)
	{
		AtlTrace(_T("Main dialog creation failed!\n"));
		return 0;
	}

	dlgMain.ShowWindow(nCmdShow);

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	//////////////////////////////////////////////////////////////////////////
	// 设置重启时等待原进程退出.
	HANDLE hMutex;
	do
	{
		hMutex = ::CreateMutex(NULL, TRUE, _T("TEST_REBOOT"));
		DWORD dwLastError = ::GetLastError();
		if (dwLastError == ERROR_ALREADY_EXISTS || dwLastError == ERROR_ACCESS_DENIED)
		{
			DWORD dwWaitRet = ::WaitForSingleObject(hMutex, 3000);
			if (dwWaitRet == WAIT_ABANDONED || dwWaitRet == WAIT_TIMEOUT)
			{
				::MessageBox(NULL, _T("启动了多个实例."), _T("初始化失败."), MB_OK | MB_ICONWARNING);
				return 0;
			}
		}
		else
		{
			break;
		}
	} while (1);

	HRESULT hRes = ::CoInitialize(NULL);
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls

	setlocale(LC_ALL, "chs");

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();
	::CoUninitialize();

	CloseHandle(hMutex);

	return nRet;
}
