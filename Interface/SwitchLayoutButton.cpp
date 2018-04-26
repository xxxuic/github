// uic 2.11.2010

#pragma once

#include <stdafx.h>

#include "SwitchLayoutButton.h"
#include "CLR.h"

#ifndef SUPPRESS_OLD_VE_INCLUDE
#include "ValueEditor.h"
#endif

CFont *SwitchLayoutButton::m_vertical_font1 = NULL, *SwitchLayoutButton::m_vertical_font2 = NULL, *SwitchLayoutButton::m_horizontal_font = NULL;
CCfgFiles* SwitchLayoutButton::m_cfg_file = NULL;

////////////////////////////////////////////////////////////////////////////////
SwitchLayoutButton::SwitchLayoutButton():
	m_hide_border_direction(LAYOUT_direction_bottom), m_hide_border(false), m_layout(NULL), m_pale_when_closed(false), m_save_in_common_way(true)
{
}

CString SwitchLayoutButton::GetCommonParamName()
{
	CString text = m_text;
	text.Replace(_T(" "), _T(""));
	text = _T("SLB_") + text;
	return text;
}

void SwitchLayoutButton::LoadOpenState(bool apply)
{
	bool opened = true;
	if (m_cfg_file != NULL && m_save_in_common_way)
		m_cfg_file->ReadPar(GetCommonParamName(), opened, true);
	if (m_layout)
		m_layout->Show(opened, apply, false);
}

void SwitchLayoutButton::OnMouseUp()
{
	if (!m_enabled)
		return;

#ifndef SUPPRESS_OLD_VE_INCLUDE
	bool prev_val = ValueEditorBase::m_allow_to_show_dlg_by_set_focus;
	if (prev_val)
		ValueEditorBase::m_allow_to_show_dlg_by_set_focus = false; // <<<
#endif

	BmpButton::OnMouseUp();

	if (m_function)
		m_function(this);
	else
	if (m_layout)
		m_layout->Show(m_layout->IsHidden());

	if (m_cfg_file != NULL && m_save_in_common_way)
		m_cfg_file->WritePar(GetCommonParamName(), !m_layout->IsHidden());

#ifndef SUPPRESS_OLD_VE_INCLUDE
	if (prev_val)
		ValueEditorBase::m_allow_to_show_dlg_by_set_focus = true; // <<<
#endif
}

void SwitchLayoutButton::Create(CWnd* parent, CString name, LAYOUT_Direction hide_border_direction, bool hide_border, UINT id, SwitchLayoutButtonFunction func, DWORD wsstyle)
{
	BmpButton::Create(m_action, -1, -1, parent, id, 0, 0, 10, 10, WS_TABSTOP | wsstyle);
	m_hide_border = hide_border;
	m_hide_border_direction = hide_border_direction;
	m_text = name;
	m_function = func;
}

void SwitchLayoutButton::Draw(CDC* dc)
{
	DWORD col_light = CLR_GetSysColor(COLOR_3DFACE), col_dark = CLR_GetSysColor(COLOR_3DDKSHADOW);
	if (m_pale_when_closed && m_layout->IsHidden())
	{
		DWORD fone = CLR_GetSysColor(COLOR_3DFACE);
		DWORD tmp = CLR_MID_COLOR(col_light, fone);
		col_light = CLR_MID_COLOR(tmp, fone);
		tmp = CLR_MID_COLOR(col_dark, fone);
		col_dark = CLR_MID_COLOR(tmp, fone);
	}
	DWORD bk_col = CLR_MID_COLOR(col_light, col_dark);
	DWORD col = CLR_MID_COLOR(0xFFFFFF, CLR_GetSysColor(COLOR_3DFACE));

	int w = m_window_rect.Width(), h = m_window_rect.Height();
	int d = 0, ww = w - 2 * d, hh = h - 2 * d;

	CGdiObject* prev_brush = dc->SelectStockObject(NULL_BRUSH);
	
	dc->SetTextColor(col);
	dc->SetBkMode(TRANSPARENT);

	CFont *prev_font = NULL;
	if (m_hide_border_direction >= LAYOUT_direction_left)
	{
		CLR_GradientFill(dc, d, d, ww/2, hh, CLR_MID_COLOR(col_light, bk_col), bk_col, false);
		CLR_GradientFill(dc, d + ww/2, d, ww - (ww - ww/2), hh, col_dark, CLR_MID_COLOR(col_dark, bk_col), false);
		prev_font = dc->SelectObject((m_hide_border_direction == LAYOUT_direction_right ? m_vertical_font1 : m_vertical_font2));
		dc->TextOut((m_hide_border_direction == LAYOUT_direction_right) ? -1 : w + 1, (m_hide_border_direction == LAYOUT_direction_right) ? h - 5 : 5, m_text);
	}
	else
	{
		CLR_GradientFill(dc, d, d, ww, hh/2, CLR_MID_COLOR(col_light, bk_col), bk_col, true);
		CLR_GradientFill(dc, d, d + hh/2, ww, hh - (hh - hh/2), col_dark, CLR_MID_COLOR(col_dark, bk_col), true);
		CFont* fnt = SwitchLayoutButton::m_horizontal_font;
		if (fnt == NULL)
			fnt = InterfaceObject::m_font;
		prev_font = dc->SelectObject(fnt);
		dc->TextOut(5, -1, m_text);
	}

	bool allways = m_layout->IsHidden() || !m_hide_border;
		
	//dc->RoundRect(0, 0, w, h, 2, 2);
	if (allways || m_hide_border_direction != LAYOUT_direction_top)
		dc->FillSolidRect(1, 0, w - 2, 1, 0x000000);
	if (allways || m_hide_border_direction != LAYOUT_direction_bottom)
		dc->FillSolidRect(1, h - 1, w - 2, 1, 0x000000);
	if (allways || m_hide_border_direction != LAYOUT_direction_left)
		dc->FillSolidRect(0, 1, 1, h - 2, 0x000000);
	if (allways || m_hide_border_direction != LAYOUT_direction_right)
		dc->FillSolidRect(w - 1, 1, 1, h - 2, 0x000000);

	allways = m_layout->IsHidden();

	col = col_light;
	if (!allways && (m_hide_border_direction == LAYOUT_direction_top || m_hide_border_direction == LAYOUT_direction_left))
		col = 0x000000;
	dc->FillSolidRect(0, 0, 1, 1, col);

	col = col_light;
	if (!allways && (m_hide_border_direction == LAYOUT_direction_top || m_hide_border_direction == LAYOUT_direction_right))
		col = 0x000000;
	dc->FillSolidRect(w - 1, 0, 1, 1, col);

	col = col_light;
	if (!allways && (m_hide_border_direction == LAYOUT_direction_bottom || m_hide_border_direction == LAYOUT_direction_left))
		col = 0x000000;
	dc->FillSolidRect(0, h - 1, 1, 1, col);

	col = col_light;
	if (!allways && (m_hide_border_direction == LAYOUT_direction_bottom || m_hide_border_direction == LAYOUT_direction_right))
		col = 0x000000;
	dc->FillSolidRect(w - 1, h - 1, 1, 1, col);

	dc->SelectObject(prev_font);
	dc->SelectObject(prev_brush);
}

////////////////////////////////////////////////////////////////////////////////
// end