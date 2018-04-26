// uic 10.03.2010

#include <stdafx.h>

#include "CS.h"

CFont* CS::m_CS_font = NULL;
DWORD CS::m_colors[3] = {0x808080, };

CS::CS():
	m_bottom_less(false), m_horizontal_text_alignment(0), m_vertical_text_alignment(0)
{
}

bool CS::OwnSelectFont(CDC*dc)
{
	if (CS::m_CS_font)
	{
		dc->SelectObject(CS::m_CS_font);
		return true;
	}
	return false;
}

void CS::Draw(CDC*dc)
{
	CRect rect;
	DWORD *colors = m_use_own_colors ? m_own_colors : m_colors;
	GetWnd()->GetClientRect(&rect);
	if (m_bottom_less)
		rect.bottom++;
	CBrush rect_brush(colors[0]);
	CPen rect_pen(PS_SOLID, 1, colors[1]);
	dc->SelectObject(&rect_brush);
	dc->SelectObject(&rect_pen);
	dc->Rectangle(rect);
	SelectFont(dc);
	CSize sz = dc->GetTextExtent(m_text);

	int x, y;

	if (m_horizontal_text_alignment < 0)
	{
		x = rect.Width() - sz.cx + (m_horizontal_text_alignment + 1);
	}
	else
	if (m_horizontal_text_alignment > 0)
	{
		x = m_horizontal_text_alignment - 1;
	}
	else
	{
		x = (rect.Width() - sz.cx)/2 + 1;
	}
	if (m_vertical_text_alignment > 0)
	{
		y = rect.Height() - sz.cy - m_vertical_text_alignment;
	}
	else
	if (m_vertical_text_alignment < 0)
	{
		y = -(1 + m_vertical_text_alignment);
	}
	else
	{
		y = (rect.Height() - sz.cy)/2 + 1;
	}
	dc->SetTextColor(colors[2]);
	dc->TextOut(x, y, m_text);

}

////////////////////////////////////////////////////////////////////////////////
// end