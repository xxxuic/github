// uic 1.03.2010

#include <stdafx.h>

#include "ScrollBar.h"
#include "CLR.h"

ScrollBar::ScrollBar(bool horisontal, int button_step):
	m_horisontal(horisontal), m_button_step(button_step),
	m_activated(NotActivated),
	m_reaction_hwnd(NULL), m_reaction_wnd(NULL), m_reaction_msg(0)
{
	m_capture_focus = true;
}

void ScrollBar::OnCreate()
{
	DWORD c1 = CLR_GetSysColor(COLOR_3DSHADOW), c2 = CLR_GetSysColor(COLOR_3DDKSHADOW);
	SetColors(CLR_GetSysColor(COLOR_3DHIGHLIGHT), CLR_GetSysColor(COLOR_3DFACE), c1, CLR_MID_COLOR(c1, c2), CLR_GetSysColor(COLOR_WINDOWFRAME));

	SetMessageHandler(WM_MOUSEMOVE, (InterfaceObject_OwnFunction)(&ScrollBar::OnMouseMove));
	SetMessageHandler(WM_LBUTTONDOWN, (InterfaceObject_OwnFunction)(&ScrollBar::OnMouseDown));
	SetMessageHandler(WM_LBUTTONUP, (InterfaceObject_OwnFunction)(&ScrollBar::OnMouseUp));
	SetMessageHandler(WM_MOUSEWHEEL, (InterfaceObject_OwnFunction)(&ScrollBar::OnMouseWheel));
}

void ScrollBar::OnChangeSize()
{
	CalculateRects();
}

////////////////////////////////////////////////////////////////////////////////
// messages >>
////////////////////////////////////////////////////////////////////////////////
void ScrollBar::OnMouseDown()
{
	if (m_visible_elements >= m_total_elements)
	{
		m_activated = NotActivated;
		return;
	}

	CPoint point = m_winmessage_point;

	if (m_low_button_rect.PtInRect(point))
		m_activated = LowButton;
	else
	if (m_high_button_rect.PtInRect(point))
		m_activated = HighButton;
	else
	if (m_slider_rect.PtInRect(point))
	{
		m_activated = Slider;
		m_d_pos = PointToPos(point) - m_first_visible_element;
	}
	else
	if (m_low_space_rect.PtInRect(point))
		m_activated = LowSpace;
	else
	if (m_high_space_rect.PtInRect(point))
		m_activated = HighSpace;

	if (m_activated != Slider)
		SetTimer(1, 20);

	Invalidate();
}

void ScrollBar::OnMouseUp()
{
	if (m_activated != NotActivated)
	{
		m_activated = NotActivated;
		Invalidate();
	}
}

void ScrollBar::OnMouseMove()
{
	CPoint point = m_winmessage_point;
	if (m_activated == Slider && m_prev_winmessage_point != point)
	{		
		int pos = PointToPos(point);
		int el = max(0, min(m_total_elements - m_visible_elements, pos - m_d_pos));
		if (el != m_first_visible_element)
		{
			m_first_visible_element = el;
			m_first_visible_element = max(0, min(m_total_elements - m_visible_elements, m_first_visible_element));
			CalculateRects();
			Invalidate();
			DoReaction();
		}
	}
}

void ScrollBar::OnMouseWheel()
{
	int prev = GetFirstVisibleElement();
	int dir = (m_wheel_direction > 0) ? 1 : ((m_wheel_direction < 0) ? -1 : 0);
	SetFirstVisibleElement(prev - dir * GetButtonStep());
	Invalidate();
	DoReaction();
}

void ScrollBar::DoReaction()
{
	if (m_reaction_hwnd)
		::PostMessage(m_reaction_hwnd, m_reaction_msg, m_first_visible_element, (int)m_activated);
	if (m_reaction_wnd)
		m_reaction_wnd->PostMessage(m_reaction_msg, m_first_visible_element, (int)m_activated);
}

void ScrollBar::OnTimer(UINT timer_id)
{
	if (m_activated == NotActivated)
	{
		KillTimer(timer_id);
		return;
	}

	if (timer_id == 1)
	{
		KillTimer(1);
		SetTimer(2, 300);
	}

	if (timer_id == 2)
	{
		KillTimer(2);
		SetTimer(3, 50);
	}

	int el = m_first_visible_element;

	if (m_activated == LowButton)
		el = max(0, m_first_visible_element - m_button_step);
	else
	if (m_activated == LowSpace && m_low_space_rect.PtInRect(m_winmessage_point))
		el = max(0, m_first_visible_element - m_visible_elements);
	else
	if (m_activated == HighSpace && m_high_space_rect.PtInRect(m_winmessage_point))
		el = min(m_total_elements - m_visible_elements, m_first_visible_element + m_visible_elements);
	else
	if (m_activated == HighButton)
		el = min(m_total_elements - m_visible_elements, m_first_visible_element + m_button_step);

	if (el != m_first_visible_element)
	{
		m_first_visible_element = el;
		CalculateRects();
		Invalidate();
		DoReaction();
	}
}
////////////////////////////////////////////////////////////////////////////////
// messages <<
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// draw >>
////////////////////////////////////////////////////////////////////////////////
void ScrollBar::Draw(CDC* dc)
{
	dc->SelectObject(&m_pen);
	dc->SelectObject(&m_brush);

	dc->FillSolidRect(0, 0, m_window_rect.Width(), m_window_rect.Height(), m_color[3]);

	dc->FillSolidRect(m_low_space_rect, (m_activated == LowSpace && m_low_space_rect.PtInRect(m_winmessage_point))?m_color[3]:m_color[2]);
	dc->FillSolidRect(m_high_space_rect, (m_activated == HighSpace && m_high_space_rect.PtInRect(m_winmessage_point))?m_color[3]:m_color[2]);

	DrawRect(m_low_button_rect, m_activated == LowButton, dc);
	DrawRect(m_high_button_rect, m_activated == HighButton, dc);
	DrawArrows(dc);
	DrawRect(m_slider_rect, m_activated == Slider, dc);
}

void ScrollBar::DrawArrows(CDC* dc)
{
	int all_pos = GetLength() - 3;
	int sz = GetWidth() - 2;
	int across_sz = sz - 6;
	int pos_sz = across_sz/2;
	CPoint pt1, pt2, pt3;

	int pos = sz/2-pos_sz/2;
	PosToXY(pos, sz/2, pt1.x, pt1.y);
	PosToXY(pos+pos_sz, sz/2-across_sz/2, pt2.x, pt2.y);
	PosToXY(pos+pos_sz, sz/2+across_sz/2, pt3.x, pt3.y);
	dc->BeginPath();
	dc->MoveTo(pt1);
	dc->LineTo(pt2);
	dc->LineTo(pt3);
	dc->LineTo(pt1);
	dc->EndPath();
	dc->StrokeAndFillPath();

	PosToXY(all_pos - (pos), sz/2, pt1.x, pt1.y);
	PosToXY(all_pos - (pos+pos_sz), sz/2-across_sz/2, pt2.x, pt2.y);
	PosToXY(all_pos - (pos+pos_sz), sz/2+across_sz/2, pt3.x, pt3.y);
	dc->BeginPath();
	dc->MoveTo(pt1);
	dc->LineTo(pt2);
	dc->LineTo(pt3);
	dc->LineTo(pt1);
	dc->EndPath();
	dc->StrokeAndFillPath();
}

void ScrollBar::DrawRect(CRect rect, bool downed, CDC*dc)
{
	dc->FillSolidRect(rect, m_color[0]);
	InflateRect(rect, -1, -1);
//	DWORD c1 = downed?m_color[2]:m_color[1], c2 = downed?CLR_MID_COLOR(m_color[2], m_color[3]):CLR_MID_COLOR(m_color[1], m_color[2]), c3 = downed?m_color[4]:m_color[3], c4 = downed?m_color[3]:m_color[2];
//	CLR_GradientFill(dc, rect.left, rect.top, rect.Width(), rect.Height()/2, c1, c2);
//	CLR_GradientFill(dc, rect.left, rect.top + rect.Height() / 2, rect.Width(), rect.Height() - rect.Height()/2 - 1, c3, CLR_MID_COLOR(c3, c4));

//	DWORD c1 = downed?m_color[2]:m_color[1], c2 = downed?m_color[3]:m_color[2], c3 = downed?m_color[4]:m_color[3], c4 = downed?m_color[3]:m_color[2];
//	CLR_GradientFill(dc, rect.left, rect.top, rect.Width(), rect.Height()/2, CLR_MID_COLOR(c1, c2), c2);
//	CLR_GradientFill(dc, rect.left, rect.top + rect.Height() / 2, rect.Width(), rect.Height() - rect.Height()/2 - 1, CLR_MID_COLOR(c3, c4), c4);

	dc->FillSolidRect(rect, downed?m_color[3]:m_color[1]);
}
////////////////////////////////////////////////////////////////////////////////
// draw <<
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// coordinates >>
////////////////////////////////////////////////////////////////////////////////
int ScrollBar::GetLength()
{
	return m_horisontal ? m_window_rect.Width() : m_window_rect.Height();
}

int ScrollBar::GetWidth()
{
	return m_horisontal ? m_window_rect.Height() : m_window_rect.Width();
}

void ScrollBar::PosToXY(int along, int across, LONG &x, LONG &y)
{
	x = 1 + (m_horisontal ? along : across);
	y = 1 + (m_horisontal ? across : along);
}

int ScrollBar::PointToPos(CPoint point)
{
	int pos = m_horisontal ? point.x : point.y;
	if (pos < GetWidth())
		return 0;
	if (pos > GetLength() - GetWidth())
		return m_total_elements;
	int free_len = GetLength() - 2*GetWidth();
	pos -= GetWidth();
	return m_total_elements*pos/free_len;
}

void ScrollBar::CalculateRects()
{
	int free_len = GetLength() - 2*GetWidth();
	double perc = (m_total_elements == 0) ? 1.0 : (double(m_visible_elements)/m_total_elements);
	int dragger_len = max(5, int(perc*free_len));
	int offset = (m_total_elements == m_visible_elements)?0:((free_len - dragger_len)*m_first_visible_element/(m_total_elements - m_visible_elements));

	int sz = GetWidth() - 2;
	GetRect(0, sz, m_low_button_rect);
	GetRect(GetLength() - GetWidth(), sz, m_high_button_rect);
	GetRect(GetWidth() - 1 + offset, dragger_len, m_slider_rect);

	GetRect(sz, offset+1, m_low_space_rect);
	GetRect(GetWidth() - 1 + offset + dragger_len, free_len - dragger_len - offset + 1, m_high_space_rect);
}

void ScrollBar::GetRect(int pos, int len, CRect &rect)
{
	PosToXY(pos, 0, rect.left, rect.top);
	PosToXY(pos + len, GetWidth() - 2, rect.right, rect.bottom);
}
////////////////////////////////////////////////////////////////////////////////
// coordinates <<
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// set params >>
////////////////////////////////////////////////////////////////////////////////
void ScrollBar::OnSetParams(bool redraw)
{
	if (m_visible_elements > m_total_elements)
		m_visible_elements = m_total_elements;
	if (m_first_visible_element + m_visible_elements > m_total_elements)
		m_first_visible_element = m_total_elements - m_visible_elements;
	CalculateRects();
	if (redraw)
		Invalidate(FALSE);
}

void ScrollBar::SetTotalElements(int total, bool redraw)
{
	m_total_elements = max(1, total);
	OnSetParams(redraw);
}

void ScrollBar::SetVisibleElements(int visible, bool redraw)
{
	m_visible_elements = max(1, visible);
	OnSetParams(redraw);
}

void ScrollBar::SetFirstVisibleElement(int first, bool redraw)
{
	m_first_visible_element = max(0, first);
	OnSetParams(redraw);
}
////////////////////////////////////////////////////////////////////////////////
// set params <<
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// end