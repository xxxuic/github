// uic 2013.04.24

#include <stdafx.h>

#include "InterfaceObject2D.h"

#ifdef _D2D
#include "G2D_D2D.h"
#else
#include "G2D_GDI.h"
#endif

////////////////////////////////////////////////////////////////////////////////
// InterfaceObject2D
////////////////////////////////////////////////////////////////////////////////
InterfaceObject2D::InterfaceObject2D(): InterfaceObject(),
	m_subinterface_bg_color(NULL), m_subinterface_text_color(NULL), m_subinterface_text_brush(NULL), m_subinterface_font(NULL)
{
#ifdef _D2D
	m_dc = new G2D_D2D_DC;
	m_use_mem_dc = false;
#else
	m_dc = new G2D_GDI_DC;
#endif
}

InterfaceObject2D::~InterfaceObject2D()
{
	if (m_dc)
		delete m_dc;
}

void InterfaceObject2D::Draw(CDC* dc)
{
	Draw();
}

void InterfaceObject2D::OnChangeSize()
{
	if (!m_dc->IsInited())
		m_dc->Init(this);
	else
		m_dc->OnResize();
}

void InterfaceObject2D::Draw()
{
	if (m_dc->IsInited())
	{
		m_dc->BeginDraw();

		if (m_draw_function)
			m_draw_function(this);

		m_dc->EndDraw();
	}
}

////////////////////////////////////////////////////////////////////////////////
// end