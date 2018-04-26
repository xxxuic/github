// uic 2013.04.24

#pragma once

#include "InterfaceObject.h"

#include "G2D.h"
//#include <atlstr.h>

struct InterfaceObject2D;
typedef void (*InterfaceObject2D_Function)(InterfaceObject2D*);

////////////////////////////////////////////////////////////////////////////////
// InterfaceObject2D
////////////////////////////////////////////////////////////////////////////////
struct InterfaceObject2D: InterfaceObject
{
	InterfaceObject2D();
	~InterfaceObject2D();

	virtual void Draw(CDC* dc);
	virtual void OnChangeSize();

	void Draw();

	G2D_DC* m_dc;
	G2D_Color *m_subinterface_bg_color, *m_subinterface_text_color;
	G2D_Brush *m_subinterface_text_brush;
	G2D_Font *m_subinterface_font;
	InterfaceObject2D_Function m_draw_function;
};

////////////////////////////////////////////////////////////////////////////////
// end