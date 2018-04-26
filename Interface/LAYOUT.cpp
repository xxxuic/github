// uic 16.12.2009

#include <stdafx.h>
#include "LAYOUT.h"

LAYOUT_Element::LAYOUT_Element(LAYOUT_Element *parent, LAYOUT_Orientation childs_orientation, int desired_w, int desired_h):
	m_parent(parent),
	m_childs_orientation(childs_orientation),
	m_desired_w(desired_w), m_desired_h(desired_h),
	m_calculated_w(-1), m_calculated_h(-1),
	m_x(-1), m_y(-1), m_w(-1), m_h(-1), 
	m_gap_left(0), m_gap_right(0), m_gap_top(0), m_gap_bottom(0),
	m_interface_object(NULL), m_wnd_object(NULL), m_wnd_id(-1),
	m_hidden(false),
	m_transparent(false)
{
	if (m_parent != NULL)
		m_parent->m_childs.push_back(this);
}

LAYOUT_Element::~LAYOUT_Element()
{
	LAYOUT_Elements::iterator current = m_childs.begin();
	while (current != m_childs.end())
	{
		delete *current;
		current++;
	}
}

void LAYOUT_Element::SetGaps(int left, int right, int top, int bottom)
{
	m_gap_left = left;
	m_gap_right = right;
	m_gap_top = top;
	m_gap_bottom = bottom;
}

int LAYOUT_Element::CalculateFixedSize(LAYOUT_Orientation orientation)
{
	if (orientation == LAYOUT_orientation_vertical && m_desired_h > 0)
	{
		m_calculated_h = m_desired_h;//m_gap_top + m_desired_h + m_gap_bottom;
		return m_calculated_h;
	}
	if (orientation == LAYOUT_orientation_horizontal && m_desired_w > 0)
	{
		m_calculated_w = m_desired_w;//m_gap_left + m_desired_w + m_gap_right;
		return m_calculated_w;
	}

	int res = 0, size;
	LAYOUT_Element* element;
	LAYOUT_Elements::iterator current = m_childs.begin();
	while (current != m_childs.end())
	{
		element = *current;
		if (!element->m_hidden)
		{
			size = element->CalculateFixedSize(orientation);
			if (orientation != m_childs_orientation)
				res = max(res, size);
			else
				res += size;
		}
		current++;
	}
	if (orientation == LAYOUT_orientation_vertical)
	{
		res += m_gap_top + m_gap_bottom;
		m_calculated_h = res;
	}
	else
	{
		res += m_gap_left + m_gap_right;
		m_calculated_w = res;
	}
	return res;
}

void LAYOUT_Element::CalculateSize(int suggested_w, int suggested_h)
{
	LAYOUT_Element* element;
	int total_w = 0, total_h = 0;
	
	suggested_w -= m_gap_left + m_gap_right;
	suggested_h -= m_gap_top + m_gap_bottom;

	if (!m_childs.empty())
	{
		LAYOUT_Elements::iterator current;
		int fixed_w = 0, fixed_h = 0, total_w_weight = 0, total_h_weight = 0;

		// собираем фиксированные и весовые размеры >>
		current = m_childs.begin();
		while (current != m_childs.end())
		{
			element = *current;
			if (!element->m_hidden)
			{
				if (m_childs_orientation == LAYOUT_orientation_vertical)
				{
					if (element->m_desired_w == 0) // для вычисляемых треба вычислить сначала
						fixed_w = max(fixed_w, element->CalculateFixedSize(LAYOUT_orientation_horizontal));
					else
						fixed_w = max(fixed_w, element->m_desired_w); // выбираем максимум фиксированных ширин
		
					if (element->m_desired_h == 0) // для вычисляемых треба вычислить сначала
						fixed_h += element->CalculateFixedSize(LAYOUT_orientation_vertical);
					else
					if (element->m_desired_h > 0)
						fixed_h += element->m_desired_h;
					else
						total_h_weight += -element->m_desired_h; // суммиреум веса ширин
				}
				else
				{
					if (element->m_desired_h == 0) // для вычисляемых треба вычислить сначала
						fixed_h = max(fixed_h, element->CalculateFixedSize(LAYOUT_orientation_vertical));
					else
						fixed_h = max(fixed_h, element->m_desired_h); // выбираем максимум фиксированных высот

					if (element->m_desired_w == 0) // для вычисляемых треба вычислить сначала
						fixed_w += element->CalculateFixedSize(LAYOUT_orientation_horizontal);
					else
					if (element->m_desired_w > 0)
						fixed_w += element->m_desired_w;
					else
						total_w_weight += -element->m_desired_w; // суммиреум веса высот
				}
			}
			current++;
		}
		// собираем фиксированные и весовые размеры <<

		double unfixed_suggested_w, unfixed_suggested_h;
		int rest_w = suggested_w - fixed_w, rest_h = suggested_h - fixed_h;
		if (m_childs_orientation == LAYOUT_orientation_vertical)
		{
			unfixed_suggested_w = max(fixed_w, suggested_w);
			unfixed_suggested_h = (total_h_weight == 0) ? 0 : double(rest_h) / total_h_weight;
		}
		else
		{
			unfixed_suggested_w = (total_w_weight == 0) ? 0 : double(rest_w) / total_w_weight;
			unfixed_suggested_h = max(fixed_h, suggested_h);
		}

		// вычисление размеров детей >>
		int sugg_w, sugg_h;
		current = m_childs.begin();
		while (current != m_childs.end())
		{
			element = *current;
			if (!element->m_hidden)
			{
				if (m_childs_orientation == LAYOUT_orientation_vertical)
				{
					if (element->m_desired_h == 0)
						sugg_h = element->m_calculated_h;
					else
					if (element->m_desired_h > 0)
						sugg_h = element->m_desired_h;
					else
					{
						total_h_weight -= -element->m_desired_h;
						if (total_h_weight > 0)
						{
							sugg_h = -element->m_desired_h * unfixed_suggested_h;
							rest_h -= sugg_h;
						}
						else
							sugg_h = rest_h;
					}
					sugg_w = unfixed_suggested_w;
				}
				else
				{
					if (element->m_desired_w == 0)
						sugg_w = element->m_calculated_w;
					else
					if (element->m_desired_w > 0)
						sugg_w = element->m_desired_w;
					else
					{
						total_w_weight -= -element->m_desired_w;
						if (total_w_weight > 0)
						{
							sugg_w = -element->m_desired_w * unfixed_suggested_w;
							rest_w -= sugg_w;
						}
						else
							sugg_w = rest_w;
					}
					sugg_h = unfixed_suggested_h;
				}
				element->CalculateSize(sugg_w, sugg_h);
			}
			current++;
		}
		// вычисление размеров детей <<
	}

	m_w = suggested_w + m_gap_left + m_gap_right;
	m_h = suggested_h + m_gap_top + m_gap_bottom;
}

void LAYOUT_Element::CalculatePosition(int x, int y)
{
	m_x = x;
	m_y = y;

	x += m_gap_left;
	y += m_gap_top;

	if (m_interface_object)
		m_interface_object->ChangePosition(m_x + m_gap_left, m_y + m_gap_top, m_w - m_gap_left - m_gap_right, m_h - m_gap_top - m_gap_bottom);
	if (m_wnd_id != -1)
	{
		CWnd* wnd = m_wnd_object->GetDlgItem(m_wnd_id);
		wnd->SetWindowPos(NULL, m_x + m_gap_left, m_y + m_gap_top, m_w - m_gap_left - m_gap_right, m_h - m_gap_top - m_gap_bottom, SWP_NOZORDER);
	}
	else
	if (m_wnd_object)
		m_wnd_object->SetWindowPos(NULL, m_x + m_gap_left, m_y + m_gap_top, m_w - m_gap_left - m_gap_right, m_h - m_gap_top - m_gap_bottom, SWP_NOZORDER);

	LAYOUT_Element* element;

	if (!m_childs.empty())
	{
		LAYOUT_Elements::iterator current;
		current = m_childs.begin();
		while (current != m_childs.end())
		{
			element = *current;
			if (!element->m_hidden)
			{
				element->CalculatePosition(x, y);
				if (m_childs_orientation == LAYOUT_orientation_vertical)
					y += element->m_h;
				else
					x += element->m_w;
			}
			current++;
		}
	}
}

void LAYOUT_Element::ChangePosition(int x, int y, int w, int h)
{
	CalculateSize(w, h);
	CalculatePosition(x, y);
}

void LAYOUT_Element::ShowHideObjects(bool show)
{
	if (m_hidden)
		return;

	DWORD swhd = (show ? SW_NORMAL : SW_HIDE);
	if (m_interface_object)
		m_interface_object->GetWnd()->ShowWindow(swhd);
	if (m_wnd_id != -1)
	{
		CWnd* wnd = m_wnd_object->GetDlgItem(m_wnd_id);
		wnd->ShowWindow(swhd);
	}
	else
	if (m_wnd_object)
		m_wnd_object->ShowWindow(swhd);

	LAYOUT_Elements::iterator current = m_childs.begin();
	while (current != m_childs.end())
	{
		(*current)->ShowHideObjects(show);
		current++;
	}
}

void LAYOUT_Element::Show(bool show, bool apply, bool redraw)
{
	LAYOUT_Element *parent_layout = this;
	while (parent_layout->m_parent != NULL)
	//if (parent_layout->m_parent != NULL)
		parent_layout = parent_layout->m_parent;

	if (!show && apply)
		ShowHideObjects(false);
	m_hidden = !show;
	parent_layout->ChangePosition(parent_layout->m_x, parent_layout->m_y, parent_layout->m_w, parent_layout->m_h);
	if (show && apply)
		ShowHideObjects(true);

	if (redraw)
	{
		// перерисовка >>
		CWnd *wnd = GetElementWnd();
		if (wnd)
			wnd = wnd->GetParent();
		if (wnd == NULL) 
			wnd = AfxGetMainWnd();
/*
		LAYOUT_Element *parent_layout = this;
		while (parent_layout->GetParent() != NULL)
			parent_layout = parent_layout->GetParent();
/**/
		CRgn rgn_all, rgn_free;
		rgn_all.CreateRectRgn(0, 0, 1, 1);
		parent_layout->GeatherRegion(&rgn_all);
		rgn_free.CreateRectRgn(parent_layout->m_x, parent_layout->m_y, parent_layout->m_x + parent_layout->m_w, parent_layout->m_y + parent_layout->m_h);
		rgn_free.CombineRgn(&rgn_free, &rgn_all, RGN_XOR);
		parent_layout->Invalidate(FALSE); // перерисовка всего основного layout
		wnd->InvalidateRgn(&rgn_free, TRUE);
		// перерисовка <<
	}
}

void LAYOUT_Element::GeatherRegion(CRgn* rgn)
{
	if (m_hidden)
		return;

	if ((m_interface_object || m_wnd_object) && !m_transparent)
	{
		CWnd *wnd = GetElementWnd();
		if (wnd)
		{
			CWnd *parent_wnd = wnd->GetParent();
			CRect rect;
			wnd->GetClientRect(&rect);
			wnd->ClientToScreen(&rect);
			parent_wnd->ScreenToClient(&rect);
			CRgn this_rgn;
			this_rgn.CreateRectRgnIndirect(&rect);
			rgn->CombineRgn(rgn, &this_rgn, RGN_OR);
		}
		return;
	}
		
	LAYOUT_Elements::iterator current = m_childs.begin();
	while (current != m_childs.end())
	{
		(*current)->GeatherRegion(rgn);
		current++;
	}
}

CWnd* LAYOUT_Element::GetElementWnd()
{
	if (m_interface_object || m_wnd_object)
		return (m_wnd_object) ? ((m_wnd_id != -1) ? m_wnd_object->GetDlgItem(m_wnd_id) : m_wnd_object) : ((m_interface_object != NULL) ? m_interface_object->GetWnd() : NULL);
	return NULL;	
}

void LAYOUT_Element::Invalidate(BOOL erase)
{
	if (m_interface_object || m_wnd_object)
	{
		CWnd *wnd = GetElementWnd();
		if (wnd)
			wnd->Invalidate(erase);
	}
		
	LAYOUT_Elements::iterator current = m_childs.begin();
	while (current != m_childs.end())
	{
		(*current)->Invalidate(erase);
		current++;
	}
}

void LAYOUT_Element::AdjustElementAndWindindow()
{
	CWnd* wnd = GetElementWnd();
	if (wnd == NULL)
		return;

	LAYOUT_Element* layout = this;
	while (!layout->m_hidden && layout->m_parent != NULL)
	{
		layout = layout->m_parent;
	}

//	wnd->ShowWindow(layout->m_hidden ? SW_HIDE : SW_NORMAL);
}

void LAYOUT_Element::SetInterfaceObject(InterfaceObject* interface_object)
{
	m_interface_object = interface_object;
	AdjustElementAndWindindow();
}

void LAYOUT_Element::SetWndObject(CWnd* wnd_object)
{
	m_wnd_object = wnd_object;
	AdjustElementAndWindindow();
}

void LAYOUT_Element::SetWndID(UINT id, CWnd* parent_wnd_object)
{
	m_wnd_id = id;
	m_wnd_object = parent_wnd_object;
	AdjustElementAndWindindow();
}

////////////////////////////////////////////////////////////////////////////////
// end