/*--------------------------------------------------------------------------------------+
|
|     $Source: Bentley/nonport/BeSystemInfo.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    #include <windows.h>
#endif

#include "../BentleyInternal.h"

#if defined (__unix__)
    #include <sys/time.h>
    #include <sys/resource.h>
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <errno.h>
    #include <fcntl.h>
    #include <iostream>
    #include <fstream>
#endif

#if defined (__APPLE__)
    #include <sys/sysctl.h>
#elif defined (ANDROID) || defined (__linux)
    #undef __unused
    #include <linux/sysctl.h>
    #include <sys/sysinfo.h>
#endif

#include <Bentley/BeSystemInfo.h>

#if defined (ANDROID) && defined (NOTNOW)
static void getMemStats()
    {
        struct rusage rUsageTemp;

        getrusage (RUSAGE_SELF, &rUsageTemp);

        int    h1 = open ("/proc/self", O_RDONLY);
        printf ("after open of self h1 = %d, errno = %d\n", h1, errno);

        char    f2[256];
        sprintf (f2, "/proc/%d", getpid());
        int h2 = open (f2, O_RDONLY);
        printf ("after open of %s h1 = %d, errno = %d\n", f2, h1, errno);
    }
#endif


#if defined (ANDROID) || defined (__linux)
//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
uint64_t BeSystemInfo::GetAmountOfPhysicalMemory ()
    {
    struct sysinfo myInfo;
    sysinfo(&myInfo);
    return myInfo.totalram;
    }
#endif

#if defined (__APPLE__)
//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
uint64_t BeSystemInfo::GetAmountOfPhysicalMemory ()
    {
    int     name [2];
    size_t  sizeOutput;
    uint64_t retval = 0;

    name[0] = CTL_HW;
    name[1] = HW_PHYSMEM;
    sizeOutput = sizeof (retval);
    sysctl(name, 2, (Byte*)&retval, &sizeOutput, NULL, 0);
    BeAssert (4 == sizeOutput);

    return retval;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
Utf8String BeSystemInfo::GetMachineName()
    {
    int     name [2];
    size_t  sizeOutput;
    Byte output[64];

    name[0] = CTL_HW;
    name[1] = HW_MACHINE;
    sizeOutput = sizeof (output) - 1;
    sysctl(name, 2, output, &sizeOutput, NULL, 0);
    output[sizeOutput] = 0;
    return Utf8String((CharCP)output);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
Utf8String BeSystemInfo::GetModelName()
    {
    int     name [2];
    size_t  sizeOutput;
    Byte output[64];

    name[0] = CTL_HW;
    name[1] = HW_MODEL;
    sizeOutput = sizeof (output) - 1;
    sysctl(name, 2, output, &sizeOutput, NULL, 0);
    output[sizeOutput] = 0;
    return Utf8String((CharCP)output);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2013
//---------------------------------------------------------------------------------------
uint32_t BeSystemInfo::GetNumberOfCpus()
    {
    int     name [2];
    size_t  sizeOutput;
    Byte output[64];

    name[0] = CTL_HW;
    name[1] = HW_NCPU;
    sizeOutput = sizeof (output);
    sysctl(name, 2, output, &sizeOutput, NULL, 0);

    uint32_t retval = 0;
    for (unsigned i = 0; i < sizeOutput; ++i)
        retval += output[i] << (8 * i);

    return retval;
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2014
//---------------------------------------------------------------------------------------
#if defined (ANDROID) || defined (__linux)
uint32_t BeSystemInfo::GetNumberOfCpus()
    {
    return sysconf(_SC_NPROCESSORS_ONLN);
    }
#endif

#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Shaun.Sewall    06/2014
//---------------------------------------------------------------------------------------
uint32_t BeSystemInfo::GetNumberOfCpus()
    {
    SYSTEM_INFO systemInfo;
    GetNativeSystemInfo (&systemInfo);

    return systemInfo.dwNumberOfProcessors;
    }
#endif

#if defined (BENTLEY_WIN32)
//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    04/2014
//---------------------------------------------------------------------------------------
uint64_t BeSystemInfo::GetAmountOfPhysicalMemory ()
    {
    MEMORYSTATUSEX  memoryStatus;
    memoryStatus.dwLength = sizeof(memoryStatus);

    GlobalMemoryStatusEx(&memoryStatus);
    return memoryStatus.ullTotalPhys;
    }
#endif
