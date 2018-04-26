// uic 2017.10.20

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
//#include "./Interface/SubInterface.h"
#include "./Interface/Dialogs/DlgMsgBox.h"

#include <math.h>
#define PI 3.1415692654
#include <vector>
using namespace std;

#include "./HARD/Marzhouser/Marzhouser.h"
#include "./HARD/camera/camera.h"
Camera_Interface* g_camera = NULL;
#ifdef CAMERA_IMPERX_GRABBER
	#include "./HARD/camera/IMPERX_Grabber/IMPERX_Grabber.h"
#endif
#ifdef CAMERA_DATA_FROM_DISK
	#include "./HARD/camera/DataFromDisk/DataFromDisk.h"
#endif

static Camera_Frame *g_last_frame = NULL;
static CMutex g_last_frame_mutex;

static DWORD g_frames_counter = 0;
static int g_w = -1, g_h = -1, g_bitcount = -1, g_max_brightness = -1;

struct SpotInfo
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

vector<SpotInfo> g_spots;
CMutex g_spots_mutex;

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
			else
			if (start_y != -1) // конец пятна
			{
				specific_weight = total_weight / (y - start_y);
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

void CalcSpots(void *data)
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
static VE<float> g_ve_threashold, g_ve_move_step, g_ve_base_pos;
static CS g_cs_coords;

static bool g_draw_base = false, g_marzhouser_joystick_mode = false, g_calc_spots = true;
static CB g_cb_draw_base, g_cb_marzhouser_joystick_mode, g_cb_calc_spots;

static BmpButton g_btn_move[4], g_btn_calibration;

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

void SpotInfo::Draw(G2D_DC* dc, float dx, float dy, float scale)
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
			if (g_spots[i].m_coord2_cm > 0)
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
// Калибровка >>
////////////////////////////////////////////////////////////////////////////////
void CalWaitFunc(void*);
void CalOnFinishFunc(void*);
static ThreadWrap g_calibrovka_thread(NULL, false, 30, CalWaitFunc);

float GetAvgCoord()
{
	MutexWrap spots_access(g_spots_mutex);
	g_spots.resize(g_w);
	int i, counter = 0;
	double cm, avg_cm = 0;
	for (i = 0; i < g_w; i++)
	{
		cm = g_spots[i].m_coord2_cm;
		if (cm > 0)
		{
			avg_cm += cm;
			counter++;
		}
	}
	return (counter > g_w / 2) ? (avg_cm / counter) : -1;
}

struct CalParams
{
	enum Stage
	{
		NotExecuting = 0,
		Initialization,
		WaitFrame,
		WaitMove,
	};
	
	CalParams():
		m_successfull(false), m_stage(NotExecuting)
	{
	}

	void Init()
	{
		LOGGER_AddMessage(KaKaO_log_id, LOG_DBG, _T("Init"), true);

		g_calibrovka_thread.m_on_finish_function = CalOnFinishFunc;

		m_cal_tab_y_to_depth.clear();
		m_start_depth = Marzhouser_GetPos(m_s_depth_coord);
		m_successfull = false;
		m_move_direction = +1;
		m_calibration_step = 100;
		m_stop_counter = 0;
		m_stage = Initialization;
	}
	void StartWaitFrame()
	{
		//LOGGER_AddMessage(KaKaO_log_id, LOG_DBG, _T("WaitFrame"), true);
		MutexWrap last_frame_access(g_last_frame_mutex);
		m_frame_counter = g_frames_counter;
		m_stage = WaitFrame;
	}
	void Move()
	{
		//LOGGER_AddMessage(KaKaO_log_id, LOG_DBG, _T("Move"), true);
		if (m_move_direction == 0)
		{
			Marzhouser_MoveTo(m_s_depth_coord, m_start_depth - m_calibration_step);
			m_move_direction = -1;
		}
		else
			Marzhouser_Offset(m_s_depth_coord, m_move_direction * m_calibration_step);

		m_prev_pos = Marzhouser_GetPos(m_s_depth_coord);
		m_stop_counter = 0;
		m_stage = WaitMove;
	}
	void CheckWaitFrame()
	{
		bool success = false;
		{
			MutexWrap last_frame_access(g_last_frame_mutex);
			success = (g_frames_counter > m_frame_counter);
		}
		if (success)
		{
			float coord = GetAvgCoord();
			if (coord > 0)
			{
				double depth = Marzhouser_GetPos(m_s_depth_coord) - m_start_depth;
				m_cal_tab_y_to_depth[coord] = depth;
				
				LOGGER_AddMessage(KaKaO_log_id, LOG_DBG, TXT(_T("y = %.1f -> depth = %.2lf")) << coord << depth, true);
				Move();
			}
			else
			{
				if (m_move_direction == -1)
				{
					m_stage = NotExecuting;
					m_successfull = true;
					SetEvent(g_calibrovka_thread.m_stop_thread_event);
					LOGGER_AddMessage(KaKaO_log_id, LOG_DBG, _T("Finish"), true);
					Marzhouser_MoveTo(m_s_depth_coord, m_start_depth);
				}
				else
				{
					LOGGER_AddMessage(KaKaO_log_id, LOG_DBG, _T("Return to start pos"), true);
					m_move_direction = 0;
					Move();
				}
			}
		}
	}
	void CheckWaitMove()
	{
		float pos = Marzhouser_GetPos(m_s_depth_coord);
		if (fabs(m_prev_pos - pos) < 0.1)
		{
			m_stop_counter++;
			if (m_stop_counter > 3)
				StartWaitFrame();
		}
		else
			m_prev_pos = pos;
	}
	void MainFunc()
	{
		switch (m_stage)
		{
			case Initialization:
				g_calc_spots = true;
				g_cb_calc_spots.Enable(false);
				StartWaitFrame();
				break;
			case WaitFrame:
				CheckWaitFrame();
				break;
			case CalParams::WaitMove:
				CheckWaitMove();
				break;
		}
	}
	bool Save()
	{
		if (!m_successfull)
			return false;
		FILE *f = fopen(APP_root_path + "\\cal.txt", "wt");
		if (f == NULL)
		{
			LOGGER_AddMessage(KaKaO_log_id, LOG_ERROR, _T("Калибровочная таблица не может быть записана"), true);
			return false;
		}
		CString str;
		map <double, float>::iterator current = m_cal_tab_y_to_depth.begin();
		while (current != m_cal_tab_y_to_depth.end())
		{
			str.Format("%.1f, %.2lf\n", current->first, current->second);
			fputs(str.GetBuffer(), f);
			current++;
		}
		fclose(f);
		return true;
	}
	bool Load()
	{
		FILE *f = fopen(APP_root_path + "\\cal.txt", "rt");
		if (f == NULL)
		{
			LOGGER_AddMessage(KaKaO_log_id, LOG_ERROR, _T("Калибровочная таблица не найдена"), true);
			return false;
		}
		m_cal_tab_y_to_depth.clear();

		m_successfull = false;
		char str[1024];
		float y;
		double depth;
		while (!feof(f))
		{
			if (fgets(str, 1024, f) == NULL)
				break;
			if (sscanf(str, "%.1f, %.2lf", &y, &depth) == 2)
			{
				m_cal_tab_y_to_depth[y] = depth;
			}
		}
		m_successfull = m_cal_tab_y_to_depth.size() >= 10;
		return m_successfull;
	}

	bool m_successfull;
	map <double, float> m_cal_tab_y_to_depth;

	Stage m_stage;
	float m_start_depth, m_calibration_step, m_prev_pos;
	int m_frame_counter, m_move_direction, m_stop_counter;

	static char m_s_depth_coord;
} g_calibrovka_param;

char CalParams::m_s_depth_coord = 'x';

void CalWaitFunc(void*)
{
	g_calibrovka_param.MainFunc();
}

void SetCalBtnText()
{
	g_btn_calibration.m_text = (g_calibrovka_param.m_stage != CalParams::NotExecuting) ? _T("СТОП") : _T("Калибровка");
	g_btn_calibration.Invalidate();
}

void CalOnFinishFunc(void*)
{
	g_cb_calc_spots.Enable(true);
	g_calibrovka_param.m_stage = CalParams::NotExecuting;
	LOGGER_AddMessage(KaKaO_log_id, LOG_DBG, g_calibrovka_param.m_successfull ? _T("Калибровка завершена") : _T("Ошибка калибровки"), true);
	SetCalBtnText();
	
	if (g_calibrovka_param.m_successfull)
		g_calibrovka_param.Save();
}

void CalcHeight()
{
	MutexWrap spots_access(g_spots_mutex);
	if (g_calibrovka_param.m_cal_tab_y_to_depth.size() < 2)
		return;

	map<double, float>::iterator left_iter = g_calibrovka_param.m_cal_tab_y_to_depth.begin(), right_iter;
	int i;
	double cm, k;
	for (i = 0; i < g_w; i++)
	{
		cm = g_spots[i].m_coord2_cm;
		if (cm > 0)
		{
			while (left_iter->first < cm)
				left_iter++;
			while (left_iter->first > cm)
				left_iter--;
			right_iter = left_iter;
			right_iter++;

			k = (cm - left_iter->first)/(right_iter->first - left_iter->first);
			
			g_spots[i].m_depth = left_iter->second + k * (right_iter->second - left_iter->second);
		}
	}
}
////////////////////////////////////////////////////////////////////////////////
// Калибровка <<
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Камера >>
////////////////////////////////////////////////////////////////////////////////
HANDLE	g_frame_ready_event = CreateEvent(NULL, FALSE, FALSE, NULL);
static ThreadWrap g_camera_thread;

void CameraOnFrameReady(void*)
{
	Camera_Frame *frame = g_camera->GetFrame(true);
	if (frame)
	{

		if (g_calc_spots)
		{
			CalcSpots(frame->m_data);
			if (g_calibrovka_param.m_successfull)	
				CalcHeight();
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

#ifdef CAMERA_IMPERX_GRABBER
	g_camera = new IMPERX_Grabber;

	int w, h, bits, taps;
	MNP_cfg_file.ReadPar("CAMERA_W", w, 1608);
	MNP_cfg_file.ReadPar("CAMERA_H", h, 1208);
	MNP_cfg_file.ReadPar("CAMERA_BITS", bits, 12);
	MNP_cfg_file.ReadPar("CAMERA_TAP", taps, (int)VCECLB_TapConfig_2TapInterLR);
	((IMPERX_Grabber*)g_camera)->SetupParams(w, h, bits, (VCECLB_TapConfigPredefined)taps);

#endif

#ifdef CAMERA_DATA_FROM_DISK

	char file_name[2048];
	MNP_cfg_file.ReadPar("CAMERA_CUBE_FILE", file_name, ".//TMP//camera.cube");
	DataFromDisk_CameraInterface* data_cam = new DataFromDisk_CameraInterface;
	data_cam->m_cube_name = file_name;
	g_camera = data_cam;

#endif
}

static void ClearSpotsAct(BmpButton*)
{
	if (!g_calc_spots)
		OnChangeFrameWidth();
}

static void TurnJoystickAct(BmpButton*)
{
	if (!g_marzhouser_joystick_mode)
	{
		Marzhouser_SendCommand("!joy 0"); // выкл джойстик
		Marzhouser_TurnPower(false); // вкл приводы
	}
	else
	{
		Marzhouser_SendCommand("!joy 2"); // вкл джойстик
		Marzhouser_TurnPower(true); // выкл приводы
	}
}

static void MoveAct(BmpButton*btn)
{
	int num = btn - g_btn_move;
	char axe = num < 2 ? 'x' : 'y';
	int dir = ((num % 2) == 0) ? -1 : +1;
	Marzhouser_Offset(axe, dir * g_move_step);
}

////////////////////////////////////////////////////////////////////////////////
static void OnOpen(InterfaceForm*)
{
	if (!g_calibrovka_param.m_successfull)
		g_calibrovka_param.Load();

	LOGGER_AddMessage(KaKaO_log_id, LOG_DBG, _T("Открывается форма: ") + g_form.m_btn_name, true);

	bool marzhouser_inited = Marzhouser_Init(3);
	LOGGER_AddMessage(KaKaO_log_id, marzhouser_inited ? LOG_MAIN : LOG_ERROR, CString("Столик ") + (marzhouser_inited?"инициализирован":"не инициализирован"), true);
	if (marzhouser_inited)
	{
		Marzhouser_SetVelocity('x', 3.0f);
		Marzhouser_SetVelocity('y', 3.0f);
		TurnJoystickAct(NULL);
	//	Marzhouser_Offset('x', 1000);
	}


	InitCamera();
	if (g_camera == NULL)
	{
		LOGGER_AddMessage(KaKaO_log_id, LOG_ERROR, _T("Камера не создана"), true);
		return;
	}

	bool inited = false;
#ifdef CAMERA_IMPERX_GRABBER
	IMPERX_Grabber *ig = ((IMPERX_Grabber*)g_camera);
	ig->SetPort(1);
	//IMPERX_Grabber::StartMassInit();
	//IMPERX_Grabber::FinishMassInit();
#endif
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

	Marzhouser_Shutdown();

	Interface_KillTimer(g_form.m_id, 3);
	Interface_KillTimer(g_form.m_id, 10);
/*
	DlgEditText_func_before_open = NULL;
	DlgEditText_func_after_close = NULL;
	CancelKeyboardHook(); // выключаем процедуру перехвата клавиатуры
*/
}

static void OutputCoords()
{
	double x = Marzhouser_GetPos('x'), y = Marzhouser_GetPos('y');
	g_cs_coords.m_text.Format("X: %.1lf, Y: %.1lf", x, y);
	g_cs_coords.Invalidate();
}

static void OnTimer(InterfaceForm*, int num)
{
	switch (num)
	{
		case 3:
//			SetKeyboardHook();
			break;
		case 10:
			OutputCoords();
			break;
	}
}

static void CalibrationAct(BmpButton* btn)
{
	if (g_calibrovka_thread.IsRunnung())
	{
		g_calibrovka_thread.Stop();
	}
	else
	{
		if (DlgMsgBox_DoModal("ПОДТВЕРДИТЕ ДЕЙСТВИЕ", "Произвести калибровку?", MB_YESNO) != IDYES)
			return;

		g_calibrovka_param.Init();
		
		g_calibrovka_thread.Run();
	}
	SetCalBtnText();
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

	g_ve_move_step.Create(g_wnd, 0, 0, 100, 20, true, false);
	g_ve_move_step.Setup("Шаг (мкм)", "%.1f", "%.1f", &g_move_step, 0.1f, 5000.0f, 10.0f);
	float vars[5] = {1, 10, 100, 500, 1000};
	g_ve_move_step.SetVariants(5, vars);
	
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

	g_cb_marzhouser_joystick_mode.Setup(&g_marzhouser_joystick_mode, "Джойстик");
	g_cb_marzhouser_joystick_mode.Create(TurnJoystickAct, -1, -1, g_wnd);

	g_cb_calc_spots.Setup(&g_calc_spots, "Рассчет");
	g_cb_calc_spots.Create(ClearSpotsAct, -1, -1, g_wnd);

	g_btn_calibration.Create("Калибровка", CalibrationAct, g_wnd);
	SetCalBtnText();

	for (i = 0; i < 4; i++)
		g_btn_move[i].Create("", MoveAct, g_wnd);
	g_btn_move[0].m_text = "<";
	g_btn_move[1].m_text = ">";
	g_btn_move[2].m_text = "/\\";
	g_btn_move[3].m_text = "\\/";

	BmpButton *btn = g_btn_move + 0;

	CS::m_colors[0] = btn->m_color_norm;	// bg
	CS::m_colors[1] = btn->m_color_norm;	// frame
	CS::m_colors[2] = btn->m_color_shadow;	// text
	g_cs_coords.Create(g_wnd, 0, 0, 100, 20, true, false);

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

				g_form.StartEmptyLayout(LAYOUT_orientation_horizontal, -1, 3*InterfaceForm::m_button_h, InterfaceForm_TOP_INSETS);
					g_form.StartEmptyLayout(LAYOUT_orientation_vertical, 3*InterfaceForm::m_button_h, -1);
						g_form.StartEmptyLayout(LAYOUT_orientation_horizontal);
							g_form.SingleElementLayout((CWnd*)NULL);
							g_form.SingleElementLayout(g_btn_move + 2);
							g_form.SingleElementLayout((CWnd*)NULL);
						g_form.FinishLayout(_T("[][up][]"));
						g_form.StartEmptyLayout(LAYOUT_orientation_horizontal);
							g_form.SingleElementLayout(g_btn_move + 0);
							g_form.SingleElementLayout((CWnd*)NULL);
							g_form.SingleElementLayout(g_btn_move + 1);
						g_form.FinishLayout(_T("[<][][>]"));
						g_form.StartEmptyLayout(LAYOUT_orientation_horizontal);
							g_form.SingleElementLayout((CWnd*)NULL);
							g_form.SingleElementLayout(g_btn_move + 3);
							g_form.SingleElementLayout((CWnd*)NULL);
						g_form.FinishLayout(_T("[][dn][]"));
					g_form.FinishLayout(_T("move buttons"));
					g_form.StartEmptyLayout(LAYOUT_orientation_vertical);
						g_form.SingleElementLayout(&g_cb_marzhouser_joystick_mode);
						g_form.SingleElementLayout(&g_cs_coords);
						g_form.SingleElementLayout(&g_ve_move_step);
					g_form.FinishLayout(_T("move params"));
				g_form.FinishLayout(_T("move controls"));

				g_form.SingleElementLayout(&g_btn_calibration, -1, InterfaceForm::m_button_h, InterfaceForm_TOP_INSETS);

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
void ProfilometryForm_Setup(int num, CWnd* wnd)
{
	g_form.m_name = _T("ИЗМЕРЕНИЕ ПРОФИЛЯ");
	g_form.m_btn_name = _T("Измерение профиля");
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