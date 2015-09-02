// RegistryHelper.cpp

#include "stdafx.h"
#include "RegistryHelper.h"

#define CLASS_NAME_LENGTH 255

BOOL RegistryHelper::RegKeyExists(HKEY type, LPCTSTR subKey) 
{
	HKEY hKey = 0;
	if(::RegOpenKeyEx(type, subKey, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		::RegCloseKey(hKey);
		return TRUE;
	}
	
	return FALSE;
};

BOOL RegistryHelper::RegValueExists(HKEY type, LPCTSTR subKey, LPCTSTR valueName) 
{
	BOOL bError = FALSE;
	HKEY hKey = 0;

	if(::RegOpenKeyEx(type, (LPCTSTR)subKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		bError = (::RegQueryValueEx(hKey, valueName, NULL, NULL, NULL, NULL) != ERROR_SUCCESS);
		::RegCloseKey(hKey);
	}
	
	return !bError;
};

int RegistryHelper::RegGetSubKeyCount(HKEY type, LPCTSTR subKey)
{
	/* Call this function to determine the number of subkeys. the function returns -1 on error */
	HKEY hKey;

	if (::RegOpenKeyEx(type, (LPCTSTR)subKey, 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS) return -1;

	LONG lResult;
	DWORD dwSubKeyCount, dwValueCount, dwClassNameLength, dwMaxSubKeyName, dwMaxValueName, dwMaxValueLength;
	FILETIME ftLastWritten;

	TCHAR szClassBuffer[CLASS_NAME_LENGTH] = {0};
		
	dwClassNameLength = CLASS_NAME_LENGTH;
	lResult = ::RegQueryInfoKey(hKey, szClassBuffer, &dwClassNameLength,
					NULL, &dwSubKeyCount, &dwMaxSubKeyName, NULL, &dwValueCount,
					&dwMaxValueName, &dwMaxValueLength, NULL, &ftLastWritten);
				
	::RegCloseKey(hKey);
	if (lResult != ERROR_SUCCESS) return -1;

	return (int)dwSubKeyCount;
};

int RegistryHelper::RegGetValueCount(HKEY type, LPCTSTR subKey)
{
	HKEY hKey;

	if (::RegOpenKeyEx(type, (LPCTSTR)subKey, 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS) return -1;

	LONG lResult;
	DWORD dwSubKeyCount, dwValueCount, dwClassNameLength, dwMaxSubKeyName, dwMaxValueName, dwMaxValueLength;
	FILETIME ftLastWritten;

	TCHAR szClassBuffer[CLASS_NAME_LENGTH] = {0};
		
	dwClassNameLength = CLASS_NAME_LENGTH;
	lResult = ::RegQueryInfoKey(hKey, szClassBuffer, &dwClassNameLength,
					NULL, &dwSubKeyCount, &dwMaxSubKeyName, NULL, &dwValueCount,
					&dwMaxValueName, &dwMaxValueLength, NULL, &ftLastWritten);
				
	::RegCloseKey(hKey);
	if (lResult != ERROR_SUCCESS) return -1;

	return (int)dwValueCount;
};

int RegistryHelper::RegGetSubKeyList(HKEY type, LPCTSTR subKey, stdvstring &list) 
{
	BOOL bError = FALSE;
	HKEY hKey = 0;
	int keyCount = RegGetSubKeyCount(type, subKey);
	if(keyCount < 1) return 0;
	
	if(::RegOpenKeyEx(type, (LPCTSTR)subKey, 0, KEY_READ | KEY_ENUMERATE_SUB_KEYS, &hKey) == ERROR_SUCCESS)
	{
		DWORD dwIndex = 0;
		TCHAR szGuid[MAX_PATH] = {0};
		list.clear();

        while (::RegEnumKey(hKey, dwIndex++, szGuid, MAX_PATH) == ERROR_SUCCESS)
        {
			if((int)dwIndex == keyCount) break;

			HKEY hGuid = 0;
			if (::RegOpenKeyEx(hKey, szGuid, 0, KEY_READ, &hGuid) != ERROR_SUCCESS) continue;

			//list.push_back(szGuid);
			::RegCloseKey(hGuid);
		}
		
		::RegCloseKey(hKey);
	}
	else
		bError = TRUE;

	return (int)keyCount;
};

int RegistryHelper::RegGetValueKeyList(HKEY type, LPCTSTR subKey, stdvstring &list) 
{
	BOOL bError = FALSE;
	HKEY hKey = 0;

    int enumIndex = 0;
    DWORD dwNameLen = 0;
    DWORD dwType = 0;
    LONG lResult = 0;

	int valueCount = RegGetValueCount(type, subKey);
	if(valueCount < 1) return 0;

	lResult = ::RegOpenKeyEx(type, subKey, 0, KEY_READ, &hKey);
	if(lResult != ERROR_SUCCESS) return 0;

	list.clear();

	do {
		if(enumIndex == valueCount) break;
        lResult = ERROR_NOT_ENOUGH_MEMORY;

        do {
			TCHAR cValueName[MAX_PATH+1];
            dwNameLen = MAX_PATH+1;
            
			lResult = ::RegEnumValue(hKey, enumIndex, cValueName, &dwNameLen, NULL, &dwType, NULL, 0);
            if ((lResult != ERROR_SUCCESS) && (lResult != ERROR_NO_MORE_ITEMS)) break;

			list.push_back(cValueName);

        } while (lResult == ERROR_NOT_ENOUGH_MEMORY);

        if (lResult == ERROR_NO_MORE_ITEMS) break;

        // unknown error return
        if (lResult != ERROR_SUCCESS) break;

        // read next value
        enumIndex++;
    } while (true);

	::RegCloseKey(hKey);
    if (lResult != ERROR_NO_MORE_ITEMS) bError = TRUE;

	// Remove Last One
	//if(list.size() > 1) list.resize(list.size() - 1);

	return (int)valueCount;
};

BOOL RegistryHelper::RegRead(HKEY type, LPCTSTR subKey, LPCTSTR valueName, LPTSTR value) 
{
	BOOL bError = FALSE;
	HKEY hKey = 0;
	TCHAR data[1024] = {0};
	DWORD dataSize = sizeof(data);
	DWORD dwType = REG_SZ;
	
	if(::RegOpenKeyEx(type, (LPCTSTR)subKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		bError = (::RegQueryValueEx(hKey, (LPCTSTR)valueName, NULL, &dwType, (LPBYTE)(LPTSTR)&data, &dataSize) != ERROR_SUCCESS);
		::RegCloseKey(hKey);
	}
	else
		bError = TRUE;


	if(!bError) _tcscpy(value, data);
	return !bError;
};

BOOL RegistryHelper::RegRead(HKEY type, LPCTSTR subKey, LPCTSTR valueName, DWORD &value) 
{
	BOOL bError = FALSE;
	HKEY hKey = 0;
	DWORD dwType = REG_DWORD;
	DWORD dataSize = sizeof(DWORD);
	value = 0;

#pragma warning( disable: 4311 )
#pragma warning( disable: 4312 )
	if(::RegOpenKeyEx(type, (LPCTSTR)subKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		bError = (::RegQueryValueEx(hKey, (LPCTSTR)valueName, NULL, &dwType, (BYTE*)(DWORD)&value, &dataSize) != ERROR_SUCCESS);
		::RegCloseKey(hKey);
	}
	else
		bError = TRUE;
#pragma warning( default: 4311 )
#pragma warning( default: 4312 )

	return !bError;
};

BOOL RegistryHelper::RegWrite(HKEY type, LPCTSTR subKey, LPCTSTR valueName, LPCTSTR value) 
{
	BOOL bError = FALSE;
	HKEY hKey = 0;
	DWORD dwType = REG_SZ;

	if(::RegOpenKeyEx(type, (LPCTSTR)subKey, 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
	{
		bError = (::RegSetValueEx(hKey, (LPCTSTR)valueName, 0, dwType, (LPBYTE)(LPCTSTR)value, (DWORD)_tcslen(value)) != ERROR_SUCCESS);
		if(!bError) ::RegFlushKey(hKey);

		::RegCloseKey(hKey);
	}
	else
		bError = TRUE;

	return !bError;
};

BOOL RegistryHelper::RegWrite(HKEY type, LPCTSTR subKey, LPCTSTR valueName, DWORD value) 
{
	BOOL bError = FALSE;
	HKEY hKey = 0;
	DWORD dwType = REG_DWORD;

	if(::RegOpenKeyEx(type, (LPCTSTR)subKey, 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
	{
		bError = (::RegSetValueEx(hKey, (LPCTSTR)valueName, 0, dwType, (LPBYTE)&value, sizeof(value)) != ERROR_SUCCESS);
		if(!bError) ::RegFlushKey(hKey);

		::RegCloseKey(hKey);
	}
	else
		bError = TRUE;

	return !bError;
};

BOOL RegistryHelper::RegCreate(HKEY type, LPCTSTR subKey) 
{
	BOOL bError = FALSE;
	HKEY hKey = 0;
	if(::RegOpenKeyEx(type, NULL, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		bError = (::RegCreateKeyEx(type, (LPCTSTR)subKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) != ERROR_SUCCESS);
		::RegCloseKey(hKey);
	}
	else
		bError = TRUE;

	return !bError;
};

BOOL RegistryHelper::RegDelete(HKEY type, LPCTSTR subKey, const TCHAR *valueName) 
{
	BOOL bError = FALSE;
	HKEY hKey = 0;
	TCHAR data[MAX_PATH] = {0};
	DWORD dataSize = sizeof(data);

	if(_tcscmp(valueName, _T("")) == 0 || valueName[0] == '\0')
		bError = (::RegDeleteKey(hKey, (LPCTSTR)subKey) != ERROR_SUCCESS);
	else
	{
		if(::RegOpenKeyEx(type, (LPCTSTR)subKey, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
			bError = (::RegDeleteValue(hKey, (LPCTSTR)valueName) != ERROR_SUCCESS);
		else
			bError = TRUE;

		::RegCloseKey(hKey);
	}
	return !bError;
};

BOOL RegistryHelper::RegSave(HKEY type, LPCTSTR subKey, LPCTSTR lpszFileName) 
{
	BOOL bError = FALSE;
	HKEY hKey = 0;
	if(::RegOpenKeyEx(type, (LPCTSTR)subKey, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		bError = (::RegSaveKey(hKey, lpszFileName, NULL) != ERROR_SUCCESS);
		::RegCloseKey(hKey);
	}
	else
		bError = TRUE;

	return !bError;
};

BOOL RegistryHelper::RegRestore(HKEY type, LPCTSTR subKey, LPCTSTR lpszFileName, DWORD dwFlags) 
{
	BOOL bError = FALSE;
	HKEY hKey = 0;
	if(::RegOpenKeyEx(type, (LPCTSTR)subKey, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		bError = (::RegRestoreKey(hKey, lpszFileName, dwFlags) != ERROR_SUCCESS);
		::RegCloseKey(hKey);
	}
	else
		bError = TRUE;

	return !bError;
};

BOOL RegistryHelper::RegLoad(HKEY type, LPCTSTR subKey, LPCTSTR lpszFileName) 
{
	BOOL bError = FALSE;
	HKEY hKey = 0;
	if(::RegOpenKeyEx(type, (LPCTSTR)subKey, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		bError = (::RegLoadKey((HKEY)type, subKey, lpszFileName) != ERROR_SUCCESS);
		::RegCloseKey(hKey);
	}
	else
		bError = TRUE;

	return !bError;
};

BOOL RegistryHelper::RegSetStartup(HKEY type, LPCTSTR lpszFileName, LPCTSTR arg, BOOL bDelete) 
{
	if(type != HKEY_CURRENT_USER && type != HKEY_LOCAL_MACHINE) return FALSE;
	if(!::PathFileExists(lpszFileName)) return FALSE;

	const TCHAR subKey[] = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run");
	BOOL bError = FALSE;
	HKEY hKey = 0;
	DWORD dwType = REG_SZ;
	TCHAR szPath[255] = {0};
	_tcscpy(szPath, lpszFileName);
	
	TCHAR *valueName = ::PathFindFileName(lpszFileName);
	::PathRemoveExtension(valueName);

	if(::RegOpenKeyEx(type, (LPCTSTR)subKey, 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
	{
		if(bDelete)
			bError = (::RegDeleteValue(hKey, (LPCTSTR)valueName) != ERROR_SUCCESS);
		else
		{
			if(_tcscmp(arg, _T("")) != 0) 
			{
				_tcscat(szPath, _T(" "));
				_tcscat(szPath, arg);
			}
			bError = (::RegSetValueEx(hKey, (LPCTSTR)valueName, 0, dwType, (LPBYTE)(LPCTSTR)szPath, (DWORD)_tcslen(szPath)) != ERROR_SUCCESS);
			if(!bError) ::RegFlushKey(hKey);
		}
		::RegCloseKey(hKey);
	}
	else
		bError = TRUE;

	return !bError;
};

BOOL RegistryHelper::RegSetAppPath(LPCTSTR lpszFileName, BOOL bDelete) 
{
	if(!::PathFileExists(lpszFileName)) return FALSE;

	HKEY type = HKEY_LOCAL_MACHINE;
	const TCHAR subKey[] = _T("Software\\Microsoft\\Windows\\CurrentVersion\\App Paths");
	BOOL bError = FALSE;
	HKEY hKey = 0;
	DWORD dwType = REG_SZ;
	TCHAR *valueName = ::PathFindFileName(lpszFileName);

	if(::RegOpenKeyEx(type, (LPCTSTR)subKey, 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
	{
		TCHAR szSubKey[255] = {0};
		_stprintf(szSubKey, _T("%s\\%s"), subKey, valueName);

		if(bDelete)
			bError = (::RegDeleteKey(hKey, (LPCTSTR)szSubKey) != ERROR_SUCCESS);
		else
		{
			bError = (::RegCreateKeyEx(type, (LPCTSTR)szSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) != ERROR_SUCCESS);
			if(!bError) bError = (::RegSetValueEx(hKey, NULL, 0, dwType, (LPBYTE)(LPCTSTR)lpszFileName, (DWORD)_tcslen(lpszFileName)) != ERROR_SUCCESS);
			if(!bError) bError = (::RegSetValueEx(hKey, _T("Path"), 0, dwType, (LPBYTE)(LPCTSTR)lpszFileName, (DWORD)_tcslen(lpszFileName)) != ERROR_SUCCESS);
			if(!bError) ::RegFlushKey(hKey);
		}
		::RegCloseKey(hKey);
	}
	else
		bError = TRUE;

	return !bError;
};

