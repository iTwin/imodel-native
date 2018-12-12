#include "PointoolsVortexAPIInternal.h"

#include <pt/timestamp.h>

using namespace pt;

void TimeStamp::tick()
    {
    m_timeMillis = BeTimeUtilities::QueryMillisecondsCounter();
    }

double TimeStamp::delta_s(const TimeStamp &t0, const TimeStamp &t1)
    {
    return delta_ms(t0, t1) / 1000.0;
    }

double TimeStamp::delta_ms(const TimeStamp &t0, const TimeStamp &t1)
    {
    return (double) (t1.m_timeMillis - t0.m_timeMillis);
    }
