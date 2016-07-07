#pragma once

#include <Windows.h>

class PerformanceTimer
{
protected:

		LARGE_INTEGER	startTime;
		LARGE_INTEGER	endTime;

static	LARGE_INTEGER	Frequency;

public:

						PerformanceTimer			(void);

static	void			initialize					(void);

		void			start						(void);
		void			stop						(void);
		long long		ellapsedTimeMicroseconds	(void);
		double			ellapsedTimeSeconds			(void);
};