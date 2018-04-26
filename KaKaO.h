// uic 2017.12.05

#pragma once

#define PROFILOMETRY_FOMR_ID 1
#define LASER_SECTION_FOMR_ID 2

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"

#include "./Interface/LOGGER.h"
#include "./Interface/LogListControl.h"

#define LOG_MAIN 0
#define LOG_DBG 1
#define LOG_ERROR 2
#define LOG_WARNING 3

#define WM_CLOSE_APP (WM_USER + 2)
#define WM_MAIN_LOG_ADDED (WM_USER + 10)

class KaKaO_App : public CWinApp
{
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

extern KaKaO_App KaKaO_app;

extern CString APP_root_path;

extern LOGGER_Id KaKaO_log_id;
extern int KaKaO_log_list_h;
extern LogListControl KaKaO_log_list;

////////////////////////////////////////////////////////////////////////////////
// end