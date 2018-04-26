#include "stdafx.h"
#include "cfgfiles.h"
#include <stdio.h>
#include <string>

using namespace std;

// ======================================================================================= //
CCfgFiles::CCfgFiles(bool write_on_read):
	m_write_on_read(write_on_read)
{
	m_file_name = new TCHAR[1024];
	m_file_name[0] = '\0';

	m_tmpread = new char[1024];
}

CCfgFiles::~CCfgFiles()
{
	delete []m_file_name;
	delete []m_tmpread;
}
// ======================================================================================= //

void CCfgFiles::AssignTo(const TCHAR *fname)
{
	_tcscpy(m_file_name, fname);
}

bool CCfgFiles::WritePar(const TCHAR *parname, bool value)
{
	return WritePar_(parname, (void*)&value, CFG_TYPE_BOOL);
}

bool CCfgFiles::WritePar(const TCHAR *parname, int value)
{
	return WritePar_(parname, (void*)&value, CFG_TYPE_INT);
}

bool CCfgFiles::WritePar(const TCHAR *parname, DWORD value, bool hex)
{
	return WritePar_(parname, (void*)&value, hex?CFG_TYPE_HEX:CFG_TYPE_DWORD);
}

bool CCfgFiles::WritePar(const TCHAR *parname, float value)
{
	return WritePar_(parname, (void*)&value, CFG_TYPE_FLOAT);
}

bool CCfgFiles::WritePar(const TCHAR *parname, double value)
{
	return WritePar_(parname, (void*)&value, CFG_TYPE_DOUBLE);
}

bool CCfgFiles::WritePar(const TCHAR *parname, const TCHAR *value)
{
	return WritePar_(parname, (void*)value, CFG_TYPE_STRING);
}

static CString FloatWOZeros(float f, int precision)
{
	CString result, format;
	format.Format(_T("%%.%df"), precision);
	result.Format(format, f);
	int len = result.GetLength(), i = len - 1;
	if (result.Find('.') != -1)
	{
		while (result.GetAt(i) == '0') i--;
		if (i < len - 1)
		{
			if (result.GetAt(i) == '.') i--;
			return result.Left(i+1);
		}
	}
	return result;
}

CString CCfgFiles::GetStrValue(const void*value, BYTE type)
{
	CString result;
	switch (type)
	{
		case CFG_TYPE_BOOL:
			result.Format(_T("%s"), (*(bool*)value)?_T("true"):_T("false"));
			break;
		case CFG_TYPE_INT:
			result.Format(_T("%i"), *(int*)value);
			break;
		case CFG_TYPE_DWORD:
			result.Format(_T("%d"), *(DWORD*)value);
			break;
		case CFG_TYPE_HEX:
			result.Format(_T("0x%X"), *(DWORD*)value);
			break;
		case CFG_TYPE_FLOAT:
			result.Format(_T("%s"), FloatWOZeros(*(float*)value, 6));
			break;
		case CFG_TYPE_DOUBLE:
			result.Format(_T("%s"), FloatWOZeros(*(double*)value, 12));
			break;
		case CFG_TYPE_STRING:
			result.Format(_T("\"%s\""), (const char*)value);
			break;
		default:
			result = _T("WRONG VAULE TYPE");
	}
	return result;
}

void CCfgFiles::GetValueStr(const char* parname, const void* value, BYTE type, char* result)
{
	char value_str[1024];
	if (type == CFG_TYPE_STRING)
	{
#ifdef _UNICODE
		CW2AEX<1024> converter((LPCWSTR)value);
		sprintf(value_str, "\"%s\"", converter);
#else
		sprintf(value_str, "\"%s\"", (const char*)value);
#endif
	}
	else
	switch (type)
	{
		case CFG_TYPE_BOOL:
			sprintf(value_str, "%s", (*(bool*)value) ? "true" : "false");
			break;
		case CFG_TYPE_INT:
			sprintf(value_str, "%i", *(int*)value);
			break;
		case CFG_TYPE_DWORD:
			sprintf(value_str, "%d", *(DWORD*)value);
			break;
		case CFG_TYPE_HEX:
			sprintf(value_str, "0x%X", *(DWORD*)value);
			break;
		case CFG_TYPE_FLOAT:
			sprintf(value_str, "%f", *(float*)value);
			break;
		case CFG_TYPE_DOUBLE:
			sprintf(value_str, "%lf", *(double*)value);
			break;
//		case CFG_TYPE_STRING:
//			break;
		default:
			sprintf(value_str, "%s", "WRONG VAULE TYPE");
	}
	sprintf(result, "%s = %s\n", parname, value_str);
}

bool CCfgFiles::WritePar_(const TCHAR *parname, const void* value, BYTE type)
{
	FILE *cfg_file, *tmp_file;
	TCHAR tmp_file_name[1000];
	char pname[1000], wstr[2000], oldch = '\n';
	CString str;
	bool f_parwritten = false, f_skipping = false;

	_stprintf(tmp_file_name, _T("%s.$$$"), m_file_name);

	if (	(cfg_file = _tfopen(m_file_name, _T("rt"))) == NULL && 
			(cfg_file = _tfopen(m_file_name, _T("wt"))) == NULL
		)
		return false;		// ошибка открытия файла
	if ( (tmp_file = _tfopen(tmp_file_name, _T("wt"))) == NULL)
	{
		fclose(cfg_file);
		return false;		// ошибка открытия файла
	}

	while (!feof(cfg_file))
	{
		// чтение строки
		if ( fgets(m_tmpread, 1024, cfg_file) != NULL )
		{
			int str_len = strlen(m_tmpread);
			if (str_len < 5) // x=1\n\r
				continue;
			oldch = m_tmpread[str_len - 1];
			str = m_tmpread;

			if (!f_parwritten)
			{
				// теперь выделяем имя параметра
				int i = -1;
				do
				{
					i++; pname[i] = str[i];
				} while (/*!iswspace(str[i]) && */!(str[i]=='='));
				while (i > 0 && iswspace(pname[i - 1]))
					i--;
				pname[i] = 0;

				if (i != str.GetLength()) // значит это не конец строки
				{
					if (CString(pname) == CString(parname))
					{
						// наш параметр - пишем свое значение
						GetValueStr(pname, value, type, wstr);
						fputs(wstr, tmp_file);
						f_parwritten = true;
						f_skipping = true;
					}
				}
			}
			if (!f_skipping)
				fputs(m_tmpread, tmp_file);
			else
				f_skipping = false;			
		}
		else
			break;
	}
	if (!f_parwritten)
	{
		// проверяем - был ли в последней строке перевод
		if (oldch != '\n' && oldch != '\r')
			fputs("\n", tmp_file);
#ifdef _UNICODE
		int i, _len = _tcsclen(parname);
		for (i = 0; i < _len; i++)
			pname[i] = parname[i];
		pname[_len] = '\0';
		GetValueStr(pname, value, type, wstr);
#else
		GetValueStr(parname, value, type, wstr);
#endif
		//_fputts(parname, tmp_file);
		fputs(wstr, tmp_file);
	}

	fclose(cfg_file);
	fclose(tmp_file);
	if (_tremove(m_file_name) != 0)
	{
		_tremove(tmp_file_name);
		return false;
	}
	else
		_trename(tmp_file_name, m_file_name);
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
int CCfgFiles::ReadPar(const TCHAR *parname, bool& value, const bool defvalue)
{
	int res = ReadPar_(parname, &value, CFG_TYPE_BOOL);
	if (res==CFGF_ERR_NOTFOUND || res==CFGF_ERR_TYPE)
	{
		value = defvalue;
		if (m_write_on_read)
			if(!WritePar(parname, defvalue)) return CFGF_ERR_WRITEDEF;
	}
	return res;
}
int CCfgFiles::ReadPar(const TCHAR *parname, int& value, const int defvalue)
{
	int res = ReadPar_(parname, &value, CFG_TYPE_INT);
	if (res==CFGF_ERR_NOTFOUND || res==CFGF_ERR_TYPE)
	{
		value = defvalue;
		if (m_write_on_read)
			if (!WritePar(parname, defvalue)) return CFGF_ERR_WRITEDEF;
	}
	return res;
}
int CCfgFiles::ReadPar(const TCHAR *parname, DWORD& value, const DWORD defvalue, bool hex)
{
	int res = ReadPar_(parname, &value, hex?CFG_TYPE_HEX:CFG_TYPE_DWORD);
	if (res==CFGF_ERR_NOTFOUND || res==CFGF_ERR_TYPE)
	{
		value = defvalue;
		if (m_write_on_read)
			if (!WritePar(parname, defvalue, hex)) return CFGF_ERR_WRITEDEF;
	}
	return res;
}
int CCfgFiles::ReadPar(const TCHAR *parname, float& value, const float defvalue)
{
	int res = ReadPar_(parname, &value, CFG_TYPE_FLOAT);
	if (res==CFGF_ERR_NOTFOUND || res==CFGF_ERR_TYPE)
	{
		value = defvalue;
		if (m_write_on_read)
			if (!WritePar(parname, defvalue)) return CFGF_ERR_WRITEDEF;
	}
	return res;
}
int CCfgFiles::ReadPar(const TCHAR *parname, double& value, const double defvalue)
{
	int res = ReadPar_(parname, &value, CFG_TYPE_DOUBLE);
	if (res==CFGF_ERR_NOTFOUND || res==CFGF_ERR_TYPE)
	{
		value = defvalue;
		if (m_write_on_read)
			if (!WritePar(parname, defvalue)) return CFGF_ERR_WRITEDEF;
	}
	return res;
}
int CCfgFiles::ReadPar(const TCHAR *parname, TCHAR* value, const TCHAR* defvalue)
{
	int res = ReadPar_(parname,value, CFG_TYPE_STRING);
	if (res == CFGF_ERR_NOTFOUND || res == CFGF_ERR_TYPE)
	{
		_stprintf(value, _T("%s"), defvalue);
		if (m_write_on_read)
			if (!WritePar(parname, defvalue)) return CFGF_ERR_WRITEDEF;
	}
	return res;
}

int CCfgFiles::ReadPar_(const TCHAR *parname, void *value, BYTE type)
{
	FILE *cfg_file;
	TCHAR pname[1024];
	CString str;
	int str_len;
	int res = 1;

	if ( (cfg_file = _tfopen(m_file_name, _T("rt"))) == NULL) return CFGF_ERR_NOTFOUND;
	
	while (!feof(cfg_file))
	{
		if ( fgets(m_tmpread, 1024, cfg_file) != NULL )
		{
			// ошибка чтения строки
			str = m_tmpread;

			// теперь выделяем имя параметра
			int i = -1;
			str_len = min(1023, str.GetLength());
			if (str_len < 5) // x=1\n\r
				continue;
			do
			{
				i++;
				pname[i] = str[i];
			} while (str[i] != '=' && i < str_len);
			while (i > 0 && i < str_len && iswspace(pname[i - 1]))
				i--;
			pname[i] = '\0';
		
			if (i != str.GetLength())
			{
				// значит это не конец строки
				if (_tcscmp(pname, parname) == 0)
				{
					str.Delete(0, i);	
					// теперь ищем "="
					i = -1;	do{	i++; } while (str[i] != 0 && str[i] != '=');
					// нашли
					if (i != str.GetLength())
					{
						// значит это не конец строки
						// трем "="
						str.Delete(0, i + 1);	
						// теперь ищем начало значения
						i = -1;	do { i++; } while (str[i] != '\0' && iswspace(str[i]));
						if (i != str.GetLength()) // значит это не конец строки
						{
							str.Delete(0, i);	 // трем
							// теперь вычищаем с конца не значимые символы, в том числе и переводы 
							i = str.GetLength(); do { i--; } while (i != 0 && iswspace(str[i]));
							if (str.GetLength() - i > 1)
								str.Delete(i + 1, str.GetLength() - i);

							// на этом этапе в str уже лежит готовое значение

							// строка
							if (str[0] == '"' && str[str.GetLength() - 1] == '"')
							{
								if (type != CFG_TYPE_STRING)
									res = CFGF_ERR_TYPE;
								else
								{
									str.Delete(0, 1);
									str.Delete(str.GetLength() - 1, 1);
									res = (1 == _stscanf(str, _T("%[^\n]s"), (TCHAR*)value)) ? CFGF_OK : CFGF_ERR_NOTFOUND;
								}
							}
							else
							// булеан
							if (_tcscmp(str, _T("false")) == 0)
							{
								if (type != CFG_TYPE_BOOL)
									res = CFGF_ERR_TYPE;
								else
								{
									*(bool*)value = false;
									res = CFGF_OK;
								}
							}
							else
							if (_tcscmp(str, _T("true"))==0)
							{
								if (type != CFG_TYPE_BOOL)
									res = CFGF_ERR_TYPE;
								else
								{
									*(bool*)value = true;
									res = CFGF_OK;
								}
							}
							else
							// флоат
							if (str.Find('.', 0) != -1)
							{
								if (type != CFG_TYPE_DOUBLE && type != CFG_TYPE_FLOAT)
									res = CFGF_ERR_TYPE;
								else
								{
									if (type == CFG_TYPE_FLOAT)
										res = (1 == _stscanf(str, _T("%f"), (float*)value)) ? CFGF_OK : CFGF_ERR_NOTFOUND;
									else
										res = (1 == _stscanf(str, _T("%lf"), (double*)value)) ? CFGF_OK : CFGF_ERR_NOTFOUND;
								}
							}
							else
							{
								// число
								if (type == CFG_TYPE_FLOAT)
									res = (1 == _stscanf(str, _T("%f"), (float*)value)) ? CFGF_OK : CFGF_ERR_NOTFOUND;
								else
								if (type == CFG_TYPE_DOUBLE)
									res = (1 == _stscanf(str, _T("%lf"), (double*)value)) ? CFGF_OK : CFGF_ERR_NOTFOUND;
								else
								{
									if (str.Find(_T("0x")) == 0)
										res = (1 == _stscanf(str, _T("0x%x"), (int*)value)) ? CFGF_OK : CFGF_ERR_NOTFOUND;
									else
										res = (1 == _stscanf(str, _T("%d"), (int*)value)) ? CFGF_OK : CFGF_ERR_NOTFOUND;
								}
							}
							fclose(cfg_file);
							return res;
						}
					}
				}
			}
		}
	}
	fclose(cfg_file);
	return CFGF_ERR_NOTFOUND;
}

// ======================================================================================= //
