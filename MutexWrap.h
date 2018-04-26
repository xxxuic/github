#pragma once

#include <afxmt.h>

struct MutexWrap
{
	MutexWrap(CMutex &mutex, const char* name = "");
	~MutexWrap();
	CMutex* m_mutex;
	CString m_name;
//#ifdef _DEBUG
	bool m_debug_error;
//#endif
};

//#ifdef _DEBUG
DWORD MutexWrap_InfiniteWaitForSingleObject(HANDLE the_event);
DWORD MutexWrap_InfiniteWaitForMultipleObject(int num, HANDLE* the_events, BOOL all);
//#else
//#define MutexWrap_InfiniteWaitForSingleObject(e) WaitForSingleObject(e, INFINITE);
//#endif

void MutexWrap_Init();
void MutexWrap_ShutDown();

bool MutexWrap_LockMutex(CMutex *mutex);
void MutexWrap_UnLockMutex(CMutex *mutex);
////////////////////////////////////////////////////////////////////////////////
// end