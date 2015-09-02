// RegistryHelper.h

#pragma once

#include <windows.h>
#include <string>
#include <tchar.h>
#include <vector>

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#ifndef stdstring
// Typedef for string
#if defined(_UNICODE) || defined(UNICODE)
typedef std::wstring stdstring;
typedef std::vector<std::wstring> stdvstring; 
#else
typedef std::string stdstring;
typedef std::vector<std::string> stdvstring; 
#endif
#endif

//typedef std::basic_string<TCHAR> stdstring;
//typedef std::vector<TCHAR> stdvstring; 


class RegistryHelper
{
public:
	static BOOL RegKeyExists(HKEY type, LPCTSTR subKey);
	static BOOL RegValueExists(HKEY type, LPCTSTR subKey, LPCTSTR valueName);

	static int RegGetSubKeyCount(HKEY type, LPCTSTR subKey);
	static int RegGetValueCount(HKEY type, LPCTSTR subKey);

	static int RegGetSubKeyList(HKEY type, LPCTSTR subKey, stdvstring &list);
	static int RegGetValueKeyList(HKEY type, LPCTSTR subKey, stdvstring &list);

	static BOOL RegRead(HKEY type, LPCTSTR subKey, LPCTSTR valueName, LPTSTR value);
	static BOOL RegRead(HKEY type, LPCTSTR subKey, LPCTSTR valueName, DWORD &value);

	static BOOL RegWrite(HKEY type, LPCTSTR subKey, LPCTSTR valueName, LPCTSTR value);
	static BOOL RegWrite(HKEY type, LPCTSTR subKey, LPCTSTR valueName, DWORD value);

	static BOOL RegCreate(HKEY type, LPCTSTR subKey);
	static BOOL RegDelete(HKEY type, LPCTSTR subKey, const TCHAR *valueName);
	static BOOL RegSave(HKEY type, LPCTSTR subKey, LPCTSTR lpszFileName);
	static BOOL RegRestore(HKEY type, LPCTSTR subKey, LPCTSTR lpszFileName, DWORD dwFlags);
	static BOOL RegLoad(HKEY type, LPCTSTR subKey, LPCTSTR lpszFileName);

	static BOOL RegSetStartup(HKEY type, LPCTSTR lpszFileName, LPCTSTR arg = _T(""), BOOL bDelete = FALSE);
	static BOOL RegSetAppPath(LPCTSTR lpszFileName, BOOL bDelete = FALSE);
};
