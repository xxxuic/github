// uic 27.04.2009

#pragma once

struct BMP_Bitmap
{
	BMP_Bitmap();
	~BMP_Bitmap();
	bool Load(DWORD id, bool report_errors = true);
	bool Load(CString path, bool report_errors = true);
	void Draw(CDC* dc, int x, int y);

	BMP_Bitmap* Clone();

	DWORD m_id;
	bool m_loaded;
	BITMAPINFO *m_bmi;
	byte* m_bytes;
	int m_w, m_h, m_bytes_line_filler;
};

bool BMP_SaveBitmap(CBitmap* bmp, CString filename);

////////////////////////////////////////////////////////////////////////////////
// end