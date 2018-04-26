// uic 25.05.2009

#pragma once

#include <map>

using namespace std;

extern map<int, DWORD> CLR_color_scheme;

#define CLR_MID_BYTE(c1, c2, bn) ((((c1>>(8*bn) & 0xFF) + (c2>>(8*bn) & 0xFF))>>1)<<(8*bn))
#define CLR_MID_COLOR(c1, c2) (CLR_MID_BYTE(c1, c2, 0) | CLR_MID_BYTE(c1, c2, 1) | CLR_MID_BYTE(c1, c2, 2))

DWORD CLR_GetSysColor(int id);
void CLR_GradientFill(CDC* dc, int x, int y, int w, int h, DWORD color_top, DWORD color_bottom, bool horizontal = true);

////////////////////////////////////////////////////////////////////////////////
// end