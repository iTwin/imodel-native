/*--------------------------------------------------------------------------------------+
|
|     $Source: TerrainModelNET/Caching.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <TerrainModel\TerrainModel.h>
#include <TerrainModel\Core\BcDTM.h>
#include <TerrainModel\Core\dtm2dfns.h>
USING_NAMESPACE_BENTLEY_TERRAINMODEL
#include "Caching.h"

DTMFeatureCache::DTMFeatureCache (BcDTMP dtm, int maxNumInCache, DTMBrowseFeatureCacheCallback callBackFunctP, void* userP) :
    m_maxNumInCache (maxNumInCache),
    m_callBackFunctP (callBackFunctP),
    m_userP (userP),
    m_dtm (dtm),
    m_numPoints (0)
    {
    /*
    **           Fixed Memory Blocks
    */
    if (m_maxNumInCache <  1000) m_maxNumInCache = 1000;
    else if (m_maxNumInCache >  100000) m_maxNumInCache = 100000;
    }

DTMFeatureCache::~DTMFeatureCache ()
    {
    FireCallback ();
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   djh.1aug2008   -  Created.                                          |
|                                                                       |
+----------------------------------------------------------------------*/
void DTMFeatureCache::EmptyCache ()
    {
    m_features.clear ();
    m_numPoints = 0;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   djh.1aug2008   -  Created.                                          |
|                                                                       |
+----------------------------------------------------------------------*/
int DTMFeatureCache::AddFeature (DTMFeatureType featureType, DTMUserTag featureTag, DTMFeatureId featureId, DPoint3dP featurePtsP, size_t nPoint)
    {
    int status = DTM_SUCCESS;

    if (m_numPoints + nPoint > m_maxNumInCache)
        {
        status = FireCallback ();
        EmptyCache ();
        if (status != DTM_SUCCESS)
            return status;
        }

    m_features.push_back (DTMCachedFeature (featureType, featureTag, featureId));
    DTMCachedFeature* featP = &m_features.back();

    featP->points.resize (nPoint);
    memcpy (featP->points.data (), featurePtsP, nPoint * sizeof DPoint3d);

    m_numPoints += nPoint;
    if (m_numPoints > m_maxNumInCache)
        {
        status = FireCallback ();
        EmptyCache ();
        }
    return status;
    }

int AddFeatureToCache (DTMFeatureType featureType, DTMUserTag featureTag, DTMFeatureId featureId, DPoint3dP featurePtsP, size_t nPoint, void* userP)
    {
    DTMFeatureCache* cache = (DTMFeatureCache*)userP;
    return cache->AddFeature (featureType, featureTag, featureId, featurePtsP, nPoint);
    }

int DTMFeatureCache::FireCallback ()
    {
    int ret = DTM_SUCCESS;
    if (m_features.size () != 0)
        ret = m_callBackFunctP (m_features, m_userP);
    m_numPoints = 0;
    return ret;
    }

DTMStatusInt DTMFeatureCache::BrowseDrainageFeatures (DTMFeatureType featureType, double* minLowPointP, const DTMFenceParams& fence)
    {
    return m_dtm->BrowseDrainageFeatures (featureType, minLowPointP, fence, this, AddFeatureToCache);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   cbe.11jan2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt DTMFeatureCache::BrowseFeatures (DTMFeatureType featureType, long maxSpots)
    {
    return BrowseFeatures (featureType, DTMFenceParams (), maxSpots);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   cbe.11jan2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt DTMFeatureCache::BrowseFeatures (DTMFeatureType featureType, const DTMFenceParams& fence, long maxSpots)
    {
    return m_dtm->BrowseFeatures (featureType, fence, maxSpots, this, AddFeatureToCache);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   djh.1aug2008   -  Created.                                          |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt DTMFeatureCache::BrowseContours(double interval, double reg, double zMin, double zMax, bool loadRange, DTMContourSmoothing smoothOption, double smoothFactor,
                                          int smoothDensity, const DTMFenceParams& fence, double* contourValues, int numContourValues, long maxSlopeOption,
                                          double maxSlopeValue, bool highLowOption)
    {
    return m_dtm->BrowseContours (BENTLEY_NAMESPACE_NAME::TerrainModel::DTMContourParams (interval, reg, loadRange, zMin, zMax, contourValues, numContourValues, smoothOption, smoothFactor, smoothDensity, 0, highLowOption, maxSlopeOption, maxSlopeValue),
                                  fence, this, AddFeatureToCache);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   djh.1aug2008   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt DTMFeatureCache::AnalyzeElevation(DRange1d* tInterval, int nInterval, bool polygonized, const DTMFenceParams& fence)
    {
    return m_dtm->AnalyzeElevation (tInterval, nInterval, polygonized, fence, this, AddFeatureToCache);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   djh.1aug2008   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt DTMFeatureCache::AnalyzeSlope(DRange1d* tInterval, int nInterval, bool polygonized, const DTMFenceParams& fence)
    {
    return m_dtm->AnalyzeSlope (tInterval, nInterval, polygonized, fence, this, AddFeatureToCache);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   djh.1aug2008   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt DTMFeatureCache::AnalyzeAspect(DRange1d* tInterval, int nInterval, bool polygonized, const DTMFenceParams& fence)
    {
    return m_dtm->AnalyzeAspect (tInterval, nInterval, polygonized, fence, this, AddFeatureToCache);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.18mar2010   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt DTMFeatureCache::BrowseTinPointsVisibility (double eyeX, double eyeY, double eyeZ)
    {
    return m_dtm->BrowseTinPointsVisibility (eyeX, eyeY, eyeZ, this, AddFeatureToCache);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.18mar2010   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt DTMFeatureCache::BrowseTinLinesVisibility (double eyeX, double eyeY, double eyeZ)
    {
    return m_dtm->BrowseTinLinesVisibility (eyeX, eyeY, eyeZ, this, AddFeatureToCache);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.18mar2010   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt DTMFeatureCache::BrowseRadialViewSheds (double eyeX, double eyeY, double eyeZ, long viewShedOption, long numberRadials, double radialIncrement)
    {
    return m_dtm->BrowseRadialViewSheds (eyeX, eyeY, eyeZ, viewShedOption, numberRadials, radialIncrement, this, AddFeatureToCache);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   rsc.22mar2010   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt DTMFeatureCache::BrowseRegionViewSheds (double eyeX, double eyeY, double eyeZ)
    {
    return m_dtm->BrowseRegionViewSheds (eyeX, eyeY, eyeZ, this, AddFeatureToCache);
    }

