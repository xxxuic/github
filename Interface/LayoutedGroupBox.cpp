// uic 2010.11.3

#include <stdafx.h>

#include "LayoutedGroupBox.h"

int LayoutedGroupBox_header_h = 23;

BEGIN_MESSAGE_MAP(MyBoxWnd, CStatic)
	ON_WM_PAINT()
END_MESSAGE_MAP()

void MyBoxWnd::OnPaint()
{
	CPaintDC pdc(this);
	CRect rect;
	GetClientRect(&rect);
	CGdiObject* prev_brush = pdc.SelectStockObject(NULL_BRUSH);
	DWORD col = CLR_MID_COLOR(0xFFFFFF, CLR_GetSysColor(COLOR_3DFACE));
	//DWORD col = CLR_MID_COLOR(CLR_GetSysColor(COLOR_3DSHADOW), CLR_GetSysColor(COLOR_3DFACE));
	CPen pen1(PS_SOLID, 1, DWORD(0x000000));
	CPen pen2(PS_SOLID, 1, col);
	CGdiObject* prev_pen = pdc.SelectObject(&pen1);
	pdc.Rectangle(&rect);
	/**/
	rect.DeflateRect(1, 1, 1, 1);
	pdc.SelectObject(&pen2);
	pdc.Rectangle(&rect);
	/**/
	pdc.SelectObject(prev_pen);
	pdc.SelectObject(prev_brush);
}
	
LayoutedGroupBox::LayoutedGroupBox(CString name):
	m_name(name), m_can_be_closed(true)
{
}

void LayoutedGroupBox::Create(CWnd* parent, LAYOUT_Direction open_direction, bool draw_layout_borders)
{
	m_open_direction = open_direction;
	m_draw_layout_borders = draw_layout_borders;
	m_header_button.Create(parent, m_name, open_direction, true, -1, NULL, 0);
	m_header_button.m_enabled = m_can_be_closed;
	if (m_draw_layout_borders)
	{
		m_box_window.Create(NULL, 0, CRect(0, 0, 100, 100), parent);
		m_box_window.ModifyStyleEx(0, WS_EX_TRANSPARENT);
	}
}

void LayoutedGroupBox::CreateButtonLayout(int left_gap, int right_gap, int top_gap, int bottom_gap)
{
	int w = -1, h = LayoutedGroupBox_header_h;
	if (m_open_direction >= LAYOUT_direction_left)
		swap(w, h);
	m_form->StartLayout(&m_header_button, LAYOUT_orientation_vertical, w, h, left_gap, right_gap, top_gap, bottom_gap);
	m_form->FinishLayout(_T("LayoutedGroupBox: ") + m_name + _T(" m_header_button"));
}

void LayoutedGroupBox::StartGroupBox(InterfaceForm* form, int w, int h, int left_gap, int right_gap, int top_gap, int bottom_gap, int inner_gap, LAYOUT_Orientation orientation)
{
	m_form = form;

	m_left_gap = left_gap; m_right_gap = right_gap; m_top_gap = top_gap; m_bottom_gap = bottom_gap;
	m_inner_gap = (inner_gap < 0) ? (m_draw_layout_borders ? InterfaceForm::m_gap + 1 : 0) : inner_gap;
	if (m_open_direction == LAYOUT_direction_bottom || m_open_direction == LAYOUT_direction_right)
		CreateButtonLayout(left_gap, right_gap, top_gap, bottom_gap);

	LAYOUT_Orientation body_orientation = (orientation == LAYOUT_useless) ? ((m_open_direction >= LAYOUT_direction_left) ? LAYOUT_orientation_horizontal : LAYOUT_orientation_vertical) : orientation;
	if (m_draw_layout_borders)
	{
		m_form->StartLayout(&m_box_window, body_orientation, w, h);
		m_header_button.m_layout = m_form->GetCurrentLayout();
		if (m_inner_gap > 0)
			m_form->StartEmptyLayout(body_orientation, w, h, m_inner_gap, m_inner_gap, m_inner_gap, m_inner_gap);
	}
	else
	{
		m_form->StartEmptyLayout(body_orientation, w, h, m_inner_gap, m_inner_gap, m_inner_gap, m_inner_gap);
		m_header_button.m_layout = m_form->GetCurrentLayout();
	}
	m_header_button.m_layout->m_transparent = true;
	m_header_button.LoadOpenState(false);
}

void LayoutedGroupBox::FinishGroupBox()
{
	if (m_draw_layout_borders)
	{
		if (m_inner_gap > 0)
			m_form->FinishLayout(_T("LayoutedGroupBox: ") + m_name + _T(" gap layout"));
		m_form->FinishLayout(_T("LayoutedGroupBox: ") + m_name + _T(" m_box_window"));
	}
	else
		m_form->FinishLayout(_T("LayoutedGroupBox: body") + m_name);

	if (m_open_direction == LAYOUT_direction_top || m_open_direction == LAYOUT_direction_left)
		CreateButtonLayout(m_left_gap, m_right_gap, m_top_gap, m_bottom_gap);
}

////////////////////////////////////////////////////////////////////////////////
// end