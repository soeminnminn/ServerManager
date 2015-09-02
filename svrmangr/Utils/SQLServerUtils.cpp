// SQLServerUtils.cpp

#include "stdafx.h"
#include "SQLServerUtils.h"


BOOL SQLServerUtils::IsWindowsNT()
{
	OSVERSIONINFO m_OSInfo = {NULL};
	m_OSInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	::GetVersionEx(&m_OSInfo);

	return ((m_OSInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) && (m_OSInfo.dwMajorVersion > 3));
};

int SQLServerUtils::GetLanGroupName(TCHAR* lanGroupName)
{
	int iRC = 0;

	DWORD dwLevel = 100;
	LPWKSTA_INFO_100 pBuf = NULL;
	NET_API_STATUS nStatus = NERR_Success;

	nStatus = ::NetWkstaGetInfo(NULL, dwLevel, (LPBYTE *)&pBuf);
 
	if (nStatus == NERR_Success)
	{
#if defined(_UNICODE) || defined(UNICODE)
		_tcscpy(lanGroupName, pBuf->wki100_langroup);
#else
		char* lpAscii = NULL;
		size_t lenIn = wcslen(pBuf->wki100_langroup);
		int nConvertedLen = ::WideCharToMultiByte(CP_UTF8, 0, pBuf->wki100_langroup, (int)lenIn + 1, NULL, NULL, NULL, NULL);

		lpAscii = new char[nConvertedLen];
		lpAscii[0] = 0;

		::WideCharToMultiByte(CP_UTF8, 0, pBuf->wki100_langroup, (int)lenIn + 1, lpAscii, nConvertedLen, NULL, NULL);

		strcpy(lanGroupName, lpAscii);
#endif
	}
	else
		iRC = ::GetLastError();
 
	// Free the allocated memory.
	if (pBuf != NULL) ::NetApiBufferFree(pBuf);
	return iRC;
};

int SQLServerUtils::GetSQLServerList(stdvstring &list)
{
	int iRC = 0;
	LPSERVER_INFO_101 pBuf = NULL;
	LPSERVER_INFO_101 pTmpBuf;
	DWORD dwLevel = 101;
	DWORD dwPrefMaxLen = -1;
	DWORD dwEntriesRead = 0;
	DWORD dwTotalEntries = 0;
	DWORD dwTotalCount = 0;
	DWORD dwServerType = SV_TYPE_SQLSERVER;//SV_TYPE_SERVER; (all servers)
	DWORD dwResumeHandle = 0;
	NET_API_STATUS nStatus;
	TCHAR szServerName[MAX_PATH] = {NULL};

	nStatus = ::NetServerEnum(NULL, dwLevel, (LPBYTE *) &pBuf, dwPrefMaxLen, &dwEntriesRead,
							&dwTotalEntries, dwServerType, NULL, &dwResumeHandle);
	
	// If the call succeeds,
	if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA))
	{
		if ((pTmpBuf = pBuf) != NULL)
		{
			// Loop through the entries
			for (DWORD i = 0; i < dwEntriesRead; i++)
			{
				if (pTmpBuf == NULL) break;
				
#if defined(_UNICODE) || defined(UNICODE)
				_tcscpy(szServerName, pTmpBuf->sv101_name);
#else
				char* lpAscii = NULL;
				size_t lenIn = wcslen(pTmpBuf->sv101_name);
				int nConvertedLen = ::WideCharToMultiByte(CP_UTF8, 0, pTmpBuf->sv101_name, (int)lenIn + 1, NULL, NULL, NULL, NULL);

				lpAscii = new char[nConvertedLen];
				lpAscii[0] = 0;

				::WideCharToMultiByte(CP_UTF8, 0, pTmpBuf->sv101_name, (int)lenIn + 1, lpAscii, nConvertedLen, NULL, NULL);

				strcpy(szServerName, lpAscii);
#endif
				
				list.push_back(szServerName);
				
				pTmpBuf++;
				dwTotalCount++;
			}

			if (nStatus == ERROR_MORE_DATA)
			{
				// More entries available!!!
			}
		}
	}
	else
		iRC = ::GetLastError();

	// Free the allocated buffer.
	if (pBuf != NULL) ::NetApiBufferFree(pBuf);

	return iRC;
};

int SQLServerUtils::GetSQLInstances(stdvstring &list)
{
	TCHAR szTemp[MAX_PATH + 1] = {NULL};
	stdvstring regList;

	int iCount = RegistryHelper::RegGetValueKeyList(HKEY_LOCAL_MACHINE, KEY_SOFTWARE_SQL_INSTANCE, regList);
	if(iCount > 0)
	{
		list.clear();

		for(int i = 0; i < iCount; i++)
		{
			if(_tcscmp(regList[i].c_str(), DEFAULT_INSTANCE) == 0)
			{
				_tcscpy(szTemp, DEFAULT_INSTANCE);
			}
			else
			{
				_tcscpy(szTemp, SUB_INSTANCE);
				_tcscat(szTemp, regList[i].c_str());
			}
			list.push_back(szTemp);
		}
	}

	return iCount;
};

BOOL SQLServerUtils::GetSQLServicesList(const TCHAR* hostName, stdvstring &listServices, stdvstring &listDisplay)
{
	TCHAR szServiceName[MAX_PATH + 1] = {NULL};
	TCHAR szInstanceName[MAX_PATH + 1] = {NULL};
	TCHAR szDisplayName[MAX_PATH + 1] = {NULL};

	stdvstring SQLInstancesList;
	SQLInstancesList.clear();
	int iInstCount = GetSQLInstances(SQLInstancesList);

	listServices.clear();
	listDisplay.clear();

	if (iInstCount > 0)
	{
		for(int iInst = 0; iInst < (int)iInstCount; iInst++)
		{
			FindInstanceName(SQLInstancesList[iInst].c_str(), szInstanceName);

			for(int i=0; i<nDEFSERVICES; i++)
			{
				_tcscpy(szServiceName, _DEFAULTSERVICESNAME[i].serviceName);
				if(!ServicesHelper::GetService_DisplayName(hostName, szServiceName, szDisplayName, MAX_PATH))
				{
					if (_tcscmp(szInstanceName, DEFAULT_INSTANCE) == 0)
					{
						listServices.push_back(DEFAULT_DISPAYNAME);
					}
					else
					{
						listServices.push_back(szServiceName);
					}
					listDisplay.push_back(szDisplayName);
				} 
				else if (_tcscmp(szInstanceName, DEFAULT_INSTANCE) != 0)
				{
					_tcscat(szServiceName, _T("$"));
					_tcscat(szServiceName, szInstanceName);

					if(!ServicesHelper::GetService_DisplayName(hostName, szServiceName, szDisplayName, MAX_PATH))
					{
						listServices.push_back(szServiceName);
						listDisplay.push_back(szDisplayName);
					} 
				}
			}
		}
	}

	/*
	for(int i=0; i<nDEFSERVICES; i++)
	{
		_tcscpy(szServiceName, _DEFAULTSERVICESNAME[i].serviceName);
		bool bIsSQLSvrService = (_tcscmp(szServiceName, DEFAULT_INSTANCE) == 0);
		bool bIsDefInst = false;

		if(bIsSQLSvrService)
		{
			stdvstring SQLInstancesList;
			SQLInstancesList.clear();
			int iCount = GetSQLInstances(SQLInstancesList);

			if(iCount > 0)
			{
				for(int iInst = 0; iInst < (int)iCount; iInst++)
				{
					_tcscpy(szServiceName, SQLInstancesList[iInst].c_str());

					if(!ServicesHelper::GetService_DisplayName(hostName, szServiceName, szDisplayName, MAX_PATH))
					{
						bIsDefInst = (bIsDefInst || (_tcscmp(szDisplayName, DEFAULT_DISPAYNAME) == 0));
						listServices.push_back(szServiceName);
						listDisplay.push_back(szDisplayName);
					}
				}
			}
		}

		if(!(bIsSQLSvrService && bIsDefInst))
		{
			if(!ServicesHelper::GetService_DisplayName(hostName, szServiceName, szDisplayName, MAX_PATH))
			{
				listServices.push_back(_DEFAULTSERVICESNAME[i].serviceName);
				listDisplay.push_back(_DEFAULTSERVICESNAME[i].displayName);
			}
		}
	}*/

	return ((listServices.size() > 0) && (listDisplay.size() > 0));
};

void SQLServerUtils::FindInstanceName(const TCHAR* instance, TCHAR* namedInstance)
{
	if(!instance) return;

	TCHAR str[MAX_PATH] = {NULL};
	TCHAR* delims[] = {_T("\\"), _T("$"), _T(".")};
	TCHAR *result = NULL;

	_tcscpy(str, instance);
	size_t len = _tcslen(str);

	for(int i=0; i<3; i++)
	{
		result = _tcstok(str, delims[i]);
		if(result != NULL && _tcslen(result) != len) 
		{
			// Get Named Instance
			result = _tcstok(NULL, delims[i]);
			if(result != NULL) 
			{
				_tcscpy(namedInstance, result);
				return;
			}
		}
	}

	_tcscpy(namedInstance, instance);
}

void SQLServerUtils::FindServiceName(const TCHAR* service, TCHAR* namedService)
{
	if(!service) return;

	TCHAR str[MAX_PATH] = {NULL};
	TCHAR* delims[] = {_T("\\"), _T("$"), _T(".")};
	TCHAR *result = NULL;

	_tcscpy(str, service);
	size_t len = _tcslen(str);

	for(int i=0; i<3; i++)
	{
		result = _tcstok(str, delims[i]);
		if(result != NULL && _tcslen(result) != len) 
		{
			_tcscpy(namedService, result);
			return;
		}
	}

	_tcscpy(namedService, service);
}

void SQLServerUtils::SplitInstanceName(const TCHAR* instance, TCHAR* hostname, TCHAR* namedInstance)
{
	if(!instance) return;

	TCHAR str[MAX_PATH] = {NULL};
	TCHAR delims[] = _T("\\");
	TCHAR *result = NULL;

	_tcscpy(str, instance);
	size_t len = _tcslen(str);
	result = _tcstok(str, delims);
	if(result != NULL && _tcslen(result) != len) 
	{
		// Get Host Name
		_tcscpy(hostname, result);
		// Get Named Instance
		result = _tcstok(NULL, delims);
		if(result != NULL) 
		{
			_tcscpy(namedInstance, result);
			return;
		}
	}

	_tcscpy(namedInstance, instance);
};
