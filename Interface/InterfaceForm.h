// uic 15.12.2009

#pragma once

#include "InterfaceObject.h"
#include "LAYOUT.h"

struct InterfaceForm;
typedef void (*InterfaceForm_Function)(InterfaceForm*);
typedef void (*InterfaceForm_FunctionInt)(InterfaceForm*, int);
typedef map<int, bool> InterfaceForm_Timers;

#define InterfaceForm_NULL_INSETS 0, 0, 0, 0
#define InterfaceForm_TOP_INSETS 0, 0, InterfaceForm::m_gap, 0
#define InterfaceForm_BOTTOM_INSETS 0, 0, 0, InterfaceForm::m_gap
#define InterfaceForm_LEFT_INSETS InterfaceForm::m_gap, 0, 0, 0
#define InterfaceForm_RIGHT_INSETS 0, InterfaceForm::m_gap, 0, 0
#define InterfaceForm_ALL_INSETS InterfaceForm::m_gap, InterfaceForm::m_gap, InterfaceForm::m_gap, InterfaceForm::m_gap

struct InterfaceForm
{
	InterfaceForm();
	~InterfaceForm();
	
	void Clear();

	static int m_ve_h, m_button_h, m_static_h, m_edit_h, m_header_h, m_gap;

	CString m_name, m_btn_name;
	int m_id;

	void Invalidate(BOOL erase = FALSE);
	void ShowHide(int sw_normal_sw_hide, bool redraw = true);
	void ChangePosition(int x, int y, int w, int h);
	bool OnTimer(int num);
	void ActivateTimer(int num, bool active);
	
	virtual void OnChangeSize() {}

	LAYOUT_Element* CreateBaseLayout(LAYOUT_Orientation child_orientetion, int w, int h, LAYOUT_Element *parent_layout = NULL);
	LAYOUT_Element* StartEmptyLayout(LAYOUT_Orientation child_orientetion, int w = -1, int h = -1, int left_gap = 0, int right_gap = 0, int top_gap = 0, int bottom_gap = 0); // добавляет новый Layout в качестве потомка текущего и делает его текущим
	LAYOUT_Element* StartLayout(InterfaceObject* interface_object, LAYOUT_Orientation child_orientetion, int w = -1, int h = -1, int left_gap = 0, int right_gap = 0, int top_gap = 0, int bottom_gap = 0); // добавляет новый Layout в качестве потомка текущего и делает его текущим
	LAYOUT_Element* SingleElementLayout(InterfaceObject* interface_object, int w = -1, int h = -1, int left_gap = 0, int right_gap = 0, int top_gap = 0, int bottom_gap = 0); // добавляет новый Layout в качестве потомка текущего и делает его текущим
	LAYOUT_Element* StartLayout(CWnd* wnd_object, LAYOUT_Orientation child_orientetion, int w = -1, int h = -1, int left_gap = 0, int right_gap = 0, int top_gap = 0, int bottom_gap = 0); // добавляет новый Layout в качестве потомка текущего и делает его текущим
	LAYOUT_Element* SingleElementLayout(CWnd* wnd_object, int w = -1, int h = -1, int left_gap = 0, int right_gap = 0, int top_gap = 0, int bottom_gap = 0); // добавляет новый Layout в качестве потомка текущего и делает его текущим
	LAYOUT_Element* StartLayout(UINT id, LAYOUT_Orientation child_orientetion, int w = -1, int h = -1, int left_gap = 0, int right_gap = 0, int top_gap = 0, int bottom_gap = 0); // добавляет новый Layout в качестве потомка текущего и делает его текущим
	LAYOUT_Element* SingleElementLayout(UINT id, int w = -1, int h = -1, int left_gap = 0, int right_gap = 0, int top_gap = 0, int bottom_gap = 0); // добавляет новый Layout в качестве потомка текущего и делает его текущим
	void FinishLayout(CString text); // завершает текущий Layout и делает текущим предка текущего

	LAYOUT_Element* GetCurrentLayout() { return m_current_layout; }

	void SetParentWnd(CWnd* parent) { m_parent_wnd = parent; }

	InterfaceForm_Function m_on_open_function, m_on_close_function, m_on_init_function, m_on_shutdown_function;
	InterfaceForm_FunctionInt m_on_timer_function;

protected:
	list<InterfaceObject*> m_interface_objects;
	list<CWnd*> m_wnd_objects;
	list<UINT> m_ids;

	InterfaceForm_Timers m_active_timers;
	CMutex m_timers_mutex;

	CWnd* m_parent_wnd;
	LAYOUT_Element *m_layout;
	bool m_visible;

private:
	LAYOUT_Element *m_current_layout; // для наполнения формы
};

////////////////////////////////////////////////////////////////////////////////
// end