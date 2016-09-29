/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/src/IPointsProvider.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DcPointCloudCorePch.h"
#include "PCPointsProvider.h"
#include "TMPointsProvider.h"
#include "MRMeshPointsProvider.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY
USING_NAMESPACE_POINTCLOUDAPP
USING_NAMESPACE_BENTLEY_TERRAINMODEL
USING_NAMESPACE_BENTLEY_LOGGING


BEGIN_DCPOINTCLOUDCORE_NAMESPACE

const UInt32 IPointsProvider::DATA_QUERY_BUFFER_SIZE = 400000;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool IPointsProvider::IsSupported(ElementHandleCR eh)
    {
    IPointCloudQuery* pQuery = dynamic_cast<IPointCloudQuery*>(&eh.GetHandler());
    bool isInputPointCloud(pQuery != NULL);

    if (isInputPointCloud)
        return true;

    IMRMeshAttachment* pIMRMeshQuery = dynamic_cast<IMRMeshAttachment*>(&eh.GetHandler());
    bool isInpuMRMesh(pIMRMeshQuery != NULL);

    if (isInpuMRMesh)
        {

        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static bool GetElementRangeInRoot
(
DRange3d& elementRangeInRoot,
ElementHandleCR elemH,
DgnModelRefP rootModel
)
    {
    MSElementCP elemPtr = elemH.GetElementCP();
    if (!elemPtr->ehdr.isGraphics || elemPtr->hdr.dhdr.props.b.invisible)
        return false;

    // Get the transformation from the model to the root
    Transform toRoot;
    toRoot.initIdentity();
    DgnModelRefP model = elemH.GetModelRef();
    if (model != rootModel)
        mdlRefFile_getTransformToParent(&toRoot, model->AsDgnAttachmentP(), rootModel->AsDgnAttachmentP());

    // Calculate the range instead of reading it from the element header
    DisplayHandlerP dHandler = elemH.GetDisplayHandler();
    if (NULL == dHandler)
        return false;
    if (SUCCESS != dHandler->CalcElementRange(elemH, elementRangeInRoot, &toRoot))
        return false;
    return true;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d IPointsProvider::GetBoundingBox(ElementHandleCR eh, bool useClip)
    {
    IPointCloudQuery* pQuery = dynamic_cast<IPointCloudQuery*>(&eh.GetHandler());
    bool isInputPointCloud(pQuery != NULL);

    DRange3d    boundingBoxInUors;
    DisplayHandlerP dHandler = eh.GetDisplayHandler();
    if (!isInputPointCloud && dHandler)
        {
        // Calculate exact range
        if (!GetElementRangeInRoot(boundingBoxInUors,eh,mdlModelRef_getActive()))
            dHandler->CalcElementRange(eh, boundingBoxInUors, NULL);
        }
    else
        {
        //fall back to use range store in element or special case for point cloud
        DPoint3d box[8];
        PointCloudElementFacility::GetRangeBox(box, eh, useClip);
        boundingBoxInUors = (DRange3d::From(box, 8));
        }
    return boundingBoxInUors;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IPointsProviderPtr IPointsProvider::CreateFrom(ElementHandleCR eh, DRange3dCP pRange)
    {
    DRange3d boundingBoxInUors;
    IPointCloudQuery* pQuery = dynamic_cast<IPointCloudQuery*>(&eh.GetHandler());
    bool isInputPointCloud(pQuery != NULL);


    if (NULL == pRange)
        {
        boundingBoxInUors = GetBoundingBox(eh);
        }
    else
        {
        boundingBoxInUors = *pRange;
        }
    if (isInputPointCloud)
        return PCPointsProvider::CreateFrom(eh, boundingBoxInUors);

    IMRMeshAttachment* pIMRMeshQuery = dynamic_cast<IMRMeshAttachment*>(&eh.GetHandler());
    bool isMRMesh(pIMRMeshQuery != NULL);
    if (isMRMesh)
        return MRMeshPointsProvider::CreateFrom(eh, boundingBoxInUors);

    BeAssert(!"Not implemented yet - PointProvider for this element type");
    //Try with a generic PolyfaceFromElement
    return TMPointsProvider::CreateFrom(eh, boundingBoxInUors);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IPointsProvider::IPointsProvider(ElementHandleCR eh, DRange3dCR boundingBoxInUors)
:m_useViewFilters(false),
m_isMultiThread(false),
m_queryView(-1),
m_queryMode(PointCloud::IPointCloudDataQuery::QUERY_MODE_FILE),
m_useMeterUnit(true),
m_eh(eh),
m_boundingBoxInUors(boundingBoxInUors),
m_prefetchPoints(false),
m_queryDensity(PointCloud::IPointCloudDataQuery::QUERY_DENSITY_FULL),
m_density(1.0),
m_maxPointsToPreFetch(-1), //-1 means we don't want to cap the number of points to prefetch
m_exportResolution(0.0) //not set, means use Exported Resolution from MrMesh dialog
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IPointsProvider::IPointsProvider(IPointsProviderCR object)
:m_useViewFilters(object.m_useViewFilters),
m_isMultiThread(object.m_isMultiThread),
m_queryView(object.m_queryView),
m_queryMode(object.m_queryMode),
m_useMeterUnit(object.m_useMeterUnit),
m_eh(object.m_eh),
m_boundingBoxInUors(object.m_boundingBoxInUors),
m_prefetchPoints(false),
m_queryDensity(object.m_queryDensity),
m_density(object.m_density),
m_maxPointsToPreFetch(object.m_maxPointsToPreFetch), //-1 means we don't want to cap the number of points to prefetch
m_exportResolution(object.m_exportResolution) //not set, means use Exported Resolution from MrMesh dialog
    {
    }




/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void IPointsProvider::_SetUseViewFilters(bool value) { m_useViewFilters = value; }
bool IPointsProvider::_GetUseViewFilters() const { return m_useViewFilters; }
void IPointsProvider::_SetUseMultiThread(bool value) { m_isMultiThread = value; }
bool IPointsProvider::_GetUseMultiThread() const { return m_isMultiThread; }
void IPointsProvider::_GetQueryMode(IPointCloudDataQuery::QUERY_MODE& queryMode, int& viewNumber) const { queryMode = m_queryMode; viewNumber = m_queryView; }
void IPointsProvider::_SetQueryMode(IPointCloudDataQuery::QUERY_MODE queryMode, int viewNumber) { m_queryMode = queryMode; m_queryView = viewNumber; }
void IPointsProvider::_GetQueryDensity(IPointCloudDataQuery::QUERY_DENSITY& queryDensity, float& density) const { queryDensity = m_queryDensity; density = m_density; }
void IPointsProvider::_SetQueryDensity(IPointCloudDataQuery::QUERY_DENSITY  queryDensity, float density) { m_queryDensity = queryDensity; m_density = density; }
void IPointsProvider::_SetUseMeterUnit(bool value) { m_useMeterUnit = value; }
bool IPointsProvider::_GetUseMeterUnit() const { return m_useMeterUnit; }
DRange3d IPointsProvider::_GetBoundingBox() const { return m_boundingBoxInUors; }
void IPointsProvider::_SetBoundingBox(DRange3dCR boundingBoxInUors)  { m_boundingBoxInUors = boundingBoxInUors; }
void IPointsProvider::_SetMaxPointsToPrefetch(int value) { m_maxPointsToPreFetch = value; }
int  IPointsProvider::_GetMaxPointsToPrefetch() const    { return m_maxPointsToPreFetch; }
ElementHandleCR IPointsProvider::_GetElementHandle() const { return m_eh; }
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
Transform IPointsProvider::GetUorToMeterTransform(DgnModelRefP model, bool useGlobalOrigin)
    {
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
    Transform uorToMeters;
    uorToMeters.InverseOf(metersToUors);
    return uorToMeters;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Transform IPointsProvider::_GetUorToMeterTransform(bool useGlobalOrigin) const
    {
    return GetUorToMeterTransform(mdlModelRef_getActive(), useGlobalOrigin);
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

END_DCPOINTCLOUDCORE_NAMESPACE
