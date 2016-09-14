/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshQuery.hpp $
|    $RCSfile: ScalableMeshPointQuery.hpp,v $
|   $Revision: 1.23 $
|       $Date: 2012/11/29 17:30:45 $
|     $Author: Mathieu.St-Pierre $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|                                                                        |
|    ScalableMeshNewFileCreator.h                              (C) Copyright 2001.        |
|                                                BCIVIL Corporation.        |
|                                                All Rights Reserved.    |
|                                                                       |
+----------------------------------------------------------------------*/

#pragma once

#include "ScalableMeshQuery.h"
//#include "InternalUtilityFunctions.h"
#include <ImagePP/all/h/HPMPooledVector.h>
#include "Edits\\ClipUtilities.h"
#include <json/json.h>
//#include <QuickVision\qvision.h>

// This define does not work when it is set to 10000 and a dataset of 120000 is used
#define MAX_POINTS_PER_DTM 10000
#define MAX_FEATURES_PER_DTM 10000


static HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment s_queryMemoryManager(20, 1000*sizeof(DPoint3d));

template<class EXTENT> EXTENT ScalableMeshPointQuery::GetExtentFromClipShape(const DPoint3d* pClipShapePts, 
                                                                 int             nbClipShapePts, 
                                                                 double          zMin, 
                                                                 double          zMax) 
    {
    if (1 > nbClipShapePts)
        {
        return ExtentOp<EXTENT>::Create(0.0, 0.0, 0.0, 
                                        0.0, 0.0, 0.0);
        }

    int nPts(nbClipShapePts);

    // Allow extra space to make sure the first point is equal to the last point in the array
    if (!(HDOUBLE_EQUAL_EPSILON(pClipShapePts[0].x, pClipShapePts[nbClipShapePts-1].x) && HDOUBLE_EQUAL_EPSILON(pClipShapePts[0].y, pClipShapePts[nbClipShapePts-1].y)))
        nPts++;

    HArrayAutoPtr<double> tmpPts (new double[nPts * 2]);

    // Convert to DPoint2d
    for(int i = 0; i < nbClipShapePts; i ++)
        {
        tmpPts[i * 2]     = pClipShapePts[i].x;
        tmpPts[i * 2 + 1] = pClipShapePts[i].y;
        }

    // Recopy last point to close the shape
    if (nPts > nbClipShapePts)
        {
        tmpPts[(nPts-1) * 2]     = pClipShapePts[0].x;
        tmpPts[(nPts-1) * 2 + 1] = pClipShapePts[0].y;
        }             

    HFCPtr<HGF2DCoordSys> pCoordSys(new HGF2DCoordSys());                                            
    HFCPtr<HVE2DPolygonOfSegments> pPolygon(new HVE2DPolygonOfSegments(nPts * 2, tmpPts, pCoordSys));

    HGF2DExtent extent = pPolygon->GetExtent();

    return ExtentOp<EXTENT>::Create(extent.GetXMin(), extent.GetYMin(), zMin, 
                                    extent.GetXMax(), extent.GetYMax(), zMax);
    }

template <class POINT> int ScalableMeshPointQuery::AddPoints(bvector<DPoint3d>&                   points, 
                                                             const HPMMemoryManagedVector<POINT>& pointList) /*const*/
    {        
    for (auto& pt : pointList)
        {
        points.push_back(pt);
        }    
        
    return SUCCESS;
    }

/*----------------------------------------------------------------------------+
|ScalableMeshFullResolutionPointQuery Method Definition Section - Begin
+----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------+
|ScalableMeshFullResolutionPointQuery::ScalableMeshFullResolutionPointQuery
+----------------------------------------------------------------------------*/
template <class POINT> ScalableMeshFullResolutionPointQuery<POINT>::ScalableMeshFullResolutionPointQuery(const HFCPtr<SMPointIndex<POINT, Extent3dType>>& pointIndexPtr, 
                                                                                           int                                              resolutionIndex)
    {        
    m_scmIndexPtr = pointIndexPtr;    
    m_resolutionIndex = resolutionIndex;
    }
/*----------------------------------------------------------------------------+
|ScalableMeshFullResolutionPointQuery::~ScalableMeshFullResolutionPointQuery
+----------------------------------------------------------------------------*/
template <class POINT> ScalableMeshFullResolutionPointQuery<POINT>::~ScalableMeshFullResolutionPointQuery()
    {
    }
   
/*----------------------------------------------------------------------------+
|ScalableMeshFullResolutionPointQuery::Query
+----------------------------------------------------------------------------*/
template <class POINT> int ScalableMeshFullResolutionPointQuery<POINT>::_Query(bvector<DPoint3d>&               points,
                                                                        const DPoint3d*                 pQueryShapePts, 
                                                                        int                             nbQueryShapePts, 
                                                                        const IScalableMeshQueryParametersPtr& scmQueryParamsPtr) const
    {        
    assert(scmQueryParamsPtr != 0);
    
    HPMMemoryManagedVector<POINT> pointList(&s_queryMemoryManager);

    int         status;

    Extent3dType contentExtent(m_scmIndexPtr->GetContentExtent());        
    Extent3dType queryExtent;
    
    HFCPtr<HVEShape> clipShapePtr;
    HAutoPtr<ISMPointIndexQuery<POINT, Extent3dType>> pointQueryP;

    assert(dynamic_cast<IScalableMeshFullResolutionQueryParams*>(scmQueryParamsPtr.get()) != 0);
    IScalableMeshFullResolutionQueryParams* queryParams(dynamic_cast<IScalableMeshFullResolutionQueryParams*>(scmQueryParamsPtr.get()));        

    if (pQueryShapePts != 0)
        {    
        queryExtent = GetExtentFromClipShape<Extent3dType>(pQueryShapePts, nbQueryShapePts, 
                                                                ExtentOp<Extent3dType>::GetZMin(contentExtent),
                                                                ExtentOp<Extent3dType>::GetZMax(contentExtent));
                    
        int nPts(nbQueryShapePts);

        // Allow extra space to make sure the first point is equal to the last point in the array
        if (!(HDOUBLE_EQUAL_EPSILON(pQueryShapePts[0].x, pQueryShapePts[nbQueryShapePts-1].x) && HDOUBLE_EQUAL_EPSILON(pQueryShapePts[0].y, pQueryShapePts[nbQueryShapePts-1].y)))
            nPts++;

        HArrayAutoPtr<double> tmpPts (new double[nPts * 2]);

        // Convert to DPoint2d
        for(int i = 0; i < nbQueryShapePts; i ++)
            {
            tmpPts[i * 2]     = pQueryShapePts[i].x;
            tmpPts[i * 2 + 1] = pQueryShapePts[i].y;
            }

        // Recopy last point to close the shape
        if (nPts > nbQueryShapePts)
            {
            tmpPts[(nPts-1) * 2]     = pQueryShapePts[0].x;
            tmpPts[(nPts-1) * 2 + 1] = pQueryShapePts[0].y;
            }
             
             
        HFCPtr<HGF2DCoordSys> pCoordSys(new HGF2DCoordSys());                                            
        HFCPtr<HVE2DPolygonOfSegments> pPolygon(new HVE2DPolygonOfSegments(nPts * 2, tmpPts, pCoordSys));

        HFCPtr<HVE2DShape> pContentShape(new HVE2DRectangle(ExtentOp<Extent3dType>::GetXMin(contentExtent), ExtentOp<Extent3dType>::GetYMin(contentExtent), 
                                                            ExtentOp<Extent3dType>::GetXMax(contentExtent), ExtentOp<Extent3dType>::GetYMax(contentExtent), pCoordSys));

        HFCPtr<HVEShape> pQueryShape = new HVEShape(*(pPolygon->IntersectShape(*pContentShape)));
        clipShapePtr = CreateClipShape(pQueryShape);  

        if (clipShapePtr->IsEmpty()) 
            clipShapePtr = pQueryShape;                
        }
    else
        {             
        /*queryExtent.xMin = ExtentOp<Extent3dType>::GetXMin(contentExtent);
        queryExtent.xMax = ExtentOp<Extent3dType>::GetXMax(contentExtent);
        queryExtent.yMin = ExtentOp<Extent3dType>::GetYMin(contentExtent);
        queryExtent.yMax = ExtentOp<Extent3dType>::GetYMax(contentExtent);
        queryExtent.zMin = ExtentOp<Extent3dType>::GetZMin(contentExtent);
        queryExtent.zMax = ExtentOp<Extent3dType>::GetZMax(contentExtent);*/
        queryExtent = contentExtent;
        
        DRange3d spatialIndexRange;
        spatialIndexRange.low.x = ExtentOp<Extent3dType>::GetXMin(queryExtent);
        spatialIndexRange.high.x = ExtentOp<Extent3dType>::GetXMax(queryExtent);
        spatialIndexRange.low.y = ExtentOp<Extent3dType>::GetYMin(queryExtent);
        spatialIndexRange.high.y = ExtentOp<Extent3dType>::GetYMax(queryExtent);              

        clipShapePtr = CreateClipShape(spatialIndexRange);                             
        }             
   
    if ((clipShapePtr != 0) && (clipShapePtr->IsEmpty() == false) && !clipShapePtr->IsRectangle())
        {         
        HFCPtr<HVE2DShape> clipShape = static_cast<HVE2DShape*>(clipShapePtr->GetShapePtr()->Clone());
        pointQueryP = new HGFLevelPointIndexByShapeQuery<POINT, Extent3dType>(clipShape, m_resolutionIndex, queryParams->GetReturnAllPtsForLowestLevel(), queryParams->GetMaximumNumberOfPoints());
        }
    else
        {                       
        pointQueryP = new HGFLevelPointIndexQuery<POINT, Extent3dType>(queryExtent, m_resolutionIndex, queryParams->GetReturnAllPtsForLowestLevel(), queryParams->GetMaximumNumberOfPoints());
        }      
        
    try   
        {           
    #ifdef ACTIVATE_NODE_QUERY_TRACING            
        //pointQueryP->SetTracingXMLFileName(AString("E:\\MyDoc\\SS3 - Iteration 1\\ScalableMesh\\New 3D Query\\Log\\CheckedNodesDuringLastQuery.xml"));                 
    #endif                          

        m_scmIndexPtr->Query(pointQueryP.get(), pointList);

        status = (int)SMQueryStatus::S_SUCCESS;
        
        if (queryParams->GetMaximumNumberOfPoints() < pointList.size())
            {
            status = (int)SMQueryStatus::S_NBPTSEXCEEDMAX;
            }        
        }
    catch (...)
        {
        status = (int)SMQueryStatus::S_ERROR;
        }
       
    if (status == (int)SMQueryStatus::S_SUCCESS)
        {
        status = AddPoints<POINT>(points, pointList);
        }
              
    return status;
    }

/*----------------------------------------------------------------------------+
|ScalableMeshFullResolutionPointQuery Method Definition Section - End
+----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------+
|ScalableMeshViewDependentPointQuery Method Definition Section - Begin
+----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------+
|ScalableMeshViewDependentPointQuery::ScalableMeshViewDependentPointQuery
+----------------------------------------------------------------------------*/
template <class POINT> ScalableMeshViewDependentPointQuery<POINT>::ScalableMeshViewDependentPointQuery(const HFCPtr<SMPointIndex<POINT, Extent3dType>>& pointIndexPtr)
    {        
    m_scmIndexPtr = pointIndexPtr;
    m_resolutionIndex    = INT_MAX;          
    }       

/*----------------------------------------------------------------------------+
|ScalableMeshViewDependentPointQuery::~ScalableMeshViewDependentPointQuery
+----------------------------------------------------------------------------*/
template <class POINT> ScalableMeshViewDependentPointQuery<POINT>::~ScalableMeshViewDependentPointQuery()
    {
    }

/*----------------------------------------------------------------------------+
|ScalableMeshViewDependentPointQuery::SetResolutionIndex
+----------------------------------------------------------------------------*/
template <class POINT> void ScalableMeshViewDependentPointQuery<POINT>::SetResolutionIndex(int resolutionIndex)
    {
    m_resolutionIndex = resolutionIndex;
    }
   
/*----------------------------------------------------------------------------+
|ScalableMeshViewDependentPointQuery::Query
+----------------------------------------------------------------------------*/
template <class POINT> int ScalableMeshViewDependentPointQuery<POINT>::_Query(bvector<DPoint3d>&               points,
                                                                       const DPoint3d*                  pQueryShapePts, 
                                                                       int                              nbQueryShapePts, 
                                                                       const IScalableMeshQueryParametersPtr&  scmQueryParamsPtr) const
    {    
    //MST More validation is required here.
    assert(scmQueryParamsPtr != 0);
    
    HPMMemoryManagedVector<POINT> pointList(&s_queryMemoryManager);    

    int            status;

    Extent3dType contentExtent(m_scmIndexPtr->GetContentExtent());    

    double minZ = ExtentOp<Extent3dType>::GetZMin(contentExtent);
    double maxZ = ExtentOp<Extent3dType>::GetZMax(contentExtent);
    
    Extent3dType queryExtent(GetExtentFromClipShape<Extent3dType>(pQueryShapePts, 
                                                                            nbQueryShapePts, 
                                                                            minZ,
                                                                            maxZ));

    HAutoPtr<ISMPointIndexQuery<POINT, Extent3dType>> pointQueryP;

    if (m_resolutionIndex != INT_MAX)
        {
        assert(dynamic_cast<ISrDTMViewDependentQueryParams*>(scmQueryParamsPtr.get()) != 0);
        assert((scmQueryParamsPtr->GetSourceGCS() == 0) && (scmQueryParamsPtr->GetTargetGCS() == 0));

        ISrDTMViewDependentQueryParams* queryParams(dynamic_cast<ISrDTMViewDependentQueryParams*>(scmQueryParamsPtr.get()));        

        pointQueryP = new ScalableMeshQuadTreeLevelPointIndexQuery<POINT, Extent3dType>(queryExtent, 
                                                                                      m_resolutionIndex,
                                                                                      queryParams->GetViewBox());
        }
    else        
        {
        assert(dynamic_cast<IScalableMeshViewDependentQueryParams*>(scmQueryParamsPtr.get()) != 0);

        IScalableMeshViewDependentQueryParams* queryParams(dynamic_cast<IScalableMeshViewDependentQueryParams*>(scmQueryParamsPtr.get()));       

        //MS Need to be removed
        double viewportRotMatrix[3][3];        
        double rootToViewMatrix[4][4];        

        memcpy(rootToViewMatrix, queryParams->GetRootToViewMatrix(), sizeof(double) * 4 * 4);
                        
        ScalableMeshQuadTreeViewDependentPointQuery<POINT, Extent3dType>* viewDependentQueryP(new ScalableMeshQuadTreeViewDependentPointQuery<POINT, Extent3dType>
                                                                                                    (queryExtent, 
                                                                                                     rootToViewMatrix,                   
                                                                                                     viewportRotMatrix,             
                                                                                                     queryParams->GetViewBox(),                                                                                                                                                             
                                                                                                     false));                        

       // viewDependentQueryP->SetTracingXMLFileName(AString("E:\\MyDoc\\SS3 - Iteration 17\\STM\\Bad Resolution Selection\\visitingNodes.xml"));

        viewDependentQueryP->SetMeanScreenPixelsPerPoint(queryParams->GetMinScreenPixelsPerPoint());

        //MS : Might need to be done at the ScalableMeshReprojectionQuery level.
        if ((scmQueryParamsPtr->GetSourceGCS() != 0) && (scmQueryParamsPtr->GetTargetGCS() != 0))
            {
            BaseGCSCPtr sourcePtr = scmQueryParamsPtr->GetSourceGCS();
            BaseGCSCPtr targetPtr = scmQueryParamsPtr->GetTargetGCS();
            viewDependentQueryP->SetReprojectionInfo(sourcePtr, targetPtr);
            }
            
            pointQueryP = viewDependentQueryP;            
        }

    DRange3d spatialIndexRange;

    Extent3dType ExtentPoints = m_scmIndexPtr->GetContentExtent();
    spatialIndexRange.low.x = ExtentOp<Extent3dType>::GetXMin(ExtentPoints);
    spatialIndexRange.high.x = ExtentOp<Extent3dType>::GetXMax(ExtentPoints);
    spatialIndexRange.low.y = ExtentOp<Extent3dType>::GetYMin(ExtentPoints);
    spatialIndexRange.high.y = ExtentOp<Extent3dType>::GetYMax(ExtentPoints);               

    HFCPtr<HVEShape> clipShapePtr = CreateClipShape(spatialIndexRange);

    if ((clipShapePtr != 0) && (clipShapePtr->IsEmpty() == false))
        {
        ISMPointIndexSpatialLimitWrapQuery<POINT, Extent3dType>* wrappedShapedQuery;
        HFCPtr<HVE2DShape> clipShape = static_cast<HVE2DShape*>(clipShapePtr->GetShapePtr()->Clone());
        wrappedShapedQuery = new ISMPointIndexSpatialLimitWrapQuery<POINT, Extent3dType>(clipShape, pointQueryP.release());

        pointQueryP = wrappedShapedQuery;
        }  
            
    try   
        {           
    #ifdef ACTIVATE_NODE_QUERY_TRACING            
        //queryObject.SetTracingXMLFileName(AString("E:\\MyDoc\\SS3 - Iteration 1\\ScalableMesh\\New 3D Query\\Log\\CheckedNodesDuringLastQuery.xml"));                 
    #endif                          
                    
        m_scmIndexPtr->Query(pointQueryP.get(), pointList);

        status = SUCCESS;       
        }
    catch (...)
        {
        status = ERROR;
        }
       
    if (status == SUCCESS)
        {         
        status = AddPoints(points, pointList);
        }
                   
    return status;
    }

/*----------------------------------------------------------------------------+
|ScalableMeshViewDependentPointQuery Method Definition Section - End
+----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------+
|ScalableMeshFixResolutionViewPointQuery Method Definition Section - Begin
+----------------------------------------------------------------------------*/
template <class POINT> ScalableMeshFixResolutionViewPointQuery<POINT>::ScalableMeshFixResolutionViewPointQuery(const HFCPtr<SMPointIndex<POINT, Extent3dType>>& pointIndexPtr, 
                                                                                                 const GeoCoords::GCS&                                        pointIndexGCS)
: m_pointIndexGCS(pointIndexGCS)
    {    
    m_scmIndexPtr = pointIndexPtr;    
    }

template <class POINT> ScalableMeshFixResolutionViewPointQuery<POINT>::~ScalableMeshFixResolutionViewPointQuery()
    {
    }
    
template <class POINT> int ScalableMeshFixResolutionViewPointQuery<POINT>::_Query(bvector<DPoint3d>&               points,
                                                                                  const DPoint3d*                  pQueryShapePts, 
                                                                                  int                              nbQueryShapePts, 
                                                                                  const IScalableMeshQueryParametersPtr&  scmQueryParamsPtr) const
    {
    assert(scmQueryParamsPtr != 0);
    
    HPMMemoryManagedVector<POINT> pointList(&s_queryMemoryManager);   
    IScalableMeshPtr scalableMeshFixResViewPtr; 

    if (dynamic_cast<IScalableMeshFixResolutionIndexQueryParams*>(scmQueryParamsPtr.get()) != 0)
        {
        IScalableMeshFixResolutionIndexQueryParams* queryParams(dynamic_cast<IScalableMeshFixResolutionIndexQueryParams*>(scmQueryParamsPtr.get()));               

        scalableMeshFixResViewPtr = new ScalableMeshSingleResolutionPointIndexView<POINT>(m_scmIndexPtr, queryParams->GetResolutionIndex(), m_pointIndexGCS);       
        }
    else    
        {
        assert(dynamic_cast<IScalableMeshFixResolutionMaxPointsQueryParams*>(scmQueryParamsPtr.get()) != 0);

        IScalableMeshFixResolutionMaxPointsQueryParams* queryParams(dynamic_cast<IScalableMeshFixResolutionMaxPointsQueryParams*>(scmQueryParamsPtr.get()));       
                                                            
        if (m_scmIndexPtr->IsEmpty() == false)
            {                    
            __int64 nbObjectsForLevel;            
            int     resolutionIndex = 0;
            
            size_t  nbResolution = m_scmIndexPtr->GetDepth();        
            
            for (; resolutionIndex <= (int)nbResolution; resolutionIndex++)
                    {
                    nbObjectsForLevel = m_scmIndexPtr->GetNbObjectsAtLevel(resolutionIndex);
                    
                    if (nbObjectsForLevel > queryParams->GetMaxNumberPoints())
                        {                
                        break;
                        }
                    }   

            resolutionIndex--; 

            if (resolutionIndex == -1)
                {
                return ERROR;
                }
            else
                {
                scalableMeshFixResViewPtr = new ScalableMeshSingleResolutionPointIndexView<POINT>(m_scmIndexPtr, resolutionIndex, m_pointIndexGCS);
                }   
            }  
        else
            {
            return ERROR;             
            }                   
        }
    
    IScalableMeshPointQueryPtr scalableMeshFixViewQueryPtr = scalableMeshFixResViewPtr->GetQueryInterface(SCM_QUERY_FULL_RESOLUTION);  
    IScalableMeshQueryParametersPtr queryParam((const IScalableMeshQueryParametersPtr&)IScalableMeshFullResolutionQueryParams::CreateParams());        
    int status = scalableMeshFixViewQueryPtr->Query(points, 0, 0, queryParam); 
    
    return status;
    }
            
/*----------------------------------------------------------------------------+
|ScalableMeshFixResolutionViewPointQuery Method Definition Section - End
+----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------+
|ScalableMeshViewDependentMeshQuery::ScalableMeshViewDependentMeshQuery
+----------------------------------------------------------------------------*/
template <class POINT> ScalableMeshViewDependentMeshQuery<POINT>::ScalableMeshViewDependentMeshQuery(const HFCPtr<SMPointIndex<POINT, Extent3dType>>& pointIndexPtr)
    {        
    m_scmIndexPtr = pointIndexPtr;    
    }       

/*----------------------------------------------------------------------------+
|ScalableMeshViewDependentMeshQuery::~ScalableMeshViewDependentMeshQuery
+----------------------------------------------------------------------------*/
template <class POINT> ScalableMeshViewDependentMeshQuery<POINT>::~ScalableMeshViewDependentMeshQuery()
    {
    }
   
/*----------------------------------------------------------------------------+
|ScalableMeshViewDependentMeshQuery::Query
+----------------------------------------------------------------------------*/
template <class POINT> int ScalableMeshViewDependentMeshQuery<POINT>::_Query(IScalableMeshMeshPtr&                                meshPtr,         
                                                                      const DPoint3d*                               pQueryExtentPts, 
                                                                      int                                           nbQueryExtentPts, 
                                                                      const IScalableMeshMeshQueryParamsPtr&  scmQueryParamsPtr) const 
    {    
    //MST More validation is required here.
    assert(scmQueryParamsPtr != 0);

    HPMMemoryManagedVector<POINT> pointList(&s_queryMemoryManager);    

    int            status;

    Extent3dType contentExtent(m_scmIndexPtr->GetContentExtent());    

    double minZ = ExtentOp<Extent3dType>::GetZMin(contentExtent);
    double maxZ = ExtentOp<Extent3dType>::GetZMax(contentExtent);
        
    Extent3dType queryExtent(ScalableMeshPointQuery::GetExtentFromClipShape<Extent3dType>(pQueryExtentPts,
                                                                                        nbQueryExtentPts,
                                                                                        minZ,
                                                                                        maxZ));

    HAutoPtr<ISMPointIndexQuery<POINT, Extent3dType>> pointQueryP;
             
    //MS Need to be removed
    double viewportRotMatrix[3][3];        
    double rootToViewMatrix[4][4];        

    static size_t maxNbPoints = 200000;
    IScalableMeshViewDependentMeshQueryParamsPtr scmViewDependentParamsPtr = IScalableMeshViewDependentMeshQueryParamsPtr(dynamic_cast<ScalableMeshViewDependentMeshQueryParams*>(scmQueryParamsPtr.get()));

    memcpy(rootToViewMatrix, scmViewDependentParamsPtr->GetRootToViewMatrix(), sizeof(double) * 4 * 4);    
                    
    ScalableMeshQuadTreeViewDependentMeshQuery<POINT, Extent3dType>* viewDependentQueryP(new ScalableMeshQuadTreeViewDependentMeshQuery<POINT, Extent3dType>
                                                                                                (queryExtent, 
                                                                                                 rootToViewMatrix,                   
                                                                                                 viewportRotMatrix,             
                                                                                                 scmViewDependentParamsPtr->GetViewBox(),
                                                                                                 false,                                                                                                 
                                                                                                 scmViewDependentParamsPtr->GetViewClipVector(),
                                                                                                 maxNbPoints));                        

    // viewDependentQueryP->SetTracingXMLFileName(AString("E:\\MyDoc\\SS3 - Iteration 17\\STM\\Bad Resolution Selection\\visitingNodes.xml"));

    viewDependentQueryP->SetMeanScreenPixelsPerPoint(scmViewDependentParamsPtr->GetMinScreenPixelsPerPoint());
    
    //MS : Might need to be done at the ScalableMeshReprojectionQuery level.
    
    if ((scmQueryParamsPtr->GetSourceGCS() != 0) && (scmQueryParamsPtr->GetTargetGCS() != 0))
        {
        BaseGCSCPtr sourcePtr = scmQueryParamsPtr->GetSourceGCS();
        BaseGCSCPtr targetPtr = scmQueryParamsPtr->GetTargetGCS();
        viewDependentQueryP->SetReprojectionInfo(sourcePtr, targetPtr);
        }
        
    pointQueryP = viewDependentQueryP;                    

    DRange3d spatialIndexRange;

    Extent3dType ExtentPoints = m_scmIndexPtr->GetContentExtent();
    spatialIndexRange.low.x = ExtentOp<Extent3dType>::GetXMin(ExtentPoints);
    spatialIndexRange.high.x = ExtentOp<Extent3dType>::GetXMax(ExtentPoints);
    spatialIndexRange.low.y = ExtentOp<Extent3dType>::GetYMin(ExtentPoints);
    spatialIndexRange.high.y = ExtentOp<Extent3dType>::GetYMax(ExtentPoints);               

    IScalableMeshClipContainerPtr clips;

    HFCPtr<HVEShape> clipShapePtr = CreateShapeFromClips(spatialIndexRange, clips);
       
    if ((clipShapePtr != 0) && (clipShapePtr->IsEmpty() == false))
        {
        //NEEDS_WORK_SM 
        assert(!"Not implemented yet");
    /*
        ISMPointIndexSpatialLimitWrapQuery<POINT, Extent3dType>* wrappedShapedQuery;
        HFCPtr<HVE2DShape> clipShape = static_cast<HVE2DShape*>(clipShapePtr->GetShapePtr()->Clone());
        wrappedShapedQuery = new ISMPointIndexSpatialLimitWrapQuery<POINT, Extent3dType>(clipShape, pointQueryP.release());

        pointQueryP = wrappedShapedQuery;
        */
        }  
            
    try   
        {           
    #ifdef ACTIVATE_NODE_QUERY_TRACING            
        //queryObject.SetTracingXMLFileName(AString("E:\\MyDoc\\SS3 - Iteration 1\\ScalableMesh\\New 3D Query\\Log\\CheckedNodesDuringLastQuery.xml"));                 
    #endif                          
                        
        DVec3d viewNormal;
        
        static bool s_useView = true; 
        
        if (s_useView)
            {
            viewNormal = DVec3d::From (rootToViewMatrix[2][0], rootToViewMatrix[2][1], rootToViewMatrix[2][2]);
            }
        else
            {
            viewNormal = DVec3d::From (0, 0, -1);
            }

        ScalableMeshMeshPtr returnMeshPtr (ScalableMeshMesh::Create (viewNormal));
         
            
        if (m_scmIndexPtr->Query(pointQueryP.get(), returnMeshPtr.get()))
            {       
            meshPtr = returnMeshPtr;
         
            status = SUCCESS;
            }
        else
            {
            status = ERROR;
            }       
        }
    catch (...)
        {
        status = ERROR;
        }

    
    return status;
    }


/*----------------------------------------------------------------------------+
|ScalableMeshViewDependentMeshQuery::Query
+----------------------------------------------------------------------------*/
template <class POINT> int ScalableMeshViewDependentMeshQuery<POINT>::_Query(bvector<IScalableMeshNodePtr>&                       meshNodes,    
                                                                      const DPoint3d*                               pQueryExtentPts,
                                                                      int                                           nbQueryExtentPts,
                                                                      const IScalableMeshMeshQueryParamsPtr&  scmQueryParamsPtr) const 
    {    
    //MST More validation is required here.
    assert(scmQueryParamsPtr != 0);

    HPMMemoryManagedVector<POINT> pointList(&s_queryMemoryManager);    

    int            status;

    Extent3dType contentExtent(m_scmIndexPtr->GetContentExtent());    

    double minZ = ExtentOp<Extent3dType>::GetZMin(contentExtent);
    double maxZ = ExtentOp<Extent3dType>::GetZMax(contentExtent);
        
    Extent3dType queryExtent(ScalableMeshPointQuery::GetExtentFromClipShape<Extent3dType>(pQueryExtentPts,
                                                                                               nbQueryExtentPts,
                                                                                               minZ,
                                                                                               maxZ));

    HAutoPtr<ISMPointIndexQuery<POINT, Extent3dType>> pointQueryP;
             
    //MS Need to be removed
    double viewportRotMatrix[3][3];        
    double rootToViewMatrix[4][4];        

    IScalableMeshViewDependentMeshQueryParamsPtr scmViewDependentParamsPtr = IScalableMeshViewDependentMeshQueryParamsPtr(dynamic_cast<ScalableMeshViewDependentMeshQueryParams*>(scmQueryParamsPtr.get()));

    memcpy(rootToViewMatrix, scmViewDependentParamsPtr->GetRootToViewMatrix(), sizeof(double) * 4 * 4);

    //NEEDS_WORK_SM : To be removed all!
    static size_t maxNbPoints = 150000000000;
                    
    ScalableMeshQuadTreeViewDependentMeshQuery<POINT, Extent3dType>* viewDependentQueryP(new ScalableMeshQuadTreeViewDependentMeshQuery<POINT, Extent3dType>
                                                                                                (queryExtent, 
                                                                                                 rootToViewMatrix,                   
                                                                                                 viewportRotMatrix,             
                                                                                                 scmViewDependentParamsPtr->GetViewBox(),
                                                                                                 false,
                                                                                                 scmViewDependentParamsPtr->GetViewClipVector(),
                                                                                                 maxNbPoints));                        

    // viewDependentQueryP->SetTracingXMLFileName(AString("E:\\MyDoc\\SS3 - Iteration 17\\STM\\Bad Resolution Selection\\visitingNodes.xml"));

    viewDependentQueryP->SetMeanScreenPixelsPerPoint(scmViewDependentParamsPtr->GetMinScreenPixelsPerPoint());

    //MS : Might need to be done at the ScalableMeshReprojectionQuery level.
    
    if ((scmQueryParamsPtr->GetSourceGCS() != 0) && (scmQueryParamsPtr->GetTargetGCS() != 0))
        {
        BaseGCSCPtr sourcePtr = scmQueryParamsPtr->GetSourceGCS();
        BaseGCSCPtr targetPtr = scmQueryParamsPtr->GetTargetGCS();
        viewDependentQueryP->SetReprojectionInfo(sourcePtr, targetPtr);
        }
        
    pointQueryP = viewDependentQueryP;                    

    DRange3d spatialIndexRange;

    Extent3dType ExtentPoints = m_scmIndexPtr->GetContentExtent();
    spatialIndexRange.low.x = ExtentOp<Extent3dType>::GetXMin(ExtentPoints);
    spatialIndexRange.high.x = ExtentOp<Extent3dType>::GetXMax(ExtentPoints);
    spatialIndexRange.low.y = ExtentOp<Extent3dType>::GetYMin(ExtentPoints);
    spatialIndexRange.high.y = ExtentOp<Extent3dType>::GetYMax(ExtentPoints);               

    IScalableMeshClipContainerPtr clips;

    HFCPtr<HVEShape> clipShapePtr = CreateShapeFromClips(spatialIndexRange, clips);
       
    if ((clipShapePtr != 0) && (clipShapePtr->IsEmpty() == false))
        {
        //NEEDS_WORK_SM 
        assert(!"Not implemented yet");
    /*
        ISMPointIndexSpatialLimitWrapQuery<POINT, Extent3dType>* wrappedShapedQuery;
        HFCPtr<HVE2DShape> clipShape = static_cast<HVE2DShape*>(clipShapePtr->GetShapePtr()->Clone());
        wrappedShapedQuery = new ISMPointIndexSpatialLimitWrapQuery<POINT, Extent3dType>(clipShape, pointQueryP.release());

        pointQueryP = wrappedShapedQuery;
        */
        }  
            
    try   
        {           
    #ifdef ACTIVATE_NODE_QUERY_TRACING            
        //queryObject.SetTracingXMLFileName(AString("E:\\MyDoc\\SS3 - Iteration 1\\ScalableMesh\\New 3D Query\\Log\\CheckedNodesDuringLastQuery.xml"));                 
    #endif                          
                
        //ScalableMeshMeshPtr returnMeshPtr (ScalableMeshMesh::Create (DVec3d::From (rootToViewMatrix[2][0], rootToViewMatrix[2][1], rootToViewMatrix[2][2])));
                    
        vector<typename SMPointIndexNode<POINT, Extent3dType>::QueriedNode> returnedMeshNodes;
        bool isComplete = true;
        
        if (m_scmIndexPtr->Query(pointQueryP.get(), returnedMeshNodes, &isComplete,
                                      scmViewDependentParamsPtr->IsProgressiveDisplay(), false,
                                      scmViewDependentParamsPtr->GetStopQueryCallback()))
            {        
            bool removeOverview = false;

            if (isComplete)
                {
                //NEEDS_WORK_SM : 
                assert(!"Need to check display cache instead");

                bool someNodeDiscarded = false;

                for (auto& nodes : returnedMeshNodes)
                    {             
                    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(nodes.m_indexNode->GetPointsPtr(false));

                    if (!pointsPtr.IsValid())
                        {
                        someNodeDiscarded = true;
                        break;
                        }
                    }   

                removeOverview = !someNodeDiscarded;
                }

            if (removeOverview)
                {                
                for (size_t nodeInd = 0; nodeInd < returnedMeshNodes.size(); nodeInd++)
                    {                
                    if (!returnedMeshNodes[nodeInd].m_isOverview)
                        {
                        meshNodes.push_back(new ScalableMeshNode<POINT>(returnedMeshNodes[nodeInd].m_indexNode));
                        }
                    }            
                }
            else
                {     
                isComplete = false; //NEEDS_WORK_SM_PROGRESSIVE : Could be done in Query object instead
                meshNodes.resize(returnedMeshNodes.size());
                for (size_t nodeInd = 0; nodeInd < returnedMeshNodes.size(); nodeInd++)
                    {                
                    meshNodes[nodeInd] = new ScalableMeshNode<POINT>(returnedMeshNodes[nodeInd].m_indexNode);
                    }            
                }

            status = (int)SMQueryStatus::S_SUCCESS;
            }
        else
            {
            status = (int)SMQueryStatus::S_ERROR;
            }

        if (!isComplete)
            {
            status = (int)SMQueryStatus::S_SUCCESS_INCOMPLETE;
            }        
        }
    catch (...)
        {
        status = (int)SMQueryStatus::S_ERROR;
        }

    return status;    
    }

/*----------------------------------------------------------------------------+
|ScalableMeshContextMeshQuery::ScalableMeshContextMeshQuery
+----------------------------------------------------------------------------*/
    template <class POINT> ScalableMeshContextMeshQuery<POINT>::ScalableMeshContextMeshQuery(const HFCPtr<SMPointIndex<POINT, Extent3dType>>& pointIndexPtr)
        : ScalableMeshViewDependentMeshQuery(pointIndexPtr)
    {          
    }       

/*----------------------------------------------------------------------------+
|ScalableMeshContextMeshQuery::~ScalableMeshContextMeshQuery
+----------------------------------------------------------------------------*/
    template <class POINT> ScalableMeshContextMeshQuery<POINT>::~ScalableMeshContextMeshQuery()
    {
    }

/*----------------------------------------------------------------------------+
|ScalableMeshContextMeshQuery::Query
+----------------------------------------------------------------------------*/
    template <class POINT> int ScalableMeshContextMeshQuery<POINT>::_Query(IScalableMeshMeshPtr&                                meshPtr,
                                                                             const DPoint3d*                               pQueryExtentPts,
                                                                             int                                           nbQueryExtentPts,
                                                                             const IScalableMeshMeshQueryParamsPtr&  scmQueryParamsPtr) const
    {
    int status = S_SUCCESS;
    Extent3dType contentExtent(m_scmIndexPtr->GetContentExtent());

    double minZ = ExtentOp<Extent3dType>::GetZMin(contentExtent);
    double maxZ = ExtentOp<Extent3dType>::GetZMax(contentExtent);

    Extent3dType queryExtent(ScalableMeshPointQuery::GetExtentFromClipShape<Extent3dType>(pQueryExtentPts,
        nbQueryExtentPts,
        minZ,
        maxZ));

    double viewportRotMatrix[3][3];
    double rootToViewMatrix[4][4];

    IScalableMeshViewDependentMeshQueryParamsPtr scmViewDependentParamsPtr = IScalableMeshViewDependentMeshQueryParamsPtr(dynamic_cast<ScalableMeshViewDependentMeshQueryParams*>(scmQueryParamsPtr.get()));

    memcpy(rootToViewMatrix, scmViewDependentParamsPtr->GetRootToViewMatrix(), sizeof(double) * 4 * 4);

    ScalableMeshQuadTreeContextMeshQuery<POINT, Extent3dType>* contextQueryP(new ScalableMeshQuadTreeContextMeshQuery < POINT, Extent3dType >
                                                                                         (queryExtent,
                                                                                         rootToViewMatrix,
                                                                                         viewportRotMatrix,
                                                                                         scmViewDependentParamsPtr->GetViewClipVector()
                                                                                         )
                                                                                         );
    ScalableMeshMeshPtr returnMeshPtr(ScalableMeshMesh::Create());
    if (!m_scmIndexPtr->Query(contextQueryP, returnMeshPtr.get()))
        {
        status = S_ERROR;
        }

    meshPtr = returnMeshPtr;
    return status;
    }

/*----------------------------------------------------------------------------+
|ScalableMeshContextMeshQuery::Query
+----------------------------------------------------------------------------*/
template <class POINT> int ScalableMeshContextMeshQuery<POINT>::_Query(bvector<IScalableMeshNodePtr>&                       meshNodes,
                                                                             const DPoint3d*                               pQueryExtentPts,
                                                                             int                                           nbQueryExtentPts,
                                                                             const IScalableMeshMeshQueryParamsPtr&  scmQueryParamsPtr) const
    {
    int status = S_SUCCESS;
    Extent3dType contentExtent(m_scmIndexPtr->GetContentExtent());

    double minZ = ExtentOp<Extent3dType>::GetZMin(contentExtent);
    double maxZ = ExtentOp<Extent3dType>::GetZMax(contentExtent);

    Extent3dType queryExtent(ScalableMeshPointQuery::GetExtentFromClipShape<Extent3dType>(pQueryExtentPts,
        nbQueryExtentPts,
        minZ,
        maxZ));

    IScalableMeshViewDependentMeshQueryParamsPtr scmViewDependentParamsPtr = IScalableMeshViewDependentMeshQueryParamsPtr(dynamic_cast<ScalableMeshViewDependentMeshQueryParams*>(scmQueryParamsPtr.get()));

    double viewportRotMatrix[3][3];
    double rootToViewMatrix[4][4];
    memcpy(rootToViewMatrix, scmViewDependentParamsPtr->GetRootToViewMatrix(), sizeof(double) * 4 * 4);

    ScalableMeshQuadTreeContextMeshQuery<POINT, Extent3dType>* contextQueryP(new ScalableMeshQuadTreeContextMeshQuery < POINT, Extent3dType >
                                                                             (queryExtent,
                                                                             rootToViewMatrix,
                                                                             viewportRotMatrix,
                                                                             scmViewDependentParamsPtr->GetViewClipVector()
                                                                             )
                                                                             );

    vector<typename SMPointIndexNode<POINT, Extent3dType>::QueriedNode> returnedMeshNodes;

    if (m_scmIndexPtr->Query(contextQueryP, returnedMeshNodes))
        {
        meshNodes.resize(returnedMeshNodes.size());
        for (size_t nodeInd = 0; nodeInd < returnedMeshNodes.size(); nodeInd++)
            {
            meshNodes[nodeInd] = new ScalableMeshNode<POINT>(returnedMeshNodes[nodeInd].m_indexNode);
            }


        status = S_SUCCESS;
        }
    else status = S_ERROR;


    return status;
    }

template <class POINT> ScalableMeshFullResolutionMeshQuery<POINT>::ScalableMeshFullResolutionMeshQuery(const HFCPtr<SMPointIndex<POINT, Extent3dType>>& pointIndexPtr)
    {
    m_scmIndexPtr = pointIndexPtr;
    }


template <class POINT> ScalableMeshFullResolutionMeshQuery<POINT>::~ScalableMeshFullResolutionMeshQuery()
    {}


template <class POINT> int ScalableMeshFullResolutionMeshQuery<POINT>::_Query(IScalableMeshMeshPtr&                                meshPtr,
                                                                       const DPoint3d*                               pQueryExtentPts,
                                                                       int                                           nbQueryExtentPts,
                                                                       const IScalableMeshMeshQueryParamsPtr&  scmQueryParamsPtr) const
    {
    Extent3dType contentExtent(m_scmIndexPtr->GetContentExtent());

    double minZ = ExtentOp<Extent3dType>::GetZMin(contentExtent);
    double maxZ = ExtentOp<Extent3dType>::GetZMax(contentExtent);

    Extent3dType queryExtent(ScalableMeshPointQuery::GetExtentFromClipShape<Extent3dType>(pQueryExtentPts,
        nbQueryExtentPts,
        minZ,
        maxZ));
    int status = SUCCESS;
    static size_t maxNbPoints = 150000000000;
    DPoint3d box[8] = { DPoint3d::From(ExtentOp<Extent3dType>::GetXMin(queryExtent), ExtentOp<Extent3dType>::GetYMin(queryExtent), ExtentOp<Extent3dType>::GetZMin(queryExtent)),
        DPoint3d::From(ExtentOp<Extent3dType>::GetXMax(queryExtent), ExtentOp<Extent3dType>::GetYMin(queryExtent), ExtentOp<Extent3dType>::GetZMin(queryExtent)),
        DPoint3d::From(ExtentOp<Extent3dType>::GetXMin(queryExtent), ExtentOp<Extent3dType>::GetYMax(queryExtent), ExtentOp<Extent3dType>::GetZMin(queryExtent)),
        DPoint3d::From(ExtentOp<Extent3dType>::GetXMin(queryExtent), ExtentOp<Extent3dType>::GetYMin(queryExtent), ExtentOp<Extent3dType>::GetZMax(queryExtent)),
        DPoint3d::From(ExtentOp<Extent3dType>::GetXMax(queryExtent), ExtentOp<Extent3dType>::GetYMax(queryExtent), ExtentOp<Extent3dType>::GetZMin(queryExtent)),
        DPoint3d::From(ExtentOp<Extent3dType>::GetXMax(queryExtent), ExtentOp<Extent3dType>::GetYMin(queryExtent), ExtentOp<Extent3dType>::GetZMax(queryExtent)),
        DPoint3d::From(ExtentOp<Extent3dType>::GetXMin(queryExtent), ExtentOp<Extent3dType>::GetYMax(queryExtent), ExtentOp<Extent3dType>::GetZMax(queryExtent)),
        DPoint3d::From(ExtentOp<Extent3dType>::GetXMax(queryExtent), ExtentOp<Extent3dType>::GetYMax(queryExtent), ExtentOp<Extent3dType>::GetZMax(queryExtent)) };
    ScalableMeshQuadTreeLevelMeshIndexQuery<POINT, Extent3dType>* meshQueryP(new ScalableMeshQuadTreeLevelMeshIndexQuery<POINT, Extent3dType>(queryExtent, m_scmIndexPtr->GetDepth(), box));

    try
        {
        ScalableMeshMeshPtr returnMeshPtr(ScalableMeshMesh::Create());


        if (m_scmIndexPtr->Query(meshQueryP, returnMeshPtr.get()))
            {
            meshPtr = returnMeshPtr;

            status = SUCCESS;
            }
        else
            {
            status = ERROR;
            }     
        }
    catch (...)
        {
        status = 1;
        }
    delete meshQueryP;
    return status;
    }

template <class POINT> int ScalableMeshFullResolutionMeshQuery<POINT>::_Query(bvector<IScalableMeshNodePtr>&                       meshNodes,
                                                                        const DPoint3d*                               pQueryExtentPts,
                                                                        int                                           nbQueryExtentPts,
                                                                        const IScalableMeshMeshQueryParamsPtr&  scmQueryParamsPtr) const
    {
    Extent3dType contentExtent(m_scmIndexPtr->GetContentExtent());

    double minZ = ExtentOp<Extent3dType>::GetZMin(contentExtent);
    double maxZ = ExtentOp<Extent3dType>::GetZMax(contentExtent);

    Extent3dType queryExtent(ScalableMeshPointQuery::GetExtentFromClipShape<Extent3dType>(pQueryExtentPts,
        nbQueryExtentPts,
        minZ,
        maxZ));
    if (nbQueryExtentPts == 0) queryExtent = contentExtent;
    int status = SUCCESS;
    
    DRange3d range; 
    DPoint3d box[8];

    range.low.x = ExtentOp<Extent3dType>::GetXMin(queryExtent);
    range.low.y = ExtentOp<Extent3dType>::GetYMin(queryExtent);
    range.low.z = ExtentOp<Extent3dType>::GetZMin(queryExtent);
    range.high.x = ExtentOp<Extent3dType>::GetXMax(queryExtent);
    range.high.y = ExtentOp<Extent3dType>::GetYMax(queryExtent);
    range.high.z = ExtentOp<Extent3dType>::GetZMax(queryExtent);

    range.Get8Corners (box);

    ScalableMeshQuadTreeLevelMeshIndexQuery<POINT, Extent3dType>* meshQueryP(new ScalableMeshQuadTreeLevelMeshIndexQuery<POINT, Extent3dType>(queryExtent, scmQueryParamsPtr->GetLevel() < (size_t)-1 ? scmQueryParamsPtr->GetLevel() : m_scmIndexPtr->GetDepth(), box));
    try
        {

        vector<typename SMPointIndexNode<POINT, Extent3dType>::QueriedNode> returnedMeshNodes;

        if (m_scmIndexPtr->Query(meshQueryP, returnedMeshNodes))
            {
            meshNodes.resize(returnedMeshNodes.size());
            for (size_t nodeInd = 0; nodeInd < returnedMeshNodes.size(); nodeInd++)
                {
                meshNodes[nodeInd] = new ScalableMeshNode<POINT>(returnedMeshNodes[nodeInd].m_indexNode);
                }


            status = SUCCESS;
            }
        else
            {
            status = ERROR;
            }   
        }
    catch (...)
        {
        status = 1;
        }
    delete meshQueryP;
    return status;
    }

template <class POINT> ScalableMeshReprojectionMeshQuery<POINT>::ScalableMeshReprojectionMeshQuery(IScalableMeshMeshQueryPtr         originalQueryPtr,
                                                                                     const HFCPtr<SMMeshIndex<POINT, Extent3dType>>& indexPtr,
                                                                              const  GeoCoords::GCS& sourceGCS,
                                                                              const  GeoCoords::GCS& targetGCS,
                                               const DRange3d&        extentInTargetGCS)

                                               : m_sourceGCS(sourceGCS),
                                               m_targetGCS(targetGCS),
                                               m_targetToSourceReproj(Reprojection::GetNull()),
                                               m_sourceToTargetReproj(Reprojection::GetNull()),
                                               m_extentInTargetGCS(extentInTargetGCS),
                                               m_scmIndexPtr(indexPtr)
    {
    m_originalQueryPtr = originalQueryPtr;

    static const ReprojectionFactory REPROJECTION_FACTORY;

    SMStatus reprojCreateStatus;
    m_targetToSourceReproj = REPROJECTION_FACTORY.Create(m_targetGCS, m_sourceGCS, 0, reprojCreateStatus);
    assert(SMStatus::S_SUCCESS == reprojCreateStatus);

    m_sourceToTargetReproj = REPROJECTION_FACTORY.Create(m_sourceGCS, m_targetGCS, 0, reprojCreateStatus);
    assert(SMStatus::S_SUCCESS == reprojCreateStatus);
    }


template <class POINT> int ScalableMeshReprojectionMeshQuery<POINT>::_Query(IScalableMeshMeshPtr&                               meshPtr,
                                       const DPoint3d*                              pQueryExtentPts,
                                       int                                          nbQueryExtentPts,
                                       const IScalableMeshMeshQueryParamsPtr& scmQueryParamsPtr) const
    {
    bvector<DPoint3d> extentReprojected;
    if (nbQueryExtentPts > 0)
        {
        // Create a shape from clip points
        HFCPtr<HVE2DShape> pReprojectedShape = ReprojectShapeDomainLimited(const_cast<BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr&>(m_sourceGCS.GetGeoRef().GetBasePtr()),
                                                                           const_cast<BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr&>(m_targetGCS.GetGeoRef().GetBasePtr()), pQueryExtentPts, nbQueryExtentPts);

        if (NULL == pReprojectedShape) return ERROR;

        // We extract the list of points from the simple shape
        HGF2DLocationCollection listOfPoints;
        if (NULL != pReprojectedShape)
            pReprojectedShape->Drop(&listOfPoints, 0.0);

        extentReprojected.resize(listOfPoints.size());

        struct HGF2DLocationToDPoint3d : unary_function<HGF2DLocation, DPoint3d>
            {
            DPoint3d operator () (const HGF2DLocation& rhs) const
                {
                const DPoint3d myPoint = { rhs.GetX(), rhs.GetY(), 0.0 };
                return myPoint;
                }
            };
        std::transform(listOfPoints.begin(), listOfPoints.end(), &extentReprojected[0], HGF2DLocationToDPoint3d());
        }

   int status = m_originalQueryPtr->Query(meshPtr, &extentReprojected[0], (int)nbQueryExtentPts, scmQueryParamsPtr);
    if(status == SUCCESS) m_sourceToTargetReproj.Reproject(meshPtr->EditPoints(), meshPtr->GetNbPoints(), meshPtr->EditPoints());
    return status;
    }

template <class POINT> int ScalableMeshReprojectionMeshQuery<POINT>::_Query(bvector<IScalableMeshNodePtr>&                      meshNodes,
                                       const DPoint3d*                              pQueryExtentPts,
                                       int                                          nbQueryExtentPts,
                                       const IScalableMeshMeshQueryParamsPtr& scmQueryParamsPtr) const
    {
    bvector<DPoint3d> extentReprojected;
    if (nbQueryExtentPts > 0)
        {
        // Create a shape from clip points
        HFCPtr<HVE2DShape> pReprojectedShape = ReprojectShapeDomainLimited(const_cast<BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr&>(m_sourceGCS.GetGeoRef().GetBasePtr()),
                                                                           const_cast<BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr&>(m_targetGCS.GetGeoRef().GetBasePtr()), pQueryExtentPts, nbQueryExtentPts);

        if (NULL == pReprojectedShape) return ERROR;

        // We extract the list of points from the simple shape
        HGF2DLocationCollection listOfPoints;
        if (NULL != pReprojectedShape)
            pReprojectedShape->Drop(&listOfPoints, 0.0);
        extentReprojected.resize(listOfPoints.size());

        struct HGF2DLocationToDPoint3d : unary_function<HGF2DLocation, DPoint3d>
            {
            DPoint3d operator () (const HGF2DLocation& rhs) const
                {
                const DPoint3d myPoint = { rhs.GetX(), rhs.GetY(), 0.0 };
                return myPoint;
                }
            };
        std::transform(listOfPoints.begin(), listOfPoints.end(), &extentReprojected[0], HGF2DLocationToDPoint3d());
        }
    IScalableMeshMeshQueryParamsPtr reprojectedQueryParams = new ScalableMeshViewDependentMeshQueryParams(*dynamic_cast<ScalableMeshViewDependentMeshQueryParams*>(scmQueryParamsPtr.get()));
    GeoCoordinates::BaseGCSCPtr targetGCSP = &*m_targetGCS.GetGeoRef().GetBasePtr();
    GeoCoordinates::BaseGCSCPtr sourceGCSP = &*m_sourceGCS.GetGeoRef().GetBasePtr();
    reprojectedQueryParams->SetGCS(sourceGCSP,
                                targetGCSP);
    int status = m_originalQueryPtr->Query(meshNodes, &extentReprojected[0], (int)nbQueryExtentPts, reprojectedQueryParams);
    for (size_t i = 0; i < meshNodes.size(); i++)
        {
        meshNodes[i] = IScalableMeshNodePtr(new ScalableMeshNodeWithReprojection<POINT>(meshNodes[i], m_sourceToTargetReproj));
        }
    return status;
    }

template <class POINT> ScalableMeshNodeRayQuery<POINT>::ScalableMeshNodeRayQuery(const HFCPtr<SMPointIndex<POINT, Extent3dType>>& pointIndexPtr)
    {
    m_scmIndexPtr = pointIndexPtr;
    }

template <class POINT> ScalableMeshNodeRayQuery<POINT>::~ScalableMeshNodeRayQuery()
    {
    }

template <class POINT> ScalableMeshNodePlaneQuery<POINT>::ScalableMeshNodePlaneQuery(const HFCPtr<SMPointIndex<POINT, Extent3dType>>& pointIndexPtr)
    {
    m_scmIndexPtr = pointIndexPtr;
    }

template <class POINT> ScalableMeshNodePlaneQuery<POINT>::~ScalableMeshNodePlaneQuery()
    {}

#define LOAD_NODE \
    if (!m_node->IsLoaded()) \
        m_node->Load(); \


template <class POINT> ScalableMeshNode<POINT>::ScalableMeshNode(HFCPtr<SMPointIndexNode<POINT, Extent3dType>>& nodePtr)
    {        
    m_node = nodePtr;
    }

template <class POINT> bool ScalableMeshNode<POINT>::_ArePoints3d() const
    {
    LOAD_NODE

    return m_node->m_nodeHeader.m_arePoints3d;
    }

template <class POINT> bool ScalableMeshNode<POINT>::_ArePointsFullResolution() const
    {
    LOAD_NODE

    return m_node->m_nodeHeader.m_IsLeaf;
    }

template <class POINT> IScalableMeshMeshPtr ScalableMeshNode<POINT>::_GetMesh(IScalableMeshMeshFlagsPtr& flags) const
        {
        LOAD_NODE

    auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, Extent3dType>, SMPointIndexNode<POINT, Extent3dType>>(m_node);

    IScalableMeshMeshPtr meshP;
    if (flags->ShouldLoadGraph())
        {
//        m_meshNode->PinGraph();
#ifdef SCALABLE_MESH_ATP
        int64_t loadAttempts;
        int64_t loadMisses;
        IScalableMeshATP::GetInt(L"nOfGraphLoadAttempts", loadAttempts);
        IScalableMeshATP::GetInt(L"nOfGraphStoreMisses", loadMisses);
        loadAttempts++;
#endif

        RefCountedPtr<SMMemoryPoolGenericBlobItem<MTGGraph>> graphPtr(m_meshNode->GetGraphPtr());
#ifdef SCALABLE_MESH_ATP
        IScalableMeshATP::StoreInt(L"nOfGraphLoadAttempts", loadAttempts);
        IScalableMeshATP::StoreInt(L"nOfGraphStoreMisses", loadMisses);
#endif

        ScalableMeshMeshWithGraphPtr meshPtr = ScalableMeshMeshWithGraph::Create(graphPtr->EditData(), ArePoints3d());


        RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> ptIndices(m_meshNode->GetPtsIndicePtr());
        RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(m_meshNode->GetPointsPtr());
        
        int status = meshPtr->AppendMesh(pointsPtr->size(), const_cast<DPoint3d*>(&pointsPtr->operator[](0)), ptIndices->size(), &(*ptIndices)[0], 0, 0, 0, 0, 0, 0);
        assert(status == SUCCESS);
        meshP = meshPtr.get();
        }
    else
        {               
        //NEEDS_WORK_SM_PROGRESSIF : Node header loaded unexpectingly  
        RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(m_node->GetPointsPtr());
        if (pointsPtr->size() > 0)
            {           
            ScalableMeshMeshPtr meshPtr = ScalableMeshMesh::Create();
        
            vector<DPoint3d> dataPoints(pointsPtr->size());
            pointsPtr->get(&dataPoints[0], dataPoints.size());

            int status = meshPtr->AppendMesh(pointsPtr->size(), &dataPoints[0],0,0, 0, 0, 0, 0, 0,0);
                                   
            RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> faceIndexes(m_meshNode->GetPtsIndicePtr());
            RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> uvIndexes(m_meshNode->GetUVsIndicesPtr());
            RefCountedPtr<SMMemoryPoolVectorItem<DPoint2d>> uvCoords(m_meshNode->GetUVCoordsPtr());
            if (faceIndexes->size() > 0)
                {
                status = meshPtr->AppendMesh(0, 0, faceIndexes->size(), &(*faceIndexes)[0], 0, 0, 0, flags->ShouldLoadTexture() && uvCoords.IsValid()? uvCoords->size() : 0,
                                             flags->ShouldLoadTexture() && uvCoords.IsValid() && uvCoords->size() > 0 ? &(*uvCoords)[0] : 0,
                                             flags->ShouldLoadTexture() && uvIndexes.IsValid() && uvIndexes->size() > 0 ? &(*uvIndexes)[0] : 0);
                }
            
            if (meshPtr->GetNbFaces() == 0)
                {                                              
                return nullptr;
                }

            assert(status == SUCCESS || m_node->GetNbPoints() ==0);        

            meshP = meshPtr.get();            
            }        
        }
    
    if (meshP == nullptr || meshP->GetNbFaces() == 0) return nullptr;
    return meshP;    
    }


template <class POINT> IScalableMeshMeshPtr ScalableMeshNode<POINT>::_GetMeshUnderClip(IScalableMeshMeshFlagsPtr& flags, uint64_t clipId) const
    {
    LOAD_NODE

    if (m_node->GetNbPoints() == 0) return nullptr;
    
    IScalableMeshMeshPtr meshP;
    ScalableMeshMeshPtr meshPtr = ScalableMeshMesh::Create();
    auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, Extent3dType>, SMPointIndexNode<POINT, Extent3dType>>(m_node);

    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(m_node->GetPointsPtr());
    bvector<DPoint3d> allPts(pointsPtr->size());
    memcpy(&allPts[0], &pointsPtr->operator[](0), pointsPtr->size()*sizeof(DPoint3d));
    for (size_t i = 0; i < m_meshNode->m_nbClips; ++i)
        {
        DifferenceSet d = m_meshNode->GetClipSet(i);
        if (d.clientID == clipId && d.toggledForID)
            {
            allPts.insert(allPts.end(), d.addedVertices.begin(), d.addedVertices.end());
            int status = meshPtr->AppendMesh(allPts.size(), &allPts[0], 0, 0, 0, 0, 0, 0, 0, 0);

            if (d.addedFaces.size() == 0) break;
            bvector<int32_t> allIndices;
            for (int i = 0; i + 2 < d.addedFaces.size(); i += 3)
                {

                for (size_t j = 0; j < 3; ++j)
                    {
                    int32_t idx = (int32_t)(d.addedFaces[i + j] >= d.firstIndex ? d.addedFaces[i + j] - d.firstIndex + m_node->GetNbPoints() + 1 : d.addedFaces[i + j]);
                    assert(idx > 0 && idx <= m_node->GetNbPoints() + d.addedVertices.size());
                    allIndices.push_back(idx);
                    }
                }

            status = meshPtr->AppendMesh(0, 0, allIndices.size(), &allIndices[0], 0, 0, 0, 0, 0, 0);
            meshP = meshPtr.get();
            assert(status == SUCCESS);
            break;
            }
        }
    
    if (meshP.get() != nullptr && meshP->GetNbFaces() == 0) return nullptr;
    return meshP;
    }

template <class POINT> void ScalableMeshNode<POINT>::ComputeDiffSet(DifferenceSet& diffs, const bset<uint64_t>& clipsToShow) const
    {
#ifdef USE_DIFFSET
    bool allClips = true;
    for (bool val : clipsToShow) if (!val) allClips = false;
#endif

    auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, Extent3dType>, SMPointIndexNode<POINT, Extent3dType>>(m_node);
    //std::cout << "ComputeDiffset for node " << m_meshNode->GetBlockID().m_integerID << std::endl;
    diffs.firstIndex = (int)m_meshNode->GetNbPoints() + 1;
    //auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, Extent3dType>, SMPointIndexNode<POINT, Extent3dType>>(m_node);    
    for (size_t i = 0; i < m_meshNode->m_nbClips; ++i)
        {
        DifferenceSet d = m_meshNode->GetClipSet(i);


            if (d.toggledForID && (d.clientID == 0 || (d.clientID < ((uint64_t)-1) && clipsToShow.count(d.clientID) == 0) && d.upToDate))
                {
                //meshPtr->ApplyDifferenceSet(d);
                diffs.ApplySet(d, 0);
                //break;

            }
        }
    }

template <class POINT> IScalableMeshMeshPtr ScalableMeshNode<POINT>::_GetMeshByParts(const bset<uint64_t>& clipsToShow) const
    {
    LOAD_NODE
        
    auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, Extent3dType>, SMPointIndexNode<POINT, Extent3dType>>(m_node);    
    IScalableMeshMeshPtr meshP;
    //NEEDS_WORK_SM_PROGRESSIF : Node header loaded unexpectingly
    if (m_node->GetNbPoints() > 0)
        {
        //auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, Extent3dType>, SMPointIndexNode<POINT, Extent3dType>>(m_node);        
        ScalableMeshMeshPtr meshPtr = ScalableMeshMesh::Create();

        RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(m_node->GetPointsPtr());

        vector<DPoint3d> dataPoints(pointsPtr->size());

        PtToPtConverter converter;

        for (size_t pointInd = 0; pointInd < pointsPtr->size(); pointInd++)
            {
            dataPoints[pointInd] = converter.operator()(pointsPtr->operator[](pointInd));
            }

        int status = meshPtr->AppendMesh(pointsPtr->size(), &dataPoints[0], 0, 0, 0, 0, 0, 0, 0, 0);               
        
        RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> faceIndexes = m_meshNode->GetPtsIndicePtr();        

        if (faceIndexes->size() > 0)
            {
            RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> uvIndexes = m_meshNode->GetUVsIndicesPtr();
            RefCountedPtr<SMMemoryPoolVectorItem<DPoint2d>> uvCoords = m_meshNode->GetUVCoordsPtr();        

            status = meshPtr->AppendMesh(0, 0, faceIndexes->size(), &(*faceIndexes)[0], 0, 0, 0, uvCoords->size(), &(*uvCoords)[0], &(*uvIndexes)[0]);
            }

        // release all                                   
        if (meshPtr->GetNbFaces() == 0)
            {                                    
            return nullptr;
            }

        if (clipsToShow.size() > 0 && status == SUCCESS)
            {
            DifferenceSet diffs;

            ComputeDiffSet(diffs, clipsToShow);


            if (m_meshNode->m_nbClips > 0)
                {                
                meshPtr->ApplyClipMesh(diffs);                
                }

            }
        assert(status == SUCCESS || m_node->GetNbPoints() == 0);

        meshP = meshPtr.get();        
        }
            
    if (meshP != nullptr && meshP->GetNbFaces() == 0) return nullptr;

    return meshP;
    }

template <class POINT> IScalableMeshTexturePtr ScalableMeshNode<POINT>::_GetTexture() const
    {
    LOAD_NODE
    
    IScalableMeshTexturePtr texturePtr;
    
    if (m_node->GetNbPoints() > 0)
        {
        auto meshNode = dynamic_pcast<SMMeshIndexNode<POINT, Extent3dType>, SMPointIndexNode<POINT, Extent3dType>>(m_node);

        RefCountedPtr<SMMemoryPoolBlobItem<Byte>> texPtr(meshNode->GetTexturePtr());

        if (texPtr.IsValid())
            {
            ScalableMeshTexturePtr textureP(ScalableMeshTexture::Create(texPtr));
        
            if (textureP->GetSize() != 0)
                texturePtr = IScalableMeshTexturePtr(textureP.get());                
            }
        }
    
    return texturePtr;
    }

template <class POINT> bool ScalableMeshNode<POINT>::_IsTextured() const
    {
    auto meshNode = dynamic_pcast<SMMeshIndexNode<POINT, Extent3dType>, SMPointIndexNode<POINT, Extent3dType>>(m_node);
    return meshNode->IsTextured();
    }

template <class POINT> IScalableMeshMeshPtr ScalableMeshCachedMeshNode<POINT>::_GetMesh(IScalableMeshMeshFlagsPtr& flags) const
    {        
    //NEEDS_WORK_TEXTURE use m_loadedMesh
    return __super::_GetMesh(flags);  
    }


template <class POINT> IScalableMeshMeshPtr ScalableMeshCachedMeshNode<POINT>::_GetMeshByParts(const bset<uint64_t>& clipsToShow) const
    {

    if (m_loadedMesh != 0)
        {
        return m_loadedMesh;
        }
    else
        {
        return __super::_GetMeshByParts(clipsToShow);
        }
    }

template <class POINT> IScalableMeshTexturePtr ScalableMeshCachedMeshNode<POINT>::_GetTexture() const
    {    
    if (m_loadedTexture != 0)
        {                
        return m_loadedTexture;        
        }
    else
        {
        return __super::_GetTexture();
        }   
    }


template <class POINT> void ScalableMeshCachedMeshNode<POINT>::LoadMesh(bool loadGraph, const bset<uint64_t>& clipsToShow) 
    {
    //Not implemented yet
    assert(loadGraph == false);    

    m_loadedMesh = __super::_GetMeshByParts(clipsToShow);
    m_loadedTexture = __super::_GetTexture();                 
    }

#if 0 //NEEDS_WORK_SM : Need do create a cached for texture since the same texture can pertain to more than one node
QvCachedNodeManager::CachedNodesList QvCachedNodeManager::m_cachedNodes;

QvCachedNodeManager::QvCachedNodeManager()
    {
    m_maxNbPoints = 3000000;
    m_totalNbPoints = 0;
    }

void QvCachedNodeManager::AddCachedNode(__int64 nodeId, QvElem* qvElem, DTMDataRef* dtmDataRef, size_t nbPoints/*, MaterialPtr& materialPtr*/)
    {
    assert(qvElem != 0);

    while (m_totalNbPoints + nbPoints > m_maxNbPoints)
        {
        assert(m_totalNbPoints >= m_cachedNodes.back().m_nbPoints);
        m_totalNbPoints -= m_cachedNodes.back().m_nbPoints;
        /*cachedNodeIter->*/m_cachedNodes.back().ReleaseQVisionCache(m_cachedNodes.back().m_nodeId);
        DTMElementMeshDisplayHandler::s_materialMap.erase(m_cachedNodes.back().m_nodeId);
        T_HOST.GetGraphicsAdmin()._DeleteQvElem (m_cachedNodes.back().m_qvElem);
//        m_cachedNodes.back().ReleaseQVisionCache();
        m_cachedNodes.pop_back();
        }            

    m_cachedNodes.push_front(QvCachedNode(nodeId, qvElem, dtmDataRef, nbPoints/*, materialPtr*/));

    m_totalNbPoints += nbPoints;
    }

void QvCachedNodeManager::ClearCachedNodes(DTMDataRef* dtmDataRef)
    {
    auto cachedNodeIter(m_cachedNodes.begin());
    auto cachedNodeIterEnd(m_cachedNodes.end());
    
    while (cachedNodeIter != cachedNodeIterEnd)
        {
        if (cachedNodeIter->m_dtmDataRef == dtmDataRef)            
            {
            // Delete QVCache for this particular node
            // Delete s_material id for this node too.
            cachedNodeIter->ReleaseQVisionCache(cachedNodeIter->m_nodeId);
            DTMElementMeshDisplayHandler::s_materialMap.erase(cachedNodeIter->m_nodeId);
            cachedNodeIter = m_cachedNodes.erase(cachedNodeIter);                        
            }
        else
            {
            cachedNodeIter++;
            }        
        }

    m_totalNbPoints = 0;
    }
     
//NEEDS_WORK_SM_PROGRESSIVE : Too slow, need optimization.
QvElem* QvCachedNodeManager::FindQvElem(__int64 nodeId, DTMDataRef* dtmDataRef)
    {
    auto cachedNodeIter(m_cachedNodes.begin());
    auto cachedNodeIterEnd(m_cachedNodes.end());

    QvElem* foundElem = 0;

    while (cachedNodeIter != cachedNodeIterEnd)
        {
        if ((cachedNodeIter->m_dtmDataRef == dtmDataRef) && 
            (cachedNodeIter->m_nodeId == nodeId))
            {
            QvCachedNode qvCachedNode(*cachedNodeIter);
            foundElem = cachedNodeIter->m_qvElem;
//            cachedNodeIter->ReleaseQVisionCache();
            m_cachedNodes.erase(cachedNodeIter);
            m_cachedNodes.push_front(qvCachedNode);                    
            break;
            }

        cachedNodeIter++;
        }

    return foundElem;
    }
/*
MaterialPtr QvCachedNodeManager::GetMaterial(__int64 nodeId, DTMDataRef* dtmDataRef)
{
    auto cachedNodeIter(m_cachedNodes.begin());
    auto cachedNodeIterEnd(m_cachedNodes.end());

    //QvElem* foundElem = 0;

    while (cachedNodeIter != cachedNodeIterEnd)
    {
        if ((cachedNodeIter->m_dtmDataRef == dtmDataRef) &&
            (cachedNodeIter->m_nodeId == nodeId))
        {
            return cachedNodeIter->m_material;
            //QvCachedNode qvCachedNode(*cachedNodeIter);
            //foundElem = cachedNodeIter->m_qvElem;
            //m_cachedNodes.erase(cachedNodeIter);
            //m_cachedNodes.push_front(qvCachedNode);
            //break;
        }

        cachedNodeIter++;
    }

    return nullptr;
}*/

QvCachedNodeManager& QvCachedNodeManager::GetManager()
    {
    static QvCachedNodeManager* s_manager = 0;

    if (s_manager == 0)
        {
        s_manager = new QvCachedNodeManager();
        }

    return *s_manager;
    }    
#endif

typedef struct {float x, y;} FloatXY;
typedef struct {float x, y, z;} FloatXYZ;

inline void ApplyClipDiffSetToMesh(FloatXYZ*& points, size_t& nbPoints, 
                                   int32_t*& faceIndexes, size_t& nbFaceIndexes, 
                                   FloatXY*& pUv, const int32_t*& pUvIndex, size_t& uvCount,
                                   FloatXYZ const* inPoints, size_t inNbPoints, 
                                   int32_t const*  inFaceIndexes, size_t inNbFaceIndexes, 
                                   const DPoint2d* pInUv, const int32_t* pInUvIndex, size_t inUvCount, 
                                   const DifferenceSet& d, 
                                   const DPoint3d& ptTranslation)
    {       
    points = new FloatXYZ[d.addedVertices.size() + inNbPoints];    

    for (size_t ind = inNbPoints; ind < d.addedVertices.size() + inNbPoints; ind++)    
        {
        points[ind].x = (float)(d.addedVertices[ind - inNbPoints].x - ptTranslation.x);
        points[ind].y = (float)(d.addedVertices[ind - inNbPoints].y - ptTranslation.y);
        points[ind].z = (float)(d.addedVertices[ind - inNbPoints].z - ptTranslation.z);        
        }
    
    if (inNbPoints > 0)
        {
        memcpy(points, inPoints, sizeof(FloatXYZ) * inNbPoints);        
        }    

    if (d.addedUvIndices.size() > 0)
        {
        pUv = new FloatXY[d.addedUvs.size() + inUvCount];

        for (size_t ind = inUvCount; ind < d.addedUvs.size() + inUvCount; ind++)
            {
            pUv[ind].x = d.addedUvs[ind - inUvCount].x;
            pUv[ind].y = d.addedUvs[ind - inUvCount].y;
            }
        
        if (inUvCount > 0)
            {
            for (size_t ind = 0; ind < inUvCount; ind++)
                {
                pUv[ind].x = pInUv[ind].x;
                pUv[ind].y = pInUv[ind].y;
                }            
            }
                
        for (size_t uvI = 0; uvI < d.addedUvs.size() + inUvCount; ++uvI)
            { 
            if (pUv[uvI].x < 0)  pUv[uvI].x = 0;
            if (pUv[uvI].y < 0)  pUv[uvI].y = 0;
            if (pUv[uvI].x > 1)  pUv[uvI].x = 1;
            if (pUv[uvI].y > 1)  pUv[uvI].y = 1;
            }
        }
    else if (pInUvIndex)
        {        
        pUvIndex = 0;
        }

    if (d.addedFaces.size() >= 3 && d.addedFaces.size() < 1024 * 1024)
        {
        size_t newMaxNIndexes = d.addedFaces.size();
        int32_t* newfaceIndexes = new int32_t[newMaxNIndexes];
        size_t newNIndexes = 0;
        int32_t* newUvIndices = d.addedUvIndices.size() > 0? new int32_t[newMaxNIndexes] : nullptr;
        for (int i = 0; i + 2 < d.addedFaces.size(); i += 3)
            {
            if (!(d.addedFaces[i] - 1 >= 0 && d.addedFaces[i] - 1 < inNbPoints + d.addedVertices.size() && d.addedFaces[i + 1] - 1 >= 0 && d.addedFaces[i + 1] - 1 < inNbPoints + d.addedVertices.size()
                && d.addedFaces[i + 2] - 1 >= 0 && d.addedFaces[i + 2] - 1 < inNbPoints + d.addedVertices.size()))
                {
#if SM_TRACE_CLIP_MESH
                std::string s;
                s += "INDICES " + std::to_string(d.addedFaces[i]) + " " + std::to_string(d.addedFaces[i + 1]) + " " + std::to_string(d.addedFaces[i + 2]);
#endif
                continue;
                }
             assert(d.addedFaces[i] - 1 >= 0 && d.addedFaces[i] - 1 < inNbPoints + d.addedVertices.size() && d.addedFaces[i + 1] - 1 >= 0 && d.addedFaces[i + 1] - 1 < inNbPoints + d.addedVertices.size()
            && d.addedFaces[i + 2] - 1 >= 0 && d.addedFaces[i + 2] - 1 < inNbPoints + d.addedVertices.size());
            for (size_t j = 0; j < 3 && newNIndexes <newMaxNIndexes; ++j)
                {
                int32_t idx = (int32_t)(d.addedFaces[i + j] >= d.firstIndex ? d.addedFaces[i + j] - d.firstIndex + inNbPoints + 1 : d.addedFaces[i + j]);
                assert(idx > 0 && idx <= inNbPoints + d.addedVertices.size());
                newfaceIndexes[newNIndexes] = idx;

                if (d.addedUvIndices.size() > 0)
                    {
                    if (i + j > d.addedUvIndices.size()) newUvIndices[newNIndexes] = 1;
                    newUvIndices[newNIndexes] = d.addedUvIndices[i + j] + (int32_t)inUvCount;
                    assert(newUvIndices[newNIndexes] <= inUvCount + d.addedUvs.size());
                    }
                newNIndexes++;
                }
            }
        nbFaceIndexes = newNIndexes;        
        faceIndexes = newfaceIndexes;
        if (d.addedUvIndices.size() > 0)
            {            
            pUvIndex = newUvIndices;
            }
        }
    else
        {
        nbFaceIndexes = 0;        
        faceIndexes = nullptr;
        }

    nbPoints = inNbPoints + d.addedVertices.size();

    if (d.addedUvIndices.size() > 0) uvCount = inUvCount + d.addedUvs.size();
    }


#define QV_RGBA_FORMAT   0
#define QV_BGRA_FORMAT   1
#define QV_RGB_FORMAT    2
#define QV_BGR_FORMAT    3
#define QV_GRAY_FORMAT   4
#define QV_ALPHA_FORMAT  5
#define QV_RGBS_FORMAT   6      // 4 band with alpha stencil (0 or 255 only)
#define QV_BGRS_FORMAT   7      // 4 band with alpha stencil (0 or 255 only)



template <class POINT> ScalableMeshCachedDisplayNode<POINT>::ScalableMeshCachedDisplayNode(HFCPtr<SMPointIndexNode<POINT, Extent3dType>>& nodePtr)
    : ScalableMeshNode(nodePtr)
    {    
    auto meshNode = dynamic_pcast<SMMeshIndexNode<POINT, Extent3dType>, SMPointIndexNode<POINT, Extent3dType>>(m_node);                
    m_cachedDisplayData = meshNode->GetDisplayData();                
    }

template <class POINT> ScalableMeshCachedDisplayNode<POINT>::~ScalableMeshCachedDisplayNode()
    {
    }

template <class POINT> void ScalableMeshCachedDisplayNode<POINT>::AddClipVector(ClipVectorPtr& clipVector)
    {
    m_clipVectors.push_back(clipVector);    
    }
     
template <class POINT> bool ScalableMeshCachedDisplayNode<POINT>::IsLoaded() const
    {
    return m_cachedDisplayData.IsValid();
    }

template <class POINT> bool ScalableMeshCachedDisplayNode<POINT>::HasCorrectClipping(const bset<uint64_t>& clipsToShow) const
    {
    assert(IsLoaded() == true);

    const bvector<uint64_t>& appliedClips(const_cast<SmCachedDisplayData*>(m_cachedDisplayData->GetData())->GetAppliedClips());
    auto meshNode = dynamic_pcast<SMMeshIndexNode<POINT, Extent3dType>, SMPointIndexNode<POINT, Extent3dType>>(m_node);                
    bvector<bool> appliedClipsVisible;
    appliedClipsVisible.resize(appliedClips.size(), false);        
    
    for (auto& clipToShow : clipsToShow)
        {                    
        if (meshNode->HasClip(clipToShow))
            {        
            size_t clipInd = 0;

            for (; clipInd < appliedClips.size(); clipInd++)
                {              
                if (appliedClips[clipInd] == clipToShow)
                    {
                    appliedClipsVisible[clipInd] = true;
                    break;
                    }
                }

            //Some visible clip was not previously applied.
            if (clipInd == appliedClips.size())
                {                            
                return false;                            
                }
            }
        }
    
    //One of the previously applied clip is not visible
    for (auto& appliedClipVisible : appliedClipsVisible)
        {
        if (!appliedClipVisible)
            return false;
        }
                        
    return true;
    }

template <class POINT> void ScalableMeshCachedDisplayNode<POINT>::RemoveDisplayDataFromCache() 
    {
    if (IsLoaded())
        {
        auto meshNode = dynamic_pcast<SMMeshIndexNode<POINT, Extent3dType>, SMPointIndexNode<POINT, Extent3dType>>(m_node);                
            
       // assert(m_cachedDisplayData->GetRefCount() == 2);
        m_cachedDisplayData = 0;
    
        meshNode->RemoveDisplayData();
        }
    }

template <class POINT> void ScalableMeshCachedDisplayNode<POINT>::LoadMesh(bool loadGraph, const bset<uint64_t>& clipsToShow, IScalableMeshDisplayCacheManagerPtr& displayCacheManagerPtr, bool loadTexture)
    {
    static bool s_deactivateTexture = false; 

    //Not implemented yet
    assert(loadGraph == false);
    assert(displayCacheManagerPtr != 0);

    LOAD_NODE
            
    if (displayCacheManagerPtr != 0 && !m_cachedDisplayData.IsValid())                
        {        
        //NEEDS_WORK_SM_PROGRESSIF : Node header loaded unexpectingly
        if (m_node->GetNbPoints() > 0)
            {
            //NEEDS_WORK_SM : Load texture here
            //m_cachedDisplayTexture

            auto meshNode = dynamic_pcast<SMMeshIndexNode<POINT, Extent3dType>, SMPointIndexNode<POINT, Extent3dType>>(m_node);            
            
            DRange3d range3D(_GetContentExtent());

            DPoint3d centroid;
            centroid = DPoint3d::From((range3D.high.x + range3D.low.x) / 2.0, (range3D.high.y + range3D.low.y) / 2.0, (range3D.high.z + range3D.low.z) / 2.0);            

            RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(m_node->GetPointsPtr());
            vector<FloatXYZ> dataPoints(pointsPtr->size());

            for (size_t pointInd = 0; pointInd < pointsPtr->size(); pointInd++)
                {
                dataPoints[pointInd].x = (float)(PointOp<POINT>::GetX(pointsPtr->operator[](pointInd)) - centroid.x);
                dataPoints[pointInd].y = (float)(PointOp<POINT>::GetY(pointsPtr->operator[](pointInd)) - centroid.y);
                dataPoints[pointInd].z = (float)(PointOp<POINT>::GetZ(pointsPtr->operator[](pointInd)) - centroid.z);
                }            
            
            FloatXYZ*       toLoadPoints = 0;
            size_t          toLoadNbPoints = 0;
            int32_t*        toLoadFaceIndexes = 0;
            size_t          toLoadNbFaceIndexes = 0;
            FloatXY*        toLoadUv = 0;
            const int32_t*  toLoadUvIndex = 0;
            size_t          toLoadUvCount = 0;                

            RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> faceIndexes(meshNode->GetPtsIndicePtr());
                                    
            size_t nbFaceIndices = faceIndexes->size(); 

            if (nbFaceIndices == 0)
                {                        
                return;
                }


            SmCachedDisplayTexture* cachedDisplayTexture = 0;
            size_t                  qvMemorySizeEstimate = 0;
            
            if (loadTexture && meshNode->IsTextured())
                {             
                //NEEDS_WORK_SM : Don't keep texture in memory.
                IScalableMeshTexturePtr smTexturePtr(GetTexture());

                if (smTexturePtr.IsValid())
                    {
                    BentleyStatus status = displayCacheManagerPtr->_CreateCachedTexture(cachedDisplayTexture,
                                                                                        smTexturePtr->GetDimension().x,
                                                                                        smTexturePtr->GetDimension().y,
                                                                                        false,
                                                                                        QV_RGB_FORMAT,
                                                                                        smTexturePtr->GetData());

                    //Estimate which seems to give good result. 
                    qvMemorySizeEstimate += smTexturePtr->GetDimension().x * smTexturePtr->GetDimension().y * 6;

                    assert(status == SUCCESS);
                    }
                }

            const DPoint2d* uvPtr = 0;
            size_t          nbUvs = 0;
            const int32_t*  uvIndicesP = 0;

            RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> uvIndicesPtr;
            RefCountedPtr<SMMemoryPoolVectorItem<DPoint2d>> uvCoordsPtr;

            if (cachedDisplayTexture != 0)
                {                                                                
                uvIndicesPtr = meshNode->GetUVsIndicesPtr();

                if (uvIndicesPtr.IsValid() && uvIndicesPtr->size() > 0)
                    uvIndicesP = &(*uvIndicesPtr)[0];

                uvCoordsPtr = meshNode->GetUVCoordsPtr();

                if (uvCoordsPtr.IsValid() && uvCoordsPtr->size() > 0)
                    {
                    nbUvs = uvCoordsPtr->size();
                    uvPtr = &(*uvCoordsPtr)[0];
                    }                
                }

            DifferenceSet clipDiffSet;
            bool isClipped = false;
            bvector<uint64_t> appliedClips;

            if (meshNode->m_nbClips > 0 && (clipsToShow.size() > 0))
                {
                ComputeDiffSet(clipDiffSet, clipsToShow);

                ApplyClipDiffSetToMesh(toLoadPoints, toLoadNbPoints,
                                       toLoadFaceIndexes, toLoadNbFaceIndexes,
                                       toLoadUv, toLoadUvIndex, toLoadUvCount,
                                       &dataPoints[0], dataPoints.size(),
                                       &(*faceIndexes)[0], nbFaceIndices,
                                       uvPtr, uvIndicesP, nbUvs,
                                       clipDiffSet,
                                       centroid);

                bool dbg = false;

                if (dbg)
                    {
                    const wchar_t* s_path = L"E:\\output\\scmesh\\2016-05-31\\";
                    bvector<DPoint3d> ptArray;
                    for (size_t i = 0; i < toLoadNbPoints; ++i)
                        ptArray.push_back(DPoint3d::From(toLoadPoints[i].x + centroid.x, toLoadPoints[i].y + centroid.y, toLoadPoints[i].z + centroid.z));
                    WString name = WString(s_path) + L"fmeshduringdraw_";
                    name.append(to_wstring(meshNode->GetBlockID().m_integerID).c_str());
                    name.append(L"_");
                    name.append(to_wstring(meshNode->m_nodeHeader.m_nodeExtent.low.x).c_str());
                    name.append(L"_");
                    name.append(to_wstring(meshNode->m_nodeHeader.m_nodeExtent.low.y).c_str());
                    name.append(L".m");
                    FILE* meshAfterClip = _wfopen(name.c_str(), L"wb");
                    size_t ptCount = toLoadNbPoints;
                    size_t faceCount = toLoadNbFaceIndexes;
                    fwrite(&ptCount, sizeof(size_t), 1, meshAfterClip);
                    fwrite(&ptArray[0], sizeof(DPoint3d), ptCount, meshAfterClip);
                    fwrite(&faceCount, sizeof(size_t), 1, meshAfterClip);
                    fwrite(toLoadFaceIndexes, sizeof(int32_t), faceCount, meshAfterClip);
                    fclose(meshAfterClip);
                    }


                for (size_t ind = 0; ind < toLoadNbFaceIndexes; ind++)
                    {
                    toLoadFaceIndexes[ind] -= 1;
                    }
                
                for (auto& clipToShow : clipsToShow)
                    {
                    if (meshNode->HasClip(clipToShow))
                        {
                        appliedClips.push_back(clipToShow);
                        }
                    }

                isClipped = true;
                }
            else
                {
                toLoadPoints = &dataPoints[0];
                toLoadNbPoints = dataPoints.size();
                toLoadFaceIndexes = new int32_t[nbFaceIndices];
                toLoadNbFaceIndexes = nbFaceIndices;

                //NEEDS_WORK_SM : Could generate them starting at 0.
                for (size_t ind = 0; ind < toLoadNbFaceIndexes; ind++)
                    {
                    toLoadFaceIndexes[ind] = (*faceIndexes)[ind] - 1;
                    }

                if (nbUvs > 0)
                    {
                    toLoadUv = new FloatXY[nbUvs];

                    for (size_t ind = 0; ind < nbUvs; ind++)
                        {
                        toLoadUv[ind].x = uvPtr[ind].x;
                        toLoadUv[ind].y = uvPtr[ind].y;
                        }
                    }

                toLoadUvIndex = uvIndicesP;
                toLoadUvCount = nbUvs;
                }

            // Pointers to the arrays that we will pass to QV
            float* finalPointPtr = (float*)toLoadPoints;
            size_t finalPointNb = toLoadNbPoints;
            int32_t* finalIndexPtr = toLoadFaceIndexes;
            size_t finalIndexNb = toLoadNbFaceIndexes;
            float* finalUVPtr = 0;
            
            // For textured meshes, we need to create new arrays of points, UV coords and face indices, such that face indices map to the correct points and UVs
            // In particular, this requires to duplicate points having different UVs in different triangles, as it typically happens for meshes generated by ContextCapture
            typedef std::pair<int32_t, int32_t> PointUVIndexPair;
            bmap<PointUVIndexPair, int32_t> mapPointUVIndexToNewIndex;
            bvector<FloatXYZ> newPoints;
            bvector<FloatXY> newUVs;
            bvector<int32_t> newIndices;  

            if (cachedDisplayTexture != 0 && toLoadUvCount > 0)
                {                                                         
                for (size_t faceInd = 0; faceInd < toLoadNbFaceIndexes; faceInd++)
                    {
                    int32_t pointInd = toLoadFaceIndexes[faceInd];
                    int32_t uvInd = toLoadUvIndex[faceInd] - 1; // For UVs, we haven't yet made the indices zero-based
                    // When we encounter the point/UV pair for the first time, we create a new element in the new point and UV arrays
                    // Otherwise, we retrieve the index of the point/UV pair from the map
                    PointUVIndexPair p(pointInd, uvInd);
                    auto mapIt = mapPointUVIndexToNewIndex.find(p);
                    if (mapIt == mapPointUVIndexToNewIndex.end())
                        {
                        int32_t newIndex = (int32_t)newPoints.size();
                        mapPointUVIndexToNewIndex[p] = newIndex;
                        newIndices.push_back(newIndex);
                        newPoints.push_back(toLoadPoints[pointInd]);
                        FloatXY uv = toLoadUv[uvInd];
                        assert(uv.x <= 1.f && uv.x >= 0.f);
                        assert(uv.y <= 1.f && uv.y >= 0.f);
                        newUVs.push_back({ uv.x, 1.f - uv.y }); // Different convention in QV (or ScalableMesh bug?)
                        }
                    else
                        {
                        int32_t newIndex = mapIt->second;
                        newIndices.push_back(newIndex);
                        }
                    }

                // Update the pointers passed to QV so that they point to the new arrays
                finalPointPtr = (float*)newPoints.data();
                finalUVPtr = (float*)newUVs.data();
                finalPointNb = newPoints.size();
                finalIndexPtr = newIndices.data();
                finalIndexNb = newIndices.size();                    
                }

            SmCachedDisplayMesh* cachedDisplayMesh = 0;            
            
            if (s_deactivateTexture)
                {
                BentleyStatus status = displayCacheManagerPtr->_CreateCachedMesh(cachedDisplayMesh, 
                                                                                 finalPointNb, 
                                                                                 &centroid,
                                                                                 finalPointPtr,
                                                                                 0,
                                                                                 (int)finalIndexNb / 3,
                                                                                 finalIndexPtr, 
                                                                                 0,
                                                                                 0);

                assert(status == SUCCESS);
                }
            else
                {
                BentleyStatus status = displayCacheManagerPtr->_CreateCachedMesh(cachedDisplayMesh, 
                                                                                 finalPointNb,
                                                                                 &centroid,
                                                                                 finalPointPtr,
                                                                                 0,
                                                                                 (int)finalIndexNb / 3,
                                                                                 finalIndexPtr,
                                                                                 finalUVPtr,
                                                                                 cachedDisplayTexture);

                assert(status == SUCCESS);
                }         
            
            qvMemorySizeEstimate += finalPointNb * sizeof(float) * 3 + finalIndexNb * sizeof(int32_t) + sizeof(float) * 2 * finalPointNb;

            SmCachedDisplayData* displayData(new SmCachedDisplayData(cachedDisplayMesh,
                                                                     cachedDisplayTexture,
                                                                     displayCacheManagerPtr, 
                                                                     qvMemorySizeEstimate, 
                                                                     appliedClips));
            
            m_cachedDisplayData = meshNode->AddDisplayData(displayData);

            if (isClipped)
                {
                if (toLoadPoints != 0)
                    delete[] toLoadPoints;

                if (toLoadUvIndex != 0)
                    delete[] toLoadUvIndex;
                }

            if (toLoadFaceIndexes != 0)
                delete[] toLoadFaceIndexes;

            if (toLoadUv != 0)
                delete[] toLoadUv;
            }       
        }                    
    }
    
    
template <class POINT>bvector<IScalableMeshNodePtr> ScalableMeshNode<POINT>::_GetNeighborAt(char relativePosX, char relativePosY, char relativePosZ) const
    {
    LOAD_NODE

    bvector<IScalableMeshNodePtr> neighbors;
    size_t neighborInd = 0;
    //see HGFSpatialIndex.hpp for node neighbor relations
    if (relativePosZ == -1) neighborInd += 8;
    else if (relativePosZ == 1) neighborInd += 17;
    neighborInd += (relativePosX - (-1)) - 3 * (relativePosY - 1);
    if (relativePosZ == 0 && neighborInd >= 5) --neighborInd; //accounts for there being 1 less neighbor on same level
    for (size_t i = 0; i < m_node->m_apNeighborNodes[neighborInd].size(); i++)
        {
        neighbors.push_back(new ScalableMeshNode<POINT>(m_node->m_apNeighborNodes[neighborInd][i]));
        }
    return neighbors;
    }

template <class POINT>bvector<IScalableMeshNodePtr> ScalableMeshNode<POINT>::_GetChildrenNodes() const
    {
    LOAD_NODE

    bvector<IScalableMeshNodePtr> children;
    if (m_node->m_nodeHeader.m_IsLeaf) return children;
    if (m_node->GetSubNodeNoSplit() != NULL)
        {
        auto var = m_node->GetSubNodeNoSplit();
        children.push_back(new ScalableMeshNode<POINT>(var));
        }
    else
        for (size_t i = 0; i < m_node->m_apSubNodes.size(); i++)
            {
            children.push_back(new ScalableMeshNode<POINT>(m_node->m_apSubNodes[i]));
            }

    return children;
    }

#ifdef WIP_MESH_IMPORT
template <class POINT> bool ScalableMeshNode<POINT>::_IntersectRay(DPoint3d& pt, const DRay3d& ray, Json::Value& retrievedMetadata)
    {
    bvector<IScalableMeshMeshPtr> meshParts;
    bvector<Utf8String> metadata;
    bvector<bvector<uint8_t>> tex;
    ((SMMeshIndexNode<POINT,Extent3dType>*)m_node.GetPtr())->GetMetadata();
    ((SMMeshIndexNode<POINT,Extent3dType>*)m_node.GetPtr())->GetMeshParts();
    ((SMMeshIndexNode<POINT,Extent3dType>*)m_node.GetPtr())->GetMeshParts(meshParts, metadata, tex);
    volatile bool dbg = false;

    double minParam = DBL_MAX;

    for (auto& part : meshParts)
        {
        if (part.IsValid())
            {
            if(dbg)
                {
                WString str = L"e:\\output\\test_mesh.m";
                part->WriteToFile(str);
                }
            if (part->IntersectRay(pt, ray))
                {
                double param;
                DPoint3d pPt;
                if(ray.ProjectPointUnbounded(pPt, param, pt) && param < minParam)
                    {
                    minParam = param;
                    size_t id = &part - &meshParts[0];
                    Json::Value val;
                    Json::Reader reader;
                    bool parsingSuccessful = reader.parse(metadata[id], val);
                    if (!parsingSuccessful) continue;
                    retrievedMetadata = val;
                    }
                }
            }
        }
    return minParam != DBL_MAX;
    }
#endif

template <class POINT> size_t ScalableMeshNode<POINT>::_GetLevel() const
    {
    LOAD_NODE

    return m_node->m_nodeHeader.m_level;
    }

template <class POINT> DRange3d ScalableMeshNode<POINT>::_GetNodeExtent() const
    {
    LOAD_NODE

    DRange3d range;
    Extent3dType ext = m_node->m_nodeHeader.m_nodeExtent;
    range = DRange3d::From(DPoint3d::From(ExtentOp<Extent3dType>::GetXMin(ext), ExtentOp<Extent3dType>::GetYMin(ext), ExtentOp<Extent3dType>::GetZMin(ext)),
                           DPoint3d::From(ExtentOp<Extent3dType>::GetXMax(ext), ExtentOp<Extent3dType>::GetYMax(ext), ExtentOp<Extent3dType>::GetZMax(ext)));
    return range;
    }

template <class POINT> DRange3d ScalableMeshNode<POINT>::_GetContentExtent() const
    {
    LOAD_NODE

    DRange3d range;
    Extent3dType ext = m_node->m_nodeHeader.m_contentExtent;
    range = DRange3d::From(DPoint3d::From(ExtentOp<Extent3dType>::GetXMin(ext), ExtentOp<Extent3dType>::GetYMin(ext), ExtentOp<Extent3dType>::GetZMin(ext)),
                           DPoint3d::From(ExtentOp<Extent3dType>::GetXMax(ext), ExtentOp<Extent3dType>::GetYMax(ext), ExtentOp<Extent3dType>::GetZMax(ext)));
    return range;
    }

template <class POINT> __int64 ScalableMeshNode<POINT>::_GetNodeId() const
    {
    LOAD_NODE

    return m_node->GetBlockID().m_integerID;
    }

template <class POINT> size_t ScalableMeshNode<POINT>::_GetPointCount() const
    {
    LOAD_NODE

    return m_node->GetNbObjects();

    }  

template <class POINT> bool ScalableMeshNode<POINT>::_IsHeaderLoaded() const
    {            
    return m_node->IsLoaded();
    }

template <class POINT> bool ScalableMeshNode<POINT>::_IsMeshLoaded() const
    {   
    LOAD_NODE
    //NEEDS_WORK_SM : Only good for points, not whole mesh.
    assert(!"Only good for points, not whole mesh.");
    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(m_node->GetPointsPtr(false));
    return pointsPtr.IsValid();    
    }

template <class POINT> void ScalableMeshNode<POINT>::_LoadHeader() const
    {    
    return m_node->Load();    
    }

extern size_t s_nGetDTMs;
extern size_t s_nMissedDTMs;

template <class POINT> BcDTMPtr ScalableMeshNode<POINT>::_GetBcDTM() const
    {
    s_nGetDTMs++;
    auto m_meshNode = dynamic_cast<SMMeshIndexNode<POINT, Extent3dType>*>(m_node.GetPtr());

    if (m_meshNode->GetTileDTM().get() == nullptr || m_meshNode->GetTileDTM()->GetData() == nullptr) return nullptr;
    return *m_meshNode->GetTileDTM()->GetData();
    }

template <class POINT> void ScalableMeshNode<POINT>::_GetSkirtMeshes(bvector<PolyfaceHeaderPtr>& meshes) const
    {
    auto m_meshNode = dynamic_cast<SMMeshIndexNode<POINT, Extent3dType>*>(m_node.GetPtr());
    for (size_t i = 0; i < m_meshNode->m_nbClips; ++i)
        {
        DifferenceSet d = m_meshNode->GetClipSet(i);
        if (d.toggledForID) continue;

        RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(m_node->GetPointsPtr());
        
        PolyfaceHeaderPtr mesh = d.ToPolyfaceMesh(&pointsPtr->operator[](0), pointsPtr->size());
        meshes.push_back(mesh);
        }
    }

template <class POINT> bool ScalableMeshNode<POINT>::_RunQuery(ISMPointIndexQuery<DPoint3d, DRange3d>& query, bvector<IScalableMeshNodePtr>& nodes) const
    {
    std::vector<SMPointIndexNode<DPoint3d, DRange3d>::QueriedNode> nodesP;
    for (auto& node : nodes)
        {
        nodesP.push_back(SMPointIndexNode<DPoint3d, DRange3d>::QueriedNode(dynamic_cast<ScalableMeshNode<POINT>*>(node.get())->m_node, false));
        }
    bool retval = query.Query(m_node, 0, 0, nodesP);

    if (nodesP.size() == 0) return false;
    nodes.clear();
    for (auto& currentNodeP : nodesP)
        {
        IScalableMeshNodePtr nodePtr = new ScalableMeshNode<POINT>(currentNodeP.m_indexNode);
        nodes.push_back(nodePtr);
        }
    return retval;
    }

template <class POINT> bool ScalableMeshNode<POINT>::_RunQuery(ISMPointIndexQuery<DPoint3d, DRange3d>& query) const
    {
    HFCPtr<SMPointIndexNode<DPoint3d, DRange3d>> ptr;
    return query.Query(m_node, 0, 0, ptr) && ptr != nullptr;
    }

template <class POINT> bool ScalableMeshNode<POINT>::_HasClip(uint64_t clip) const
    {
    auto m_meshNode = dynamic_cast<SMMeshIndexNode<POINT, Extent3dType>*>(m_node.GetPtr());
    if (m_meshNode == nullptr) return false;
    if (m_meshNode->m_nbClips == 0) return false;
    else return m_meshNode->HasClip(clip);
    }

template <class POINT> bool ScalableMeshNode<POINT>::_IsClippingUpToDate() const
    {
    auto m_meshNode = dynamic_cast<SMMeshIndexNode<POINT, Extent3dType>*>(m_node.GetPtr());
    if (m_meshNode == nullptr) return true;
    if (m_meshNode->m_nbClips == 0) return true;
    else return m_meshNode->IsClippingUpToDate();
    }

template <class POINT> void ScalableMeshNode<POINT>::_ApplyAllExistingClips() const
    {
       
    _RefreshMergedClip();    
    }

template <class POINT> void ScalableMeshNode<POINT>::_RefreshMergedClip() const
    {
    dynamic_cast<SMMeshIndexNode<POINT, Extent3dType>*>(m_node.GetPtr())->BuildSkirts();
    dynamic_cast<SMMeshIndexNode<POINT, Extent3dType>*>(m_node.GetPtr())->ComputeMergedClips();
    }

template <class POINT> bool ScalableMeshNode<POINT>::_AddClip(uint64_t id, bool isVisible) const
    {
    return dynamic_cast<SMMeshIndexNode<POINT, Extent3dType>*>(m_node.GetPtr())->AddClip(id, isVisible);
    }

template <class POINT> bool ScalableMeshNode<POINT>::_ModifyClip(uint64_t id, bool isVisible) const
    {
    return dynamic_cast<SMMeshIndexNode<POINT, Extent3dType>*>(m_node.GetPtr())->ModifyClip(id,  isVisible);
    }

template <class POINT> bool ScalableMeshNode<POINT>::_DeleteClip(uint64_t id, bool isVisible) const
    {
    return dynamic_cast<SMMeshIndexNode<POINT, Extent3dType>*>(m_node.GetPtr())->DeleteClip(id,isVisible);
    }

template <class POINT> StatusInt ScalableMeshNodeEdit<POINT>::_AddMesh(DPoint3d* vertices, size_t nVertices, int32_t* indices, size_t nIndices)
    {
    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(m_node->GetPointsPtr());    
    pointsPtr->clear();
    auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, Extent3dType>, SMPointIndexNode<POINT, Extent3dType>>(m_node);
    m_meshNode->m_nodeHeader.m_arePoints3d = true;
    vector<POINT> nodePts(nVertices);
    vector<size_t> sorted(nVertices);
    std::iota(sorted.begin(), sorted.end(), 0);
    std::sort(sorted.begin(), sorted.end(), [vertices] (const size_t&i, const size_t&j) {
        if (vertices[i].x < vertices[j].x) return true;
        else if (vertices[i].x == vertices[j].x && vertices[i].y < vertices[j].y) return true;
        else if (vertices[i].x == vertices[j].x && vertices[i].y == vertices[j].y && vertices[i].z < vertices[j].z) return true;
        return false;
        });
    vector<int> indicesVec(nIndices);
    vector<size_t> sortedReverse(nVertices);
    for (size_t pointInd = 0; pointInd < nVertices; pointInd++)
        {
        sortedReverse[sorted[pointInd]] = pointInd;
        nodePts[pointInd].x = vertices[sorted[pointInd]].x;
        nodePts[pointInd].y = vertices[sorted[pointInd]].y;
        nodePts[pointInd].z = vertices[sorted[pointInd]].z;
        }
    memcpy(&indicesVec[0], indices, nIndices*sizeof(int));
    for (int& idx : indicesVec) idx = (int)sortedReverse[idx - 1] + 1;
    pointsPtr->push_back(&nodePts[0], nodePts.size());

    m_meshNode->m_nodeHeader.m_nbFaceIndexes = nIndices;
    bvector<int> componentPointsId;
   // if (NULL == m_meshNode->GetGraphPtr()) m_meshNode->CreateGraph();
    RefCountedPtr<SMMemoryPoolGenericBlobItem<MTGGraph>> graphPtr(m_meshNode->GetGraphPtr());
    MTGGraph* newGraphP = new MTGGraph();
    CreateGraphFromIndexBuffer(newGraphP, (const long*)&indicesVec[0], (int)nIndices, (int)nodePts.size(), componentPointsId, &nodePts[0]);
    graphPtr->SetData(newGraphP);
    graphPtr->SetDirty();


    if (componentPointsId.size() > 0)
        {
        if (m_meshNode->m_nodeHeader.m_meshComponents == nullptr) m_meshNode->m_nodeHeader.m_meshComponents = new int[componentPointsId.size()];
        else if (m_meshNode->m_nodeHeader.m_numberOfMeshComponents != componentPointsId.size())
            {
            delete[] m_meshNode->m_nodeHeader.m_meshComponents;
            m_meshNode->m_nodeHeader.m_meshComponents = new int[componentPointsId.size()];
            }
        m_meshNode->m_nodeHeader.m_numberOfMeshComponents = componentPointsId.size();
        memcpy(m_meshNode->m_nodeHeader.m_meshComponents, componentPointsId.data(), componentPointsId.size()*sizeof(int));
        }

    m_meshNode->PushPtsIndices(&indicesVec[0], m_meshNode->m_nodeHeader.m_nbFaceIndexes);        
    m_meshNode->IncreaseTotalCount(m_meshNode->GetNbPoints());

    m_meshNode->SetDirty(true);
    return BSISUCCESS;
    }

template <class POINT> StatusInt ScalableMeshNodeEdit<POINT>::_AddTexturedMesh(bvector<DPoint3d>& vertices, bvector<bvector<int32_t>>& ptsIndices, bvector<DPoint2d>& uv, bvector<bvector<int32_t>>& uvIndices, size_t nTexture, int64_t texID)
    {
    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(m_node->GetPointsPtr());    
    pointsPtr->clear();
    
    auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, Extent3dType>, SMPointIndexNode<POINT, Extent3dType>>(m_node);
    m_meshNode->m_nodeHeader.m_arePoints3d = true;    
    m_meshNode->m_nodeHeader.m_isTextured = true;

    if (texID != -1)
        m_meshNode->m_nodeHeader.m_textureID = texID;


    size_t nIndicesCount = 0;
    vector<POINT> nodePts(vertices.size());

    for (size_t pointInd = 0; pointInd < vertices.size(); pointInd++)
        {
        nodePts[pointInd].x = vertices[pointInd].x;
        nodePts[pointInd].y = vertices[pointInd].y;
        nodePts[pointInd].z = vertices[pointInd].z;
        }

    pointsPtr->push_back(&nodePts[0], nodePts.size());
    m_meshNode->PushUV(&uv[0], uv.size());

    vector<int32_t> indicesLine;

    // Untexturing part : 
    nIndicesCount += ptsIndices[0].size();
    m_meshNode->PushPtsIndices(&ptsIndices[0][0], ptsIndices[0].size());        
    indicesLine.insert(indicesLine.end(), ptsIndices[0].begin(), ptsIndices[0].end());

    // Texturing points
    for(size_t i = 0; i < nTexture; i++)
        {
        nIndicesCount += ptsIndices[i+1].size();
        m_meshNode->PushPtsIndices(&ptsIndices[i+1][0], ptsIndices[i+1].size());                
        m_meshNode->PushUVsIndices(i, &uvIndices[i][0], uvIndices[i].size());        
        
        indicesLine.insert(indicesLine.end(), ptsIndices[i+1].begin(), ptsIndices[i+1].end());
        }
    
    bvector<int> componentPointsId;
   // if (NULL == m_meshNode->GetGraphPtr()) m_meshNode->CreateGraph();
    RefCountedPtr<SMMemoryPoolGenericBlobItem<MTGGraph>> graphPtr(m_meshNode->GetGraphPtr());
    MTGGraph* newGraphP = new MTGGraph();
    CreateGraphFromIndexBuffer(newGraphP , (const long*)&indicesLine[0], (int)indicesLine.size(), (int)nodePts.size(), componentPointsId, &vertices[0]);

    graphPtr->SetData(newGraphP);
    graphPtr->SetDirty();


    if (componentPointsId.size() > 0)
        {
        if (m_meshNode->m_nodeHeader.m_meshComponents == nullptr) m_meshNode->m_nodeHeader.m_meshComponents = new int[componentPointsId.size()];
        else if (m_meshNode->m_nodeHeader.m_numberOfMeshComponents != componentPointsId.size())
            {
            delete[] m_meshNode->m_nodeHeader.m_meshComponents;
            m_meshNode->m_nodeHeader.m_meshComponents = new int[componentPointsId.size()];
            }
        m_meshNode->m_nodeHeader.m_numberOfMeshComponents = componentPointsId.size();
        memcpy(m_meshNode->m_nodeHeader.m_meshComponents, componentPointsId.data(), componentPointsId.size()*sizeof(int));
        }

    m_meshNode->m_nodeHeader.m_nbFaceIndexes = indicesLine.size();
    m_meshNode->m_nodeHeader.m_nbUvIndexes = uv.size();
    m_meshNode->IncreaseTotalCount(m_meshNode->GetNbPoints());
    
    m_meshNode->SetDirty(true);

    return BSISUCCESS;
    }

template <class POINT> StatusInt ScalableMeshNodeEdit<POINT>::_AddTextures(bvector<Byte>& data, bool sibling)
    {
    assert(m_node->m_nodeHeader.m_isTextured == false);

    auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, Extent3dType>, SMPointIndexNode<POINT, Extent3dType>>(m_node);

    if (m_node->m_nodeHeader.m_isTextured == false)
        {        
        m_meshNode->PushTexture(&data[0], data.size());                
        m_node->m_nodeHeader.m_isTextured = true;

        return BSISUCCESS;
        }

    return BSIERROR;
    }    

template <class POINT> StatusInt ScalableMeshNodeEdit<POINT>::_SetNodeExtent(DRange3d& extent)
    {
    m_node->m_nodeHeader.m_nodeExtent = ExtentOp<Extent3dType>::Create(extent.low.x, extent.low.y, extent.low.z, extent.high.x, extent.high.y, extent.high.z);
    return BSISUCCESS;
    }

template <class POINT> StatusInt ScalableMeshNodeEdit<POINT>::_SetContentExtent(DRange3d& extent)
    {
    m_node->m_nodeHeader.m_contentExtentDefined = true;
    m_node->m_nodeHeader.m_contentExtent = ExtentOp<Extent3dType>::Create(extent.low.x, extent.low.y, extent.low.z, extent.high.x, extent.high.y, extent.high.z);
    return BSISUCCESS;
    }

template <class POINT> StatusInt ScalableMeshNodeEdit<POINT>::_SetArePoints3d(bool arePoints3d)
    {
    m_node->m_nodeHeader.m_arePoints3d = arePoints3d;
    return BSISUCCESS;
    }

template <class POINT> ScalableMeshNodeEdit<POINT>::ScalableMeshNodeEdit(HFCPtr<SMPointIndexNode<POINT, Extent3dType>>& nodePtr)
    {
    if (!nodePtr->IsLoaded())
        nodePtr->Load();
    m_node = nodePtr;
    }


template <class POINT> bool ScalableMeshNodeEdit<POINT>::_IsHeaderLoaded() const
    {    
    return m_node->IsLoaded();
    }

template <class POINT> bool ScalableMeshNodeEdit<POINT>::_IsMeshLoaded() const
    {        
    //NEEDS_WORK_SM : Only good for points, not whole mesh.
    assert(!"Only good for points, not whole mesh.");
    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(m_node->GetPointsPtr(false));
    return pointsPtr.IsValid();        
    }

template <class POINT> void ScalableMeshNodeEdit<POINT>::_LoadHeader() const
    {    
    return m_node->Load();    
    }


template <class POINT> ScalableMeshNodeWithReprojection<POINT>::ScalableMeshNodeWithReprojection(HFCPtr<SMPointIndexNode<POINT, Extent3dType>>& nodePtr, const GeoCoords::Reprojection& reproject)
    :ScalableMeshNode<POINT>(nodePtr), m_reprojectFunction(reproject)
    {
    }

template <class POINT> ScalableMeshNodeWithReprojection<POINT>::ScalableMeshNodeWithReprojection(IScalableMeshNodePtr nodeInfo, const GeoCoords::Reprojection& reproject)
    : m_reprojectFunction(reproject)
    {
    auto nodeInfoImpl = dynamic_cast<ScalableMeshNode<POINT> *>(nodeInfo.get());
    m_node = nodeInfoImpl->GetNodePtr();
    }


template <class POINT> IScalableMeshMeshPtr ScalableMeshNodeWithReprojection<POINT>::_GetMesh(IScalableMeshMeshFlagsPtr& flags) const
    {
    IScalableMeshMeshPtr meshP;
    if (flags->ShouldLoadGraph())
        {
        auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, Extent3dType>, SMPointIndexNode<POINT, Extent3dType>>(m_node);
        if (m_meshNode->GetGraphPtr() == NULL)
            {
            m_meshNode->LoadGraph();
            }

        RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(m_node->GetPointsPtr());

        POINT* pts = new POINT[pointsPtr->size()];
        pointsPtr->get(pts, pointsPtr->size());

        DPoint3d* points = new DPoint3d[pointsPtr->size()];
        m_reprojectFunction.Reproject(points, pointsPtr->size(), points);

        PtToPtConverter::Transform(points, &(*pointsPtr)[0], pointsPtr->size());
        RefCountedPtr<SMMemoryPoolGenericBlobItem<MTGGraph>> graphPtr(m_meshNode->GetGraphPtr());
        meshP = ScalableMeshMeshWithGraph::Create(pointsPtr->size(), points, m_node->m_nodeHeader.m_nbFaceIndexes, (int32_t*)(&pts[0] + pointsPtr->size()), 0, nullptr, nullptr, graphPtr->EditData(), ArePoints3d(), 0, 0, 0);
        delete pts;
        delete points;
        }
    else
        {
        auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, Extent3dType>, SMPointIndexNode<POINT, Extent3dType>>(m_node);
        ScalableMeshMeshPtr meshPtr = ScalableMeshMesh::Create();

        RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(m_node->GetPointsPtr());

        vector<DPoint3d> dataPoints(pointsPtr->size());

        PtToPtConverter converter;

        for (size_t pointInd = 0; pointInd < pointsPtr->size(); pointInd++)
            {
            dataPoints[pointInd] = converter.operator()(pointsPtr->operator[](pointInd));
            }
        m_reprojectFunction.Reproject(&dataPoints[0], dataPoints.size(), &dataPoints[0]);

        RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> ptsIndices(m_meshNode->GetPtsIndicePtr());

        int status = meshPtr->AppendMesh(dataPoints.size(), &dataPoints[0], m_node->m_nodeHeader.m_nbFaceIndexes, &(*ptsIndices)[0], 0, 0, 0, 0, 0, 0);

        assert(status == SUCCESS);

        meshP = meshPtr.get();
        }

    return meshP;
    }

template <class POINT> IScalableMeshMeshPtr ScalableMeshNodeWithReprojection<POINT>::_GetMeshByParts(const bset<uint64_t>& clipsToShow) const
    {
    auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, Extent3dType>, SMPointIndexNode<POINT, Extent3dType>>(m_node);
    ScalableMeshMeshPtr meshPtr = ScalableMeshMesh::Create();

    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(m_node->GetPointsPtr());
    vector<DPoint3d> dataPoints(pointsPtr->size());

    PtToPtConverter converter;

    for (size_t pointInd = 0; pointInd < pointsPtr->size(); pointInd++)
        {
        dataPoints[pointInd] = converter.operator()(pointsPtr->operator[](pointInd));
        }
    m_reprojectFunction.Reproject(&dataPoints[0], pointsPtr->size(), &dataPoints[0]);
    
    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> ptsIndices = m_meshNode->GetPtsIndicePtr();
    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> uvIndexes = m_meshNode->GetUVsIndicesPtr();
    RefCountedPtr<SMMemoryPoolVectorItem<DPoint2d>> uvCoords = m_meshNode->GetUVCoordsPtr();

    int status = meshPtr->AppendMesh(pointsPtr->size(), &dataPoints[0], m_node->m_nodeHeader.m_nbFaceIndexes, &(*ptsIndices)[0], 0, 0, 0, uvCoords->size(), &(*uvCoords)[0], &(*uvIndexes)[0]);

    assert(status == SUCCESS);

    return meshPtr.get();
    }

template <class POINT> int ScalableMeshNodeRayQuery<POINT>::_Query(IScalableMeshNodePtr&                               nodePtr,
                                                                   const DPoint3d*                           pTestPt,
                                                            const DPoint3d*                              pClipShapePts,
                                                            int                                          nbClipShapePts,
                                                            const IScalableMeshNodeQueryParamsPtr& scmQueryParamsPtr) const
    {
    ScalableMeshNodeRayQueryParams* params = (ScalableMeshNodeRayQueryParams*)scmQueryParamsPtr.get();
    DRay3d ray = DRay3d::FromOriginAndVector(*pTestPt, params->GetDirection());
 
    HFCPtr<SMPointIndexNode<POINT, Extent3dType>> currentNodeP(nullptr);
    ScalableMeshQuadTreeLevelIntersectIndexQuery<POINT, Extent3dType> query(m_scmIndexPtr->GetContentExtent(), 
                                                                                 scmQueryParamsPtr->GetLevel() == (size_t)-1 ? m_scmIndexPtr->GetDepth() : scmQueryParamsPtr->GetLevel(), 
                                                                                 ray, 
                                                                                 params->Get2d(), 
                                                                                 params->GetDepth(), 
                                                                                 params->GetUseUnboundedRay(),
                                                                                 params->Get2d() ? ScalableMeshQuadTreeLevelIntersectIndexQuery<POINT, Extent3dType>::RaycastOptions::FIRST_INTERSECT : ScalableMeshQuadTreeLevelIntersectIndexQuery<POINT, Extent3dType>::RaycastOptions::LAST_INTERSECT);
    m_scmIndexPtr->Query(&query, currentNodeP);
    if (currentNodeP == nullptr) return ERROR;
    nodePtr = IScalableMeshNodePtr(new ScalableMeshNode<POINT>(currentNodeP));
    return SUCCESS;
    }

template <class POINT> int ScalableMeshNodeRayQuery<POINT>::_Query(bvector<IScalableMeshNodePtr>&                               nodesPtr,
                                                                   const DPoint3d*                           pTestPt,
                                                                   const DPoint3d*                              pClipShapePts,
                                                                   int                                          nbClipShapePts,
                                                                   const IScalableMeshNodeQueryParamsPtr& scmQueryParamsPtr) const
    {
    ScalableMeshNodeRayQueryParams* params = (ScalableMeshNodeRayQueryParams*)scmQueryParamsPtr.get();
    DRay3d ray = DRay3d::FromOriginAndVector(*pTestPt, params->GetDirection());

    std::vector<SMPointIndexNode<POINT, Extent3dType>::QueriedNode> nodesP;
    ScalableMeshQuadTreeLevelIntersectIndexQuery<POINT, Extent3dType> query(m_scmIndexPtr->GetContentExtent(), 
                                                                                 scmQueryParamsPtr->GetLevel() == (size_t)-1 ? m_scmIndexPtr->GetDepth() : scmQueryParamsPtr->GetLevel(),
                                                                                 ray, 
                                                                                 params->Get2d(), 
                                                                                 params->GetDepth(), 
                                                                                 params->GetUseUnboundedRay(),
                                                                                 ScalableMeshQuadTreeLevelIntersectIndexQuery<POINT, Extent3dType>::RaycastOptions::ALL_INTERSECT);
    m_scmIndexPtr->Query(&query, nodesP);

    if (nodesP.size() == 0) return ERROR;
    for (auto& currentNodeP : nodesP)
        {
        IScalableMeshNodePtr nodePtr = new ScalableMeshNode<POINT>(currentNodeP.m_indexNode);
        nodesPtr.push_back(nodePtr);
        }
    return SUCCESS;
    }

template <class POINT> int ScalableMeshNodePlaneQuery<POINT>::_Query(bvector<IScalableMeshNodePtr>&                       meshNodesPtr,
                                                                   const DPoint3d*                              pQueryExtentPts,
                                                                   int                                          nbQueryExtentPts,
                                                                   const IScalableMeshMeshQueryParamsPtr& scmQueryParamsPtr) const
    {
    ScalableMeshNodePlaneQueryParams* params = (ScalableMeshNodePlaneQueryParams*)scmQueryParamsPtr.get();

    vector<typename SMPointIndexNode<POINT, Extent3dType>::QueriedNode> nodes;
    ScalableMeshQuadTreeLevelPlaneIntersectIndexQuery<POINT, Extent3dType> query(m_scmIndexPtr->GetContentExtent(), m_scmIndexPtr->GetDepth(), params->GetPlane(), params->GetDepth());
    m_scmIndexPtr->Query(&query, nodes);
    if (0 == nodes.size()) return ERROR;
    for (auto& node : nodes)
        {
        meshNodesPtr.push_back(new ScalableMeshNode<POINT>(node.m_indexNode));
        }
    return SUCCESS;
    }

template <class POINT> int ScalableMeshNodePlaneQuery<POINT>::_Query(IScalableMeshMeshPtr&                                meshPtr,
                                                                     const DPoint3d*                               pQueryExtentPts,
                                                                     int                                           nbQueryExtentPts,
                                                                     const IScalableMeshMeshQueryParamsPtr&  scmQueryParamsPtr) const
    {
    assert(!"Not supported");
    return ERROR;
    }