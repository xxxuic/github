// uic 15.12.2009

#include <stdafx.h>
#include "InterfaceForm.h"
#include "MutexWrap.h"

int InterfaceForm::m_ve_h = 28;
int InterfaceForm::m_button_h = 30;
int InterfaceForm::m_static_h = 25;
int InterfaceForm::m_edit_h = 25;
int InterfaceForm::m_gap = 5;
int InterfaceForm::m_header_h = 30;

InterfaceForm::InterfaceForm():
	m_id(0),	
	m_layout(NULL), m_parent_wnd(NULL), m_visible(false),
	m_on_open_function(NULL), m_on_close_function(NULL), m_on_init_function(NULL),
	m_on_timer_function(NULL)
{
}

InterfaceForm::~InterfaceForm()
{
	Clear();
}

void InterfaceForm::Clear()
{
	if (m_layout)
		delete m_layout;
	m_current_layout = m_layout = NULL;
	m_interface_objects.clear();
	m_wnd_objects.clear();
	m_ids.clear();
	
	{
		MutexWrap timers_access(m_timers_mutex);
		m_active_timers.clear();
	}
}

void InterfaceForm::Invalidate(BOOL erase)
{
	list<InterfaceObject*>::iterator current_itf = m_interface_objects.begin();
	while (current_itf != m_interface_objects.end())
	{
		(*current_itf)->GetWnd()->Invalidate(erase);
		current_itf++;
	};

	list<CWnd*>::iterator current_wnd = m_wnd_objects.begin();
	while (current_wnd != m_wnd_objects.end())
	{
		(*current_wnd)->Invalidate(erase);
		current_wnd++;
	};

	if (m_parent_wnd != NULL)
	{
		list<UINT>::iterator current_wnd_id = m_ids.begin();
		while (current_wnd_id != m_ids.end())
		{
			m_parent_wnd->GetDlgItem(*current_wnd_id)->Invalidate(erase);
			current_wnd_id++;
		};
	}
}

void InterfaceForm::ShowHide(int sw_normal_sw_hide, bool redraw)
{
	m_visible = (sw_normal_sw_hide == SW_NORMAL);
	if (m_layout)
		m_layout->Show(m_visible, true, redraw);
}

void InterfaceForm::ChangePosition(int x, int y, int w, int h)
{
	if (m_layout == NULL)
		return;
	if (m_layout)
		m_layout->ChangePosition(x, y, w, h);
	OnChangeSize();
}

void InterfaceForm::ActivateTimer(int num, bool active)
{
	MutexWrap timers_access(m_timers_mutex);
	m_active_timers[num] = active;
}

bool InterfaceForm::OnTimer(int num)
{
	// есть ли вообще реакция на таймеры
	if (!m_on_timer_function)
		return false;

	// поиск таймеров в списке
	{
		MutexWrap timers_access(m_timers_mutex);
		InterfaceForm_Timers::iterator desired = m_active_timers.find(num);
		if (desired == m_active_timers.end() || desired->second == false)
			return false;
	}

	// обработка таймера
	m_on_timer_function(this, num);
	return true;
}

LAYOUT_Element* InterfaceForm::CreateBaseLayout(LAYOUT_Orientation child_orientetion, int w, int h, LAYOUT_Element *parent_layout)
{
	Clear();
	m_layout = new LAYOUT_Element(parent_layout, child_orientetion, w, h);
	m_current_layout = m_layout;
	return m_current_layout;
}

LAYOUT_Element* InterfaceForm::StartEmptyLayout(LAYOUT_Orientation child_orientetion, int w, int h, int left_gap, int right_gap, int top_gap, int bottom_gap)
{
	m_current_layout = new LAYOUT_Element(m_current_layout, child_orientetion, (w <= 0) ? w : (left_gap + w + right_gap), (h <= 0) ? h : (top_gap + h + bottom_gap));
	m_current_layout->SetGaps(left_gap, right_gap, top_gap, bottom_gap);
	return m_current_layout;
}

LAYOUT_Element* InterfaceForm::StartLayout(InterfaceObject* interface_object, LAYOUT_Orientation child_orientetion, int w, int h, int left_gap, int right_gap, int top_gap, int bottom_gap)
{
	m_interface_objects.push_back(interface_object);
	StartEmptyLayout(child_orientetion, w, h, left_gap, right_gap, top_gap, bottom_gap);
	if (interface_object)
		m_current_layout->SetInterfaceObject(interface_object);
	return m_current_layout;
}

LAYOUT_Element* InterfaceForm::SingleElementLayout(InterfaceObject* interface_object, int w, int h, int left_gap, int right_gap, int top_gap, int bottom_gap)
{
	LAYOUT_Element* layout = StartLayout(interface_object, LAYOUT_useless, w, h, left_gap, right_gap, top_gap, bottom_gap);
	CString str;
	str.Format(_T("intf = 0x%08X"), (int)interface_object);
	FinishLayout(str);
	return layout;
}

LAYOUT_Element* InterfaceForm::StartLayout(CWnd* wnd_object, LAYOUT_Orientation child_orientetion, int w, int h, int left_gap, int right_gap, int top_gap, int bottom_gap)
{
	m_wnd_objects.push_back(wnd_object);
	StartEmptyLayout(child_orientetion, w, h, left_gap, right_gap, top_gap, bottom_gap);
	if (wnd_object)
		m_current_layout->SetWndObject(wnd_object);
	return m_current_layout;
}

LAYOUT_Element* InterfaceForm::SingleElementLayout(CWnd* wnd_object, int w, int h, int left_gap, int right_gap, int top_gap, int bottom_gap)
{
	LAYOUT_Element* layout = StartLayout(wnd_object, LAYOUT_useless, w, h, left_gap, right_gap, top_gap, bottom_gap);
	CString str;
	str.Format(_T("wnd = 0x%08X"), (int)wnd_object);
	FinishLayout(str);
	return layout;
}

LAYOUT_Element* InterfaceForm::StartLayout(UINT id, LAYOUT_Orientation child_orientetion, int w, int h, int left_gap, int right_gap, int top_gap, int bottom_gap)
{
	m_ids.push_back(id);
	StartEmptyLayout(child_orientetion, w, h, left_gap, right_gap, top_gap, bottom_gap);
	if (m_current_layout)
		m_current_layout->SetWndID(id, m_parent_wnd);
	return m_current_layout;
}

LAYOUT_Element* InterfaceForm::SingleElementLayout(UINT id, int w, int h, int left_gap, int right_gap, int top_gap, int bottom_gap)
{
	LAYOUT_Element* layout = StartLayout(id, LAYOUT_useless, w, h, left_gap, right_gap, top_gap, bottom_gap);
	CString str;
	str.Format(_T("ID = 0x%08X"), id);
	FinishLayout(str);
	return layout;
}

void InterfaceForm::FinishLayout(CString name)
{
	if (m_current_layout == NULL)
	{
#ifdef _DEBUG
		::AfxAssertFailedLine(THIS_FILE, __LINE__);
#endif
		return;
	}
	m_current_layout->m_name = name;
	m_current_layout = m_current_layout->GetParent();
}

////////////////////////////////////////////////////////////////////////////////
// end