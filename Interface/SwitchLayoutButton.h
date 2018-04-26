// uic 2.11.2010

#pragma once

#include "LAYOUT.h"
#include "BmpButton.h"
#include "./CfgFiles.h"

struct SwitchLayoutButton;

typedef void (*SwitchLayoutButtonFunction)(SwitchLayoutButton*);

struct SwitchLayoutButton: BmpButton
{
	SwitchLayoutButton();

	static CFont *m_vertical_font1, *m_vertical_font2, *m_horizontal_font;

	void Create(CWnd* parent, CString name, LAYOUT_Direction hide_border_direction, bool hide_border, UINT id = -1, SwitchLayoutButtonFunction func = NULL, DWORD wsstyle = 0/*WS_VISIBLE*/);

	virtual void OnMouseUp();

	LAYOUT_Element *m_layout;
	LAYOUT_Direction m_hide_border_direction;
	bool m_hide_border, m_pale_when_closed;
	SwitchLayoutButtonFunction m_function;

	static CCfgFiles* m_cfg_file;
	bool m_save_in_common_way;
	CString GetCommonParamName();

	void LoadOpenState(bool apply);

protected:
	virtual void Draw(CDC* dc);
};

////////////////////////////////////////////////////////////////////////////////
// end