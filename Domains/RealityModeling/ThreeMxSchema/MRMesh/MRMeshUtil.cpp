/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/MRMesh/MRMeshUtil.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ThreeMxSchemaInternal.h"

#if defined (BENTLEYCONFIG_OS_WINDOWS)
#include <windows.h>
#endif

#include <regex>

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName Util::ConstructNodeName(Utf8StringCR childName, BeFileNameCP parentName)
    {
    BeFileName nodeFileName(childName.c_str());

    if (nodeFileName.IsAbsolutePath() ||  nodeFileName.IsUrl())
        return nodeFileName;

    BeFileName fullNodeFileName = (NULL == parentName) ? BeFileName() : *parentName;

    fullNodeFileName.AppendToPath(nodeFileName.c_str());

    return fullNodeFileName;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
#if defined (BENTLEYCONFIG_OS_WINDOWS)
void Util::GetMemoryStatistics(size_t& memoryLoad, size_t& total, size_t& available)
    {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof (statex);
    ::GlobalMemoryStatusEx(&statex);

    memoryLoad = (size_t) statex.dwMemoryLoad;
    total      = (size_t) statex.ullTotalPhys;
    available  = (size_t) statex.ullAvailPhys;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double Util::CalculateResolutionRatio()
    {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof (statex);
    ::GlobalMemoryStatusEx(&statex);

    static  uint32_t      s_memoryThresholdPercent = 90;       // Start limiting usage at 70% memory usage.

    if (statex.dwMemoryLoad < s_memoryThresholdPercent)
        return 1.0;

    if (statex.dwMemoryLoad > 99)
        return 100.0;

    return (100.0 - (double) s_memoryThresholdPercent) / (100.0 - (double) statex.dwMemoryLoad);
    }
#endif

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                              Nicholas.Woodfield     01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Util::ParseTileId(Utf8StringCR name, uint32_t& tileX, uint32_t& tileY)
    {
    if (name.empty())
        return ERROR;
      
    std::string strName(name.c_str());
    std::regex pattern("([0-9]*)");
    std::sregex_token_iterator iter(strName.begin(), strName.end(), pattern);
    std::sregex_token_iterator end;

    uint32_t count = 0;
    while (count < 2 && iter != end)
        {
        std::string substring = iter->str();
        if (!substring.empty())
            {
            // *** NEEDS WORK: std::stoi does not seem to be implemented the version of GCC that we use for Android (yet)
            #ifndef ANDROID
            int val = std::stoi(substring);
            #else
            int val;
            sscanf(substring.c_str(), "%d", &val);
            #endif

            if (count == 0)
                tileX = (uint32_t)val;
            else
                tileY = (uint32_t)val;

            count++;
            }

        iter++;
        }

    return count != 2 ? ERROR : SUCCESS;
    }

