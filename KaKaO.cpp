// uic 2017.12.05

#include "stdafx.h"
#include "KaKaO.h"
#include "KaKaODlg.h"

#include "MutexWrap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(KaKaO_App, CWinApp)
END_MESSAGE_MAP()

BOOL KaKaO_App::InitInstance()
{
	CWinApp::InitInstance();

	MutexWrap_Init();
	
	//SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	TCHAR buff[2048];
	GetCurrentDirectory(2048, buff);
	buff[2047] = '\0';
	APP_root_path = buff;


	KaKaO_Dlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();
	return FALSE;
}

KaKaO_App KaKaO_app;

CString APP_root_path;

LOGGER_Id KaKaO_log_id;
LogListControl KaKaO_log_list;
int KaKaO_log_list_h = 92;

////////////////////////////////////////////////////////////////////////////////
// end