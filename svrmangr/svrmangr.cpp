// svrmangr.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "svrmangr.h"
#include "MainDialog.h"

#include <tlhelp32.h>
#include <psapi.h>
#pragma comment(lib, "Psapi.lib")

// Global Variables:
HINSTANCE hInst;								// current instance

BOOL CheckInstance();


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
 	MSG msg = {0};
	
	INITCOMMONCONTROLSEX icex;
	DWORD dwICC = ICC_ANIMATE_CLASS | \
							ICC_BAR_CLASSES | \
							ICC_COOL_CLASSES | \
							ICC_DATE_CLASSES | \
							ICC_LISTVIEW_CLASSES | \
							ICC_PROGRESS_CLASS | \
							ICC_TAB_CLASSES | \
							ICC_TREEVIEW_CLASSES | \
							ICC_UPDOWN_CLASS | \
							ICC_USEREX_CLASSES | \
							ICC_WIN95_CLASSES;

	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC  = dwICC;
	::InitCommonControlsEx(&icex);

	if(CheckInstance() == TRUE) return FALSE;

	MainDialog mainDlg;
	return (int)mainDlg.Create(hInstance, NULL);
};

BOOL CheckInstance()
{
	TCHAR appName[MAX_PATH+1] = {0};
	::GetModuleFileName(NULL, appName, MAX_PATH);
	TCHAR *strExeName = ::PathFindFileName(appName);
	int iCount = 0;

	PROCESSENTRY32 uProcess = {NULL};
	uProcess.dwSize = sizeof(uProcess);

	HANDLE hSnapShot = ::CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0L);

	if(::Process32First(hSnapShot, &uProcess) != TRUE) return FALSE;
	do
	{
		if(_tcscmp(uProcess.szExeFile, strExeName) == 0) iCount++;
	}
	while(::Process32Next(hSnapShot, &uProcess));

	::CloseHandle(hSnapShot);
	return ((iCount > 1) ? TRUE : FALSE);
};

BOOL ReduceMemory()
{
	TCHAR appName[MAX_PATH+1] = {0};
	::GetModuleFileName(NULL, appName, MAX_PATH);
	TCHAR *strExeName = ::PathFindFileName(appName);

	PROCESSENTRY32 uProcess = {NULL};
	uProcess.dwSize = sizeof(PROCESSENTRY32);

	HANDLE hSnapShot = ::CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0L);

	BOOL ret = ::Process32First(hSnapShot, &uProcess);
	if(!ret) return ret;

	do {
		if(_tcscmp(uProcess.szExeFile, strExeName) == 0)
		{
			if(uProcess.th32ProcessID > 0)
			{
				HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, uProcess.th32ProcessID);
				ret = ::EmptyWorkingSet(hProcess);
				::CloseHandle(hProcess);
			}
			break;
		}
	} while (::Process32Next(hSnapShot, &uProcess));
	::CloseHandle(hSnapShot);

	return ret;
};
