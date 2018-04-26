// uic 2017.12.05

#include "stdafx.h"
#include "KaKaO.h"
#include "KaKaODlg.h"

#include "./MNP.h"

#include "./TXT.h"

#include "./Interface/Interface.h"

#include "./Interface/CLR.h"
#include "./Interface/Forms/ProfilometryForm.h"
#include "./Interface/Forms/LaserSectionForm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

KaKaO_Dlg::KaKaO_Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(KaKaO_Dlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

BEGIN_MESSAGE_MAP(KaKaO_Dlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_MESSAGE(WM_CLOSE_APP, OnCloseApp)
	ON_MESSAGE(WM_MAIN_LOG_ADDED, OnMainLogAdded)
END_MESSAGE_MAP()

void LogInit(CWnd* wnd)
{
	MNP_cfg_file.AssignTo("./KKO.cfg");

	SYSTEMTIME st;
	GetLocalTime(&st);
	CString file_name_prefix = APP_root_path + _T("\\LOG\\") + (TXT("%04d.%02d.%02d.")<<st.wYear<<st.wMonth<<st.wDay);

	CreateDirectory(APP_root_path + _T("\\LOG"), NULL);
	CreateDirectory(APP_root_path + _T("\\TMP"), NULL);
	MNP_tmp_path = APP_root_path + _T("\\TMP");

	LOGFONT lf;
	Interface_small_font->GetLogFont(&lf);
	int cells_h = lf.lfHeight > 0 ? lf.lfHeight : -lf.lfHeight;
	cells_h += 4;

	// создаем логгеры
	KaKaO_log_id = LOGGER_InitLog(file_name_prefix + _T("main.log.txt"), 10000, 1, wnd, WM_MAIN_LOG_ADDED);

	// задаем доступные сообщения
	LOGGER_SetupMessageType(-1, LOG_MAIN, _T(""), LOGGER_both);
	LOGGER_SetupMessageType(-1, LOG_DBG, _T("DBG:"), LOGGER_both);
	LOGGER_SetupMessageType(-1, LOG_ERROR, _T("ERROR:"), LOGGER_both);
	LOGGER_SetupMessageType(-1, LOG_WARNING, _T("WARNING:"), LOGGER_both);

	// создаем интерфей2сные объекты
//	KaKaO_log_list.Create(wnd, 0, 0, 1000, 100, true, false, 0x000000, false, 0, WS_VISIBLE);
	KaKaO_log_list.m_report_mouse_move_when_leave = true;
	KaKaO_log_list.m_log_id = KaKaO_log_id;
	KaKaO_log_list.m_own_font = Interface_small_font;
	KaKaO_log_list.SetCellsHeight(cells_h);
	KaKaO_log_list.SetColorForMessageType(LOG_ERROR, 0x0000FF);
	KaKaO_log_list.SetColorForMessageType(LOG_WARNING, 0x0080FF);
	KaKaO_log_list.SetColorForMessageType(LOG_DBG, 0x555555);

	LOGGER_AddMessage(KaKaO_log_id, LOG_DBG, _T("Начало работы программы"), true, true);
}

BOOL KaKaO_Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	// color scheme
	CLR_color_scheme[COLOR_BTNFACE] = CLR_color_scheme[COLOR_3DFACE] = 0xF0F0F0;
	CLR_color_scheme[COLOR_BTNSHADOW] = CLR_color_scheme[COLOR_3DSHADOW] = 0xA6B4B9;
	CLR_color_scheme[COLOR_BTNHIGHLIGHT] = CLR_color_scheme[COLOR_3DHIGHLIGHT] = 0xFFFFFF;
	CLR_color_scheme[COLOR_3DDKSHADOW] = 0x909699;

	// forms setup
	ProfilometryForm_Setup(PROFILOMETRY_FOMR_ID, this);
	LaserSectionForm_Setup(LASER_SECTION_FOMR_ID, this);
	//...

	// interface init
	KaKaO_log_list.Create(this, 0, 0, 1000, 100, true, false, 0x000000, false, 0, WS_VISIBLE);
	Interface_Init(this, WM_CLOSE_APP, true, _T("KaKaO"), _T("Система контроля качества\r\nоболочек ТВЭЛ"));
	LogInit(this);
/*
	UseDll_SetMessageСallback(HardMessageCallback);

	if (!UseDllCUDA_Init())
		LOGGER_AddMessage(MII4_main_log_id, LOG_ERROR, _T("Ошибка инициализации CUDA"), true, true);
*/
	m_inited = true;
/*
	HARD_SetMessageСallback(HardMessageCallback);
	HARD_Init(this, WM_HARD_CHANGES);
*/

	ShowWindow(SW_MAXIMIZE | SW_NORMAL);

	//Interface_OpenForm(PROFILOMETRY_FOMR_ID);
	Interface_OpenForm(LASER_SECTION_FOMR_ID);

	return TRUE;
}

void KaKaO_Dlg::OnSize(UINT nType, int cx, int cy)
{
	if (!m_inited)
		return;

	Interface_OnSize(nType, cx, cy);
}

void KaKaO_Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR KaKaO_Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT KaKaO_Dlg::OnMainLogAdded(WPARAM, LPARAM)
{
	if (m_inited)
		KaKaO_log_list.Invalidate();
	return +1;
}

LRESULT KaKaO_Dlg::OnCloseApp(WPARAM, LPARAM)
{
	Interface_CloseCurrentForm();

	m_inited = false;
	//HARD_ShutDown();

	LOGGER_AddMessage(KaKaO_log_id, LOG_DBG, _T("Конец работы программы"), true, true);
	LOGGER_ShutDownLog(KaKaO_log_id);

	Interface_Shutdown();
/*
	UseDllCUDA_Shutdown3D();
	UseDllCUDA_ShutdownCalcDispersion();
	UseDllCUDA_ShutdownCorrelogram();
	UseDllCUDA_Shutdown();
*/
	EndDialog(IDCANCEL);
	return +1;
}

void KaKaO_Dlg::OnTimer(UINT nIDEvent)
{
	Interface_OnTimer(nIDEvent);
}

////////////////////////////////////////////////////////////////////////////////
// end