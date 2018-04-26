#include <StdAfx.h>
#include "./MutexWrap.h"

#ifdef MutexWrap_Report
#include "./Interface/Dialogs/DlgMsgBox.h"
#endif

#ifdef _DEBUG
#ifdef MutexWrap_DEBUG_LISTS
#include <list>
using namespace std;
typedef list<CMutex*> MutexList;
MutexList *g_debug_error_mutex = NULL;
MutexList *g_debug_ok_mutex = NULL;
#endif
#endif

void MutexWrap_Init()
{
#ifdef _DEBUG
#ifdef MutexWrap_DEBUG_LISTS
	if (!g_debug_error_mutex)
		g_debug_error_mutex = new MutexList;
	if (!g_debug_ok_mutex)
		g_debug_ok_mutex = new MutexList;
#endif
#endif
}

void MutexWrap_ShutDown()
{
#ifdef _DEBUG
#ifdef MutexWrap_DEBUG_LISTS
	if (g_debug_error_mutex)
	{
		delete g_debug_error_mutex;
		g_debug_error_mutex = NULL;
	}
	if (g_debug_ok_mutex)
	{
		delete g_debug_ok_mutex;
		g_debug_ok_mutex = NULL;
	}
#endif
#endif
}

MutexWrap::MutexWrap(CMutex &mutex, const char* name):
	m_mutex(&mutex)
#ifdef _DEBUG
	, m_name(name)
#endif
{
#ifdef _DEBUG
m_debug_error = !MutexWrap_LockMutex(m_mutex); // смогли / не смогли залочить мьютекс
#else
	m_mutex->Lock();
#endif
}

MutexWrap::~MutexWrap()
{
#ifdef _DEBUG
	if (!m_debug_error)
		MutexWrap_UnLockMutex(m_mutex);
	else
	{
#ifdef MutexWrap_DEBUG_LISTS
		// убираем из списка плохих >>
		MutexList::iterator current = g_debug_error_mutex->begin();
		while (current != g_debug_error_mutex->end())
		{
			if (m_mutex == *current)
			{
				g_debug_error_mutex->erase(current); // этот дедлочил
				break;
			}
			current++;
		}
		// убираем из списка плохих <<
#endif
	}
#else
	m_mutex->Unlock();
#endif
}

bool MutexWrap_LockMutex(CMutex *mutex)
{
	if (!mutex->Lock(25000))
	{
#ifdef _DEBUG
#ifdef MutexWrap_DEBUG_LISTS
		MutexList::iterator current = g_debug_error_mutex->begin();
		while (current != g_debug_error_mutex->end())
		{
			if (mutex == *current)
				return false; // повторно не заносим
			current++;
		}
		g_debug_error_mutex->push_back(mutex); // заносим в список плохих
		//MessageBox(NULL, CString("LOCK failed: ") + name, "ERROR", MB_OK);
#endif
#endif
#ifdef MutexWrap_Report
		DlgMsgBox_DoModal("MutexWrap_LockMutex", "TIMEOUT");
#endif
		return false;
	}
#ifdef _DEBUG
#ifdef MutexWrap_DEBUG_LISTS
	else
	{
		g_debug_ok_mutex->push_back(mutex); // заносим в список хороших
	}
#endif
#endif
	return true;
}

void MutexWrap_UnLockMutex(CMutex *mutex)
{
	mutex->Unlock();
#ifdef _DEBUG
#ifdef MutexWrap_DEBUG_LISTS
	// убираем из списка хороших >>
	MutexList::iterator current = g_debug_ok_mutex->begin();
	while (current != g_debug_ok_mutex->end())
	{
		if (mutex == *current)
		{
			g_debug_ok_mutex->erase(current);
			break;
		}
		current++;
	}
	// убираем из списка хороших <<
#endif
#endif
}

//#ifdef _DEBUG
DWORD MutexWrap_InfiniteWaitForSingleObject(HANDLE the_event)
{
	DWORD res = WaitForSingleObject(the_event, 250000);
	if (res == WAIT_TIMEOUT)
	{
		int err = 1;
#ifdef MutexWrap_Report
		DlgMsgBox_DoModal("MutexWrap_InfiniteWaitForSingleObject", "TIMEOUT");
#endif
		err++;
	}
	return res;
}

DWORD MutexWrap_InfiniteWaitForMultipleObject(int num, HANDLE* the_events, BOOL all)
{
	DWORD res = WaitForMultipleObjects(num, the_events, all, 250000);
	if (res == WAIT_TIMEOUT)
	{
		int err = 1;
#ifdef MutexWrap_Report
		DlgMsgBox_DoModal("MutexWrap_InfiniteWaitForMultipleObject", "TIMEOUT");
#endif
		err++;
	}
	return res;
}
//#endif
////////////////////////////////////////////////////////////////////////////////
// end