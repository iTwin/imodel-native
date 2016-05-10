#include "PointoolsVortexAPIInternal.h"

#include <ptds/FilePath.h>

#ifndef NO_DATA_SOURCE_SERVER
#include <PTRMI/DataBuffer.h>
#endif


// #ifndef NO_DATA_SOURCE_SERVER
// class PTRMI::URL;
// #endif

namespace ptds
{

#ifndef NO_DATA_SOURCE_SERVER

void FilePath::read(PTRMI::DataBuffer &buffer)
{
    wchar_t path[PT_MAXPATH];
    PTRMI::Array<> strArray((PT_MAXPATH) * sizeof(wchar_t), reinterpret_cast<PTRMI::Array<>::Data *>(&(path[0])));
	buffer >> strArray;

    BeFileName tmp(path);
    m_path = tmp;
}

void FilePath::write(PTRMI::DataBuffer &buffer) const
{
	PTRMI::Array<const unsigned char> str((wcslen(m_path.c_str()) + 1) * sizeof(wchar_t), reinterpret_cast<const unsigned char *>(&(m_path.c_str()[0])));

	buffer << str;
}

#endif

} // End ptds namespace

