// uic 24.10.2008

#pragma once

int DlgEditText_Show(CStringA* text, bool number_only, int x, int y, int w, int h, CWnd* parent, DWORD es_styles = ES_CENTER);
extern CFont* DlgEditText_font;

typedef void (*DlgEditText_Func)(void);
extern DlgEditText_Func DlgEditText_func_before_open, DlgEditText_func_after_close;

////////////////////////////////////////////////////////////////////////////////
// end