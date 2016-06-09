//--------------------------------------------------------------------
// ascii_importer.h: interface for the ascii_importer class.
//
// Database importer
//
// (c) Copyright 2003 Pointools Ltd
// All Rights Reserved
//--------------------------------------------------------------------

#ifndef POINTOOLS_DB_BUILDER_ASCII_IMPORTER_070503
#define POINTOOLS_DB_BUILDER_ASCII_IMPORTER_070503

#include <wchar.h>
#include <fstream>
#include <vector>

#define LINE_BUFFER_SIZE 512
#define HALF_LINE_BUFFER_SIZE 256

class ascii_importer
{
public:	
	ascii_importer(const char *filename=0)
	{
		m_file = 0;
		m_bytes_read = 0;
		m_fileline = 0;

		if (filename)
		{
			set_filename(filename);
			open();
		}
	};
	ascii_importer(const wchar_t *filename)
	{
		m_file = 0;
		m_bytes_read = 0;
		m_fileline = 0;

		if (filename)
		{
			set_filename(filename);
			open();
		}
	};
	bool open()
	{
		close();
		m_bytes_read = 0;
		m_pos = 0;
		m_file = new std::ifstream;
		m_file->open(m_filename, std::ios::in);
		return (m_file->is_open()) ? true : false;
	};
	bool is_open() const { return m_file && m_file->is_open(); }
	void close()
	{
		if (!m_file || !m_file->is_open()) return;
		m_file->close();

		delete m_file;
		m_file = 0;
	}
	/*-------------------------------------------------------------*/ 
	class Reader
	{
	public:
		Reader(int num_values) : _count(0), _size(num_values) {};
		inline bool add(const char*t)
		{ 
			if (_count < _size)
			{ 
				if (t) 
				{
					readValue(t);
					_count++;
					return true;
				}
			}
			return false;			
		};
		inline void reset() { _count = 0; }
		virtual void readValue(const char*)=0;

		int size() const { return _size; }
		int count() const { return _count; }

	protected:
		int _size;
		int _count;
	};
	template <class T, int C> class NumReader : public Reader
	{
	public:
		NumReader() : Reader(C) {};
		void readValue(const char*t) { values[_count] = (T)atof(t); }
		T values[C];
	};

	template <int C> class StringReader : public Reader
	{
	public:
		StringReader() : Reader(C) {};
		void readValue(const char*t) { strings[_count] = t; }
		std::string strings[C];
	};
	/*-------------------------------------------------------------*/ 
	void set_filename(const char* filename)
	{
		utf8towc(filename, (unsigned int)strlen(filename), m_filename, sizeof(m_filename));
	}
	void set_filename(const wchar_t* filename)
	{
		wcscpy(m_filename, filename);
	}
	void clear_read_buffer()
	{
		memset(m_buffer,0,  LINE_BUFFER_SIZE);
	}
	void clear_val_buffer()
	{
		memset(m_valbuffer,0,  LINE_BUFFER_SIZE);
	}
	int linesread() const { return m_fileline; }

	bool readline(char term='\n')
	{
		if (m_file->eof()) return false;
		clear_read_buffer();
		m_file->getline(m_buffer, LINE_BUFFER_SIZE, term);
		if (m_buffer[0] == 0 && term != '\n')
			m_file->getline(m_buffer, LINE_BUFFER_SIZE, '\n');
		m_pos = 0;
		m_bytes_read += strlen(m_buffer) + 2;
		m_fileline++;
		return true;
	}
	bool read_tokens(std::vector<Reader*> &readers)
	{
		if (!readline()) return false;
		
		readers[0]->reset();
		readers[0]->add(read_token(true));
		
		for (unsigned int i=0; i<readers.size(); i++)
		{
			if (i) readers[i]->reset();
			while (readers[i]->add(read_token(false)));
		}
		return true;
	}
	inline bool isSeperator(const char &c) const
	{
		return (c >= 'a' && c <= 'z')	
		|| (c >= '0' && c <= '9') || c == '.'
		|| (c >= 'A' && c <= 'Z')		
		|| c == '_' || c == '*'	|| c == '#' || c == '-' || c == '[' || c==']' || c=='+' || c=='=' || c=='\''
		|| c == '<' || c == '>' || c == ':' || c == '@' || c == '~' || c == '\\' || c == '/'
		? false : true;
	}
	inline bool isNumeric(const char &c) const
	{
		return ((c >= '0' && c <='0') || c == '.' || c=='e'|| c=='E');
	}
	template <class T>
	bool read_numeric_token(T &val, bool fromstart=false)
	{
		const char*token = read_token(fromstart);
		if (token)
		{
			val = (T)atof(token);
			return true;
		}
		return false;
	}
	const char *read_token(bool fromstart=false, char term='\n')
	{
		bool valid = false;
		bool start = false;
		if (fromstart) m_pos = 0;
		int pos = 0;
	
		size_t len = strlen(m_buffer);

		clear_val_buffer();
	
		do
		{
			valid = !isSeperator(m_buffer[m_pos]);

			if (m_buffer[m_pos] == term) break;

			if (!start && valid) start = true;

			if (valid)
			{
				m_valbuffer[pos++] = m_buffer[m_pos];
			}
			m_pos++;
		}
		while(m_pos < len && (valid || !start));

		m_valbuffer[pos] = '\0';

		if (pos>0)
		{
			memcpy(m_tokenbuffer, m_valbuffer, pos);
			m_tokenbuffer[pos] = '\0';
			return m_tokenbuffer;
		}
		else
		{
			return 0;
		}
	}
	const char*line(bool from_cursor=false) const { return from_cursor ? &m_buffer[m_pos] : m_buffer; }

	const char *read_quoted_token(bool fromstart=false, char term='\n', bool quoted_only = false)
	{
		bool valid = false;
		bool start = false;
		bool start_quote = false;

		if (fromstart) m_pos = 0;
		int pos = 0;
	
		size_t len = strlen(m_buffer);

		clear_val_buffer();
		
		do
		{
			if (m_buffer[m_pos] == '\"') start_quote = true;
			else valid = start_quote || (!quoted_only && !isSeperator(m_buffer[m_pos]));

			if (start_quote && m_buffer[m_pos] == '\"') valid = false;

			if (m_buffer[m_pos] == term) break;

			if (!start && valid) start = true;

			if (valid)
			{
				m_valbuffer[pos++] = m_buffer[m_pos];
			}
			m_pos++;
		}
		while(m_pos < len && (valid || !start));

		m_valbuffer[pos] = '\0';

		if (pos>0)
		{
			memcpy(m_tokenbuffer, m_valbuffer, pos);
			m_tokenbuffer[pos] = '\0';
			return m_tokenbuffer;
		}
		else
		{
			return 0;
		}
	}
	template <typename T> bool read_real(T &dest, bool fromstart=false, char term='\n')
	{
		bool valid = false;
		bool start = false;
		if (fromstart) m_pos = 0;
		int pos = 0;
	
		size_t len = strlen(m_buffer);

		clear_val_buffer();
	
		do
		{
			valid = (m_buffer[m_pos]>44) && (m_buffer[m_pos]<58) ? true : false;

			if (m_buffer[m_pos] == term || m_buffer[m_pos] == 0x0)
				break;

			if (!start && valid)
				start = true;

			if (valid)
			{
				m_valbuffer[pos++] = m_buffer[m_pos];
			}

			m_pos++;
		}

		while(m_pos < len && (valid || !start) && pos < LINE_BUFFER_SIZE);

		m_valbuffer[pos] = '\0';

		if (pos>0)
		{
			dest = (T)atof(m_valbuffer);
			return true;
		}
		else
		{
			dest = (T)0;
			return false;
		}
	}
	int row_size()
	{
		float v;
		if (!read_real(v, true)) return 1;
		int c = 1;
		while (read_real(v,false)) c++;
		return c;
	}
	const char* at_read_position() const { return &m_buffer[m_pos]; }

protected:
	/*filestream*/ 
	std::ifstream *m_file;

	/*buffer*/ 
	char m_buffer[LINE_BUFFER_SIZE];
	char m_valbuffer[LINE_BUFFER_SIZE];
	char m_tokenbuffer[LINE_BUFFER_SIZE];

	/*file*/ 
	wchar_t m_filename[512];
	size_t	 m_pos;
	int	 m_fileline;
	int64_t m_bytes_read;
};

#endif