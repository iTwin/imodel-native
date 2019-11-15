/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PointoolsVortexAPIInternal.h"

#include <PTRMI/ServerInterface.h>


void PTRMI::ServerInterfaceBase::setClientManagerGUID(const PTRMI::GUID &guid)
{
	hostName = guid;
}

const PTRMI::GUID PTRMI::ServerInterfaceBase::getClientManagerGUID(void) const
{
	return hostName.getGUID();
}

