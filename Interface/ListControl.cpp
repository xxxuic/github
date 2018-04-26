// uic 2.03.2010

#include <stdafx.h>

#include "Listcontrol.h"
#include "CLR.h"

#include "./MutexWrap.h"

#define ListControl_WM_SCROLL (WM_USER + 1)

ListControl::ListControl(int scroll_spaceing, int scroll_width, int const_cells_height):
	m_scroll_spaceing(scroll_spaceing), m_scroll_width(scroll_width), m_vertical_scroll_bar(false)
{
	m_capture_focus = true;

	if (m_scroll_spaceing > 0)
		m_scroll_spaceing += 2;

	SetCellsHeight(const_cells_height);

	m_filled_cells = 0;
	m_cells.UpdateSize(100);
	m_cells.Shrink();
}

void ListControl::SetCellsHeight(int const_cells_height)
{
	m_const_cells_height = const_cells_height;
	m_vertical_scroll_bar.SetButtonStep((m_const_cells_height > 0) ? m_const_cells_height : 1);
}

void ListControl::OnSetButtonStep()
{
	if (m_const_cells_height > 0)
		m_vertical_scroll_bar.SetButtonStep(m_window_rect.Height() / 100);
}

void ListControl::OnChangeSize()
{
	int x = m_window_rect.Width() - m_scroll_width, y = 0, w = m_scroll_width, h = m_window_rect.Height();
	m_vertical_scroll_bar.ChangePosition(x, y, w, h);

//	m_vertical_scroll_bar.SetTotalElements(h * 2);
	m_vertical_scroll_bar.SetVisibleElements(h - 2);
	OnSetButtonStep();
}

void ListControl::OnCreate()
{
	int x = m_window_rect.Width() - m_scroll_width, y = 0, w = m_scroll_width, h = m_window_rect.Height();
	m_vertical_scroll_bar.Create(GetWnd(), x, y, w, h, true, false, 0, false, 0, WS_VISIBLE);
	m_vertical_scroll_bar.SetReaction(GetWnd()->m_hWnd, ListControl_WM_SCROLL);
	SetMessageHandler(ListControl_WM_SCROLL, (InterfaceObject_OwnFunction)(&ListControl::OnScroll));
	SetMessageHandler(WM_MOUSEWHEEL, (InterfaceObject_OwnFunction)(&ListControl::OnMouseWheel));
}

void ListControl::OnMouseWheel()
{
	int prev = m_vertical_scroll_bar.GetFirstVisibleElement();
	int dir = (m_wheel_direction > 0) ? 1 : ((m_wheel_direction < 0) ? -1 : 0);
	m_vertical_scroll_bar.SetFirstVisibleElement(prev - dir * m_vertical_scroll_bar.GetButtonStep());
	if (prev != m_vertical_scroll_bar.GetFirstVisibleElement())
	{
		Invalidate();
		//KillTimer(1);
		//SetTimer(1, 50);
	}
}

void ListControl::OnTimer(UINT timer_id)
{
	if (timer_id == 1) // реакция на колесо
	{
		KillTimer(1);
		OnScroll();
	}
}

void ListControl::OnScroll()
{
	int w = m_window_rect.Width() - m_scroll_width - m_scroll_spaceing,
		h = m_window_rect.Height();
	CRect rect(0, 0, w, h);
	GetWnd()->InvalidateRect(rect);
}

void ListControl::Invalidate(BOOL erase)
{
	OnScroll();
	m_vertical_scroll_bar.Invalidate(erase);
}

void ListControl::Draw(CDC* dc)
{
	MutexWrap list_access(m_list_mutex, "ListControl::Draw list_access");

	DWORD border_color = /*(GetWnd()->GetFocus() == GetWnd()) ? 0x0000FF : */m_vertical_scroll_bar.GetColor(3);
	int w = m_window_rect.Width() - m_scroll_width - m_scroll_spaceing,
		h = m_window_rect.Height(), list_h = h;
	//m_vertical_scroll_bar.SetVisibleElements(h - 2);
	dc->FillSolidRect(1, 1, w, h - 2, 0xFFFFFF/*m_vertical_scroll_bar.GetColor(1)*/); // фон. центр
	dc->FillSolidRect(0, 0, 1, h, border_color); // рамка. лево
	if (m_scroll_spaceing > 0)
	{
		dc->FillSolidRect(w + 1, 0, 1, h, border_color); // рамка. лево
		dc->FillSolidRect(w + 2, 0, m_scroll_spaceing - 1, h, m_vertical_scroll_bar.GetColor(1)); // зазор
	}

	ListControl_CellInfo* buff = m_cells.GetBuff();
	int pos = 0, num = 0, total_h = 0, y = 0, top_visible = m_vertical_scroll_bar.GetFirstVisibleElement();
	h = 0;
	while (num < m_filled_cells)
	{
		h = GetCellHeight(num);
		if (total_h + h > top_visible)
		{
			y = total_h - top_visible;
			break;
		}
		total_h += h;
		num++;
	}
	if (total_h + h > top_visible)
	{
		int x = 1;
		y++;
		while (num < m_filled_cells)
		{
			h = GetCellHeight(num);
			DrawCell(dc, pos, num, buff[num].m_data, x, y, w, h);
			y += h;
			num++;
			pos++;
			if (y >= list_h)
				break;
		}
	}

	h = m_window_rect.Height();
	dc->FillSolidRect(0, 0, w + 1, 1, border_color); // рамка. верх
	dc->FillSolidRect(0, h - 1, w + 1, 1, border_color); // рамка. низ
}

void ListControl::DrawCell(CDC* dc, int cell_pos, int cell_num, DWORD cell_data, int x, int y, int w, int h)
{
	dc->FillSolidRect(x, y, w, h, 15 * cell_num);
}

bool ListControl::InsertCell(int pos, DWORD data, int height)
{
	MutexWrap list_access(m_list_mutex, "ListControl::InsertCell list_access");

	// определяем высоту
	if (height == -1)
		height = m_const_cells_height;
	if (height <= 0)
		return false;

	// увеличиваем массив, если надо
	if (m_cells.GetCurrentSize() < m_filled_cells + 1)
		m_cells.UpdateSize(m_filled_cells * 2);

	// проверка границ массива
	if (pos < 0 || pos > m_filled_cells)
		pos = m_filled_cells;

	ListControl_CellInfo* buff = m_cells.GetBuff();

	// сдвигаем, если вставка внутрь
	if (pos < m_filled_cells)
	{
		for (int i = m_filled_cells; i > pos; i--)
			buff[i] = buff[i - 1];
	}

	// добавление элемента
	buff[pos].m_data = data;
	buff[pos].m_height = height;
	m_filled_cells++;

	m_total_cells_height += GetCellHeight(pos); // после добавления

	m_vertical_scroll_bar.SetTotalElements(GetTotalCellHeight());
	m_vertical_scroll_bar.SetVisibleElements(GetTotalVisibleElements());

	return true;
}

int ListControl::GetTotalVisibleElements()
{
	return m_window_rect.Height() - 2;
}

int ListControl::GetTotalCellHeight()
{
	return m_total_cells_height;
}

int ListControl::GetCellHeight(int pos)
{
	if (m_const_cells_height != -1)
		return m_const_cells_height;
	{
		MutexWrap list_access(m_list_mutex, "ListControl::GetCellHeight list_access");
		ListControl_CellInfo* buff = m_cells.GetBuff();
		if (pos < 0 || pos >= m_filled_cells)
			return 0;
		return buff[pos].m_height;
	}
}

bool ListControl::DeleteCell(int pos)
{
	MutexWrap list_access(m_list_mutex, "ListControl::DeleteCell list_access");

	// проверка границ массива
	if (pos < 0 || pos >= m_filled_cells)
		return false;

	m_total_cells_height -= GetCellHeight(pos); // до удаления

	// сдвигаем, если удаление не конца
	ListControl_CellInfo* buff = m_cells.GetBuff();
	for (int i = pos; i < m_filled_cells; i++)
		buff[i] = buff[i + 1];

	m_filled_cells--;

	m_vertical_scroll_bar.SetTotalElements(GetTotalCellHeight());
	m_vertical_scroll_bar.SetVisibleElements(GetTotalVisibleElements());

	return true;
}

void ListControl::Clear()
{
	MutexWrap list_access(m_list_mutex, "ListControl::Clear list_access");

	m_filled_cells = 0;
	m_cells.UpdateSize(100);
	m_cells.Shrink();
}

////////////////////////////////////////////////////////////////////////////////
// end