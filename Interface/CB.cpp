// uic 2017.10.20
// очистил CB от скверны ресурсной зависимости

#include <stdafx.h>

#include "CB.h"
#include "CLR.h"

// Кнопка для выбора выбора значений булевой переменной
CB::CB()
{
	m_bmp_vertical_orientation = 0;
	m_text_horizontal_orientation = -1;
	m_text_vertical_orientation = 0;
	m_draw_borders = false;
	m_draw_fone = true;
}

void CB::OnMouseOut()
{
	m_color_norm = m_tmp_norm_color;
	Invalidate();
}

void CB::OnMouseIn()
{
	if (!m_enabled)
		return;
	m_tmp_norm_color = m_color_norm;
	m_color_norm = CLR_MID_COLOR(m_color_norm, CLR_MID_COLOR(m_color_norm, m_color_shadow));
	Invalidate();
}

void CB::OnCreate()
{
	m_tmp_norm_color = m_color_norm;
	StateButton<bool>::OnCreate();
}

void CB::Setup(bool* value, CString name)
{
	bool values[2] = { true, false };
	StateButton<bool>::Setup(value, 2, values);
	m_text = name;
	UpdateValue();
}

void CB::OnUpdateValue()
{
}

void CB::OnChangeSize()
{
	m_bmp_enabled.m_w = m_bmp_enabled.m_h = m_bmp_disabled.m_w = m_bmp_disabled.m_h = m_bmp_enabled_pressed.m_w = m_bmp_enabled_pressed.m_h = m_box_size;
	m_bmp_horizontal_orientation = - (m_window_rect.Height() - m_box_size) / 2;
}

int CB::m_box_size = 16;

void CB::Draw(CDC* dc)
{
	BmpButton::Draw(dc);

	// рисуем галку
	int w = m_window_rect.Width(), h = m_window_rect.Height();
	int x, y;	
	if (m_bmp_horizontal_orientation < 0)
		x = -m_bmp_horizontal_orientation;
	else
	if (m_bmp_horizontal_orientation > 0)
		x = w - m_bmp_horizontal_orientation - CB::m_box_size;
	else
		x = (w - CB::m_box_size)/2;
	if (m_bmp_vertical_orientation < 0)
		y = -m_bmp_vertical_orientation;
	else
	if (m_bmp_vertical_orientation > 0)
		y = h - m_bmp_vertical_orientation - CB::m_box_size;
	else
		y = (h - CB::m_box_size)/2;
	
	int dd = (m_pressed && !m_bmp_enabled_pressed.m_loaded) ? 1 : 0;
	
	int xx = x + dd, yy = y + dd, sz = CB::m_box_size;
	dc->FillSolidRect(xx, yy, sz, sz, 0x000000);
	sz-=2;
	xx++;
	yy++;
	dc->FillSolidRect(xx, yy, sz, sz, 0xFFFFFF);
	
	if (*m_value || m_pressed)
	{
		CPen pen(PS_SOLID, 2, m_pressed ? COLORREF(0x888888) : COLORREF(0x000000)), *oldpen;
		oldpen = dc->SelectObject(&pen);
		dc->MoveTo(xx + 2, yy + sz * 0.5);
		dc->LineTo(xx + sz * 0.5, yy + sz - 2);
		dc->LineTo(xx + sz - 2, yy + 2);
		dc->SelectObject(&oldpen);
	}
}

////////////////////////////////////////////////////////////////////////////////
// end