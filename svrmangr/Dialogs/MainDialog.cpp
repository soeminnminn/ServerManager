

#include "stdafx.h"
#include "MainDialog.h"
#include "resource.h"
#include "RegistryHelper.h"
#include <shellapi.h>
#include <initguid.h>

LRESULT CALLBACK DialogProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) 
{
	if(Message == WM_INITDIALOG)
	{
		::SetWindowLong(hWnd, GWL_USERDATA, (long)lParam);
		((MainDialog *)(lParam))->OnMessage(hWnd, Message, wParam, lParam);
		return TRUE;
	}
	else
	{
#pragma warning( disable: 4312 )
		MainDialog* pDlg = reinterpret_cast<MainDialog *>(::GetWindowLong(hWnd, GWL_USERDATA));
#pragma warning( default: 4312 )
		if (!pDlg) return FALSE;
		return pDlg->OnMessage(hWnd, Message, wParam, lParam);
	}
};

void MainDialog::GetWndClassEx(WNDCLASSEX &wc)
{
	memset(&wc, 0, sizeof(wc));

	wc.cbSize        = sizeof(WNDCLASSEX);

	// Get the class for the system dialog class
	::GetClassInfoEx(NULL, _T("#32770"), &wc);
    wc.style &= ~CS_GLOBALCLASS;

	wc.lpfnWndProc   = DialogProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = DLGWINDOWEXTRA;
	wc.hInstance     = NULL;
	wc.hIcon         = NULL;
	wc.hCursor       = ::LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = ::CreateSolidBrush(::GetSysColor(COLOR_BTNFACE));
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = NULL;
	wc.hIconSm       = NULL;
};

bool MainDialog::RegisterClass(LPCTSTR lpszClass, HINSTANCE hInst)
{
	WNDCLASSEX wc = {NULL};

	if ( ! ::GetClassInfoEx(hInst, lpszClass, &wc) )
	{
		GetWndClassEx(wc);

		wc.hInstance     = hInst;
		wc.lpszClassName = lpszClass;

		if ( ! ::RegisterClassEx(&wc) )
			return false;
	}

	return true;
};

/////////////////////////////////////////////////////////////////////////////////////////
/// Do Create and Show Dialog
int MainDialog::Create(HINSTANCE hInst, HWND hParent)
{
	m_hInst = hInst;
	m_hParent = hParent;

	MSG msg;
	HACCEL hAccelTable;

	::CoInitialize(NULL);
	::LoadString(m_hInst, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	::LoadString(m_hInst, IDC_APP_NAME, szWindowClass, MAX_LOADSTRING);

	if(szWindowClass)
	{
		if(!RegisterClass(szWindowClass, m_hInst))
		{
			MessageBox (NULL, _T("Error in RegisterClassEx"), _T("error"), MB_ICONERROR);
			return NULL;
		}
	}
	
	//return (UINT)::DialogBoxParam(m_hInst, MAKEINTRESOURCE(IDD_DIALOGMAIN), m_hParent, (DLGPROC)dlgProc, (LPARAM)this);

	hAccelTable = ::LoadAccelerators(m_hInst, MAKEINTRESOURCE(IDC_ACCELERATORS));

	// create the dialog    
    m_hWnd = ::CreateDialogParam(m_hInst, MAKEINTRESOURCE(IDD_MAIN), m_hParent, (DLGPROC)DialogProc, (LPARAM)this);

    if (m_hWnd == NULL)
    {
        ::MessageBox (NULL, _T("Can't create Main window"), _T("error"), MB_ICONERROR);
        return 0;
    }

	//::ShowWindow(m_hWnd, SW_NORMAL);

    // Message Loop
    while (::GetMessage(&msg, NULL, 0, 0) > 0)
    {
        if (!::TranslateAccelerator(m_hWnd, hAccelTable, &msg))
        {
            if (!::IsDialogMessage(m_hWnd, &msg))
            {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
            }
        }
    }

	::CoUninitialize();

	return (int)msg.wParam;
};

/////////////////////////////////////////////////////////////////////////////////////////
/// Set Dialog Position
BOOL MainDialog::SetCenter(void) const 
{ 
	int workAreaWidth = 0;
	int workAreaHeight = 0;

	RECT rcWindow = {NULL};
	::GetWindowRect(m_hWnd, &rcWindow);

	int wndWidth = rcWindow.right - rcWindow.left;
	int wndHeight = rcWindow.bottom - rcWindow.top;

	if(::IsWindow(m_hParent))
	{
		RECT rcParent = {NULL};
		::GetClientRect(m_hParent, &rcParent);
		
		workAreaWidth = rcParent.right - rcParent.left;
		workAreaHeight = rcParent.bottom - rcParent.top;
	}
	else
	{
		RECT workAreaRect = {NULL};
		::SystemParametersInfo(SPI_GETWORKAREA, 0, &workAreaRect, 0);

		workAreaWidth = workAreaRect.right - workAreaRect.left;
		workAreaHeight = workAreaRect.bottom - workAreaRect.top;
	}
	
	rcWindow.left = (workAreaWidth - wndWidth) / 2;
	rcWindow.top = (workAreaHeight - wndHeight) / 2;

	return ::MoveWindow(m_hWnd, rcWindow.left, rcWindow.top, wndWidth, wndHeight, TRUE);
};

/////////////////////////////////////////////////////////////////////////////////////////
/// Initialize Methods
void MainDialog::InitializeControls(HWND hWnd)
{
	LoadSetting();
	InitializeSysmenu(m_hWnd);
	::SendDlgItemMessage(m_hWnd, IDC_CBO_SERVICES, (UINT)CB_SETDROPPEDWIDTH, (WPARAM)240, (LPARAM)0L);

	RefreshControls(hWnd);
	::SetFocus(::GetDlgItem(hWnd, IDM_START));
	
	if(verifyAction)
		::ShowWindow(hWnd, SW_HIDE);
	else
		::ShowWindow(m_hWnd, SW_NORMAL);
};

void MainDialog::InitializeSysmenu(HWND hWnd)
{
	HMENU hSysMenu = ::GetSystemMenu(hWnd, 0L);

	if(!hSysMenu) return;

	::InsertMenu(hSysMenu, 0, MF_BYPOSITION, MF_SEPARATOR, NULL);
	::InsertMenu(hSysMenu, 0, MF_BYPOSITION | MF_STRING, IDM_ABOUT, _T("&About..."));
	::InsertMenu(hSysMenu, 0, MF_BYPOSITION | MF_STRING, IDM_OPTION, _T("&Option..."));
};

void MainDialog::RefreshControls(HWND hWnd)
{
	TCHAR szTemp[MAX_PATH + 1] = {NULL};
	TCHAR szComputerName[MAX_PATH + 1] = {NULL};
	TCHAR szLanGroupName[MAX_PATH + 1] = {NULL};
	DWORD dwSize = (DWORD)MAX_PATH;
	
	// Add Items to Server Combobox
	::GetComputerName(szComputerName, &dwSize);

	stdvstring serversList;
	serversList.clear();
	::SendDlgItemMessage(hWnd, IDC_CBO_SERVER, (UINT)CB_RESETCONTENT, (WPARAM)0L, (LPARAM)0L);

	if(!SQLServerUtils::GetSQLServerList(serversList))
	{
		for(int i = 0; i < (int)serversList.size(); i++)
		{
			_tcscpy(szTemp, serversList[i].c_str());
			::SendDlgItemMessage(hWnd, IDC_CBO_SERVER, (UINT)CB_ADDSTRING, (WPARAM)0L, (LPARAM)szTemp);
		}
	}
	::SendDlgItemMessage(hWnd, IDC_CBO_SERVER, (UINT)WM_SETTEXT, (WPARAM)0L, (LPARAM)szComputerName);

	// Add Items to Services Combobox
	int defaultSelIndex = 0;
	listServices.clear();
	listDisplay.clear();
	::SendDlgItemMessage(hWnd, IDC_CBO_SERVICES, (UINT)CB_RESETCONTENT, (WPARAM)0L, (LPARAM)0L);

	if(SQLServerUtils::GetSQLServicesList(szComputerName, listServices, listDisplay))
	{
		for(int i = 0; i < (int)listDisplay.size(); i++)
		{
			_tcscpy(szTemp, listDisplay[i].c_str());
			::SendDlgItemMessage(hWnd, IDC_CBO_SERVICES, (UINT)CB_ADDSTRING, (WPARAM)0L, (LPARAM)szTemp);

			if(_tcscmp(listServices[i].c_str(), defaultSelection) == 0)
				defaultSelIndex = i;
		}
	}
	::SendDlgItemMessage(hWnd, IDC_CBO_SERVICES, (UINT)CB_SETCURSEL, (WPARAM)defaultSelIndex, (LPARAM)0L);

	SetCurrentState(hWnd, STATE_PENDING, TRUE);
	ReduceMemory();
};

/////////////////////////////////////////////////////////////////////////////////////////
/// Set UI State for Services
void MainDialog::SetCurrentState(HWND hWnd, UINT state, UINT type, BOOL cantTypeChange)
{
	if((state < STATE_NONE) || (state > STATE_UNKNOWN)) return;
	int iIndex = GetServiceIDFromDefault();

	SetStartType(hWnd, type, cantTypeChange);
	SetStateUI(hWnd, iIndex, state);

	if (currServiceState == state) return;
	currServiceState = state;

	disableService = (type == STARTTYPE_DISABLED);
	EnableControls(hWnd, (disableService ? FALSE : TRUE));

	if(!disableService) SetButtonsState(hWnd, state);
	
	SetStatePicture(hWnd, state);
};

void MainDialog::SetStartType(HWND hWnd, UINT type, BOOL cantChange)
{
	::SendDlgItemMessage(hWnd, IDC_CHK_AUTOSTART, (UINT)BM_SETCHECK, (WPARAM)((type == STARTTYPE_AUTO) ? 1L : 0L), (LPARAM)0L);
	::EnableWindow(::GetDlgItem(hWnd, IDC_CHK_AUTOSTART), (((type == STARTTYPE_DISABLED) || (cantChange)) ? 0L : 1L));
};

void MainDialog::SetButtonsState(HWND hWnd, UINT state)
{
	if((state < STATE_NONE) || (state > STATE_UNKNOWN)) return;

	HICON hIconStart = NULL;
	HICON hIconPause = NULL;
	HICON hIconStop = NULL;

	// Set Buttons to Enable / Disiable Icons
	switch (state) 
	{
		case STATE_STOPPED:
			hIconStart = ::LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_START));
			hIconPause = ::LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_PAUSE_D));
			hIconStop = ::LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_STOP_D));
			break;

		case STATE_RUNNING:
			hIconStart = ::LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_START_D));
			hIconPause = ::LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_PAUSE));
			hIconStop = ::LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_STOP));
			break;

		case STATE_PAUSED:
			hIconStart = ::LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_START));
			hIconPause = ::LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_PAUSE_D));
			hIconStop = ::LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_STOP));
			break;

		case STATE_NONE:
		case STATE_PENDING:
		case STATE_UNKNOWN:
			hIconStart = ::LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_START_D));
			hIconPause = ::LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_PAUSE_D));
			hIconStop = ::LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_STOP_D));
			break;
	}

	::SendDlgItemMessage(hWnd, IDM_START, (UINT)BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)(HANDLE)hIconStart);
	::SendDlgItemMessage(hWnd, IDM_PAUSE, (UINT)BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)(HANDLE)hIconPause);
	::SendDlgItemMessage(hWnd, IDM_STOP, (UINT)BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)(HANDLE)hIconStop);

	// Enable / Disiable
	switch (state) 
	{
		case STATE_STOPPED:
			::EnableWindow(::GetDlgItem(hWnd, IDM_START), TRUE);
			::EnableWindow(::GetDlgItem(hWnd, IDM_PAUSE), FALSE);
			::EnableWindow(::GetDlgItem(hWnd, IDM_STOP), FALSE);
			break;

		case STATE_RUNNING:
			::EnableWindow(::GetDlgItem(hWnd, IDM_START), FALSE);
			::EnableWindow(::GetDlgItem(hWnd, IDM_PAUSE), TRUE);
			::EnableWindow(::GetDlgItem(hWnd, IDM_STOP), TRUE);
			break;

		case STATE_PAUSED:
			::EnableWindow(::GetDlgItem(hWnd, IDM_START), TRUE);
			::EnableWindow(::GetDlgItem(hWnd, IDM_PAUSE), FALSE);
			::EnableWindow(::GetDlgItem(hWnd, IDM_STOP), TRUE);
			break;

		case STATE_NONE:
		case STATE_PENDING:
		case STATE_UNKNOWN:
			::EnableWindow(::GetDlgItem(hWnd, IDM_START), FALSE);
			::EnableWindow(::GetDlgItem(hWnd, IDM_PAUSE), FALSE);
			::EnableWindow(::GetDlgItem(hWnd, IDM_STOP), FALSE);
			break;
	}
};

void MainDialog::SetStateUI(HWND hWnd, int svrIndex, UINT state)
{
	if((state < STATE_NONE) || (state > STATE_UNKNOWN)) return;

	HICON hTrayIcon = NULL;
	UINT iconId = 0;
	TCHAR szTip[MAX_PATH + 1] = {NULL};
	TCHAR szTemp[MAX_PATH + 1] = {NULL};

	switch (svrIndex)
	{
		case 0:
		case 1:
			iconId = IDI_SQLSERVERAGENT;
			break;

		case 2:
			iconId = IDI_ANALYSISSERVER;
			break;

		case 3:
			iconId = IDI_MSDTC;
			break;

		case 4:
			iconId = IDI_FULLTEXTSEARCH;
			break;

		case 5:
			iconId = IDI_INTEGRATIONSERVICES;
			break;

		case 6:
			iconId = IDI_NOTIFICATIONSERVICES;
			break;

		case 7:
			iconId = IDI_REPORTSERVER;
			break;

		case 8:
		case 9:
			iconId = IDI_SQLSERVER;
			break;

		case 10:
			iconId = IDI_SQLSERVERADHELPER;
			break;

		case 11:
			iconId = IDI_SQLSERVERBROWSER;
			break;

		case 12:
			iconId = IDI_SQLWRITER;
			break;
	}

	iconId += (state - STATE_NONE);
	hTrayIcon = ::LoadIcon(m_hInst, MAKEINTRESOURCE(iconId));
	if(!hTrayIcon) return;
	// Modify Dialog Icon
	::SendMessage(hWnd, WM_SETICON, (WPARAM)1, (LPARAM)hTrayIcon);

	// Get Tooltip
	switch (state) 
	{
		case STATE_STOPPED:
			_tcscpy(szTip, _T("Stoped"));
			break;

		case STATE_RUNNING:
			_tcscpy(szTip, _T("Running"));
			break;

		case STATE_PAUSED:
			_tcscpy(szTip, _T("Paused"));
			break;

		case STATE_PENDING:
			_tcscpy(szTip, _T("Pending"));
			break;

		default:
			_tcscpy(szTip, _T("Unknown"));
			break;
	}

	if(disableService) _tcscpy(szTip, _T("Disiabled"));
	
	::GetDlgItemText(hWnd, IDC_CBO_SERVER, szTemp, MAX_PATH);
	if(szTemp) 
	{
		_tcscat(szTip, _T(" - \\\\"));
		_tcscat(szTip, szTemp);

		if(listServices.size() > 0)
		{
			::GetDlgItemText(hWnd, IDC_CBO_SERVICES, szTemp, MAX_PATH);
			if(szTemp) 
			{
				_tcscat(szTip, _T(" - "));
				_tcscat(szTip, szTemp);
			}
		}
		else
		{
			_tcscpy(szTip, szTitle);
		}
	}

	// Modify Status Text
	_tcscpy(szTemp, _T(" "));
	_tcscat(szTemp, szTip);
	::SetDlgItemText(hWnd, IDC_LBL_STATUS, szTemp);

	// ModifyNotifyIcon
	if(hTrayIcon) ModifyNotifyIcon(hWnd, IDC_MAINMENU, hTrayIcon, szTip);
};

void MainDialog::SetStatePicture(HWND hWnd, UINT state)
{
	if((state < STATE_NONE) || (state > STATE_UNKNOWN)) return;

	HBITMAP hBitmap = NULL;
	UINT bitmapId = 0;

	switch (state) 
	{
		case STATE_STOPPED:
			bitmapId = IDB_SVR_STOPED;
			break;

		case STATE_RUNNING:
			bitmapId = IDB_SVR_RUNNING;
			break;

		case STATE_PAUSED:
			bitmapId = IDB_SVR_PAUSED;
			break;

		case STATE_NONE:
		case STATE_PENDING:
		case STATE_UNKNOWN:
			bitmapId = IDB_SVR_UNKNOWN;
			break;
	}

	hBitmap = ::LoadBitmap(m_hInst, MAKEINTRESOURCE(bitmapId));
	if(!hBitmap) return;
	::SendDlgItemMessage(hWnd, IDC_PIC_STATUS, (UINT)STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)(HANDLE)hBitmap);
};

/////////////////////////////////////////////////////////////////////////////////////////
/// Get Methods
HMENU MainDialog::GetPopupMenu(HWND hWnd, UINT state)
{
	if((state < STATE_NONE) || (state > STATE_UNKNOWN)) return NULL;
	UINT menuFlags = MF_BYPOSITION | MF_STRING;

	// Create Popup Menu
	HMENU hPopup = ::CreatePopupMenu();
	::InsertMenu(hPopup, 0xFFFFFFFF, menuFlags | MF_DEFAULT, IDM_OPEN, _T("Open SQL Server Service &Manager"));

	if(!hPopup) return 0L;
	TCHAR szTemp[MAX_PATH + 1] = {NULL};
	TCHAR szMenuText[MAX_PATH + 1] = {NULL};

	// Add Services
	if(listDisplay.size() > 0)
	{
		::GetDlgItemText(hWnd, IDC_CBO_SERVER, szTemp, MAX_PATH);
		if(szTemp) 
		{
			// Add Serivces Items
			HMENU hServices = ::CreatePopupMenu();
			int servicesCount = (int)listDisplay.size();
			int selectedService = (int)::SendDlgItemMessage(hWnd, IDC_CBO_SERVICES, (UINT)CB_GETCURSEL, (WPARAM)0L, (LPARAM)0L);

			if((hServices) && (servicesCount))
			{
				for(int i = 0; i < servicesCount; i++)
				{
					if(i == selectedService)
						::InsertMenu(hServices, 0xFFFFFFFF, menuFlags | MF_CHECKED, IDM_SERVICE + i, listDisplay[i].c_str());
					else
						::InsertMenu(hServices, 0xFFFFFFFF, menuFlags, IDM_SERVICE + i, listDisplay[i].c_str());
				}

				_tcscpy(szMenuText, _T("Current service on \\\\"));
				_tcscat(szMenuText, szTemp);
				::InsertMenu(hPopup, 0xFFFFFFFF, menuFlags | MF_POPUP, (UINT_PTR)hServices, szMenuText);
			}

			::DestroyMenu(hServices);
		}
	}
	::InsertMenu(hPopup, 0xFFFFFFFF, MF_SEPARATOR, -1, _T("-"));	

	// Add Actions
	if(((int)(DWORD)::SendDlgItemMessage(hWnd, IDC_CBO_SERVICES, (UINT)CB_GETCURSEL, (WPARAM)0L, (LPARAM)0L)) != -1)
	{
		::GetDlgItemText(hWnd, IDC_CBO_SERVICES, szTemp, MAX_PATH);
		if(szTemp) 
		{
			_tcscpy(szMenuText, szTemp); _tcscat(szMenuText, _T(" - &Start"));
			::InsertMenu(hPopup, 0xFFFFFFFF, menuFlags, IDM_START, szMenuText);

			_tcscpy(szMenuText, szTemp); _tcscat(szMenuText, _T(" - &Pause"));
			::InsertMenu(hPopup, 0xFFFFFFFF, menuFlags, IDM_PAUSE, szMenuText);

			_tcscpy(szMenuText, szTemp); _tcscat(szMenuText, _T(" - St&op"));
			::InsertMenu(hPopup, 0xFFFFFFFF, menuFlags, IDM_STOP, szMenuText);

			::InsertMenu(hPopup, 0xFFFFFFFF, MF_SEPARATOR, -1, _T("-"));				
		}
	}

	::InsertMenu(hPopup, 0xFFFFFFFF, menuFlags, IDM_EXIT, _T("E&xit"));
	::InsertMenu(hPopup, 0xFFFFFFFF, menuFlags, IDM_OPTION, _T("&Option..."));
	::InsertMenu(hPopup, 0xFFFFFFFF, menuFlags, IDM_ABOUT, _T("&About..."));

	// Enable / Disiable
	if(disableService)
	{
		::EnableMenuItem(hPopup, IDM_START, TRUE);
		::EnableMenuItem(hPopup, IDM_PAUSE, TRUE);
		::EnableMenuItem(hPopup, IDM_STOP, TRUE);
	}
	else
	{
		switch (state) 
		{
			case STATE_STOPPED:
				::EnableMenuItem(hPopup, IDM_START, FALSE);
				::EnableMenuItem(hPopup, IDM_PAUSE, TRUE);
				::EnableMenuItem(hPopup, IDM_STOP, TRUE);
				break;

			case STATE_RUNNING:
				::EnableMenuItem(hPopup, IDM_START, TRUE);
				::EnableMenuItem(hPopup, IDM_PAUSE, FALSE);
				::EnableMenuItem(hPopup, IDM_STOP, FALSE);
				break;

			case STATE_PAUSED:
				::EnableMenuItem(hPopup, IDM_START, FALSE);
				::EnableMenuItem(hPopup, IDM_PAUSE, TRUE);
				::EnableMenuItem(hPopup, IDM_STOP, FALSE);
				break;

			case STATE_NONE:
			case STATE_PENDING:
			case STATE_UNKNOWN:
				::EnableMenuItem(hPopup, IDM_START, TRUE);
				::EnableMenuItem(hPopup, IDM_PAUSE, TRUE);
				::EnableMenuItem(hPopup, IDM_STOP, TRUE);
				break;
		}
	}

	::SetMenuDefaultItem(hPopup, IDM_OPEN, 0L);

	return hPopup;
};

int MainDialog::GetCurrentService(TCHAR* hostName, TCHAR* serviceName, TCHAR* displayName)
{
	int iIndex = -1;
	if(listServices.size() < 1) return iIndex;
	if(listDisplay.size() < 1) return iIndex;

	TCHAR szHostName[MAX_PATH + 1] = {NULL};
	::GetDlgItemText(m_hWnd, IDC_CBO_SERVER, szHostName, MAX_PATH);
	_tcscpy(hostName, szHostName);

	iIndex = (int)::SendDlgItemMessage(m_hWnd, IDC_CBO_SERVICES, (UINT)CB_GETCURSEL, (WPARAM)0L, (LPARAM)0L);
	_tcscpy(serviceName, listServices[iIndex].c_str());
	_tcscpy(displayName, listDisplay[iIndex].c_str());

	return iIndex;
};

int MainDialog::GetServiceIDFromDefault()
{
	TCHAR szTemp[MAX_PATH] = {NULL};
	TCHAR szService[MAX_PATH] = {NULL};
	TCHAR hostName[MAX_PATH + 1] = {NULL};
	TCHAR serviceName[MAX_PATH] = {NULL};
	TCHAR displayName[MAX_PATH] = {NULL};

	int iService = GetCurrentService(hostName, serviceName, displayName);
	SQLServerUtils::FindServiceName(serviceName, szService);

	for(int i=0; i<nDEFSERVICES; i++)
	{
		_tcscpy(szTemp, _DEFAULTSERVICESNAME[i].serviceName);
		if(_tcscmp(szTemp, szService) == 0) return i;
	}

	return -1;
};

/////////////////////////////////////////////////////////////////////////////////////////
/// Private Methods
void MainDialog::EnableControls(HWND hWnd, BOOL enable)
{
	::EnableWindow(::GetDlgItem(hWnd, IDC_CHK_AUTOSTART), enable);

	::EnableWindow(::GetDlgItem(hWnd, IDM_START), enable);
	::EnableWindow(::GetDlgItem(hWnd, IDM_PAUSE), enable);
	::EnableWindow(::GetDlgItem(hWnd, IDM_STOP), enable);
};

UINT MainDialog::CurrentStateToState(DWORD dwCurrentState)
{
	switch(dwCurrentState)
	{
		case SERVICE_STOPPED:
			return STATE_STOPPED;

		case SERVICE_PAUSED:
			return STATE_PAUSED;

		case SERVICE_RUNNING:
			return STATE_RUNNING;

		case SERVICE_START_PENDING:
		case SERVICE_STOP_PENDING:
		case SERVICE_CONTINUE_PENDING:
		case SERVICE_PAUSE_PENDING:
			return STATE_PENDING;

		default:
			return STATE_NONE;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
/// Do Actions Methods
void MainDialog::DoOption(HWND hWnd)
{
	TCHAR hostName[MAX_PATH] = {NULL};
	TCHAR serviceName[MAX_PATH] = {NULL};
	TCHAR displayName[MAX_PATH] = {NULL};
	int defaultSelIndex = 0;

	::SetForegroundWindow(hWnd);

	defaultSelIndex = (int)::SendDlgItemMessage(hWnd, IDC_CBO_SERVICES, (UINT)CB_GETCURSEL, (WPARAM)0L, (LPARAM)0L);

	optionDialog.SetData(listDisplay, defaultSelIndex, pollingInterval, verifyAction);
	if(optionDialog.Create(m_hInst, hWnd) == IDOK)
	{
		// Remove Thread
		DetachProcessThread();
		
		// Get Data
		defaultSelIndex = optionDialog.GetDefaultIndex();
		::SendDlgItemMessage(hWnd, IDC_CBO_SERVICES, (UINT)CB_SETCURSEL, (WPARAM)defaultSelIndex, (LPARAM)0L);

		if(GetCurrentService(hostName, serviceName, displayName) > -1)
			_tcscpy(defaultSelection, serviceName);

		pollingInterval = optionDialog.GetPollingInterval();
		verifyAction = optionDialog.GetVerifyAction();
		
		// Save
		TCHAR appName[MAX_PATH+1] = {0};
		::GetModuleFileName(NULL, appName, MAX_PATH);
		RegistryHelper::RegSetStartup(HKEY_CURRENT_USER, appName, _T(""), (verifyAction ? FALSE : TRUE));

		SaveSetting();

		// Create Thread
		AttachProcessThread((DWORD)THREADID);
	}
};

void MainDialog::DoRefresh(HWND hWnd)
{
	RefreshControls(hWnd);
};

void MainDialog::DoStart(HWND hWnd)
{
	TCHAR hostName[MAX_PATH] = {NULL};
	TCHAR serviceName[MAX_PATH] = {NULL};
	TCHAR displayName[MAX_PATH] = {NULL};

	if(GetCurrentService(hostName, serviceName, displayName) > -1)
	{
		if(currServiceState == STATE_PAUSED)
			ServicesHelper::Control_Service(hostName, serviceName, SERVICE_CONTROL_CONTINUE);
		else
			ServicesHelper::Control_Service(hostName, serviceName, SERVICE_CONTROL_START);
	}
};

void MainDialog::DoPause(HWND hWnd)
{
	TCHAR hostName[MAX_PATH] = {NULL};
	TCHAR serviceName[MAX_PATH] = {NULL};
	TCHAR displayName[MAX_PATH] = {NULL};

	if(GetCurrentService(hostName, serviceName, displayName) > -1)
	{
		ServicesHelper::Control_Service(hostName, serviceName, SERVICE_CONTROL_PAUSE);
	}
};

void MainDialog::DoStop(HWND hWnd)
{
	TCHAR hostName[MAX_PATH] = {NULL};
	TCHAR serviceName[MAX_PATH] = {NULL};
	TCHAR displayName[MAX_PATH] = {NULL};

	if(GetCurrentService(hostName, serviceName, displayName) > -1)
	{
		TCHAR message[MAX_PATH] = {NULL};
		_stprintf(message, MSG_STOP, _T("STOP"), displayName, hostName);

		if(::MessageBox(hWnd, message, szTitle, MB_YESNO | MB_ICONQUESTION) == IDYES)
			ServicesHelper::Control_Service(hostName, serviceName, SERVICE_CONTROL_STOP);
	}
};

void MainDialog::DoAutoStartChange(HWND hWnd)
{
	BOOL autoStart = (BOOL)(::SendDlgItemMessage(hWnd, IDC_CHK_AUTOSTART, (UINT)BM_GETCHECK, (WPARAM)0L, (LPARAM)0L) == BST_CHECKED);
	//
	TCHAR hostName[MAX_PATH] = {NULL};
	TCHAR serviceName[MAX_PATH] = {NULL};
	TCHAR displayName[MAX_PATH] = {NULL};

	if(GetCurrentService(hostName, serviceName, displayName) > -1)
	{
		int iRC = ServicesHelper::SetService_StartType(hostName, serviceName, (autoStart ? SERVICE_AUTO_START : SERVICE_DEMAND_START));
	}
};

BOOL MainDialog::DoProcess()
{
	TCHAR hostName[MAX_PATH] = {NULL};
	TCHAR serviceName[MAX_PATH] = {NULL};
	TCHAR displayName[MAX_PATH] = {NULL};

	do
	{
		if(GetCurrentService(hostName, serviceName, displayName) > -1)
		{
			BOOL cantTypeChange = FALSE;
			UINT uStartType = STARTTYPE_DISABLED;
			DWORD dwStartType = ServicesHelper::GetService_StartType(hostName, serviceName);
			
			switch(dwStartType)
			{
				case SERVICE_BOOT_START:
				case SERVICE_SYSTEM_START:
					uStartType = STARTTYPE_AUTO;
					cantTypeChange = TRUE;
					break;

				case SERVICE_AUTO_START:
					uStartType = STARTTYPE_AUTO;
					break;

				case SERVICE_DEMAND_START:
					uStartType = STARTTYPE_MENUAL;
					break;

				case SERVICE_DISABLED:
					uStartType = STARTTYPE_DISABLED;
					break;
			}

			DWORD dwCurrentState = ServicesHelper::GetService_CurrentState(hostName, serviceName);
			UINT uState = CurrentStateToState(dwCurrentState);
			SetCurrentState(m_hWnd, uState, uStartType, cantTypeChange);
		}

		if (WAIT_OBJECT_0 == ::WaitForSingleObject(m_hEvent, pollingInterval)) { goto CleanExit; }

	} while(m_hEvent);

CleanExit:
	return 1L;
};

