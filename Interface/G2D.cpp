// uic 2014.04.24

#include <stdafx.h>
#include "G2D.h"
#include "MutexWrap.h"

////////////////////////////////////////////////////////////////////////////////
// G2D_DC
////////////////////////////////////////////////////////////////////////////////
G2D_DC::G2D_DC():
	m_inited(false), m_current_brush(NULL), m_bk_color(NULL),
	m_on_init_dc_function(NULL),
	m_interface_object(NULL),
	m_selected_font(NULL)
{
}

G2D_DC::~G2D_DC()
{
	Cleanup();
}

void G2D_DC::Cleanup()
{
	if (m_bk_color)
	{
		delete m_bk_color;
		m_bk_color = NULL;
	}

	G2D_Object** obj_ptr = m_objects.GetBuff();
	int i, isz = m_objects.GetFilledSize();
	for (i = 0; i < isz; i++, obj_ptr++)
		delete *obj_ptr;
	m_objects.Clear();
	OnCleanup();
}

void G2D_DC::BeginDraw()
{
	m_objects_mutex.Lock();
	SetIdentityTransform();
	OnBeginDraw();
}

void G2D_DC::EndDraw()
{
	OnEndDraw();
	m_objects_mutex.Unlock();
}

void G2D_DC::SetIdentityTransform()
{
	m_origin_x = 0;
	m_origin_y = 0;
	m_origin_rotation = 0;
	OwnSetIdentityTransform();
}

void G2D_DC::Rotate(float alpha, TYPE xc, TYPE yc)
{
	m_origin_rotation += alpha;
	OwnRotate(alpha, xc ,yc);
}

void G2D_DC::OffsetOrigin(TYPE dx, TYPE dy)
{
	m_origin_x += dx;
	m_origin_y += dy;
	OwnOffsetOrigin(dx, dy);
}

void G2D_DC::RemoveObject(int index)
{
	MutexWrap objects_access(m_objects_mutex);
	int  isz = m_objects.GetFilledSize();
	if (index < isz - 1)
	{
		G2D_Object** obj_ptr = m_objects.GetBuff() + index;
		memmove(obj_ptr, obj_ptr + 1, (isz - 1 - index) * sizeof(G2D_Object*));
	}
	m_objects.SetFilledSize(isz - 1);
}

void G2D_DC::RemoveObject(G2D_Object* obj)
{
	MutexWrap objects_access(m_objects_mutex);
	G2D_Object** obj_ptr = m_objects.GetBuff();
	int i, isz = m_objects.GetFilledSize();
	for (i = 0; i < isz; i++, obj_ptr++)
		if (*obj_ptr == obj)
		{
			RemoveObject(i);
			return;
		}
}

int G2D_DC::AddObject(G2D_Object* obj, CString name)
{
	MutexWrap objects_access(m_objects_mutex);
	obj->m_name = name;
	m_objects.AddValue(obj);
	return m_objects.GetFilledSize() - 1;
}

void G2D_DC::SetObject(G2D_Object* obj, int index)
{
	MutexWrap objects_access(m_objects_mutex);
	if (index < m_objects.GetFilledSize())
		*(m_objects.GetBuff() + index) = obj;
}

G2D_Object* G2D_DC::GetObject(CString name)
{
	MutexWrap objects_access(m_objects_mutex);
	G2D_Object** obj_ptr = m_objects.GetBuff();
	int i, isz = m_objects.GetFilledSize();
	for (i = 0; i < isz; i++, obj_ptr++)
		if ((*obj_ptr)->m_name.Compare(name) == 0)
			return *obj_ptr;
	return NULL;
}

G2D_Color* G2D_DC::CheckColor(CString name, byte r, byte g, byte b, byte alpha)
{
	G2D_Color* res = (G2D_Color*)GetObject(name);
	if (res == NULL)
	{
		res = MakeColor(r, g, b, alpha);
		AddObject(res, name);
	}
	return res;
}

G2D_Brush* G2D_DC::CheckBrush(CString name, G2D_Color* color, float width)
{
	G2D_Brush* res = (G2D_Brush*)GetObject(name);
	if (res == NULL)
	{
		res = MakeBrush(color, width);
		AddObject(res, name);
	}
	return res;
}

G2D_Font* G2D_DC::CheckFont(CString name, WCHAR* font_name, int height, bool bold, bool italic)
{
	G2D_Font* res = (G2D_Font*)GetObject(name);
	if (res == NULL)
	{
		res = MakeFont(font_name, height, bold, italic);
		res->m_name = font_name;
		res->m_height = height;
		res->m_bold = bold;
		res->m_italic = italic;
		AddObject(res, name);
	}
	return res;
}

G2D_Bitmap* G2D_DC::CheckBitmap(CString name, BMP_Bitmap* bmp)
{
	G2D_Bitmap* res = (G2D_Bitmap*)GetObject(name);
	if (res == NULL)
	{
		res = MakeBitmap(bmp);
		AddObject(res, name);
	}
	return res;
}

bool G2D_DC::IsInited()
{
	return m_inited;
}

void G2D_DC::Init(InterfaceObject* interface_object)
{
	if (IsInited())
		return;

	m_interface_object = interface_object;

	OnInit();
}

void G2D_DC::SelectBrush(G2D_Brush* brush)
{
	m_current_brush = brush;
	if (brush)
		brush->OnSelect(this);
}

////////////////////////////////////////////////////////////////////////////////
// end