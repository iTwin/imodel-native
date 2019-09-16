#include "stdafx.h"
#include "PerformanceTimer.h"

LARGE_INTEGER PerformanceTimer::Frequency;



PerformanceTimer::PerformanceTimer(void)
{

    QueryPerformanceCounter(&startTime);

}

void PerformanceTimer::initialize(void)
{
    QueryPerformanceFrequency(&Frequency);
}

void PerformanceTimer::start(void)
{
    QueryPerformanceCounter(&startTime);
}

void PerformanceTimer::stop(void)
{
    QueryPerformanceCounter(&endTime);
}

long long PerformanceTimer::ellapsedTimeMicroseconds(void)
{
    LARGE_INTEGER    elapsedMicroseconds;
    
    elapsedMicroseconds.QuadPart = endTime.QuadPart - startTime.QuadPart;

    elapsedMicroseconds.QuadPart *= 1000000;
    elapsedMicroseconds.QuadPart /= Frequency.QuadPart;

    return elapsedMicroseconds.QuadPart;
}

double PerformanceTimer::ellapsedTimeSeconds(void)
{
    long long t = ellapsedTimeMicroseconds();

    return static_cast<double>(t) / 1000000.0;
}
