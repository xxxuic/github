// uic 2010.11.3

#pragma once

#include "LAYOUT.h"
#include "InterfaceForm.h"
#include "SwitchLayoutButton.h"

#include "CLR.h"

extern int LayoutedGroupBox_header_h;

struct MyBoxWnd : CStatic
{
	void OnPaint();
	DECLARE_MESSAGE_MAP()
};

struct LayoutedGroupBox
{
	LayoutedGroupBox(CString name = "");
	void Create(CWnd* parent, LAYOUT_Direction open_direction = LAYOUT_direction_bottom, bool draw_layout_borders = true);

	void StartGroupBox(InterfaceForm* form, int w, int h, int left_gap = 0, int right_gap = 0, int top_gap = 0, int bottom_gap = 0, int inner_gap = -1, LAYOUT_Orientation orientation = LAYOUT_useless);
	void FinishGroupBox();

	SwitchLayoutButton m_header_button;
	LAYOUT_Direction m_open_direction;
	InterfaceForm* m_form;
	CString m_name;
	bool m_draw_layout_borders, m_can_be_closed;

private:
	MyBoxWnd m_box_window;
	void CreateButtonLayout(int left_gap, int right_gap, int top_gap, int bottom_gap);
	int m_left_gap, m_right_gap, m_top_gap, m_bottom_gap, m_inner_gap;
};

////////////////////////////////////////////////////////////////////////////////
// end