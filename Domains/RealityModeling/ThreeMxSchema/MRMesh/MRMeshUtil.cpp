/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/MRMesh/MRMeshUtil.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "..\ThreeMxSchemaInternal.h"

#include    <windows.h>


USING_NAMESPACE_BENTLEY_DGNPLATFORM

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName MRMeshUtil::ConstructNodeName (std::string const& childName, BeFileNameCP parentName)
    {
    BeFileName nodeFileName(childName.c_str());

    if (nodeFileName.IsAbsolutePath() ||  nodeFileName.IsUrl())
        return nodeFileName;

    BeFileName fullNodeFileName = (NULL == parentName) ? BeFileName() : *parentName;

    fullNodeFileName.AppendToPath (nodeFileName.c_str());

    return fullNodeFileName;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void    MRMeshUtil::GetMemoryStatistics (size_t& memoryLoad, size_t& total, size_t& available)

    {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof (statex);
    ::GlobalMemoryStatusEx (&statex);

    memoryLoad = (size_t) statex.dwMemoryLoad;
    total      = (size_t) statex.ullTotalPhys;
    available  = (size_t) statex.ullAvailPhys;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double MRMeshUtil::CalculateResolutionRatio ()
    {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof (statex);
    ::GlobalMemoryStatusEx (&statex);

    static  uint32_t      s_memoryThresholdPercent = 90;       // Start limiting usage at 70% memory usage.

    if (statex.dwMemoryLoad < s_memoryThresholdPercent)
        return 1.0;

    if (statex.dwMemoryLoad > 99)
        return 100.0;

    return (100.0 - (double) s_memoryThresholdPercent) / (100.0 - (double) statex.dwMemoryLoad);
    }

