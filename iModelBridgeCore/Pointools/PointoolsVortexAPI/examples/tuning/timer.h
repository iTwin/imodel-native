//----------------------------------------------------------------------------
//
// timer.h
//
// Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.
//
//----------------------------------------------------------------------------
#ifndef POINTOOLS_PEFORMANCE_TIMER_INCLUDE
#define POINTOOLS_PEFORMANCE_TIMER_INCLUDE

namespace pt
{
	extern LARGE_INTEGER QPCfreq; 

	class PerformanceTimer
	{
	public:	
		void start() { QueryPerformanceCounter(&_btime); }
		void end() { QueryPerformanceCounter(&_etime); }
		double millisecs() { return 1000 * (double)(_etime.QuadPart - _btime.QuadPart ) / QPCfreq.QuadPart; }

		static void initialize();
	
	private:
		LARGE_INTEGER _btime;
		LARGE_INTEGER _etime;

	};
	class SimplePerformanceTimer : PerformanceTimer
	{
	public:
		SimplePerformanceTimer(double &ms) : _msTime(ms)
		{
			start();
		};
		~SimplePerformanceTimer()
		{
			end();
			_msTime = millisecs();
		}
	private:
		double &_msTime;
	};

}
#endif