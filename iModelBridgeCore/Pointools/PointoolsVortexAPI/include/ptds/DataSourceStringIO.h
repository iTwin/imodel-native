#pragma once

#include <pt/ptstring.h>
#include <ptds/dataSource.h>

namespace ptds
{

class DataSourceStringIO
{
public:
	template<class DS>
	static DataSource::Size write( DS *ds, const pt::String &str )
	{
		int strLen = str.length();
		wchar_t term = 0;
		DataSource::Size s = ds->writeBytes(strLen);
		if (strLen)
		{
			s += ds->writeBytes(str.c_wstr(), sizeof(wchar_t) * strLen);
			s += ds->writeBytes(term);
		}
		return s;
	}

	template<class DS>
	static DataSource::Size read( DS *ds, pt::String &str )
	{
		int strLen = 0;

		DataSource::Size s = ds->readBytes(strLen);

		if (strLen && strLen > 0 && strLen < 16777216)	// sanity check
		{
			try
			{
				wchar_t *buffer = new wchar_t[strLen+4];
				memset(buffer, 0, sizeof(wchar_t)*strLen+1);

				s += ds->readBytes(buffer, sizeof(wchar_t) * (strLen + 1));
				str = buffer;

				delete [] buffer;
			}
			catch (std::bad_alloc)
			{
				return s;	// ERROR HANDLING NEEDED
			}
		}
		return s;
	}
};
}