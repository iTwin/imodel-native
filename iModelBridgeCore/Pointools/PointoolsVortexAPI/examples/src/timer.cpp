/******************************************************************************

Pointools Vortex API Examples

timer.cpp

Simple timer class

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#include "../include/timer.h"

using namespace pt;

namespace pt
{
	LARGE_INTEGER QPCfreq;
}
void PerformanceTimer::initialize()
{
	QueryPerformanceFrequency(&QPCfreq);
}