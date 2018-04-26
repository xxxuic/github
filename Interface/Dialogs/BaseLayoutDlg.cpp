// uic 2010.10.25

#include <stdafx.h>

#include "BaseLayoutDlg.h"

CFont* BaseLayoutDlg::m_font = NULL;

float BaseLayoutDlg_pixel_k = 1.5; // как-то надо определять

BaseLayoutDlg::BaseLayoutDlg(CString name, int w, int h):
	m_template_inited(false), m_inited(false), m_name(name), m_w(w), m_h(h)
{
}

void BaseLayoutDlg::SetName(CString new_name)
{
	m_name = new_name;
	m_header.m_main_text = m_name;
	m_header.Invalidate();
}

BEGIN_MESSAGE_MAP(BaseLayoutDlg, CDialog)
	ON_WM_TIMER()
	ON_MESSAGE(WM_WINDOWPOSCHANGING, OnChangePosition)
	ON_WM_ACTIVATE()
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
END_MESSAGE_MAP()

INT_PTR BaseLayoutDlg::DoModal()
{
CWnd* parent = NULL;
	m_inited = false;
	if (!m_template_inited)
	{
		DLGTEMPLATE *dlg_template = m_template_data;
		dlg_template->style = WS_POPUP | WS_BORDER | DS_CENTER;
		dlg_template->dwExtendedStyle = 0;
		dlg_template->cdit = 0;

CRect rect;
GetDesktopWindow()->GetClientRect(&rect);
int x = (rect.Width() - (m_w + 2)) / 2, y = (rect.Height() - (m_h + 2)) / 2;

		dlg_template->x = x / BaseLayoutDlg_pixel_k;
		dlg_template->y = y / BaseLayoutDlg_pixel_k;
		dlg_template->cx = (m_w + 2) / BaseLayoutDlg_pixel_k;
		dlg_template->cy = (m_h + 2) / BaseLayoutDlg_pixel_k;
		DWORD* ptr = (DWORD*)(m_template_data + 1);
		*ptr = 0;
		ptr++;
		*ptr = 0;
		ptr++;
		*ptr = 0;
		parent = AfxGetMainWnd();
		m_template_inited = (InitModalIndirect(m_template_data, parent) == TRUE);
	}
	return (m_template_inited) ? CDialog::DoModal() : IDCANCEL;
}

void BaseLayoutDlg::OnSize(UINT nType, int cx, int cy)
{
	if (!m_inited)
		return;
	m_w = cx;
	m_h = cy;
	m_form.ChangePosition(0, 0, m_w, m_h);
}

BOOL BaseLayoutDlg::OnInitDialog()
{
	m_bk_color = CLR_GetSysColor(COLOR_3DFACE);

	m_header.Create(this, 0, 0, m_w, InterfaceForm::m_header_h, true, false, 0x0000FF, false, 0, WS_VISIBLE);
	m_header.m_move_parent = true;
	m_header.m_main_text = m_name;

	m_form.CreateBaseLayout(LAYOUT_orientation_vertical, m_w, m_h);
		m_form.StartLayout(&m_header, LAYOUT_useless, -1, InterfaceForm::m_header_h);
		m_form.FinishLayout(_T("BaseLayoutDlg m_header"));
		CreateLayouts();
	m_form.FinishLayout(_T("BaseLayoutDlg body"));

	// всем детям по фонту >>
	CWnd *child = GetWindow(GW_CHILD);
	if (child)
	{
		CWnd* last_child = child->GetWindow(GW_HWNDLAST);
		do
		{
			child->SetFont(m_font, FALSE);
			child = child->GetWindow(GW_HWNDNEXT);
		} while(child != NULL && child != last_child);
	}
	// всем детям по фонту <<

	m_form.ShowHide(SW_HIDE);
	m_inited = true;
	SetWindowPos(NULL, 0, 0, m_w + 2, m_h + 2, SWP_NOZORDER | SWP_NOMOVE);
	m_form.ShowHide(SW_NORMAL);

	BOOL res = CDialog::OnInitDialog();

	return res;
}

LRESULT BaseLayoutDlg::OnChangePosition(WPARAM wParam, LPARAM lParam)
{
	WINDOWPOS* wp = (WINDOWPOS*)lParam;
	if (wp->flags == (SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE))
	{
		m_bklinked_color = true;
		m_header.SetActive(!m_bklinked_color);
		m_blink_counter = 0;
		SetTimer(1, 75, NULL);
	};
	return TRUE;
}

void BaseLayoutDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)	
	{
		m_bklinked_color = !m_bklinked_color;
		m_blink_counter++;
		if (m_blink_counter == 5)
			KillTimer(1);
		m_header.SetActive(!m_bklinked_color);
	}
	CDialog::OnTimer(nIDEvent);
}

void BaseLayoutDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CDialog::OnActivate(nState, pWndOther, bMinimized);
	m_header.SetActive(nState != WA_INACTIVE);
}

void BaseLayoutDlg::OnPaint()
{
	CPaintDC pdc(this);
/*
	CRect rect;
	GetClientRect(&rect);
	pdc.SelectStockObject(NULL_BRUSH);
//	pdc.Rectangle(rect);

	for (int i = 0; i < 10; i++)
	{
		rect.right = i * 100;
		pdc.Rectangle(rect);
	}
*/
}

HBRUSH BaseLayoutDlg::OnCtlColor(CDC* pDC, CWnd* wnd, UINT nCtlColor)
{
//	pDC->SetBkColor(0xFFFFFF/*m_bk_color*/);
	//pDC->SetBkMode(TRANSPARENT);
//	pDC->SetTextColor(0);
//	return m_brush;
	return CDialog::OnCtlColor(pDC, wnd, nCtlColor);
}

////////////////////////////////////////////////////////////////////////////////
// end