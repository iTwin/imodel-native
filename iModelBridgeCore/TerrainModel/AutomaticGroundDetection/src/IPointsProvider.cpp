/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/src/IPointsProvider.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "AutomaticGroundDetectionPch.h"

#include <TerrainModel/AutomaticGroundDetection/GroundDetectionMacros.h>
#include <TerrainModel/AutomaticGroundDetection/IPointsProvider.h>

/*
#include "PCPointsProvider.h"
#include "TMPointsProvider.h"
#include "MRMeshPointsProvider.h"
*/

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_TERRAINMODEL
//USING_NAMESPACE_BENTLEY_LOGGING


BEGIN_GROUND_DETECTION_NAMESPACE

const uint32_t IPointsProvider::DATA_QUERY_BUFFER_SIZE = 400000;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IPointsProviderPtr IPointsProviderCreator::CreatePointProvider(DRange3d const& boundingBoxInUors)
    {
    return _CreatePointProvider(boundingBoxInUors);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IPointsProviderPtr IPointsProviderCreator::CreatePointProvider()
    {
    return _CreatePointProvider();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void IPointsProviderCreator::GetAvailableRange(DRange3d& availableRange)
    {
    _GetAvailableRange(availableRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IPointsProviderPtr IPointsProvider::CreateFrom(IPointsProviderCreatorPtr& pointsProviderCreator, DRange3d* pRange)
    {
    assert(pointsProviderCreator.IsValid());

    if (pRange == nullptr)
        {
        return pointsProviderCreator->CreatePointProvider();
        }
    
    return pointsProviderCreator->CreatePointProvider(*pRange); 

    /*
    if (isInputPointCloud)
        return PCPointsProvider::CreateFrom(eh, boundingBoxInUors);

    IMRMeshAttachment* pIMRMeshQuery = dynamic_cast<IMRMeshAttachment*>(&eh.GetHandler());
    bool isMRMesh(pIMRMeshQuery != NULL);
    if (isMRMesh)
        return MRMeshPointsProvider::CreateFrom(eh, boundingBoxInUors);

    BeAssert(!"Not implemented yet - PointProvider for this element type");
    //Try with a generic PolyfaceFromElement
    return TMPointsProvider::CreateFrom(eh, boundingBoxInUors);
    */
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IPointsProvider::IPointsProvider(DRange3d const& boundingBoxInUors)
:m_isMultiThread(false),
m_useMeterUnit(true),
m_boundingBoxInUors(boundingBoxInUors),
m_prefetchPoints(false),
m_density(1.0),
m_maxPointsToPreFetch(-1), //-1 means we don't want to cap the number of points to prefetch
m_exportResolution(0.0) //not set, means use Exported Resolution from MrMesh dialog
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IPointsProvider::IPointsProvider(IPointsProvider const& object)
:m_isMultiThread(object.m_isMultiThread),
m_useMeterUnit(object.m_useMeterUnit),
m_boundingBoxInUors(object.m_boundingBoxInUors),
m_prefetchPoints(false),
m_density(object.m_density),
m_maxPointsToPreFetch(object.m_maxPointsToPreFetch), //-1 means we don't want to cap the number of points to prefetch
m_exportResolution(object.m_exportResolution) //not set, means use Exported Resolution from MrMesh dialog
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void IPointsProvider::_SetUseMultiThread(bool value) { m_isMultiThread = value; }
bool IPointsProvider::_GetUseMultiThread() const { return m_isMultiThread; }
void IPointsProvider::_SetUseMeterUnit(bool value) { m_useMeterUnit = value; }
bool IPointsProvider::_GetUseMeterUnit() const { return m_useMeterUnit; }
DRange3d IPointsProvider::_GetBoundingBox() const { return m_boundingBoxInUors; }
void IPointsProvider::_SetBoundingBox(DRange3d const& boundingBoxInUors)  { m_boundingBoxInUors = boundingBoxInUors; }
void IPointsProvider::_SetMaxPointsToPrefetch(int value) { m_maxPointsToPreFetch = value; }
int  IPointsProvider::_GetMaxPointsToPrefetch() const    { return m_maxPointsToPreFetch; }
void IPointsProvider::_SetExportResolution(double exportResolution) { m_exportResolution = exportResolution;}
double IPointsProvider::_GetExportResolution() const                {return m_exportResolution; }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t IPointsProvider::_GetMemorySize() const
    {
    return ComputeMemorySize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t IPointsProvider::_ComputeMemorySize() const
    {
    size_t  memorySize(sizeof(*this));
    memorySize += m_prefetchedPoints.size() * sizeof(DPoint3d);
    return memorySize;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Transform IPointsProvider::GetUorToMeterTransformIntern(/*DgnModelRefP model,*/ bool useGlobalOrigin) 
    {
    /*
    double uorPerMetersScale = mdlModelRef_getUorPerMeter(model);
    DPoint3d globalOrigin;
    mdlModelRef_getGlobalOrigin(model, &globalOrigin);
    DPoint3d translation = { 0.0, 0.0, 0.0 };
    if (useGlobalOrigin)
        {
        translation.x = globalOrigin.x;
        translation.y = globalOrigin.y;
        translation.z = globalOrigin.z;
        }
    //Create a simple scale transform
    Transform metersToUors;
    metersToUors.InitFromRowValues(uorPerMetersScale, 0.0,               0.0,               translation.x,
                                   0.0,               uorPerMetersScale, 0.0,               translation.y,
                                   0.0,               0.0,               uorPerMetersScale, translation.z);
                                   */
    Transform uorToMeters(Transform::FromIdentity());
    //uorToMeters.InverseOf(metersToUors);
    return uorToMeters;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Transform IPointsProvider::_GetUorToMeterTransform(bool useGlobalOrigin) const
    {
    Transform transform(Transform::FromIdentity());

    return transform;

    //return GetUorToMeterTransform(mdlModelRef_getActive(), useGlobalOrigin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Daniel.McKenzie                 04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t IPointsProvider::_GetPrefetchedPoints(DPoint3dP points, size_t maxSize)
    {
    if (m_prefetchedPoints.empty())
        _PrefetchPoints();
    
    if (!m_prefetchPoints) //couldn't fetch the points means we got to many points return max points
        return m_maxPointsToPreFetch;

    size_t sizeVector = m_prefetchedPoints.size();

    if (sizeVector >= maxSize) //We are gonna bust the size of the array if the vector size is bigger than the max size
        return sizeVector;

    if (sizeVector != 0)
        memcpy(&points[0], &m_prefetchedPoints[0], sizeof(DPoint3d)* sizeVector);

    return sizeVector;
    }

END_GROUND_DETECTION_NAMESPACE
