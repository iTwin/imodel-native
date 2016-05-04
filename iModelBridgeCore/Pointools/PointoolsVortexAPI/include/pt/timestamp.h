#ifndef POINTOOLS_TIMESTAMP_INCLUDE
#define POINTOOLS_TIMESTAMP_INCLUDE

#include <pt/classes.h>

namespace pt
{
struct TimeStamp
{
    void tick();
    void zero() { m_timeMillis = 0; }

	CCLASSES_API static double delta_s(const TimeStamp &t0, const TimeStamp &t1);
	CCLASSES_API static double delta_ms(const TimeStamp &t0, const TimeStamp &t1);

    uint64_t m_timeMillis;
};


class SimpleTimer
{

public:

	typedef double	Time;

protected:

	pt::TimeStamp	timer;
	Time			timeTotal;
	unsigned int	numIntervals;
	bool			started;

public:

	SimpleTimer(void)
	{
		clear();
	}

	void clear(void)
	{
		timer.zero();
		timeTotal = 0;
		numIntervals = 0;

		setStarted(false);
	}

	void setStarted(bool initStarted)
	{
		started = initStarted;
	}

	bool getStarted(void)
	{
		return started;
	}

	Time getEllapsedTimeSeconds(void)
	{
		pt::TimeStamp	t;

		t.tick();

		return timer.delta_s(timer, t);		
	}

	void start(void)
	{
		timer.tick();

		setStarted(true);
	}

	void stop(void)
	{
		timeTotal += getEllapsedTimeSeconds();
		++numIntervals;

		setStarted(false);
	}

	Time getTimeSeconds(void)
	{
		return timeTotal;
	}

	Time getAveragePerInterval(void)
	{
		if(getNumIntervals() > 0)
		{
			return getTimeSeconds() / static_cast<Time>(getNumIntervals());
		}

		return 0;
	}

	unsigned int getNumIntervals(void)
	{
		return numIntervals;
	}
};

}
#endif