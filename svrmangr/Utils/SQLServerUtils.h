// SQLServerUtils.h

#pragma once

#include <windows.h>
#include <string>
#include <tchar.h>
#include <vector>

#include <lm.h>
#pragma comment(lib, "netapi32.lib")

#include "RegistryHelper.h"
#include "ServicesHelper.h"

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

//----------------------------------
// Structure to define Services
//----------------------------------
const struct SERVICESNAME {
	const TCHAR * serviceName;
	const TCHAR * displayName;
} extern _DEFAULTSERVICESNAME[] = 
{
	{ _T("SQLServerAgent"), _T("SQL Server Agent") },
	{ _T("SQLAgent"), _T("SQL Server Agent") },
    { _T("MSSQLServerOLAPService"), _T("Analysis Server") },
	{ _T("MSDTC"), _T("Distributed Transaction Coordinator") }, 
    { _T("MSFTESQL"), _T("Full-Text Search") },
	{ _T("MSDTSServer"), _T("Integration Services") },
	{ _T("NSService"), _T("Notification Services") },
	{ _T("ReportServer "), _T("Report Server") },
	{ _T("MSSQLSERVER"), _T("SQL Server") },
	{ _T("MSSQL"), _T("SQL Server") },
	{ _T("MSSQLServerADHelper"), _T("SQL Server AD Helper") },
	{ _T("SQLBrowser"), _T("SQL Server Browser") },
	{ _T("SQLWriter"), _T("SQL Writer") }
};
#define nDEFSERVICES (sizeof(_DEFAULTSERVICESNAME)/sizeof(SERVICESNAME))

#define DEFAULT_INSTANCE			(_T("MSSQLSERVER")) 	// Default Instance name of SQL Server
#define DEFAULT_DISPAYNAME		(_T("SQL Server (MSSQLSERVER)")) 	// Default Instance Display Name of SQL Server
#define SUB_INSTANCE					(_T("MSSQL$")) 	// Default Instance name of SQL Server

#define KEY_SOFTWARE_SQL_INSTANCE (_T("SOFTWARE\\Microsoft\\Microsoft SQL Server\\Instance Names\\SQL"))

class SQLServerUtils
{
public:
	SQLServerUtils() {};
	virtual ~SQLServerUtils() {};

	static BOOL IsWindowsNT();

	static int GetLanGroupName(TCHAR* lanGroupName);

	static int GetSQLServerList(stdvstring &list);

	static int GetSQLInstances(stdvstring &list);

	static BOOL GetSQLServicesList(const TCHAR* hostName, stdvstring &listServices, stdvstring &listDisplay);

	static void FindInstanceName(const TCHAR* instance, TCHAR* namedInstance);

	static void FindServiceName(const TCHAR* service, TCHAR* namedService);

	static void SplitInstanceName(const TCHAR* instance, TCHAR* hostname, TCHAR* namedInstance);
};

