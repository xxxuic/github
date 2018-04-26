// uic 2016.11.09

#include "InterfaceForm.h"
#include "InterfaceHeader.h"

void Interface_AddForm(int num, InterfaceForm* form);
void Interface_Init(CWnd* wnd, DWORD msg, bool with_navigation_buttons, CString main_text, CString add_text);
void Interface_Shutdown();

void Interface_OpenForm(int num);
void Interface_CloseCurrentForm();
int Interface_GetCurrentFormNum();
void Interface_StartTimer(int form_num, int timer_num, DWORD period);
void Interface_KillTimer(int form_num, int timer_num);

void Interface_OnSize(UINT nType, int cx, int cy);
bool Interface_OnTimer(UINT num);

void Interface_LayoutNavButtonsVar1(InterfaceForm &form);
void Interface_LayoutNavButtonsVar2(InterfaceForm &form);

extern CFont *Interface_big_font, *Interface_small_font;
extern InterfaceHeader Interface_header;

////////////////////////////////////////////////////////////////////////////////
// end