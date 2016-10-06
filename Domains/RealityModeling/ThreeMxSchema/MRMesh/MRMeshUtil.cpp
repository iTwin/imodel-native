/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/MRMesh/MRMeshUtil.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "..\ThreeMxSchemaInternal.h"

#include    <windows.h>
#include    <regex>


USING_NAMESPACE_BENTLEY_DGNPLATFORM





/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName MRMeshUtil::ConstructNodeName (std::string const& childName, BeFileNameCP parentName)
    {
    BeFileName nodeFileName(childName.c_str());

    if (nodeFileName.IsAbsolutePath() ||  nodeFileName.IsUrl())
        return nodeFileName;

    // This is an unfortunate patch to temporarily support ThreeMX on Stream-X HTTP servers in ConceptStation
	// on the stream DgnDb06 ONLY. Code on newer streams will require other changes.
	// DO NOT PORT!
    MRMeshFileName fullNodeFileName;

    // Patch to support Stream-X
    if (parentName != NULL)
		{
		Utf8String tempString;
        BeStringUtilities::WCharToUtf8(tempString, *parentName);
		fullNodeFileName = tempString;
		}

    fullNodeFileName.AppendToPath (childName.c_str());

    return BeFileName(fullNodeFileName.c_str());
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

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                              Nicholas.Woodfield     01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MRMeshUtil::ParseTileId(std::string const& name, uint32_t& tileX, uint32_t& tileY)
    {
    if(name.empty())
        return ERROR;
      
    std::regex pattern("([0-9]*)");
    std::sregex_token_iterator iter(name.begin(), name.end(), pattern);
    std::sregex_token_iterator end;

    uint32_t x = 0, y = 0;
    uint32_t count = 0;

    while (count < 2 && iter != end)
        {
        std::string substring = iter->str();
        if (!substring.empty())
            {
            int val = std::stoi(substring);

            if (count == 0)
                x = (uint32_t)val;
            else
                y = (uint32_t)val;

            count++;
            }

        iter++;
        }

    if (count == 2)
        {
        tileX = x;
        tileY = y;
        
        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
MRMeshContext::MRMeshContext (TransformCR transform, ViewContextR viewContext, double fixedResolution) 
    : m_transform (transform), m_useFixedResolution (false), m_fixedResolution (0.0), m_nodeCount (0), m_pointCount (0)    
    {
    m_qvCache = viewContext.GetDgnDb().Models().GetQvCache();

    m_loadSynchronous    = // FILTER_LOD_Off == viewContext.GetFilterLODFlag() ||          // If the LOD filter is off we assume that this is an application that is interested in full detail (and isn't going to wait for nodes to load.(DrawPurpose::CaptureGeometry == viewContext.GetDrawPurpose() || DrawPurpose::ModelFacet == viewContext.GetDrawPurpose())
                           DrawPurpose::ModelFacet == viewContext.GetDrawPurpose();          
    
    if (m_loadSynchronous)
        {
        if (DrawPurpose::Update != viewContext.GetDrawPurpose())       // For capture image the LOD filter is off - but we still want view dependent resolution.
            {
            m_useFixedResolution = true;
            m_fixedResolution    = .1;  // (0.0 == fixedResolution ? MRMeshElementHandler::ComputeDefaultExportResolution (element) : fixedResolution) / transform.ColumnXMagnitude();
            }
        }
    }

