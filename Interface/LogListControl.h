// uic 4.03.2010

#pragma once

#include "ListControl.h"
#include "LOGGER.h"

#define LogListControl_CELL_SIZE 15

struct LogListControl: ListControl
{
	LogListControl();
	virtual void OnSetButtonStep() {};

	DWORD m_color[2];
	LOGGER_Id m_log_id;

	LOGGER_Messages* m_messages;

	virtual void Draw(CDC* dc);
	virtual void DrawCell(CDC* dc, int cell_pos, int cell_num, DWORD cell_data, int x, int y, int w, int h);

	int NumCellOf(CPoint pt, CString &text);

	void OnMouseMove();
	void OnMouseWheel();
	void OnMouseWheelPressed();
	void OnMouseRightPressed();

	virtual void OnTimer(UINT timer_id);
	
	void SetColorForMessageType(DWORD msg_type, DWORD color);
	map<DWORD, DWORD> m_color_table;
	DWORD m_def_color;
	
	int m_horizontal_offset, m_max_horizontal_offset, m_last_horizontal_dir, m_horizontal_speed;
	DWORD m_last_horizontal_offset_tick;
};

////////////////////////////////////////////////////////////////////////////////
// end