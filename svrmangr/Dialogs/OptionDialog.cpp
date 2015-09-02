// OptionDialog.cpp

#include "stdafx.h"
#include "resource.h"
#include "OptionDialog.h"


/////////////////////////////////////////////////////////////////////////////////////////
/// Do Create and Show Dialog
UINT OptionDialog::Create(HINSTANCE hInst, HWND hParent)
{
	m_hInst = hInst;
	m_hParent = hParent;

	return (UINT)::DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_OPTIONS), hParent, (DLGPROC)dlgProc, (LPARAM)this);
}

/////////////////////////////////////////////////////////////////////////////////////////
/// Window Procture
LRESULT CALLBACK OptionDialog::runDialogProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message) 
	{
		case WM_INITDIALOG:
			return (LRESULT)(DWORD)(UINT)(BOOL)OnInitDialog((hWnd), (HWND)(wParam), lParam);

		case WM_COMMAND:
			OnCommand((hWnd), (int)(LOWORD(wParam)), (HWND)(lParam), (UINT)HIWORD(wParam));
			break;

		/*case WM_VSCROLL:
			OnVScroll((hWnd), (HWND)(lParam), (UINT)(LOWORD(wParam)),  (int)(short)HIWORD(wParam));
			break;*/

		default:
			break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
/// Events
BOOL OptionDialog::OnInitDialog(HWND hWnd, HWND hWndFocus, LPARAM lParam)
{
	m_hWnd = hWnd;
	TCHAR szTemp[MAX_PATH + 1] = {NULL};

	::SendDlgItemMessage(hWnd, IDC_CBO_SERVICES, (UINT)CB_SETDROPPEDWIDTH, (WPARAM)240, (LPARAM)0L);

	// Add Services to ComboBox
	if(listDisplay.size() > 0)
	{
		for(int i = 0; i < (int)listDisplay.size(); i++)
		{
			_tcscpy(szTemp, listDisplay[i].c_str());
			::SendDlgItemMessage(hWnd, IDC_CBO_SERVICES, (UINT)CB_ADDSTRING, (WPARAM)0L, (LPARAM)szTemp);
		}
		::SendDlgItemMessage(hWnd, IDC_CBO_SERVICES, (UINT)CB_SETCURSEL, (WPARAM)defaultIndex, (LPARAM)0L);
	}

	// Set Polling Interval
	_itot(pollingInterval, szTemp, 10);
	::SendDlgItemMessage(hWnd, IDC_TXT_INTERVAL, (UINT)WM_SETTEXT, (WPARAM)0L, (LPARAM)szTemp);
	::SendDlgItemMessage(hWnd, IDC_SPIN_INTERVAL, (UINT)UDM_SETPOS, (WPARAM)0L, (LPARAM)MAKELPARAM(pollingInterval, 0));

	// Set Verify Action State
	::SendDlgItemMessage(hWnd, IDC_CHK_VERIFY, (UINT)BM_SETCHECK , (WPARAM)(verifyAction == 0 ? 0 : 1), (LPARAM)0L);
	return 1L;
}

void OptionDialog::OnVScroll(HWND hWnd, HWND hWndCtl, UINT code, int pos)
{
	if(hWndCtl == ::GetDlgItem(hWnd, IDC_SPIN_INTERVAL))
	{
		TCHAR szTemp[MAX_PATH + 1] = {0};
		_itot(pos, szTemp, 10);
		::SetDlgItemText(hWnd, IDC_TXT_INTERVAL, szTemp);
		::SendDlgItemMessage(hWnd, IDC_TXT_INTERVAL, (UINT)EM_SETSEL, (WPARAM)(int)_tcslen(szTemp), (LPARAM)0L);
	}
};

void OptionDialog::OnCommand(HWND hWnd, int id, HWND hWndCtl, UINT codeNotify)
{
	switch(id)
	{
		case IDOK:
		case IDCANCEL:
			GetDataFromControls(hWnd);
			::EndDialog(hWnd, id);
			break;

		default:
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
/// Set/Get Data from controls
void OptionDialog::GetDataFromControls(HWND hWnd)
{
	// Get From Controls
	defaultIndex = (int)::SendDlgItemMessage(hWnd, IDC_CBO_SERVICES, (UINT)CB_GETCURSEL, (WPARAM)0L, (LPARAM)0L);
	pollingInterval = (int)::SendDlgItemMessage(hWnd, IDC_SPIN_INTERVAL, (UINT)UDM_GETPOS, (WPARAM)0L, (LPARAM)0l);
	verifyAction = (::SendDlgItemMessage(hWnd, IDC_CHK_VERIFY, (UINT)BM_GETCHECK, (WPARAM)0L, (LPARAM)0L) == BST_CHECKED);
};

void OptionDialog::SetData(stdvstring listServices, int defIndex, int pollInterval, BOOL verifyAct)
{
	defaultIndex = defIndex;
	pollingInterval = (pollInterval / MUL_POLLINGINTERVAL);
	verifyAction = verifyAct;

	listDisplay.clear();
	listDisplay = listServices;
};

