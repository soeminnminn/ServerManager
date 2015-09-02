// NotifyIcon.cpp

#include "stdafx.h"
#include "MainDialog.h"

/////////////////////////////////////////////////////////////////////////////////////////
/// NotifyIcon Helper
void MainDialog::AddNotifyIcon(HWND hWnd, UINT uid, HICON hIcon, const TCHAR* tooltip)
{
	nidApp.cbSize = sizeof(NOTIFYICONDATA);				// sizeof the struct in bytes 
	nidApp.hWnd = (HWND) hWnd;								//handle of the window which will process this app. messages 
	nidApp.uID = uid;													//ID of the icon that willl appear in the system tray 
	nidApp.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP; //ORing of all the flags 
	nidApp.hIcon = hIcon;											// handle of the Icon to be displayed, obtained from LoadIcon 
	nidApp.uCallbackMessage = WM_USER_SHELLICON; 
	_tcscpy(nidApp.szTip, tooltip);

	::Shell_NotifyIcon(NIM_ADD, &nidApp);
};

void MainDialog::ModifyNotifyIcon(HWND hWnd, UINT uid, HICON hIcon, const TCHAR* tooltip)
{
	nidApp.cbSize = sizeof(NOTIFYICONDATA);				// sizeof the struct in bytes 
	nidApp.hWnd = (HWND) hWnd;								//handle of the window which will process this app. messages 
	nidApp.uID = uid;													//ID of the icon that willl appear in the system tray 
	nidApp.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP; //ORing of all the flags 
	nidApp.hIcon = hIcon;											// handle of the Icon to be displayed, obtained from LoadIcon 
	nidApp.uCallbackMessage = WM_USER_SHELLICON; 
	_tcscpy(nidApp.szTip, tooltip);

	::Shell_NotifyIcon(NIM_MODIFY, &nidApp);
};

