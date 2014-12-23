
#pragma once

#include <PTRMI/ClientInterface.h>
#include <PTRMI/ServerInterface.h>

#define NULL_OBJECT_CLASS_NAME			L"NullObject"

#define NULL_OBJECT_NULL_METHOD_NAME	L"nullMethod"

namespace PTRMI
{


class NullObject
{

public:

public:

	void nullMethod(void)
	{

	}
};


PTRMI_DECLARE_CLIENT_INTERFACE(NullClientInterface)
{

};


PTRMI_DECLARE_SERVER_INTERFACE(NullServerInterface, Obj)
{

public:

	NullServerInterface(void)
	{
		addServerMethod(Name(NULL_OBJECT_NULL_METHOD_NAME), &Super::receiveAutoVoid<&NullObject::nullMethod>);
	}
};


PTRMI_USE_CLIENT_INTERFACE(PTRMI::NullClientInterface, PTRMI::NullObject);

PTRMI_INSTANCE_SERVER_INTERFACE(PTRMI::NullServerInterface<PTRMI::NullObject>, PTRMI::NullObject)
PTRMI_USE_SERVER_INTERFACE(PTRMI::NullServerInterface<PTRMI::NullObject>, PTRMI::NullObject)


} // End PTRMI namespace