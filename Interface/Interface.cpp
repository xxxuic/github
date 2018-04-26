// uic 2016.11.09

#include "stdafx.h"
#include "Interface.h"
#include "MutexWrap.h"
#include "./Interface/Dialogs/BaseLayoutDlg.h"

////////////////////////////////////////////////////////////////////////////////
typedef map<int, InterfaceForm*> Forms;
typedef map<int, BmpButton*> Buttons;

////////////////////////////////////////////////////////////////////////////////
CFont *Interface_big_font = NULL, *Interface_small_font = NULL;
InterfaceHeader Interface_header;

static Forms g_forms;
static Buttons g_nav_buttons;
static CMutex g_forms_mutex;

static CRect g_rect(0, 0, 100, 100);
static int g_current_form_num = -1;
static CWnd *g_wnd = NULL;
static DWORD g_exit_msg = 0;

////////////////////////////////////////////////////////////////////////////////
static void MakeFormRect(int cx, int cy)
{
	g_rect = CRect(InterfaceForm::m_gap, InterfaceForm::m_header_h + InterfaceForm::m_gap, cx - InterfaceForm::m_gap, cy - InterfaceForm::m_gap + 1);
}

static void ExitAct(BmpButton* btn)
{
	if (g_wnd && g_exit_msg)
		g_wnd->PostMessage(g_exit_msg, 0, 0);
}

static void SetAppName(CString name)
{
	Interface_header.m_main_text = name;
	Interface_header.Invalidate();
	AfxGetMainWnd()->SetWindowText(name);
}

void OpenFormAct(BmpButton* btn)
{
	Buttons::iterator current_button = g_nav_buttons.begin();
	while (current_button != g_nav_buttons.end())
	{
		if (btn == current_button->second)
		{
			Interface_OpenForm(current_button->first);
			break;
		}
		current_button++;
	}
}

////////////////////////////////////////////////////////////////////////////////
void Interface_AddForm(int num, InterfaceForm* form)
{
	if (g_forms.find(num) != g_forms.end())
#ifdef _DEBUG
		::AfxAssertFailedLine(THIS_FILE, __LINE__);
#else
		::MessageBox(0, THIS_FILE, THIS_FILE, MB_OK);
		ASSERT(false);
#endif
	form->m_id = num;
	g_forms[num] = form;
}

void Interface_Init(CWnd* wnd, DWORD msg, bool with_navigation_buttons, CString main_text, CString add_text)
{
	g_wnd = wnd;
	g_exit_msg = msg;

	LOGFONT lf = {-19, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, RUSSIAN_CHARSET, 
				OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
				FF_DONTCARE, _T("Arial")};

	if (Interface_big_font)
		delete Interface_big_font;
	Interface_big_font = new CFont();
	Interface_big_font->CreateFontIndirect(&lf);

	if (Interface_small_font)
		delete Interface_small_font;
	Interface_small_font = new CFont();
	lf.lfHeight = -11;
	Interface_small_font->CreateFontIndirect(&lf);

	InterfaceHeader::m_big_font = Interface_big_font;
	InterfaceHeader::m_small_font = Interface_small_font;
	//FrameViewer::m_font = Interface_small_font;
	BmpButton::m_font = Interface_big_font;
	InterfaceForm::m_edit_h = 26;
	BaseLayoutDlg::m_font = Interface_big_font;

	int screen_w = GetSystemMetrics(SM_CXSCREEN), screen_h = GetSystemMetrics(SM_CYSCREEN);
	MakeFormRect(screen_w, screen_h);

	Interface_header.m_close_function = ExitAct;
	Interface_header.m_main_text = main_text;
	Interface_header.m_additional_text = add_text;
	Interface_header.m_has_minimize_button = true;
	Interface_header.m_draw_text_shadow = true;
	Interface_header.Create(wnd, 0, 0, screen_w, InterfaceForm::m_header_h, true, false);

	g_wnd->SetWindowText(main_text); // на всякий случай. при открытии формы текст поменяется

	Forms::iterator current_form;
	Buttons::iterator current_button;

	// создаем навигационные кнопки
	if (with_navigation_buttons)
	{
		BmpButton* btn;
		current_form = g_forms.begin();
		while (current_form != g_forms.end())
		{
			btn = new BmpButton;
			btn->Create(OpenFormAct, -1, -1, wnd, -1, 0, 0, 0, 0, WS_CHILD);
			g_nav_buttons[current_form->first] = btn;
			current_form++;
		}
	}

	// инициализируем формы
	InterfaceForm *form;
	current_form = g_forms.begin();
	current_button = g_nav_buttons.begin();
	while (current_form != g_forms.end())
	{
		form = current_form->second;
		if (form && form->m_on_init_function)
		{
			form->m_on_init_function(form);
		}
		if (form && with_navigation_buttons)
		{
			current_button->second->m_text = form->m_btn_name;
			current_button++;
		}
		current_form++;
	}
}

void Interface_Shutdown()
{
	if (Interface_big_font)
	{
		delete Interface_big_font;
		Interface_big_font = NULL;
	}
	if (Interface_small_font)
	{
		delete Interface_small_font;
		Interface_small_font = NULL;
	}
	
	InterfaceForm *form;
	Forms::iterator current_form = g_forms.begin();
	while (current_form != g_forms.end())
	{
		form = current_form->second;
		if (form && form->m_on_shutdown_function)
			form->m_on_shutdown_function(form);
		current_form++;
	}

	Buttons::iterator current_button = g_nav_buttons.begin();
	while (current_button != g_nav_buttons.end())
	{
		if (current_button->second)
			delete current_button->second;
		current_button++;
	}
	g_nav_buttons.clear();
}

void Interface_CloseCurrentForm()
{
	if (g_current_form_num != -1)
	{
		InterfaceForm* form = g_forms[g_current_form_num];
		if (form->m_on_close_function)
			form->m_on_close_function(form);
		form->ShowHide(SW_HIDE, true);
	}
}

void Interface_OpenForm(int num)
{
	InterfaceForm* form = NULL;
	Forms::iterator desired = g_forms.find(num);
	if (desired == g_forms.end() || g_current_form_num == num)
		return;

	Buttons::iterator desired_button;
	desired_button = g_nav_buttons.find(g_current_form_num);
	if (desired_button != g_nav_buttons.end())
		desired_button->second->Enable(true);

	Interface_CloseCurrentForm();

	g_current_form_num = num;

	form = g_forms[g_current_form_num];
	form->ChangePosition(g_rect.left, g_rect.top, g_rect.Width(), g_rect.Height());
	if (form->m_on_open_function)
		form->m_on_open_function(form);
	form->ShowHide(SW_NORMAL);

	SetAppName(form->m_name);

	desired_button = g_nav_buttons.find(g_current_form_num);
	if (desired_button != g_nav_buttons.end())
		desired_button->second->Enable(false);
}

int Interface_GetCurrentFormNum()
{
	return g_current_form_num;
}

void Interface_OnSize(UINT nType, int cx, int cy)
{
	Interface_header.ChangePosition(0, 0, cx, InterfaceForm::m_header_h, SWP_SHOWWINDOW);

	MakeFormRect(cx, cy);
	Forms::iterator desired = g_forms.find(g_current_form_num);
	if (desired != g_forms.end())
		desired->second->ChangePosition(g_rect.left, g_rect.top, g_rect.Width(), g_rect.Height());
}

void Interface_StartTimer(int form_num, int timer_num, DWORD period)
{
	InterfaceForm* form = NULL;

	{
		MutexWrap forms_access(g_forms_mutex);
		Forms::iterator desired = g_forms.find(form_num);
		if (desired == g_forms.end())
			return;
		form = desired->second;
	}

	form->ActivateTimer(timer_num, true);
	g_wnd->SetTimer(timer_num, period, NULL); // ???
}

void Interface_KillTimer(int form_num, int timer_num)
{
	Forms::iterator desired = g_forms.find(form_num);
	if (desired == g_forms.end())
		return;
	InterfaceForm* form = desired->second;
	
	form->ActivateTimer(timer_num, false);
	g_wnd->KillTimer(timer_num); // ???
}

bool Interface_OnTimer(UINT num)
{
	bool is_any = false;
	InterfaceForm *form;
	Forms::iterator current_form = g_forms.begin();
	while (current_form != g_forms.end())
	{
		form = current_form->second;
		is_any = form->OnTimer(num);
		current_form++;
	}
	return is_any;
}

void Interface_LayoutNavButtonsVar1(InterfaceForm &form)
{
	Buttons::iterator current_button = g_nav_buttons.begin(), next_button;
	while (current_button != g_nav_buttons.end())
	{
		next_button = current_button;
		next_button++;
		if (next_button == g_nav_buttons.end())
			form.SingleElementLayout(current_button->second, -1, InterfaceForm::m_button_h, InterfaceForm_NULL_INSETS);
		else
			form.SingleElementLayout(current_button->second, -1, InterfaceForm::m_button_h, InterfaceForm_BOTTOM_INSETS);
		current_button++;
	}
}

void Interface_LayoutNavButtonsVar2(InterfaceForm &form)
{
	int sz = InterfaceForm::m_button_h * 2;
	form.StartEmptyLayout(LAYOUT_orientation_horizontal, -1, sz);
	Buttons::iterator current_button = g_nav_buttons.begin();
	while (current_button != g_nav_buttons.end())
	{
		current_button->second->m_own_font = Interface_small_font;
		if (current_button == g_nav_buttons.begin())
			form.SingleElementLayout(current_button->second, -1, -1, InterfaceForm_NULL_INSETS);
		else
			form.SingleElementLayout(current_button->second, -1, -1, InterfaceForm_LEFT_INSETS);
		current_button++;
	}
	//form.SingleElementLayout((CWnd*)NULL, -1, -1, InterfaceForm_NULL_INSETS);
	form.FinishLayout(_T("nav buttons var2"));
}

////////////////////////////////////////////////////////////////////////////////
// end