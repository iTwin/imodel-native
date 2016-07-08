#pragma once
#include "DataSourceDefs.h"
#include <Windows.h>

class PerformanceTimer
{
protected:

		LARGE_INTEGER	startTime;
		LARGE_INTEGER	endTime;

static	LARGE_INTEGER	Frequency;

public:

CLOUD_EXPORT						PerformanceTimer			(void);

CLOUD_EXPORT static	void			initialize					(void);

CLOUD_EXPORT		void			start						(void);
CLOUD_EXPORT		void			stop						(void);
CLOUD_EXPORT		long long		ellapsedTimeMicroseconds	(void);
CLOUD_EXPORT		double			ellapsedTimeSeconds			(void);
};