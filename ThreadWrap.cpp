// uic 2017.12.08

#include <stdafx.h>
#include "ThreadWrap.h"

////////////////////////////////////////////////////////////////////////////////
static UINT ThreadWrapFunction(LPVOID param)
{
	ThreadWrap* tw = (ThreadWrap*)param;
	
	int i, events_num = 1 + tw->m_user_events_num;
	HANDLE *events = new HANDLE[events_num];
	events[0] = tw->m_stop_thread_event;
	
	for (i = 0; i < tw->m_user_events_num; i++)
		events[1 + i] = tw->m_user_events[i];

	UINT res, infinite_wait_period = INFINITE;
#ifdef _DEBUG
	infinite_wait_period = 15000;
#endif
	while (true)
	{
		if (tw->m_wait_period != INFINITE)
			res = WaitForMultipleObjects(events_num, events, FALSE, tw->m_wait_period);
		else
			res = WaitForMultipleObjects(events_num, events, FALSE, infinite_wait_period);
		if (res == WAIT_OBJECT_0)
			break;
#ifdef _DEBUG
		if (res == WAIT_TIMEOUT && tw->m_wait_period == INFINITE)
		{
			int zzz = 0;
			//DlgMsgBox_DoModal("INFINITE WAIT FAILED", "INFINITE WAIT FAILED");
		}
#endif
		if (res == WAIT_TIMEOUT && tw->m_idle_function)
		{
			tw->m_idle_function(tw->m_param);
			continue;
		}
		i = res - WAIT_OBJECT_0 - 1;
		if (i >= 0 && i < tw->m_user_events_num && tw->m_user_function[i])
		{
			tw->m_user_function[i](tw->m_param);
			continue;
		}
	};

	delete[] events;

	if (tw->m_on_finish_function)
		tw->m_on_finish_function(tw->m_param);

	SetEvent(tw->m_thread_stopped_event);
	tw->m_running = false;

	return 777333777;
}

////////////////////////////////////////////////////////////////////////////////
// ThreadWrap
////////////////////////////////////////////////////////////////////////////////
ThreadWrap::ThreadWrap(void *param/* = NULL*/, bool delete_param/* = false*/, DWORD wait_period/* = INFINITE*/, UserFunction idle_function/* = NULL*/):
	m_param(param), m_delete_param(delete_param),
	m_wait_period(wait_period), m_idle_function(idle_function),
	m_user_events_num(0), m_user_events(NULL), m_user_function(NULL),
	m_running(false)
{
	m_stop_thread_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_thread_stopped_event = CreateEvent(NULL, TRUE, FALSE, NULL);
}

ThreadWrap::~ThreadWrap()
{
	Stop();
	if (m_delete_param && m_param)
		delete m_param;
	if (m_user_events)
		delete[] m_user_events;
	if (m_user_function)
		delete[] m_user_function;
}

bool ThreadWrap::SetUserEventsNum(int num)
{
	if (num < 0 || num > 1000)
		return false;
	m_user_events_num = num;
	if (m_user_events)
		delete[] m_user_events;
	m_user_events = new HANDLE[m_user_events_num];
	memset(m_user_events, 0, sizeof(HANDLE) * m_user_events_num);
	m_user_function = new UserFunction[m_user_events_num];
	memset(m_user_function, 0, sizeof(UserFunction) * m_user_events_num);
	return true;
}

bool ThreadWrap::SetUserReaction(int event_num, HANDLE _event, UserFunction _function)
{
	if (event_num < 0 || event_num >= m_user_events_num)
		return false;
	m_user_events[event_num] = _event;
	m_user_function[event_num] = _function;
	return true;
}

void ThreadWrap::Run(int priority/* = THREAD_PRIORITY_NORMAL*/)
{
	if (m_running)
		return;
	
	ResetEvent(m_stop_thread_event);
	ResetEvent(m_thread_stopped_event);
	m_running = true;
	AfxBeginThread(ThreadWrapFunction, this, priority);
}

void ThreadWrap::Stop()
{
	if (m_running)
	{
		SetEvent(m_stop_thread_event);
		WaitForSingleObject(m_thread_stopped_event, INFINITE);
	}
}

bool ThreadWrap::IsRunnung()
{
	return m_running;
}

////////////////////////////////////////////////////////////////////////////////
// end