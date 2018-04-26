// uic 10.03.2010

#pragma once

#include "InterfaceObject.h"

struct CS: InterfaceObject
{
	CS();
	static DWORD m_colors[3];

	static CFont* m_CS_font;
	virtual bool OwnSelectFont(CDC*dc);

	DWORD m_own_colors[3];
	bool m_use_own_colors;

	CString m_text;
	bool m_bottom_less;
	int m_horizontal_text_alignment, m_vertical_text_alignment;
	virtual void Draw(CDC*dc);
};

////////////////////////////////////////////////////////////////////////////////
// end