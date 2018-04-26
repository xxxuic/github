// uic 2015.10.15

#include <stdafx.h>
#include <math.h>
#include "GRF.h"

#define RISKA_SZ 5

////////////////////////////////////////////////////////////////////////////////
// GRF_Scale
////////////////////////////////////////////////////////////////////////////////
GRF_Scale::GRF_Scale(CStringW scale_name, CStringW unit_name, double start_unit, double final_unit, double risk_unit):
	m_scale_name(scale_name), m_unit_name(unit_name),
	m_start_unit(start_unit), m_available_start_unit(start_unit),
	m_final_unit(final_unit), m_available_final_unit(final_unit),
	m_visible_digits(1),
	m_units_per_risk(risk_unit), m_risk_splitting1(2), m_risk_splitting2(5)
{
}

double GRF_Scale::GetCoord(double unit)
{
	return (unit - m_start_unit) * m_pix_per_unit;
}

double GRF_Scale::GetUnit(double coord)
{
	return m_start_unit + coord / m_pix_per_unit;
}

void GRF_Scale::SetScale(double size)
{
	double du = max(1e-20, (m_final_unit - m_start_unit));
	m_pix_per_unit = (size - 1) / du;
}

void GRF_Scale::MemorizeAvailableUnits()
{
	if (m_available_start_unit == -1)
		m_available_start_unit = m_start_unit;
	if (m_available_final_unit == -1)
		m_available_final_unit = m_final_unit;
}

////////////////////////////////////////////////////////////////////////////////
// GRF_Array
////////////////////////////////////////////////////////////////////////////////
GRF_Array::GRF_Array():
	m_scale(NULL)
{
	MakeEmpty();
}

void GRF_Array::MakeEmpty()
{
	//m_empty = true;
	int i, isz = m_points.GetFilledSize();
	double* ptr = m_points.GetBuff();
	for (i = 0; i < isz; i++, ptr++)
		*ptr = GRF_NAN;
}

////////////////////////////////////////////////////////////////////////////////
// GRF_Trace
////////////////////////////////////////////////////////////////////////////////
int GRF_Trace::m_max_id = 0;

GRF_Trace::GRF_Trace():
	m_vertical_data(NULL), m_horizontal_data(NULL), m_last_pos(-1), m_visible(true), m_width(1.0f),
	m_draw_selected_point_mode(0),
	m_draw_selected_point_size(5),
	m_points_mode(0),
	m_points_sz(0)
{
	m_uniq_id = GRF_Trace::m_max_id++;
}

GRF_Trace::~GRF_Trace()
{
	if (m_vertical_data)
		delete m_vertical_data;
}

void GRF_Trace::CheckGraphObjects(G2D_DC* dc)
{
	CString prefix;
	prefix.Format(_T("trace %d "), m_uniq_id);

	m_color = dc->CheckColor(prefix + _T("color"), (m_color_rgba & 0xFF000000) >> 24, (m_color_rgba & 0x00FF0000) >> 16, (m_color_rgba & 0x0000FF00) >> 8, (m_color_rgba & 0x000000FF));
	m_brush = dc->CheckBrush(prefix + _T("brush"), m_color, m_width);
	m_last_color = dc->CheckColor(prefix + _T("last color"), (m_color_rgba & 0xFF000000) >> 24, (m_color_rgba & 0x00FF0000) >> 16, (m_color_rgba & 0x0000FF00) >> 8, (m_color_rgba & 0x000000FF) * 0.3);
	m_last_brush = dc->CheckBrush(prefix + _T("last brush"), m_last_color, m_width);
}

bool GRF_Trace::GetUnitByUnit(double x_unit, double& y_unit)
{
	int xsz = m_horizontal_data->m_points.GetFilledSize(),
		ysz = m_vertical_data->m_points.GetFilledSize();
	if (xsz < 2 || ysz < 2)
		return false;
	double *x_units_ptr = m_horizontal_data->m_points.GetBuff(), *y_units_ptr = m_vertical_data->m_points.GetBuff();
	double x_min = *x_units_ptr, x_max = *(x_units_ptr + xsz - 1);
	if (x_min > x_unit || x_max < x_unit)
		return false;
	double x_step = (x_max - x_min) / (xsz - 1);
	double x_k = (x_unit - x_min) / (x_max - x_min);
	int x_pos = (xsz - 1) * x_k;
	if (x_pos >= xsz)
		return false;
	while (x_units_ptr[x_pos] > x_unit && x_pos > 0)
		x_pos--;
	while (x_units_ptr[x_pos + 1] < x_unit && x_pos < xsz - 1)
		x_pos++;
	if (x_pos < 0 || x_pos >= xsz - 1 || x_units_ptr[x_pos] > x_unit || x_units_ptr[x_pos + 1] < x_unit)
		return false;
	if (x_units_ptr[x_pos + 1] == x_units_ptr[x_pos])
	{
		y_unit = y_units_ptr[x_pos];
		return true;
	}
	if (y_units_ptr[x_pos + 1] == GRF_NAN || y_units_ptr[x_pos] == GRF_NAN)
		return false;
	x_k = (x_unit - x_units_ptr[x_pos]) / (x_units_ptr[x_pos + 1] - x_units_ptr[x_pos]);
	y_unit = y_units_ptr[x_pos] + x_k * (y_units_ptr[x_pos + 1] - y_units_ptr[x_pos]);
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// GRF_Main
////////////////////////////////////////////////////////////////////////////////
GRF_Main::GRF_Main()
{
	m_show_legenda = false;
	m_mouse_left_button_down = false;
	m_mouse_x = m_mouse_y = -1;
	SetFieldPosition(0, 0, 100, 100);
	m_visible_horizontal_scale = NULL;
	m_visible_vertical_scale = NULL;


	m_s_black_color = NULL;
	m_s_black_brush = NULL;
	m_s_white_color = NULL;
	m_s_white_brush = NULL;

	m_s_axis_color = NULL;
	m_s_axis_brush = NULL;
	m_s_mouse_lines_color = NULL;
	m_s_mouse_lines_brush = NULL;
	m_s_grid_lines_color = NULL;
	m_s_grid_lines_brush = NULL;

	m_s_font = NULL;
}

GRF_Main::~GRF_Main()
{
	Cleanup();
}

void GRF_Main::Cleanup()
{
	m_visible_horizontal_scale = NULL;
	m_visible_vertical_scale = NULL;

	ElasticBuffer_DeletePointerBuffer(m_horizontal_scales);
	ElasticBuffer_DeletePointerBuffer(m_horizontal_arrays);
	ElasticBuffer_DeletePointerBuffer(m_vertical_scales);
	ElasticBuffer_DeletePointerBuffer(m_traces);
}

void GRF_Main::AddHorizontalScale(GRF_Scale* h_scale)
{
	if (m_horizontal_scales.GetFilledSize() == 0)
		SetVisibleHorizontalScale(h_scale);
	m_horizontal_scales.AddValue(h_scale);
}

void GRF_Main::AddHorizontalArray(GRF_Array* h_array)
{
	m_horizontal_arrays.AddValue(h_array);
}

void GRF_Main::AddVerticalScale(GRF_Scale* v_scale)
{
	if (m_vertical_scales.GetFilledSize() == 0)
		SetVisibleVerticalScale(v_scale);
	m_vertical_scales.AddValue(v_scale);
}

GRF_Trace* GRF_Main::AddTrace(GRF_Array* v_array/*новое и уникальное*/, GRF_Array* h_array/*уже добавленное*/, GRF_Scale* v_scale/*уже добавленное*/, DWORD color_rgba, float width, CStringW name, CStringW legenda_name)
{
	GRF_Trace* new_trace = new GRF_Trace();
	new_trace->m_vertical_data = v_array;
	new_trace->m_vertical_data->m_scale = v_scale;
	new_trace->m_horizontal_data = h_array;
	new_trace->m_color_rgba = color_rgba;
	new_trace->m_name = name;
	new_trace->m_legenda_name = legenda_name;
	new_trace->m_width = width;
	
	m_traces.AddValue(new_trace);
	return new_trace;
}

void GRF_Main::SetFieldPosition(int x, int y, int w, int h)
{
	m_field_x = x;
	m_field_y = y;
	m_field_w = w;
	m_field_h = h;

	int i, isz;
	GRF_Scale **scale_ptr;

	scale_ptr = m_horizontal_scales.GetBuff();
	isz = m_horizontal_scales.GetFilledSize();
	for (i = 0; i < isz; i++, scale_ptr++)
		(*scale_ptr)->SetScale(w);

	scale_ptr = m_vertical_scales.GetBuff();
	isz = m_vertical_scales.GetFilledSize();
	for (i = 0; i < isz; i++, scale_ptr++)
		(*scale_ptr)->SetScale(h);
}

void GRF_Main::SetHScalePosition(int y, int size)
{
	m_h_scale_pos = y;
	m_h_scale_size = size;
}

void GRF_Main::SetVScalePosition(int x, int size)
{
	m_v_scale_pos = x;
	m_v_scale_size = size;
}

void GRF_Main::SetVisibleHorizontalScale(GRF_Scale *h_scale) // по умолчанию первая добавленная шкала становится видимой
{
	m_visible_horizontal_scale = h_scale;
}

void GRF_Main::SetVisibleVerticalScale(GRF_Scale *v_scale) // по умолчанию первая добавленная шкала становится видимой
{
	m_visible_vertical_scale = v_scale;
}

////////////////////////////////////////////////////////////////////////////////
// рисование >>
////////////////////////////////////////////////////////////////////////////////
static char g_s_GRF_black_color_name[] = "GRF::black_color";
static char g_s_GRF_black_brush_name[] = "GRF::black_brush";
static char g_s_GRF_white_color_name[] = "GRF::white_color";
static char g_s_GRF_white_brush_name[] = "GRF::white_brush";

static char g_s_GRF_axis_color_name[] = "GRF::axis_color";
static char g_s_GRF_axis_brush_name[] = "GRF::axis_brush";
static char g_s_GRF_mouse_lines_color_name[] = "GRF::mouse_lines_color";
static char g_s_GRF_mouse_lines_brush_name[] = "GRF::mouse_lines_brush";
static char g_s_GRF_grid_lines_color_name[] = "GRF::grid_lines_color";
static char g_s_GRF_grid_lines_brush_name[] = "GRF::grid_lines_brush";

static char g_s_GRF_font_name[] = "GRF::font";

void GRF_Main::PrepareToDraw()
{
	m_s_black_color = m_dc->CheckColor(g_s_GRF_black_color_name, 0, 0, 0, 255);
	m_s_black_brush = m_dc->CheckBrush(g_s_GRF_black_brush_name, m_s_black_color, 1);

	m_s_white_color = m_dc->CheckColor(g_s_GRF_white_color_name, 255, 255, 255, 255);
	m_s_white_brush = m_dc->CheckBrush(g_s_GRF_white_brush_name, m_s_white_color, 1);

	m_s_axis_color = m_dc->CheckColor(g_s_GRF_axis_color_name, 120, 120, 120, 255);
	m_s_axis_brush = m_dc->CheckBrush(g_s_GRF_axis_brush_name, m_s_axis_color, 1);

	m_s_mouse_lines_color = m_dc->CheckColor(g_s_GRF_mouse_lines_color_name, 180, 180, 180, 100);
	m_s_mouse_lines_brush = m_dc->CheckBrush(g_s_GRF_mouse_lines_brush_name, m_s_mouse_lines_color, 1);

	m_s_grid_lines_color = m_dc->CheckColor(g_s_GRF_grid_lines_color_name, 120, 120, 120, 100);
	m_s_grid_lines_brush = m_dc->CheckBrush(g_s_GRF_grid_lines_brush_name, m_s_grid_lines_color, 1);
	
	m_s_font = m_dc->CheckFont(g_s_GRF_font_name, L"Calibri", 15, false, false);

	//if (m_dc->m_selected_font == NULL)
	m_dc->SelectFont(m_s_font);
}

void GRF_Main::DrawField()
{
	if (m_visible_horizontal_scale == NULL || m_visible_vertical_scale == NULL)
		return;
}

CStringW NumberWOZeros(double val, int visible_digits)
{
	CStringW format;
	format.Format(L"%%.%dlf", visible_digits);
	CStringW text;
	text.Format(format, val);
	WCHAR *ptr = text.GetBuffer();
	int len = text.GetLength();
	while (ptr[len - 1] == L'0')
		len--;
	if (ptr[len - 1] == L'.')
		len--;
	text = text.Left(len);
	if (text.Compare(L"-0") == 0)
		text = L"0";
	return text;
}

inline int GRF_ROUND(double x)
{
	int sign = (x >= 0)? +1 : -1;
	x *= sign;
	int int_x = int(x);
	if (x - int_x > 0.5)
		return sign*(int_x + 1);
	else
		return sign*int_x;
}

void GRF_Main::DrawScaleRisk(GRF_Scale* scale, int scale_height, float unit, float text_rotate, int text_x_align, int text_y_align, int grid_line_length, bool fill_bg)
{
	int pos = GRF_ROUND(scale->GetCoord(unit));
	if (fill_bg)
	{
		m_dc->FillRect(pos-9, 0, 18, scale_height, m_s_white_brush);
		G2D_Brush* prev = m_dc->m_current_brush;
		m_dc->SelectBrush(m_s_axis_brush);
		m_dc->DrawRect(pos-10, -1, 20, scale_height+2);
		m_dc->SelectBrush(prev);
	}
	m_dc->FillRect(pos, 0, 1, RISKA_SZ, m_s_axis_brush);
	m_dc->FillRect(pos, -grid_line_length, 1, grid_line_length, m_s_grid_lines_brush);
	CStringW text = NumberWOZeros(unit, scale->m_visible_digits);
	int off = RISKA_SZ + 2;
	m_dc->Rotate(text_rotate, pos, off);
	m_dc->SelectBrush(m_s_axis_brush);
	m_dc->DrawText(pos, off, text.GetBuffer(), text_x_align, text_y_align);
	m_dc->Rotate(-text_rotate, pos, off);	
}

void GRF_Main::DrawScale(int length, int height, GRF_Scale* scale, float text_rotate, int text_align, int grid_line_length, int mouse_risk_pos)
{
	if (scale == NULL)
		return;

	CStringW text;

	m_dc->SelectBrush(m_s_black_brush);
	// первая риска
	DrawScaleRisk(scale, height, scale->m_start_unit, text_rotate, text_align, -text_align, grid_line_length);

	// промежуточные риски
	float units_per_risk = scale->m_units_per_risk;
	int step = 0;
	while (scale->GetCoord(scale->m_start_unit + units_per_risk) > 50)
	{
		units_per_risk /= ((step%2) == 0) ? scale->m_risk_splitting1 : scale->m_risk_splitting2;
		step++;
	}
	while (scale->GetCoord(scale->m_start_unit + units_per_risk) < 25)
	{
		units_per_risk *= ((step%2) == 0) ? scale->m_risk_splitting1 : scale->m_risk_splitting2;
		step++;
	}

	int num = (scale->m_start_unit - scale->m_available_start_unit) / units_per_risk;
	num++;
	double unit = scale->m_available_start_unit + num * units_per_risk;
	while (unit < scale->m_final_unit)
	{
		if (scale->GetCoord(unit) - scale->GetCoord(scale->m_start_unit) > 20 &&
			scale->GetCoord(scale->m_final_unit) - scale->GetCoord(unit) > 20
			)
			DrawScaleRisk(scale, height, unit, text_rotate, text_align, 0, grid_line_length);
		num++;
		unit = scale->m_available_start_unit + num * units_per_risk;
	}

	// последняя риска
	DrawScaleRisk(scale, height, scale->m_final_unit, text_rotate, text_align, text_align, grid_line_length);

	// мышиная риска
	if (mouse_risk_pos != -1)
	{
		m_dc->SelectBrush(m_s_axis_brush);
		DrawScaleRisk(scale, height, scale->GetUnit(mouse_risk_pos), text_rotate, text_align, 0, 0, true);
	}
	// риски последних данных
	/**/
	GRF_Trace **trace_ptr = m_traces.GetBuff(), *trace;
	GRF_Array* array_ptr;
	int i, isz = m_traces.GetFilledSize();
	for (i = 0; i < isz; i++, trace_ptr++)
	{
		trace = *trace_ptr;
		if (trace->m_last_pos != -1)
		{
			m_dc->SelectBrush(trace->m_brush);
			array_ptr = (trace->m_horizontal_data->m_scale == scale) ? trace->m_horizontal_data : trace->m_vertical_data;
			DrawScaleRisk(scale, height, *(array_ptr->m_points.GetBuff() + trace->m_last_pos), text_rotate, text_align, 0, 0, true);
		}
	}
	/**/

	// название [юниты]
	m_dc->SelectBrush(m_s_axis_brush);
	text.Format(L"%s [%s]", scale->m_scale_name, scale->m_unit_name);
	m_dc->DrawText(length * 0.5, height, text.GetBuffer(), 0, -1);
}

void GRF_Main::DrawScales()
{
	// рисуем горизонтальную шкалу >>
	m_dc->OffsetOrigin(m_field_x, m_h_scale_pos);
	DrawScale(m_field_w - 1, m_h_scale_size - 1, m_visible_horizontal_scale, -90, -1, m_field_h, (m_mouse_x == -1) ? -1 : m_mouse_x - m_field_x);
	m_dc->OffsetOrigin(-m_field_x, -m_h_scale_pos);
	// рисуем горизонтальную шкалу <<

	// рисуем вертикальную шкалу >>
	m_dc->OffsetOrigin(m_v_scale_pos, m_field_y + m_field_h);
	m_dc->Rotate(-90, 0, 0);
	DrawScale(m_field_h - 1, m_v_scale_size - 1, m_visible_vertical_scale, 90, +1, m_field_w, (m_mouse_y == -1) ? -1 : m_field_h - (m_mouse_y - m_field_y));
	m_dc->Rotate(90, 0, 0);
	m_dc->OffsetOrigin(-m_v_scale_pos, -(m_field_y + m_field_h));
	// рисуем вертикальную шкалу <<
}

void GRF_Main::DrawTraceLast(GRF_Trace* trace)
{
	if (trace->m_last_pos < 0 || !trace->m_visible)
		return;
	
	TYPE x = trace->m_horizontal_data->m_scale->GetCoord(*(trace->m_horizontal_data->m_points.GetBuff() + trace->m_last_pos)),
		y = trace->m_vertical_data->m_scale->GetCoord(*(trace->m_vertical_data->m_points.GetBuff() + trace->m_last_pos));
	if (x >= 0 && x <= m_field_w)
		m_dc->FillRect(x, 0, 1, m_field_h + 1 + RISKA_SZ, trace->m_last_brush);
	if (y >= 0 && y <= m_field_h)
		m_dc->FillRect(0, m_field_h - y, m_field_w + 1 + RISKA_SZ, 1, trace->m_last_brush);
}

void GRF_Main::DrawTraceSP(GRF_Trace* trace)
{
	if (!trace->m_visible)
		return;

	GRF_Scale
		*h_scale = trace->m_horizontal_data->m_scale,
		*v_scale = trace->m_vertical_data->m_scale;
	CStringW text;
	double x, y, xx, yy, radius = trace->m_draw_selected_point_size, diameter = 2 * radius;
	int x_sgn, y_sgn;
	double *x_ptr = trace->m_selected_points.GetBuff();
	int i, isz = trace->m_selected_points.GetFilledSize();
	for (i = 0; i < isz; i++, x_ptr++)
	{
		x = *x_ptr;
		if (x != GRF_NAN && trace->GetUnitByUnit(x, y))
		{
			xx = h_scale->GetCoord(x);
			yy = v_scale->GetCoord(y);
			m_dc->SelectBrush(trace->m_brush);
			m_dc->FillEllipse(xx - radius, m_field_h - yy - radius, diameter, diameter, trace->m_brush);
			text = NumberWOZeros(y, 3);
			x_sgn = (xx < m_field_w / 2) ? +1 : -1;
			y_sgn = (yy < m_field_h / 2) ? -1 : +1;
			m_dc->DrawTextWithRect(xx, m_field_h - (yy - y_sgn * radius), 10, 0, text.GetBuffer(), m_s_white_color, trace->m_brush, x_sgn, y_sgn);
		}
	}
}

void GRF_Main::DrawTrace(GRF_Trace* trace)
{
	int i, isz = trace->m_vertical_data->m_points.GetFilledSize();
	if (isz == 0 || !trace->m_visible)
		return;

	m_dc->SelectBrush(trace->m_brush);

	double *x_buff = trace->m_horizontal_data->m_points.GetBuff(), *y_byff = trace->m_vertical_data->m_points.GetBuff();
	double *y_ptr, *x_start_ptr, *x_final_ptr, *x_ptr, x, y;

	GRF_Scale
		*h_scale = trace->m_horizontal_data->m_scale,
		*v_scale = trace->m_vertical_data->m_scale;
	int start_pos = 0, final_pos = 0;

// TBD определить зону видимости
	// определяем границы видимости по x
	i = 0;
	x_start_ptr = x_buff;
	while (*x_start_ptr < h_scale->m_start_unit && i < isz)
	{
		x_start_ptr++;
		i++;
	}
	if (x_start_ptr > trace->m_horizontal_data->m_points.GetBuff())
	{
		x_start_ptr--;
		i--;
	}
	start_pos = i;
	x_final_ptr = x_start_ptr;
	while (*x_final_ptr < h_scale->m_final_unit && i < isz)
	{
		x_final_ptr++;
		i++;
	}
	if (i == isz)
	{
		x_final_ptr--;
		i--;
	}
	final_pos = i;

	i = start_pos;
	x_ptr = x_start_ptr;
	y_ptr = y_byff + start_pos;
	bool started = false, draw_points = (trace->m_points_sz > 0);
	double radius = trace->m_points_sz, diameter = 2 * radius;
	while (i < final_pos + 1)
	{
		if (*y_ptr == GRF_NAN)
		{
			started = false;
		}
		else
		{
			x = h_scale->GetCoord(*x_ptr);
			y = m_field_h - v_scale->GetCoord(*y_ptr);

			if (started)
				m_dc->LineTo(x, y);
			else
			{
				m_dc->MoveTo(x, y);
				started = true;
			}
			if (draw_points)
			{
				if (trace->m_points_mode == 0)
					m_dc->FillEllipse(x - radius, y - radius, diameter, diameter, trace->m_brush);
				else
					m_dc->FillRect(x - radius, y - radius, diameter, diameter, trace->m_brush);
			}
		}
		i++;
		x_ptr++;
		y_ptr++;
	}
}

void GRF_Main::Draw()
{
	// сдвиги
	if (((m_dc->m_interface_object->GetFlag() & MK_CONTROL) == 0) && m_mouse_left_button_down && m_dc->m_interface_object->IsLeftButtonDown())
	{
		CPoint pt = m_dc->m_interface_object->GetMousePos();
		GRF_Scale* scale;
		int abs_delta_pos, sign_delta_pos;
		double delta_unit, start_unit_offset, final_unit_offset;
		// x
		abs_delta_pos = abs(pt.x - m_left_click_pos.x);
		if (abs_delta_pos > 0)
		{
			scale = m_visible_horizontal_scale;

			sign_delta_pos = pt.x > m_left_click_pos.x ? -1 : +1;
			delta_unit = scale->GetUnit(abs_delta_pos) - scale->GetUnit(0);
			start_unit_offset = max(scale->m_available_start_unit, scale->m_start_unit + sign_delta_pos * delta_unit) - scale->m_start_unit;
			final_unit_offset = min(scale->m_available_final_unit, scale->m_final_unit + sign_delta_pos * delta_unit) - scale->m_final_unit;
			delta_unit = fabs(start_unit_offset) < fabs(final_unit_offset) ? start_unit_offset : final_unit_offset;
			scale->m_start_unit += delta_unit;
			scale->m_final_unit += delta_unit;
		}
		// y
		abs_delta_pos = abs(pt.y - m_left_click_pos.y);
		if (abs_delta_pos > 0)
		{
			scale = m_visible_vertical_scale;

			sign_delta_pos = pt.y > m_left_click_pos.y ? +1 : -1;
			delta_unit = scale->GetUnit(abs_delta_pos) - scale->GetUnit(0);
			start_unit_offset = max(scale->m_available_start_unit, scale->m_start_unit + sign_delta_pos * delta_unit) - scale->m_start_unit;
			final_unit_offset = min(scale->m_available_final_unit, scale->m_final_unit + sign_delta_pos * delta_unit) - scale->m_final_unit;
			delta_unit = fabs(start_unit_offset) < fabs(final_unit_offset) ? start_unit_offset : final_unit_offset;
			scale->m_start_unit += delta_unit;
			scale->m_final_unit += delta_unit;
		}
		//
		m_left_click_pos = pt;
	}

	PrepareToDraw();

	int i, isz = m_traces.GetFilledSize();
	GRF_Trace** trace_ptr = m_traces.GetBuff();
	for (i = 0; i < isz; i++, trace_ptr++)
		(*trace_ptr)->CheckGraphObjects(m_dc);

	// рисуем мышиные линии
	if (m_mouse_x > 0)
		m_dc->FillRect(m_mouse_x, m_field_y, 1, m_field_h + m_h_scale_size, m_s_mouse_lines_brush);
	if (m_mouse_y > 0)
		m_dc->FillRect(m_field_x, m_mouse_y, m_field_w + m_v_scale_size, 1, m_s_mouse_lines_brush);

	// рисуем линии последних данныx >>
	{
		m_dc->OffsetOrigin(m_field_x, m_field_y);
		trace_ptr = m_traces.GetBuff();
		for (i = 0; i < isz; i++, trace_ptr++)
			DrawTraceLast(*trace_ptr);
		m_dc->OffsetOrigin(-m_field_x, -m_field_y);
	}
	// рисуем линии последних данныx <<

	DrawField();
	DrawScales();

	// рисуем данные >>
	{
		m_dc->OffsetOrigin(m_field_x, m_field_y);
		m_dc->ClipRect(0, 0, m_field_w, m_field_h);
		trace_ptr = m_traces.GetBuff();
		for (i = 0; i < isz; i++, trace_ptr++)
			DrawTrace(*trace_ptr);
		trace_ptr = m_traces.GetBuff();
		for (i = 0; i < isz; i++, trace_ptr++)
			DrawTraceSP(*trace_ptr);
		m_dc->UnClip();
		m_dc->OffsetOrigin(-m_field_x, -m_field_y);
	}
	// рисуем данные <<

	// обводка поля
	m_dc->SelectBrush(m_s_axis_brush);
	m_dc->DrawRect(m_field_x, m_field_y, m_field_w, m_field_h);

	if (m_dc->m_interface_object->GetFlag() & MK_CONTROL)
	{
		if (m_dc->m_interface_object->IsLeftButtonDown())
			AddSelectedPoint(0);
		if (m_dc->m_interface_object->IsRightButtonDown())
			AddSelectedPoint(1);
	}
	
	// legenda
	if (m_show_legenda || m_dc->m_interface_object->IsRightButtonDown())
	{
		int mouse_x = m_mouse_x - m_field_x, font_h = abs(m_dc->m_selected_font->m_height);
		bool show_val = m_dc->m_interface_object->IsRightButtonDown() && mouse_x >= 0 && mouse_x < m_field_w;
		GRF_Trace* trace;
		CStringW text;
		double y_unit = 0, x_unit = m_visible_horizontal_scale->GetUnit(mouse_x);
		int x = m_field_x + 5, y = m_field_y + 5 + m_dc->m_selected_font->m_height / 2;

		// пишем значение по X
		if (show_val)
		{
			text.Format(L"%s = %.2lf", m_visible_horizontal_scale->m_scale_name, x_unit);
			m_dc->DrawText(x, y, text.GetBuffer(), +1, 0);
			y += font_h;
		}

		for (i = 0, trace_ptr = m_traces.GetBuff(); i < isz; i++, trace_ptr++)
		{
			trace = *trace_ptr;
			if (trace->m_visible)
			{
				m_dc->SelectBrush(trace->m_brush);
				m_dc->FillRect(x, y, 20, 1, trace->m_brush);
				text = L"";
				if (show_val && trace->GetUnitByUnit(x_unit, y_unit) && y_unit != GRF_NAN)
					text.Format(L" = %.2lf", y_unit);
				text = trace->m_legenda_name + text;
				m_dc->DrawText(x + 25, y, text.GetBuffer(), +1, 0);
				y += font_h;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// рисование <<
////////////////////////////////////////////////////////////////////////////////

void GRF_Main::ScalingScale(GRF_Scale* scale, int pos, int sz, int dir)
{
	scale->MemorizeAvailableUnits(); // на всякий случай

	double k = (dir > 0 ? 0.9 : 1.1);
	double old_unit = scale->GetUnit(pos);
	double new_start_unit = old_unit - (old_unit - scale->m_start_unit)*k;
	double new_final_unit = old_unit + (scale->m_final_unit - old_unit)*k;

	scale->m_start_unit = max(scale->m_available_start_unit, new_start_unit);
	scale->m_final_unit = min(scale->m_available_final_unit, new_final_unit);
	scale->SetScale(sz);
}

void GRF_Main::OnMouseWheel()
{
	int wheel_dir = m_dc->m_interface_object->GetWheelDirection();

	if (m_mouse_x != -1 && m_visible_horizontal_scale)
		ScalingScale(m_visible_horizontal_scale, m_mouse_x - m_field_x, m_field_w, wheel_dir);
	if (m_mouse_y != -1 && m_visible_vertical_scale)
		ScalingScale(m_visible_vertical_scale, m_field_h - (m_mouse_y - m_field_y), m_field_h, wheel_dir);
}

void GRF_Main::ClearSelectedPoints()
{
	int i, isz = m_traces.GetFilledSize();
	GRF_Trace *trace, **trace_ptr;
	for (i = 0, trace_ptr = m_traces.GetBuff(); i < isz; i++, trace_ptr++)
	{
		trace = *trace_ptr;
		trace->m_selected_points.Clear();
	}
}

void GRF_Main::OnMiddleButton(bool down)
{
	if (!down)
		return;

	// чистим точки >>
	if (m_dc->m_interface_object->GetFlag() & MK_CONTROL)
	{
		ClearSelectedPoints();
		return;
	}
	// чистим точки <<

	if (m_mouse_x != -1 && m_visible_horizontal_scale)
	{
		m_visible_horizontal_scale->m_start_unit = m_visible_horizontal_scale->m_available_start_unit;
		m_visible_horizontal_scale->m_final_unit = m_visible_horizontal_scale->m_available_final_unit;
	}
	if (m_mouse_y != -1 && m_visible_vertical_scale)
	{
		m_visible_vertical_scale->m_start_unit = m_visible_vertical_scale->m_available_start_unit;
		m_visible_vertical_scale->m_final_unit = m_visible_vertical_scale->m_available_final_unit;
	}
}

void GRF_Main::AddSelectedPoint(int num)
{
	int i, isz = m_traces.GetFilledSize(), sz;
	double x_unit = m_visible_horizontal_scale->GetUnit(m_mouse_x - m_field_x), y_unit, *x_ptr;
	GRF_Trace *trace, **trace_ptr;
	for (i = 0, trace_ptr = m_traces.GetBuff(); i < isz; i++, trace_ptr++)
	{
		trace = *trace_ptr;
		if (trace->m_visible)
		{
			if (trace->GetUnitByUnit(x_unit, y_unit) && y_unit != GRF_NAN)
			{
				sz = trace->m_selected_points.GetFilledSize();
				if (num >= sz)
				{
					trace->m_selected_points.UpdateSize(num + 1);
					x_ptr = trace->m_selected_points.GetBuff() + sz;
					while (sz < num + 1)
					{
						*x_ptr = GRF_NAN;
						x_ptr++;
						sz++;
					}
					trace->m_selected_points.SetFilledSize(sz);
				}
				*(trace->m_selected_points.GetBuff() + num) = x_unit;
			}
		}
	}
}

void GRF_Main::OnRightButton(bool down)
{
	if (m_dc->m_interface_object->GetFlag() & MK_CONTROL)
		AddSelectedPoint(1);
}

void GRF_Main::OnLeftButton(bool down)
{
	m_mouse_left_button_down = down;
	m_left_click_pos = m_dc->m_interface_object->GetMousePos();
	
	if (m_dc->m_interface_object->GetFlag() & MK_CONTROL)
		AddSelectedPoint(0);
}

////////////////////////////////////////////////////////////////////////////////

void GRF_SetupIntArray(GRF_Array* grf_array, int max_num)
{
	//if (grf_array->m_points.GetFilledSize() == max_num)
	//	return;
	grf_array->m_points.UpdateSize(max_num, false);
	grf_array->m_points.SetFilledSize(max_num);
	double *ptr = grf_array->m_points.GetBuff();
	for (int i = 0; i < max_num; i++, ptr++)
		*ptr = i;
}

////////////////////////////////////////////////////////////////////////////////
// end