#include "stdafx.h"
#include "timer.h"

using namespace pt;

namespace pt
{
	LARGE_INTEGER QPCfreq;
}
void PerformanceTimer::initialize()
{
	QueryPerformanceFrequency(&QPCfreq);
}