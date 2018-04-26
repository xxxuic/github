// uic 1.03.2010

#pragma once

#include "InterfaceObject.h"

struct ScrollBar : InterfaceObject
{
	ScrollBar(bool horisontal, int button_step = 1);

	void SetTotalElements(int total, bool redraw = false);
	int GetTotalElements() { return m_total_elements; }
	void SetVisibleElements(int visible, bool redraw = false);
	int GetVisibleElements() { return m_visible_elements; }
	void SetFirstVisibleElement(int first, bool redraw = false);
	int GetFirstVisibleElement() { return m_first_visible_element; }
	void SetReaction(HWND hwnd, UINT msg) { m_reaction_hwnd = hwnd; m_reaction_msg = msg; }
	void SetReaction(CWnd *wnd, UINT msg) { m_reaction_wnd = wnd; m_reaction_msg = msg; }
	void GetReaction(HWND &hwnd, UINT &msg) { hwnd = m_reaction_hwnd; msg = m_reaction_msg; }

	void SetButtonStep(int button_step) { m_button_step = button_step; }
	int GetButtonStep() { return m_button_step; }

	void SetColors(DWORD col1, DWORD col2, DWORD col3, DWORD col4, DWORD col5)
	{
		m_color[0] = col1; // —¬≈“À¿ﬂ Œ ¿Õ“Œ¬ ¿  ÕŒœŒ 
		m_color[1] = col2; // Õ¿Õ¿∆¿“€≈  ÕŒœ » » —À¿…ƒ≈–
		m_color[2] = col3; // Õ≈Õ¿∆¿“Œ≈ œ–Œ—“–¿Õ—“¬Œ
		m_color[3] = col4; // Õ¿∆¿“€≈  ÕŒœ » » —À¿…ƒ≈–, œ–Œ—“–¿Õ—“¬Œ » Œ¡Ÿ»… ‘ŒÕ
		m_color[4] = col5; // ÷¬≈“ —“–≈ÀŒ◊≈ 
		m_pen.DeleteObject();
		m_pen.CreatePen(PS_SOLID, 1, m_color[4]);
		m_brush.DeleteObject();
		m_brush.CreateSolidBrush(m_color[4]);
	}

	DWORD GetColor(int index)
	{
		if (index < 0 && index > 4) return 0x000000;
		return m_color[index];
	}

	int GetLength();
	int GetWidth();

protected:
	DWORD m_color[5];
	CPen m_pen;
	CBrush m_brush;
	bool m_horisontal;
	int m_button_step;

	enum Activated
	{
		NotActivated = -1,
		LowButton,
		LowSpace,
		Slider,
		HighSpace,
		HighButton
	};
	Activated m_activated;

	CRect m_low_button_rect, m_low_space_rect, m_slider_rect, m_high_space_rect, m_high_button_rect;

	void OnMouseDown();
	void OnMouseUp();
	void OnMouseMove();
	void OnMouseWheel();
	virtual void OnTimer(UINT timer_id);

	virtual void Draw(CDC* dc);
	void DrawRect(CRect rect, bool activated, CDC*dc);
	void DrawArrows(CDC*dc);

	virtual void OnChangeSize();
	virtual void OnCreate();

	void OnSetParams(bool redraw);
	void DoReaction();

	void CalculateRects();
	void GetRect(int pos, int len, CRect &rect);
	void PosToXY(int along, int across, LONG &x, LONG &y);
	int PointToPos(CPoint point);

	int m_total_elements, m_visible_elements, m_first_visible_element;
	HWND m_reaction_hwnd;
	CWnd* m_reaction_wnd;
	UINT m_reaction_msg;
	int m_d_pos;
};

typedef ScrollBar* ScrollBarPtr;

////////////////////////////////////////////////////////////////////////////////
// end