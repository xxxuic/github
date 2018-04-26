// uic 18.04.2007

#pragma once

struct TXT
{
	TXT(const char* initial):
		m_data(initial)
	{}

	operator CString()
	{
		return m_data;
	}

	const TCHAR* GetChars()
	{
		return m_data.GetBuffer(1);
	}

	template <typename T> TXT& operator << (const T& param)
	{
		CString format = m_data;
		int ind = format.Find('%');
		if (ind == -1)
			return *this; // нет формата
		format.SetAt(ind, '#');
		format.Replace(_T("%"), _T("%%"));
		format.SetAt(ind, '%');
		m_data.Format(format, param);
		return *this;
	}

private:
	CString m_data;
};

template <typename T> CString TXT_FloatWOZeros(T f, int precision)
{
	CString result, format;
	format.Format(_T("%%.%df"), precision);
	result.Format(format, f);
	int len = result.GetLength(), i = len - 1;
	if (result.Find('.') != -1)
	{
		while (result.GetAt(i) == '0')
			i--;
		if (i < len - 1)
		{
			if (result.GetAt(i) == '.')
				i--;
			result = result.Left(i + 1);
		}
	}
	if (result == "-0")
		result = "0";
	return result;
}

////////////////////////////////////////////////////////////////////////////////
// end