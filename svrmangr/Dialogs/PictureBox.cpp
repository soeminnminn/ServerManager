// PictureBox.cpp

#include "stdafx.h"
#include "MainDialog.h"
#include "resource.h"

#include <initguid.h>
#include "IImgCtx.h"

// ImgCtx Member
IImgCtx*		m_pImgCtx;
HPALETTE		m_hPal;         
SIZE  			sizeImage;


// Subclass Member
WNDPROC		m_hDefaultPictureProc;
LRESULT CALLBACK wndDefaultPictureProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);

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
/// Subclassing PictureBox
void MainDialog::SubclassPictureBox(HWND hWndCtrl)
{
#pragma warning( disable: 4311 )
#pragma warning( disable: 4312 )
	::SetWindowLong(hWndCtrl, GWL_USERDATA, reinterpret_cast<LONG>(this));
	m_hDefaultPictureProc = reinterpret_cast<WNDPROC>(::SetWindowLong(hWndCtrl, GWL_WNDPROC, reinterpret_cast<LONG>(wndDefaultPictureProc)));
#pragma warning( default: 4312 )
#pragma warning( default: 4311 )
};

/////////////////////////////////////////////////////////////////////////////////////////
/// PictureBox Procdure
LRESULT CALLBACK wndDefaultPictureProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) 
{
#pragma warning( disable: 4312 )
	return ((reinterpret_cast<MainDialog *>(::GetWindowLong(hWnd, GWL_USERDATA)))->PictureBox_OnMessage(hWnd, Message, wParam, lParam));
#pragma warning( default: 4312 )
};

LRESULT CALLBACK MainDialog::PictureBox_OnMessage(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
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
/// ImgCtx Methods
BOOL LoadImage(const TCHAR* resName)
{
	wchar_t res[MAX_PATH] = {NULL};
	HRESULT hr = 0L;
	ULONG ulState = 0;

	m_pImgCtx = NULL;
	hr = ::CoCreateInstance( ((const IID)CLSID_IImgCtx), NULL, CLSCTX_INPROC_SERVER, ((const IID)IID_IImgCtx), (LPVOID*) &m_pImgCtx);
	if (FAILED(hr)) return FALSE;

	//hr = m_pImgCtx->lpVtbl->Load(m_pImgCtx, L"res://loadimg.exe/EXIT.PNG", 0);
	hr = m_pImgCtx->Load(res, 0);
	if (FAILED(hr)) return FALSE;

	do {
		//m_pImgCtx->lpVtbl->GetStateInfo(m_pImgCtx, &ulState, &sizeImage, FALSE);
		m_pImgCtx->GetStateInfo(&ulState, &sizeImage, FALSE);
		
		// This image should load/decode instantly, but everyone deserves some sleep
		Sleep(5);
	} while((ulState & IMGLOAD_LOADING));
	
	//m_pImgCtx->lpVtbl->GetPalette(m_pImgCtx, &m_hPal);
	m_pImgCtx->GetPalette(&m_hPal);

	return TRUE;
};

void DrawImage(HDC hdc, POINT ptImage)
{
	RECT imgrect = {NULL};
	::SelectPalette(hdc, m_hPal, TRUE);
	::RealizePalette(hdc);

	::SetRect(&imgrect, ptImage.x, ptImage.y, sizeImage.cx, sizeImage.cy);

	//m_pImgCtx->lpVtbl->Draw(m_pImgCtx, hdc, &imgrect);
	m_pImgCtx->Draw(hdc, &imgrect);
};

void CleanImage()
{
	if (m_pImgCtx) 
	{
		//m_pImgCtx->lpVtbl->Release(m_pImgCtx);
		m_pImgCtx->Release();
	}
};

