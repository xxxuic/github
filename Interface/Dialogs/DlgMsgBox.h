// uic 31.07.2008

#pragma once

#include "BaseLayoutDlg.h"

struct DlgMsgBox : BaseLayoutDlg
{
	DlgMsgBox(CWnd* pParent = NULL);

	int m_type; // MB_OK, MB_YESNO
	CString m_text;

protected:
	virtual void CreateLayouts();

	virtual BOOL OnInitDialog();

	virtual void OnCancel();
	virtual void OnOK() {};

	CEdit m_edit_text;
	CButton m_button_ok, m_button_yes, m_button_no;

	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedYes();
	afx_msg void OnBnClickedNo();
	afx_msg void OnBnClickedOk();
};

int DlgMsgBox_DoModal(CString title, CString text, int type = MB_OK);
int DlgMsgBox_DoModal(CString title, CString line1, CString line2, int type = MB_OK);
int DlgMsgBox_DoModal(CString title, CString line1, CString line2, CString line3, int type = MB_OK);
int DlgMsgBox_DoModal(CString title, CString line1, CString line2, CString line3, CString line4, int type = MB_OK);
int DlgMsgBox_DoModal(CString title, CString line1, CString line2, CString line3, CString line4, CString line5, int type = MB_OK);

extern CString
	DlgMsgBox_button_ok_name,
	DlgMsgBox_button_yes_name,
	DlgMsgBox_button_no_name;

////////////////////////////////////////////////////////////////////////////////
// end