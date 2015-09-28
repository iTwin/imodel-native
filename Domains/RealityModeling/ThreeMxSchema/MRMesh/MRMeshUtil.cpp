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

