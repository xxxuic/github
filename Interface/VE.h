#pragma once

#include "BmpButton.h"
#include "CLR.h"
#include "DlgEditText.h"

////////////////////////////////////////////////////////////////////////////////
// VE_Button
////////////////////////////////////////////////////////////////////////////////
struct VE_Base;

struct VE_Button: BmpButton
{
	VE_Button();

	void Create(int sgn, VE_Base* ve);
	int m_sgn;
	VE_Base *m_ve;
};

////////////////////////////////////////////////////////////////////////////////
// VE_Base
////////////////////////////////////////////////////////////////////////////////
struct VE_Base: InterfaceObject
{
	VE_Base():
		m_enabled(true), m_dlg_result(IDCANCEL), m_name_width_in_percent(50.0f),
		m_variants_num(0), m_name_x_align(-1), m_gap(2), m_has_buttons(true), m_buttons_mode(1)
	{
	}

	virtual void PlusMinusAction(int dir) {}

	int m_name_x_align;
	bool m_has_buttons;
	int m_buttons_mode; // 0 - "+ / -", 1 - arrows

protected:
	VE_Button m_plus_button, m_minus_button;
	int m_dlg_result, m_gap;
	bool m_enabled;

	CRect m_value_window_rect;

	int m_variants_num;
	float m_name_width_in_percent;
	
	CString m_name, m_io_value_format, m_idle_value_format, m_value_text;
};

////////////////////////////////////////////////////////////////////////////////
// VE<T>
////////////////////////////////////////////////////////////////////////////////
template <typename T> struct VE: VE_Base
{
	VE<T>(): VE_Base(),
		m_value(NULL), m_min(0), m_max(0), m_delta(0), m_variants(NULL)
	{
	}

	~VE<T>()
	{
		if (m_variants)
			delete[] m_variants;
	}

	virtual void Draw(CDC* dc)
	{
		CRect rect = GetWindowRect();
		CRgn total_rgn;
		total_rgn.CreateRectRgnIndirect(&rect);
		dc->FillSolidRect(0, 0, rect.Width(), rect.Height(), m_bk_color);
		CSize sz = dc->GetTextExtent(m_name);
		int x = m_gap, name_width;
		name_width = 0.01 * m_name_width_in_percent * (rect.Width() - 2 * m_gap);
		switch (m_name_x_align)
		{
			case 0: x += (name_width - sz.cx) / 2; break;
			case +1: x += name_width - sz.cx; break;
		}

		CRect name_rect;
		name_rect.left = m_gap;
		name_rect.top = m_gap;
		name_rect.right = name_rect.left + name_width;
		name_rect.bottom = rect.Height() - m_gap;
		dc->SelectClipRgn(&total_rgn);
		dc->IntersectClipRect(&name_rect);
		dc->SetTextColor(0x000000);
		dc->TextOut(x, (rect.Height() - sz.cy) / 2, m_name);

		dc->SelectClipRgn(&total_rgn);
		dc->IntersectClipRect(&m_value_window_rect);
		dc->FillSolidRect(m_value_window_rect.left, m_value_window_rect.top, m_value_window_rect.Width(), m_value_window_rect.Height(), 0x666666);
		dc->FillSolidRect(m_value_window_rect.left + 1, m_value_window_rect.top + 1, m_value_window_rect.Width() - 2, m_value_window_rect.Height() - 2, m_enabled ? 0xFFFFFF : m_bk_color);
		sz = dc->GetTextExtent(m_value_text);
		x = m_value_window_rect.left + (m_value_window_rect.Width() - sz.cx) / 2;
		dc->SetTextColor(m_enabled ? 0x000000 : 0x888888);
		dc->TextOut(x, (rect.Height() - sz.cy) / 2, m_value_text);

		dc->SelectClipRgn(&total_rgn);
		if (m_has_buttons)
		{
			m_plus_button.Invalidate();
			m_minus_button.Invalidate();
		}
	}

	virtual void OnCreate()
	{
		m_minus_button.Create(-1, this);
		m_plus_button.Create(+1, this);
		m_bk_color = CLR_GetSysColor(COLOR_3DFACE);
		SetMessageHandler(WM_LBUTTONDOWN, (InterfaceObject_OwnFunction)(&VE<T>::OnButtonDown));
	}

	virtual void OnChangeSize()
	{
		CRect rect = GetWindowRect();
		int name_width = 0.01 * m_name_width_in_percent * (rect.Width() - 2 * m_gap),
			value_field_w = rect.Width() - (m_has_buttons ? 4 : 3) * m_gap - name_width,
			value_field_x = rect.Width() - 1 * m_gap - value_field_w;
		if (m_has_buttons)
			value_field_w -= rect.Height();
		m_value_window_rect.left = value_field_x;
		m_value_window_rect.right = value_field_x + value_field_w;
		m_value_window_rect.top = m_gap;
		m_value_window_rect.bottom = rect.Height() - m_gap;
		if (m_has_buttons)
		{
			int x = m_value_window_rect.right + m_gap, w = rect.Width() - (m_value_window_rect.right + m_gap) - m_gap, h = (rect.Height() + 1)/2 - m_gap;
			m_plus_button.ChangePosition(x, m_gap, w, h);
			m_minus_button.ChangePosition(x, m_gap + h - 1, w, rect.Height() - h + 1 - 2*m_gap);
		}
	}

	void UpdateButtonsEnabled()
	{
		bool ena = m_enabled && *m_value > m_min;
		m_minus_button.Enable(ena);
		if (!ena)
			m_minus_button.m_clicked = false;
		m_plus_button.Enable(m_enabled && *m_value < m_max);
	}

	void Enable(bool ena)
	{
		m_enabled = ena;
		UpdateButtonsEnabled();
		Invalidate();
	}

	virtual void OnButtonDown()
	{
		if (!m_enabled)
			return;

		CWnd *own = GetWnd();
		CRect rect = m_value_window_rect;
		if (!PtInRect(&rect, m_winmessage_point))
			return;

		own->ClientToScreen(&rect);
		CString txt, scan_format = m_io_value_format;

		// правим формат, убираем точность из форматов типа "%.6f" >>
		int idx = scan_format.Find('.');
		if (idx != -1)
		{
			scan_format.Delete(idx);
			while (isdigit(scan_format.GetAt(idx)))
			{
				scan_format.Delete(idx);
			}
		}
		// правим формат, убираем точность из форматов типа "%.6f" <<

		txt.Format(m_io_value_format, *m_value);
		CFont *old = DlgEditText_font;
		DlgEditText_font = m_font;
		m_dlg_result = DlgEditText_Show(&txt, false, rect.left, rect.top, rect.Width(), rect.Height(), own, ES_CENTER | ES_AUTOHSCROLL);
		DlgEditText_font = old;
		if (m_dlg_result != IDCANCEL)
		{
			T new_val;
			txt.Replace(',', '.');
			if (_stscanf(txt, scan_format, &new_val) == 1)
			{
				T delta = new_val - *m_value;
				if (delta != 0)
					ChangeValue(delta);
			}
		}
	}
	
	virtual void UpdateValue(bool reaction = true) // надо вызвать после изменения величины извне
	{
		T prev_val = *m_value;
		if (*m_value < m_min)
			*m_value = m_min;
		if (*m_value > m_max)
			*m_value = m_max;
		if (reaction && prev_val != *m_value && m_on_set_function)
			m_on_set_function(this);
			
		UpdateButtonsEnabled();

		CString result;
		if (m_text_out_function)
			result = m_text_out_function(m_value);
		else
			result.Format(m_idle_value_format, *m_value);

		int len = result.GetLength(), i = len - 1;
		if (result.Find('.') != -1)
		{
			while (result.GetAt(i) == '0') i--;
			if (i < len - 1)
			{
				if (result.GetAt(i) == '.') i--;
					result = result.Left(i+1);
			}
		}

		if (result == "-0")
			result = "0";

		m_value_text = result;
	}

	void Setup(CString name, CString io_format, CString idle_format, T* val, T minimal, T maximal, T delta, float name_width_in_percent = 50.0f, InterfaceObject_GlobalFunctionWithParam on_set_function = NULL)
	{
		m_name = name;
		m_io_value_format = io_format;
		m_idle_value_format = idle_format;
		m_name_width_in_percent = name_width_in_percent;

		m_value = val;
		m_min = minimal;
		m_max = maximal;
		m_delta = delta;
		m_on_set_function = on_set_function;
		UpdateValue();
	}

	void ChangeValue(T delta)
	{
		*m_value += delta;
		UpdateValue();
		if (m_on_set_function)
			m_on_set_function(this);
		Invalidate(FALSE);
	}

	void SetValPtr(T* val)
	{
		m_value = val;
		UpdateValue();
	}

	void ChangeMinMax(T min, T max)
	{
		m_min = min;
		m_max = max;
		UpdateValue();
	}

	virtual void PlusMinusAction(int dir)
	{
		if (m_variants)
		{
			// определяем ближайший шаг в этом направлении и устанавливаем его
			int i;
			for (i = 0; i < m_variants_num; i++)
				if (m_variants[i] > *m_value)
					break;
			if (dir == -1)
			{
				i--;
				if (i >= 0 && abs(m_variants[i] - *m_value) < 1e-9)
					i--;
			}
			if (i < 0)
				i = 0;
			if (i >= m_variants_num)
				i = m_variants_num - 1;
			ChangeValue(m_variants[i] - *m_value);
		}
		else
			ChangeValue(m_delta * dir);
	}

	virtual void SetVariants(int variants_num, T* variants)
	{
		if (m_variants)
			delete[] m_variants;
		if (variants_num < 0)
		{
			m_variants = NULL;
			m_variants_num = 0;
		}
		else
		{
			m_variants_num = variants_num;
			m_variants = new T[m_variants_num];
			memcpy(m_variants, variants, m_variants_num * sizeof(T));
		}
	}

	static bool m_allow_to_show_dlg_by_set_focus;
	InterfaceObject_GlobalFunctionWithParam m_on_set_function;
	CString (*m_text_out_function)(void*);

protected:
	T *m_value, m_min, m_max, m_delta, *m_variants;
};

struct VE_String: InterfaceObject
{
	CString m_name, *m_value;
	float m_name_width_in_percent;
	bool m_enabled;
	CRect m_value_window_rect;
	int m_gap, m_name_x_align, m_dlg_result;
	
	VE_String():
		m_enabled(true), m_dlg_result(IDCANCEL), m_name_width_in_percent(50.0f), m_gap(2)
	{
	}
	
	virtual void OnCreate()
	{
		m_bk_color = CLR_GetSysColor(COLOR_3DFACE);
		SetMessageHandler(WM_LBUTTONDOWN, (InterfaceObject_OwnFunction)(&VE_String::OnButtonDown));
	}

	void Setup(CString name, CString* edit_value, float name_width_in_percent = 50.0f, InterfaceObject_GlobalFunctionWithParam on_set_function = NULL)
	{
		m_name = name;
		m_name_width_in_percent = name_width_in_percent;
		m_value = edit_value;
		m_on_set_function = on_set_function;
	}

	virtual void Draw(CDC* dc)
	{
		CRect rect = GetWindowRect();
		CRgn total_rgn;
		total_rgn.CreateRectRgnIndirect(&rect);
		dc->FillSolidRect(0, 0, rect.Width(), rect.Height(), m_bk_color);
		CSize sz = dc->GetTextExtent(m_name);
		int x = m_gap, name_width;
		name_width = 0.01 * m_name_width_in_percent * (rect.Width() - 2 * m_gap);
		switch (m_name_x_align)
		{
			case 0: x += (name_width - sz.cx) / 2; break;
			case +1: x += name_width - sz.cx; break;
		}

		CRect name_rect;
		name_rect.left = m_gap;
		name_rect.top = m_gap;
		name_rect.right = name_rect.left + name_width;
		name_rect.bottom = rect.Height() - m_gap;
		dc->SelectClipRgn(&total_rgn);
		dc->IntersectClipRect(&name_rect);
		dc->SetTextColor(0x000000);
		dc->TextOut(x, (rect.Height() - sz.cy) / 2, m_name);

		dc->SelectClipRgn(&total_rgn);
		dc->IntersectClipRect(&m_value_window_rect);
		dc->FillSolidRect(m_value_window_rect.left, m_value_window_rect.top, m_value_window_rect.Width(), m_value_window_rect.Height(), 0x666666);
		dc->FillSolidRect(m_value_window_rect.left + 1, m_value_window_rect.top + 1, m_value_window_rect.Width() - 2, m_value_window_rect.Height() - 2, m_enabled ? 0xFFFFFF : m_bk_color);
		CString value_str(*m_value);
		sz = dc->GetTextExtent(value_str);
		x = m_value_window_rect.left + (m_value_window_rect.Width() - sz.cx) / 2;
		dc->SetTextColor(m_enabled ? 0x000000 : 0x888888);
		dc->TextOut(x, (rect.Height() - sz.cy) / 2, value_str);

		dc->SelectClipRgn(&total_rgn);
	}

	virtual void OnChangeSize()
	{
		CRect rect = GetWindowRect();
		int name_width = 0.01 * m_name_width_in_percent * (rect.Width() - 2 * m_gap),
			value_field_w = rect.Width() - 3 * m_gap - name_width,
			value_field_x = rect.Width() - 1 * m_gap - value_field_w;

		m_value_window_rect.left = value_field_x;
		m_value_window_rect.right = value_field_x + value_field_w;
		m_value_window_rect.top = m_gap;
		m_value_window_rect.bottom = rect.Height() - m_gap;
	}

	virtual void OnButtonDown()
	{
		if (!m_enabled)
			return;

		CWnd *own = GetWnd();
		CRect rect = m_value_window_rect;
		if (!PtInRect(&rect, m_winmessage_point))
			return;

		own->ClientToScreen(&rect);
		CStringA txt(*m_value);

		CFont *old = DlgEditText_font;
		DlgEditText_font = m_font;
		m_dlg_result = DlgEditText_Show(&txt, false, rect.left, rect.top, rect.Width(), rect.Height(), own, ES_CENTER | ES_AUTOHSCROLL);
		DlgEditText_font = old;
		if (m_dlg_result == IDCANCEL)
			return;

		*m_value = CString(txt);
		m_on_set_function(this);
		Invalidate(FALSE);
	}

	void Enable(bool ena)
	{
		m_enabled = ena;
		Invalidate(FALSE);
	}

	InterfaceObject_GlobalFunctionWithParam m_on_set_function;
};

////////////////////////////////////////////////////////////////////////////////
// end