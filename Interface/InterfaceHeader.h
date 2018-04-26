// uic 15.12.2009

#pragma once

#include "InterfaceObject.h"
#include "CLR.h"
#include "BMP.h"
#include "BMPButton.h"

typedef void (*InterfaceHeader_DrawFunction)(CDC*);

struct InterfaceHeader: InterfaceObject
{
	static CFont *m_small_font, *m_big_font;

	InterfaceHeader();

	void CaptureParent();
	void MoveParent();
	void ReleaseParent();

	virtual void OnCreate();
	virtual void OnChangeSize();

	void SetActive(bool active);

	void GradientFill(CDC* dc, int x, int y, int w, int h, DWORD color_top, DWORD color_bottom);
	void DrawText(CString text, bool align_left, CDC* dc, DWORD text_color, DWORD text_shadow_color, CRect& header_rect);
	virtual void Draw(CDC* dc);

	BmpButtonFunction m_close_function;
	BmpButton m_close_button, m_minimize_button;

	InterfaceHeader_DrawFunction m_draw_function;
	
	bool m_active, m_main_text_aligned_to_left, m_draw_text_shadow,
		m_has_close_button, m_has_minimize_button, m_move_parent;
	CString m_main_text, m_additional_text;
	int m_shadow_distance;
	BMP_Bitmap m_lois_bmp;

protected:
	bool m_parent_is_moving;
	CPoint m_click_point;
};

////////////////////////////////////////////////////////////////////////////////
// end