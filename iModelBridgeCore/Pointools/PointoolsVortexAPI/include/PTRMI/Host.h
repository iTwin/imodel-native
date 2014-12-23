
#pragma once

#include <PTRMI/ManagerInfo.h>
#include <PTRMI/HostAddress.h>
#include <PTRMI/Name.h>
#include <PTRMI/Status.h>
#include <PTRMI/Message.h>

#include <set>

#include <pt/TimeStamp.h>


namespace PTRMI
{

class Pipe;


class Host
{
public:

typedef	Message::MessageVersion		MessageVersion;


protected:

	typedef std::set<Pipe *>	PipeSet;

protected:

	Name						hostName;

	ManagerInfo					managerInfo;

	MessageVersion				messageVersionUsed;

	Pipe					*	pipe;

	pt::SimpleTimer				timeLastUsed;

public:
								Host					(void);
								Host					(const PTRMI::Name &name);
							   ~Host					(void);

	void						setName					(const PTRMI::Name &name);
	const Name				&	getName					(void);

	bool						updateName				(const PTRMI::Name &name);

	void						setManagerInfo			(const ManagerInfo &initManagerInfo);	
	const ManagerInfo		&	getManagerInfo			(void);

	void						setManagerGUID			(const PTRMI::GUID &guid);
	const PTRMI::GUID			getManagerGUID			(void);

	void						setMessageVersionUsed	(MessageVersion version);
	MessageVersion				getMessageVersionUsed	(void);

	void						setPipe					(Pipe *initPipe);
	Pipe					*	getPipe					(void);

	void						setTimeLastUsed			(void);
	pt::SimpleTimer::Time		getTimeSinceLastUsed	(void);
	
};

} // End PTRMI namespace