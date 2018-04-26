// uic 22.10.2008

#pragma once

#include <afxmt.h>
#include <map>
using namespace std;

struct InterfaceObject;

typedef void (InterfaceObject::*InterfaceObject_OwnFunction)(void);
typedef map<UINT, InterfaceObject_OwnFunction> InterfaceObject_OwnFunctions;

typedef void (*InterfaceObject_GlobalFunction)(void);
typedef map<UINT, InterfaceObject_GlobalFunction> InterfaceObject_GlobalFunctions;

typedef void (*InterfaceObject_GlobalFunctionWithParam)(InterfaceObject*);
typedef map<UINT, InterfaceObject_GlobalFunctionWithParam> InterfaceObject_GlobalFunctionsWithParams;

typedef void (*InterfaceObject_AfterDrawFunction)(CDC*, InterfaceObject*);

struct InterfaceObject
{
	InterfaceObject();
	virtual ~InterfaceObject();

	friend struct InterfaceObject_Window;

	void SetMessageHandler(UINT msg, InterfaceObject_OwnFunction handler);
	void SetMessageHandler(UINT msg, InterfaceObject_GlobalFunction handler);
	void SetMessageHandler(UINT msg, InterfaceObject_GlobalFunctionWithParam handler);

	// border - черная граница в один пиксель толщиной
	// with_brush определяет, будет ли окно иметь кисть для зарисовки, если да, то фоновым цветом будет зарисовыватьс при Invalidate(TRUE), если нет, то никак не будет рисоваться
	void Create(CWnd *parent, CRect &rect, bool child, bool border, DWORD bk_color = 0, bool with_brush = false, DWORD additional_class_style = 0, DWORD window_style = 0, UINT id = -1);
	void Create(CWnd *parent, int x, int y, int w, int h, bool child, bool border, DWORD bk_color = 0, bool with_brush = false, DWORD additional_class_style = 0, DWORD window_style = 0/*WS_VISIBLE*/, UINT id = -1);
	virtual void OnCreate() {}
	virtual void OnDestroy() {}

	void ChangePosition(int x, int y, int new_w, int new_h, int add_swp = SWP_NOREDRAW);

	void SetTimer(UINT timer_id, UINT millisec);
	void KillTimer(UINT timer_id);
	virtual void OnTimer(UINT timer_id) {}

	virtual void Invalidate(BOOL erase = FALSE);

	CWnd* GetWnd() { return m_wnd; }
	CRect GetWindowRect() { return m_window_rect; }
	CPoint GetWindowPos() { return m_window_pos; }
	CPoint GetMousePos() { return m_winmessage_point; }

	DWORD GetFlag() { return m_winmessage_flag; }
	DWORD GetMessage() { return m_winmessage; }
	int GetWheelDirection() { return m_wheel_direction; }

	void SetWindowName(CString str);

	bool IsAvailableMemDC() { return m_mem_dc_available; }
	CDC* GetMemDC() { return &m_mem_dc; }
	CBitmap* GetMemBitmap() { return &m_bitmap; }

	bool IsLeftButtonDown() { return m_left_button_down; }
	bool IsRightButtonDown() { return m_right_button_down; }
	bool IsMiddleButtonDown() { return m_middle_button_down; }

	bool IsMouseIn();
	virtual void OnMouseOut() {}
	virtual void OnMouseIn() {}

	DWORD m_bk_color;
	bool m_report_mouse_move_when_leave;/*если да, то сообщение WM_MOUSEMOVE вызовется и при выезде за пределы*/
	
	static CFont* m_font; // общий шрифт для всех объектов
	CFont* m_own_font; // персональный шрифт
	static DWORD* m_text_color; // общий цвет текста для всех объектов
	DWORD* m_own_text_color; // персональный цвет
	void SelectFont(CDC*dc);
	virtual bool OwnSelectFont(CDC*dc) { return false; } // если вернул true, значит шрифт установлен в DC

	bool IsKeyPressed(UINT key);
	void SetCaptureFocus(bool capture_focus) { m_capture_focus = capture_focus; }

	bool m_thinning_available;
	DWORD m_thinning_last_tick, m_thinning_free_ticks, m_invalidate_times;

	bool HasBorder() { return m_has_borders; }
	
	InterfaceObject_AfterDrawFunction m_after_draw_function;

	CString m_debug_name;
	
	virtual void OnCleanup() {};

protected:
	void CreateMemDC();
	void DeleteMemDC();

	virtual void Draw(CDC* dc) {}
	virtual void OnCloseWindow() {}
	virtual void OnDestroyWindow() {}
	virtual void OnChangeSize() {}

	bool m_use_mem_dc, m_capture_focus;

	CPoint m_window_pos;
	CRect m_window_rect;	
	CWnd* m_wnd;
	CPoint m_winmessage_point, m_prev_winmessage_point;
	DWORD m_winmessage, m_winmessage_flag;
	WPARAM m_winmessage_wparam;
	LPARAM m_winmessage_lparam;
	InterfaceObject_OwnFunctions m_message_own_handlers;
	InterfaceObject_GlobalFunctions m_message_global_handlers;
	InterfaceObject_GlobalFunctionsWithParams m_message_global_with_param_handlers;
	map<UINT, UINT> m_key_pressed;
	int m_wheel_direction;
	bool m_left_button_down, m_right_button_down, m_middle_button_down;

	bool m_can_loose_capture/*если поверх мыши другое окно*/, m_always_capture/*не терять капчу даже при выходе за границы окна и не нажатых клавишах*/;
	void AdjustStartMouseStatus(); // метод специально для меню - при появлении меню устанавливает состояние, как будто Capture захвачено на окно элемента, независимо от того, попал курсор в меню или нет и устанавливает флаги нажатости кнопок мыши

private:
	bool m_need_to_set_downs/*для проверки нажатия кнопок мыши и присвоения значений переменным*/;

	CDC m_mem_dc;
	CBitmap m_bitmap;
	bool m_mem_dc_available;
	bool m_has_borders;
	CMutex m_mem_dc_mutex;

	void PrivateOnChangeSize();
	void PrivateDraw(CDC* dc);
	void ExecuteHandler(UINT msg);
	void OnWinMessage(UINT msg, UINT flags, CPoint point);	
};
////////////////////////////////////////////////////////////////////////////////
// end