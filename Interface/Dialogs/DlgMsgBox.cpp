// uic 31.07.2008

#include "stdafx.h"
#include "DlgMsgBox.h"

#include <list>
using namespace std;

static CString g_br = _T("\r\n");

CString
	DlgMsgBox_button_ok_name = _T("OK"),
	DlgMsgBox_button_yes_name = _T("Да"),
	DlgMsgBox_button_no_name = _T("Нет");

DlgMsgBox::DlgMsgBox(CWnd* pParent) : BaseLayoutDlg(_T(""), 500, 250),
	m_type(MB_OK)
{
}

BOOL DlgMsgBox::OnInitDialog() 
{
	CRect rect(0, 0, m_w, m_h);

	DWORD edit_style = ES_AUTOHSCROLL | ES_READONLY | ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL | ES_CENTER;
	DWORD button_style = WS_TABSTOP;

	m_edit_text.Create(edit_style, rect, this, -1);
	m_edit_text.ModifyStyleEx(0, WS_EX_CLIENTEDGE);

	if (m_type == MB_OK)
		m_button_ok.Create(DlgMsgBox_button_ok_name, button_style, rect, this, IDOK);
	else
	{
		m_button_yes.Create(DlgMsgBox_button_yes_name, button_style, rect, this, IDYES);
		m_button_no.Create(DlgMsgBox_button_no_name, button_style, rect, this, IDNO);
	}

	// разбиваем текст на строчки >>
	list<CString> lines;
	int start_pos = 0;
	int pos = m_text.Find(g_br, start_pos);
	while (pos != -1)
	{
		lines.push_back(m_text.Mid(start_pos, pos - start_pos));
		start_pos = pos + 2;
		pos = m_text.Find(g_br, start_pos);
	}
	lines.push_back(m_text.Mid(start_pos));
	m_edit_text.SetWindowText(m_text);
	// разбиваем текст на строчки <<

	// определяем максимальную ширину строки >>
	CClientDC dc(this);
	CFont *font = (m_font != NULL) ? m_font : GetFont();
	dc.SelectObject(font);
	CSize sz;
	sz = dc.GetTextExtent(m_name);

	GetDesktopWindow()->GetClientRect(&rect);
	int max_w = max(400, sz.cx), max_available_w = rect.Width() - 50 - 2*InterfaceForm::m_gap - 6;
	int line_h = sz.cy + 2, add_lines = 0;
	list<CString>::iterator current = lines.begin();
	while (current != lines.end())
	{
		sz = dc.GetTextExtent(*current);
		if (sz.cx > max_available_w)
		{
			while (sz.cx > max_available_w)
			{
				sz.cx -= max_available_w;
				add_lines++;
			}
			sz.cx = max_available_w;
		}
		if (sz.cx > max_w)
			max_w = sz.cx;
		current++;
	}
	// определяем максимальную ширину строки <<

	m_w = max(400, max_w + 25) + 2 * InterfaceForm::m_gap + 20 +2;
	m_h = InterfaceForm::m_header_h + InterfaceForm::m_gap + (lines.size() + add_lines) * line_h + InterfaceForm::m_gap + InterfaceForm::m_button_h + InterfaceForm::m_gap + 2;

	return BaseLayoutDlg::OnInitDialog();
}

BEGIN_MESSAGE_MAP(DlgMsgBox, BaseLayoutDlg)
	ON_BN_CLICKED(IDYES, &DlgMsgBox::OnBnClickedYes)
	ON_BN_CLICKED(IDNO, &DlgMsgBox::OnBnClickedNo)
	ON_BN_CLICKED(IDOK, &DlgMsgBox::OnBnClickedOk)
END_MESSAGE_MAP()

void DlgMsgBox::CreateLayouts()
{
	int buttons_w = 150;

	m_form.StartEmptyLayout(LAYOUT_orientation_vertical, -1, -1, InterfaceForm::m_gap, InterfaceForm::m_gap, InterfaceForm::m_gap, InterfaceForm::m_gap);

		m_form.StartLayout(&m_edit_text, LAYOUT_orientation_horizontal, -1, -1);
		m_form.FinishLayout(_T("m_edit_text"));

		m_form.StartEmptyLayout(LAYOUT_orientation_horizontal, -1, InterfaceForm::m_button_h, 0, 0, InterfaceForm::m_gap, 0);
			if (m_type == MB_OK)
			{
				m_form.StartEmptyLayout(LAYOUT_orientation_horizontal, -1, -1);
				m_form.FinishLayout(_T("left filler m_button_ok"));
				m_form.StartLayout(&m_button_ok, LAYOUT_orientation_horizontal, buttons_w, -1);
				m_form.FinishLayout(_T("m_button_ok"));
				m_form.StartEmptyLayout(LAYOUT_orientation_horizontal, -1, -1);
				m_form.FinishLayout(_T("right filler m_button_ok"));
			}
			else
			{
				m_form.StartLayout(&m_button_no, LAYOUT_orientation_horizontal, buttons_w, -1);
				m_form.FinishLayout(_T("m_button_no"));
				m_form.StartEmptyLayout(LAYOUT_orientation_horizontal, -1, -1);
				m_form.FinishLayout(_T("filler buttons yes no"));
				m_form.StartLayout(&m_button_yes, LAYOUT_orientation_horizontal, buttons_w, -1);
				m_form.FinishLayout(_T("m_button_yes"));
			}
		m_form.FinishLayout(_T("buttons line"));

	m_form.FinishLayout(_T("DlgMsgBox body"));
}

void DlgMsgBox::OnBnClickedYes()
{
	if (m_type == MB_YESNO)
		EndDialog(IDYES);
}

void DlgMsgBox::OnBnClickedNo()
{
	if (m_type == MB_YESNO)
		EndDialog(IDNO);
}

void DlgMsgBox::OnBnClickedOk()
{
	if (m_type == MB_OK)
		EndDialog(IDOK);
}

void DlgMsgBox::OnCancel()
{
	if (m_type == MB_OK)
		EndDialog(IDOK);
	if (m_type == MB_YESNO)
		EndDialog(IDNO);
	if (m_type == MB_YESNOCANCEL)
		EndDialog(IDCANCEL);
}

////////////////////////////////////////////////////////////////////////////////
// DlgMsgBox_DoModal
////////////////////////////////////////////////////////////////////////////////
int DlgMsgBox_DoModal(CString title, CString text, int type)
{
	DlgMsgBox msg_box(AfxGetMainWnd());
	
	msg_box.m_type = type;
	msg_box.m_name = title;
	msg_box.m_text = g_br + text + g_br;
	return msg_box.DoModal();
}

int DlgMsgBox_DoModal(CString title, CString line1, CString line2, int type)
{
	return DlgMsgBox_DoModal(title, line1 + g_br + line2, type);
}

int DlgMsgBox_DoModal(CString title, CString line1, CString line2, CString line3, int type)
{
	return DlgMsgBox_DoModal(title, line1 + g_br + line2 + g_br + line3, type);
}

int DlgMsgBox_DoModal(CString title, CString line1, CString line2, CString line3, CString line4, int type)
{
	return DlgMsgBox_DoModal(title, line1 + g_br + line2 + g_br + line3 + g_br + line4, type);
}

int DlgMsgBox_DoModal(CString title, CString line1, CString line2, CString line3, CString line4, CString line5, int type)
{
	return DlgMsgBox_DoModal(title, line1 + g_br + line2 + g_br + line3 + g_br + line4 + g_br + line5, type);
}

////////////////////////////////////////////////////////////////////////////////
// end