/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*	Points fileIO win32 class												*/
/*--------------------------------------------------------------------------*/

#ifndef POINTOOLS_FILEIO_WIN32
#define POINTOOLS_FILEIO_WIN32

#include <sys/stat.h>
#include <sys/types.h>
#include <ptds/ptwin32_api.h>
#include <pt/ptmath.h>

#include <stdio.h>

#include <pt/UnicodeConversion.h>

#include <ptds/DataSourceManager.h>

#include <ptds/ptds.h>

namespace ptds
{

	enum DataSourceState
	{
		DataSourceStateRead,
		DataSourceStateWrite,
		DataSourceStateClosed
	};

	class FileType
	{
	public:

		const char *	extension;
		const char *	descriptor;

	public:
		FileType() : extension(0), descriptor(0) {};
		FileType(const char*ext, const char*desc) : extension(ext), descriptor(desc) {}

		FileType &operator = (const FileType &f) { extension = f.extension; descriptor = f.descriptor; return *this; }

		inline bool operator == (const FileType &b) const	{ return strcmp(extension, b.extension) == 0;	}
		inline bool operator < (const FileType &b) const	{ return strcmp(extension, b.extension) < 0;	}
		inline bool operator > (const FileType &b) const	{ return strcmp(extension, b.extension) > 0;	}
	};


};



#endif
