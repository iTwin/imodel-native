/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ptengine/StreamScheduler.h>

namespace pointsengine
{

class StreamSchedulerSequential : public StreamScheduler
{

protected:

	bool					calculateStreamHostPeriods		(StreamManagerParameters &params);

	bool					processStreamHost				(StreamManagerParameters &params, StreamHost &streamHost);

	bool streamHostReads(StreamHost &streamHost);

	bool					processStreamHostRead			(StreamHost &streamHost);

public:

	bool					processStreamHosts				(StreamManagerParameters &params);

	bool					processStreamHostsRead			(void);
};


} // End pointsengine namespace
