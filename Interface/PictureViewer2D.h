// uic 2014.04.28

#pragma once;

#include "InterfaceObject2D.h"

struct PictureViewer2D;
typedef void (*PictureViewer2DFuntion)(PictureViewer2D*);
typedef bool (*PictureViewer2DBoolFuntion)(PictureViewer2D*);

enum PictureViewer2D_Mode
{
	PictureViewer2D_idle = 0,
	PictureViewer2D_pressed,
	PictureViewer2D_current,
};

struct PictureViewer2D_BitmapWrap
{
	PictureViewer2D_BitmapWrap(): m_bmp_bitmap(NULL), m_bmp_2d(NULL), m_bmp_obj_index(-1),
		m_offset_x(0), m_offset_y(0)
	{
	}

	~PictureViewer2D_BitmapWrap()
	{
		if (m_bmp_bitmap)
			delete m_bmp_bitmap;
		if (m_bmp_2d)
			delete m_bmp_2d;
	}

	float m_offset_x, m_offset_y;
	int m_w, m_h;
	BMP_Bitmap* m_bmp_bitmap;
	G2D_Bitmap* m_bmp_2d;
	int m_bmp_obj_index;
};

struct PictureViewer2D: InterfaceObject2D
{
	PictureViewer2D();
	virtual ~PictureViewer2D();

	virtual void Cleanup();

	void SetGrayscaleData(void* data, int w, int h, int bpp = 8);
	void SetBitmap(BMP_Bitmap* bmp);

	void UpdatePalette();
	void SetPaletteColor(byte colornum, byte r, byte g, byte b);
	void ClearPalette();
	RGBQUAD m_palette[256];
	bool m_changed_palette;

	void SetScales(float idle, float pressed); // <0 - не меняем

	void CenterPicture();

	void* GetData() { return m_data; }

	double GetCurrentScale() { return m_current_scale; }
	double GetPressedScale() { return m_pressed_scale; }
	double GetIdleScale() { return m_idle_scale; }
	
	void SetPressedScale(double scale) { m_pressed_scale = scale; if (m_pressed) m_current_scale = scale; }
	void SetIdleScale(double scale) { m_default_scale = m_idle_scale = scale; if (!m_pressed) m_current_scale = scale; }
	void SetDefaultScale(double scale) { m_default_scale = scale; }
	double GetDefaultScale() { return m_default_scale; }
	
	float GetCurrentPictureX() { return m_current_picture_x; }
	float GetCurrentPictureY() { return m_current_picture_y; }

	int GetPictureW() { return m_picture_width; }
	int GetPictureH() { return m_picture_height; }

	void CorrectPicturePosition();

	G2D_Bitmap* GetBitmap2D() { return m_bmp_2d; }

	PictureViewer2DFuntion m_after_draw_2d_function;
	PictureViewer2DFuntion m_init_2d_function;
	PictureViewer2DBoolFuntion m_on_mouse_function;

	void ScreenToPicture(float sx, float sy, float& px, float &py, PictureViewer2D_Mode mode = PictureViewer2D_current);
	void PictureToScreen(float px, float py, float& sx, float &sy, PictureViewer2D_Mode mode = PictureViewer2D_current);

//	CMutex m_objects_mutex;
//	void LockObjectsMutex() { m_objects_mutex.Lock(); }
//	void UnlockObjectsMutex() { m_objects_mutex.Unlock(); }

	G2D_Bitmap* m_bmp_2d;
	G2D_Font *m_font1;
	G2D_Color *m_color_white, *m_color_black, *m_color_frame;
	G2D_Brush *m_brush_white, *m_brush_black, *m_brush_frame;


	float m_picture_frame_width;
	byte m_picture_frame_color_r255, m_picture_frame_color_g255, m_picture_frame_color_b255;
	void OnSetBitmap(int additional_pos = -1);

	void SetShowZoomInfoParams(bool show_zoom, int pos_x, int pos_y) { m_show_zoom_info = show_zoom; m_zoom_info_x = pos_x; m_zoom_info_y = pos_y; }
	bool m_show_zoom_info, m_zoom_info_is_visible, m_invalidate_on_mouse_events;
	int m_zoom_info_x, m_zoom_info_y;
	virtual void OnTimer(UINT timer_id);
	void ShowZoomChange();

	void OnChangeScale(float screen_x_to_keep, float screen_y_to_keep, float old_scale, PictureViewer2D_Mode old_mode);
	void OnMouseMove();
	void OnLBDown();
	void OnLBUp();
	void OnMBDown();
	void OnOtherMouse();
	void OnWheel();

	double m_min_scale, m_max_scale, m_scale_change_koeff, m_scale_change_offset;

	virtual void OnCleanup();

	int GetAdditionalBmpNum() { return m_bmp_wraps.GetFilledSize(); }
	void SetAdditionalGrayscaleData(void* data, int w, int h, int bpp, int additional_num, float dx, float dy);

	ElasticBuffer<PictureViewer2D_BitmapWrap*> m_bmp_wraps; // дополнительные изображения
	void MultiDelete2D();
	void MultiDeleteBmp();
	void MultiNull2D();

protected:
	double m_current_scale, m_idle_scale, m_pressed_scale, m_default_scale;
	bool m_pressed, m_pressed_availale;

	float m_current_picture_x, m_current_picture_y,
		m_idle_picture_x, m_idle_picture_y, m_pressed_picture_x, m_pressed_picture_y;

	bool m_dragging;
	CPoint m_dragging_point;
	float m_dragging_pic_x, m_dragging_pic_y, m_dragging_idle_x, m_dragging_idle_y, m_dragging_pressed_x, m_dragging_pressed_y;

	void *m_data;
	int m_picture_width, m_picture_height;
	int m_prev_picture_width, m_prev_picture_height;
	BMP_Bitmap *m_bmp_bitmap;
	int m_bmp_obj_index;

	void MakeBmpOnSetGrayscale(void* data, int w, int h, int bpp, BMP_Bitmap **bmp_bitmap);
};

void PictureViewer2D_DrawPicture(InterfaceObject2D* obj2d);
void PictureViewer2D_InitObjects(InterfaceObject* obj);
extern float PictureViewer2D_angle, PictureViewer2D_x_scale, PictureViewer2D_y_scale;

////////////////////////////////////////////////////////////////////////////////
// end