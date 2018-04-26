// uic 16.12.2009

#pragma once

#include <list>
using namespace std;

#include "InterfaceObject.h"

enum LAYOUT_Orientation
{
	LAYOUT_orientation_vertical = 0,
	LAYOUT_orientation_horizontal,
	LAYOUT_useless
};

enum LAYOUT_Direction
{
	LAYOUT_direction_bottom = 0,
	LAYOUT_direction_top,
	LAYOUT_direction_left,
	LAYOUT_direction_right
};

struct LAYOUT_Element;

typedef list<LAYOUT_Element*> LAYOUT_Elements;

struct LAYOUT_Element
{
	LAYOUT_Element(LAYOUT_Element *parent, LAYOUT_Orientation childs_orientation, int desired_w = -1/*вес ширины == 1, не фиксированная*/, int desired_h = -1/*вес высоты == 1, не фиксированная*/);
	// отрицательный ширины/высоты - весовые, положительные - фиксированные, нулевая - вычисляемая по фиксированным чилдренам
	~LAYOUT_Element();

	void ChangePosition(int x, int y, int w, int h);

	void SetGaps(int left, int right, int top, int bottom);
	void SetInterfaceObject(InterfaceObject* interface_object);
	void SetWndObject(CWnd* wnd_object);
	void SetWndID(UINT id, CWnd* parent_wnd_object);

	LAYOUT_Element* GetParent() { return m_parent; }

	void Show(bool show, bool apply = true, bool redraw = true);
	bool IsHidden() { return m_hidden; }
	void GetRect(CRect* rect) { rect->left = m_x; rect->right = m_x + m_w; rect->top = m_y; rect->bottom = m_y + m_h; }

	void GeatherRegion(CRgn* rgn);
	void Invalidate(BOOL erase);

	CWnd* GetElementWnd();

	void AdjustElementAndWindindow();

	CString m_name; // for debugging
	bool m_transparent;

	int GetX() { return m_x; }
	int GetY() { return m_y; }
	int GetW() { return m_w; }
	int GetH() { return m_h; }

	InterfaceObject* GetInterfaceObject() { return m_interface_object; }

private:
	int CalculateFixedSize(LAYOUT_Orientation orientation);
	void ShowHideObjects(bool show);

	InterfaceObject* m_interface_object;
	CWnd* m_wnd_object;
	UINT m_wnd_id;

	LAYOUT_Element* m_parent;
	LAYOUT_Elements m_childs;
	LAYOUT_Orientation m_childs_orientation;

	int m_desired_w, m_desired_h; // > 0 фиксированное, < 0 весовое, == 0, неизвестное фиксированное, определяется если все конечные потомки фиксированные
	int m_calculated_w, m_calculated_h; // вычисляется на первом проходе по desired, для вычисления фиксированных размеров, если desired == 0 // > 0 фиксированное, < 0 весовое
	int m_gap_left, m_gap_right, m_gap_top, m_gap_bottom; // зазоры внутри ячейки для потомков и m_interface_object

	int m_x, m_y, m_w, m_h; // конечный результат вычислений - область ячейки

	bool m_hidden;

//	void PreCalculateSize();
	void CalculateSize(int suggested_w, int suggested_h);
	void CalculatePosition(int x, int y);
};

////////////////////////////////////////////////////////////////////////////////
// end