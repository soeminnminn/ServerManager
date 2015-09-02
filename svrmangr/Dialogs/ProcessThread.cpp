// ProcessThread.cpp

#include "stdafx.h"
#include "MainDialog.h"

HANDLE m_hThread;

static DWORD CALLBACK runProcessThread(LPVOID lParam);


/////////////////////////////////////////////////////////////////////////////////////////
/// Thread Helper Methods
void MainDialog::AttachProcessThread(DWORD dwThreadId)
{
	::OutputDebugString( _T("\n AttachProcessThread() "));

	if(!m_hEvent) m_hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	
	// Reset the thread control event
	::ResetEvent(m_hEvent);
	// Begin create the self-extract data file
	m_hThread = ::CreateThread(NULL, 0, &runProcessThread, (LPVOID)this, 0, &dwThreadId);
};

void MainDialog::DetachProcessThread()
{
	::OutputDebugString( _T("\n DetachProcessThread() "));

	DWORD		dwTime		= 0;
	DWORD		dwExitCode = 0;

	if (m_hThread != NULL) ::GetExitCodeThread(m_hThread, &dwExitCode);

	// Check both the thread state
	if (dwExitCode == STILL_ACTIVE)
	{
		::SetEvent(m_hEvent);

		dwTime = ::GetTickCount();
		do
		{
			// DO NOTHING
		} while (::GetTickCount() - dwTime < 500);
	}
	
	// release the used memory
	if (m_hEvent != NULL) ::CloseHandle(m_hEvent);
	m_hEvent = NULL;
};

void MainDialog::StopProcessThread()
{
	::OutputDebugString( _T("\n StopProcessThread() "));

	DWORD		dwExitCode = 0;

	if (m_hThread != NULL) ::GetExitCodeThread(m_hThread, &dwExitCode);
	// Check the thread state.
	if (dwExitCode == STILL_ACTIVE) ::SetEvent(m_hEvent);
};

/////////////////////////////////////////////////////////////////////////////////////////
/// Thread Procture
DWORD CALLBACK runProcessThread(LPVOID lParam)
{
	return ((reinterpret_cast<MainDialog *>(lParam))->DoProcess() ? 1 : 0);
};
