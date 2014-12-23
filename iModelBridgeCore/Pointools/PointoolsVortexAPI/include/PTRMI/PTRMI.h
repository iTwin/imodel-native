#pragma once

//#include <PTRMI/Name.h>
#include <PTRMI/RemotePtr.h>
#include <PTRMI/Status.h>
#include <ptds/ptds.h>

namespace PTRMI
{
	class							ObjectManager;

	typedef wchar_t *				MethodName;

	typedef ptds::DataPointer		DataPtr;
	typedef ptds::DataSize			DataSize;
}