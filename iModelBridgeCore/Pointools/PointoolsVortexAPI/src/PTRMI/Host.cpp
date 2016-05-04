#include "PointoolsVortexAPIInternal.h"

#include <PTRMI/Manager.h>
#include <PTRMI/Host.h>


namespace PTRMI
{

Host::Host(void)
{
															// Initially no Pipe used
	setPipe(NULL);
															// Default to standard message version
	setMessageVersionUsed(Message::getDefaultMessageVersion());	
}


Host::Host(const PTRMI::Name &name)
{
	hostName = name;
															// Initially no Pipe used
	setPipe(NULL);
															// Default to standard message version
	setMessageVersionUsed(Message::getDefaultMessageVersion());
}


Host::~Host(void)
{
															// Delete Host's Pipe. Must be done explicitly using Pipe because
															// indexing to host has been deleted
#ifdef NEEDS_WORK_VORTEX_DGNDB_SERVER
	getManager().getPipeProtocolManager().deletePipe(getPipe());
#endif

	setPipe(NULL);
}


void Host::setName(const PTRMI::Name &name)
{
	hostName = name;
}


const Name &Host::getName(void)
{
	return hostName;
}

void Host::setManagerInfo(const ManagerInfo &initManagerInfo)
{
	managerInfo = initManagerInfo;
}


const ManagerInfo &Host::getManagerInfo(void)
{
	return managerInfo;
}


void Host::setManagerGUID(const PTRMI::GUID &guid)
{
	hostName = guid;
}


const PTRMI::GUID Host::getManagerGUID(void)
{
	return hostName.getGUID();
}


void Host::setMessageVersionUsed(MessageVersion version)
{
	messageVersionUsed = version;
}


Host::MessageVersion Host::getMessageVersionUsed(void)
{
	return messageVersionUsed;
}


void Host::setPipe(Pipe *initPipe)
{
	pipe = initPipe;
}


Pipe *Host::getPipe(void)
{
	return pipe;
}

bool Host::updateName(const PTRMI::Name &name)
{
	return hostName.update(name);
}


void Host::setTimeLastUsed(void)
{
	timeLastUsed.start();
}


pt::SimpleTimer::Time Host::getTimeSinceLastUsed(void)
{
	return timeLastUsed.getEllapsedTimeSeconds();
}


} // End PTRMI namespace