// uic 2014.04.28

#include <stdafx.h>

#include "PictureViewer2D.h"

#include "MutexWrap.h"

#include <math.h>

float PictureViewer2D_angle = 0.0f, PictureViewer2D_x_scale = 1.0f, PictureViewer2D_y_scale = 1.0f;

////////////////////////////////////////////////////////////////////////////////
// PictureViewer2D
////////////////////////////////////////////////////////////////////////////////
static TCHAR g_pv2_white_color_name[] = _T("pv2_white_color"),
			g_pv2_white_brush_name[] = _T("pv2_white_brush"),
			g_pv2_black_color_name[] = _T("pv2_black_color"),
			g_pv2_black_brush_name[] = _T("pv2_black_brush"),
			g_pv2_frame_color_name[] = _T("pv2_frame_color"),
			g_pv2_frame_brush_name[] = _T("pv2_frame_brush"),
			g_pv2_font_name[] = _T("pv2_font");

void PictureViewer2D_InitObjects(InterfaceObject* obj)
{
	PictureViewer2D* pv2d = (PictureViewer2D*) obj;
	G2D_DC* dc = pv2d->m_dc;

	pv2d->m_color_white = dc->CheckColor(g_pv2_white_color_name, 255, 255, 255);
	pv2d->m_color_black = dc->CheckColor(g_pv2_black_color_name, 0, 0, 0);
	pv2d->m_color_frame = dc->CheckColor(g_pv2_frame_color_name, pv2d->m_picture_frame_color_r255, pv2d->m_picture_frame_color_g255, pv2d->m_picture_frame_color_b255);

	pv2d->m_brush_white = dc->CheckBrush(g_pv2_white_brush_name, pv2d->m_color_white, 1);
	pv2d->m_brush_black = dc->CheckBrush(g_pv2_black_brush_name, pv2d->m_color_black, 1);
	pv2d->m_brush_frame = dc->CheckBrush(g_pv2_frame_brush_name, pv2d->m_color_frame, pv2d->m_picture_frame_width);
	
	pv2d->m_font1 = dc->CheckFont(g_pv2_font_name, L"Calibri", 15, false, false);

	if (pv2d->m_init_2d_function)
		pv2d->m_init_2d_function(pv2d);

	pv2d->OnSetBitmap();
}

void PictureViewer2D::CorrectPicturePosition()
{
	return; // tmp

	// центр вьюера должен быть быть с картинкой
	CRect rect = GetWindowRect();
	float rh_w = 0.5 * rect.Width(), rh_h = 0.5 * rect.Height(); // rect half width and height
	float p_w = m_current_scale * m_picture_width, p_h = m_current_scale * m_picture_height;
	float dx = 0, dy = 0;
	if (m_current_picture_x > rh_w)
		dx = rh_w - m_current_picture_x;
	else
	if (m_current_picture_x + p_w < rh_w)
		dx = rh_w - (m_current_picture_x + p_w);
	if (m_current_picture_y > rh_h)
		dy = rh_h - m_current_picture_y;
	else
	if (m_current_picture_y + p_h < rh_h)
		dy = rh_h - (m_current_picture_y + p_h);
	if (dx !=0 || dy != 0)
	{
		m_current_picture_x += dx;
		m_current_picture_y += dy;
		if (m_pressed)
		{
			m_pressed_picture_x = m_current_picture_x;
			m_pressed_picture_y = m_current_picture_y;
			m_idle_picture_x += dx * m_idle_scale / m_current_scale;
			m_idle_picture_y += dx * m_idle_scale / m_current_scale;
		}
		else
		{
			m_idle_picture_x = m_current_picture_x;
			m_idle_picture_y = m_current_picture_y;
			m_pressed_picture_x = dx * m_pressed_scale / m_current_scale;
			m_pressed_picture_y = dy * m_pressed_scale / m_current_scale;
		}
	}
}

void PictureViewer2D_DrawPicture(InterfaceObject2D* obj2d)
{
	PictureViewer2D* pv2d = (PictureViewer2D*) obj2d;
	G2D_DC* dc = obj2d->m_dc;
	if (dc == NULL || !dc->m_inited)
		return;
	double scale = pv2d->GetCurrentScale();
	CRect window_rect = pv2d->GetWindowRect();
	//MutexWrap objects_access(pv->m_objects_mutex);
	G2D_Bitmap *g2d_bmp = pv2d->GetBitmap2D();

	pv2d->CorrectPicturePosition();
	float x = pv2d->GetCurrentPictureX(), y = pv2d->GetCurrentPictureY(), w = pv2d->GetPictureW() * scale, h = pv2d->GetPictureH() * scale;
	x = int(x);
	y = int(y);

	if (pv2d->m_font1)
		dc->SelectFont(pv2d->m_font1);

	if (g2d_bmp)
	{
		// рисуем обводку, если есть
		float frame_width = pv2d->m_picture_frame_width;
		if (frame_width > 0)
		{
			dc->SelectBrush(pv2d->m_brush_frame);
			dc->DrawRect(x - frame_width, y - frame_width, w + 2 * frame_width, h + 2 * frame_width);
		}

		dc->DrawBitmap(x, y, g2d_bmp, 1, w, h, NULL, scale < 1);
	}
	
	PictureViewer2D_BitmapWrap** wrap_ptr = pv2d->m_bmp_wraps.GetBuff();
	int i, isz = pv2d->m_bmp_wraps.GetFilledSize();
	float alpha = 0.6f, dx, dy, center_x = x + 0.5f * w, center_y = y + 0.5f * h, aw, ah;
	for (i = 0; i < isz; i++, wrap_ptr++)
	{
		if ((*wrap_ptr)->m_bmp_2d)
		{
			//alpha *= 0.7;
			dx = cos(PictureViewer2D_angle) * (*wrap_ptr)->m_offset_x * PictureViewer2D_x_scale + sin(PictureViewer2D_angle) * (*wrap_ptr)->m_offset_y * PictureViewer2D_y_scale;
			dy = -sin(PictureViewer2D_angle) * (*wrap_ptr)->m_offset_x * PictureViewer2D_x_scale + cos(PictureViewer2D_angle) * (*wrap_ptr)->m_offset_y * PictureViewer2D_y_scale;
			aw = (*wrap_ptr)->m_w;
			ah = (*wrap_ptr)->m_h;
			dc->DrawBitmap(center_x + (dx - 0.5 *aw) * scale, center_y + (dy - 0.5 * ah) * scale, (*wrap_ptr)->m_bmp_2d, alpha, aw * scale, aw * scale, NULL, scale < 1);
		}
	}

	if (pv2d->m_after_draw_2d_function)
		pv2d->m_after_draw_2d_function(pv2d);

	// рисование текста, и пр.
	if (pv2d->m_zoom_info_is_visible)
	{
		int x = pv2d->m_zoom_info_x, y = pv2d->m_zoom_info_y;
		
		dc->SelectBrush(pv2d->m_brush_black);
		CStringW str;
		if (scale < 2.0 && scale != 1.0)
			str.Format(L"Zoom x%.2f", scale);
		else
		if (scale < 5.0)
			str.Format(L"Zoom x%.1f", scale);
		else
			str.Format(L"Zoom x%.0f", scale);
			
		float text_w, text_h;
		pv2d->m_dc->GetTextExtent(str.GetBuffer(), text_w, text_h);
		text_w = int(text_w) + 4;
		text_h = int(text_h);

		if (x < 0)
			x = window_rect.Width() - text_w + x;
		if (y < 0)
			y = window_rect.Height() - text_h + y;

		dc->FillRect(x, y, text_w, text_h, pv2d->m_brush_white);
		dc->DrawText(x + 2, y, str.GetBuffer(), +1, +1);
	}
}

void PictureViewer2D::OnCleanup()
{
	m_bmp_2d = NULL;
	m_bmp_obj_index = -1;
	
	MultiNull2D();
}

void PictureViewer2D::MultiDelete2D()
{
	int i, isz = m_bmp_wraps.GetFilledSize();
	PictureViewer2D_BitmapWrap** bmp_wrap_ptr = m_bmp_wraps.GetBuff();
	for (i = 0; i < isz; i++, bmp_wrap_ptr++)
		if ((*bmp_wrap_ptr)->m_bmp_2d)
			delete (*bmp_wrap_ptr)->m_bmp_2d;
}

void PictureViewer2D::MultiDeleteBmp()
{
	int i, isz = m_bmp_wraps.GetFilledSize();
	PictureViewer2D_BitmapWrap** bmp_wrap_ptr = m_bmp_wraps.GetBuff();
	for (i = 0; i < isz; i++, bmp_wrap_ptr++)
		if ((*bmp_wrap_ptr)->m_bmp_bitmap)
		{
			delete (*bmp_wrap_ptr)->m_bmp_bitmap;
			(*bmp_wrap_ptr)->m_bmp_bitmap = NULL;
		}
}

void PictureViewer2D::MultiNull2D()
{
	int i, isz = m_bmp_wraps.GetFilledSize();
	PictureViewer2D_BitmapWrap** bmp_wrap_ptr = m_bmp_wraps.GetBuff();
	for (i = 0; i < isz; i++, bmp_wrap_ptr++)
	{
		(*bmp_wrap_ptr)->m_bmp_2d = NULL;
		(*bmp_wrap_ptr)->m_bmp_obj_index = -1;
	}
}

void PictureViewer2D::Cleanup()
{
	if (m_dc)
		m_dc->Cleanup();
	else
	{
		if (m_bmp_2d)
			delete m_bmp_2d;
		
		MultiDelete2D();
	}

	if (m_bmp_bitmap)
	{
		delete m_bmp_bitmap;
		m_bmp_bitmap = NULL;
	}
	MultiDeleteBmp();

	OnCleanup();
}

PictureViewer2D::PictureViewer2D():
	m_after_draw_2d_function(NULL), m_init_2d_function(NULL), m_on_mouse_function(NULL),
	m_bmp_2d(NULL), m_bmp_bitmap(NULL),
	m_font1(NULL), m_brush_white(NULL), m_brush_black(NULL), m_brush_frame(NULL),
	m_pressed(false), m_dragging(false), m_pressed_availale(true),
	m_color_white(NULL), m_color_black(NULL),
	m_picture_frame_width(0),
	m_picture_frame_color_r255(0), m_picture_frame_color_g255(0), m_picture_frame_color_b255(0),
	m_show_zoom_info(true), m_zoom_info_is_visible(false), m_zoom_info_x(5), m_zoom_info_y(5),
	m_prev_picture_width(INT_MIN), m_prev_picture_height(INT_MIN),
	m_bmp_obj_index(-1),
	m_min_scale(-1), m_max_scale(-1), m_scale_change_koeff(1.2), m_scale_change_offset(0),
	m_default_scale(1.0),
	m_invalidate_on_mouse_events(true)
{
	m_capture_focus = true;
	m_draw_function = PictureViewer2D_DrawPicture;
	m_dc->m_on_init_dc_function = PictureViewer2D_InitObjects;

	SetScales(1.0f, 3.0f);

	SetMessageHandler(WM_MOUSEMOVE, (InterfaceObject_OwnFunction)(&PictureViewer2D::OnMouseMove));
	SetMessageHandler(WM_LBUTTONDOWN, (InterfaceObject_OwnFunction)(&PictureViewer2D::OnLBDown));
	SetMessageHandler(WM_LBUTTONUP, (InterfaceObject_OwnFunction)(&PictureViewer2D::OnLBUp));
	SetMessageHandler(WM_MBUTTONDOWN, (InterfaceObject_OwnFunction)(&PictureViewer2D::OnMBDown));
	SetMessageHandler(WM_MBUTTONUP, (InterfaceObject_OwnFunction)(&PictureViewer2D::OnOtherMouse));
	SetMessageHandler(WM_RBUTTONDOWN, (InterfaceObject_OwnFunction)(&PictureViewer2D::OnOtherMouse));
	SetMessageHandler(WM_RBUTTONUP, (InterfaceObject_OwnFunction)(&PictureViewer2D::OnOtherMouse));
	SetMessageHandler(WM_MOUSEWHEEL, (InterfaceObject_OwnFunction)(&PictureViewer2D::OnWheel));

	ClearPalette();
}

PictureViewer2D::~PictureViewer2D()
{
	if (m_bmp_bitmap)
		delete m_bmp_bitmap;

	ElasticBuffer_DeletePointerBuffer(m_bmp_wraps);
}

void PictureViewer2D::OnWheel()
{
	double *scale_ptr = m_pressed ? &m_pressed_scale : &m_idle_scale;
	float constant_px, constant_py;
	PictureViewer2D_Mode mode = m_pressed ? PictureViewer2D_pressed : PictureViewer2D_idle;

	ScreenToPicture(m_winmessage_point.x, m_winmessage_point.y, constant_px, constant_py, mode);

	if (m_scale_change_koeff > 0)
	{
		double k = m_scale_change_koeff;
		if (m_wheel_direction < 0)
			k = 1.0 / k;
		*scale_ptr *= k;
	}
	if (m_scale_change_offset > 0)
	{
		double k = m_scale_change_offset;
		if (m_wheel_direction < 0)
			k = -k;
		*scale_ptr += k;
	}

	if (m_min_scale > 0)
	{
		if (*scale_ptr < m_min_scale)
			*scale_ptr = m_min_scale;
	}
	if (m_max_scale > 0)
	{
		if (*scale_ptr > m_max_scale)
			*scale_ptr = m_max_scale;
	}

	// сохраняем координаты для нажатого режима
	float px, py;
	PictureToScreen(constant_px, constant_py, px, py, PictureViewer2D_pressed);
	m_pressed_picture_x += (m_winmessage_point.x - px);
	m_pressed_picture_y += (m_winmessage_point.y - py);

	// сохраняем координаты для ненажатого режима
	float ix, iy;
	PictureToScreen(constant_px, constant_py, ix, iy, PictureViewer2D_idle);
	m_idle_picture_x += (m_winmessage_point.x - ix);
	m_idle_picture_y += (m_winmessage_point.y - iy);

	m_current_picture_x = m_pressed ? m_pressed_picture_x : m_idle_picture_x;
	m_current_picture_y = m_pressed ? m_pressed_picture_y : m_idle_picture_y;
	m_current_scale = m_pressed ? m_pressed_scale : m_idle_scale;

	ShowZoomChange();

	Invalidate();
}

void PictureViewer2D::ShowZoomChange()
{
	if (m_show_zoom_info)
	{
		m_zoom_info_is_visible = true;
		KillTimer(1);
		SetTimer(1, 1000); // hide zoom info in 1 sec
	}
}

void PictureViewer2D::OnTimer(UINT timer_id)
{
	if (timer_id == 1)
	{
		KillTimer(1);
		m_zoom_info_is_visible = false;
		Invalidate();
	}
}

void PictureViewer2D::OnMouseMove()
{
	if (m_on_mouse_function && m_on_mouse_function(this))
		return;

	if (m_dragging)
	{
		float dx = m_winmessage_point.x - m_dragging_point.x;
		float dy = m_winmessage_point.y - m_dragging_point.y;
		m_idle_picture_x = m_dragging_idle_x + dx;
		m_idle_picture_y = m_dragging_idle_y + dy;
		m_pressed_picture_x = m_dragging_pressed_x + dx * m_idle_scale / m_pressed_scale;
		m_pressed_picture_y = m_dragging_pressed_y + dy * m_idle_scale / m_pressed_scale;
		m_current_picture_x = m_pressed ? m_pressed_picture_x : m_idle_picture_x;
		m_current_picture_y = m_pressed ? m_pressed_picture_y : m_idle_picture_y;
	}
	else
	if (m_pressed)
		OnChangeScale(m_winmessage_point.x, m_winmessage_point.y, m_idle_scale, PictureViewer2D_idle);
	else
		return;
	Invalidate();
}

void PictureViewer2D::ScreenToPicture(float sx, float sy, float& px, float &py, PictureViewer2D_Mode mode)
{
	if (mode == PictureViewer2D_current)
		mode = m_pressed ? PictureViewer2D_pressed : PictureViewer2D_idle;
	bool pressed = (mode == PictureViewer2D_pressed);
	double scale = pressed ? m_pressed_scale : m_idle_scale;
	float pic_pos_x = pressed ? m_pressed_picture_x : m_idle_picture_x;
	float pic_pos_y = pressed ? m_pressed_picture_y : m_idle_picture_y;
	px = (sx - pic_pos_x) / scale;
	py = (sy - pic_pos_y) / scale;
}

void PictureViewer2D::PictureToScreen(float px, float py, float& sx, float &sy, PictureViewer2D_Mode mode)
{
	if (mode == PictureViewer2D_current)
		mode = m_pressed ? PictureViewer2D_pressed : PictureViewer2D_idle;
	bool pressed = (mode == PictureViewer2D_pressed);
	double scale = pressed ? m_pressed_scale : m_idle_scale;
	float pic_pos_x = pressed ? m_pressed_picture_x : m_idle_picture_x;
	float pic_pos_y = pressed ? m_pressed_picture_y : m_idle_picture_y;
	sx = px * scale + pic_pos_x;
	sy = py * scale + pic_pos_y;
}

void PictureViewer2D::OnChangeScale(float screen_x_to_keep, float screen_y_to_keep, float old_scale, PictureViewer2D_Mode old_mode)
{
	float constant_px, constant_py;
	ScreenToPicture(screen_x_to_keep, screen_y_to_keep, constant_px, constant_py, old_mode);

	// сохраняем координаты для нажатого режима
	float px, py;
	PictureToScreen(constant_px, constant_py, px, py, PictureViewer2D_pressed);
	m_pressed_picture_x += (screen_x_to_keep - px);
	m_pressed_picture_y += (screen_y_to_keep - py);

	// сохраняем координаты для ненажатого режима
	float ix, iy;
	PictureToScreen(constant_px, constant_py, ix, iy, PictureViewer2D_idle);
	m_idle_picture_x += (screen_x_to_keep - ix);
	m_idle_picture_y += (screen_y_to_keep - iy);

	m_current_picture_x = m_pressed ? m_pressed_picture_x : m_idle_picture_x;
	m_current_picture_y = m_pressed ? m_pressed_picture_y : m_idle_picture_y;
	m_current_scale = m_pressed ? m_pressed_scale : m_idle_scale;
}

void PictureViewer2D::OnLBDown()
{
	if (m_on_mouse_function && m_on_mouse_function(this))
		return;

	if (!m_pressed_availale)
		return;

	if ((m_winmessage_flag & MK_CONTROL) == 0)
	{
		m_dragging = true;
		m_dragging_point = m_winmessage_point;
		ScreenToPicture(m_dragging_point.x, m_dragging_point.y, m_dragging_pic_x, m_dragging_pic_y, PictureViewer2D_idle);
		m_dragging_idle_x = m_idle_picture_x;
		m_dragging_idle_y = m_idle_picture_y;
		m_dragging_pressed_x = m_pressed_picture_x;
		m_dragging_pressed_y = m_pressed_picture_y;
	}
	else
	{
		m_pressed = true;
		// точку мышки оставляем неподвижной
		OnChangeScale(m_winmessage_point.x, m_winmessage_point.y, m_idle_scale, PictureViewer2D_idle);
		ShowZoomChange();
	}
	Invalidate();
}

void PictureViewer2D::OnOtherMouse()
{
	if (m_on_mouse_function)
		m_on_mouse_function(this);
}

void PictureViewer2D::OnMBDown()
{
	if (m_on_mouse_function && m_on_mouse_function(this))
		return;

	if (m_pressed)
		return;
	double old_scale = m_current_scale;
	m_idle_scale = m_current_scale = m_default_scale;
	CenterPicture();
	ShowZoomChange();
	//OnChangeScale(m_winmessage_point.x, m_winmessage_point.y, old_scale, PictureViewer2D_idle);
	Invalidate();
}

void PictureViewer2D::OnLBUp()
{
	if (m_on_mouse_function && m_on_mouse_function(this))
		return;

	if (m_pressed)
	{
		m_pressed = false;
		// точку мышки оставляем неподвижной
		OnChangeScale(m_winmessage_point.x, m_winmessage_point.y, m_pressed_scale, PictureViewer2D_pressed);
		ShowZoomChange();
	}
	m_dragging = false;
	if (!m_pressed_availale)
		return;
	Invalidate();
}

void PictureViewer2D::SetScales(float idle, float pressed)
{
	float old_scale = m_pressed ? m_pressed_scale : m_idle_scale;
	if (idle > 0)
		m_idle_scale = idle;
	if (pressed > 0)
		m_pressed_scale = pressed;
	m_current_scale = m_pressed ? m_pressed_scale : m_idle_scale;

	// оставляем центр неподвижным
	CRect client_rect = GetWindowRect();
	OnChangeScale(0.5f * client_rect.Width(), 0.5f * client_rect.Height(), old_scale, m_pressed ? PictureViewer2D_pressed : PictureViewer2D_idle);
}

void PictureViewer2D::CenterPicture()
{
	CRect client_rect = GetWindowRect();
	int client_w = client_rect.Width(), client_h = client_rect.Height();

	m_idle_picture_x = 0.5f * (client_w - m_picture_width * m_idle_scale);
	m_idle_picture_y = 0.5f * (client_h - m_picture_height * m_idle_scale);

	m_pressed_picture_x = 0.5f * (client_w - m_picture_width * m_pressed_scale);
	m_pressed_picture_y = 0.5f * (client_h - m_picture_height * m_pressed_scale);

	m_current_picture_x = m_pressed ? m_pressed_picture_x : m_idle_picture_x;
	m_current_picture_y = m_pressed ? m_pressed_picture_y : m_idle_picture_y;
}

void PictureViewer2D::OnSetBitmap(int additional_num)
{
	{
		MutexWrap objects_access(m_dc->m_objects_mutex);

		G2D_Bitmap* new_bmp_2d = NULL;

		BMP_Bitmap **bmp_bitmap = &m_bmp_bitmap;
		G2D_Bitmap **bmp_2d = &m_bmp_2d;
		int *bmp_obj_index = &m_bmp_obj_index;

		if (additional_num != -1)
		{
			PictureViewer2D_BitmapWrap *wrap = *(m_bmp_wraps.GetBuff() + additional_num);
			bmp_bitmap = &(wrap->m_bmp_bitmap);
			bmp_2d = &(wrap->m_bmp_2d);
			bmp_obj_index = &(wrap->m_bmp_obj_index);
		}

		if (*bmp_bitmap == NULL)
		{
			if (*bmp_obj_index != -1)
			{
				m_dc->RemoveObject(*bmp_obj_index);
				*bmp_obj_index = -1;
			}	
		}
		else
		{
			new_bmp_2d = m_dc->MakeBitmap(*bmp_bitmap);
			new_bmp_2d->m_name.Format("bitmap 2D (idx %d)", additional_num);
			if (*bmp_obj_index == -1)
				*bmp_obj_index = m_dc->AddObject(new_bmp_2d, new_bmp_2d->m_name);
			else
				m_dc->SetObject(new_bmp_2d, *bmp_obj_index);
		}

		if (*bmp_2d)
			delete *bmp_2d;
		*bmp_2d = new_bmp_2d;
	}

	if (additional_num == -1 && (m_prev_picture_width != m_picture_width || m_prev_picture_height != m_picture_height))
	{
		m_prev_picture_width = m_picture_width;
		m_prev_picture_height = m_picture_height;
		if (m_bmp_2d)
			CenterPicture();
	}

	Invalidate();
}

void PictureViewer2D::ClearPalette()
{
	for (int i = 0; i < 256; i++)
	{
		m_palette[i].rgbBlue = m_palette[i].rgbGreen = m_palette[i].rgbRed = i;
		m_palette[i].rgbReserved = 0;
	}
	m_changed_palette = false;
}

void PictureViewer2D::UpdatePalette()
{
	if (m_changed_palette)
		memcpy(m_bmp_bitmap->m_bmi->bmiColors, m_palette, 256 * 4);
}

void PictureViewer2D::SetPaletteColor(byte colornum, byte r, byte g, byte b)
{
	m_changed_palette = true;
	m_palette[colornum].rgbRed = r;
	m_palette[colornum].rgbGreen = g;
	m_palette[colornum].rgbBlue = b;
}

void PictureViewer2D::MakeBmpOnSetGrayscale(void* data, int w, int h, int bpp, BMP_Bitmap **bmp_bitmap)
{
	if (*bmp_bitmap)
		delete *bmp_bitmap;
	*bmp_bitmap = NULL;

	if (data)
	{
		// делаем m_bmp_bitmap по grayscale данным
		*bmp_bitmap = new BMP_Bitmap;
		(*bmp_bitmap)->m_w = (*bmp_bitmap)->m_bmi->bmiHeader.biWidth = w;
		(*bmp_bitmap)->m_h = (*bmp_bitmap)->m_bmi->bmiHeader.biHeight = h;
		(*bmp_bitmap)->m_bmi->bmiHeader.biBitCount = 8;
		(*bmp_bitmap)->m_bytes_line_filler = (4 - (w % 4)) % 4;
		int sz = (w + (*bmp_bitmap)->m_bytes_line_filler) * h;
		(*bmp_bitmap)->m_bytes = new byte[sz];
		if (bpp <= 8)
			memcpy((*bmp_bitmap)->m_bytes, data, sz);
		else
		if (bpp == 32)
		{
			int i;
			float *float_data = (float *)data;
			byte *bytes_ptr = (*bmp_bitmap)->m_bytes;
			for (i = 0; i < sz; i++, float_data++, bytes_ptr++)
				*bytes_ptr = byte(*float_data);
		}
		else
		{
			int i, shift = bpp - 8;
			WORD *word_data = (WORD *)data;
			byte *bytes_ptr = (*bmp_bitmap)->m_bytes;
			for (i = 0; i < sz; i++, word_data++, bytes_ptr++)
				*bytes_ptr = (*word_data) >> shift;
		}
		(*bmp_bitmap)->m_loaded = true;
	}
}

void PictureViewer2D::SetAdditionalGrayscaleData(void* data, int w, int h, int bpp, int additional_num, float dx, float dy)
{
	if (additional_num > m_bmp_wraps.GetFilledSize())
		return;
	PictureViewer2D_BitmapWrap* wrap = NULL;
	if (additional_num == m_bmp_wraps.GetFilledSize())
	{
		wrap = new PictureViewer2D_BitmapWrap;
		m_bmp_wraps.AddValue(wrap);
	}
	else
		wrap = *(m_bmp_wraps.GetBuff() + additional_num);

	wrap->m_offset_x = dx;
	wrap->m_offset_y = dy;
	wrap->m_w = w;
	wrap->m_h = h;
	MakeBmpOnSetGrayscale(data, w, h, bpp, &(wrap->m_bmp_bitmap));
	OnSetBitmap(additional_num);
}

void PictureViewer2D::SetGrayscaleData(void* data, int w, int h, int bpp)
{
	MutexWrap objects_access(m_dc->m_objects_mutex);

	if (m_bmp_bitmap)
		delete m_bmp_bitmap;
	m_bmp_bitmap = NULL;

	if (data)
	{
		MakeBmpOnSetGrayscale(data, w, h, bpp, &m_bmp_bitmap);
		UpdatePalette();
	}
	else
	{
		w = INT_MIN;
		h = INT_MIN;
	}

	m_picture_width = w;
	m_picture_height = h;

	OnSetBitmap();
}

void PictureViewer2D::SetBitmap(BMP_Bitmap* bmp)
{
	if (m_bmp_bitmap)
		delete m_bmp_bitmap;

	m_bmp_bitmap = (bmp == NULL) ? NULL : bmp->Clone();

	m_picture_width = (bmp == NULL) ? 0 : bmp->m_w;
	m_picture_height = (bmp == NULL) ? 0 : bmp->m_h;

	OnSetBitmap();
}

////////////////////////////////////////////////////////////////////////////////
// end