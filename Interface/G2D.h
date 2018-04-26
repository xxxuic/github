// uic 2014.04.24

#pragma once;

#include "InterfaceObject.h"
#include "BMP.h"
#include "ElasticBuffer.h"

#include <afxmt.h>

struct G2D_DC;

#define TYPE float

////////////////////////////////////////////////////////////////////////////////
// G2D_Object
////////////////////////////////////////////////////////////////////////////////
struct G2D_Object
{
	G2D_Object() {}
	virtual ~G2D_Object() {}

	CString m_name;
	virtual void OnRestore(G2D_DC*dc) {}
};

////////////////////////////////////////////////////////////////////////////////
// G2D_Color
////////////////////////////////////////////////////////////////////////////////
struct G2D_Color : G2D_Object
{
	G2D_Color() {}
	virtual ~G2D_Color() {}

	byte m_r, m_g, m_b, m_alpha;
};

////////////////////////////////////////////////////////////////////////////////
// G2D_Brush
////////////////////////////////////////////////////////////////////////////////
struct G2D_Brush : G2D_Object
{
	G2D_Brush(G2D_Color* col) { if (col) m_color = *col; }
	virtual ~G2D_Brush() {}

	virtual void OnSelect(G2D_DC* dc) = 0;

	G2D_Color* GetColor() { return &m_color; }
	G2D_Color m_color;
	float m_width;
};

////////////////////////////////////////////////////////////////////////////////
// G2D_Bitmap
////////////////////////////////////////////////////////////////////////////////
struct G2D_Bitmap : G2D_Object
{
	G2D_Bitmap(): m_width(0), m_height(0) {};
	virtual ~G2D_Bitmap() {}

	int m_width, m_height;
};

////////////////////////////////////////////////////////////////////////////////
// G2D_Font
////////////////////////////////////////////////////////////////////////////////
struct G2D_Font : G2D_Object
{
	G2D_Font(): m_height(-1) {}
	virtual ~G2D_Font() {}

	int m_height;
	bool m_bold, m_italic;
	CStringW m_name;
};

////////////////////////////////////////////////////////////////////////////////
// G2D_DC
////////////////////////////////////////////////////////////////////////////////
struct G2D_DC
{
	G2D_DC();
	virtual ~G2D_DC();

	bool m_inited;
	InterfaceObject* m_interface_object;
	InterfaceObject_GlobalFunctionWithParam m_on_init_dc_function;

	void Init(InterfaceObject* m_interface_object);
	virtual void OnInit() = 0;
	void Cleanup();
	virtual void OnCleanup() {};
	virtual void OnResize() {};
	bool IsInited();

	void BeginDraw();
	void EndDraw();
	virtual void OnBeginDraw() {}
	virtual void OnEndDraw() {}

	TYPE m_origin_x, m_origin_y;
	float m_origin_rotation;
	void SetIdentityTransform();
	void Rotate(float alpha, TYPE xc, TYPE yc);
	void OffsetOrigin(TYPE dx, TYPE dy);
	virtual void OwnSetIdentityTransform() = 0;
	virtual void OwnRotate(float alpha, TYPE xc, TYPE yc) = 0;
	virtual void OwnOffsetOrigin(TYPE dx, TYPE dy) = 0;

	virtual void ClipRect(int x, int y, int w, int h) = 0;
	virtual void UnClip() = 0;

	void SelectFont(G2D_Font* font) { m_selected_font = font; OnSelectFont(font); }
	virtual void OnSelectFont(G2D_Font* font) = 0;
	G2D_Font* m_selected_font;

	G2D_Color* CheckColor(CString name, byte r, byte g, byte b, byte alpha = 255);
	G2D_Brush* CheckBrush(CString name, G2D_Color* color, float width = 1);
	G2D_Font* CheckFont(CString name, WCHAR* font_name, int height, bool bold, bool italic);
	G2D_Bitmap* CheckBitmap(CString name, BMP_Bitmap* bmp);

	ElasticBuffer<G2D_Object*> m_objects;
	CMutex m_objects_mutex;

	G2D_Object* GetObject(CString name);
	int AddObject(G2D_Object* obj, CString name);
	void SetObject(G2D_Object* obj, int index);
	void RemoveObject(int index);
	void RemoveObject(G2D_Object* obj);

	G2D_Color *m_bk_color;
	G2D_Brush *m_current_brush;
	void SelectBrush(G2D_Brush* brush);

	virtual void DrawBitmap(TYPE x, TYPE y, G2D_Bitmap* bm, float alpha = 1, TYPE to_w = 0, TYPE to_h = 0, CRect* from_rect = NULL, bool smooth = true) = 0;

	virtual void DrawRect(TYPE x, TYPE y, TYPE w, TYPE h) = 0;
	virtual void FillRect(TYPE x, TYPE y, TYPE w, TYPE h, G2D_Color* col) = 0;
	virtual void FillRect(TYPE x, TYPE y, TYPE w, TYPE h, G2D_Brush* brush) = 0;

	virtual void DrawEllipse(TYPE x, TYPE y, TYPE w, TYPE h) = 0;
	virtual void FillEllipse(TYPE x, TYPE y, TYPE w, TYPE h, G2D_Color* col) = 0;
	virtual void FillEllipse(TYPE x, TYPE y, TYPE w, TYPE h, G2D_Brush* brush) = 0;

	virtual void MoveTo(TYPE x, TYPE y) = 0;
	virtual void LineTo(TYPE x, TYPE y) = 0;
	virtual void DrawText(TYPE x, TYPE y, WCHAR* text, int xalign = +1, int yalign = +1) = 0;
	virtual void DrawTextWithRect(TYPE x, TYPE y, TYPE dw, TYPE dh, WCHAR* text, G2D_Color* fill_color, G2D_Brush* stroke_brush, int xalign = +1, int yalign = +1) = 0;
	virtual bool GetTextExtent(WCHAR* text, float &w, float &h, int len = -1) = 0;

	virtual G2D_Bitmap* MakeBitmap(BMP_Bitmap* bmp) = 0;
	virtual G2D_Color* MakeColor(byte r, byte g, byte b, byte alpha) = 0;
	virtual G2D_Brush* MakeBrush(G2D_Color* color, float width) = 0;
	virtual G2D_Font* MakeFont(WCHAR* name, int height, bool bold, bool italic) = 0;

};

////////////////////////////////////////////////////////////////////////////////
// end