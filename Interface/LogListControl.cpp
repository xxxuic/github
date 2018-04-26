// uic 4.03.2010

#include <stdafx.h>

#include "LogListControl.h"
#include "./Interface/CLR.h"
#include "./MutexWrap.h"
#include "./TXT.h"
#include "./Interface/DlgEditText.h"

struct ToolTip: InterfaceObject
{
	virtual void Draw(CDC* dc)
	{
		CRect r = GetWindowRect();
		dc->FillSolidRect(0, 0, r.Width(), r.Height(), 0x000000);
		dc->FillSolidRect(1, 1, r.Width() - 2, r.Height() - 2, CLR_GetSysColor(COLOR_INFOBK));
		SelectFont(dc);
		dc->TextOut(5, 0, m_text);
		CSize sz = dc->GetTextExtent(m_text);
		if (r.Width() < sz.cx + 20)
		{
			GetWnd()->ClientToScreen(&r);
			ChangePosition(r.left, r.top, sz.cx + 20, r.Height());
		}
	}
	void SetText(CString text)
	{
		m_text = text;
		Invalidate();
	}
	CString m_text;
};

ToolTip g_tool_tip;
bool g_tool_tip_shown = false;
int g_tool_tip_line = -1;

LogListControl::LogListControl():
	ListControl(0, 15, LogListControl_CELL_SIZE), m_log_id(-1), m_def_color(0x000000),
	m_horizontal_offset(0), m_max_horizontal_offset(0),
	m_horizontal_speed(0), m_last_horizontal_dir(0), m_last_horizontal_offset_tick(0)
{
//	SetMessageHandler(WM_MOUSEMOVE, (InterfaceObject_OwnFunction)&LogListControl::OnMouseMove);
	SetMessageHandler(WM_MOUSEWHEEL, (InterfaceObject_OwnFunction)(&LogListControl::OnMouseWheel));
	SetMessageHandler(WM_MBUTTONDOWN, (InterfaceObject_OwnFunction)(&LogListControl::OnMouseWheelPressed));
	SetMessageHandler(WM_RBUTTONDOWN, (InterfaceObject_OwnFunction)(&LogListControl::OnMouseRightPressed));
}

void LogListControl::OnMouseRightPressed()
{
	CString text;
	int line = NumCellOf(m_winmessage_point, text);
	if (line == -1)
		return;
	int cell_h = GetConstantCellsHeight(),
		num_vis_cell = line - m_vertical_scroll_bar.GetFirstVisibleElement() / cell_h;

	CFont* prev = DlgEditText_font;
	CFont *fnt = m_own_font ? m_own_font : (m_font ? m_font : NULL);
	if (fnt)
		DlgEditText_font = fnt;
	DlgEditText_Show(&text, false, m_window_pos.x, m_window_pos.y + num_vis_cell * cell_h, GetWindowRect().Width() - m_vertical_scroll_bar.GetWindowRect().Width() + 1, cell_h, GetWnd(), ES_LEFT | ES_AUTOHSCROLL);
	DlgEditText_font = prev;
}

void LogListControl::OnMouseWheelPressed()
{
	m_horizontal_offset = 0;
	Invalidate();
}

void LogListControl::OnMouseWheel()
{
	if ((m_winmessage_flag & MK_CONTROL) == 0)
		ListControl::OnMouseWheel();
	else
	{
		int dir = m_wheel_direction > 0 ? +1 : -1;
		if (m_last_horizontal_dir != dir || GetTickCount() - m_last_horizontal_offset_tick > 1000)
		{
			m_last_horizontal_offset_tick = GetTickCount();
			m_horizontal_speed = 1;
			m_last_horizontal_dir = dir;
		}
		else
		if (m_horizontal_speed < 50)
		{
			m_horizontal_speed++;
		}
		m_horizontal_offset += dir * m_horizontal_speed;
		// проверка на границы
		if (m_horizontal_offset < 0)
			m_horizontal_offset = 0;
		if (m_horizontal_offset > m_max_horizontal_offset)
			m_horizontal_offset = m_max_horizontal_offset;
		Invalidate();
	}
}

int LogListControl::NumCellOf(CPoint pt, CString &text)
{
	MutexWrap list_access(m_list_mutex, "ListControl::Draw list_access");

	ListControl_CellInfo* buff = m_cells.GetBuff();
	int num = 0, h_cell = GetConstantCellsHeight(), total_h = 0, y = 0, top_visible = m_vertical_scroll_bar.GetFirstVisibleElement();
	while (num < m_filled_cells)
	{
		if (total_h + h_cell > top_visible)
		{
			y = total_h - top_visible;
			break;
		}
		total_h += h_cell;
		num++;
	}
	if (total_h + h_cell > top_visible)
	{
		y++;
		while (num < m_filled_cells)
		{
			if (pt.y >= y && pt.y <= y + h_cell)
			{
				text = m_messages->Get(num)->m_text;
				return num;
			}
			y += h_cell;
			num++;
		}
	}
	return -1;
}

void LogListControl::OnMouseMove()
{
	KillTimer(1);
	
	CString text;
	int line = NumCellOf(m_winmessage_point, text);

	if (g_tool_tip_shown)
	{
		if (line != g_tool_tip_line || text.Compare(g_tool_tip.m_text) != 0)
		{
			g_tool_tip_line = line;
			g_tool_tip.SetText(text);
		}
		if (!IsMouseIn())
		{
			g_tool_tip_shown = false;
			g_tool_tip.GetWnd()->SendMessage(WM_CLOSE, 0, 0);
		}
	}
	else
	if (IsMouseIn())
	{
		// определяем номер строчки и текст
		SetTimer(1, 500);
	}
}

void LogListControl::OnTimer(UINT timer_id)
{
	if (timer_id == 1)
	{
		g_tool_tip_shown = true;
		CWnd* wnd = g_tool_tip.GetWnd();
		if (wnd->m_hWnd == 0)
			g_tool_tip.Create(GetWnd(), 0, 0, 100, 100, false, false, CLR_GetSysColor(COLOR_INFOBK), false, 0, 0, 0);
		//wnd = g_tool_tip.GetWnd();
		if (wnd->m_hWnd)
		{
			CPoint pt = m_winmessage_point;
			GetWnd()->ClientToScreen(&pt);
			wnd->ModifyStyle(WS_CAPTION | WS_DLGFRAME, WS_POPUP, 0);
			g_tool_tip.ChangePosition(pt.x + 3,  pt.y - 8, 200, 16, SWP_SHOWWINDOW);
			//wnd->ShowWindow(SW_NORMAL);
		}
		CString text;
		g_tool_tip_line = NumCellOf(m_winmessage_point, text);
		g_tool_tip.SetText(text);
		KillTimer(1);
	}
}

void LogListControl::Draw(CDC* dc)
{
	LOGGER_LockAll();
	do
	{
		m_max_horizontal_offset = 0;
		m_messages = LOGGER_Lock(m_log_id);
		if (m_messages)
		{
			int num = m_messages->GetNumOfMessages();
			bool reset_visible = (num > m_filled_cells) && (m_vertical_scroll_bar.GetFirstVisibleElement()  == (m_filled_cells * m_const_cells_height - m_vertical_scroll_bar.GetVisibleElements())); // добавлено сообщение и видели последнее
			while (num > m_filled_cells)
			{
				InsertCell(-1, 0);
			}
			if (reset_visible)
				m_vertical_scroll_bar.SetFirstVisibleElement((m_filled_cells - m_vertical_scroll_bar.GetVisibleElements() / m_const_cells_height) * m_const_cells_height);
		}
		ListControl::Draw(dc);
		LOGGER_Unlock(m_log_id);

		m_max_horizontal_offset -= GetWindowRect().Width() - 2 - m_vertical_scroll_bar.GetWindowRect().Width();
		if (m_max_horizontal_offset < 0)
			m_max_horizontal_offset = 0;
		if (m_horizontal_offset > m_max_horizontal_offset)
			m_horizontal_offset = m_max_horizontal_offset;
		else
			break;
	} while (true);
	LOGGER_UnlockAll();
}

void LogListControl::SetColorForMessageType(DWORD msg_type, DWORD color)
{
	m_color_table[msg_type] = color;
}

void LogListControl::DrawCell(CDC* dc, int cell_pos, int cell_num, DWORD cell_data, int x, int y, int w, int h)
{
	m_color[0] = 0xFFFFFF;//CLR_MID_COLOR(m_vertical_scroll_bar.GetColor(1), m_vertical_scroll_bar.GetColor(0));
	m_color[1] = 0xFFFFFF;//0xF0F0F0;//CLR_MID_COLOR(m_color[0], m_vertical_scroll_bar.GetColor(1));
	int add = m_messages->GetTotalNumOfMessagesAdded() - m_messages->GetMaxNumOfMessages();
	if (add < 0)
		add = 0;
//	if (m_color_table.size() > 3)
//		int sss = 90;
	dc->FillSolidRect(x, y, w, h, m_color[(add + cell_num) % 2]);
	SelectFont(dc);
	LOGGER_MessageInfo *mi = m_messages->Get(cell_num);
	if (mi == NULL)
		return;
	map<DWORD, DWORD>::iterator desired = m_color_table.find(mi->m_type);
	if (desired != m_color_table.end())
		dc->SetTextColor(desired->second);
	else
		dc->SetTextColor(m_def_color);
	dc->TextOut(x + 5 - m_horizontal_offset, y + 1, mi->m_text);
	CSize sz = dc->GetTextExtent(mi->m_text);
	if (5 + sz.cx + 5 > m_max_horizontal_offset)
		m_max_horizontal_offset = 5 + sz.cx + 5;
}

////////////////////////////////////////////////////////////////////////////////
// end