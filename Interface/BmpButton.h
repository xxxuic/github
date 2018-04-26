// uic 29.10.2009

#pragma once

#include "InterfaceObject.h"
#include "BMP.h"

struct BmpButton;
typedef void (*BmpButtonFunction)(BmpButton*);

struct BmpButton : InterfaceObject
{
	static CFont* m_BmpButton_font;
	virtual bool OwnSelectFont(CDC*dc);

	BmpButton();
	virtual ~BmpButton();
	void Create(CString name, BmpButtonFunction action, CWnd* parent);
	void Create(BmpButtonFunction action, UINT bmp_enable_id, UINT bmp_disable_id, CWnd* parent, UINT id = -1, int x = 0, int y = 0, int w = 10, int h = 10, DWORD add_ws = 0/*WS_VISIBLE*/);

	virtual void OnMouseDown();
	virtual void OnMouseUp();
	virtual void OnMouseMove();

	void Enable(bool ena) { m_enabled = ena; if (!ena && m_pressed && m_clicked) { m_pressed = false; m_clicked = false; } Invalidate(); }

	bool m_unpress_if_mouse_leave, m_draw_bmp, m_draw_text, m_draw_only_enabled_bmp;
	int m_id, m_bmp_horizontal_orientation, m_bmp_vertical_orientation, m_text_horizontal_orientation, m_text_vertical_orientation;
	BMP_Bitmap m_bmp_enabled, m_bmp_disabled, m_bmp_enabled_pressed;
	CString m_text;

	bool m_enabled, m_pressed, m_clicked, m_draw_fone, m_draw_borders, m_continuous, m_thread_is_running;
	HANDLE m_unpress_event;

	friend UINT ContinuousThreadProcedure(LPVOID param);

	DWORD m_color_norm, m_color_light, m_color_shadow;

	static BmpButtonFunction m_s_dbg_before_action_function;

protected:
	virtual void Draw(CDC* dc);

	BmpButtonFunction m_action;
};

////////////////////////////////////////////////////////////////////////////////
// end