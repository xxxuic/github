// uic 2017.12.08

struct ThreadWrap
{
	typedef void (*UserFunction)(void*);

	ThreadWrap(void *param = NULL, bool delete_param = false, DWORD wait_period = INFINITE, UserFunction idle_function = NULL);
	~ThreadWrap();

	bool SetUserEventsNum(int num);
	bool SetUserReaction(int event_num, HANDLE _event, UserFunction _function);

	DWORD m_wait_period;
	int m_user_events_num;
	HANDLE m_stop_thread_event, m_thread_stopped_event, *m_user_events;
	void *m_param;
	bool m_delete_param;
	
	UserFunction *m_user_function, m_idle_function, m_on_finish_function;
	
	bool m_running;

	void Run(int priority = THREAD_PRIORITY_NORMAL);
	void Stop();
	bool IsRunnung();
};

////////////////////////////////////////////////////////////////////////////////
// end