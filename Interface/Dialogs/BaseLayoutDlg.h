// uic 2010.10.25

#pragma once

#include "./Interface/InterfaceHeader.h"
#include "./Interface/InterfaceForm.h"

extern float BaseLayoutDlg_pixel_k;

struct BaseLayoutDlg: CDialog
{
	BaseLayoutDlg(CString name, int w, int h);

	CString m_name;
	InterfaceHeader m_header;
	int m_start_y;
	int m_w, m_h;

	static CFont* m_font;

	virtual INT_PTR DoModal();
	virtual void CreateLayouts() {}

	bool IsInited() { return m_inited; }
	void SetName(CString new_name);

protected:
	DLGTEMPLATE m_template_data[2];
	bool m_template_inited;

	InterfaceForm m_form;

	bool m_dragging, m_inited;
	CPoint m_click_point;
	int m_blink_counter;
	bool m_bklinked_color;

	DWORD m_bk_color;
	CBrush m_brush;

	virtual BOOL OnInitDialog();
	LRESULT OnChangePosition(WPARAM wParam, LPARAM lParam);

	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* wnd, UINT nCtlColor);

	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};

////////////////////////////////////////////////////////////////////////////////
// end