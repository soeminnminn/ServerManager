// ServicesHelper.h

#pragma once

#include <windows.h>
#include <tchar.h>

#define SERVICE_CONTROL_START	0x00000000

typedef struct _DESCRIPTION
{
	DWORD typeId;
	LPTSTR szDescription;
} DESCRIPTION, *PDESCRIPTION;

// Description of Type of Service
static DESCRIPTION TYPEOFSERVICEDESC[] = 
{
	{ SERVICE_WIN32_OWN_PROCESS, _T("Run its own process") },
	{ SERVICE_WIN32_SHARE_PROCESS, _T("Share a process with other application") },
	{ SERVICE_KERNEL_DRIVER, _T("Device driver") },
	{ SERVICE_FILE_SYSTEM_DRIVER, _T("File system driver") },
	{ SERVICE_INTERACTIVE_PROCESS, _T("Service can interactive with desktop") }
};
#define nTYPEOFSERVICEDESC (sizeof(TYPEOFSERVICEDESC) / sizeof(DESCRIPTION))

// Description of Current Status
static DESCRIPTION CURRENTSTATUSDESC[] = 
{
	{ SERVICE_STOPPED, _T("Stoped") }, 
	{ SERVICE_START_PENDING, _T("Starting") }, 
	{ SERVICE_STOP_PENDING, _T("Stopping") }, 
	{ SERVICE_RUNNING, _T("Running") }, 
	{ SERVICE_CONTINUE_PENDING, _T("Continue is pending") }, 
	{ SERVICE_PAUSE_PENDING, _T("Pause is pending") }, 
	{ SERVICE_PAUSED, _T("Paused") }
};
#define nCURRENTSTATUSDESC (sizeof(CURRENTSTATUSDESC) / sizeof(DESCRIPTION))

// Description of Control Code
static DESCRIPTION CONTROLCODEDESC[] = 
{
	{ SERVICE_ACCEPT_STOP, _T("Can Stop") },
	{ SERVICE_ACCEPT_PAUSE_CONTINUE, _T("Can Pause and continue") },
	{ SERVICE_ACCEPT_SHUTDOWN, _T("Notified when shutdown") },
	// win 2000 and above
	{ SERVICE_ACCEPT_PARAMCHANGE, _T("Reread startup paramater") },
	// win 2000 and above
	{ SERVICE_ACCEPT_NETBINDCHANGE, _T("Can change network binding") }
};
#define nCONTROLCODEDESC (sizeof(CONTROLCODEDESC) / sizeof(DESCRIPTION))


class ServicesHelper
{
public:
	ServicesHelper(void) {};
	virtual ~ServicesHelper(void) {};

	static void ErrorDescription(DWORD iReturnCode, TCHAR* description);

	static int GetAvailable_Services(const TCHAR* hostName, ENUM_SERVICE_STATUS* pServices, int servicesCount);
	static int GetService_DisplayName(const TCHAR* hostName, const TCHAR* serviceName, TCHAR* name, int buffSize);

	static int QueryService_Status(const TCHAR* hostName, const TCHAR* serviceName, SERVICE_STATUS_PROCESS &ssStatus);
	static int QueryService_Config(const TCHAR* hostName, const TCHAR* serviceName, QUERY_SERVICE_CONFIG &serviceConfig);

	static int Control_Service(const TCHAR* hostName, const TCHAR* serviceName, DWORD state, BOOL stopDependencies = TRUE);
	static DWORD GetService_CurrentState(const TCHAR* hostName, const TCHAR* serviceName);

	static DWORD GetService_Type(const TCHAR* hostName, const TCHAR* serviceName);

	static int SetService_StartType(const TCHAR* hostName, const TCHAR* serviceName, DWORD type);
	static DWORD GetService_StartType(const TCHAR* hostName, const TCHAR* serviceName);

	static int Enum_DependentServices(const TCHAR* hostName, const TCHAR* serviceName, ENUM_SERVICE_STATUS **ppEnumServiceStatus, DWORD *pdwServicesReturned);

private:
	static BOOL ServiceStop(const TCHAR* hostName, const TCHAR* serviceName, BOOL stopDependencies = TRUE);
};