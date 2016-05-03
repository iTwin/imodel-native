#include "PointoolsVortexAPIInternal.h"

#include <pt/timestamp.h>
using namespace pt;

namespace {
LARGE_INTEGER QPCfreq;
}

bool TimeStamp::initialize()
{
	return QueryPerformanceFrequency(&QPCfreq) != 0;
}
double TimeStamp::delta_s(const TimeStamp &t0, const TimeStamp &t1)
{
	if (!QPCfreq.QuadPart) initialize();
	return (double)(t1._time.QuadPart - t0._time.QuadPart) / QPCfreq.QuadPart;
}
double TimeStamp::delta_ms(const TimeStamp &t0, const TimeStamp &t1)
{
	if (!QPCfreq.QuadPart)
	{
		initialize();

	}
	return 1000 * (double)(t1._time.QuadPart - t0._time.QuadPart ) / QPCfreq.QuadPart;
}
