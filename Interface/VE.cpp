#include <stdafx.h>
#include "VE.h"

////////////////////////////////////////////////////////////////////////////////
// VE_Button
////////////////////////////////////////////////////////////////////////////////
static void PlusMinusAction(BmpButton *btn)
{
	VE_Button *ve_btn = (VE_Button*) btn;
	ve_btn->m_ve->PlusMinusAction(ve_btn->m_sgn);
}

static void DrawButtonsFunction(CDC* dc, InterfaceObject *obj)
{
	VE_Button *btn = (VE_Button*)obj;
	if (btn->m_ve->m_buttons_mode == 1)
	{
		CRect rect = btn->GetWindowRect();
		int w = rect.Width(), h = rect.Height();
		DWORD clr = btn->m_enabled ? 0x000000 : 0xAAAAAA;
		int arr_sz = min(11, w - 6), arr_y = h / 2 + btn->m_sgn * arr_sz / 4, i, curr_w = arr_sz;
		for (i = 0; i < arr_sz; i+=2, arr_y -= btn->m_sgn, curr_w -= 2)
			dc->FillSolidRect((w - curr_w)/2, arr_y, curr_w, 1, clr);
	}
}

VE_Button::VE_Button():
	m_sgn(0), m_ve(NULL)
{
}

void VE_Button::Create(int sgn, VE_Base* ve)
{
	m_sgn = sgn;
	m_ve = ve;
	BmpButton::Create(::PlusMinusAction, -1, -1, ve->GetWnd(), -1, 0, 0, 10, 10, WS_VISIBLE);
	m_continuous = true;
	if (m_ve->m_buttons_mode == 0)
		m_text = sgn == -1 ? '-' : '+';
	else
		m_after_draw_function = DrawButtonsFunction;
}

////////////////////////////////////////////////////////////////////////////////
// end