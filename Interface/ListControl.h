// uic 2.03.2010

#pragma once

#include <afxmt.h>

#include "InterfaceObject.h"
#include "ScrollBar.h"
#include "ElasticBuffer.h"

#include <vector>
using namespace std;

struct ListControl_CellInfo
{
	DWORD m_data;
	int m_height;
};

typedef ElasticBuffer<ListControl_CellInfo> ListControl_Cells;

struct ListControl : InterfaceObject
{
	ListControl(int scroll_spaceing, int scroll_width, int const_cells_height = -1);

	void SetCellsHeight(int const_cells_height);
	
	virtual void Draw(CDC* dc);
	virtual void OnChangeSize();
	virtual void OnCreate();
	virtual void OnSetButtonStep();

	virtual void OnScroll();
	virtual void OnMouseWheel();

	virtual void OnTimer(UINT timer_id);

	bool InsertCell(int pos, DWORD data, int height = -1);
	bool DeleteCell(int pos);
	void Clear();

	virtual int GetCellHeight(int pos);
	virtual int GetTotalCellHeight();
	virtual int GetTotalVisibleElements();
	virtual int GetConstantCellsHeight() { return m_const_cells_height; }

	virtual void DrawCell(CDC* dc, int cell_pos, int cell_num, DWORD cell_data, int x, int y, int w, int h);
	virtual void Invalidate(BOOL erase = FALSE);

protected:
	ScrollBar m_vertical_scroll_bar;

	CMutex m_list_mutex;
	ListControl_Cells m_cells;
	int m_filled_cells;

	ListControl_Cells* GetCellsInfo() { return &m_cells; }

	int m_scroll_spaceing, m_scroll_width; // ширина €чеек определ€етс€ из ширины всего объекта минус m_scroll_spaceing, минус m_scroll_width
	int m_const_cells_height, m_total_cells_height;
};

////////////////////////////////////////////////////////////////////////////////
// end