// uic 2014.04.24

#include <stdafx.h>

#include "G2D_D2D.h"

#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")

////////////////////////////////////////////////////////////////////////////////
// G2D_D2D_Brush
////////////////////////////////////////////////////////////////////////////////
G2D_D2D_Brush::G2D_D2D_Brush(G2D_Color* col): G2D_Brush(col), m_brush(NULL)
{
}

G2D_D2D_Brush::~G2D_D2D_Brush()
{
	if (m_brush)
		m_brush->Release();
}

void G2D_D2D_Brush::OnSelect(G2D_DC* dc)
{
	((G2D_D2D_DC*)dc)->m_current_line_width = m_width;
}

////////////////////////////////////////////////////////////////////////////////
// G2D_D2D_Bitmap
////////////////////////////////////////////////////////////////////////////////
G2D_D2D_Bitmap::G2D_D2D_Bitmap():
	m_bitmap(NULL), m_bmp_bitmap(NULL)
{
}

G2D_D2D_Bitmap::~G2D_D2D_Bitmap()
{
	if (m_bitmap)
		m_bitmap->Release();
	if (m_bmp_bitmap)
		delete m_bmp_bitmap;
}

bool G2D_D2D_Bitmap::Make(G2D_D2D_DC* dc, BMP_Bitmap* bmp)
{
	if (m_bmp_bitmap)
		delete m_bmp_bitmap;
	m_bmp_bitmap = bmp->Clone();

	int bpp = 4;
	int pitch = bmp->m_w * bpp, sz = bmp->m_w * bmp->m_h * bpp;
	D2D1_SIZE_U size = D2D1::SizeU(bmp->m_w, bmp->m_h);
	D2D1_BITMAP_PROPERTIES props;
	props.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);
	props.dpiX = 96;
	props.dpiY = 96;

	int in_bpp = 1 + (max(0, bmp->m_bmi->bmiHeader.biBitCount - 1) / 8);
	int in_line = bmp->m_w * in_bpp, filler = in_line%4;
	if (filler > 0)
		filler = 4 - filler;
	int x, y, in_pitch = in_line + filler, clr_num;
	byte *tmp_data = new byte[sz], *tmp_ptr = tmp_data, *in_ptr = bmp->m_bytes;
	RGBQUAD *palette = bmp->m_bmi->bmiColors;
	memset(tmp_data, 0, sz);
	for (y = 0; y < bmp->m_h; y++)
	{
		tmp_ptr = tmp_data + y * pitch;
		in_ptr = bmp->m_bytes + y * in_pitch;
		for (x = 0; x < bmp->m_w; x++, tmp_ptr+=4)
		{
			tmp_ptr[3] = 255; // alpha
			if (in_bpp == 4)
			{
				tmp_ptr[2] = *in_ptr; in_ptr++; // r
				tmp_ptr[1] = *in_ptr; in_ptr++; // g
				tmp_ptr[0] = *in_ptr; in_ptr++; // b
				in_ptr++; // игнор альфы
			}
			else
			if (in_bpp == 3)
			{
				tmp_ptr[2] = *in_ptr; in_ptr++; // r
				tmp_ptr[1] = *in_ptr; in_ptr++; // g
				tmp_ptr[0] = *in_ptr; in_ptr++; // b
			}
			else
			if (in_bpp == 1)
			{
				clr_num = *in_ptr; in_ptr++;
				tmp_ptr[2] = palette[clr_num].rgbRed;	// r
				tmp_ptr[1] = palette[clr_num].rgbGreen;	// g
				tmp_ptr[0] = palette[clr_num].rgbBlue;	// b
				if (palette[clr_num].rgbReserved == 255)
					tmp_ptr[3] = 0;
			}
		}
	}
	HRESULT hr = dc->m_render_target->CreateBitmap(size, tmp_data, pitch, &props, &m_bitmap);
	delete[] tmp_data;
	if (hr != S_OK)
		return false;
	m_width = bmp->m_w;
	m_height = bmp->m_h;
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// G2D_D2D_Font
////////////////////////////////////////////////////////////////////////////////
G2D_D2D_Font::G2D_D2D_Font(): m_text_format(NULL)
{
}

G2D_D2D_Font::~G2D_D2D_Font()
{
	if (m_text_format)
		m_text_format->Release();
}

bool G2D_D2D_Font::Make(G2D_D2D_DC*dc, WCHAR* name, int height, bool bold, bool italic)
{
	if (m_text_format)
		m_text_format->Release();
	if (
			dc->m_dwrite_factory->CreateTextFormat(
				name, NULL,
				bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_REGULAR,
				italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				height,
				L"en-us", &m_text_format) != S_OK
		)
	{
		m_text_format = NULL;
		return false;
	}
	m_text_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING); // x -> +1
	m_text_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR); // y -> +1
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// G2D_D2D_DC
////////////////////////////////////////////////////////////////////////////////
G2D_D2D_DC::G2D_D2D_DC():
	m_render_target(NULL),
	m_current_line_width(1.0f),
	m_dwrite_factory(NULL), m_selected_text_format(NULL),
	m_tmp_brush(NULL)
{
	m_current_draw_position.x = 0;
	m_current_draw_position.y = 0;
}

G2D_D2D_DC::~G2D_D2D_DC()
{
}

void G2D_D2D_DC::OnSelectFont(G2D_Font* font)
{
	m_selected_text_format = ((G2D_D2D_Font*)font)->m_text_format;
}

void G2D_D2D_DC::OnCleanup()
{
	m_inited = false;
/*
	if (m_tmp_brush)
	{
		delete m_tmp_brush;
		m_tmp_brush = NULL;
	}
*/
	if (m_dwrite_factory)
	{
		m_dwrite_factory->Release();
		m_dwrite_factory = NULL;
	}
	if (m_render_target)
	{
		m_render_target->Release();
		m_render_target = NULL;
	}
/*
	if (m_selected_text_format)
	{
		m_selected_text_format->Release();
		m_selected_text_format = NULL;
	}
*/
	int i, isz = m_layers.GetFilledSize();
	ID2D1Layer** layer_ptr = m_layers.GetBuff();
	for (i = 0; i < isz; i++, layer_ptr++)
		(*layer_ptr)->Release();
	m_layers.Clear();
	
	m_interface_object->OnCleanup();
}

void G2D_D2D_DC::OnResize()
{
	CRect window_rect = m_interface_object->GetWindowRect();
	m_render_target->Resize(D2D1::SizeU(window_rect.Width(), window_rect.Height()));
}

void G2D_D2D_DC::OnInit()
{
	CWnd* wnd = m_interface_object->GetWnd();
	CRect window_rect = m_interface_object->GetWindowRect();
	ID2D1Factory* factory = NULL;
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory);
	if (factory)
	{
		m_rt_props = D2D1::RenderTargetProperties();
		HRESULT hr = 
		factory->CreateHwndRenderTarget(
				 m_rt_props,
				 D2D1::HwndRenderTargetProperties(wnd->m_hWnd, D2D1::SizeU(window_rect.Width(), window_rect.Height())),
				 &m_render_target);
		factory->Release();

		m_tmp_brush = new G2D_D2D_Brush;
		m_render_target->CreateSolidColorBrush(D2D1::ColorF(0, 0, 0, 1), &(m_tmp_brush->m_brush));
		m_tmp_brush->m_width = 1;
		AddObject(m_tmp_brush, _T("# m_tmp_brush #"));

		DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&m_dwrite_factory));

		if (m_on_init_dc_function)
			m_on_init_dc_function(m_interface_object);
	}
	m_inited = true;
}

G2D_Color* G2D_D2D_DC::MakeColor(byte r, byte g, byte b, byte alpha)
{
	G2D_D2D_Color* res = new G2D_D2D_Color;
	res->m_r = r;
	res->m_g = g;
	res->m_b = b;
	res->m_alpha = alpha;
	float k = 1.0f / 255;
	res->m_color = D2D1::ColorF(k * r, k * g, k * b, k * alpha);
	return res;
}

G2D_Brush* G2D_D2D_DC::MakeBrush(G2D_Color* color, float width)
{
	G2D_D2D_Brush* res = new G2D_D2D_Brush(color);
	m_render_target->CreateSolidColorBrush(((G2D_D2D_Color*)color)->m_color, &res->m_brush);
	res->m_color = *color;
	res->m_width = width;
	return res;
}

G2D_Bitmap* G2D_D2D_DC::MakeBitmap(BMP_Bitmap* bmp)
{
	G2D_D2D_Bitmap *res = new G2D_D2D_Bitmap;
	if (!res->Make(this, bmp))
	{
		delete res;
		return NULL;
	}
	res->m_width = bmp->m_w;
	res->m_height = bmp->m_h;
	return res;
}

G2D_Font* G2D_D2D_DC::MakeFont(WCHAR* name, int height, bool bold, bool italic)
{
	G2D_D2D_Font *res = new G2D_D2D_Font;
	if (!res->Make(this, name, height, bold, italic))
	{
		delete res;
		return NULL;
	}
	return res;
}

void G2D_D2D_DC::OnBeginDraw()
{
	m_render_target->BeginDraw();
	if (m_bk_color == NULL)
	{
		DWORD bk_color = m_interface_object->m_bk_color;
		m_bk_color = MakeColor((bk_color) & 0xFF, (bk_color >> 8) & 0xFF, (bk_color >> 16) & 0xFF, 255);
	}
	m_render_target->Clear(((G2D_D2D_Color*)m_bk_color)->m_color);
}

void G2D_D2D_DC::OnEndDraw()
{
	HRESULT hr = m_render_target->EndDraw();
	if (hr != S_OK)
	{
		// Restore all
		Cleanup();
		OnInit();
		m_interface_object->Invalidate();
	}
}

void G2D_D2D_DC::DrawBitmap(TYPE x, TYPE y, G2D_Bitmap* bm, float alpha, TYPE to_w, TYPE to_h, CRect *from_rect, bool smooth)
{
	if (bm == NULL)
		return;
	G2D_D2D_Bitmap *d2d_bm = (G2D_D2D_Bitmap *)bm;
	if (d2d_bm->m_bitmap == NULL)
		return;
	if (to_w == 0)
		to_w = bm->m_width;
	if (to_h == 0)
		to_h = bm->m_height;
	D2D1_RECT_F d2d_to_rect = D2D1::RectF(x, y, x + to_w, y + to_h), *d2d_from_rect = NULL;
	if (from_rect != NULL)
	{
		d2d_from_rect = new D2D1_RECT_F;
		*d2d_from_rect = D2D1::RectF(d2d_from_rect->left, d2d_from_rect->top, d2d_from_rect->right, d2d_from_rect->bottom);
	}
	m_render_target->DrawBitmap(d2d_bm->m_bitmap, d2d_to_rect, alpha, smooth ? D2D1_BITMAP_INTERPOLATION_MODE_LINEAR : D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, d2d_from_rect);
}

void G2D_D2D_DC::DrawRect(TYPE x, TYPE y, TYPE w, TYPE h)
{
	if (m_current_brush == NULL)
		return;

	if (w < 0)
	{
		x += w + 1;
		w = -w;
	}
	if (h < 0)
	{
		y += h + 1;
		h = -h;
	}
	float half = 0.5f * m_current_line_width;
	D2D1_RECT_F rect = D2D1::RectF(x + half, y + half, x + w - half, y + h - half);
	m_render_target->DrawRectangle(&rect, ((G2D_D2D_Brush*)m_current_brush)->m_brush, m_current_line_width, NULL);
}

void G2D_D2D_DC::OwnSetIdentityTransform()
{
	Matrix3x2F new_transform = Matrix3x2F::Identity();
	m_render_target->SetTransform(&new_transform);
}

void G2D_D2D_DC::OwnRotate(float alpha, TYPE xc, TYPE yc)
{
	Matrix3x2F matix_before;
	m_render_target->GetTransform(&matix_before);
	Matrix3x2F matix_rotate;
	D2D1_POINT_2F pt;
	pt.x = xc;
	pt.y = yc;
	matix_rotate = Matrix3x2F::Rotation(alpha, pt);

	Matrix3x2F new_transform;
	new_transform.SetProduct(matix_rotate, matix_before);
	m_render_target->SetTransform(&new_transform);
}

void G2D_D2D_DC::OwnOffsetOrigin(TYPE dx, TYPE dy)
{
	Matrix3x2F matix_before;
	m_render_target->GetTransform(&matix_before);
	Matrix3x2F matix_translate;
	matix_translate = Matrix3x2F::Translation(dx, dy);
/*	matix_after._11 = 1;
	matix_after._12 = 0;
	matix_after._21 = 0;
	matix_after._22 = 1;
	matix_after._31 = dx;
	matix_after._32 = dy;
*/
	Matrix3x2F new_transform;
	new_transform.SetProduct(matix_before, matix_translate);
	m_render_target->SetTransform(&new_transform);
}

void G2D_D2D_DC::DrawEllipse(TYPE x, TYPE y, TYPE w, TYPE h)
{
	if (m_current_brush == NULL)
		return;
	D2D1_POINT_2F pt;
	pt.x = x + 0.5f * w;
	pt.y = y + 0.5f * h;
	m_render_target->DrawEllipse(D2D1::Ellipse(pt, 0.5f * w, 0.5f * h), ((G2D_D2D_Brush*)m_current_brush)->m_brush, m_current_line_width, NULL);
}

void G2D_D2D_DC::FillEllipse(TYPE x, TYPE y, TYPE w, TYPE h, G2D_Brush* brush)
{
	D2D1_POINT_2F pt;
	pt.x = x + 0.5f * w;
	pt.y = y + 0.5f * h;
	m_render_target->FillEllipse(D2D1::Ellipse(pt, 0.5f * w, 0.5f * h), ((G2D_D2D_Brush*)brush)->m_brush);
}

void G2D_D2D_DC::FillEllipse(TYPE x, TYPE y, TYPE w, TYPE h, G2D_Color* col)
{
	m_tmp_brush->m_brush->SetColor(((G2D_D2D_Color*)col)->m_color);
	FillEllipse(x, y, w, h, m_tmp_brush);
}

void G2D_D2D_DC::FillRect(TYPE x, TYPE y, TYPE w, TYPE h, G2D_Brush* brush)
{
	m_render_target->FillRectangle(D2D1::RectF(x, y, x + w, y + h), ((G2D_D2D_Brush*)brush)->m_brush);
}

void G2D_D2D_DC::FillRect(TYPE x, TYPE y, TYPE w, TYPE h, G2D_Color* col)
{
	m_tmp_brush->m_brush->SetColor(((G2D_D2D_Color*)col)->m_color);
	FillRect(x, y, w, h, m_tmp_brush);
}

void G2D_D2D_DC::MoveTo(TYPE x, TYPE y)
{
	float half = 0.5f * m_current_line_width;
	m_current_draw_position.x = x;// - half;
	m_current_draw_position.y = y;// - half;
}

void G2D_D2D_DC::LineTo(TYPE x, TYPE y)
{
	if (m_current_brush == NULL)
		return;

	float half = 0.5f * m_current_line_width;
	D2D1_POINT_2F next_pos;
	next_pos.x = x;// - half;
	next_pos.y = y;// - half;
	m_render_target->DrawLine(m_current_draw_position, next_pos, ((G2D_D2D_Brush*)m_current_brush)->m_brush, m_current_line_width, NULL);
	m_current_draw_position = next_pos;
}

bool G2D_D2D_DC::GetTextExtent(WCHAR* text, float &w, float &h, int len)
{
	CSize sz;
	if (len == -1)
	{
		while (text[len + 1] != 0)
			len++;
		len++;
	}

	IDWriteTextLayout* text_layout = NULL;
	m_dwrite_factory->CreateTextLayout(text, len, m_selected_text_format, 1920, 1080, &text_layout);
	DWRITE_TEXT_METRICS tm;
	text_layout->GetMetrics(&tm);
	w = tm.width;
	h = tm.height;
	text_layout->Release();
	return true;
}

void G2D_D2D_DC::DrawTextWithRect(TYPE x, TYPE y, TYPE dw, TYPE dh, WCHAR* text, G2D_Color* fill_color, G2D_Brush* stroke_brush, int xalign, int yalign)
{
	float rect_w, rect_h, half_dw = 0.5*dw, half_dh = 0.5*dh;
	GetTextExtent(text, rect_w, rect_h);

	//x -= xalign * half_dw;
	//y -= yalign * half_dh;
	rect_w += dw;
	rect_h += dh;

	D2D1_RECT_F rect = D2D1::RectF(x, y, x + xalign * rect_w, y + yalign * rect_h);
	if (xalign == -1)
		swap(rect.left, rect.right);
	if (yalign == -1)
		swap(rect.top, rect.bottom);
	if (xalign == 0)
	{
		rect.left -= float(rect_w)/2;
		rect.right = rect.left + rect_w;
	}
	if (yalign == 0)
	{
		rect.top -= float(rect_h)/2;
		rect.bottom = rect.top + rect_h;
	}

	G2D_D2D_Brush* text_brush = (G2D_D2D_Brush*)m_current_brush;
	if (fill_color)
		FillRect(rect.left, rect.top, rect_w, rect_h, fill_color);
	if (stroke_brush)
	{
		SelectBrush(stroke_brush);
		DrawRect(rect.left, rect.top, rect_w, rect_h);
	}
	if (text_brush)
	{
		rect.left += half_dw;
		rect.right -= half_dw; rect.right++;
		rect.top += half_dh;
		rect.bottom -= half_dh;
		
		m_selected_text_format->SetTextAlignment(xalign == +1 ? DWRITE_TEXT_ALIGNMENT_LEADING : xalign == -1 ? DWRITE_TEXT_ALIGNMENT_TRAILING : DWRITE_TEXT_ALIGNMENT_CENTER);
		m_selected_text_format->SetParagraphAlignment(yalign == +1 ? DWRITE_PARAGRAPH_ALIGNMENT_NEAR : yalign == -1 ? DWRITE_PARAGRAPH_ALIGNMENT_FAR : DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		int len = 0;
		while (text[len] != 0)
			len++;
		m_render_target->DrawText(text, len, m_selected_text_format, rect, text_brush->m_brush);
	}
}

void G2D_D2D_DC::DrawText(TYPE x, TYPE y, WCHAR* text, int xalign, int yalign)
{
	float text_w, text_h;
	GetTextExtent(text, text_w, text_h);
	if (text_w - int(text_w) > 0)
		text_w = int(text_w) + 1;
	D2D1_RECT_F rect = D2D1::RectF(x, y, x + xalign * text_w, y + yalign * text_h);
	if (xalign == 0)
	{
		rect.left -= text_w * 0.5f;
		rect.right = rect.left + text_w;
	}
	if (yalign == 0)
	{
		rect.top -= text_h * 0.5f;
		rect.bottom = rect.top + text_h;
	}
/*
	if (m_test_brush)
	{
		G2D_Brush *tmp = m_current_brush;
		SelectBrush(m_test_brush);
		DrawRect(rect.left, rect.top, rect.right - rect.left + 1, rect.bottom - rect.top + 1);
		DrawRect(x - 1, y - 1, 3, 3);
		SelectBrush(tmp);
	}
*/
	m_selected_text_format->SetTextAlignment(xalign == +1 ? DWRITE_TEXT_ALIGNMENT_LEADING : xalign == -1 ? DWRITE_TEXT_ALIGNMENT_TRAILING : DWRITE_TEXT_ALIGNMENT_CENTER);
	m_selected_text_format->SetParagraphAlignment(yalign == +1 ? DWRITE_PARAGRAPH_ALIGNMENT_NEAR : yalign == -1 ? DWRITE_PARAGRAPH_ALIGNMENT_FAR : DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	int len = 0;
	while (text[len] != 0)
		len++;
	m_render_target->DrawText(text, len, m_selected_text_format, rect, ((G2D_D2D_Brush*)m_current_brush)->m_brush);
	//DrawRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
}

void G2D_D2D_DC::ClipRect(int x, int y, int w, int h)
{
	ID2D1Layer* layer;
	m_render_target->CreateLayer(NULL, &layer);
	m_layers.AddValue(layer);
	m_render_target->PushLayer(D2D1::LayerParameters(D2D1::RectF(x, y, x + w, y + h)), layer);
}

void G2D_D2D_DC::UnClip()
{
	m_render_target->PopLayer();
	int isz = m_layers.GetFilledSize();
	if (isz > 0)
	{
		ID2D1Layer* layer = *(m_layers.GetBuff() + isz - 1);
		layer->Release();
		m_layers.SetFilledSize(isz - 1);
	}
}

////////////////////////////////////////////////////////////////////////////////
// end