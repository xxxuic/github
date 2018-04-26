// uic 22.10.2008

#include <StdAfx.h>
#include "InterfaceObject.h"
#include <MutexWrap.h>

////////////////////////////////////////////////////////////////////////////////
// InterfaceObject_Window
////////////////////////////////////////////////////////////////////////////////
struct InterfaceObject_Window: public CWnd
{
public:
	InterfaceObject_Window();
	~InterfaceObject_Window();

	bool MyCreate(int x, int y, int w, int h, CWnd *parent, bool child, bool with_brush, DWORD additional_class_style, DWORD window_style, UINT id);

	InterfaceObject* m_parent_object;
	CWnd *m_parent_window, *m_prev_focus_window;
	bool m_mouse_in;
	HBRUSH m_bk_brush;

	CString m_name_for_debug;
	void CheckSetDowns(UINT flags);

	void MouseIn();
	void MouseOut();

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR timer_id);
	
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT flags, short delta, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

	afx_msg void OnCaptureChanged(CWnd* new_wnd);
	afx_msg void OnKillFocus(CWnd* new_wnd);

	afx_msg void OnKeyDown(UINT nchar, UINT repeat, UINT flags);
	afx_msg void OnKeyUp(UINT nchar, UINT repeat, UINT flags);

	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(InterfaceObject_Window, CWnd)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_MBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_MBUTTONDBLCLK()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
	ON_WM_CAPTURECHANGED()
	ON_WM_KILLFOCUS()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
END_MESSAGE_MAP()

InterfaceObject_Window::InterfaceObject_Window():
	m_parent_window(NULL), m_parent_object(NULL), m_bk_brush(NULL)
{
	m_name_for_debug = "не задано";
}

InterfaceObject_Window::~InterfaceObject_Window()
{
	if (m_parent_object)
		m_parent_object->OnDestroyWindow();
	if (m_bk_brush)
		DeleteObject(m_bk_brush);
}

bool InterfaceObject_Window::MyCreate(int x, int y, int w, int h, CWnd *parent, bool child, bool with_brush, DWORD additional_class_style, DWORD window_style, UINT id)
{
	m_mouse_in = false;
	m_parent_window = parent;
	m_prev_focus_window = NULL;
	if (m_bk_brush)
		DeleteObject(m_bk_brush);
	m_bk_brush = (with_brush) ? CreateSolidBrush(m_parent_object->m_bk_color) : NULL;

	// регистрация класса окна >>
	CString class_name;
	bool err = true;
	int err_counter = 0;
	while (err && err_counter < 2)
	{
		try
		{
			class_name = ::AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW | additional_class_style, ::LoadCursor(NULL, IDC_ARROW), m_bk_brush/*если NULL, то не стирается при Invalidate(TRUE)*/, NULL);
			err = false;
		}
		catch(...)
		{
			additional_class_style = 0;
			err_counter++;
		}
	}
	if (err)
		return false;
	// регистрация класса окна <<

	m_parent_object->m_window_rect.left = 0;
	m_parent_object->m_window_rect.top = 0;
	m_parent_object->m_window_rect.right = w - (m_parent_object->m_has_borders?2:0);
	m_parent_object->m_window_rect.bottom = h - (m_parent_object->m_has_borders?2:0);
	DWORD flag = window_style;
	if (m_parent_object->m_has_borders)
		flag |= WS_BORDER;
	if (child)
		flag |= WS_CHILD;
	else
		flag |= WS_OVERLAPPED;
	CRect rrr(x, y, x + w, y + h);
	BOOL res = CreateEx(WS_EX_NOPARENTNOTIFY, class_name, NULL, flag, rrr, m_parent_window, id, NULL);
	return (res == TRUE);
}

void InterfaceObject_Window::MouseIn()
{
	if (m_mouse_in)
		return;
	m_mouse_in = true;
	SetCapture();
	if (m_parent_object->m_capture_focus)
		m_prev_focus_window = SetFocus();
	m_parent_object->OnMouseIn();
}

void InterfaceObject_Window::MouseOut()
{
	if (!m_mouse_in || m_parent_object->m_always_capture)
		return;

	if (m_parent_object->m_left_button_down)
	{
		m_parent_object->OnWinMessage(WM_LBUTTONUP, 0, m_parent_object->m_winmessage_point);
		m_parent_object->m_left_button_down = false;
	}
	if (m_parent_object->m_right_button_down)
	{
		m_parent_object->OnWinMessage(WM_RBUTTONUP, 0, m_parent_object->m_winmessage_point);
		m_parent_object->m_right_button_down = false;
	}
	if (m_parent_object->m_middle_button_down)
	{
		m_parent_object->OnWinMessage(WM_MBUTTONUP, 0, m_parent_object->m_winmessage_point);
		m_parent_object->m_middle_button_down = false;
	}
	m_mouse_in = false;
	if (GetCapture() == this)
		ReleaseCapture();
	if (m_parent_object->m_capture_focus)
		m_parent_window->SetFocus();

	m_parent_object->OnMouseOut();
}

void InterfaceObject_Window::OnPaint()
{
	CPaintDC dc(this);
	m_parent_object->PrivateDraw(&dc);
}

void InterfaceObject_Window::OnTimer(UINT_PTR timer_id)
{
	m_parent_object->OnTimer(timer_id);
}

void InterfaceObject_Window::OnLButtonDown(UINT flags, CPoint point)
{
	CheckSetDowns(flags);
	m_parent_object->m_left_button_down = true;
	m_parent_object->OnWinMessage(WM_LBUTTONDOWN, flags, point);
}

void InterfaceObject_Window::OnRButtonDown(UINT flags, CPoint point)
{
	CheckSetDowns(flags);
	m_parent_object->m_right_button_down = true;
	m_parent_object->OnWinMessage(WM_RBUTTONDOWN, flags, point);
}

void InterfaceObject_Window::OnMButtonDown(UINT flags, CPoint point)
{
	CheckSetDowns(flags);
	m_parent_object->m_middle_button_down = true;
	m_parent_object->OnWinMessage(WM_MBUTTONDOWN, flags, point);
}

void InterfaceObject_Window::OnLButtonUp(UINT flags, CPoint point)
{
	CheckSetDowns(flags);
	m_parent_object->m_left_button_down = false;
	m_parent_object->OnWinMessage(WM_LBUTTONUP, flags, point);

	// теперь на случай въезда в другой объект InterfaceObject сделаем проверку
	// если этого не делать, то при отпускании мышки в нём кепшын на него не переметнётся
	if (::IsWindow(m_hWnd))
	{
		ClientToScreen(&point);
		CWnd* global_parent = AfxGetMainWnd()->WindowFromPoint(point);
		if (global_parent)
		{
			global_parent->ScreenToClient(&point);
			global_parent->PostMessage(WM_MOUSEMOVE, 0, (point.y << 16) | point.x);
		}
	}
}

void InterfaceObject_Window::OnRButtonUp(UINT flags, CPoint point)
{
	CheckSetDowns(flags);
	m_parent_object->m_right_button_down = false;
	m_parent_object->OnWinMessage(WM_RBUTTONUP, flags, point);
}

void InterfaceObject_Window::OnMButtonUp(UINT flags, CPoint point)
{
	CheckSetDowns(flags);
	m_parent_object->m_middle_button_down = false;
	m_parent_object->OnWinMessage(WM_MBUTTONUP, flags, point);
}

void InterfaceObject_Window::OnLButtonDblClk(UINT flags, CPoint point)
{
	m_parent_object->OnWinMessage(WM_LBUTTONDBLCLK, flags, point);
}

void InterfaceObject_Window::OnRButtonDblClk(UINT flags, CPoint point)
{
	m_parent_object->OnWinMessage(WM_RBUTTONDBLCLK, flags, point);
}

void InterfaceObject_Window::OnMButtonDblClk(UINT flags, CPoint point)
{
	m_parent_object->OnWinMessage(WM_MBUTTONDBLCLK, flags, point);
}

void InterfaceObject_Window::OnKeyDown(UINT nchar, UINT repeat, UINT flags)
{
	if (flags & 1<<14) // отсев повторных сообщений KeyDown
		return;
	flags |= 1<<9; // признак того, что клавиша нажата
	m_parent_object->m_key_pressed[nchar] = flags;
	m_parent_object->ExecuteHandler(WM_KEYDOWN);
}

void InterfaceObject_Window::OnKeyUp(UINT nchar, UINT repeat, UINT flags)
{
	flags &= ~(1<<9); // признак того, что клавиша не нажата
	m_parent_object->m_key_pressed[nchar] = flags;
	m_parent_object->ExecuteHandler(WM_KEYUP);
}

BOOL InterfaceObject_Window::OnMouseWheel(UINT flags, short delta, CPoint point)
{
	CheckSetDowns(flags);

	m_parent_object->m_wheel_direction = delta;
	ScreenToClient(&point);
	m_parent_object->OnWinMessage(WM_MOUSEWHEEL, flags, point);
	return TRUE;
}

void InterfaceObject_Window::CheckSetDowns(UINT flags)
{
	if (m_parent_object->m_need_to_set_downs)
	{
		m_parent_object->m_left_button_down = (flags & MK_LBUTTON) != 0;
		m_parent_object->m_right_button_down = (flags & MK_RBUTTON) != 0;
		m_parent_object->m_middle_button_down = (flags & MK_MBUTTON) != 0;
		m_parent_object->m_need_to_set_downs = false;
	}
}

void InterfaceObject_Window::OnMouseMove(UINT flags, CPoint point)
{
	m_parent_object->m_winmessage_point = point;

	CheckSetDowns(flags);

	// на всякий случай >>
	if (m_parent_object->m_left_button_down && (flags & MK_LBUTTON) == 0)
	{
		m_parent_object->OnWinMessage(WM_LBUTTONUP, flags, point);
		m_parent_object->m_left_button_down = false;
	}
	if (m_parent_object->m_right_button_down && (flags & MK_RBUTTON) == 0)
	{
		m_parent_object->OnWinMessage(WM_RBUTTONUP, flags, point);
		m_parent_object->m_right_button_down = false;
	}
	if (m_parent_object->m_middle_button_down && (flags & MK_MBUTTON) == 0)
	{
		m_parent_object->OnWinMessage(WM_MBUTTONUP, flags, point);
		m_parent_object->m_middle_button_down = false;
	}
	// на всякий случай <<

	bool in = (m_parent_object->m_window_rect.PtInRect(point) == TRUE);

	if (!m_parent_object->m_always_capture && m_parent_object->m_can_loose_capture && in)
	{
		CPoint screen_point = point;
		ClientToScreen(&screen_point);
		CWnd* wnd = WindowFromPoint(screen_point);
		if (wnd != NULL && wnd != this)
			in = false;	
	}
	else
	if (m_parent_object->m_left_button_down || m_parent_object->m_right_button_down || m_parent_object->m_middle_button_down)
		in = true;

	if (in && !m_mouse_in)
		MouseIn();
	else
	if (!in && m_mouse_in)
		MouseOut();
	if (in || m_parent_object->m_report_mouse_move_when_leave)
		m_parent_object->OnWinMessage(WM_MOUSEMOVE, flags, point);
}

BOOL InterfaceObject_Window::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->hwnd != m_hWnd)
		return CWnd::PreTranslateMessage(pMsg);

	m_parent_object->m_winmessage_wparam = pMsg->wParam;
	m_parent_object->m_winmessage_lparam = pMsg->lParam;

	switch (pMsg->message)
	{
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_MOUSEWHEEL:
		case WM_MOUSEMOVE:
			break;
		case WM_KEYUP:
			OnKeyUp(pMsg->wParam, 0, 0xFFFF & (pMsg->lParam >> 16));
			return TRUE;
		case WM_KEYDOWN:
			OnKeyDown(pMsg->wParam, 0, 0xFFFF & (pMsg->lParam >> 16));
			return TRUE;
		default:
			m_parent_object->ExecuteHandler(pMsg->message);
	}
	return CWnd::PreTranslateMessage(pMsg);
}

void InterfaceObject_Window::OnCaptureChanged(CWnd* new_wnd)
{
	if (m_mouse_in)
		MouseOut();
}

void InterfaceObject_Window::OnKillFocus(CWnd* new_wnd)
{
	if (m_mouse_in)
		MouseOut();
}

////////////////////////////////////////////////////////////////////////////////
// InterfaceObject
////////////////////////////////////////////////////////////////////////////////
CFont* InterfaceObject::m_font = NULL;
DWORD* InterfaceObject::m_text_color = NULL;

InterfaceObject::InterfaceObject():
	m_left_button_down(false), m_right_button_down(false), m_middle_button_down(false),
	m_mem_dc_available(false),
	m_use_mem_dc(true),
	m_need_to_set_downs(false), m_can_loose_capture(true), m_always_capture(false),
	m_capture_focus(false), m_report_mouse_move_when_leave(false),
	m_thinning_available(false), m_thinning_last_tick(0), m_thinning_free_ticks(50), m_invalidate_times(0),
	m_own_font(NULL), m_own_text_color(NULL),
	m_after_draw_function(NULL)
{
	m_prev_winmessage_point = m_winmessage_point = CPoint(0, 0);
	m_wnd = new InterfaceObject_Window();
}

void InterfaceObject::SelectFont(CDC*dc)
{
	dc->SetBkMode(TRANSPARENT);

	DWORD* clr = m_text_color ? m_text_color : (m_own_text_color ? m_own_text_color : NULL);
	dc->SetTextColor(clr ? *clr : 0x000000);

	if (!OwnSelectFont(dc))
	{
		CFont *fnt = m_own_font ? m_own_font : (m_font ? m_font : NULL);
		if (fnt)
			dc->SelectObject(fnt);
	}
}

bool InterfaceObject::IsMouseIn()
{
	return ((InterfaceObject_Window*)m_wnd)->m_mouse_in;
}

void InterfaceObject::SetWindowName(CString str)
{
	((InterfaceObject_Window*)m_wnd)->m_name_for_debug = str;
}

InterfaceObject::~InterfaceObject()
{
	OnDestroy();
	if (m_wnd)
		delete m_wnd;
}

void InterfaceObject::AdjustStartMouseStatus()
{
	InterfaceObject_Window* wnd = (InterfaceObject_Window*)m_wnd;
	m_need_to_set_downs = true;
	wnd->m_mouse_in = false;
	wnd->MouseIn();
}

void InterfaceObject::SetMessageHandler(UINT msg, InterfaceObject_OwnFunction handler)
{
	if (handler == NULL)
		m_message_own_handlers.erase(msg);
	else
		m_message_own_handlers[msg] = handler;
}

void InterfaceObject::SetMessageHandler(UINT msg, InterfaceObject_GlobalFunction handler)
{
	if (handler == NULL)
		m_message_global_handlers.erase(msg);
	else
		m_message_global_handlers[msg] = handler;
}

void InterfaceObject::SetMessageHandler(UINT msg, InterfaceObject_GlobalFunctionWithParam handler)
{
	if (handler == NULL)
		m_message_global_with_param_handlers.erase(msg);
	else
		m_message_global_with_param_handlers[msg] = handler;
}

void InterfaceObject::ExecuteHandler(UINT msg)
{
	InterfaceObject_OwnFunctions::iterator desired_own = m_message_own_handlers.find(msg);
	if (desired_own != m_message_own_handlers.end())
	{
		InterfaceObject_OwnFunction func = desired_own->second;
		if (func)
			(this->*func)();
	}
	InterfaceObject_GlobalFunctions::iterator desired_global = m_message_global_handlers.find(msg);
	if (desired_global != m_message_global_handlers.end())
	{
		InterfaceObject_GlobalFunction func = desired_global->second;
		if (func)
			(*func)();
	}
	InterfaceObject_GlobalFunctionsWithParams::iterator desired_global2 = m_message_global_with_param_handlers.find(msg);
	if (desired_global2 != m_message_global_with_param_handlers.end())
	{
		InterfaceObject_GlobalFunctionWithParam func = desired_global2->second;
		if (func)
			(*func)(this);
	}
}

void InterfaceObject::OnWinMessage(UINT msg, UINT flags, CPoint point)
{
//	if (point.x > 1000 || msg == WM_LBUTTONDOWN)
//		int ggg = 1;
	m_winmessage = msg;
	m_winmessage_flag = flags;
	m_winmessage_point = point;
	ExecuteHandler(msg);
	m_prev_winmessage_point = m_winmessage_point;
}

void InterfaceObject::Create(CWnd *parent, CRect &rect, bool child, bool border, DWORD bk_color, bool with_brush, DWORD additional_class_style, DWORD window_style, UINT id)
{
	Create(parent, rect.left, rect.top, rect.Width(), rect.Height(), child, border, bk_color, with_brush, additional_class_style, window_style, id);
}

void InterfaceObject::Create(CWnd *parent, int x, int y, int w, int h, bool child, bool border, DWORD bk_color, bool with_brush, DWORD additional_class_style, DWORD window_style, UINT id)
{
	m_bk_color = bk_color;
	m_has_borders = border;
	InterfaceObject_Window* wnd = ((InterfaceObject_Window*)m_wnd);
	wnd->m_parent_object = this;
	wnd->MyCreate(x, y, w, h, parent, child, with_brush, additional_class_style, window_style, id);
	OnCreate();
	PrivateOnChangeSize();
}

void InterfaceObject::DeleteMemDC()
{
	if (m_mem_dc_available)
	{
		MutexWrap mem_dc_access(m_mem_dc_mutex);
		m_bitmap.DeleteObject();
		m_mem_dc.DeleteDC();
		m_mem_dc_available = false;
	}
}

void InterfaceObject::CreateMemDC()
{
	if (m_wnd == NULL || !::IsWindow(m_wnd->m_hWnd))
		return;
	DeleteMemDC();

	{
		MutexWrap mem_dc_access(m_mem_dc_mutex);
		CClientDC dc(m_wnd);
		int w = m_window_rect.Width(), h = m_window_rect.Height();
		if (w > 0 && h > 0 && m_mem_dc.CreateCompatibleDC(&dc))
		{
			if (m_bitmap.CreateCompatibleBitmap(&dc, w, h))
			{
				m_mem_dc.SelectObject(&m_bitmap);
				m_mem_dc_available = true;
			}
			else
				m_mem_dc.DeleteDC();
		}
	}
}

void InterfaceObject::PrivateOnChangeSize()
{
	if (m_wnd == NULL || m_wnd->m_hWnd == NULL)
		return;
	if (m_use_mem_dc)
	{
		CreateMemDC();
	}
	OnChangeSize();
}

void InterfaceObject::ChangePosition(int x, int y, int w, int h, int add_swp)
{
	bool size_change = true;
	if (m_wnd->m_hWnd)
	{
		bool change = false;
		CRect rect;
		m_wnd->GetWindowRect(&rect);
		m_wnd->ScreenToClient(&rect);
		size_change = (rect.Width() != w || rect.Height() != h);
		if (rect.left != x || rect.right != x + w || rect.top != y || rect.bottom != y + h)
			change = true;
		BOOL visible = m_wnd->IsWindowVisible();
		if (((add_swp & SWP_HIDEWINDOW) && visible) || ((add_swp & SWP_SHOWWINDOW) && !visible))
			change = true;
		if (!change)
			return;
		m_wnd->SetWindowPos(NULL, x, y, w, h, SWP_NOZORDER | add_swp);
	}
	m_window_pos.x = x;
	m_window_pos.y = y;
	m_window_rect.right = w - (m_has_borders?2:0);
	m_window_rect.bottom = h - (m_has_borders?2:0);
	if (size_change)
		PrivateOnChangeSize();
	Invalidate();
}

void InterfaceObject::Invalidate(BOOL erase)
{
	if (m_wnd && ::IsWindow(m_wnd->m_hWnd))
		m_wnd->Invalidate(erase);
}

void InterfaceObject::SetTimer(UINT timer_id, UINT millisec)
{
	m_wnd->SetTimer(timer_id, millisec, NULL);
}

void InterfaceObject::KillTimer(UINT timer_id)
{
	m_wnd->KillTimer(timer_id);
}

void InterfaceObject::PrivateDraw(CDC* dc)
{
	if (m_thinning_available)
	{
		DWORD tick = GetTickCount();
		if (tick - m_thinning_last_tick < m_thinning_free_ticks)
			return;
	}
	m_invalidate_times++;

	{
		MutexWrap mem_dc_access(m_mem_dc_mutex);
		CDC* cdc = (m_use_mem_dc && m_mem_dc_available) ? (&m_mem_dc) : dc;
		if (m_use_mem_dc)
			cdc->FillSolidRect(m_window_rect, m_bk_color);

	//	if (m_debug_name == "sd")
	//		int zz = 0;
		SelectFont(cdc);
		Draw(cdc);

		if (m_after_draw_function)
			m_after_draw_function(cdc, this);

		if (m_use_mem_dc && m_mem_dc_available)
			dc->BitBlt(0, 0, m_window_rect.Width(), m_window_rect.Height(), &m_mem_dc, 0, 0, SRCCOPY);
	}

	m_thinning_last_tick = GetTickCount();
}

bool InterfaceObject::IsKeyPressed(UINT key)
{
	map<UINT, UINT>::iterator desired = m_key_pressed.find(key);
	if (desired == m_key_pressed.end())
		return false;
	return (desired->second & (1<<9)) != 0;
}

////////////////////////////////////////////////////////////////////////////////
// end