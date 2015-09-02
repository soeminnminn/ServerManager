// AboutDialog.h

#pragma once

#include <windows.h>
#include <tchar.h>


class AboutDialog
{
public:
	AboutDialog() : m_hInst(NULL), m_hWnd(NULL), m_hParent(NULL) {
		m_hPictureCtrl = NULL;
		m_hDefaultPictureProc = NULL;
	};
	virtual ~AboutDialog() {
		if(!m_hPictureCtrl) m_hPictureCtrl = NULL;
	};

	UINT Create(HINSTANCE hInst, HWND hParent);

	static LRESULT CALLBACK dlgProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
		if(Message == WM_INITDIALOG)
		{
			::SetWindowLong(hWnd, GWL_USERDATA, (long)lParam);
			((AboutDialog *)(lParam))->runDialogProc(hWnd, Message, wParam, lParam);
			return TRUE;
		}
		else
		{
#pragma warning( disable: 4312 )
			AboutDialog* pDlg = reinterpret_cast<AboutDialog *>(::GetWindowLong(hWnd, GWL_USERDATA));
#pragma warning( default: 4312 )
			if (!pDlg) return FALSE;
			return pDlg->runDialogProc(hWnd, Message, wParam, lParam);
		}
	};

	virtual LRESULT CALLBACK runDialogProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);

protected:
	BOOL OnInitDialog(HWND hWnd, HWND hWndFocus, LPARAM lParam);
	void OnCommand(HWND hWnd, int id, HWND hWndCtl, UINT codeNotify);

	/* Subclassing Picture */
	LRESULT CALLBACK runPictureProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK wndDefaultPictureProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
#pragma warning( disable: 4312 )
		return (((AboutDialog *)(::GetWindowLong(hWnd, GWL_USERDATA)))->runPictureProc(hWnd, Message, wParam, lParam));
#pragma warning( default: 4312 )
	};

protected:
	HWND m_hWnd;
	HWND m_hParent;
	HINSTANCE m_hInst;

	HWND			m_hPictureCtrl;
	WNDPROC		m_hDefaultPictureProc;
};