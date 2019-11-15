/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <PTRMI/DataBuffer.h>

namespace PTRMI
{

class ClientInterfaceExtData
{
public:

protected:

public:


	virtual DataBuffer::Data *		getExtData				(DataBuffer::DataSize *size) = 0;
	virtual DataBuffer::DataSize	getExtDataSize			(void) = 0;

};

}
