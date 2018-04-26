// uic 2018.04.20

#include "stdafx.h"

#include "./KaKaO.h"

#include "./MNP.h"
#include "./TXT.h"
#include "./ThreadWrap.h"

//#include "./Algorithm/EndoAlg.h"

#include "./Interface/Interface.h"
#include "./Interface/PictureViewer2D.h"
#include "./Interface/BmpButton.h"
#include "./Interface/CB.h"
#include "./Interface/CS.h"
#include "./Interface/LayoutedGroupBox.h"
#include "./Interface/VE.h"
#include "./Interface/BMP.h"

#include <math.h>
#define PI 3.1415692654
#include <vector>
using namespace std;

#include "./HARD/camera/camera.h"
static Camera_Interface* g_camera = NULL;
#include "./HARD/camera/DataFromFile/DataFromFile.h"

static Camera_Frame *g_last_frame = NULL;
static CMutex g_last_frame_mutex;

static DWORD g_frames_counter = 0;
static int g_w = -1, g_h = -1, g_bitcount = -1, g_max_brightness = -1;

static LayoutedGroupBox g_gb_test_speed("Тест скорости");
static int g_test_frames = 1000;
static VE<int> g_ve_test_frames;
static BmpButton g_btn_test_cpu, g_btn_test_gpu;

static struct LaserSpotInfo
{
	void Init(int coord1)
	{
		m_coord1 = coord1;
		m_coord2_start = 0;
		m_coord2_end = 0;
		m_coord2_cm = m_depth = -1;
	}
	void Draw(G2D_DC* dc, float dx, float dy, float scale);

	int m_coord1, m_coord2_start, m_coord2_end;
	double m_coord2_cm, m_depth;
};

static vector<LaserSpotInfo> g_spots;
static CMutex g_spots_mutex;

static void OnChangeFrameWidth()
{
	MutexWrap spots_access(g_spots_mutex);
	g_spots.resize(g_w);
	for (int i = 0; i < g_w; i++)
		g_spots[i].Init(i);
}

template <typename T> static void _CalcSpots(T *data)
{
	MutexWrap spots_access(g_spots_mutex);
	T* ptr;
	int x, y, start_y;
	double weighted_coordinate, total_weight, specific_weight, best_specific_weight;
	T br, thr = g_max_brightness * g_threashold;
	for (int x = 0; x < g_w; x++)
	{
		g_spots[x].m_coord2_cm = -1;

		start_y = -1;
		best_specific_weight = 0;

		ptr = data + x;
		for (y = 0; y < g_h; y++, ptr += g_w)
		{
			br = *ptr;
			if (br > thr)
			{
				if (start_y == -1) // начало пятна
				{
					start_y = y;
					weighted_coordinate = 0;
					total_weight = 0;
				}
				total_weight += br;
				weighted_coordinate += double(y) * br;
			}
			
			if ((br <= thr || y == g_h - 1) && start_y != -1) // конец пятна
			{
				specific_weight = total_weight / (y - start_y + 1);
				if (specific_weight > best_specific_weight)
				{
					best_specific_weight = specific_weight;
					g_spots[x].m_coord2_start = start_y;
					g_spots[x].m_coord2_end = y - 1;
					g_spots[x].m_coord2_cm = weighted_coordinate / total_weight;
				}	
				start_y = -1;
			}
		}
	}
}

static void CalcSpots(void *data)
{
	if (g_bitcount <= 8)
		_CalcSpots((byte*)data);
	else
	if (g_bitcount <= 16)
		_CalcSpots((WORD*)data);
	else
	if (g_bitcount == 32)
		_CalcSpots((float*)data);
}

////////////////////////////////////////////////////////////////////////////////
// элементы интерфейса
////////////////////////////////////////////////////////////////////////////////
static InterfaceForm g_form;
static CWnd *g_wnd = NULL;

static float g_threashold = 0.3f, g_move_step = 1000, g_base_pos = 0.3f;
static VE<float> g_ve_threashold, g_ve_base_pos;
//static CS g_cs_coords;

static bool g_draw_base = false, g_marzhouser_joystick_mode = false, g_calc_spots = true;
static CB g_cb_draw_base, g_cb_calc_spots;

//static BmpButton g_btn_move[4], g_btn_calibration;

static PictureViewer2D g_frames_viewer;

////////////////////////////////////////////////////////////////////////////////
static void UpdateControls()
{
	
}

////////////////////////////////////////////////////////////////////////////////
static G2D_Color *g_color_white = NULL, *g_color_circles = NULL, *g_color_circles2 = NULL, *g_color_base = NULL;
static G2D_Brush *g_bold_white_brush = NULL, *g_brush_circles = NULL, *g_brush_circles2 = NULL, *g_brush_base = NULL;

static void InitObjectsForViewer(InterfaceObject* obj)
{
	PictureViewer2D_InitObjects(obj);
	PictureViewer2D* pv = (PictureViewer2D*) obj;

	G2D_DC* dc = pv->m_dc;
	g_color_white = dc->CheckColor("color white", 0xFF, 0xFF, 0xFF, 0xFF);
	g_bold_white_brush = dc->CheckBrush("bold white brush", g_color_white, 3);
	g_color_circles = dc->CheckColor("circles color", 255, 127, 127, 100);
	g_brush_circles = dc->CheckBrush("circles brush", g_color_circles, 2);

	g_color_circles2 = dc->CheckColor("circles2 color", 255, 0, 0, 100);
	g_brush_circles2 = dc->CheckBrush("circles2 brush", g_color_circles2, 2);

	g_color_base = dc->CheckColor("base color", 0, 200, 100, 150);
	g_brush_base = dc->CheckBrush("base brush", g_color_base, 2);

//	SubInterface_OnInitFunction(pv);
}

void LaserSpotInfo::Draw(G2D_DC* dc, float dx, float dy, float scale)
{

	if (scale > 2)
		dc->DrawRect(dx + scale * m_coord1, dy + scale * m_coord2_start, scale, scale * (m_coord2_end - m_coord2_start + 1));
	else
		dc->FillRect(dx + scale * m_coord1, dy + scale * m_coord2_start, scale, scale * (m_coord2_end - m_coord2_start + 1), g_color_circles);

	dc->FillRect(dx + scale * m_coord1, dy + scale * m_coord2_cm, scale, scale, g_color_circles2);
}

static void DrawInFrameViewer(InterfaceObject2D* obj2d)
{
	PictureViewer2D* pv2d = (PictureViewer2D*)obj2d;
	G2D_DC *dc = pv2d->m_dc;

	PictureViewer2D_DrawPicture(obj2d);
	
	float x1, y1, x2, y2, scale = pv2d->GetCurrentScale();
	pv2d->PictureToScreen(0, 0, x1, y1);
	pv2d->PictureToScreen(pv2d->GetPictureW(), pv2d->GetPictureH(), x2, y2);
	
	dc->SelectBrush(g_brush_circles);
	{
		MutexWrap spots_access(g_spots_mutex);
		for (int i = 0; i < g_w; i++)
			if (g_spots[i].m_coord2_cm > 0)
				g_spots[i].Draw(dc, x1, y1, scale);
	}

	dc->SelectBrush(g_brush_circles2);
	{
		MutexWrap spots_access(g_spots_mutex);
		bool line_started = false;
		for (int i = 0; i < g_w; i++)
			if (g_spots[i].m_depth > 0)
			{
				pv2d->PictureToScreen(i, pv2d->GetPictureH() * 0.5 + g_spots[i].m_depth, x1, y1);
				if (!line_started)
				{
					dc->MoveTo(x1, y1);
					line_started = true;
				}
				else
					dc->LineTo(x1, y1);
			}
			else
				line_started = false;
	}

	if (g_draw_base)
	{
		float x3, y3;
		pv2d->PictureToScreen(0, int(g_base_pos * pv2d->GetPictureH()), x3, y3);
		dc->FillRect(x1, y3, x2 - x1, scale, g_brush_base);
	}

//	if (g_subinterface)
//		g_subinterface->MainDraw();
}

////////////////////////////////////////////////////////////////////////////////
// Камера >>
////////////////////////////////////////////////////////////////////////////////
static HANDLE g_frame_ready_event = CreateEvent(NULL, FALSE, FALSE, NULL);
static ThreadWrap g_camera_thread;

static void CameraOnFrameReady(void*)
{
	Camera_Frame *frame = g_camera->GetFrame(true);
	if (frame)
	{

		if (g_calc_spots)
		{
			CalcSpots(frame->m_data);
//			if (g_calibrovka_param.m_successfull)	
//				CalcHeight();
		}
		g_frames_viewer.SetGrayscaleData(frame->m_data, g_w, g_h, g_bitcount);
		
		{
			MutexWrap last_frame_access(g_last_frame_mutex);
			swap(g_last_frame, frame);
			g_frames_counter++;
		}
		if (frame)
			frame->Delete();
	}
}

////////////////////////////////////////////////////////////////////////////////
// Камера <<
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
static void InitCamera()
{
	if (g_camera)
	{
		delete g_camera;
		g_camera = NULL;
	}

	char info_file_path[2048];
	MNP_cfg_file.ReadPar("CAMERA_FILE", info_file_path, MNP_tmp_path + "\\camera.1.0.txt");

	DataFromFile_CameraInterface* data_cam = new DataFromFile_CameraInterface;
	data_cam->m_info_file_path = info_file_path;
	g_camera = data_cam;
}

static void ClearSpotsAct(BmpButton*)
{
	if (!g_calc_spots)
		OnChangeFrameWidth();
}

////////////////////////////////////////////////////////////////////////////////
static void OnOpen(InterfaceForm*)
{
//	if (!g_calibrovka_param.m_successfull)
//		g_calibrovka_param.Load();

	LOGGER_AddMessage(KaKaO_log_id, LOG_DBG, _T("Открывается форма: ") + g_form.m_btn_name, true);

	InitCamera();
	if (g_camera == NULL)
	{
		LOGGER_AddMessage(KaKaO_log_id, LOG_ERROR, _T("Камера не создана"), true);
		return;
	}

	bool inited = false;
	inited = g_camera->Init(1);
	if (!inited)
	{
		LOGGER_AddMessage(KaKaO_log_id, LOG_ERROR, _T("Камера не инициализирована"), true);
	}
	else
	{
		g_w = g_camera->m_width;
		g_h = g_camera->m_height;
		g_bitcount = g_camera->GetBitCount();
		g_max_brightness = (g_bitcount == 32) ? 255 : ((1 << g_bitcount) - 1);
		OnChangeFrameWidth();

		ResetEvent(g_frame_ready_event);
		g_camera->SetFrameEvent(g_frame_ready_event);

		if (g_camera_thread.m_user_events_num == 0)
		{
			g_camera_thread.SetUserEventsNum(1);
			g_camera_thread.SetUserReaction(0, g_frame_ready_event, CameraOnFrameReady);
		}
		g_camera_thread.Run();

		g_camera->Start(false, 2);
	}

/*
	DlgEditText_func_before_open = BeforeOpenDlgEditText;
	DlgEditText_func_after_close = AfterCloseDlgEditText;
	AfterCloseDlgEditText(); // включаем процедуру перехвата клавиатуры
*/
	Interface_StartTimer(g_form.m_id, 10, 100);
}

static void OnClose(InterfaceForm*)
{
	if (g_camera)
	{
		g_camera_thread.Stop();
		g_camera->SetFrameEvent(NULL);
		g_camera->ShutDown();
		delete g_camera;
		g_camera = NULL;
	}

	Interface_KillTimer(g_form.m_id, 3);
	Interface_KillTimer(g_form.m_id, 10);
/*
	DlgEditText_func_before_open = NULL;
	DlgEditText_func_after_close = NULL;
	CancelKeyboardHook(); // выключаем процедуру перехвата клавиатуры
*/
}

static void OnTimer(InterfaceForm*, int num)
{
	switch (num)
	{
		case 3:
//			SetKeyboardHook();
			break;
		case 10:
//			OutputCoords();
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// TEST >>
////////////////////////////////////////////////////////////////////////////////
static bool g_test_is_running = false, g_test_for_cpu = false;
static byte* g_test_in_data = NULL;
static float* g_test_out_data = NULL;
static int g_test_frames_get = -1, g_frame_data_size = -1;

static void TestCalcFrameByte(byte *data, float * const cms_buff)
{
	byte* ptr;
	float *cms_ptr, cms, avg_cms = 0;
	int x, y, start_y, cms_counter = 0;
	double weighted_coordinate, total_weight, specific_weight, best_specific_weight;
	byte br, thr = g_max_brightness * g_threashold;
	for (x = 0, cms_ptr = cms_buff; x < g_w; x++, cms_ptr++)
	{
		cms = -1;

		start_y = -1;
		best_specific_weight = 0;

		ptr = data + x;
		for (y = 0; y < g_h; y++, ptr += g_w)
		{
			br = *ptr;
			if (br > thr)
			{
				if (start_y == -1) // начало пятна
				{
					start_y = y;
					weighted_coordinate = 0;
					total_weight = 0;
				}
				total_weight += br;
				weighted_coordinate += double(y) * br;
			}
			
			if ((br <= thr || y == g_h -1) && start_y != -1) // конец пятна
			{
				specific_weight = total_weight / (y - start_y + 1);
				if (specific_weight > best_specific_weight)
				{
					best_specific_weight = specific_weight;
					cms = weighted_coordinate / total_weight;
				}	
				start_y = -1;
			}
		}

		*cms_ptr = cms;
		/**/
		if (cms > -1)
		{
			avg_cms += cms;
			cms_counter++;
		}
		/**/
	}
	
	// вычитание среднего значения
	/**/
	if (cms_counter > 0)
	{
		avg_cms /= cms_counter;
		for (x = 0, cms_ptr = cms_buff; x < g_w; x++, cms_ptr++)
			if (*cms_ptr == -1)
				*cms_ptr = -1000;
			else
				*cms_ptr -= avg_cms;
	}
	/**/
}

static void CameraOnFrameReadyForTest(void*)
{
	Camera_Frame *frame = g_camera->GetFrame(true);
	
	if (frame)
	{
		memcpy(g_test_in_data + g_frame_data_size * g_test_frames_get, frame->m_data, g_frame_data_size);
		frame->Delete();

		g_test_frames_get++;
		if (g_test_frames_get == g_test_frames)
			g_camera_thread.SetUserReaction(0, g_frame_ready_event, NULL);
	}
}

UINT TestThread(LPVOID param)
{
	DataFromFile_CameraInterface* data_cam = (DataFromFile_CameraInterface*)g_camera;
	CString out_str;
	DWORD test_counter;

	// запоминаем прежний темп камеры и ставим максимальный
	int prev_delay = data_cam->m_frames_delay;
	data_cam->m_frames_delay = 0;

	// копим кадры >>
	g_test_frames_get = 0;
	g_camera->Start(false, 2);
	test_counter = GetTickCount();
	while (g_test_frames_get < g_test_frames)
	{
		Sleep(100);
	}
	test_counter = GetTickCount() - test_counter;
	out_str.Format(_T("Накопление кадров: %d кадров за %.1f секунд"), g_test_frames, 0.001f * test_counter);
	LOGGER_AddMessage(KaKaO_log_id, LOG_DBG, out_str, true);
	// копим кадры <<
	
	// стопорим камеру и восстанавливаем темп
	g_camera_thread.Stop();
	data_cam->m_frames_delay = prev_delay;
	g_camera->Stop();

	// измеряем время
	if (g_test_for_cpu)
	{
		// запускаем расчет на процессоре (параллелим на ядра)
		byte* in_ptr = g_test_in_data;
		float* out_ptr = g_test_out_data;
		test_counter = GetTickCount();
		for (int i = 0; i < g_test_frames; i++, in_ptr += g_frame_data_size, out_ptr += g_w)
			TestCalcFrameByte(in_ptr, out_ptr);
		test_counter = GetTickCount() - test_counter;
		out_str.Format(_T("Тест на CPU: %d кадров за %.1f секунд"), g_test_frames, 0.001f * test_counter);
		LOGGER_AddMessage(KaKaO_log_id, LOG_DBG, out_str, true);
	}
	else
	{
		// считаем на CUDA
	}

	// создаем битмап и сохраняем
	/**/
	FILE* file_3d = fopen(MNP_tmp_path + "\\res.3d", "wb");
	fwrite(&g_w, 4, 1, file_3d);
	fwrite(&g_test_frames, 4, 1, file_3d);
	float z;

	int brightness, bmp_width = g_w, line_filler = 0;
	if (g_w % 4 > 0)
	{
		line_filler = (4 - (g_w % 4));
		bmp_width += line_filler;
	}
	byte *heightmap_bytes = new byte[bmp_width * g_test_frames], *hb_ptr = heightmap_bytes;
	float* cms_ptr = g_test_out_data;
	for (int frame = 0; frame < g_test_frames; frame++, hb_ptr += line_filler)
		for (int x = 0; x < g_w; x++, cms_ptr++, hb_ptr++)
			{
				if (*cms_ptr > -1000)
				{
					brightness = max(1, min(255, 127 - 25 * *cms_ptr));
					*hb_ptr = brightness;
					z = -12.0*1e3 * *cms_ptr;
				}
				else
				{
					*hb_ptr = 0;
					z = 0;
				}
				fwrite(&z, sizeof(z), 1, file_3d);
			}
	fclose(file_3d);
	CBitmap bmp_to_save;
	bmp_to_save.CreateBitmap(bmp_width, g_test_frames, 1, 8, heightmap_bytes);
	BMP_SaveBitmap(&bmp_to_save, MNP_tmp_path + "\\res.bmp");
	delete[] heightmap_bytes;
	/**/
	
	// сносим тестовые данные
	delete[] g_test_in_data;
	g_test_in_data = NULL;
	delete[] g_test_out_data;
	g_test_out_data = NULL;
	
	// перезапускаем камеру
	g_camera_thread.SetUserReaction(0, g_frame_ready_event, CameraOnFrameReady);
	g_camera_thread.Run();
	g_camera->Start(false, 2);

	g_test_is_running = false;
	g_btn_test_cpu.Enable(true);
	g_btn_test_gpu.Enable(true);

	return 524623094;
}

static void TestAct(BmpButton*btn)
{
	if (g_camera == NULL)
		return;

	g_frame_data_size = g_w * g_h * g_camera->GetBytesPerPixel();
	g_test_in_data = new byte[g_frame_data_size * g_test_frames];
	if (g_test_in_data == NULL)
		return;
	g_test_out_data = new float[g_w * g_test_frames];
	if (g_test_out_data == NULL)
	{
		delete[] g_test_in_data;
		g_test_in_data = NULL;
		return;
	}

	g_camera->Stop();
	g_camera_thread.Stop();
	g_camera_thread.SetUserReaction(0, g_frame_ready_event, CameraOnFrameReadyForTest);
	g_camera_thread.Run();
		
	g_test_is_running = true;
	g_test_for_cpu = (btn == &g_btn_test_cpu);
	g_btn_test_cpu.Enable(false);
	g_btn_test_gpu.Enable(false);

	AfxBeginThread(TestThread, btn, THREAD_PRIORITY_NORMAL);
}

static void OnInit(InterfaceForm*)
{
	int i, j;
	CString text;

	g_frames_viewer.m_draw_function = DrawInFrameViewer;
	g_frames_viewer.m_dc->m_on_init_dc_function = InitObjectsForViewer;
	g_frames_viewer.Create(g_wnd, 0, 0, 100, 100, true, true, 0x909699);
	
	g_ve_threashold.Create(g_wnd, 0, 0, 100, 20, true, false);
	g_ve_threashold.Setup("Порог (отн.ед.)", "%.2f", "%.2f", &g_threashold, 0.05f, 1.0f, 0.05f);

	g_ve_base_pos.Create(g_wnd, 0, 0, 100, 20, true, false);
	g_ve_base_pos.Setup("База (отн.ед.)", "%.2f", "%.2f", &g_base_pos, 0.0f, 1.0f, 0.05f);
	
	g_gb_test_speed.Create(g_wnd);
	g_ve_test_frames.Create(g_wnd, 0, 0, 100, 20, true, false);
	g_ve_test_frames.Setup("Кол-во кадров", "%d", "%d", &g_test_frames, 1, 1e9, 1000);
	g_btn_test_cpu.Create("CPU", TestAct, g_wnd);

	/*
	g_frames_viewer.SetMessageHandler(WM_LBUTTONDOWN, (InterfaceObject_OwnFunction)NULL);
	g_frames_viewer.SetMessageHandler(WM_LBUTTONDOWN, (InterfaceObject_GlobalFunction)(&OnLeftButtonDown));
	g_frames_viewer.SetMessageHandler(WM_LBUTTONUP, (InterfaceObject_OwnFunction)NULL);
	g_frames_viewer.SetMessageHandler(WM_LBUTTONUP, (InterfaceObject_GlobalFunction)(&OnLeftButtonUp));
	g_frames_viewer.SetMessageHandler(WM_RBUTTONDOWN, (InterfaceObject_OwnFunction)NULL);
	g_frames_viewer.SetMessageHandler(WM_RBUTTONDOWN, (InterfaceObject_GlobalFunction)(&OnRightButtonDown));
	g_frames_viewer.SetMessageHandler(WM_RBUTTONUP, (InterfaceObject_OwnFunction)NULL);
	g_frames_viewer.SetMessageHandler(WM_RBUTTONUP, (InterfaceObject_GlobalFunction)(&OnRightButtonUp));
	g_frames_viewer.SetMessageHandler(WM_MBUTTONDOWN, (InterfaceObject_OwnFunction)NULL);
	g_frames_viewer.SetMessageHandler(WM_MBUTTONDOWN, (InterfaceObject_GlobalFunction)(&OnMiddleButtonDown));
	g_frames_viewer.SetMessageHandler(WM_MBUTTONUP, (InterfaceObject_OwnFunction)NULL);
	g_frames_viewer.SetMessageHandler(WM_MBUTTONUP, (InterfaceObject_GlobalFunction)(&OnMiddleButtonUp));
	g_frames_viewer.SetMessageHandler(WM_MOUSEMOVE, (InterfaceObject_OwnFunction)NULL);
	g_frames_viewer.SetMessageHandler(WM_MOUSEMOVE, (InterfaceObject_GlobalFunction)(&OnMouseMove));
	g_frames_viewer.SetMessageHandler(WM_MOUSEWHEEL, (InterfaceObject_OwnFunction)NULL);
	g_frames_viewer.SetMessageHandler(WM_MOUSEWHEEL, (InterfaceObject_GlobalFunction)(&OnMouseWheel));
	*/
	g_frames_viewer.SetShowZoomInfoParams(true, -5, -5);
	g_frames_viewer.m_min_scale = 0.5;
	g_frames_viewer.m_max_scale = 100;
	g_frames_viewer.SetIdleScale(1.0);
	g_frames_viewer.SetDefaultScale(1.0);

	g_cb_draw_base.Setup(&g_draw_base, "Рисовать базу");
	g_cb_draw_base.Create(NULL, -1, -1, g_wnd);

	g_cb_calc_spots.Setup(&g_calc_spots, "Рассчет");
	g_cb_calc_spots.Create(ClearSpotsAct, -1, -1, g_wnd);

/*
	BmpButton *btn = g_btn_move + 0;
	CS::m_colors[0] = btn->m_color_norm;	// bg
	CS::m_colors[1] = btn->m_color_norm;	// frame
	CS::m_colors[2] = btn->m_color_shadow;	// text
*/
//	g_cs_coords.Create(g_wnd, 0, 0, 100, 20, true, false);

	int gap = InterfaceForm::m_gap;
	g_form.CreateBaseLayout(LAYOUT_orientation_horizontal, 100, 100);

		g_form.StartEmptyLayout(LAYOUT_orientation_horizontal, -1, -1);
			g_form.SingleElementLayout(&g_frames_viewer, -1, -1);
			g_form.SingleElementLayout((CWnd*)NULL, gap, -1); // горизонтальный разделитель
			g_form.StartEmptyLayout(LAYOUT_orientation_vertical, 380, -1);
				Interface_LayoutNavButtonsVar1(g_form);
				
				g_form.SingleElementLayout(&g_cb_calc_spots, -1, InterfaceForm::m_static_h, InterfaceForm_TOP_INSETS);
				//...
				g_form.SingleElementLayout(&g_cb_draw_base, -1, InterfaceForm::m_static_h, InterfaceForm_TOP_INSETS);
				g_form.SingleElementLayout(&g_ve_base_pos, -1, InterfaceForm::m_ve_h, InterfaceForm_TOP_INSETS);
				g_form.SingleElementLayout(&g_ve_threashold, -1, InterfaceForm::m_ve_h, InterfaceForm_TOP_INSETS);
				//g_form.SingleElementLayout(&g_cb_marzhouser_joystick_mode, -1, InterfaceForm::m_static_h, InterfaceForm_TOP_INSETS);

				g_gb_test_speed.StartGroupBox(&g_form, -1, 0, InterfaceForm_TOP_INSETS);
					g_form.SingleElementLayout(&g_ve_test_frames, -1, InterfaceForm::m_ve_h, InterfaceForm_NULL_INSETS);
					g_form.SingleElementLayout(&g_btn_test_cpu, -1, InterfaceForm::m_button_h, InterfaceForm_TOP_INSETS);
				g_gb_test_speed.FinishGroupBox();

				g_form.SingleElementLayout((CWnd*)NULL, -1, -1); // filler

				g_form.SingleElementLayout(&KaKaO_log_list, -1, KaKaO_log_list_h, InterfaceForm_TOP_INSETS);

			g_form.FinishLayout(_T("controls"));
		g_form.FinishLayout(_T("single view layout"));

	g_form.FinishLayout(g_form.m_name);
}

static void OnShutdown(InterfaceForm*)
{
	MutexWrap last_frame_access(g_last_frame_mutex);
	if (g_last_frame)
		g_last_frame->Delete();
}

////////////////////////////////////////////////////////////////////////////////
void LaserSectionForm_Setup(int num, CWnd* wnd)
{
	g_form.m_name = _T("ЛАЗЕРНОЕ СЕЧЕНИЕ (CUDA)");
	g_form.m_btn_name = _T("Лазерное сечение (CUDA)");
	g_form.SetParentWnd(wnd);
	g_wnd = wnd;
	g_form.m_on_init_function = OnInit;
	g_form.m_on_shutdown_function = OnShutdown;
	g_form.m_on_open_function = OnOpen;
	g_form.m_on_close_function = OnClose;
	g_form.m_on_timer_function = OnTimer;
	Interface_AddForm(num, &g_form);
}

////////////////////////////////////////////////////////////////////////////////
// end