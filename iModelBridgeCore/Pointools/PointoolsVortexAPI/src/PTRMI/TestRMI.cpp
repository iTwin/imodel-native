// RMI.cpp : Defines the entry point for the console application.
//


#include <PTRMI/ptrmi.h>

#include <PTRMI/ClientInterface.h>
#include <PTRMI/ServerInterface.h>

#include <PTRMI/ObjectManager.h>

#include <PTRMI/RemotePtr.h>

#include <PTRMI/Manager.h>
#include <PTRMI/PipeManagerExt.h>

#include <PTRMI/Stream.h>

#include <ptds/DataSourceFile.h>
#include <ptds/DataSourceCache.h>
#include <ptds/DataSourceServer.h>


using namespace PTRMI;

class MyObjClass
{

public:

	int a;


public:

			void	f_void_void				(void)		{};

			float	f_float_float			(float v)	{return 2*v;}

			void	f_void_float			(float v)	{}

			float	f_float_float_ptr		(float *v)	{return 3*(*v);}

	virtual float	f_virt_float_float		(float v) = 0;

			int		f_int_int_array			(int *a, int size)
			{
				int t;

				if(a)
				{
					for(t = 0; t < size; t++)
					{
						a[t] = t + 1;
					}
				}

				return t;
			}
};


class MyDerivedObjClass : public MyObjClass
{

public:

	float b;


public:

	float f_virt_float_float(float v) {return 10*v;}

};



PTRMI_DECLARE_CLIENT_INTERFACE(MyClientInterface)
{
public:

	void	f_void_void			(void)				{sendAutoVoid(L"f_void_void");}

	float	f_float_float		(float v)			{return sendAuto<float, PC<In<float>>>(0, L"f_float_float",  v);}

	float	f_float_float_ptr	(float *v)			{return sendAuto<float, PC<InOut<float *>> >(0, L"f_float_float_ptr", v);}

	float	f_float_float_ref	(float &v)			{return sendAuto<float, PC<InOut<float *>> >(0, L"f_float_float_ref", &v);}

	int		f_int_int_array		(int *a, int size)	{return sendAuto<int, PC<Out<ArrayDirect<int>>>, PC<In<int>> >(0, L"f_int_int_array", ArrayDirect<int>(size, a), size);}

//	virtual float	f_virt		(float v) = 0;

};


PTRMI_DECLARE_SERVER_INTERFACE(MyServerInterface, Obj)
{

public:

protected:

	PTRMI::Status f(PTRMI::Stream &stream)
	{
		return PTRMI::Status_OK;
	}

public:

	MyServerInterface(void)
	{
		addServerMethod(PTRMI::Name(L"f_void_void"), &Super::receiveAutoVoid<&MyObjClass::f_void_void>);

		addServerMethod(PTRMI::Name(L"f_float_float"), &Super::receiveAuto<float, PS<In<float>>, &MyObjClass::f_float_float>);

		addServerMethod(PTRMI::Name(L"f_void_float"), &Super::receiveAutoVoid<PS<InOut<float>>, &MyObjClass::f_void_float>);

		addServerMethod(PTRMI::Name(L"f_float_float_ptr"), &Super::receiveAuto<float, PS<In<float *>>, &MyObjClass::f_float_float_ptr>);

		addServerMethod(PTRMI::Name(L"f_virt_float_float"), &Super::receiveAuto<float, PS<In<float>>, &MyObjClass::f_virt_float_float>);

		addServerMethod(PTRMI::Name(L"f_int_int_array"), &Super::receiveAuto<int, PS<Out<ArrayDirect<int>>>, PS<In<int>>, &MyObjClass::f_int_int_array>);
	}
};

PTRMI_USE_CLIENT_INTERFACE(MyClientInterface, MyDerivedObjClass);

PTRMI_INSTANCE_SERVER_INTERFACE(MyServerInterface<MyObjClass>, MyObjClass)
PTRMI_INHERIT_SERVER_INTERFACE(MyServerInterface<MyObjClass>, MyDerivedObjClass)


void testClient(void)
{
	MyClientInterface	ci;

	ci.f_void_void();

	float r1 = ci.f_float_float((float) 1.234);

	float v = 3.5;
	ci.f_float_float_ptr(&v);

	float vr = 4.5;
	ci.f_float_float_ref(vr);

	float r2 = ci.f_float_float((float) 1.2);
	
	//PTRMI::RemotePtr<MyObjClass> remoteObj = PTRMI::getManager().getRemoteObject<MyObjClass>(PTRMI::Name(L"MyObjClass"));
	//remoteObj->f_void_void();


	RemotePtr<MyDerivedObjClass> remoteObj = getManager().newRemoteObject<MyDerivedObjClass>(Name(L"MyObjClass"), Name(L"MyObj"));

}


void testServer(void)
{
	MyServerInterface<MyObjClass>	si;
	PTRMI::Stream					s;
	PTRMI::Status					status;
	MyDerivedObjClass				obj;

	si.setServerObject(&obj);

	status = si.invokeServerMethod(PTRMI::Name(L"f_void_void"), s);
	status = si.invokeServerMethod(PTRMI::Name(L"f_float_float"), s);
	status = si.invokeServerMethod(PTRMI::Name(L"f_void_float"), s);
	status = si.invokeServerMethod(PTRMI::Name(L"f_float_float_ptr"), s);
	status = si.invokeServerMethod(PTRMI::Name(L"f_virt_float_float"), s);

}


void testName(void)
{
	Name			name;
	HostAddressIP4	hostAddressIP4;
	Name			protocol;
	Name			obj;

	name.set(L"PTRS://192.168.0.1/Manager");
	name.getProtocol(protocol);
	name.getHostAddress(hostAddressIP4);

	name.set(L"PTRS://pointools.com/Manager");
	name.getProtocol(protocol);
	name.getHostAddress(hostAddressIP4);

	name.set(L"PTRS://pointools.com");
	name.getProtocol(protocol);
	name.getHostAddress(hostAddressIP4);

	name.set(L"PTRS://802pointools.com");
	name.getProtocol(protocol);
	name.getHostAddress(hostAddressIP4);

	name.set(L"PTRS://192.168.0.1/Manager");
	name.getProtocol(protocol);
	name.getObject(obj);

	name.set(L"PTRS://192.168.0.1/Manager/Obj_1");
	name.getProtocol(protocol);
	name.getObject(obj);

	bool guidValid = name.getGUID().isValidGUID();

	name.generate();

	guidValid = name.getGUID().isValidGUID();
}

template<typename T, unsigned int s> class X
{
public:

	T array[s];

	X(void)
	{
		unsigned int t;
		for(t = 0; t < s; t++)
			array[t] = (T) 0;
	}

	void count(void)
	{
		unsigned int t;
		for(t = 0; t < s; t++)
			array[t] = (T) t;
	}

	void countBack(void)
	{
		unsigned int t;
		for(t = 0; t < s; t++)
			array[s - t - 1] = (T) t;
	}

	Status read(DataBuffer &buffer)
	{
		return Status();
	}

	Status write(DataBuffer &buffer)
	{
		return Status();
	}

};

void testDataBuffer(void)
{
	DataBuffer			buffer(1024 * 1024);								// Internal Buffer
//	DataBuffer			buffer(new unsigned char[1024*1024], 1024*1024);	// External Buffer

	unsigned char	*	output;

	if((output = new unsigned char[1024 * 1024 * 5]) == NULL)
		return;


	buffer << (unsigned char) 1 << (unsigned char) 2 << (unsigned char) 3;

	X<float, 10>			xIn;
	X<float, 10>			xOut;
	X<unsigned char, 5>		xAllocate;
	Name					nameIn(L"PTRS://192.168.0.1/Manager");
	Name					nameOut;

	nameIn.generate();

	xIn.count();
	xAllocate.countBack();

	buffer << xIn;


	float floatDataIn[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	float floatDataOut[10];

	PTRMI::Array<float>	floatArrayIn(10, floatDataIn);
	PTRMI::Array<float> floatArrayOut(10, floatDataOut);

	buffer << floatArrayIn;

	buffer << nameIn;

	buffer.reset();

	unsigned char c[10];

	buffer >> c[0] >> c[1] >> c[2];

	buffer >> xOut;

	buffer >> floatArrayOut;

	buffer >> nameOut;

	buffer.reset();

	buffer.readFromBuffer((void *) output, buffer.getBufferSize());

	bool sameName = (nameOut == nameIn);

	if(output)
		delete output;

	int a = 1;
}


void testRemote(void)
{
	RemotePtr<MyDerivedObjClass> myObj = PTRMI::getManager().newRemoteObject<MyDerivedObjClass>(Name(L"MyClass"), Name(L"PTRS://192.168.0.1/Root/MyObjects/Object1"));

	PTRMI::getManager().discardRemoteObject(myObj);
}



void testObjectManager(void)
{

	ObjectManager *objectManager = getManager().getObjectManager();


	objectManager->newMetaInterface<MyDerivedObjClass>(L"MyDerivedObjClass");

	RemotableObject *obj = objectManager->newObject(Name(L"MyDerivedObjClass"), Name(L"MyDerivedObjNew"));

	MyDerivedObjClass * obj2 = objectManager->newObject<MyDerivedObjClass>(Name(L"MyDerivedObjClass"), Name(L"MyDerivedObjNew2"));


	RemotePtr<MyDerivedObjClass> myDerivedObj = objectManager->newObjectClientInterface<MyDerivedObjClass>(Name(L"MyDerivedObj"), &Name(L"DummyRemoteServerInterface"));
	
	myDerivedObj->f_void_void();

// Note:	This is a bit wrong, as it is a method to create a new object and server interface for it.
//			newObjectServerInterface() should not need the class name if possible

// From an object name, we need to be able to map back to it's type's MetaInterface

// Instead of storing a set of Name->RemoteObject pointers, store a set of Name->ServerInterfaceBase pointers
// of the Server Interfaces that own the objects. This way, virtual functions can t

	objectManager->newObjectServerInterface(Name(L"MyDerivedObjClass"), Name(L"MyNewObject?"));
}


void testProtocolManager(void)
{

}


void testManager(void)
{
	Status status;

//	RemotePtr<Manager> newManager = PTRMI::getManager().newRemoteObject<Manager>(Name(L"Manager"), Name(L"PTRI://192.168.0.1/Manager2"), status);


}



void testCalls(const Name &objectName)
{
	Status	status;

	RemotePtr<MyDerivedObjClass> remoteObj = getManager().newRemoteObject<MyDerivedObjClass>(Name(L"MyDerivedObjClass"), objectName);

	int a[25];
	int n = remoteObj->f_int_int_array(a, 25);

	float r1 = remoteObj->f_float_float(5);
	float r2 = remoteObj->f_float_float(25);

	status = getManager().discardRemoteObject(remoteObj);

}

void testTCP(void)
{
	Status status;

	testCalls(Name(L"PTRI://localhost/MyDerivedObj"));
}


unsigned int serverCallback(void *sendBuffer, unsigned int sendSize, void *extData, unsigned int extDataSize, void **receiveBuffer, unsigned int *receiveSize)
{
	Status	status;

	PTRMI::PipeManagerExt *pipeManagerExt = dynamic_cast<PTRMI::PipeManagerExt *>(getManager().getPipeProtocolManager().getPipeManager(std::wstring(L"Ext")));
															// Cross over send and receive
	status = pipeManagerExt->receiveExternalCall(sendBuffer, sendSize, receiveBuffer, receiveSize);

	return static_cast<unsigned int>(status.get());
}


void testExt(void)
{
	Status status;

	PTRMI::PipeManagerExt *pipeManagerExt = dynamic_cast<PTRMI::PipeManagerExt *>(getManager().getPipeProtocolManager().getPipeManager(std::wstring(L"Ext")));

	pipeManagerExt->setSendExternalCallFunction(serverCallback);

	testCalls(Name(L"PTRE://localhost/MyDerivedObj"));
}


void initializeTests(void)
{
	getManager().newMetaInterface<MyDerivedObjClass>(L"MyDerivedObjClass");
}


bool createDataSourceCacheTestFile(const wchar_t *filePath)
{
	if(filePath == NULL)
		return false;

	ptds::DataSourceFile	f;
	
	typedef unsigned long CacheTestDataType;

	f.openForWrite(&ptds::FilePath(filePath));

	unsigned int numItems = 1024*1024;
	unsigned int numBytes = numItems * sizeof(CacheTestDataType);

	CacheTestDataType *buffer = new CacheTestDataType[numItems];
	if(buffer == NULL)
		return false;

	CacheTestDataType t;
	
	for(t = 1; t <= numItems; t++)
	{
		buffer[t - 1] = t;		
	}

	f.writeBytes(reinterpret_cast<ptds::DataSource::Data *>(buffer), numBytes);

	f.close();

	return true;
}

void testDataSourceCacheSequential(void)
{
	ptds::DataSourceCache	d;

//	createDataSourceCacheTestFile(L"C:\\CacheTestFile.pod");

	d.setCacheFilePath(L"C:\\CacheTestFile.podc");
//	d.setCachePageSize(1024 * 128);
	d.setCachePageSize(16);

	d.openForRead(&ptds::FilePath(L"C:\\CacheTestFile.pod"));

	unsigned long	totalFileSize	= 1024*1024*4;
	unsigned char	buffer[1024];

	unsigned long	numItems		= 1024*1024;
	unsigned long	numItemsRead	= 5;
	unsigned int	t, i;


	for(t = 1; t <= numItems; t += numItemsRead)
	{
		d.readBytes(buffer, numItemsRead * sizeof(unsigned long));
	
		for(i = 0; i < numItemsRead && ((t + i) <= numItems); i++)
		{
			if(((unsigned long *) buffer)[i] != t + i)
			{
				int a = 1;
			}
		}
	}


	d.close();

}


bool testDataSourceCacheRandom(void)
{
	ptds::DataSourceCache	d;

	//	createDataSourceCacheTestFile(L"C:\\CacheTestFile.pod");

	d.setCacheFilePath(L"C:\\CacheTestFile.podc");
	//	d.setCachePageSize(1024 * 128);
	d.setCachePageSize(16);

	d.openForRead(&ptds::FilePath(L"C:\\CacheTestFile.pod"));

	ptds::DataSource::DataSize totalFileSize = d.getFileSize();

	unsigned char *	buffer;

	if((buffer = new unsigned char[totalFileSize]) == NULL)
		return false;

	unsigned long	numReads		= 1024 * 20;
	unsigned long	numItemsRead	= 1;
	unsigned int	t, i;

	ptds::DataSource::DataPointer	position;
	ptds::DataSource::DataPointer	positionBytes;
	ptds::DataSource::DataSize		r;
	ptds::DataSource::DataSize		numBytesRead;
	ptds::DataSource::DataSize		sizeOfType = sizeof(unsigned long);


	for(t = 1; t <= numReads; t++)
	{
		position = ((totalFileSize / sizeOfType) - 1) * (((double) rand()) / ((double) RAND_MAX));

		positionBytes = position * sizeOfType;

		r = numItemsRead;
		numBytesRead = r * sizeOfType;
															// Make sure file read is within the file size
		if((positionBytes  + numBytesRead) > totalFileSize)
		{
			numBytesRead = totalFileSize - positionBytes;
			r = numBytesRead / sizeOfType;
		}

		d.movePointerTo(positionBytes);

		d.readBytes(buffer, numBytesRead);

		for(i = 0; i < r && ((t + i) <= r); i++)
		{
			if(((unsigned long *) buffer)[i] != (position + i + 1))
			{
				int a = 1;
			}
		}
	}

	d.close();

	if(buffer)
		delete []buffer;

	return true;
}


void testRMI(void)
{
	initializeTests();

//	createDataSourceCacheTestFile(L"C:\\CacheTestFile.pod");
	testDataSourceCacheRandom();

    BeThreadUtilities::BeSleep(1000 * 2);

	testTCP();

//	testExt();

//	testCallClient();



//	testManager();

//	testObjectManager();

//	testProtocolManager();

//	testDataBuffer();

//	testParams();

//	testName();

//	testRemote();

//	testClient();
//	testServer();


}





