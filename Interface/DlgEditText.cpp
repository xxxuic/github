// uic 28.10.2008

#include <stdafx.h>
#include "DlgEditText.h"
#ifdef _MNP
#include "MNP.h"
#endif

#define WM_ENFORCE_CLOSE (WM_USER + 1)

CFont* DlgEditText_font = NULL;
DlgEditText_Func DlgEditText_func_before_open = NULL, DlgEditText_func_after_close = NULL;

static HHOOK *g_hook_ptr = NULL;

////////////////////////////////////////////////////////////////////////////////
// My_Edit
////////////////////////////////////////////////////////////////////////////////
struct My_Edit: CEdit
{
	void TranslateClick(DWORD msg, UINT flags, CPoint point)
	{
		m_translate_click = true;
		m_message = msg;
		m_flags = flags;
		m_point = point;
		ClientToScreen(&m_point);
		GetParent()->SendMessage(WM_ENFORCE_CLOSE, IDYES, 0);
	}
	
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point)
	{
		CRect rect;
		GetClientRect(&rect);
		if (rect.PtInRect(point))
			CEdit::OnLButtonDown(nFlags, point);
		else
			TranslateClick(WM_LBUTTONDOWN, nFlags, point);
	}

	afx_msg void OnRButtonDown(UINT nFlags, CPoint point)
	{
		CRect rect;
		GetClientRect(&rect);
		if (rect.PtInRect(point))
			CEdit::OnRButtonDown(nFlags, point);
		else
			TranslateClick(WM_RBUTTONDOWN, nFlags, point);
	}

	afx_msg void OnCaptureChanged(CWnd* new_wnd)
	{
		SetCapture();
	}

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB)
		{
			GetParent()->PostMessage(WM_ENFORCE_CLOSE, IDNO, 0);
			return FALSE;
		}
		return CWnd::PreTranslateMessage(pMsg);
	}

	bool m_translate_click;
	DWORD m_message;
	UINT m_flags;
	CPoint m_point;

protected:
	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(My_Edit, CEdit)
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_CAPTURECHANGED()
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////
// DlgEditText
////////////////////////////////////////////////////////////////////////////////
struct DlgEditText: CDialog
{
	DlgEditText():
		m_inited(false), m_es_styles(ES_CENTER)
	{
	}

	virtual INT_PTR DoModal()
	{
		if (!m_inited)
		{
			DLGTEMPLATE *dlg_template = m_template_data;
			dlg_template->style = WS_POPUP | WS_BORDER;
			dlg_template->dwExtendedStyle = 0;
			dlg_template->cdit = 0;
			dlg_template->x = m_x;
			dlg_template->y = m_y;
			dlg_template->cx = m_w;
			dlg_template->cy = m_h;
			DWORD* ptr = (DWORD*)(m_template_data + 1);
			*ptr = 0;
			ptr++;
			*ptr = 0;
			ptr++;
			*ptr = 0;
			m_inited = (InitModalIndirect(m_template_data, m_parent) == TRUE);
		}
		if (m_inited)
			return CDialog::DoModal();
		return IDCANCEL;
	}

	DLGTEMPLATE m_template_data[2];

	bool m_inited;
	bool m_number_only;
	int m_x, m_y, m_w, m_h;
	My_Edit m_edit;
	CStringA m_text;
	CWnd* m_parent;
	DWORD m_es_styles;

protected:
	int m_res;
	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog()
	{
		m_res = -1;
		CDialog::OnInitDialog();

		SetWindowPos(NULL, m_x, m_y, m_w, m_h, SWP_NOZORDER);

		CRect rect;
		rect.left = 0;
		rect.right = m_w;
		rect.top = 0;
		rect.bottom = m_h;
		m_edit.Create(WS_VISIBLE | (m_number_only?ES_NUMBER:0) | m_es_styles, rect, this, 10);
		//m_edit.SetWindowText(m_text);
		::SetWindowTextA(m_edit.m_hWnd, m_text);
		m_edit.SetFont(DlgEditText_font);

		m_edit.m_translate_click = false;
		m_edit.SetCapture();
		return TRUE;
	}

	LRESULT OnEnforceFlose(WPARAM id, LPARAM)
	{
		if (m_res == -1)
		{
			m_res = id;
			if (id == IDOK)
			{
				//	GetDlgItemText(10, m_text);
				char tmp_str[1024];
				::GetDlgItemTextA(m_hWnd, 10, tmp_str, 1024);
				m_text = tmp_str;
			}
			EndDialog(m_res);
		}
		return 0;
	}
	
	void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
	{
		if (nState != WA_INACTIVE && ::IsWindow(m_edit.m_hWnd))
			m_edit.SetCapture();
	}

	void OnOK()
	{
	}

	void OnCancel()
	{
	}

	int OnCmdMsg(UINT id, int code, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
	{
		if (id == IDOK || id == IDCANCEL)
		{
			if (code == -1)
			{
				SendMessage(WM_ENFORCE_CLOSE, id, NULL);
			}
		}
		//else
		//	TRACE2("ID %d  CODE %d\n", nID, nCode);
		return 1;
	}
};

BEGIN_MESSAGE_MAP(DlgEditText, CDialog)
	ON_WM_ACTIVATE()
	ON_MESSAGE(WM_ENFORCE_CLOSE, OnEnforceFlose)
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////
// процедурки
////////////////////////////////////////////////////////////////////////////////
int DlgEditText_Show(CStringA* text, bool number_only, int x, int y, int w, int h, CWnd* parent, DWORD es_styles)
{
	if (DlgEditText_func_before_open)
		DlgEditText_func_before_open();

	bool with_black = false;

	DlgEditText edit_dlg;
	edit_dlg.m_text = *text;
	edit_dlg.m_number_only = number_only;
	edit_dlg.m_es_styles = es_styles;
	edit_dlg.m_x = x;
	edit_dlg.m_y = y;
	edit_dlg.m_w = w;
	edit_dlg.m_h = h;
	int res;
	{
		edit_dlg.m_parent = parent;
		res = edit_dlg.DoModal();
	}
	if (edit_dlg.m_edit.m_translate_click)
	{
		CPoint pt = edit_dlg.m_edit.m_point;
		//MNP_main_frame->ScreenToClient(&pt);
		CWnd* global_parent = AfxGetMainWnd()->WindowFromPoint(pt);
		if (global_parent)
		{
			pt = edit_dlg.m_edit.m_point;
			global_parent->ScreenToClient(&pt);
			global_parent->PostMessage(edit_dlg.m_edit.m_message, edit_dlg.m_edit.m_flags, (pt.y << 16) | pt.x);
		}
	}
	*text = edit_dlg.m_text;

	if (DlgEditText_func_after_close)
		DlgEditText_func_after_close();

	return res;
}

////////////////////////////////////////////////////////////////////////////////
// end