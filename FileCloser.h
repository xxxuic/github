// uic 2018.04.26

struct FileCloser
{
	FileCloser(FILE* file): m_file(file) {}
	~FileCloser() { if (m_file) fclose(m_file); }

	FILE* m_file;
};

////////////////////////////////////////////////////////////////////////////////
// end