#include "PointoolsVortexAPIInternal.h"

#include <ptds/FilePath.h>

#ifndef NO_DATA_SOURCE_SERVER
#include <PTRMI/DataBuffer.h>
#endif


#ifndef NO_DATA_SOURCE_SERVER
class PTRMI::URL;
#endif

namespace ptds
{

bool FilePath::findFile(wchar_t *dst, bool parent_dir, bool project_dir)
{
	fullpath(dst);

	if (_access(pt::Unicode2Ascii::convert(dst).c_str(),0) == 0) return true;

	wcscpy(dst, m_sPath);
	::PathStripPathW(dst);

	/*try parents directory*/ 
	if (m_parent && parent_dir)
	{
		wchar_t d[PT_MAXPATH];
		m_parent->fulldirectory(d);

		swprintf(dst, L"%s\\%s", d, filename());

		if (_access(pt::Unicode2Ascii::convert(dst).c_str(),0) == 0) return true;
	}
	if (project_dir && wcslen(projectDirectory()) > 0)
	{
		const wchar_t *dir [] = { projectDirectory(), L"/0" };
		return ::PathFindOnPathW(dst, dir) ? true : false;
	}
	return false;
}



#ifndef NO_DATA_SOURCE_SERVER

void FilePath::read(PTRMI::DataBuffer &buffer)
{
	PTRMI::Array<> strArray((PT_MAXPATH) * sizeof(wchar_t), reinterpret_cast<PTRMI::Array<>::Data *>(&(m_sPath[0])));

	buffer >> strArray;
}

void FilePath::write(PTRMI::DataBuffer &buffer) const
{
															// Write string plus terminator
	PTRMI::Array<const unsigned char> str((wcslen(m_sPath) + 1) * sizeof(wchar_t), reinterpret_cast<const unsigned char *>(&(m_sPath[0])));

	buffer << str;
}

#endif


} // End ptds namespace

