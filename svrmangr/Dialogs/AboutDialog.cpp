// AboutDialog.cpp

#include "stdafx.h"
#include "resource.h"
#include "AboutDialog.h"

/////////////////////////////////////////////////////////////////////////////////////////
/// Drawing Methods
inline void TransparentBlt( HDC hdcDest, int nXDest, int nYDest, int nWidth, 
					int nHeight, HBITMAP hBitmap, int nXSrc, int nYSrc,
					COLORREF colorTransparent, HPALETTE hPal ) 
{
	HDC dc, memDC, maskDC, tempDC;
	dc =  hdcDest;
	maskDC = ::CreateCompatibleDC(dc);
	memDC = ::CreateCompatibleDC(dc);
	tempDC = ::CreateCompatibleDC(dc);
	
	//add these to store return of SelectObject() calls
	HBITMAP pOldMemBmp;
	HBITMAP pOldMaskBmp;
	HBITMAP hOldTempBmp;
	HBITMAP maskBitmap;
	
	HBITMAP bmpImage = ::CreateCompatibleBitmap( dc, nWidth, nHeight );
	pOldMemBmp = (HBITMAP)::SelectObject( memDC, bmpImage );
	
	// Select and realize the palette
	if( ::GetDeviceCaps(dc, RASTERCAPS) & RC_PALETTE && hPal ) {
		::SelectPalette( dc, hPal, FALSE );
		::RealizePalette(dc);
		::SelectPalette( memDC, hPal, FALSE );
	}
	
	hOldTempBmp = (HBITMAP)::SelectObject( tempDC, hBitmap );
	::BitBlt( memDC, 0, 0, nWidth, nHeight, tempDC, nXSrc, nYSrc, SRCCOPY );
	
	// Create monochrome bitmap for the mask
	maskBitmap = ::CreateBitmap( nWidth, nHeight, 1, 1, NULL );
	pOldMaskBmp = (HBITMAP)::SelectObject( maskDC, maskBitmap );
	::SetBkColor( memDC, colorTransparent );
	
	// Create the mask from the memory DC
	::BitBlt( maskDC, 0, 0, nWidth, nHeight, memDC, 0, 0, SRCCOPY );
	
	// Set the background in memDC to black. Using SRCPAINT with black 
	// and any other color results in the other color, thus making 
	// black the transparent color
	::SetBkColor( memDC, RGB(0,0,0));
	::SetTextColor( memDC, RGB(255,255,255));
	::BitBlt( memDC, 0, 0, nWidth, nHeight, maskDC, 0, 0, SRCAND);
	
	// Set the foreground to black. See comment above.
	::SetBkColor( dc, RGB(255,255,255));
	::SetTextColor(dc, RGB(0,0,0));
	::BitBlt( dc, nXDest, nYDest, nWidth, nHeight, maskDC, 0, 0, SRCAND);
	
	// Combine the foreground with the background
	::BitBlt( dc, nXDest, nYDest, nWidth, nHeight, memDC, 0, 0, SRCPAINT);
	
	::SelectObject( tempDC, hOldTempBmp);
	::SelectObject( maskDC, pOldMaskBmp );
	::SelectObject( memDC, pOldMemBmp );

	::DeleteObject(memDC);
	::DeleteObject(maskDC);
	::DeleteObject(tempDC);

	::DeleteObject(maskBitmap);
	::DeleteObject(bmpImage);
	::DeleteObject(hOldTempBmp);
	::DeleteObject(pOldMaskBmp);
	::DeleteObject(pOldMemBmp);
};

/////////////////////////////////////////////////////////////////////////////////////////
/// Do Create and Show Dialog
UINT AboutDialog::Create(HINSTANCE hInst, HWND hParent)
{
	m_hInst = hInst;
	m_hParent = hParent;

	return (UINT)::DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hParent, (DLGPROC)dlgProc, (LPARAM)this);
}

/////////////////////////////////////////////////////////////////////////////////////////
/// Window Procture
LRESULT CALLBACK AboutDialog::runDialogProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message) 
	{
		case WM_INITDIALOG:
			return (LRESULT)(DWORD)(UINT)(BOOL)OnInitDialog((hWnd), (HWND)(wParam), lParam);

		case WM_COMMAND:
			OnCommand((hWnd), (int)(LOWORD(wParam)), (HWND)(lParam), (UINT)HIWORD(wParam));
			break;

		default:
			break;
	}
	return FALSE;
}

LRESULT CALLBACK AboutDialog::runPictureProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch(Message)
	{
		case STM_SETIMAGE:
			::InvalidateRect(hWnd, NULL, FALSE);
			break;

		case WM_PAINT:
			{
				PAINTSTRUCT ps = {NULL};
				HDC hdc = ::BeginPaint(hWnd, &ps);

				HBITMAP hBitmap = (HBITMAP)::SendMessage(hWnd, (UINT)STM_GETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)0L);
				if(!hBitmap) break;

				RECT rcClient = {NULL};
				::GetClientRect(hWnd, &rcClient);
				int iWidth = (rcClient.right - rcClient.left);
				int iHeight = (rcClient.bottom - rcClient.top);
	
				TransparentBlt(hdc, 0, 0, iWidth, iHeight, hBitmap, 0, 0, RGB(255, 0, 255), NULL);

				::EndPaint(hWnd, &ps);
			}
			break;
	}
	return ::CallWindowProc(m_hDefaultPictureProc, hWnd, Message, wParam, lParam);
};

/////////////////////////////////////////////////////////////////////////////////////////
/// Events
BOOL AboutDialog::OnInitDialog(HWND hWnd, HWND hWndFocus, LPARAM lParam)
{
	// SubClass Picture Control
	m_hPictureCtrl = ::GetDlgItem(hWnd, IDC_PIC_ABOUT);
#pragma warning( disable: 4311 )
#pragma warning( disable: 4312 )
	::SetWindowLong(m_hPictureCtrl, GWL_USERDATA, reinterpret_cast<LONG>(this));
	m_hDefaultPictureProc = reinterpret_cast<WNDPROC>(::SetWindowLong(m_hPictureCtrl, GWL_WNDPROC, reinterpret_cast<LONG>(wndDefaultPictureProc)));
#pragma warning( default: 4312 )
#pragma warning( default: 4311 )

	return 1L;
}

void AboutDialog::OnCommand(HWND hWnd, int id, HWND hWndCtl, UINT codeNotify)
{
	switch(id)
	{
		case IDOK:
		case IDCANCEL:
			::EndDialog(hWnd, id);
			break;

		default:
			break;
	}
}

