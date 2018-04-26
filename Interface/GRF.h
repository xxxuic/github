// uic 2015.10.15

#include <ElasticBuffer.h>
#include "G2D.h"

#define GRF_NAN (double)(0xCDCDCDCD)

inline int GRF_ROUND(double x);

////////////////////////////////////////////////////////////////////////////////
// GRF_Scale
////////////////////////////////////////////////////////////////////////////////
// ��� ����:
//		����� ������ ��������� ������ (unit) � ������������ ��.���. (pixel)
//		������� ������� ������� �����������
//
//		������ ������������ ����� �����,
//		����� ������ ��������� �� ��� ����
////////////////////////////////////////////////////////////////////////////////
struct GRF_Scale
{
	GRF_Scale(CStringW scale_name, CStringW unit_name, double start_unit, double final_unit, double risk_unit);

	double GetCoord(double unit);
	double GetUnit(double coord);
	void SetScale(double size);

	CStringW m_scale_name, m_unit_name;
	double
		m_start_unit, // ��������� ����
		m_final_unit, // �������� ����
		m_pix_per_unit, // ������� (���� ��� ��������������� ����)
		m_available_start_unit, // �������� (���������� ���������) ��������� ����
		m_available_final_unit; // �������� (����������� ���������) �������� ����

	double m_units_per_risk; // ����� �� ����� ��������, ������� � m_available_start_unit
	int m_visible_digits, // ����� ����� ������� ��� ������
		m_risk_splitting1,
		m_risk_splitting2; // ������� ����� ��� ���������� ��������

	void MemorizeAvailableUnits();
};

////////////////////////////////////////////////////////////////////////////////
// GRF_Array
////////////////////////////////////////////////////////////////////////////////
// ��� ����:
//		�������� ������
//		����� �� ������
//		����� � �������������� ������� (��� ������������ ������)
////////////////////////////////////////////////////////////////////////////////
struct GRF_Array
{
	GRF_Array();

	//bool m_empty;
	ElasticBuffer<double> m_points;
	GRF_Scale *m_scale;
	
	void MakeEmpty(); // ��������� ���� ������ ������� �������
};

////////////////////////////////////////////////////////////////////////////////
// GRF_Trace
////////////////////////////////////////////////////////////////////////////////
// ��� ����:
//		������������ ������������ ������ � ������ � ������
////////////////////////////////////////////////////////////////////////////////
struct GRF_Trace
{
	GRF_Trace();
	~GRF_Trace();

	void CheckGraphObjects(G2D_DC* dc);
	G2D_Color *m_color, *m_last_color;
	G2D_Brush *m_brush, *m_last_brush;
	float m_width;

	bool GetUnitByUnit(double x_unit, double& y_unit);

	GRF_Array* m_vertical_data;
	GRF_Array* m_horizontal_data;
	DWORD m_color_rgba;	
	CStringW m_name, m_legenda_name;
	int m_last_pos; // ������� ��������� ������ ��� ���������
	bool m_visible;

	ElasticBuffer<double> m_selected_points;
	int m_draw_selected_point_mode, m_points_mode; // 0 - ����, 1 - �������
	double m_draw_selected_point_size, m_points_sz;

	int m_uniq_id;
	static int m_max_id;
};

////////////////////////////////////////////////////////////////////////////////
// GRF_Main
////////////////////////////////////////////////////////////////////////////////
// ��� ����:
//		�������� �����
//		���������, ��� ��� ������� (�������, ����� � ��.) ������� ��� ����
//		� �������� �� �����.
////////////////////////////////////////////////////////////////////////////////
struct GRF_Main
{
	GRF_Main();
	~GRF_Main();
	void Cleanup();

	void AddHorizontalScale(GRF_Scale* h_scale);
	void AddHorizontalArray(GRF_Array* h_array);

	void AddVerticalScale(GRF_Scale* v_scale);
	GRF_Trace* AddTrace(GRF_Array* v_array/*����� � ����������*/, GRF_Array* h_array/*��� �����������*/, GRF_Scale* v_scale/*��� �����������*/, DWORD color_rgba, float width, CStringW name, CStringW legenda_name);

	void SetFieldPosition(int x, int y, int w, int h);
	void SetHScalePosition(int y, int size);
	void SetVScalePosition(int x, int size);
	void SetVisibleHorizontalScale(GRF_Scale *h_scale); // �� ��������� ������ ����������� ����� ���������� �������
	void SetVisibleVerticalScale(GRF_Scale *v_scale); // �� ��������� ������ ����������� ����� ���������� �������

	// ��������� >>
	G2D_DC *m_dc;
	G2D_Color *m_s_black_color, *m_s_white_color, *m_s_axis_color, *m_s_mouse_lines_color, *m_s_grid_lines_color;
	G2D_Brush *m_s_black_brush, *m_s_white_brush, *m_s_axis_brush, *m_s_mouse_lines_brush, *m_s_grid_lines_brush;
	G2D_Font *m_s_font;
	
	virtual void PrepareToDraw();
	virtual void Draw();
	virtual void DrawField();
	virtual void DrawScales();
	virtual void DrawScale(int length, int height, GRF_Scale* scale, float text_rotate, int text_align, int grid_line_length, int mouse_risk_pos);
	virtual void DrawScaleRisk(GRF_Scale* scale, int scale_height, float unit, float text_rotate, int text_x_align, int text_y_align, int grid_line_length, bool fill_bg = false);
	virtual void DrawTrace(GRF_Trace* trace);
	virtual void DrawTraceSP(GRF_Trace* trace);
	virtual void DrawTraceLast(GRF_Trace* trace);
	// ��������� <<

	GRF_Scale* GetVisibleHorizontaScale() { return m_visible_horizontal_scale; }

	CRect GetField() { return CRect(m_field_x, m_field_y, m_field_x + m_field_w, m_field_y + m_field_h); }
	void SetMouseX(int x) { m_mouse_x = x; }
	void SetMouseY(int y) { m_mouse_y = y; }

	bool m_mouse_left_button_down;
	CPoint m_left_click_pos;
	virtual void OnMouseWheel();
	virtual void OnMiddleButton(bool down);
	virtual void OnLeftButton(bool down);
	virtual void OnRightButton(bool down);

	void ScalingScale(GRF_Scale* scale, int pos, int sz, int dir);
	void AddSelectedPoint(int num);
	void ClearSelectedPoints();

	bool m_show_legenda;
	
protected:
	int m_field_x, m_field_y, m_field_w, m_field_h, m_h_scale_pos, m_h_scale_size, m_v_scale_pos, m_v_scale_size;
	int m_mouse_x, m_mouse_y;
	GRF_Scale *m_visible_horizontal_scale, *m_visible_vertical_scale; // ������������ �����

	ElasticBuffer<GRF_Scale*> m_horizontal_scales; // ����� ��������������
	ElasticBuffer<GRF_Array*> m_horizontal_arrays; // �������������� ������
	ElasticBuffer<GRF_Scale*> m_vertical_scales; // ����� ������������
	ElasticBuffer<GRF_Trace*> m_traces; // ������ ��� �����������
};

void GRF_SetupIntArray(GRF_Array* grf_array, int max_num);

typedef GRF_Scale* GRF_ScalePtr;
typedef GRF_Array* GRF_ArrayPtr;
typedef GRF_Trace* GRF_TracePtr;

////////////////////////////////////////////////////////////////////////////////
// end