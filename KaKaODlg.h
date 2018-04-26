// uic 2017.12.05

#pragma once

class KaKaO_Dlg : public CDialog
{
public:
	KaKaO_Dlg(CWnd* pParent = NULL);	// standard constructor
	enum { IDD = IDD_KAKAO_DIALOG };

protected:
	HICON m_hIcon;

	afx_msg LRESULT OnCloseApp(WPARAM, LPARAM);
	afx_msg LRESULT OnMainLogAdded(WPARAM, LPARAM);
	
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg	void OnTimer(UINT nIDEvent);
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	
	bool m_inited;
};

////////////////////////////////////////////////////////////////////////////////
// end