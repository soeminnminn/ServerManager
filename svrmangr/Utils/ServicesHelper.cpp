// ServicesHelper.cpp

#include "stdafx.h"
#include "ServicesHelper.h"

BOOL ServicesHelper::ServiceStop(const TCHAR* hostName, const TCHAR* serviceName, BOOL stopDependencies)
{
	return ServicesHelper::Control_Service(hostName, serviceName, SERVICE_CONTROL_STOP, stopDependencies);
};

void ServicesHelper::ErrorDescription(DWORD iReturnCode, TCHAR* description) 
{
	HLOCAL hLocal = NULL;

	::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL, iReturnCode, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),(LPTSTR)&hLocal, 0, NULL);

	_tcscpy(description, (LPCTSTR)LocalLock(hLocal));
	::LocalFree(hLocal);
};

int ServicesHelper::GetAvailable_Services(const TCHAR* hostName, ENUM_SERVICE_STATUS* pServices, int servicesCount)
{
	int iRC = 0;
	DWORD dwBytesNeeded = 0;
	DWORD dwServicesReturned = 0;
	DWORD dwResumedHandle = 0;

	SC_HANDLE hSCManager = ::OpenSCManager(hostName, NULL, SC_MANAGER_ENUMERATE_SERVICE);

	if (hSCManager)
	{
		ENUM_SERVICE_STATUS service = {NULL};
		// Query services
		iRC = ::EnumServicesStatus(hSCManager, SERVICE_WIN32 | SERVICE_DRIVER, SERVICE_STATE_ALL, 
					&service, sizeof(ENUM_SERVICE_STATUS), &dwBytesNeeded, &dwServicesReturned,
					&dwResumedHandle);
		
		if(!iRC)
		{
			// Need big buffer
			if (ERROR_MORE_DATA == ::GetLastError()) 
			{
				// Set the buffer
				DWORD dwBytes = sizeof(ENUM_SERVICE_STATUS) + dwBytesNeeded;
				pServices = new ENUM_SERVICE_STATUS [dwBytes];

				// Now query again for services
				::EnumServicesStatus(hSCManager, SERVICE_WIN32 | SERVICE_DRIVER, SERVICE_STATE_ALL, 
								pServices, dwBytes, &dwBytesNeeded, &dwServicesReturned, &dwResumedHandle);
			}
			else
				iRC = ::GetLastError();
		}
		else
			iRC = ::GetLastError();

		// Close Service control Manager
		::CloseServiceHandle(hSCManager);
	}
	else
		iRC = ::GetLastError();

	servicesCount = (int)dwServicesReturned;
	return iRC;
};

int ServicesHelper::GetService_DisplayName(const TCHAR* hostName, const TCHAR* serviceName, TCHAR* name, int buffSize)
{
	int iRC = 0;
	DWORD dwSize = (DWORD)buffSize;

	SC_HANDLE hSCManager = ::OpenSCManager(hostName, NULL, GENERIC_EXECUTE);

	if (hSCManager)
	{
		if(::GetServiceDisplayName(hSCManager, serviceName, name, &dwSize))
			iRC = 0;
		else
			iRC = ::GetLastError();

		// Close Service control Manager
		::CloseServiceHandle(hSCManager);
	}
	else
		iRC = ::GetLastError();

	return iRC;
};

int ServicesHelper::QueryService_Status(const TCHAR* hostName, const TCHAR* serviceName, SERVICE_STATUS_PROCESS &ssStatus)
{
	DWORD dwBytesNeeded = 0;
	DWORD dwBufSize = 0;
	DWORD dwStatus = 0;
	int iRC = 0;
	SERVICE_STATUS_PROCESS *pssStatus = NULL;

	SC_HANDLE hSCManager = ::OpenSCManager(hostName, NULL, GENERIC_EXECUTE);

	if (hSCManager)
	{
		// Open the Service
		SC_HANDLE hSCService = ::OpenService(hSCManager, serviceName, SERVICE_ALL_ACCESS);

		if(hSCService)
		{
			dwBytesNeeded = sizeof(SERVICE_STATUS_PROCESS);

			for (int iNb = 0; iNb < 2; iNb++)
			{
				dwBufSize = dwBytesNeeded;
				pssStatus = (SERVICE_STATUS_PROCESS*)malloc(dwBufSize);
				
				// Request the Status of the Service
				iRC = ::QueryServiceStatusEx( hSCService, SC_STATUS_PROCESS_INFO, (LPBYTE)pssStatus, dwBufSize, &dwBytesNeeded);
				
				// If the Request is successful
				if (iRC)
				{
					ssStatus.dwCheckPoint = pssStatus->dwCheckPoint;
					ssStatus.dwControlsAccepted = pssStatus->dwControlsAccepted;
					ssStatus.dwCurrentState = pssStatus->dwCurrentState;
					ssStatus.dwProcessId = pssStatus->dwProcessId;
					ssStatus.dwServiceFlags = pssStatus->dwServiceFlags;
					ssStatus.dwServiceSpecificExitCode = pssStatus->dwServiceSpecificExitCode;
					ssStatus.dwServiceType = pssStatus->dwServiceType;
					ssStatus.dwWaitHint = pssStatus->dwWaitHint;
					ssStatus.dwWin32ExitCode = pssStatus->dwWin32ExitCode;

					iRC = 0;
					break;
				}
				// If the size of buffer is NOT sufficient
				else if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
				{
					// Continue the loop...
					free(pssStatus);
					pssStatus = 0;
				}
				else
				{
					// Error: Exit loop in error
					break;
				}
			}

			// Close Service
			::CloseServiceHandle(hSCService);
		}
		else
			iRC = ::GetLastError();

		// Close Service control Manager
		::CloseServiceHandle(hSCManager);
	}
	else
		iRC = ::GetLastError();

	return iRC;
};

int ServicesHelper::QueryService_Config(const TCHAR* hostName, const TCHAR* serviceName, QUERY_SERVICE_CONFIG &serviceConfig)
{
	DWORD dwBytesNeeded = 0;
	DWORD dwBufSize = 0;
	DWORD dwStartType = 0;
	int iRC = 0;
	QUERY_SERVICE_CONFIG *pServiceConfig = NULL;

	SC_HANDLE hSCManager = ::OpenSCManager(hostName, NULL, GENERIC_EXECUTE);

	if (hSCManager)
	{
		// Open the Service
		SC_HANDLE hSCService = ::OpenService(hSCManager, serviceName, SERVICE_ALL_ACCESS);

		if(hSCService)
		{
			dwBytesNeeded = sizeof (QUERY_SERVICE_CONFIG);
			for (int iNb = 0; iNb < 2; iNb++)
			{
				dwBufSize = dwBytesNeeded;
				pServiceConfig = (QUERY_SERVICE_CONFIG *)malloc(dwBufSize);
				
				// Request the Config of the Service
				iRC = ::QueryServiceConfig(hSCService, pServiceConfig, dwBufSize, &dwBytesNeeded);
				
				// If the Request is successful
				if (iRC)
				{
					serviceConfig.dwErrorControl = pServiceConfig->dwErrorControl;
					serviceConfig.dwServiceType = pServiceConfig->dwServiceType;
					serviceConfig.dwStartType = pServiceConfig->dwStartType;
					serviceConfig.dwTagId = pServiceConfig->dwTagId;
					serviceConfig.lpBinaryPathName = pServiceConfig->lpBinaryPathName;
					serviceConfig.lpDependencies = pServiceConfig->lpDependencies;
					serviceConfig.lpDisplayName = pServiceConfig->lpDisplayName;
					serviceConfig.lpLoadOrderGroup = pServiceConfig->lpLoadOrderGroup;
					serviceConfig.lpServiceStartName = pServiceConfig->lpServiceStartName;

					iRC = 0;
					break;
				}
				// If the size of buffer is NOT sufficient
				else if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
				{
					// Continue the loop...
					free(pServiceConfig);
					pServiceConfig = 0;
				}
				else
				{
					// Error: Exit loop in error
					break;
				}
			}
			// Close Service
			::CloseServiceHandle(hSCService);
		}
		else
			iRC = ::GetLastError();

		// Close Service control Manager
		::CloseServiceHandle(hSCManager);
	}
	else
		iRC = ::GetLastError();

	return iRC;
};

int ServicesHelper::Control_Service(const TCHAR* hostName, const TCHAR* serviceName, DWORD state, BOOL stopDependencies)
{
	SERVICE_STATUS ssServiceStatus = {NULL};
	int iRC = 0;

	SC_HANDLE hSCManager = ::OpenSCManager(hostName, NULL, GENERIC_EXECUTE);

	if (hSCManager)
	{
		// Open the Service
		SC_HANDLE hSCService = ::OpenService(hSCManager, serviceName, SERVICE_ALL_ACCESS);

		if(hSCService)
		{
			if(state == SERVICE_CONTROL_START)
			{
				// Start SQL Service
				if (::StartService(hSCService, 0, NULL)) 
					iRC = 0;
				else
					iRC = ::GetLastError();
			}
			else
			{
				if (state == SERVICE_CONTROL_STOP && stopDependencies)
                {
					DWORD dwBytes = 0;
                    DWORD dwCount = 0;

					iRC = ::EnumDependentServices(hSCService, SERVICE_ACTIVE, NULL, 0, &dwBytes, &dwCount);
                    
                    if (!iRC)
						iRC = ::GetLastError();

                    if (dwBytes > 0)
                    {
                        LPENUM_SERVICE_STATUS lpEnum;
                        DWORD i;

						lpEnum = (LPENUM_SERVICE_STATUS)::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytes);

                        if (lpEnum)
                        {
                            if (::EnumDependentServices(hSCService, SERVICE_ACTIVE, lpEnum, dwBytes, &dwBytes, &dwCount))
                            {
                                for (i = 0; i < dwCount; i++) 
                                    ServiceStop(hostName, (*(lpEnum + i)).lpServiceName, stopDependencies);
                            }

                            ::HeapFree(::GetProcessHeap(), 0, lpEnum);
                        }
                    }
				}

				if (::ControlService(hSCService, state, &ssServiceStatus)) 
					iRC = 0;
				else
					iRC = ::GetLastError();
			}

			// Close Service
			::CloseServiceHandle(hSCService);
		}
		else
			iRC = ::GetLastError();

		// Close Service control Manager
		::CloseServiceHandle(hSCManager);
	}
	else
		iRC = ::GetLastError();

	return iRC;
};

DWORD ServicesHelper::GetService_CurrentState(const TCHAR* hostName, const TCHAR* serviceName)
{
	SERVICE_STATUS_PROCESS pssStatus = {NULL};
	DWORD dwStatus = 0;

	if (!QueryService_Status(hostName, serviceName, pssStatus)) 
		dwStatus = pssStatus.dwCurrentState;
	
	return dwStatus;
};

DWORD ServicesHelper::GetService_Type(const TCHAR* hostName, const TCHAR* serviceName)
{
	QUERY_SERVICE_CONFIG pServiceConfig = {NULL};
	DWORD dwServiceType = 0;

	if (!QueryService_Config(hostName, serviceName, pServiceConfig))
		dwServiceType = pServiceConfig.dwServiceType;

	return dwServiceType;
};

int ServicesHelper::SetService_StartType(const TCHAR* hostName, const TCHAR* serviceName, DWORD type)
{
	QUERY_SERVICE_CONFIG pServiceConfig = {NULL};
	
	int iRC = QueryService_Config(hostName, serviceName, pServiceConfig);
	DWORD dwErrorControl = 0;
	DWORD dwTagId = 0;

	if(pServiceConfig.dwStartType == type)
		return (int)SERVICE_NO_CHANGE;

	//return (int)SERVICE_NO_CHANGE;

	if (!iRC)
	{
		SC_HANDLE hSCManager = ::OpenSCManager(hostName, NULL, GENERIC_EXECUTE);

		if (hSCManager)
		{
			// Open the Service
			SC_HANDLE hSCService = ::OpenService(hSCManager, serviceName, SERVICE_ALL_ACCESS);

			if(hSCService)
			{
				if(::ChangeServiceConfig(hSCService, pServiceConfig.dwServiceType, type,
						dwErrorControl, pServiceConfig.lpBinaryPathName, pServiceConfig.lpLoadOrderGroup, 
						&dwTagId, pServiceConfig.lpDependencies, pServiceConfig.lpServiceStartName, 
						_T(""), pServiceConfig.lpDisplayName))
				{
					iRC = 0;
				}
				else
				{
					if(dwErrorControl == SERVICE_NO_CHANGE)
						iRC = (int)SERVICE_NO_CHANGE; // The existing StartType value is not to be changed.
					else
						iRC = ::GetLastError();
				}
				//ERROR_INVALID_PARAMETER

				// Close Service
				::CloseServiceHandle(hSCService);
			}
			else
				iRC = ::GetLastError();

			// Close Service control Manager
			::CloseServiceHandle(hSCManager);
		}
		else
			iRC = ::GetLastError();
	}
	else
		iRC = ::GetLastError();

	return iRC;
};

DWORD ServicesHelper::GetService_StartType(const TCHAR* hostName, const TCHAR* serviceName)
{
	QUERY_SERVICE_CONFIG pServiceConfig = {NULL};
	DWORD dwStartType = 0;
	
	if (!QueryService_Config(hostName, serviceName, pServiceConfig))
		dwStartType = pServiceConfig.dwStartType;

	return dwStartType;
};

int Enum_DependentServices(const TCHAR* hostName, const TCHAR* serviceName, ENUM_SERVICE_STATUS **ppEnumServiceStatus, DWORD *pdwServicesReturned)
{
	DWORD dwBufSize = 0;	// Buffer Size
	DWORD dwBytesNeeded = 0;	// Buffer Size needed (in case of dwBufSize is insufficient)
	int iRC = -1;	// Return Code (0 whether Ok)
	
	// Initialize the number of Services returned
	*pdwServicesReturned = 0;
	*ppEnumServiceStatus = NULL;
	
	// Establish a connection to the service control manager on the specified computer and 
	// open the specified service control manager database
	SC_HANDLE hSCManager = ::OpenSCManager(hostName, NULL, GENERIC_EXECUTE);
	
	// If the Service control manager is correctly opened
	if (hSCManager)
	{
		// Open the SQL Service
		SC_HANDLE hSCService = ::OpenService(hSCManager, serviceName, SERVICE_ALL_ACCESS);
		
		// If the SQL Service is correctly opened
		if (hSCService)
		{
			dwBytesNeeded = sizeof(ENUM_SERVICE_STATUS);
			for (int iNb = 0; iNb < 2; iNb++)
			{
				dwBufSize = dwBytesNeeded;
				*ppEnumServiceStatus = (ENUM_SERVICE_STATUS*)malloc(dwBufSize);
				
				// Enum the Dependent and Active Services
				iRC = ::EnumDependentServices(hSCService, SERVICE_ACTIVE, *ppEnumServiceStatus,
											 dwBufSize, &dwBytesNeeded, pdwServicesReturned);

				// If the Request is successful
				if (iRC)
				{
					// Reset the return code because this function return 0 when there is no error
					iRC = 0;
					break;
				}
				// If the size of buffer is NOT sufficient
				else if (::GetLastError() == ERROR_MORE_DATA)
				{
					// Continue the loop...
					free(*ppEnumServiceStatus);
					*ppEnumServiceStatus = NULL;
				}
				else
				{
					// Error: Exit loop in error
					break;
				}
			}
			// Close Service
			::CloseServiceHandle(hSCService);
		}
		else
			iRC = ::GetLastError();

		// Close Service control Manager
		::CloseServiceHandle(hSCManager);
	}
	else
		iRC = ::GetLastError();

	return iRC;
};


