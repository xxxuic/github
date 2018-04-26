// uic 25.05.2009

#include <stdafx.h>
#include "CLR.h"

map<int, DWORD> CLR_color_scheme;
static map<int, DWORD>::iterator g_desired_color;

struct CLR_Initializer
{
	CLR_Initializer()
	{
		CLR_color_scheme[COLOR_WINDOWFRAME] = 0x0;
		CLR_color_scheme[COLOR_3DHIGHLIGHT] = 0xFFFFFF;
		CLR_color_scheme[COLOR_3DFACE] = 0xC8D0D4;
		CLR_color_scheme[COLOR_3DSHADOW] = 0x808080;
		CLR_color_scheme[COLOR_3DDKSHADOW] = 0x404040;
		
		CLR_color_scheme[COLOR_INFOBK] = 0xE1FFFF;
		CLR_color_scheme[COLOR_INFOTEXT] = 0x0;

		CLR_color_scheme[COLOR_HIGHLIGHT] = 0x6A240A;
		CLR_color_scheme[COLOR_HIGHLIGHTTEXT] = 0xFFFFFF;
	}
};

static CLR_Initializer g_initializer;

DWORD CLR_GetSysColor(int id)
{
	g_desired_color = CLR_color_scheme.find(id);
	if (g_desired_color == CLR_color_scheme.end())
		return GetSysColor(id);
	return g_desired_color->second;
}

void CLR_GradientFill(CDC* dc, int x, int y, int w, int h, DWORD color_top, DWORD color_bottom, bool horizontal)
{
	DWORD color;
	byte col[6];
	col[0] = color_top & 0xFF;
	col[1] = (color_top>>8) & 0xFF;
	col[2] = (color_top>>16) & 0xFF;
	col[3] = color_bottom & 0xFF;
	col[4] = (color_bottom>>8) & 0xFF;
	col[5] = (color_bottom>>16) & 0xFF;
	double k1, k2;
	int i, max_i = horizontal ? h : w;
	for (i = 0; i <= max_i; i++)
	{
		k2 = double(i) / max_i;
		k1 = 1 - k2;
		color = byte(k1 * col[0] + k2 * col[3]) | (byte(k1 * col[1] + k2 * col[4])<<8) | (byte(k1 * col[2] + k2 * col[5])<<16);
		if (horizontal)
			dc->FillSolidRect(x, y + i, w, 1, color);
		else
			dc->FillSolidRect(x + i, y, 1, h, color);
	}
}

////////////////////////////////////////////////////////////////////////////////
// end