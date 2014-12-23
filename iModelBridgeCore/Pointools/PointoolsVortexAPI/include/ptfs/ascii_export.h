//--------------------------------------------------------------------
// ascii_exporter.h: interface for the ascii_exporter class.
//
// Database importer
//
// (c) Copyright 2003 Pointools Ltd
// All Rights Reserved
//--------------------------------------------------------------------

#ifndef POINTOOLS_DB_BUILDER_ASCII_EXPORTER_070503
#define POINTOOLS_DB_BUILDER_ASCII_EXPORTER_070503

#include <fstream>

class ascii_exporter
{
public:
	ascii_exporter()
	{
		m_file = 0;
		m_first = true;
	};	
	bool open()
	{
		close();
		m_file = new std::ofstream;

		m_file->open(m_filename, std::ios::out);
		m_file->precision(8);
		m_file->flags(std::ios::fixed | std::ios::showpoint);

		return m_file->is_open();
	};
	bool is_open() const { return m_file && m_file->is_open(); }
	void close()
	{
		if (m_file)
		{
			m_file->close();
			delete m_file;
			m_file = 0;
		}

	}
	void set_filename(const char* filename)
	{
		strcpy(m_filename, filename);
	}
	template <typename T> void write_row(const T *numbers, int count, char delimit =' ')
	{
		for (int i=0; i<count; i++)
			(*m_file) << numbers[i] << delimit;
		(*m_file) << std::endl;
		m_fileline++;
	}
	template <typename T> void write_value(const T &value, char delimit =' ')
	{
		if (!m_first) (*m_file) << delimit;
		(*m_file) << value;
		m_first = false;
		m_fileline++;
	}

	void newRow() { m_first = true; (*m_file) << std::endl; }

	int getLinesWritten() const
	{
		return m_fileline;
	}
private:
	/*filestream*/ 
	std::ofstream *m_file;

	/*file*/ 
	char m_filename[1024];
	int m_fileline;
	bool m_first;
};
#endif