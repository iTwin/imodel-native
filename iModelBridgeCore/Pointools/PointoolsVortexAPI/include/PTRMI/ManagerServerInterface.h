#pragma once 

#include <PTRMI/ServerInterface.h>

namespace PTRMI
{

PTRMI_DECLARE_SERVER_INTERFACE(ManagerServerInterface, Obj)
{

public:

public:

	Status swapManagerInfo(Stream &stream)
	{
		ManagerInfo	p1;
		Status		result;
															// This method has a non Auto dispatcher so that a pointer to this interface can be hooked into the call
		stream >> p1;

		if(initializeInvoke(stream).isOK())
		{
			ManagerInfo		resultManagerInfo;
			Manager		*	manager;

			if(manager = reinterpret_cast<Manager *>(getServerObject()))
			{
															// Call method with the extra pointer to this ServerInterface
				result = manager->swapManagerInfo(this, &p1, &resultManagerInfo);
			}

			if(isInvokeValid(stream).isOK())
			{
				stream << resultManagerInfo;
			}

			receiveReturnAuto(stream, result);
		}

		return stream.getStatus();
	}


	ManagerServerInterface(void)
	{
		addServerMethod(Name(L"swapManagerInfo"), &ManagerServerInterface::swapManagerInfo);

		addServerMethod(Name(L"getCurrentManagerInfo"), &Super::receiveAuto<Status, PS<In<unsigned int>>, PS<Out<ManagerInfo *>>, &Manager::getCurrentManagerInfo>);

		addServerMethod(Name(L"newObject"), &Super::receiveAuto<Status, PS<In<const Name *>>, PS<In<const Name *>>, PS<In<const PTRMI::GUID *>>, PS<In<const PTRMI::GUID *>>, PS<Out<PTRMI::GUID *>>, &Manager::newObject>);

		addServerMethod(Name(L"discardObject"), &Super::receiveAuto<Status, PS<In<const PTRMI::GUID *>>, &Manager::discardObject>);

/*
		addServerMethod(PTRMI::Name(L"f_void_void"), &Super::receiveAutoVoid<&MyObjClass::f_void_void>);

		addServerMethod(PTRMI::Name(L"f_float_float"), &Super::receiveAuto<float, PS<In<float>>, &MyObjClass::f_float_float>);

		addServerMethod(PTRMI::Name(L"f_void_float"), &Super::receiveAutoVoid<PS<InOut<float>>, &MyObjClass::f_void_float>);

		addServerMethod(PTRMI::Name(L"f_float_float_ptr"), &Super::receiveAuto<float, PS<In<float *>>, &MyObjClass::f_float_float_ptr>);

		addServerMethod(PTRMI::Name(L"f_virt_float_float"), &Super::receiveAuto<float, PS<In<float>>, &MyObjClass::f_virt_float_float>);
*/
	}
};

PTRMI_INSTANCE_SERVER_INTERFACE(PTRMI::ManagerServerInterface<PTRMI::Manager>, PTRMI::Manager)
PTRMI_USE_SERVER_INTERFACE(PTRMI::ManagerServerInterface<PTRMI::Manager>, PTRMI::Manager)

} // End PTRMI namespace
