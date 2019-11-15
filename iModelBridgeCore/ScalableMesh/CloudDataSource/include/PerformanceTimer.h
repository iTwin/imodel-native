/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "stdafx.h"
#include "DataSourceDefs.h"
#ifdef _WIN32
    #include <Windows.h>
#endif

class PerformanceTimer
{
protected:

        LARGE_INTEGER       startTime;
        LARGE_INTEGER       endTime;

static    LARGE_INTEGER     Frequency;

public:

                            PerformanceTimer            (void);

 static void                initialize                  (void);

        void                start                       (void);
        void                stop                        (void);
        long long           ellapsedTimeMicroseconds    (void);
        double              ellapsedTimeSeconds         (void);
};
