// uic 29.10.2009

#include <stdafx.h>
#include "BmpButton.h"
#include "CLR.h"
#include <list>
using namespace std;

CFont* BmpButton::m_BmpButton_font = NULL;
BmpButtonFunction BmpButton::m_s_dbg_before_action_function = NULL;

BmpButton::BmpButton():
	m_enabled(true), m_pressed(false), m_clicked(false), m_draw_bmp(true), m_draw_text(true),
	m_bmp_horizontal_orientation(0), m_bmp_vertical_orientation(0), m_text_horizontal_orientation(0), m_text_vertical_orientation(0), // -1 - левое равнение, 0 - центр, +1 - правое
	m_id(0), m_draw_fone(true), m_draw_borders(true), m_draw_only_enabled_bmp(false), m_continuous(false), m_thread_is_running(false),
	m_action(NULL)
{
	m_unpress_event = CreateEvent(NULL, TRUE, FALSE, NULL);
}

BmpButton::~BmpButton()
{
	SetEvent(m_unpress_event); // на всякий случай
}

bool BmpButton::OwnSelectFont(CDC*dc)
{
	if (BmpButton::m_BmpButton_font)
	{
		dc->SelectObject(BmpButton::m_BmpButton_font);
		return true;
	}
	return false;
}

void BmpButton::Create(CString name, BmpButtonFunction action, CWnd* parent)
{
	BmpButton::Create(action, -1, -1, parent, -1, 0, 0, 10, 10, 0);
	m_text = name;
}

void BmpButton::Create(BmpButtonFunction action, UINT bmp_enable_id, UINT bmp_disable_id, CWnd* parent, UINT id, int x, int y, int w, int h, DWORD add_ws)
{
	m_action = action;

	if (bmp_enable_id != -1)
		m_bmp_enabled.Load(bmp_enable_id);
	if (bmp_disable_id != -1)
		m_bmp_disabled.Load(bmp_disable_id);
	m_color_norm = CLR_GetSysColor(COLOR_BTNFACE);
	m_color_light = CLR_GetSysColor(COLOR_BTNHIGHLIGHT);
	m_color_shadow = CLR_GetSysColor(COLOR_BTNSHADOW);

	SetMessageHandler(WM_LBUTTONDOWN, (InterfaceObject_OwnFunction)(&BmpButton::OnMouseDown));
	SetMessageHandler(WM_LBUTTONUP, (InterfaceObject_OwnFunction)(&BmpButton::OnMouseUp));
	SetMessageHandler(WM_MOUSEMOVE, (InterfaceObject_OwnFunction)(&BmpButton::OnMouseMove));

	InterfaceObject::Create(parent, x, y, w, h, true, false, 0x808080, false, 0, add_ws);
}

static UINT ContinuousThreadProcedure(LPVOID param)
{
	BmpButton* button = (BmpButton*)param;
	button->m_thread_is_running = true;
	if (BmpButton::m_s_dbg_before_action_function)
		BmpButton::m_s_dbg_before_action_function(button);
	button->m_action(button);
	DWORD wait_time = 500;
	while (button->m_pressed)
	{
		if (WaitForSingleObject(button->m_unpress_event, wait_time) == WAIT_OBJECT_0)
			break;
		wait_time = 20; // после первого повтора остальные быстрее
		if (button->m_pressed)
		{
			if (BmpButton::m_s_dbg_before_action_function)
				BmpButton::m_s_dbg_before_action_function(button);
			button->m_action(button);
		}
	}
	button->m_thread_is_running = false;
	return 0;
}

void BmpButton::OnMouseDown()
{
	if (!m_enabled)
		return;
	m_clicked = m_pressed = true;
	Invalidate();
	
	while (m_thread_is_running) // для кликкеров, чтобы старый поток успел засохнуть до начала нового
		Sleep(100);
	ResetEvent(m_unpress_event);
	if (m_action && m_continuous)
		AfxBeginThread(ContinuousThreadProcedure, this, THREAD_PRIORITY_NORMAL);
}

void BmpButton::OnMouseUp()
{
	if (!m_enabled)
		return;
	m_clicked = false;
	if (!m_pressed)
		return;
	m_pressed = false;
	Invalidate();

	if (m_action && !m_continuous)
	{
		if (BmpButton::m_s_dbg_before_action_function)
			BmpButton::m_s_dbg_before_action_function(this);
		m_action(this);
	}
	SetEvent(m_unpress_event);
}

void BmpButton::OnMouseMove()
{
	if (!m_enabled || !m_clicked)
		return;

	bool in = (m_window_rect.PtInRect(m_winmessage_point) == TRUE);
	if (in == m_pressed)
		return;
	m_pressed = in;
	Invalidate();
}

void BmpButton::Draw(CDC* dc)
{
	int w = m_window_rect.Width(), h = m_window_rect.Height();

	int dd;

	if (m_draw_fone)
	{
		dd = (m_draw_borders) ? 2 : 0;
		dc->FillSolidRect(dd, dd, w - 2*dd, h - 2*dd, m_color_norm);
	}

	dd = (m_pressed && !m_bmp_enabled_pressed.m_loaded) ? 1 : 0;

	//dc->DrawState(CPoint((w - sz.cx)/2 + dd, (h - sz.cy)/2 + dd), sz, m_icon, m_enabled?DSS_NORMAL:DSS_DISABLED, HBRUSH(NULL_BRUSH));
	int x, y;
	BMP_Bitmap* bmp = NULL;
	if (m_draw_bmp)
	{
		bmp = (m_enabled || m_draw_only_enabled_bmp) ? ((m_pressed && m_bmp_enabled_pressed.m_loaded) ? &m_bmp_enabled_pressed : &m_bmp_enabled) : (m_bmp_disabled.m_loaded ? &m_bmp_disabled : &m_bmp_enabled);

		if (m_bmp_horizontal_orientation < 0)
			x = -m_bmp_horizontal_orientation;
		else
		if (m_bmp_horizontal_orientation > 0)
			x = w - m_bmp_horizontal_orientation - bmp->m_w;
		else
			x = (w - bmp->m_w)/2;
		if (m_bmp_vertical_orientation < 0)
			y = -m_bmp_vertical_orientation;
		else
		if (m_bmp_vertical_orientation > 0)
			y = h - m_bmp_vertical_orientation - bmp->m_h;
		else
			y = (h - bmp->m_h)/2;
		bmp->Draw(dc, x + dd, y + dd);
	}
	if (m_draw_text)
	{
		SelectFont(dc);
		dc->SetTextColor(m_enabled ? 0x000000 : 0xFFFFFF);
		CSize sz = dc->GetTextExtent(m_text);
		int ww = w, xx = 0, hh = h;
		if (m_draw_bmp && m_bmp_horizontal_orientation != 0)
		{
			xx = bmp->m_w + 2*abs(m_bmp_horizontal_orientation);
			ww -= xx;
		}


		CString line, rest = m_text;
		list<CString> lines;
		int found;
		while (true)
		{
			found = rest.Find('\n');
			if (found != -1)
			{
				lines.push_back(rest.Left(found));
				rest = rest.Right(rest.GetLength() - found - 1);
			}
			else
			{
				lines.push_back(rest);
				break;
			}
		}

		int raz = m_enabled ? 1 : 2;
		if (raz == 2)
			dd++;
		list<CString>::iterator current;
		for (int i = 0; i < raz; i++)
		{
			if (m_text_vertical_orientation < 0)
				y = 3;
			else
			if (m_text_vertical_orientation > 0)
				y = h - 3 - sz.cy * lines.size();
			else
				y = (h - sz.cy * lines.size())/2;

			current = lines.begin();
			while (current != lines.end())
			{
				sz = dc->GetTextExtent(*current);
				if (m_text_horizontal_orientation < 0)
				{
					if (m_draw_bmp && m_bmp_horizontal_orientation < 0)
						x = xx - m_text_horizontal_orientation;
					else
						x = 3;
				}
				else
				if (m_text_horizontal_orientation > 0)
				{
					x = w - sz.cx;
					if (m_draw_bmp && m_bmp_horizontal_orientation < 0)
						x -= 3;
					else
						x -= xx;
				}
				else
				{
					x = (ww - sz.cx)/2;
					if (m_draw_bmp && m_bmp_horizontal_orientation < 0)
						x += xx;
				}
				dc->TextOut(x + dd, y + dd, *current);
				y += sz.cy;
				current++;
			}
			if (raz == 2)
			{
				dc->SetTextColor(m_color_shadow); // для disable
				dd--;
			}
		}
	}

	if (m_draw_borders)
	{
		dc->FillSolidRect(0, 0, w, 1, 0x000000);
		dc->FillSolidRect(0, 0, 1, h, 0x000000);
		dc->FillSolidRect(0, h-1, w, 1, 0x000000);
		dc->FillSolidRect(w-1, 0, 1, h, 0x000000);

		dc->FillSolidRect(1, 1, 1, h-2, m_pressed?m_color_shadow:m_color_light);
		dc->FillSolidRect(1, 1, w-2, 1, m_pressed?m_color_shadow:m_color_light);

		dc->FillSolidRect(w-2, 1, 1, h-2, m_pressed?CLR_MID_COLOR(m_color_shadow,m_color_norm):m_color_shadow);
		dc->FillSolidRect(1, h-2, w-2, 1, m_pressed?CLR_MID_COLOR(m_color_shadow,m_color_norm):m_color_shadow);
	}
}

////////////////////////////////////////////////////////////////////////////////
// end