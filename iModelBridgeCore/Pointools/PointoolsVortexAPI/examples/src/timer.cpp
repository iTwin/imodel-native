/******************************************************************************

Pointools Vortex API Examples

timer.cpp

Simple timer class

(c) Copyright 2008-11 Pointools Ltd

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