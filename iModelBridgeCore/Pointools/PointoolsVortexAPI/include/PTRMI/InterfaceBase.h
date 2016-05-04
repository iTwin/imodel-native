#pragma once 

#include <PTRMI/GUID.h>
#include <PTRMI/Name.h>
#include <PT/TimeStamp.h>


namespace PTRMI
{

typedef void	RemotableObject;

class Stream;

class InterfaceBase
{

protected:
															// This interface's GUID
	PTRMI::GUID				interfaceGUID;
															// The Name of the remote interface
	Name					remoteInterface;
															// URL to remote host (Object name is full URL to object)
	Name					hostName;
															// Class name of object
	Name					objectClass;
															// Name of the object accessed
	Name					objectName;
															// Communications Stream
	Stream				*	stream;
															// Timestamp of last use of the interface
	pt::SimpleTimer			timeLastUsed;


public:

							InterfaceBase			(void);
	virtual				   ~InterfaceBase			(void);

	const PTRMI::GUID	&	getInterfaceGUID		(void) const;

	void					setRemoteInterface		(const Name &objectNameOrInterfacecGUID);
	const PTRMI::Name	&	getRemoteInterface		(void) const;

	PTRMI::GUID				getHostGUID				(void);

	void					setObjectClass			(const Name &initObjectClass);
	const Name			&	getObjectClass			(void);

	void					setObjectName			(const Name &initObjectName);
	const Name			&	getObjectName			(void);

	void					setHostName				(const Name &initHostName);
	const Name			&	getHostName				(void);

	bool					updateHostName			(const Name &newName);

	Status					createStream			(void);
	Status					deleteStream			(void);

	void					setStream				(Stream *initStream);
	Stream				*	getStream				(void) const;

	void					setTimeLastUsed			(void);	
	double					getIdleTimeSeconds		(void);

};





} // End PTRMI namespace
