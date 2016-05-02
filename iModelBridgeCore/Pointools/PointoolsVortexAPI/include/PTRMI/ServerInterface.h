#pragma once

// ************************************************************************
// * Includes
// ************************************************************************

#include "PTRMI.h"
#include "Name.h"
#include <PTRMI/Stream.h>
#include <PTRMI/InterfaceBase.h>

#include <map>



// ************************************************************************
// * Macros
// ************************************************************************

#define PTRMI_DECLARE_SERVER_INTERFACE(Inter, Obj) \
template<typename Obj> class Inter : public PTRMI::ServerInterface<Inter<Obj>, Obj>

#define PTRMI_INSTANCE_SERVER_INTERFACE(Inter, Obj) \
PTRMI::ServerInterface<Inter, Obj>::MethodMap PTRMI::ServerInterface<Inter, Obj>::methodMap;	// Declare static member

#define PTRMI_USE_SERVER_INTERFACE(Inter, Obj) \
template<> class PTRMI::ObjServerInfo<Obj> {public: typedef Inter I; typedef std::vector<I *> Set;};

#define PTRMI_INHERIT_SERVER_INTERFACE(Inter, Obj) \
template<> class PTRMI::ObjServerInfo<Obj> {public: typedef Inter I; typedef std::vector<I *> Set;}; \


#define SERVERINTERFACE_T	template<typename I, typename Obj>




namespace PTRMI
{

class ServerInterfaceBase : public InterfaceBase
{

public:

	virtual Status				invokeServerMethod		(const Name &name, Stream &stream) = 0;

	virtual void				setObject				(RemotableObject *obj) = 0;
	virtual RemotableObject	*	getObject				(void) = 0;

	void						setClientManagerGUID	(const PTRMI::GUID &guid);
	const PTRMI::GUID			getClientManagerGUID	(void) const;

};


template<typename I, typename Obj> class ServerInterface : public ServerInterfaceBase
{

public:
															// Method type within given Interface class
	typedef Status (I::*Method)(Stream &stream);
															// Map of method names to methods
	typedef	std::map<Name, Method>		MethodMap;
															// Export I as InterfaceType
	typedef I							InterfaceType;
															// Note: Super declared relative to derived class (not this class)
	typedef ServerInterface<I, Obj>		Super;

protected:

	static MethodMap		methodMap;

	Obj					*	object;


protected:

	void					addServerMethod				(const Name &name, Method method);
	Method					getServerMethod				(const Name &name);

	Status					isInvokeValid				(Stream &stream);

public:

							ServerInterface				(void);
	virtual				   ~ServerInterface				(void);

	void					setObject					(RemotableObject *obj);
	RemotableObject		*	getObject					(void);

	void					setServerObject				(Obj *obj);
	Obj					*	getServerObject				(void);

	Status					initializeInvoke			(Stream &stream);

	Status					invokeServerMethod			(const Name &name, Stream &stream);

	unsigned int			getNumberOfServerMethods	(void);

	template<typename R>
	void					receiveReturnAuto			(Stream &stream, R result);

															// Auto Receivers for methods that don't return a value
	template<void (Obj::*func)(void)>
	Status					receiveAutoVoid				(Stream &stream);

	template<typename T1, void (Obj::*func)(typename T1::T)>
	Status					receiveAutoVoid				(Stream &stream);

	template<typename T1, typename T2, void (Obj::*func)(typename T1::T, typename T2::T)>
	Status					receiveAutoVoid				(Stream &stream);

	template<typename T1, typename T2, typename T3, void (Obj::*func)(typename T1::T, typename T2::T, typename T3::T)>
	Status					receiveAutoVoid				(Stream &stream);

	template<typename T1, typename T2, typename T3, typename T4, void (Obj::*func)(typename T1::T, typename T2::T, typename T3::T, typename T4::T)>
	Status					receiveAutoVoid				(Stream &stream);

	template<typename T1, typename T2, typename T3, typename T4, typename T5, void (Obj::*func)(typename T1::T, typename T2::T, typename T3::T, typename T4::T, typename T5::T)>
	Status					receiveAutoVoid				(Stream &stream);

	template<typename T1, typename T2, typename T3,	typename T4, typename T5, typename T6, void (Obj::*func)(typename T1::T, typename T2::T, typename T3::T, typename T4::T, typename T5::T, typename T6::T)>
	Status					receiveAutoVoid				(Stream &stream);

	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, void (Obj::*func)(typename T1::T, typename T2::T, typename T3::T, typename T4::T, typename T5::T, typename T6::T, typename T7::T)>
	Status					receiveAutoVoid				(Stream &stream);

	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, void (Obj::*func)(typename T1::T, typename T2::T, typename T3::T, typename T4::T, typename T5::T, typename T6::T, typename T7::T, typename T8::T)>
	Status					receiveAutoVoid				(Stream &stream);


																// Auto Receivers for methods that return a value
	template<typename R, R (Obj::*func)(void)>
	Status					receiveAuto					(Stream &stream);

	template<typename R, typename T1, R (Obj::*func)(typename T1::T)>
	Status					receiveAuto					(Stream &stream);

	template<typename R, typename T1, typename T2, R (Obj::*func)(typename T1::T, typename T2::T)> 
	Status					receiveAuto					(Stream &stream);

	template<typename R, typename T1, typename T2, typename T3, R (Obj::*func)(typename T1::T, typename T2::T, typename T3::T)> 
	Status					receiveAuto					(Stream &stream);

	template<typename R, typename T1, typename T2, typename T3, typename T4, R (Obj::*func)(typename T1::T, typename T2::T, typename T3::T, typename T4::T)> 
	Status					receiveAuto					(Stream &stream);

	template<typename R, typename T1, typename T2, typename T3, typename T4, typename T5, R (Obj::*func)(typename T1::T, typename T2::T, typename T3::T, typename T4::T, typename T5::T)> 
	Status					receiveAuto					(Stream &stream);

	template<typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, R (Obj::*func)(typename T1::T, typename T2::T, typename T3::T, typename T4::T, typename T5::T, typename T6::T)> 
	Status					receiveAuto					(Stream &stream);

	template<typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, R (Obj::*func)(typename T1::T, typename T2::T, typename T3::T, typename T4::T, typename T5::T, typename T6::T, typename T7::T)> 
	Status					receiveAuto					(Stream &stream);

	template<typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, R (Obj::*func)(typename T1::T, typename T2::T, typename T3::T, typename T4::T, typename T5::T, typename T6::T, typename T7::T, typename T8::T)> 
	Status					receiveAuto					(Stream &stream);

};

SERVERINTERFACE_T
void PTRMI::ServerInterface<I, Obj>::setObject(RemotableObject *obj)
{
	setServerObject(reinterpret_cast<Obj *>(obj));
}

SERVERINTERFACE_T
RemotableObject *PTRMI::ServerInterface<I, Obj>::getObject(void)
{
	return reinterpret_cast<RemotableObject *>(getServerObject());
}


SERVERINTERFACE_T
PTRMI::ServerInterface<I, Obj>::ServerInterface(void)
{
	createStream();
}

SERVERINTERFACE_T
PTRMI::ServerInterface<I, Obj>::~ServerInterface(void)
{

}

SERVERINTERFACE_T
inline void PTRMI::ServerInterface<I, Obj>::setServerObject(Obj *obj)
{
	object = obj;
}


SERVERINTERFACE_T
inline Obj * PTRMI::ServerInterface<I, Obj>::getServerObject(void)
{
	return object;
}


SERVERINTERFACE_T
inline void PTRMI::ServerInterface<I, Obj>::addServerMethod(const Name &name, Method method)
{
 	methodMap[name] = method;
}


SERVERINTERFACE_T
inline typename ServerInterface<I, Obj>::Method PTRMI::ServerInterface<I, Obj>::getServerMethod(const Name &name)
{
	MethodMap::iterator	it;
															// Look up name
	if((it = methodMap.find(name)) != methodMap.end())
	{
															// If found, return method
		return it->second;
	}

	return NULL;
}


SERVERINTERFACE_T
inline Status PTRMI::ServerInterface<I, Obj>::initializeInvoke(Stream &stream)
{
	Status	status;

	if((status = stream.initializeServerInvoke()).isFailed())
	{
		return status;
	}

	return isInvokeValid(stream);
}


SERVERINTERFACE_T
inline Status PTRMI::ServerInterface<I, Obj>::isInvokeValid(Stream &stream)
{
	if(getServerObject() && stream.getStatus().isOK())
	{
		return Status();
	}

	return Status(Status::Status_Error_Server_Interface_Invoke_Invalid);
}


SERVERINTERFACE_T
inline Status PTRMI::ServerInterface<I, Obj>::invokeServerMethod(const Name &name, Stream &stream)
{
															// Get named method
	Method method = getServerMethod(name);
															// If method is defined
	if(method)
	{
															// Down cast this class to client's server interface class
		I *i = dynamic_cast<I *>(this);
															// If client's interface class is a derived class (it should be)
		if(i)
		{
															// Time stamp this interface's last use
			setTimeLastUsed();
															// Invoke method and return result
			return (i->*method)(stream);
		}
		else
		{
			return PTRMI::Status(PTRMI::Status::Status_Server_Class_Invalid);
		}
	}
															// Method wasn't found
	return PTRMI::Status(PTRMI::Status::Status_Server_Class_Method_Not_Found);
}

SERVERINTERFACE_T
inline unsigned int PTRMI::ServerInterface<I, Obj>::getNumberOfServerMethods(void)
{
	return methodMap.size();
}


SERVERINTERFACE_T
template<typename R>
inline void ServerInterface<I, Obj>::receiveReturnAuto(Stream &stream, R result)
{
															// If method returns result OR method 
	if(isInvokeValid(stream).isOK())
	{
															// Return result to client
		stream << result;
	}
}


SERVERINTERFACE_T
template<void (Obj::*func)(void)>
inline Status ServerInterface<I, Obj>::receiveAutoVoid(Stream &stream)
{
	if(initializeInvoke(stream).isOK())
	{
		(getServerObject()->*func)();
	}

	return stream.getStatus();
}


SERVERINTERFACE_T
template<typename T1, void (Obj::*func)(typename T1::T)>
inline Status ServerInterface<I, Obj>::receiveAutoVoid(Stream &stream)
{
	T1		p1;

	stream >> p1;

	if(initializeInvoke(stream).isOK())
	{
		(getServerObject()->*func)(p1.get());
	}


	if(isInvokeValid(stream).isOK())
	{
		stream << p1;
	}

	return stream.getStatus();
}

SERVERINTERFACE_T
template<typename T1, typename T2, void (Obj::*func)(typename T1::T, typename T2::T)>
inline Status ServerInterface<I, Obj>::receiveAutoVoid(Stream &stream)
{
	T1		p1;
	T2		p2;

	stream >> p1 >> p2;

	if(initializeInvoke(stream).isOK())
	{
		(getServerObject()->*func)(p1.get(), p2.get());
	}

	if(isInvokeValid(stream).isOK())
	{
		stream << p1 << p2;
	}

	return stream.getStatus();
}


SERVERINTERFACE_T
template<typename T1, typename T2, typename T3, void (Obj::*func)(typename T1::T, typename T2::T, typename T3::T)>
inline Status ServerInterface<I, Obj>::receiveAutoVoid(Stream &stream)
{
	T1		p1;
	T2		p2;
	T3		p3;

	stream >> p1 >> p2 >> p3;

	if(initializeInvoke(stream).isOK())
	{
		(getServerObject()->*func)(p1.get(), p2.get(), p3.get());
	}

	if(isInvokeValid(stream).isOK())
	{
		stream << p1 << p2 << p3;
	}

	return stream.getStatus();
}


SERVERINTERFACE_T
template<typename T1, typename T2, typename T3, typename T4, void (Obj::*func)(typename T1::T, typename T2::T, typename T3::T, typename T4::T)>
inline Status ServerInterface<I, Obj>::receiveAutoVoid(Stream &stream)
{
	T1		p1;
	T2		p2;
	T3		p3;
	T4		p4;

	stream >> p1 >> p2 >> p3 >> p4;

	if(initializeInvoke(stream).isOK())
	{
		(getServerObject()->*func)(p1.get(), p2.get(), p3.get(), p4.get());
	}

	if(isInvokeValid(stream).isOK())
	{
		stream << p1 << p2 << p3 << p4;
	}

	return stream.getStatus();
}

SERVERINTERFACE_T
template<typename T1, typename T2, typename T3, typename T4, typename T5, void (Obj::*func)(typename T1::T, typename T2::T, typename T3::T, typename T4::T, typename T5::T)>
inline Status ServerInterface<I, Obj>::receiveAutoVoid(Stream &stream)
{
	T1		p1;
	T2		p2;
	T3		p3;
	T4		p4;
	T5		p5;

	stream >> p1 >> p2 >> p3 >> p4 >> p5;

	if(initializeInvoke(stream).isOK())
	{
		(getServerObject()->*func)(p1.get(), p2.get(), p3.get(), p4.get(), p5.get());
	}


	if(isInvokeValid(stream).isOK())
	{
		stream << p1 << p2 << p3 << p4 << p5;
	}

	return stream.getStatus();
}


SERVERINTERFACE_T
template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, void (Obj::*func)(typename T1::T, typename T2::T, typename T3::T, typename T4::T, typename T5::T, typename T6::T)>
inline Status ServerInterface<I, Obj>::receiveAutoVoid(Stream &stream)
{
	T1		p1;
	T2		p2;
	T3		p3;
	T4		p4;
	T5		p5;
	T6		p6;

	stream >> p1 >> p2 >> p3 >> p4 >> p5 >> p6;

	if(initializeInvoke(stream).isOK())
	{
		(getServerObject()->*func)(p1.get(), p2.get(), p3.get(), p4.get(), p5.get(), p6.get());
	}

	if(isInvokeValid(stream).isOK())
	{
		stream << p1 << p2 << p3 << p4 << p5 << p6;
	}

	return stream.getStatus();
}


SERVERINTERFACE_T
template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, void (Obj::*func)(typename T1::T, typename T2::T, typename T3::T, typename T4::T, typename T5::T, typename T6::T, typename T7::T)>
inline Status ServerInterface<I, Obj>::receiveAutoVoid(Stream &stream)
{
	T1		p1;
	T2		p2;
	T3		p3;
	T4		p4;
	T5		p5;
	T6		p6;
	T7		p7;

	stream >> p1 >> p2 >> p3 >> p4 >> p5 >> p6 >> p7;

	if(initializeInvoke(stream).isOK())
	{
		(getServerObject()->*func)(p1.get(), p2.get(), p3.get(), p4.get(), p5.get(), p6.get(), p7.get());
	}

	if(isInvokeValid(stream).isOK())
	{
		stream << p1 << p2 << p3 << p4 << p5 << p6 << p7;
	}

	return stream.getStatus();
}


SERVERINTERFACE_T
template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, void (Obj::*func)(typename T1::T, typename T2::T, typename T3::T, typename T4::T, typename T5::T, typename T6::T, typename T7::T, typename T8::T)>
inline Status ServerInterface<I, Obj>::receiveAutoVoid(Stream &stream)
{
	T1		p1;
	T2		p2;
	T3		p3;
	T4		p4;
	T5		p5;
	T6		p6;
	T7		p7;
	T8		p8;

	stream >> p1 >> p2 >> p3 >> p4 >> p5 >> p6 >> p7 >> p8;

	if(initializeInvoke(stream).isOK())
	{
		(getServerObject()->*func)(p1.get(), p2.get(), p3.get(), p4.get(), p5.get(), p6.get(), p7.get(), p8.get());
	}

	if(isInvokeValid(stream).isOK())
	{
		stream << p1 << p2 << p3 << p4 << p5 << p6 << p7 << p8;
	}

	return stream.getStatus();
}


// ******************************************************************************************

SERVERINTERFACE_T
template<typename R, R (Obj::*func)(void)>
inline Status ServerInterface<I, Obj>::receiveAuto(Stream &stream)
{

	if(initializeInvoke(stream).isOK())
	{
		R result = (getServerObject()->*func)();

		receiveReturnAuto(stream, result);
	}

	return stream.getStatus();
}

SERVERINTERFACE_T
template<typename R, typename T1, R (Obj::*func)(typename T1::T)>
inline Status ServerInterface<I, Obj>::receiveAuto(Stream &stream)
{
	T1		p1;

	stream >> p1;

	if(initializeInvoke(stream).isOK())
	{
		R result = (getServerObject()->*func)(p1.get());

		if(isInvokeValid(stream).isOK())
		{
			stream << p1;
		}

		receiveReturnAuto(stream, result);
	}

	return stream.getStatus();
}

SERVERINTERFACE_T
template<typename R, typename T1, typename T2, R (Obj::*func)(typename T1::T, typename T2::T)>
inline Status ServerInterface<I, Obj>::receiveAuto(Stream &stream)
{
	T1		p1;
	T2		p2;
	Status	status;


	stream >> p1 >> p2;

	if(initializeInvoke(stream).isOK())
	{
		R result = (getServerObject()->*func)(p1.get(), p2.get());

		if(isInvokeValid(stream).isOK())
		{
			stream << p1 << p2;
		}

		receiveReturnAuto(stream, result);
	}

	return stream.getStatus();
}


SERVERINTERFACE_T
template<typename R, typename T1, typename T2, typename T3, R (Obj::*func)(typename T1::T, typename T2::T, typename T3::T)>
inline Status ServerInterface<I, Obj>::receiveAuto(Stream &stream)
{
	T1		p1;
	T2		p2;
	T3		p3;

	stream >> p1 >> p2 >> p3;

	if(initializeInvoke(stream).isOK())
	{
		R result = (getServerObject()->*func)(p1.get(), p2.get(), p3.get());

		if(isInvokeValid(stream).isOK())
		{
			stream << p1 << p2 << p3;
		}

		receiveReturnAuto(stream, result);
	}

	return stream.getStatus();
}


SERVERINTERFACE_T
template<typename R, typename T1, typename T2, typename T3, typename T4, R (Obj::*func)(typename T1::T, typename T2::T, typename T3::T, typename T4::T)>
inline Status ServerInterface<I, Obj>::receiveAuto(Stream &stream)
{
	T1		p1;
	T2		p2;
	T3		p3;
	T4		p4;

	stream >> p1 >> p2 >> p3 >> p4;

	if(initializeInvoke(stream).isOK())
	{
		R result = (getServerObject()->*func)(p1.get(), p2.get(), p3.get(), p4.get());

		if(isInvokeValid(stream).isOK())
		{
			stream << p1 << p2 << p3 << p4;
		}

		receiveReturnAuto(stream, result);
	}

	return stream.getStatus();
}



SERVERINTERFACE_T
template<typename R, typename T1, typename T2, typename T3, typename T4, typename T5, R (Obj::*func)(typename T1::T, typename T2::T, typename T3::T, typename T4::T, typename T5::T)>
inline Status ServerInterface<I, Obj>::receiveAuto(Stream &stream)
{
	T1		p1;
	T2		p2;
	T3		p3;
	T4		p4;
	T5		p5;

	stream >> p1 >> p2 >> p3 >> p4 >> p5;

	if(initializeInvoke(stream).isOK())
	{
		R result = (getServerObject()->*func)(p1.get(), p2.get(), p3.get(), p4.get(), p5.get());

		if(isInvokeValid(stream).isOK())
		{
			stream << p1 << p2 << p3 << p4 << p5;
		}

		receiveReturnAuto(stream, result);
	}

	return stream.getStatus();
}


SERVERINTERFACE_T
template<typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, R (Obj::*func)(typename T1::T, typename T2::T, typename T3::T, typename T4::T, typename T5::T, typename T6::T)>
inline Status ServerInterface<I, Obj>::receiveAuto(Stream &stream)
{
	T1		p1;
	T2		p2;
	T3		p3;
	T4		p4;
	T5		p5;
	T6		p6;

	stream >> p1 >> p2 >> p3 >> p4 >> p5 >> p6;

	if(initializeInvoke(stream).isOK())
	{
		R result = (getServerObject()->*func)(p1.get(), p2.get(), p3.get(), p4.get(), p5.get(), p6.get());

		if(isInvokeValid(stream).isOK())
		{
			stream << p1 << p2 << p3 << p4 << p5 << p6;
		}

		receiveReturnAuto(stream, result);
	}

	return stream.getStatus();
}


SERVERINTERFACE_T
template<typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, R (Obj::*func)(typename T1::T, typename T2::T, typename T3::T, typename T4::T, typename T5::T, typename T6::T, typename T7::T)>
inline Status ServerInterface<I, Obj>::receiveAuto(Stream &stream)
{
	T1		p1;
	T2		p2;
	T3		p3;
	T4		p4;
	T5		p5;
	T6		p6;
	T7		p7;

	stream >> p1 >> p2 >> p3 >> p4 >> p5 >> p6 >> p7;

	if(initializeInvoke(stream).isOK())
	{
		R result = (getServerObject()->*func)(p1.get(), p2.get(), p3.get(), p4.get(), p5.get(), p6.get(), p7.get());

		if(isInvokeValid(stream).isOK())
		{
			stream << p1 << p2 << p3 << p4 << p5 << p6 << p7;
		}

		receiveReturnAuto(stream, result);
	}

	return stream.getStatus();
}

SERVERINTERFACE_T
template<typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, R (Obj::*func)(typename T1::T, typename T2::T, typename T3::T, typename T4::T, typename T5::T, typename T6::T, typename T7::T, typename T8::T)>
inline Status ServerInterface<I, Obj>::receiveAuto(Stream &stream)
{
	Status	status;
	T1		p1;
	T2		p2;
	T3		p3;
	T4		p4;
	T5		p5;
	T6		p6;
	T7		p7;
	T8		p8;

	stream >> p1 >> p2 >> p3 >> p4 >> p5 >> p6 >> p7 >> p8;

	if(initializeInvoke(stream).isOK())
	{
		R result = (getServerObject()->*func)(p1.get(), p2.get(), p3.get(), p4.get(), p5.get(), p6.get(), p7.get(), p8.get());

		if(isInvokeValid(stream).isOK())
		{
			stream << p1 << p2 << p3 << p4 << p5 << p6 << p7 << p8;
		}

		receiveReturnAuto(stream, result);
	}

	return stream.getStatus();
}



} // End PTRMI namespace

