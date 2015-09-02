// Settings.cpp

#include "stdafx.h"
#include "MainDialog.h"

/////////////////////////////////////////////////////////////////////////////////////////
/// Get the Configuration file
inline void GetINIFile(TCHAR* filePath)
{
	TCHAR iniFilePath[MAX_PATH + 1] = {0};
	::GetModuleFileName(NULL, iniFilePath, MAX_PATH);
	::PathRemoveExtension(iniFilePath);
	_tcscat(iniFilePath, _T(".ini"));

	if (::PathFileExists(iniFilePath) == FALSE)
	{
		::CloseHandle(::CreateFile(iniFilePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL));
	}

	_tcscpy(filePath, iniFilePath);
};

/////////////////////////////////////////////////////////////////////////////////////////
/// Default Selection
inline void SetDefaultSelection(const TCHAR* value)
{
	TCHAR iniFilePath[MAX_PATH + 1] = {NULL};
	TCHAR szTemp[MAX_PATH + 1] = {NULL};
	GetINIFile(iniFilePath);

	_tcscpy(szTemp, value);
	::WritePrivateProfileString( _T("Main"), _T("DefaultSelection"), szTemp, iniFilePath);
};

inline void GetDefaultSelection(TCHAR* value)
{
	TCHAR iniFilePath[MAX_PATH + 1] = {NULL};
	TCHAR szTemp[MAX_PATH + 1] = {NULL};
	GetINIFile(iniFilePath);
	::GetPrivateProfileString(_T("Main"), _T("DefaultSelection"), _T(""), szTemp, (DWORD)MAX_PATH, iniFilePath);

	_tcscpy(value, szTemp);
};


/////////////////////////////////////////////////////////////////////////////////////////
/// Polling Intervel
inline void SetPollingIntervel(int value)
{
	TCHAR iniFilePath[MAX_PATH + 1] = {0};
	TCHAR szTemp[MAX_PATH + 1] = {0};
	GetINIFile(iniFilePath);

	value /= MUL_POLLINGINTERVAL;
	_itot(value, szTemp, 10);
	::WritePrivateProfileString( _T("Main"), _T("PollingIntervel"), szTemp, iniFilePath);
};

inline int GetPollingIntervel()
{
	TCHAR iniFilePath[MAX_PATH + 1] = {0};
	TCHAR szTemp[MAX_PATH + 1] = {0};
	GetINIFile(iniFilePath);
	int iTemp = (int)::GetPrivateProfileInt(_T("Main"), _T("PollingIntervel"), 5, iniFilePath);

	return iTemp * MUL_POLLINGINTERVAL;
};

/////////////////////////////////////////////////////////////////////////////////////////
/// Verify Action
inline void SetVerifyAction(BOOL value)
{
	TCHAR iniFilePath[MAX_PATH + 1] = {0};
	TCHAR szTemp[MAX_PATH + 1] = {0};
	GetINIFile(iniFilePath);

	if(value) 
		_tcscpy(szTemp, _T("true"));
	else
		_tcscpy(szTemp, _T("false"));

	::WritePrivateProfileString( _T("Main"), _T("VerifyAction"), szTemp, iniFilePath);
};

inline BOOL GetVerifyAction()
{
	TCHAR iniFilePath[MAX_PATH + 1] = {0};
	TCHAR szTemp[MAX_PATH + 1] = {0};
	DWORD dwSize = (DWORD)(MAX_PATH + 1);
	
	GetINIFile(iniFilePath);
	
	::GetPrivateProfileString(_T("Main"), _T("VerifyAction"), _T(""), szTemp, MAX_PATH, iniFilePath);
	return ((_tcscmp(szTemp, _T("true")) == 0) ? TRUE : FALSE);
};

/////////////////////////////////////////////////////////////////////////////////////////
/// Read/Write Setting
void MainDialog::LoadSetting()
{
	GetDefaultSelection(defaultSelection);
	pollingInterval = GetPollingIntervel();
	verifyAction = GetVerifyAction();
};

void MainDialog::SaveSetting()
{
	SetDefaultSelection(defaultSelection);
	SetPollingIntervel(pollingInterval);
	SetVerifyAction(verifyAction);
};

