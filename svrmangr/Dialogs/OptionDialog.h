// OptionDialog.h

#pragma once

#include <windows.h>
#include <tchar.h>
#include <string.h>
#include <vector>
#include <commctrl.h>

#include "svrmangr.h"

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

class OptionDialog
{
public:
	OptionDialog() : m_hInst(NULL), m_hWnd(NULL), m_hParent(NULL) {
		listDisplay.clear();
	};

	virtual ~OptionDialog() {
		listDisplay.clear();
	};

	UINT Create(HINSTANCE hInst, HWND hParent);

	void SetData(stdvstring listServices, int defIndex, int pollInterval, BOOL verifyAct);

	int GetDefaultIndex() { return defaultIndex; };
	int GetPollingInterval() { return (pollingInterval * MUL_POLLINGINTERVAL); };
	BOOL GetVerifyAction() { return verifyAction; };

	static LRESULT CALLBACK dlgProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
		if(Message == WM_INITDIALOG)
		{
			::SetWindowLong(hWnd, GWL_USERDATA, (long)lParam);
			((OptionDialog *)(lParam))->runDialogProc(hWnd, Message, wParam, lParam);
			return TRUE;
		}
		else
		{
#pragma warning( disable: 4312 )
			OptionDialog* pDlg = reinterpret_cast<OptionDialog *>(::GetWindowLong(hWnd, GWL_USERDATA));
#pragma warning( default: 4312 )
			if (!pDlg) return FALSE;
			return pDlg->runDialogProc(hWnd, Message, wParam, lParam);
		}
	};

	virtual LRESULT CALLBACK runDialogProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);

protected:
	BOOL OnInitDialog(HWND hWnd, HWND hWndFocus, LPARAM lParam);
	void OnCommand(HWND hWnd, int id, HWND hWndCtl, UINT codeNotify);
	void OnVScroll(HWND hWnd, HWND hWndCtl, UINT code, int pos);

protected:
	HWND m_hWnd;
	HWND m_hParent;
	HINSTANCE m_hInst;

	stdvstring listDisplay;
	int defaultIndex;
	int pollingInterval;
	BOOL verifyAction;

	void GetDataFromControls(HWND hWnd);
};