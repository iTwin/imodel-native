/******************************************************************************

Pointools Vortex API Examples

timer.h

Simple timer class

(c) Copyright 2008-11 Pointools Ltd

*******************************************************************************/
#ifndef POINTOOLS_PEFORMANCE_TIMER_INCLUDE
#define POINTOOLS_PEFORMANCE_TIMER_INCLUDE

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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