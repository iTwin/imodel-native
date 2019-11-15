/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
