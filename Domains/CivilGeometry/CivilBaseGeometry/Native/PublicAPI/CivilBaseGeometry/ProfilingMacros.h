/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

/*---------------------------------------------------------------------------------**//**
* Profiling Macros
+---------------+---------------+---------------+---------------+---------------+------*/
#if defined __HMR_PROFILE

#include <iostream>
#include <sstream>
#include <iomanip>
#include <windows.h>
#include <winnt.h>

#if defined max
#undef max
#endif
#if defined min
#undef min
#endif


#define PROFILE_Declare(Name)                       static LARGE_INTEGER Name##StarTime={0,0}; \
                                                    static LARGE_INTEGER Name##Frequency={0,0}; \
                                                    static double Name##EllapsedTime=0.0; \
                                                    static long Name##Call=0

#define PROFILE_Start(Name)                         QueryPerformanceFrequency(&Name##Frequency); \
                                                    QueryPerformanceCounter(&Name##StarTime); \
                                                    Name##Call ++

#define PROFILE_DeclareAndStart(Name)               PROFILE_Declare(Name); PROFILE_Start(Name)

#define PROFILE_End(Name)                           LARGE_INTEGER Name##StopTime={0,0}; \
                                                    QueryPerformanceCounter(&Name##StopTime); \
                                                    Name##EllapsedTime += (((double)Name##StopTime.QuadPart - (double)Name##StarTime.QuadPart) * (1000.0 / (double)Name##Frequency.QuadPart))/1000.0

#define PROFILE_Clear(Name)                         memset(&Name##StarTime, 0, sizeof(LARGE_INTEGER));\
                                                    memset(&Name##EllapsedTime, 0, sizeof(LARGE_INTEGER));\
                                                    Name##Call=0

#define PROFILE_Printf(Name)                        if (Name##Call > 0)\
                                                        printf("Total time: %9.4lf - Calls: %8ld [%s]\n", Name##EllapsedTime, Name##Call, #Name)

#define PROFILE_PrintfAndClear(Name)                PROFILE_End(Name); PROFILE_Printf(Name); PROFILE_Clear(Name)

#define PROFILE_ToStream(StreamName, Name)          if (Name##Call > 0) \
                                                        { \
                                                        if (Name##Call > 0) \
                                                            { \
                                                                StreamName.precision(5); \
                                                                StreamName.flags(std::ios::fixed); \
                                                                StreamName << "[" << #Name << "] -->" \
                                                                << " [Total time: " << std::setw(9) << std::right << Name##EllapsedTime  << "]" \
                                                                << " [Avg: "       << std::setw(9) << std::right << (Name##EllapsedTime/Name##Call) << "]" \
                                                                << " [Calls: "     << std::setw(5) << std::right << Name##Call << "]"; \
                                                            } \
                                                        }

#define PROFILE_ToStreamAndClear(StreamName, Name)  PROFILE_End(Name); PROFILE_ToStream(StreamName, Name); PROFILE_Clear(Name)

#define PROFILE_Code(SomeCode)                      SomeCode

#define PROFILE_DefineGuardStruct(Name)             struct Name##ProfileGuard \
                                                    { \
                                                    private: \
                                                        Name##ProfileGuard(Name##ProfileGuard const&) = delete; \
                                                        Name##ProfileGuard& operator=(Name##ProfileGuard const&) = delete; \
                                                    public: \
                                                         Name##ProfileGuard() {} \
                                                        ~Name##ProfileGuard() { PROFILE_End(Name); } \
                                                    };

#define PROFILE_DefineGuardStructWithPrintf(Name)   struct Name##ProfileGuard \
                                                    { \
                                                    private: \
                                                        Name##ProfileGuard(Name##ProfileGuard const&) = delete; \
                                                        Name##ProfileGuard& operator=(Name##ProfileGuard const&) = delete; \
                                                    public: \
                                                         Name##ProfileGuard() {} \
                                                        ~Name##ProfileGuard() { PROFILE_PrintfAndClear(Name); } \
                                                    };


#define PROFILE_DeclareGuard(Name)                  PROFILE_Declare(Name); \
                                                    PROFILE_DefineGuardStruct(Name)

#define PROFILE_DeclareGuardWithPrintf(Name)        PROFILE_Declare(Name); \
                                                    PROFILE_DefineGuardStructWithPrintf(Name)

#define PROFILE_GuardStart(Name)                    PROFILE_Start(Name); Name##ProfileGuard Name##guard

# else

#define PROFILE_Declare(Name)
#define PROFILE_Start(Name)
#define PROFILE_DeclareAndStart(Name)
#define PROFILE_End(Name)
#define PROFILE_Clear(Name)
#define PROFILE_Printf(Name)
#define PROFILE_PrintfAndClear(Name)
#define PROFILE_ToStream(StreamName, Name)
#define PROFILE_ToStreamAndClear(StreamName, Name)
#define PROFILE_Code(SomeCode)
#define PROFILE_DefineGuardStruct(SomeCode)
#define PROFILE_DefineGuardStructWithPrintf(SomeCode)
#define PROFILE_DeclareGuard(Name)
#define PROFILE_DeclareGuardWithPrintf(Name)
#define PROFILE_GuardStart(Name)

#endif
