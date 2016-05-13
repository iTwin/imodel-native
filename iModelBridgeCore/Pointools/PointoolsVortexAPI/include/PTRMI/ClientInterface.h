#pragma once



#ifdef NEEDS_WORK_VORTEX_DGNDB
#include <PTRMI/PTRMI.h>
#include <PTRMI/Stream.h>
#include <PTRMI/InterfaceBase.h>

#include <PTRMI/ClientInterfaceExtData.h>

// ************************************************************************
// * Macros
// ************************************************************************

#define PTRMI_DECLARE_CLIENT_INTERFACE(Inter) \
class Inter : public PTRMI::ClientInterface

#define PTRMI_USE_CLIENT_INTERFACE(Inter, Obj) \
template<> class PTRMI::ObjClientInfo<Obj> {public: typedef Inter I; typedef std::vector<I *> Set;};

#define PTRMI_CLIENT_INTERFACE_RECALL_MAX	10


namespace PTRMI
{

//class ClientInterfaceExtData;

class ClientInterfaceBase : public InterfaceBase
{

protected:

	Status							status;

	Mutex							interfaceMutex;
	ClientInterfaceExtData		*	extData;

	int								sharedCounter;

protected:

	void							setSharedCounter		(int counter);
	int								getSharedCounter		(void);



public:

									ClientInterfaceBase		(void);
							       ~ClientInterfaceBase		(void);

	Mutex &							getInterfaceMutex		(void);

	void							setExtData				(ClientInterfaceExtData *initExtData);
	ClientInterfaceExtData		 *	getExtData				(void);

	Status							lock					(void);
	Status							unlock					(void);

	Status 							beginShared				(ClientInterfaceExtData *extData);
	Status 							endShared				(void);

	void							setStatus				(Status initStatus);
	Status 							getStatus				(void) const;
	void							resetStatus				(void);

	Status 							releaseStream			(void);

	virtual Status					recoverRemoteObject		(void) {return Status(Status::Status_Error_Remote_Object_Not_Recoverable);}
};


class ClientInterface : public ClientInterfaceBase
{

protected:

public:

	ClientInterface(void)
	{
		createStream();
	}

	~ClientInterface(void)
	{

	}

	bool isClientValid(void) const
	{
		return (getStatus().isOK() && getStream() != NULL);
	}

															// Begin invocation
	Status beginMethod(Stream &stream, const MethodName &methodName)
	{
		Status	status;

		if((status = stream.beginClientMethod(methodName)).isFailed())
		{
			setStatus(status);
		}

		return status;
	}

	Status invokeMethod(Stream &stream, bool &recall)
	{
		Status	status;
															// Default is not to recall
		recall = false;

		if((status = stream.invokeClientMethod(*this)).isFailed() || getStatus().isFailed())
		{
															// If failure occurred, end method now
			endMethod(stream, true);

			if(status.is(Status::Status_Error_Pipe_Failed))
			{
															// Exit with no recall
				return status;
			}

			if(getStatus().is(Status::Status_Error_Failed_To_Find_Server_Interface))
			{
															// Attempt to reproduce required ServerInterface and possibly Object on the server
				if((status = recoverRemoteObject()).isFailed())
				{
					endMethod(stream, true);

					return status;
				}

				recall = true;

				return Status(Status::Status_Error_Send_Invoke_Failed);
			}
		}
															// Return whether call mechanism succeeded
		return getStatus();
	}

	Status endMethod(Stream &stream, bool methodCancelled = false)
	{
		Status	status;

		if((status = stream.endClientMethod(methodCancelled)).isFailed())
		{
			setStatus(status);
		}
															// Time stamp last successful usage of the ClientInterface
		if(status.isOK())
		{
			setTimeLastUsed();
		}

		return status;
	}

// -------------------------------------------

	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	typename void sendAutoVoid(const MethodName &methodName, typename T1::T p1 = PVoid(), typename T2::T p2 = PVoid(), typename T3::T p3 = PVoid(), typename T4::T p4 = PVoid(), typename T5::T p5 = PVoid(), typename T6::T p6 = PVoid(), typename T7::T p7 = PVoid(), typename T8::T p8 = PVoid(), unsigned int recallCount = 0)
	{
		if(isClientValid() == false)
		{
			Status status(Status::Status_Client_Stream_Not_Set);
			return;
		}
															// Set a limit on the depth of recalls to prevent infinite loops
		if(recallCount >= PTRMI_CLIENT_INTERFACE_RECALL_MAX)
		{
			return;
		}

		bool recall;
															// Re-wrap parameters with PC and Modifiers
		T1	mp1(p1);
		T2	mp2(p2);
		T3	mp3(p3);
		T4	mp4(p4);
		T5	mp5(p5);
		T6	mp6(p6);
		T7	mp7(p7);
		T8	mp8(p8);

#ifdef PTRMI_LOGGING
Status::log(L"Client Interface beginMethod void", methodName);
#endif
															// Begin method invocation
		if(beginMethod(*stream, methodName).isFailed())
		{
			return;
		}
															// Send to stream. Stream knows if parameter is really sent or not
															// based on rule base
		(*stream) << mp1 << mp2 << mp3 << mp4 << mp5 << mp6 << mp7 << mp8;

															// Invoke call
		if(invokeMethod(*stream, recall).isFailed())
		{
															// If recall specified, recall this function
			if(recall)
			{
				sendAutoVoid<T1, T2, T3, T4, T5, T6, T7, T8>(methodName, p1, p2, p3, p4, p5, p6, p7, p8, recallCount + 1);

				return;
			}
			return;
		}

		(*stream) >> mp1 >> mp2 >> mp3 >> mp4 >> mp5 >> mp6 >> mp7 >> mp8;

		endMethod(*stream);
	}

	template<typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	typename R sendAuto(R errorValue, const MethodName methodName, typename T1::T p1 = PVoid(), typename T2::T p2 = PVoid(), typename T3::T p3 = PVoid(), typename T4::T p4 = PVoid(), typename T5::T p5 = PVoid(), typename T6::T p6 = PVoid(), typename T7::T p7 = PVoid(), typename T8::T p8 = PVoid(), unsigned int recallCount = 0)
	{
		if(isClientValid() == false)
		{
			Status status(Status::Status_Client_Stream_Not_Set);
			return errorValue;
		}
															// Set a limit on the depth of recalls to prevent infinite loops
		if(recallCount >= PTRMI_CLIENT_INTERFACE_RECALL_MAX)
		{
			return errorValue;
		}

		bool recall;
															// Re-wrap parameters with PC and Modifiers
		T1	mp1(p1);
		T2	mp2(p2);
		T3	mp3(p3);
		T4	mp4(p4);
		T5	mp5(p5);
		T6	mp6(p6);
		T7	mp7(p7);
		T8	mp8(p8);

#ifdef PTRMI_LOGGING
Status::log(L"Client Interface beginMethod", methodName);
#endif

															// Begin method invocation
		if(beginMethod(*stream, methodName).isFailed())
		{
			return errorValue;
		}
															// Send to stream. Stream knows if parameter is really sent or not
															// based on rule base
		(*stream) << mp1 << mp2 << mp3 << mp4 << mp5 << mp6 << mp7 << mp8;

															// Invoke call
		if(invokeMethod(*stream, recall).isFailed())
		{
															// If recall specified, recall this function
			if(recall)
			{
				return sendAuto<R, T1, T2, T3, T4, T5, T6, T7, T8>(errorValue, methodName, p1, p2, p3, p4, p5, p6, p7, p8, recallCount + 1);
			}

			return errorValue;
		}
															// Create wrapped PC and Modifier Return type
		R  result;

															// Receive any outputs from the method
		(*stream) >> mp1 >> mp2 >> mp3 >> mp4 >> mp5 >> mp6 >> mp7 >> mp8;
															// Receive result from the server
		(*stream) >> result;

															// Method ends
		endMethod(*stream);
															// Return method result value
		return result;
	}


	typedef PC<In<PVoid> >	PCInVoid;

	void sendAutoVoid(const MethodName &method) {sendAutoVoid<PCInVoid, PCInVoid, PCInVoid, PCInVoid, PCInVoid, PCInVoid, PCInVoid, PCInVoid>(method);}


	template<typename T1>
	void sendAutoVoid(const MethodName &method, typename T1::T p1) {sendAutoVoid<T1, PCInVoid, PCInVoid, PCInVoid, PCInVoid, PCInVoid, PCInVoid, PCInVoid>(method, p1);}

	template<typename T1, typename T2>
	void sendAutoVoid(const MethodName &method, typename T1::T p1, typename T2::T p2) {sendAutoVoid<T1, T2, PCInVoid, PCInVoid, PCInVoid, PCInVoid, PCInVoid, PCInVoid>(method, p1, p2);}

	template<typename T1, typename T2, typename T3>
	void sendAutoVoid(const MethodName &method, typename T1::T p1, typename T2::T p2, typename T3::T p3) {sendAutoVoid<T1, T2, T3, PCInVoid, PCInVoid, PCInVoid, PCInVoid, PCInVoid>(method, p1, p2, p3);}

	template<typename T1, typename T2, typename T3, typename T4>
	void sendAutoVoid(const MethodName &method, typename T1::T p1, typename T2::T p2, typename T3::T p3, typename T4::T p4) {sendAutoVoid<T1, T2, T3, T4, PCInVoid, PCInVoid, PCInVoid, PCInVoid>(method, p1, p2, p3, p4);}

	template<typename T1, typename T2, typename T3, typename T4, typename T5>
	void sendAutoVoid(const MethodName &method, typename T1::T p1, typename T2::T p2, typename T3::T p3, typename T4::T p4, typename T5::T p5) {sendAutoVoid<T1, T2, T3, T4, T5, PCInVoid, PCInVoid, PCInVoid>(method, p1, p2, p3, p4, p5);}

	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	void sendAutoVoid(const MethodName &method, typename T1::T p1, typename T2::T p2, typename T3::T p3, typename T4::T p4, typename T5::T p5, typename T6::T p6) {sendAutoVoid<T1, T2, T3, T4, T5, T6, PCInVoid, PCInVoid>(method, p1, p2, p3, p4, p5, p6);}

	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	void sendAutoVoid(const MethodName &method, typename T1::T p1, typename T2::T p2, typename T3::T p3, typename T4::T p4, typename T5::T p5, typename T6::T p6, typename T7::T p7) {sendAutoVoid<T1, T2, T3, T4, T5, T6, T7, PCInVoid>(method, p1, p2, p3, p4, p5, p6, p7);}

// -----

	template<typename R>
	typename R sendAuto(R errorValue, const MethodName &method) {return sendAuto<R, PCInVoid, PCInVoid, PCInVoid, PCInVoid, PCInVoid, PCInVoid, PCInVoid, PCInVoid>(errorValue, method);}

	template<typename R, typename T1>
	typename R sendAuto(R errorValue, const MethodName &method, typename T1::T p1) {return sendAuto<R, T1, PCInVoid, PCInVoid, PCInVoid, PCInVoid, PCInVoid, PCInVoid, PCInVoid>(errorValue, method, p1);}

	template<typename R, typename T1, typename T2>
	typename R sendAuto(R errorValue, const MethodName &method, typename T1::T p1, typename T2::T p2) {return sendAuto<R, T1, T2, PCInVoid, PCInVoid, PCInVoid, PCInVoid, PCInVoid, PCInVoid>(errorValue, method, p1 ,p2);}

	template<typename R, typename T1, typename T2, typename T3>
	typename R sendAuto(R errorValue, const MethodName &method, typename T1::T p1, typename T2::T p2, typename T3::T p3) {return sendAuto<R, T1, T2, T3, PCInVoid, PCInVoid, PCInVoid, PCInVoid, PCInVoid>(errorValue, method, p1 ,p2, p3);}

	template<typename R, typename T1, typename T2, typename T3, typename T4>
	typename R sendAuto(R errorValue, const MethodName &method, typename T1::T p1, typename T2::T p2, typename T3::T p3, typename T4::T p4) {return sendAuto<R, T1, T2, T3, T4, PCInVoid, PCInVoid, PCInVoid, PCInVoid>(errorValue, method, p1 ,p2, p3, p4);}

	template<typename R, typename T1, typename T2, typename T3, typename T4, typename T5>
	typename R sendAuto(R errorValue, const MethodName &method, typename T1::T p1, typename T2::T p2, typename T3::T p3, typename T4::T p4, typename T5::T p5) {return sendAuto<R, T1, T2, T3, T4, T5, PCInVoid, PCInVoid, PCInVoid>(errorValue, method, p1 ,p2, p3, p4, p5);}

	template<typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	typename R sendAuto(R errorValue, const MethodName &method, typename T1::T p1, typename T2::T p2, typename T3::T p3, typename T4::T p4, typename T5::T p5, typename T6::T p6) {return sendAuto<R, T1, T2, T3, T4, T5, T6, PCInVoid, PCInVoid>(errorValue, method, p1 ,p2, p3, p4, p5, p6);}

	template<typename R, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	typename R sendAuto(R errorValue, const MethodName &method, typename T1::T p1, typename T2::T p2, typename T3::T p3, typename T4::T p4, typename T5::T p5, typename T6::T p6, typename T7::T p7) {return sendAuto<R, T1, T2, T3, T4, T5, T6, T7, PCInVoid>(errorValue, method, p1 ,p2, p3, p4, p5, p6, p7);}
};



}
#else

namespace PTRMI
    {
    class ClientInterfaceBase;
    };

#endif