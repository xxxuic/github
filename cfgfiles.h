#ifndef __CFG_H_
#define __CFG_H_

#define CFG_TYPE_BOOL 0
#define CFG_TYPE_INT 1
#define CFG_TYPE_DWORD 2
#define CFG_TYPE_HEX 3
#define CFG_TYPE_FLOAT 4
#define CFG_TYPE_DOUBLE 5
#define CFG_TYPE_STRING 6

#define CFGF_OK				0
#define CFGF_ERR_OPEN   	-1
#define CFGF_ERR_READ   	-2
#define CFGF_ERR_NOTFOUND	-3
#define CFGF_ERR_TYPE		-4
#define CFGF_ERR_WRITEDEF	-5

struct CCfgFiles
{
	CCfgFiles(bool write_on_read = true);
	~CCfgFiles();

	bool m_write_on_read;
	void AssignTo(const TCHAR *fname);				// ��������� ����

	int ReadPar(const TCHAR *parname, DWORD& value, const DWORD defvalue = 0, bool hex = false);	// ������� �������� ��������� �� �����, ���� ��� ��������� - false
	int ReadPar(const TCHAR *parname, int& value, const int defvalue = 0);	// ������� �������� ��������� �� �����, ���� ��� ��������� - false
	int ReadPar(const TCHAR *parname, bool& value, const bool defvalue = false);	// ������� �������� ��������� �� �����, ���� ��� ��������� - false
	int ReadPar(const TCHAR *parname, float& value, const float defvalue = 0);	// ������� �������� ��������� �� �����, ���� ��� ��������� - false
	int ReadPar(const TCHAR *parname, double& value, const double defvalue = 0);	// ������� �������� ��������� �� �����, ���� ��� ��������� - false
	int ReadPar(const TCHAR *parname, TCHAR* value, const TCHAR* defvalue = _T(""));	// ������� �������� ��������� �� �����, ���� ��� ��������� - false

	bool WritePar(const TCHAR *parname, DWORD value, bool hex = false);// �������� �������� ��������� � ����
	bool WritePar(const TCHAR *parname, int value);// �������� �������� ��������� � ����
	bool WritePar(const TCHAR *parname, bool value);// �������� �������� ��������� � ����
	bool WritePar(const TCHAR *parname, float value);// �������� �������� ��������� � ����
	bool WritePar(const TCHAR *parname, double value);// �������� �������� ��������� � ����
	bool WritePar(const TCHAR *parname, const TCHAR *value);// �������� �������� ��������� � ����

	virtual bool WritePar_(const TCHAR *parname, const void *value, BYTE type);// �������� �������� ��������� � ����
	virtual int ReadPar_(const TCHAR *parname, void* value, BYTE type);	// ������� �������� ��������� �� �����, ���� ��� ��������� - false

	static CString GetStrValue(const void*value, BYTE type);

	CString GetFilename() { return CString(m_file_name); }
protected:
	TCHAR *m_file_name;								// ��� �������� �����, � ������� ����� ���� ������
	char *m_tmpread;

    static void GetValueStr(const char* parname, const void* value, BYTE type, char* result);
};

#endif