#pragma once

#include <windows.h>
#include <tchar.h>
#include <string.h>
#include <vector>
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <shellapi.h>

#include "svrmangr.h"
#include "ServicesHelper.h"
#include "SQLServerUtils.h"
#include "OptionDialog.h"
#include "AboutDialog.h"

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


#define MAX_LOADSTRING		100
#define	WM_USER_SHELLICON WM_USER + 1

#define THREADID 1230

// CurrentState (Depand on Resources)
#define STATE_NONE		1220
#define STATE_ERROR		(STATE_NONE + 1)
#define STATE_PAUSED		(STATE_NONE + 2)
#define STATE_RUNNING	(STATE_NONE + 3)
#define STATE_STOPPED	(STATE_NONE + 4)
#define STATE_PENDING	(STATE_NONE + 5) // SUPRESS
#define STATE_UNKNOWN (STATE_NONE + 6)

// StartType
#define STARTTYPE_AUTO			(SERVICE_AUTO_START)
#define STARTTYPE_MENUAL		(SERVICE_DEMAND_START)
#define STARTTYPE_DISABLED	(SERVICE_DISABLED)

#define IDM_SERVICE 1300

#define MSG_STOP (_T("Are you sure you wish to %s the %s service on \\\\%s?"))


class MainDialog
{
public:
	MainDialog(void) : m_hInst(NULL), m_hParent(NULL), m_hWnd(NULL) {
		m_hEvent = NULL;

		defaultSelection[0] = '\0';
		pollingInterval = 10 * MUL_POLLINGINTERVAL;
		verifyAction = TRUE;
		disableService = FALSE;
		currServiceState = STATE_NONE;

		listServices.clear();
		listDisplay.clear();
	};

	virtual ~MainDialog(void) {
		DetachProcessThread();
		::Shell_NotifyIcon(NIM_DELETE,&nidApp);

		listServices.clear();
		listDisplay.clear();
	};

	int Create(HINSTANCE hInst, HWND hParent);

	BOOL SetCenter(void) const;

	BOOL DoProcess();

	/* Subclassing Dialog */
	virtual LRESULT CALLBACK OnMessage(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);

	/* Subclassing PictureBox */
	void SubclassPictureBox(HWND hWndCtrl);
	LRESULT CALLBACK PictureBox_OnMessage(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);

protected:
	/* Handles */
	HINSTANCE		m_hInst;
	HWND			m_hParent;
	HWND			m_hWnd;
	// Threading
	HANDLE m_hEvent;

protected:
	// Events
	BOOL OnInitDialog(HWND hWnd, HWND hWndFocus, LPARAM lParam);
	void OnCommand(HWND hWnd, int id, HWND hWndCtl, UINT codeNotify);
	void OnSysCommand(HWND hWnd, UINT cmd, int x, int y);
	BOOL OnUserShellIcon(HWND hWnd, UINT message);
	void OnClose(HWND hWnd);
	void OnDestroy(HWND hWnd);

private:
	TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
	TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

	TCHAR defaultSelection[MAX_PATH];
	int pollingInterval;
	BOOL verifyAction;
	BOOL disableService;
	UINT currServiceState;

	OptionDialog optionDialog;
	AboutDialog aboutDialog;

	stdvstring listServices;
	stdvstring listDisplay;

	// Register Window Class
	bool RegisterClass(LPCTSTR lpszClass, HINSTANCE hInst);
	virtual void GetWndClassEx(WNDCLASSEX &wc);

	// Initialize
	void InitializeControls(HWND hWnd);
	void InitializeSysmenu(HWND hWnd);
	void RefreshControls(HWND hWnd);

	// Set UI State
	void SetCurrentState(HWND hWnd, UINT state, UINT type, BOOL cantTypeChange = FALSE);
	void SetStartType(HWND hWnd, UINT type, BOOL cantChange = FALSE);
	void SetButtonsState(HWND hWnd, UINT state);
	void SetStateUI(HWND hWnd, int svrIndex, UINT state);
	void SetStatePicture(HWND hWnd, UINT state);

	// Get Methods
	HMENU GetPopupMenu(HWND hWnd, UINT state);
	int GetCurrentService(TCHAR* hostName, TCHAR* serviceName, TCHAR* displayName);
	int GetServiceIDFromDefault();

	// Private Methods
	UINT CurrentStateToState(DWORD dwCurrentState);
	void EnableControls(HWND hWnd, BOOL enable = TRUE);

	// Do Action
	void DoOption(HWND hWnd);
	void DoRefresh(HWND hWnd);
	void DoStart(HWND hWnd);
	void DoPause(HWND hWnd);
	void DoStop(HWND hWnd);
	void DoAutoStartChange(HWND hWnd);

	// Setting
	void LoadSetting();
	void SaveSetting();

	// Thread Helper
	void AttachProcessThread(DWORD dwThreadId);
	void DetachProcessThread();
	void StopProcessThread();

	// NotifyIcon
	NOTIFYICONDATA nidApp;

	// NotifyIcon Helper
	void AddNotifyIcon(HWND hWnd, UINT uid, HICON hIcon, const TCHAR* tooltip);
	void ModifyNotifyIcon(HWND hWnd, UINT uid, HICON hIcon, const TCHAR* tooltip);
};
