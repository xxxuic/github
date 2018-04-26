// uic 27.04.2009

#include <stdafx.h>
#include "BMP.h"
#ifndef BMP_MUTE
//#include "./Interface/Dialogs/DlgMsgBox.h"
#endif
#include "./FileCloser.h"

BMP_Bitmap::BMP_Bitmap():
	m_bytes(NULL), m_loaded(false), m_id(-1)
{
	m_bmi = (BITMAPINFO*)new char[sizeof(BITMAPINFO) + sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD))*256];
	m_bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_bmi->bmiHeader.biPlanes = 1;
	m_bmi->bmiHeader.biBitCount = 32;
	m_bmi->bmiHeader.biCompression = BI_RGB;
	m_bmi->bmiHeader.biSizeImage = 0;
	m_bmi->bmiHeader.biXPelsPerMeter = 0; 
	m_bmi->bmiHeader.biYPelsPerMeter = 0; 
	m_bmi->bmiHeader.biClrUsed = 0; 
	m_bmi->bmiHeader.biClrImportant = 0;
	for (int i = 0 ;i < 256; i++)
	{
		m_bmi->bmiColors[i].rgbRed = i;
		m_bmi->bmiColors[i].rgbGreen = i;
		m_bmi->bmiColors[i].rgbBlue = i;
		m_bmi->bmiColors[i].rgbReserved = 0;
	}
}

BMP_Bitmap::~BMP_Bitmap()
{
	if (m_bmi)
		delete[] m_bmi;
	if (m_bytes)
		delete[] m_bytes;
}

BMP_Bitmap* BMP_Bitmap::Clone()
{
	BMP_Bitmap *res = new BMP_Bitmap;
	memcpy(res->m_bmi, m_bmi, sizeof(BITMAPINFO) + sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD))*256);
	res->m_bytes_line_filler = m_bytes_line_filler;
	res->m_w = m_w;
	res->m_h = m_h;
	res->m_loaded = m_loaded;
	int bytepp = ((m_bmi->bmiHeader.biBitCount - 1)/8) + 1;
	int sz = (m_w * bytepp + m_bytes_line_filler) * m_h;
	if (sz > 0 && m_bytes)
	{
		res->m_bytes = new byte[sz];
		memcpy(res->m_bytes, m_bytes, sz);
	}
	return res;
}

bool BMP_Bitmap::Load(DWORD id, bool report_errors)
{
	CBitmap bm;
	if (!bm.LoadBitmap(id))
		return false;
	m_id = id;
	BITMAP bitmap;
	bm.GetBitmap(&bitmap);

/*	m_bmi->bmiHeader.biBitCount = bitmap.bmBitsPixel;
	if (m_bmi->bmiHeader.biBitCount < 24)
	{
		//if (!bm.LoadMappedBitmap(id, 0, m_bmi->bmiColors, 256))
		//	return false;
	}
*/

	if (m_bytes)
		delete[] m_bytes;
	m_loaded = false;
	m_w = bitmap.bmWidth;
	m_h = bitmap.bmHeight;
	m_bmi->bmiHeader.biWidth = m_w;
	m_bmi->bmiHeader.biHeight = -m_h;
	int sz = bitmap.bmWidthBytes * m_h;
	m_bytes = new byte[sz];
	int actual = bm.GetBitmapBits(sz, m_bytes);
	m_loaded = true;

	return true;
}

bool BMP_Bitmap::Load(CString path, bool report_errors)
{
	FILE *f = _tfopen(path, _T("rb"));
	if (!f)
		return false;
	FileCloser filecloser(f);
//	filecloser.m_title = "Ошибка чтения картинки";

	fseek(f, 0, SEEK_END);
	int sz = ftell(f);
	fseek(f, 0, SEEK_SET);
	byte HEADER[54];
	int bytes_read = fread(HEADER, 1, 54, f);
	if (bytes_read != 54 || HEADER[0] != 'B' || HEADER[1] != 'M')
	{
//		filecloser.m_text = "Неверный формат заголовка файла";
		return false;
	}
	DWORD dw;
	memcpy(&dw, HEADER + 2, 4);
	if (dw != sz) // размер
	{
//		filecloser.m_text = "Неверный размер файла";
		return false;
	}

	DWORD width, height;
	memcpy(&width, HEADER + 18, 4);
	memcpy(&height, HEADER + 22, 4);
	WORD bpp;
	memcpy(&bpp, HEADER + 28, 2);

	int bytes_pp =  bpp / 8;
	int line_size = width * bytes_pp;
	int data_size = height * line_size;
	if (data_size > sz - 54)
	{
//		filecloser.m_text = "Неверный размер данных";
		return false;
	}

	m_loaded = false;
	if (m_bytes)
		delete[] m_bytes;

	memcpy(&dw, HEADER + 10, 4); // data_offset
	int data_offset = dw;
	bool color = (data_offset == 54);
	if (!color)
		fread(m_bmi->bmiColors, 256*4, 1, f);
	fseek(f, data_offset, SEEK_SET);
	m_bmi->bmiHeader.biBitCount = bpp;

	m_bytes_line_filler = (4 - (line_size % 4)) % 4;
	line_size += m_bytes_line_filler;
	m_bytes = new byte[height * line_size];
	for (int y = 0; y < height; y++)
	{
		bytes_read = fread(m_bytes + (height - 1 - y) * line_size, 1, line_size, f);
		if (bytes_read != line_size)
		{
//			filecloser.m_text = "Недостаточно данных";
			delete[] m_bytes;
			m_bytes = NULL;
			return false;
		}
	}

	m_w = width;
	m_h = height;
	m_bmi->bmiHeader.biWidth = m_w;
	m_bmi->bmiHeader.biHeight = -m_h;
	m_loaded = true;

	return true;
}

void BMP_Bitmap::Draw(CDC* dc, int x, int y)
{
	if (!m_loaded)
		return;
	SetDIBitsToDevice(*dc, x, y, m_w, m_h, 0, 0, 0, m_h, m_bytes, m_bmi, DIB_RGB_COLORS);
}

bool BMP_SaveBitmap(CBitmap* bmp, CString filename)
{
	FILE* f = _tfopen(filename, _T("wb"));
	if (!f)
	{
//		DlgMsgBox_DoModal(_T("Ошибка сохраниения файла BMP"), CString(_T("Ошибка открытия файла ")) + filename);
		return false;
	}
//	FileCloser file_closer(f);
//	file_closer.m_title = "Ошибка сохранения файла BMP";

	BITMAP bitmap;
	bmp->GetBitmap(&bitmap);

	int width = bitmap.bmWidth, height = bitmap.bmHeight;
	byte channels = bitmap.bmWidthBytes / bitmap.bmWidth;
	byte out_channels = (channels == 1) ? 1 : 3;
	int line_size = width * out_channels;
	int line_filler = (4 - (line_size % 4)) % 4; // добавка к длине линии, чтобы ширина получалась всегда кратная 4
	line_size += line_filler;
	int out_data_size = height * line_size;

	int sz = bitmap.bmWidthBytes * height, x, y;
	byte *data_bytes = new byte[sz], *pdata_bytes;
	bmp->GetBitmapBits(sz, data_bytes);
	byte *out_data = new byte[out_data_size], *out_data_ptr;
	memset(out_data, 0, out_data_size);
	if (channels == 1)
	{
		// переворачиваем картинку
		for (y = 0; y < height; y++)
		{
			pdata_bytes = data_bytes + y * bitmap.bmWidthBytes;
			out_data_ptr = out_data + (height - 1 - y) * line_size;
			memcpy(out_data_ptr, pdata_bytes, width);
		}
	}
	else
	{
		// переворачиваем картинку и из 4/3 байт делаем 3
		for (y = 0; y < height; y++)
		{
			pdata_bytes = data_bytes + y * bitmap.bmWidthBytes;
			out_data_ptr = out_data + (height - 1 - y) * line_size;
			if (channels == 3)
				memcpy(out_data_ptr, pdata_bytes, width * 3);
			else
				for (x = 0; x < width; x++, pdata_bytes += channels, out_data_ptr += 3)
					memcpy(out_data_ptr, pdata_bytes, 3);
		}
	}
	delete[] data_bytes;

	byte HEADER[54];
	byte FILLER[3] = {0, 0, 0};
	int total_filler = 0;
	DWORD size = 54 + out_data_size;
	if (channels == 1)
		size += 256 * 4;
	total_filler = (4 - (size % 4)) % 4;
	size += total_filler;

	DWORD dw;
	WORD w;

	memset(HEADER, 0, 54);
	HEADER[0] = 'B';
	HEADER[1] = 'M';
	dw = size; // полный размер файла
	memcpy(HEADER + 2, &dw, 4);
	dw = 54; // смещение от начала заголовка файла до начала данных (размер заголовка)
	if (channels == 1)
		dw += 256 * 4;
	memcpy(HEADER + 10, &dw, 4);
	dw = 40; // размер заголовка ядра
	memcpy(HEADER + 14, &dw, 4);
	dw = width;
	memcpy(HEADER + 18, &dw, 4);
	dw = height;
	memcpy(HEADER + 22, &dw, 4);
	w = 1; // плоскости
	memcpy(HEADER + 26, &w, 2);
	w = out_channels * 8; // глубина цвета
	memcpy(HEADER + 28, &w, 2);
	HEADER[38] = 0x12;
	HEADER[39] = 0x0B;
	HEADER[42] = 0x12;
	HEADER[43] = 0x0B;

	fwrite(HEADER, 54, 1, f);
	if (channels == 1) // пишем палитру
	{
		HEADER[3] = 0;
		for (x = 0; x < 256; x++)
		{
			for (y = 0; y < 3; y++)
				HEADER[y] = x;
			fwrite(HEADER, 4, 1, f);
		}
	}
	fwrite(out_data, out_data_size, 1, f);
	if (total_filler > 0) // хвостик к файлу
		fwrite(FILLER, total_filler, 1, f);

	fclose(f);
	delete[] out_data;
	return true;
}
////////////////////////////////////////////////////////////////////////////////
// end