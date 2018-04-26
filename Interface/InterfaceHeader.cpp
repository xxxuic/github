// uic 15.12.2009

#include <stdafx.h>
#include "InterfaceHeader.h"

#include <Resource.h>

#include <list>
using namespace std;

CFont * InterfaceHeader::m_small_font = NULL;
CFont * InterfaceHeader::m_big_font = NULL;

InterfaceHeader::InterfaceHeader():
	m_active(true), m_main_text_aligned_to_left(false), m_draw_text_shadow(false), m_shadow_distance(1),
	m_move_parent(false), m_parent_is_moving(false), m_close_function(NULL), m_has_close_button(true),
	m_has_minimize_button(false), m_draw_function(NULL)
{
}

void InterfaceHeader::OnCreate()
{
	SetMessageHandler(WM_LBUTTONDOWN, (InterfaceObject_OwnFunction)(&InterfaceHeader::CaptureParent));
	SetMessageHandler(WM_MOUSEMOVE, (InterfaceObject_OwnFunction)(&InterfaceHeader::MoveParent));
	SetMessageHandler(WM_LBUTTONUP, (InterfaceObject_OwnFunction)(&InterfaceHeader::ReleaseParent));
}

void InterfaceHeader::CaptureParent()
{
	if (!m_move_parent)
		return;
	m_parent_is_moving = true;
	m_click_point = m_winmessage_point;
	m_wnd->ClientToScreen(&m_click_point);
}

void InterfaceHeader::MoveParent()
{
	if (!m_move_parent || !m_parent_is_moving)
		return;

	CWnd* parent_wnd = m_wnd->GetParent();
	if (parent_wnd == NULL)
		return;

	CPoint point = m_winmessage_point;
	m_wnd->ClientToScreen(&point);
	int dx = point.x - m_click_point.x, dy = point.y - m_click_point.y;
	m_click_point = point;

	CRect parent_rect;
	parent_wnd->GetWindowRect(&parent_rect);
	parent_wnd->SetWindowPos(NULL, parent_rect.left + dx, parent_rect.top + dy, 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER);
	parent_wnd->Invalidate(FALSE);
}

void InterfaceHeader::ReleaseParent()
{
	if (!m_move_parent)
		return;
	m_parent_is_moving = false;
}

void DefaultCloseFunction(BmpButton* button)
{
	CWnd* bmp_wnd = button->GetWnd();
	if (bmp_wnd == NULL)
		return;
	CWnd* header_wnd = bmp_wnd->GetParent();
	if (header_wnd == NULL)
		return;
	CWnd* parent_wnd = header_wnd->GetParent();
	if (parent_wnd == NULL)
		return;
	parent_wnd->PostMessage(WM_CLOSE, 0, 0);
}

void MinimizeFunction(BmpButton* button)
{
	//AfxGetMainWnd()->PostMessage(WM_SIZE, SIZE_MINIMIZED, SIZE_MINIMIZED);
	AfxGetMainWnd()->ShowWindow(SW_MINIMIZE);
}

void InterfaceHeader::OnChangeSize()
{
	CRect rect;
	m_wnd->GetClientRect(&rect);
	int cx = rect.Width(), cy = rect.Height();

	if (m_has_close_button)
	{
		if (m_close_button.GetWnd() == NULL || !::IsWindow(m_close_button.GetWnd()->m_hWnd))
		{
			m_close_button.Create((m_close_function == NULL) ? DefaultCloseFunction : m_close_function, IDB_CLOSE_ACTIVE, IDB_CLOSE_INACTIVE, m_wnd, -1, 0, 0, 10, 10, WS_VISIBLE);
			m_close_button.m_bmp_enabled_pressed.Load(IDB_CLOSE_ACTIVE_PRESSED);
			m_close_button.m_draw_fone = m_close_button.m_draw_borders = false;
		}
		int gap = (cy - m_close_button.m_bmp_enabled.m_h)/2 - 1;
		m_close_button.ChangePosition(cx - gap - m_close_button.m_bmp_enabled.m_w, gap, m_close_button.m_bmp_enabled.m_w, m_close_button.m_bmp_enabled.m_h);
	}
	
	if (m_has_minimize_button)
	{
		if (m_minimize_button.GetWnd() == NULL || !::IsWindow(m_minimize_button.GetWnd()->m_hWnd))
		{
			m_minimize_button.Create(MinimizeFunction, IDB_MINIMIZE, IDB_MINIMIZE, m_wnd, -1, 0, 0, 10, 10, WS_VISIBLE);
			m_minimize_button.m_bmp_enabled_pressed.Load(IDB_MINIMIZE_PRESSED);
			m_minimize_button.m_draw_fone = m_minimize_button.m_draw_borders = false;
		}
		int gap = (cy - m_minimize_button.m_bmp_enabled.m_h)/2 - 1;
		m_minimize_button.ChangePosition(cx - gap - m_close_button.m_bmp_enabled.m_w - gap - m_minimize_button.m_bmp_enabled.m_w, gap, m_minimize_button.m_bmp_enabled.m_w, m_minimize_button.m_bmp_enabled.m_h);
	}
}

void InterfaceHeader::GradientFill(CDC* dc, int x, int y, int w, int h, DWORD color_top, DWORD color_bottom)
{
	DWORD color;
	byte col[6];
	col[0] = color_top & 0xFF;
	col[1] = (color_top>>8) & 0xFF;
	col[2] = (color_top>>16) & 0xFF;
	col[3] = color_bottom & 0xFF;
	col[4] = (color_bottom>>8) & 0xFF;
	col[5] = (color_bottom>>16) & 0xFF;
	double k1, k2;
	for (int i = 0; i <= h; i++)
	{
		k2 = double(i) / h;
		k1 = 1 - k2;
		color = byte(k1 * col[0] + k2 * col[3]) | (byte(k1 * col[1] + k2 * col[4])<<8) | (byte(k1 * col[2] + k2 * col[5])<<16);
		dc->FillSolidRect(x, y + i, w, 1, color);
	}
}

void InterfaceHeader::DrawText(CString text, bool align_left, CDC* dc, DWORD text_color, DWORD text_shadow_color, CRect& header_rect)
{
	int x, y;
	list<CString> lines;
	int start_pos = 0;
	int pos = text.Find(_T("\r\n"), start_pos);
	while (pos != -1)
	{
		lines.push_back(text.Mid(start_pos, pos - start_pos));
		start_pos = pos + 2;
		pos = text.Find(_T("\r\n"), start_pos);
	}
	lines.push_back(text.Mid(start_pos));

	CSize sz = dc->GetTextExtent(_T("…Û"));
	x = lines.size();
	y = 2 + (header_rect.Height() - float(sz.cy) * ((x==1) ? 1.1 : (0.9 * x + 0.4))) * 0.5;
	if (align_left)
		x = 35;
	list<CString>::iterator current = lines.begin();
	while (current != lines.end())
	{
		sz = dc->GetTextExtent(*current);
		if (!align_left)
			x = (header_rect.Width() - sz.cx)/2 + 1;
		if (m_draw_text_shadow)
		{
			dc->SetTextColor(text_shadow_color);
			x += m_shadow_distance;
			y += m_shadow_distance;
			dc->TextOut(x, y, *current);
			x -= m_shadow_distance;
			y -= m_shadow_distance;
		}
		dc->SetTextColor(text_color);
		dc->TextOut(x, y, *current);
		y += 0.9 * sz.cy;
		current++;
	}
}

void InterfaceHeader::Draw(CDC* dc)
{
	DWORD text_color = CLR_GetSysColor(m_active?COLOR_3DHIGHLIGHT:COLOR_3DFACE);
	DWORD text_shadow_color = CLR_GetSysColor(m_active?COLOR_WINDOWFRAME:COLOR_3DDKSHADOW);
	DWORD bk_color = CLR_GetSysColor(m_active?COLOR_3DDKSHADOW:COLOR_3DSHADOW);

	CRect rect;
	m_wnd->GetClientRect(&rect);

	int h = rect.Height();
	GradientFill(dc, rect.left, rect.top, rect.Width(), h/2, CLR_MID_COLOR(0xFFFFFF, bk_color), bk_color);
	int y = rect.top + h/2;
	GradientFill(dc, rect.left, y, rect.Width(), h - y, 0x000000, CLR_MID_COLOR(0x000000, bk_color));

	if (!m_lois_bmp.m_loaded)
		m_lois_bmp.Load(IDB_LOIS);
	if (m_lois_bmp.m_loaded)
		m_lois_bmp.Draw(dc, 0, 0);

	dc->SetBkMode(TRANSPARENT);
	CFont *font = m_wnd->GetFont();
	dc->SelectObject(m_big_font);
	DrawText(m_main_text, m_main_text_aligned_to_left, dc, text_color, text_shadow_color, rect);
	if (!m_main_text_aligned_to_left)
	{
		dc->SelectObject(m_small_font);
		DrawText(m_additional_text, true, dc, text_color, text_shadow_color, rect);
	}
	
	if (m_draw_function)
		m_draw_function(dc);
}

void InterfaceHeader::SetActive(bool active)
{
	m_active = active;
	Invalidate();
}

////////////////////////////////////////////////////////////////////////////////
// end