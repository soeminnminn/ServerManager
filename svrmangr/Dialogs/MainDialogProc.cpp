
#include "stdafx.h"
#include "MainDialog.h"
#include "resource.h"
#include <shellapi.h>

/////////////////////////////////////////////////////////////////////////////////////////
/// Window Procture
LRESULT CALLBACK MainDialog::OnMessage(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message) 
	{
        case WM_INITDIALOG:
			return (LRESULT)(DWORD)(UINT)(BOOL)OnInitDialog((hWnd), (HWND)(wParam), lParam);
		
		case WM_USER_SHELLICON:
			OnUserShellIcon(hWnd, (int)(short)LOWORD(lParam));
		
		case WM_COMMAND:
			OnCommand((hWnd), (int)(LOWORD(wParam)), (HWND)(lParam), (UINT)HIWORD(wParam));
			break;

		case WM_SYSCOMMAND:
			OnSysCommand((hWnd), (UINT)(wParam), (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
			break;

		case WM_CLOSE:
			OnClose(hWnd);
			break;

		case WM_DESTROY:
			OnDestroy(hWnd);
			break;

		default:
			break;
	}
	return FALSE;
};

/////////////////////////////////////////////////////////////////////////////////////////
/// Events
BOOL MainDialog::OnInitDialog(HWND hWnd, HWND hWndFocus, LPARAM lParam)
{
	m_hWnd = hWnd;
	::InitCommonControls();
			
	HWND hBuffer = NULL;
	HICON hIcon = ::LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_SMALL));
	::SetWindowText(hWnd, szTitle);
	::SendMessage(hWnd, WM_SETICON, (WPARAM)1, (LPARAM)hIcon);
	SetCenter();
    ::UpdateWindow(hWnd); 

	AddNotifyIcon(hWnd, IDC_MAINMENU, hIcon, szTitle);
	AttachProcessThread((DWORD)THREADID);

	InitializeControls(hWnd);

	// SubClass Picture Control
	SubclassPictureBox(::GetDlgItem(hWnd, IDC_PIC_STATUS));

	return 1L;
}

void MainDialog::OnCommand(HWND hWnd, int id, HWND hWndCtl, UINT codeNotify)
{
	switch(id)
	{
		case IDC_BUT_REFRESH:
			DoRefresh(hWnd);
			break;

		case IDM_OPEN:
			::ShowWindow(hWnd, SW_SHOW);
			::SetForegroundWindow(hWnd);
			break;

		case IDM_START:
			DoStart(hWnd);
			break;

		case IDM_PAUSE:
			DoPause(hWnd);
			break;

		case IDM_STOP:
			DoStop(hWnd);
			break;

		case IDM_EXIT:
			OnDestroy(hWnd);
			break;

		case IDM_OPTION:
			DoOption(hWnd);
			break;

		case IDC_CHK_AUTOSTART:
			DoAutoStartChange(hWnd);
			break;

		case IDM_ABOUT:
			aboutDialog.Create(m_hInst, hWnd);
			::SetForegroundWindow(hWnd);
			break;
		
		default:
			{
				int servicesCount = (int)::SendDlgItemMessage(hWnd, IDC_CBO_SERVICES, (UINT)CB_GETCOUNT, (WPARAM)0L, (LPARAM)0L);
				if((servicesCount) && ((id >= IDM_SERVICE) && (id < (IDM_SERVICE + servicesCount))))
				{
					::SendDlgItemMessage(hWnd, IDC_CBO_SERVICES, (UINT)CB_SETCURSEL, (WPARAM)(id - IDM_SERVICE), (LPARAM)0L);
				}
			}
			break;
	}
};

void MainDialog::OnSysCommand(HWND hWnd, UINT cmd, int x, int y)
{
	switch(cmd)
	{
		case IDM_OPTION:
		case IDM_ABOUT:
		case IDM_EXIT:
			::PostMessage(hWnd, WM_COMMAND, (WPARAM)MAKEWPARAM(cmd, WM_SYSCOMMAND), (LPARAM)0L);
			break;

		default:
			break;
	}
};

BOOL MainDialog::OnUserShellIcon(HWND hWnd, UINT Message)
{
	POINT lpClickPoint = {NULL};

	switch(Message)
	{
		case WM_RBUTTONDOWN:
			{
				UINT uFlag = MF_BYPOSITION | MF_STRING;
				::GetCursorPos(&lpClickPoint);

				HMENU hPopMenu = GetPopupMenu(hWnd, currServiceState);
				if(!hPopMenu) return FALSE;

				::SetForegroundWindow(hWnd);
				::TrackPopupMenu(hPopMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN, lpClickPoint.x, lpClickPoint.y, 0, hWnd, NULL);
				::DestroyMenu(hPopMenu);
			}
			return TRUE;

		case WM_LBUTTONDBLCLK:
			::PostMessage(hWnd, WM_COMMAND, (WPARAM)MAKEWPARAM(IDM_OPEN, WM_USER_SHELLICON), (LPARAM)0L);
			return TRUE;
	}

	return FALSE;
};

void MainDialog::OnClose(HWND hWnd)
{
	::ShowWindow(hWnd, SW_HIDE);
};

void MainDialog::OnDestroy(HWND hWnd)
{
	SaveSetting();
	StopProcessThread();
	DetachProcessThread();

	::PostQuitMessage(0);
	::EndDialog(hWnd, 0);
};

