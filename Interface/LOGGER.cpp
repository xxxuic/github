// uic 3.03.2010
//
// модуль для ведения одного или нескольких логов

#include <stdafx.h>

#include "LOGGER.h"
#include "MutexWrap.h"
#include "TXT.h"

#include <list>
#include <map>
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// внутренние типы
////////////////////////////////////////////////////////////////////////////////

struct MessageTypeInfo
{
	DWORD m_type;
	CString m_prefix;
	LOGGER_Type m_enabled_type;
};

struct Logger
{
	Logger(LOGGER_Id id = -1):
		m_file(NULL), m_id(id)
	{
	}
	
	bool Init(CString filepath, int num_strings_to_show, int num_strings_to_flush, CWnd* wnd_for_msg, UINT msg)
	{
		MutexWrap log_access(m_log_mutex, "Logger::Init log_access");
		if (!filepath.IsEmpty())
		{
			m_file = _tfopen(filepath, _T("at"));
			if (m_file == NULL)
				return false;
		}
		m_file_string_counter = 0;
		m_num_strings_to_show = num_strings_to_show;
		m_num_strings_to_flush = num_strings_to_flush;
		m_wnd_for_msg = wnd_for_msg;
		m_msg = msg;
		m_messages.Setup(num_strings_to_show);
		return true;
	}

	void ShutDown()
	{
		MutexWrap log_access(m_log_mutex, "Logger::Shutdown log_access");
		if (m_file)
		{
			fclose(m_file);
			m_file = NULL;
		}
	}

	bool EnableMessageType(DWORD message_type, LOGGER_Type log_type)
	{
		MutexWrap log_access(m_log_mutex, "Logger::EnableMessageType log_access");
		//map<DWORD, MessageTypeInfo>::iterator desired = m_message_types_info.find(message_type);
		if (/*desired == m_message_types_info.end() ||*/
			((log_type == LOGGER_file || log_type == LOGGER_both) && m_file == NULL) ||
			((log_type == LOGGER_view || log_type == LOGGER_both) && m_num_strings_to_show < 1)
			)
			return false;
		//desired->second.m_type = message_type;
		//desired->second.m_enabled_type = log_type;
		m_message_types_info[message_type].m_type = message_type;
		m_message_types_info[message_type].m_enabled_type = log_type;
		return true;
	}
	
	bool GetMassageTypeEnabled(DWORD message_type, LOGGER_Type &log_type)
	{
		MutexWrap log_access(m_log_mutex, "Logger::GetMassageTypeEnabled log_access");
		map<DWORD, MessageTypeInfo>::iterator desired = m_message_types_info.find(message_type);
		if (desired == m_message_types_info.end())
			return false;
		log_type = desired->second.m_enabled_type;
		return true;
	}
	
	bool SetupMessageType(DWORD message_type, CString prefix, LOGGER_Type log_type)
	{
		MutexWrap log_access(m_log_mutex, "Logger::SetupMessageType log_access");
		if (!EnableMessageType(message_type, log_type))
			return false;
		m_message_types_info[message_type].m_prefix = prefix;
		return true;
	}

	bool AddMessage(DWORD message_type, CString text, bool add_time, bool add_date)
	{
		MutexWrap log_access(m_log_mutex, "Logger::AddMessage log_access,\r\nmsg = "/* + text*/);
		if (m_message_types_info.find(message_type) == m_message_types_info.end())
			return false;
		MessageTypeInfo mti = m_message_types_info[message_type];
		LOGGER_Type enable_type = mti.m_enabled_type;
		if ((enable_type == LOGGER_file || enable_type == LOGGER_both) && m_file == NULL ||
			((enable_type == LOGGER_view || enable_type == LOGGER_both) && m_num_strings_to_show < 1))
			return false;
		if (enable_type == LOGGER_none)
			return true;

		CString line;

		if (add_time || add_date)
		{
			SYSTEMTIME st;
			GetLocalTime(&st);

			line = "[";
			if (add_date)
			{
				line +=  TXT("%d.%02d.%02d")<<st.wYear<<st.wMonth<<st.wDay;
				if (add_time)
					line += " ";
			}
			if (add_time)
				line += TXT("%02d:%02d:%02d")<<st.wHour<<st.wMinute<<st.wSecond;
			line += "] ";
		}
	
		if (mti.m_prefix.GetLength() > 0)
			line += mti.m_prefix + _T(" ");

		line += text;

		// скидываем в файл
		if ((enable_type == LOGGER_file || enable_type == LOGGER_both) && m_file != NULL)
		{
			_fputts(line + _T("\n"), m_file);
			m_file_string_counter++;
			if (m_file_string_counter > m_num_strings_to_flush)
			{
				fflush(m_file);
				m_file_string_counter = 0;
			}
		}

		// сохраняем для показа
		if (enable_type == LOGGER_view || enable_type == LOGGER_both && m_num_strings_to_show > 0)
		{
			LOGGER_MessageInfo mi;
			mi.m_text = line;
			mi.m_type = mti.m_type;
			m_messages.Add(&mi);
			if (m_msg != 0 && m_wnd_for_msg != NULL && ::IsWindow(m_wnd_for_msg->m_hWnd))
				m_wnd_for_msg->PostMessage(m_msg, 0, 0);
		}

		return true;
	}

	LOGGER_Messages* Lock()
	{
		MutexWrap_LockMutex(&m_log_mutex);
		return &m_messages;
	}

	void Unlock()
	{
		MutexWrap_UnLockMutex(&m_log_mutex);
	}

	map<DWORD, MessageTypeInfo> m_message_types_info;
	LOGGER_Messages m_messages;

	LOGGER_Id m_id;
	CMutex m_log_mutex;
	FILE* m_file;
	int m_num_strings_to_show, m_num_strings_to_flush, m_file_string_counter, m_messages_filled;
	CWnd* m_wnd_for_msg;
	UINT m_msg;
};

////////////////////////////////////////////////////////////////////////////////
// LOGGER_Messages
////////////////////////////////////////////////////////////////////////////////
LOGGER_Messages::LOGGER_Messages():
	m_buffer(NULL), m_total_messages_added(0)
{
}

LOGGER_Messages::~LOGGER_Messages()
{
	if (m_buffer)
		delete[] m_buffer;
}

void LOGGER_Messages::Setup(int size)
{
	if (m_buffer)
		delete[] m_buffer;
	m_size = size;
	m_buffer = (size > 0) ? new LOGGER_MessageInfo[size] : NULL;
	m_filled = 0;
	m_start_offset = 0;
	m_total_messages_added = 0;
}

void LOGGER_Messages::Add(LOGGER_MessageInfo* mi)
{
	if (m_buffer == NULL)
		return;
	m_total_messages_added++;
	int offset = m_start_offset + m_filled;
	if (offset >= m_size)
		offset -= m_size;
	*(m_buffer + offset) = *mi;
	if (m_filled < m_size)
		m_filled++;
	else
	{
		m_start_offset++;
		if (m_start_offset >= m_size)
			m_start_offset = 0;
	}
}

LOGGER_MessageInfo* LOGGER_Messages::Get(int num)
{
	if (m_buffer == NULL)
		return NULL;
	int offset = m_start_offset + num;
	while (offset >= m_size)
		offset -= m_size;
	return m_buffer + offset;
}

////////////////////////////////////////////////////////////////////////////////
// внутренние данные
////////////////////////////////////////////////////////////////////////////////
static map<LOGGER_Id, Logger*> g_loggers;
static CMutex g_loggers_mutex;

////////////////////////////////////////////////////////////////////////////////
// процедуры
////////////////////////////////////////////////////////////////////////////////

LOGGER_Id LOGGER_InitLog(CString filepath, int num_strings_to_show, int num_strings_to_flush, CWnd* wnd_for_msg, UINT msg)
{
	MutexWrap loggers_access(g_loggers_mutex, "LOGGER_InitLog loggers_access");

	// определение индекса для нового логгера
	LOGGER_Id id = (LOGGER_Id)g_loggers.size();
	map<LOGGER_Id, Logger*>::iterator desired = g_loggers.find(id);
	if (desired != g_loggers.end())
	{
		// видимо какой-то лог уже удалили, ищем свободное место в мапе в сторону уменьшения
		LOGGER_Id search_id = id - 1;
		while (g_loggers.find(search_id) != g_loggers.end())
		{
			search_id--;
			if (search_id == -1) // это уже бред, такого быть не должно, значит в структуре мапа ошибка
			{
				// ищем свободное место в мапе в сторону увеличения
				search_id = id + 1;
				while (g_loggers.find(search_id) != g_loggers.end())
				{
					search_id++;
				}
				break;
			}
		}
		id = search_id;
	}

	// создание и инициализация
	Logger* new_logger = new Logger(id);
	if (!new_logger->Init(filepath, num_strings_to_show, num_strings_to_flush, wnd_for_msg, msg))
	{
		delete new_logger;
		return -1;
	}
	g_loggers[id] = new_logger;
	return id;
}

bool LOGGER_ShutDownLog(LOGGER_Id id)
{
	MutexWrap loggers_access(g_loggers_mutex, "LOGGER_ShutDownLog loggers_access");
	map<LOGGER_Id, Logger*>::iterator desired = g_loggers.find(id);
	if (desired == g_loggers.end())
		return false;
	desired->second->ShutDown();
	delete desired->second;
	g_loggers.erase(id);
	return true;
}

bool LOGGER_SetupMessageType(LOGGER_Id id, DWORD message_type, CString prefix, LOGGER_Type log_type)
{
	MutexWrap loggers_access(g_loggers_mutex, "LOGGER_SetupMessageType loggers_access");
	if (id == -1)
	{
		map<LOGGER_Id, Logger*>::iterator current = g_loggers.begin();
		while (current != g_loggers.end())
		{
			current->second->SetupMessageType(message_type, prefix, log_type);
			current++;
		}
		return true;
	}
	map<LOGGER_Id, Logger*>::iterator desired = g_loggers.find(id);
	if (desired == g_loggers.end())
		return false;
	return desired->second->SetupMessageType(message_type, prefix, log_type);
}

bool LOGGER_EnableMessageType(LOGGER_Id id, DWORD message_type, bool enable, LOGGER_Type log_type)
{
	MutexWrap loggers_access(g_loggers_mutex, "LOGGER_EnableMessageType loggers_access");
	map<LOGGER_Id, Logger*>::iterator desired = g_loggers.find(id);
	if (desired == g_loggers.end())
		return false;
	return desired->second->EnableMessageType(message_type, log_type);
}

bool LOGGER_GetMessageTypeEnabled(LOGGER_Id id, DWORD message_type, LOGGER_Type &log_type)
{
	MutexWrap loggers_access(g_loggers_mutex, "LOGGER_EnableMessageType loggers_access");
	map<LOGGER_Id, Logger*>::iterator desired = g_loggers.find(id);
	if (desired == g_loggers.end())
		return false;
	return desired->second->GetMassageTypeEnabled(message_type, log_type);
}

bool LOGGER_AddMessage(LOGGER_Id id, DWORD message_type, CString message, bool add_time, bool add_date)
{
	MutexWrap loggers_access(g_loggers_mutex, "LOGGER_AddMessage loggers_access");
	map<LOGGER_Id, Logger*>::iterator desired = g_loggers.find(id);
	if (desired == g_loggers.end())
		return false;
	return desired->second->AddMessage(message_type, message, add_time, add_date);
}

void LOGGER_LockAll()
{
	MutexWrap_LockMutex (&g_loggers_mutex);
}

void LOGGER_UnlockAll()
{
	MutexWrap_UnLockMutex (&g_loggers_mutex);
}

LOGGER_Messages* LOGGER_Lock(LOGGER_Id id)
{
	//MutexWrap loggers_access(g_loggers_mutex, "LOGGER_Lock loggers_access");
	map<LOGGER_Id, Logger*>::iterator desired = g_loggers.find(id);
	if (desired == g_loggers.end())
		return NULL;
	return desired->second->Lock();
}

bool LOGGER_Unlock(LOGGER_Id id)
{
	//MutexWrap loggers_access(g_loggers_mutex, "LOGGER_Unlock loggers_access");
	map<LOGGER_Id, Logger*>::iterator desired = g_loggers.find(id);
	if (desired == g_loggers.end())
		return false;
	desired->second->Unlock();
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// end