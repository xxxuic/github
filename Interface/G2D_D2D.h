// uic 2014.04.24

#pragma once;

#include "G2D.h"

#include "D2D1.h"
#include "D3D11.h"
#include "DWrite.h"
#include "D2D1Helper.h"

using namespace D2D1;

struct G2D_D2D_DC;

////////////////////////////////////////////////////////////////////////////////
// G2D_D2D_Color
////////////////////////////////////////////////////////////////////////////////
struct G2D_D2D_Color : G2D_Color
{
	G2D_D2D_Color():
		m_color(D2D1::ColorF(0, 0, 0, 1))
	{
	}

	D2D1::ColorF m_color;
};

////////////////////////////////////////////////////////////////////////////////
// G2D_D2D_Brush
////////////////////////////////////////////////////////////////////////////////
struct G2D_D2D_Brush : G2D_Brush
{
	G2D_D2D_Brush(G2D_Color* col = NULL);
	~G2D_D2D_Brush();

	virtual void OnSelect(G2D_DC* dc);
	ID2D1SolidColorBrush* m_brush;
};

////////////////////////////////////////////////////////////////////////////////
// G2D_D2D_Brush
////////////////////////////////////////////////////////////////////////////////
struct G2D_D2D_Bitmap : G2D_Bitmap
{
	G2D_D2D_Bitmap();
	virtual ~G2D_D2D_Bitmap();

	bool Make(G2D_D2D_DC*dc, BMP_Bitmap* bmp);

	ID2D1Bitmap* m_bitmap;
	BMP_Bitmap* m_bmp_bitmap;
};

////////////////////////////////////////////////////////////////////////////////
// G2D_D2D_Font
////////////////////////////////////////////////////////////////////////////////
struct G2D_D2D_Font : G2D_Font
{
	G2D_D2D_Font();
	virtual ~G2D_D2D_Font();

	bool Make(G2D_D2D_DC*dc, WCHAR* name, int height, bool bold, bool italic);

	IDWriteTextFormat *m_text_format;
};

////////////////////////////////////////////////////////////////////////////////
// G2D_D2D_DC
////////////////////////////////////////////////////////////////////////////////
struct G2D_D2D_DC : G2D_DC
{
	G2D_D2D_DC();
	virtual ~G2D_D2D_DC();

	virtual void OnCleanup();
	virtual void OnInit();
	virtual void OnResize();

//	virtual void OnRestore();

	virtual void OnBeginDraw();
	virtual void OnEndDraw();

	virtual void ClipRect(int x, int y, int w, int h);
	virtual void UnClip();

	virtual void OwnSetIdentityTransform();
	virtual void OwnOffsetOrigin(TYPE dx, TYPE dy);
	virtual void OwnRotate(float alpha, TYPE xc, TYPE yc);

	virtual void OnSelectFont(G2D_Font* font);//WCHAR* name, int sz);

	virtual G2D_Brush* MakeBrush(G2D_Color* color, float width);
	virtual G2D_Color* MakeColor(byte r, byte g, byte b, byte alpha);
	virtual G2D_Bitmap* MakeBitmap(BMP_Bitmap* bmp);
	virtual G2D_Font* MakeFont(WCHAR* name, int height, bool bold, bool italic);

	virtual void DrawBitmap(TYPE x, TYPE y, G2D_Bitmap* bm, float alpha = 1, TYPE to_w = 0, TYPE to_h = 0, CRect* from_rect = NULL, bool smooth = true);

	virtual void DrawRect(TYPE x, TYPE y, TYPE w, TYPE h);
	virtual void FillRect(TYPE x, TYPE y, TYPE w, TYPE h, G2D_Color* col);
	virtual void FillRect(TYPE x, TYPE y, TYPE w, TYPE h, G2D_Brush* brush);

	virtual void DrawEllipse(TYPE x, TYPE y, TYPE w, TYPE h);
	virtual void FillEllipse(TYPE x, TYPE y, TYPE w, TYPE h, G2D_Color* col);
	virtual void FillEllipse(TYPE x, TYPE y, TYPE w, TYPE h, G2D_Brush* brush);

	virtual void MoveTo(TYPE x, TYPE y);
	virtual void LineTo(TYPE x, TYPE y);
	virtual void DrawText(TYPE x, TYPE y, WCHAR* text, int xalign = +1, int yalign = +1);
	virtual void DrawTextWithRect(TYPE x, TYPE y, TYPE dw, TYPE dh, WCHAR* text, G2D_Color* fill_color, G2D_Brush* stroke_brush, int xalign = +1, int yalign = +1);
	virtual bool GetTextExtent(WCHAR* text, float &w, float &h, int len = -1);

	G2D_D2D_Brush *m_tmp_brush;

	float m_current_line_width;
	D2D1_POINT_2F m_current_draw_position;
	ID2D1HwndRenderTarget *m_render_target;
	ElasticBuffer<ID2D1Layer*> m_layers;

	IDWriteFactory *m_dwrite_factory;
	IDWriteTextFormat *m_selected_text_format;
	
	D2D1_RENDER_TARGET_PROPERTIES m_rt_props;
};

////////////////////////////////////////////////////////////////////////////////
// end