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
template <class POINT> ScalableMeshFullResolutionPointQuery<POINT>::ScalableMeshFullResolutionPointQuery(const HFCPtr<SMPointIndex<POINT, YProtPtExtentType>>& pointIndexPtr, 
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

    YProtPtExtentType contentExtent(m_scmIndexPtr->GetContentExtent());        
    YProtPtExtentType queryExtent;
    
    HFCPtr<HVEShape> clipShapePtr;
    HAutoPtr<ISMPointIndexQuery<POINT, YProtPtExtentType>> pointQueryP;

    assert(dynamic_cast<IScalableMeshFullResolutionQueryParams*>(scmQueryParamsPtr.get()) != 0);
    IScalableMeshFullResolutionQueryParams* queryParams(dynamic_cast<IScalableMeshFullResolutionQueryParams*>(scmQueryParamsPtr.get()));        

    if (pQueryShapePts != 0)
        {    
        queryExtent = GetExtentFromClipShape<YProtPtExtentType>(pQueryShapePts, nbQueryShapePts, 
                                                                ExtentOp<YProtPtExtentType>::GetZMin(contentExtent),
                                                                ExtentOp<YProtPtExtentType>::GetZMax(contentExtent));
                    
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

        HFCPtr<HVE2DShape> pContentShape(new HVE2DRectangle(ExtentOp<YProtPtExtentType>::GetXMin(contentExtent), ExtentOp<YProtPtExtentType>::GetYMin(contentExtent), 
                                                            ExtentOp<YProtPtExtentType>::GetXMax(contentExtent), ExtentOp<YProtPtExtentType>::GetYMax(contentExtent), pCoordSys));

        HFCPtr<HVEShape> pQueryShape = new HVEShape(*(pPolygon->IntersectShape(*pContentShape)));
        clipShapePtr = CreateClipShape(pQueryShape);  

        if (clipShapePtr->IsEmpty()) 
            clipShapePtr = pQueryShape;                
        }
    else
        {             
        /*queryExtent.xMin = ExtentOp<YProtPtExtentType>::GetXMin(contentExtent);
        queryExtent.xMax = ExtentOp<YProtPtExtentType>::GetXMax(contentExtent);
        queryExtent.yMin = ExtentOp<YProtPtExtentType>::GetYMin(contentExtent);
        queryExtent.yMax = ExtentOp<YProtPtExtentType>::GetYMax(contentExtent);
        queryExtent.zMin = ExtentOp<YProtPtExtentType>::GetZMin(contentExtent);
        queryExtent.zMax = ExtentOp<YProtPtExtentType>::GetZMax(contentExtent);*/
        queryExtent = contentExtent;
        
        DRange3d spatialIndexRange;
        spatialIndexRange.low.x = ExtentOp<YProtPtExtentType>::GetXMin(queryExtent);
        spatialIndexRange.high.x = ExtentOp<YProtPtExtentType>::GetXMax(queryExtent);
        spatialIndexRange.low.y = ExtentOp<YProtPtExtentType>::GetYMin(queryExtent);
        spatialIndexRange.high.y = ExtentOp<YProtPtExtentType>::GetYMax(queryExtent);              

        clipShapePtr = CreateClipShape(spatialIndexRange);                             
        }             
   
    if ((clipShapePtr != 0) && (clipShapePtr->IsEmpty() == false) && !clipShapePtr->IsRectangle())
        {         
        HFCPtr<HVE2DShape> clipShape = static_cast<HVE2DShape*>(clipShapePtr->GetShapePtr()->Clone());
        pointQueryP = new HGFLevelPointIndexByShapeQuery<POINT, YProtPtExtentType>(clipShape, m_resolutionIndex, queryParams->GetReturnAllPtsForLowestLevel(), queryParams->GetMaximumNumberOfPoints());
        }
    else
        {                       
        pointQueryP = new HGFLevelPointIndexQuery<POINT, YProtPtExtentType>(queryExtent, m_resolutionIndex, queryParams->GetReturnAllPtsForLowestLevel(), queryParams->GetMaximumNumberOfPoints());
        }      
        
    try   
        {           
    #ifdef ACTIVATE_NODE_QUERY_TRACING            
        //pointQueryP->SetTracingXMLFileName(AString("E:\\MyDoc\\SS3 - Iteration 1\\ScalableMesh\\New 3D Query\\Log\\CheckedNodesDuringLastQuery.xml"));                 
    #endif                          

        m_scmIndexPtr->Query(pointQueryP.get(), pointList);

        status = S_SUCCESS;
        
        if (queryParams->GetMaximumNumberOfPoints() < pointList.size())
            {
            status = S_NBPTSEXCEEDMAX; 
            }        
        }
    catch (...)
        {
        status = S_ERROR;
        }
       
    if (status == S_SUCCESS)
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
template <class POINT> ScalableMeshViewDependentPointQuery<POINT>::ScalableMeshViewDependentPointQuery(const HFCPtr<SMPointIndex<POINT, YProtPtExtentType>>& pointIndexPtr)
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

    YProtPtExtentType contentExtent(m_scmIndexPtr->GetContentExtent());    

    double minZ = ExtentOp<YProtPtExtentType>::GetZMin(contentExtent);
    double maxZ = ExtentOp<YProtPtExtentType>::GetZMax(contentExtent);
    
    YProtPtExtentType queryExtent(GetExtentFromClipShape<YProtPtExtentType>(pQueryShapePts, 
                                                                            nbQueryShapePts, 
                                                                            minZ,
                                                                            maxZ));

    HAutoPtr<ISMPointIndexQuery<POINT, YProtPtExtentType>> pointQueryP;

    if (m_resolutionIndex != INT_MAX)
        {
        assert(dynamic_cast<ISrDTMViewDependentQueryParams*>(scmQueryParamsPtr.get()) != 0);
        assert((scmQueryParamsPtr->GetSourceGCS() == 0) && (scmQueryParamsPtr->GetTargetGCS() == 0));

        ISrDTMViewDependentQueryParams* queryParams(dynamic_cast<ISrDTMViewDependentQueryParams*>(scmQueryParamsPtr.get()));        

        pointQueryP = new ScalableMeshQuadTreeLevelPointIndexQuery<POINT, YProtPtExtentType>(queryExtent, 
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
                        
        ScalableMeshQuadTreeViewDependentPointQuery<POINT, YProtPtExtentType>* viewDependentQueryP(new ScalableMeshQuadTreeViewDependentPointQuery<POINT, YProtPtExtentType>
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

    YProtPtExtentType ExtentPoints = m_scmIndexPtr->GetContentExtent();
    spatialIndexRange.low.x = ExtentOp<YProtPtExtentType>::GetXMin(ExtentPoints);
    spatialIndexRange.high.x = ExtentOp<YProtPtExtentType>::GetXMax(ExtentPoints);
    spatialIndexRange.low.y = ExtentOp<YProtPtExtentType>::GetYMin(ExtentPoints);
    spatialIndexRange.high.y = ExtentOp<YProtPtExtentType>::GetYMax(ExtentPoints);               

    HFCPtr<HVEShape> clipShapePtr = CreateClipShape(spatialIndexRange);

    if ((clipShapePtr != 0) && (clipShapePtr->IsEmpty() == false))
        {
        ISMPointIndexSpatialLimitWrapQuery<POINT, YProtPtExtentType>* wrappedShapedQuery;
        HFCPtr<HVE2DShape> clipShape = static_cast<HVE2DShape*>(clipShapePtr->GetShapePtr()->Clone());
        wrappedShapedQuery = new ISMPointIndexSpatialLimitWrapQuery<POINT, YProtPtExtentType>(clipShape, pointQueryP.release());

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
template <class POINT> ScalableMeshFixResolutionViewPointQuery<POINT>::ScalableMeshFixResolutionViewPointQuery(const HFCPtr<SMPointIndex<POINT, YProtPtExtentType>>& pointIndexPtr, 
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
template <class POINT> ScalableMeshViewDependentMeshQuery<POINT>::ScalableMeshViewDependentMeshQuery(const HFCPtr<SMPointIndex<POINT, YProtPtExtentType>>& pointIndexPtr)
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

    YProtPtExtentType contentExtent(m_scmIndexPtr->GetContentExtent());    

    double minZ = ExtentOp<YProtPtExtentType>::GetZMin(contentExtent);
    double maxZ = ExtentOp<YProtPtExtentType>::GetZMax(contentExtent);
        
    YProtPtExtentType queryExtent(ScalableMeshPointQuery::GetExtentFromClipShape<YProtPtExtentType>(pQueryExtentPts,
                                                                                        nbQueryExtentPts,
                                                                                        minZ,
                                                                                        maxZ));

    HAutoPtr<ISMPointIndexQuery<POINT, YProtPtExtentType>> pointQueryP;
             
    //MS Need to be removed
    double viewportRotMatrix[3][3];        
    double rootToViewMatrix[4][4];        

    static size_t maxNbPoints = 200000;
    IScalableMeshViewDependentMeshQueryParamsPtr scmViewDependentParamsPtr = IScalableMeshViewDependentMeshQueryParamsPtr(dynamic_cast<ScalableMeshViewDependentMeshQueryParams*>(scmQueryParamsPtr.get()));

    memcpy(rootToViewMatrix, scmViewDependentParamsPtr->GetRootToViewMatrix(), sizeof(double) * 4 * 4);    
                    
    ScalableMeshQuadTreeViewDependentMeshQuery<POINT, YProtPtExtentType>* viewDependentQueryP(new ScalableMeshQuadTreeViewDependentMeshQuery<POINT, YProtPtExtentType>
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

    YProtPtExtentType ExtentPoints = m_scmIndexPtr->GetContentExtent();
    spatialIndexRange.low.x = ExtentOp<YProtPtExtentType>::GetXMin(ExtentPoints);
    spatialIndexRange.high.x = ExtentOp<YProtPtExtentType>::GetXMax(ExtentPoints);
    spatialIndexRange.low.y = ExtentOp<YProtPtExtentType>::GetYMin(ExtentPoints);
    spatialIndexRange.high.y = ExtentOp<YProtPtExtentType>::GetYMax(ExtentPoints);               

    IScalableMeshClipContainerPtr clips;

    HFCPtr<HVEShape> clipShapePtr = CreateShapeFromClips(spatialIndexRange, clips);
       
    if ((clipShapePtr != 0) && (clipShapePtr->IsEmpty() == false))
        {
        //NEEDS_WORK_SM 
        assert(!"Not implemented yet");
    /*
        ISMPointIndexSpatialLimitWrapQuery<POINT, YProtPtExtentType>* wrappedShapedQuery;
        HFCPtr<HVE2DShape> clipShape = static_cast<HVE2DShape*>(clipShapePtr->GetShapePtr()->Clone());
        wrappedShapedQuery = new ISMPointIndexSpatialLimitWrapQuery<POINT, YProtPtExtentType>(clipShape, pointQueryP.release());

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

    YProtPtExtentType contentExtent(m_scmIndexPtr->GetContentExtent());    

    double minZ = ExtentOp<YProtPtExtentType>::GetZMin(contentExtent);
    double maxZ = ExtentOp<YProtPtExtentType>::GetZMax(contentExtent);
        
    YProtPtExtentType queryExtent(ScalableMeshPointQuery::GetExtentFromClipShape<YProtPtExtentType>(pQueryExtentPts,
                                                                                               nbQueryExtentPts,
                                                                                               minZ,
                                                                                               maxZ));

    HAutoPtr<ISMPointIndexQuery<POINT, YProtPtExtentType>> pointQueryP;
             
    //MS Need to be removed
    double viewportRotMatrix[3][3];        
    double rootToViewMatrix[4][4];        

    IScalableMeshViewDependentMeshQueryParamsPtr scmViewDependentParamsPtr = IScalableMeshViewDependentMeshQueryParamsPtr(dynamic_cast<ScalableMeshViewDependentMeshQueryParams*>(scmQueryParamsPtr.get()));

    memcpy(rootToViewMatrix, scmViewDependentParamsPtr->GetRootToViewMatrix(), sizeof(double) * 4 * 4);

    //NEEDS_WORK_SM : To be removed all!
    static size_t maxNbPoints = 150000000000;
                    
    ScalableMeshQuadTreeViewDependentMeshQuery<POINT, YProtPtExtentType>* viewDependentQueryP(new ScalableMeshQuadTreeViewDependentMeshQuery<POINT, YProtPtExtentType>
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

    YProtPtExtentType ExtentPoints = m_scmIndexPtr->GetContentExtent();
    spatialIndexRange.low.x = ExtentOp<YProtPtExtentType>::GetXMin(ExtentPoints);
    spatialIndexRange.high.x = ExtentOp<YProtPtExtentType>::GetXMax(ExtentPoints);
    spatialIndexRange.low.y = ExtentOp<YProtPtExtentType>::GetYMin(ExtentPoints);
    spatialIndexRange.high.y = ExtentOp<YProtPtExtentType>::GetYMax(ExtentPoints);               

    IScalableMeshClipContainerPtr clips;

    HFCPtr<HVEShape> clipShapePtr = CreateShapeFromClips(spatialIndexRange, clips);
       
    if ((clipShapePtr != 0) && (clipShapePtr->IsEmpty() == false))
        {
        //NEEDS_WORK_SM 
        assert(!"Not implemented yet");
    /*
        ISMPointIndexSpatialLimitWrapQuery<POINT, YProtPtExtentType>* wrappedShapedQuery;
        HFCPtr<HVE2DShape> clipShape = static_cast<HVE2DShape*>(clipShapePtr->GetShapePtr()->Clone());
        wrappedShapedQuery = new ISMPointIndexSpatialLimitWrapQuery<POINT, YProtPtExtentType>(clipShape, pointQueryP.release());

        pointQueryP = wrappedShapedQuery;
        */
        }  
            
    try   
        {           
    #ifdef ACTIVATE_NODE_QUERY_TRACING            
        //queryObject.SetTracingXMLFileName(AString("E:\\MyDoc\\SS3 - Iteration 1\\ScalableMesh\\New 3D Query\\Log\\CheckedNodesDuringLastQuery.xml"));                 
    #endif                          
                
        //ScalableMeshMeshPtr returnMeshPtr (ScalableMeshMesh::Create (DVec3d::From (rootToViewMatrix[2][0], rootToViewMatrix[2][1], rootToViewMatrix[2][2])));
                    
        vector<typename SMPointIndexNode<POINT, YProtPtExtentType>::QueriedNode> returnedMeshNodes;
        bool isComplete = true;
        
        if (m_scmIndexPtr->Query(pointQueryP.get(), returnedMeshNodes, &isComplete,
                                      scmViewDependentParamsPtr->IsProgressiveDisplay(), false,
                                      scmViewDependentParamsPtr->GetStopQueryCallback()))
            {        
            bool removeOverview = false;

            if (isComplete)
                {
                bool someNodeDiscarded = false;

                for (auto& nodes : returnedMeshNodes)
                    {
                    if (nodes.m_indexNode->Discarded())
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

            status = S_SUCCESS;
            }
        else
            {
            status = S_ERROR;
            }

        if (!isComplete)
            {
            status = S_SUCCESS_INCOMPLETE;
            }        
        }
    catch (...)
        {
        status = S_ERROR;
        }

    return status;    
    }

template <class POINT> ScalableMeshFullResolutionMeshQuery<POINT>::ScalableMeshFullResolutionMeshQuery(const HFCPtr<SMPointIndex<POINT, YProtPtExtentType>>& pointIndexPtr)
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
    YProtPtExtentType contentExtent(m_scmIndexPtr->GetContentExtent());

    double minZ = ExtentOp<YProtPtExtentType>::GetZMin(contentExtent);
    double maxZ = ExtentOp<YProtPtExtentType>::GetZMax(contentExtent);

    YProtPtExtentType queryExtent(ScalableMeshPointQuery::GetExtentFromClipShape<YProtPtExtentType>(pQueryExtentPts,
        nbQueryExtentPts,
        minZ,
        maxZ));
    int status = SUCCESS;
    static size_t maxNbPoints = 150000000000;
    DPoint3d box[8] = { DPoint3d::From(ExtentOp<YProtPtExtentType>::GetXMin(queryExtent), ExtentOp<YProtPtExtentType>::GetYMin(queryExtent), ExtentOp<YProtPtExtentType>::GetZMin(queryExtent)),
        DPoint3d::From(ExtentOp<YProtPtExtentType>::GetXMax(queryExtent), ExtentOp<YProtPtExtentType>::GetYMin(queryExtent), ExtentOp<YProtPtExtentType>::GetZMin(queryExtent)),
        DPoint3d::From(ExtentOp<YProtPtExtentType>::GetXMin(queryExtent), ExtentOp<YProtPtExtentType>::GetYMax(queryExtent), ExtentOp<YProtPtExtentType>::GetZMin(queryExtent)),
        DPoint3d::From(ExtentOp<YProtPtExtentType>::GetXMin(queryExtent), ExtentOp<YProtPtExtentType>::GetYMin(queryExtent), ExtentOp<YProtPtExtentType>::GetZMax(queryExtent)),
        DPoint3d::From(ExtentOp<YProtPtExtentType>::GetXMax(queryExtent), ExtentOp<YProtPtExtentType>::GetYMax(queryExtent), ExtentOp<YProtPtExtentType>::GetZMin(queryExtent)),
        DPoint3d::From(ExtentOp<YProtPtExtentType>::GetXMax(queryExtent), ExtentOp<YProtPtExtentType>::GetYMin(queryExtent), ExtentOp<YProtPtExtentType>::GetZMax(queryExtent)),
        DPoint3d::From(ExtentOp<YProtPtExtentType>::GetXMin(queryExtent), ExtentOp<YProtPtExtentType>::GetYMax(queryExtent), ExtentOp<YProtPtExtentType>::GetZMax(queryExtent)),
        DPoint3d::From(ExtentOp<YProtPtExtentType>::GetXMax(queryExtent), ExtentOp<YProtPtExtentType>::GetYMax(queryExtent), ExtentOp<YProtPtExtentType>::GetZMax(queryExtent)) };
    ScalableMeshQuadTreeLevelMeshIndexQuery<POINT, YProtPtExtentType>* meshQueryP(new ScalableMeshQuadTreeLevelMeshIndexQuery<POINT, YProtPtExtentType>(queryExtent, m_scmIndexPtr->GetDepth(), box));

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
    YProtPtExtentType contentExtent(m_scmIndexPtr->GetContentExtent());

    double minZ = ExtentOp<YProtPtExtentType>::GetZMin(contentExtent);
    double maxZ = ExtentOp<YProtPtExtentType>::GetZMax(contentExtent);

    YProtPtExtentType queryExtent(ScalableMeshPointQuery::GetExtentFromClipShape<YProtPtExtentType>(pQueryExtentPts,
        nbQueryExtentPts,
        minZ,
        maxZ));
    if (nbQueryExtentPts == 0) queryExtent = contentExtent;
    int status = SUCCESS;
    
    DRange3d range; 
    DPoint3d box[8];

    range.low.x = ExtentOp<YProtPtExtentType>::GetXMin(queryExtent);
    range.low.y = ExtentOp<YProtPtExtentType>::GetYMin(queryExtent);
    range.low.z = ExtentOp<YProtPtExtentType>::GetZMin(queryExtent);
    range.high.x = ExtentOp<YProtPtExtentType>::GetXMax(queryExtent);
    range.high.y = ExtentOp<YProtPtExtentType>::GetYMax(queryExtent);
    range.high.z = ExtentOp<YProtPtExtentType>::GetZMax(queryExtent);

    range.Get8Corners (box);

    ScalableMeshQuadTreeLevelMeshIndexQuery<POINT, YProtPtExtentType>* meshQueryP(new ScalableMeshQuadTreeLevelMeshIndexQuery<POINT, YProtPtExtentType>(queryExtent, scmQueryParamsPtr->GetLevel() < (size_t)-1 ? scmQueryParamsPtr->GetLevel() : m_scmIndexPtr->GetDepth(), box));
    try
        {

        vector<typename SMPointIndexNode<POINT, YProtPtExtentType>::QueriedNode> returnedMeshNodes;

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
                                                                                     const HFCPtr<SMMeshIndex<POINT, YProtPtExtentType>>& indexPtr,
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

    ReprojectionFactory::Status reprojCreateStatus;
    m_targetToSourceReproj = REPROJECTION_FACTORY.Create(m_targetGCS, m_sourceGCS, 0, reprojCreateStatus);
    assert(Reprojection::S_SUCCESS == reprojCreateStatus);

    m_sourceToTargetReproj = REPROJECTION_FACTORY.Create(m_sourceGCS, m_targetGCS, 0, reprojCreateStatus);
    assert(Reprojection::S_SUCCESS == reprojCreateStatus);
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

template <class POINT> ScalableMeshNodeRayQuery<POINT>::ScalableMeshNodeRayQuery(const HFCPtr<SMPointIndex<POINT, YProtPtExtentType>>& pointIndexPtr)
    {
    m_scmIndexPtr = pointIndexPtr;
    }

template <class POINT> ScalableMeshNodeRayQuery<POINT>::~ScalableMeshNodeRayQuery()
    {
    }

template <class POINT> ScalableMeshNodePlaneQuery<POINT>::ScalableMeshNodePlaneQuery(const HFCPtr<SMPointIndex<POINT, YProtPtExtentType>>& pointIndexPtr)
    {
    m_scmIndexPtr = pointIndexPtr;
    }

template <class POINT> ScalableMeshNodePlaneQuery<POINT>::~ScalableMeshNodePlaneQuery()
    {}

#define LOAD_NODE \
    if (!m_node->IsLoaded()) \
        m_node->Load(); \


template <class POINT> ScalableMeshNode<POINT>::ScalableMeshNode(HFCPtr<SMPointIndexNode<POINT, YProtPtExtentType>>& nodePtr)
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

template <class POINT> IScalableMeshMeshPtr ScalableMeshNode<POINT>::_GetMesh(IScalableMeshMeshFlagsPtr& flags, bvector<bool>& clipsToShow) const
        {
        LOAD_NODE

//        auto isStreaming = s_useStreamingStore;        

//        if(isStreaming) s_streamingMutex.lock();

    m_node->Pin();
    auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, YProtPtExtentType>, SMPointIndexNode<POINT, YProtPtExtentType>>(m_node);

    IScalableMeshMeshPtr meshP;
    if (flags->ShouldLoadGraph())
        {
        m_meshNode->PinGraph();
#ifdef SCALABLE_MESH_ATP
        int64_t loadAttempts;
        int64_t loadMisses;
        IScalableMeshATP::GetInt(L"nOfGraphLoadAttempts", loadAttempts);
        IScalableMeshATP::GetInt(L"nOfGraphStoreMisses", loadMisses);
        loadAttempts++;
#endif
        if (m_meshNode->GetGraphPtr() == NULL)
            {
#ifdef SCALABLE_MESH_ATP
            loadMisses++;
#endif
            m_meshNode->LoadGraph();
            }
#ifdef SCALABLE_MESH_ATP
        IScalableMeshATP::StoreInt(L"nOfGraphLoadAttempts", loadAttempts);
        IScalableMeshATP::StoreInt(L"nOfGraphStoreMisses", loadMisses);
#endif
        /*vector<DPoint3d> dataPoints(m_node->size());

        PtToPtConverter converter;
        
        for (size_t pointInd = 0; pointInd < m_node->size(); pointInd++)
            {
            dataPoints[pointInd] = converter.operator()(m_node->operator[](pointInd));
            }*/
        ScalableMeshMeshWithGraphPtr meshPtr = ScalableMeshMeshWithGraph::Create(m_meshNode->GetGraphPtr(), ArePoints3d());
        //int status = meshPtr->AppendMesh(m_node->size(), &dataPoints[0], m_node->m_nodeHeader.m_nbFaceIndexes, (int32_t*)&m_node->operator[](m_node->size()), 0, 0, 0);
        // NEEDS_WORK_SM : texture logique !
/*        std::ofstream file_s;
        file_s.open("C:\\dev\\ContextCapture\\_log.txt", ios_base::app);
        file_s << "PushIndices etc... -- shit 10" << endl;*/

        RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> ptIndices(m_meshNode->GetPtsIndicePtr());
        
        int status = meshPtr->AppendMesh(m_node->size(), const_cast<DPoint3d*>(&m_node->operator[](0)), ptIndices->size(), &(*ptIndices)[0], 0, 0, 0, 0, 0, 0);
        assert(status == SUCCESS);
        meshP = meshPtr.get();
        m_meshNode->ReleaseGraph();
        }
    else
        {        
        if (flags->ShouldLoadTexture())
            {
            m_meshNode->PinUV();
            m_meshNode->PinUVsIndices();
            }
        //NEEDS_WORK_SM_PROGRESSIF : Node header loaded unexpectingly
        if (m_node->size() > 0)
            {
            //auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, YProtPtExtentType>, SMPointIndexNode<POINT, YProtPtExtentType>>(m_node);            
            ScalableMeshMeshPtr meshPtr = ScalableMeshMesh::Create();
        
            vector<DPoint3d> dataPoints(m_node->size());
            m_node->get(&dataPoints[0], dataPoints.size());
            /*PtToPtConverter converter; 

            for (size_t pointInd = 0; pointInd < m_node->size(); pointInd++)
                {
                dataPoints[pointInd] = converter.operator()(m_node->operator[](pointInd));                                            
                }*/

            int status = meshPtr->AppendMesh(m_node->size(), &dataPoints[0],0,0, 0, 0, 0, 0, 0,0);
            if (flags->ShouldLoadTexture())
                {
                m_meshNode->PinUV();
                }
            for (size_t i = 0; i < m_meshNode->GetNbOfTextures() + 1; ++i)
                { 
                RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> faceIndexes(m_meshNode->GetPtsIndicePtr());

                if (faceIndexes->size() == 0 || (i == 0 && m_meshNode->GetNbOfTextures() > 0)) continue;
                                
                DPoint2d* pUv = flags->ShouldLoadTexture() ? m_meshNode->GetUVPtr() : nullptr;
                
                status = meshPtr->AppendMesh(0, 0, faceIndexes->size(), &(*faceIndexes)[0], 0, 0, 0, i >= 1 && flags->ShouldLoadTexture() ? m_meshNode->GetNbUVs() : 0, 
                                             i >= 1&& flags->ShouldLoadTexture() ? pUv : 0, 
                                             i >= 1 && flags->ShouldLoadTexture() ? m_meshNode->GetUVsIndicesPtr(i - 1) : 0);

                // release all
//                m_meshNode->UnPinUVsIndices(i);
                }
            if (flags->ShouldLoadTexture())
                {
                m_meshNode->UnPinUV();
                }
            if (meshPtr->GetNbFaces() == 0)
                {
                if (flags->ShouldLoadTexture())
                    {
                    m_meshNode->UnPinUVsIndices();
                    m_meshNode->UnPinUV();
                    }

                m_node->UnPin();
                //if (isStreaming) s_streamingMutex.unlock();
                return nullptr;
                }

        if (clipsToShow.size() > 0 && status == SUCCESS)
                {
                bool allClips = true;
                for (bool val : clipsToShow) if (!val) allClips = false;
                DifferenceSet diffs;
                diffs.firstIndex = (int) meshPtr->GetNbPoints()+1;
                //auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, YProtPtExtentType>, SMPointIndexNode<POINT, YProtPtExtentType>>(m_node);
                int currentTexId = -1;
                for (size_t i = 0; i < m_meshNode->m_nbClips; ++i)
                    {
                    DifferenceSet d = m_meshNode->GetClipSet(i);
#ifdef USE_DIFFSET
                    if (allClips)
                        {
                        if (d.clientID == ((uint64_t)-1)) diffs.ApplySet(d, 0);
                        }
                    else
                        {
#endif
                        uint64_t lowerId = (d.clientID << 32) >> 32;
                        if (d.clientID == 0) currentTexId++;
                        //uint64_t upperId = (d.clientID >> 32);
                        if (d.clientID == 0 || (d.clientID < ((uint64_t)-1) && lowerId - 1 < clipsToShow.size() && !clipsToShow[lowerId - 1]) && d.upToDate)
                            {
                            //meshPtr->ApplyDifferenceSet(d);
                            diffs.ApplySet(d, 0);
                            //break;
#ifdef USE_DIFFSET
                            }
#endif
                        }
                    }
#ifdef USE_DIFFSET
                meshPtr->ApplyDifferenceSet(diffs);
#else
                if (m_meshNode->m_nbClips > 0)
                    {
                    if (flags->ShouldLoadTexture())
                        m_meshNode->PinUV();
                    meshPtr->ApplyClipMesh(diffs);
                    if (flags->ShouldLoadTexture())
                        m_meshNode->UnPinUV();
                    }
#endif
                }
            assert(status == SUCCESS || m_node->size() ==0);        

            meshP = meshPtr.get();            
            }        
        }

    m_node->UnPin();
    //if (isStreaming) s_streamingMutex.unlock();
    if (meshP == nullptr || meshP->GetNbFaces() == 0) return nullptr;
    return meshP;    
    }


template <class POINT> IScalableMeshMeshPtr ScalableMeshNode<POINT>::_GetMeshUnderClip(IScalableMeshMeshFlagsPtr& flags, uint64_t clipId) const
    {
    LOAD_NODE

    if (m_node->size() == 0) return nullptr;
    m_node->Pin();
    IScalableMeshMeshPtr meshP;
    ScalableMeshMeshPtr meshPtr = ScalableMeshMesh::Create();
    auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, YProtPtExtentType>, SMPointIndexNode<POINT, YProtPtExtentType>>(m_node);
    bvector<DPoint3d> allPts(m_node->size());
    memcpy(&allPts[0], &m_node->operator[](0), m_node->size()*sizeof(DPoint3d));
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
                    int32_t idx = (int32_t)(d.addedFaces[i + j] >= d.firstIndex ? d.addedFaces[i + j] - d.firstIndex + m_node->size() + 1 : d.addedFaces[i + j]);
                    assert(idx > 0 && idx <= m_node->size() + d.addedVertices.size());
                    allIndices.push_back(idx);
                    }
                }

            status = meshPtr->AppendMesh(0, 0, allIndices.size(), &allIndices[0], 0, 0, 0, 0, 0, 0);
            meshP = meshPtr.get();
            assert(status == SUCCESS);
            break;
            }
        }

    m_node->UnPin();
    if (meshP.get() != nullptr && meshP->GetNbFaces() == 0) return nullptr;
    return meshP;
    }

    template <class POINT> void ScalableMeshNode<POINT>::ComputeDiffSet(DifferenceSet& diffs, const bvector<bool>& clipsToShow, ScalableMeshTextureID texID, bool applyAllClips) const
    {    
#ifdef USE_DIFFSET
    bool allClips = true;
    for (bool val : clipsToShow) if (!val) allClips = false;
#endif
    
    auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, YProtPtExtentType>, SMPointIndexNode<POINT, YProtPtExtentType>>(m_node);
    
    diffs.firstIndex = (int)m_meshNode->size() + 1;
    //auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, YProtPtExtentType>, SMPointIndexNode<POINT, YProtPtExtentType>>(m_node);
    int currentTexId = -1;
    for (size_t i = 0; i < m_meshNode->m_nbClips; ++i)
        {
        DifferenceSet d = m_meshNode->GetClipSet(i);
#ifdef USE_DIFFSET
        if (allClips)
            {
            if (d.clientID == ((uint64_t)-1)) diffs.ApplySet(d, 0);
            }
        else
            {
#endif
            //NEEDS_WORK_SM_ELENIE : Remove shift, use full 64 bits
            uint64_t lowerId = (d.clientID << 32) >> 32;
            if (d.clientID == 0) currentTexId++;
            if (currentTexId != texID) continue;
            //uint64_t upperId = (d.clientID >> 32);
            if (d.clientID == 0 || 
                (d.toggledForID && d.clientID < ((uint64_t)-1) && lowerId - 1 < clipsToShow.size() && !clipsToShow[lowerId - 1] && !applyAllClips) 
                && d.upToDate)
                {
                //meshPtr->ApplyDifferenceSet(d);
                diffs.ApplySet(d, 0);
                //break;
#ifdef USE_DIFFSET
                }
#endif
            }
        }
    }

template <class POINT> void ScalableMeshNode<POINT>::ComputeDiffSet(DifferenceSet& diffs, const bset<uint64_t>& clipsToShow, ScalableMeshTextureID texID) const
    {
#ifdef USE_DIFFSET
    bool allClips = true;
    for (bool val : clipsToShow) if (!val) allClips = false;
#endif

    auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, YProtPtExtentType>, SMPointIndexNode<POINT, YProtPtExtentType>>(m_node);

    diffs.firstIndex = (int)m_meshNode->size() + 1;
    //auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, YProtPtExtentType>, SMPointIndexNode<POINT, YProtPtExtentType>>(m_node);
    int currentTexId = -1;
    for (size_t i = 0; i < m_meshNode->m_nbClips; ++i)
        {
        DifferenceSet d = m_meshNode->GetClipSet(i);
#ifdef USE_DIFFSET
        if (allClips)
            {
            if (d.clientID == ((uint64_t)-1)) diffs.ApplySet(d, 0);
            }
        else
            {
#endif
            //NEEDS_WORK_SM_ELENIE : Remove shift, use full 64 bits
            //uint64_t lowerId = (d.clientID << 32) >> 32;
            if (d.clientID == 0) currentTexId++;
            if (currentTexId != texID) continue;
            //uint64_t upperId = (d.clientID >> 32);
            if (d.toggledForID && (d.clientID == 0 || (d.clientID < ((uint64_t)-1) && clipsToShow.count(d.clientID) == 0) && d.upToDate))
                {
                //meshPtr->ApplyDifferenceSet(d);
                diffs.ApplySet(d, 0);
                //break;
#ifdef USE_DIFFSET
                }
#endif
            }
        }
    }

template <class POINT> IScalableMeshMeshPtr ScalableMeshNode<POINT>::_GetMeshByParts(const bvector<bool>& clipsToShow, ScalableMeshTextureID texID) const
    {
    LOAD_NODE

        //auto isStreaming = s_useStreamingStore;        

        //if(isStreaming) s_streamingMutex.lock();

    m_node->Pin();
    auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, YProtPtExtentType>, SMPointIndexNode<POINT, YProtPtExtentType>>(m_node);    
    IScalableMeshMeshPtr meshP;
    //NEEDS_WORK_SM_PROGRESSIF : Node header loaded unexpectingly
    if (m_node->size() > 0)
        {
        //auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, YProtPtExtentType>, SMPointIndexNode<POINT, YProtPtExtentType>>(m_node);        
        ScalableMeshMeshPtr meshPtr = ScalableMeshMesh::Create();

        vector<DPoint3d> dataPoints(m_node->size());

        PtToPtConverter converter;

        for (size_t pointInd = 0; pointInd < m_node->size(); pointInd++)
            {
            dataPoints[pointInd] = converter.operator()(m_node->operator[](pointInd));
            }

        int status = meshPtr->AppendMesh(m_node->size(), &dataPoints[0], 0, 0, 0, 0, 0, 0, 0, 0);

        if (texID == IScalableMeshNode::UNTEXTURED_PART)
            {                        
            RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> faceIndexes = m_meshNode->GetPtsIndicePtr();

            if (faceIndexes->size() > 0)
                status = meshPtr->AppendMesh(0, 0, faceIndexes->size(), &(*faceIndexes)[0], 0, 0, 0, 0, nullptr, nullptr);            
            }
        else
            {
            m_meshNode->PinUV();            
            //                   m_meshNode->PinUVsIndices(i);
            RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> faceIndexes = m_meshNode->GetPtsIndicePtr();            
            DPoint2d* pUv = m_meshNode->GetUVPtr();
            if (faceIndexes->size() > 0)
                status = meshPtr->AppendMesh(0, 0, faceIndexes->size(), &(*faceIndexes)[0], 0, 0, 0, m_meshNode->GetNbUVs(), pUv, m_meshNode->GetUVsIndicesPtr(texID - 1));

            // release all            
            //                m_meshNode->UnPinUVsIndices(i);
            m_meshNode->UnPinUV();
            }
        if (meshPtr->GetNbFaces() == 0)
            {            
            m_node->UnPin();
            //if (isStreaming) s_streamingMutex.unlock();
            return nullptr;
            }

        if (clipsToShow.size() > 0 && status == SUCCESS)
            {
            DifferenceSet diffs;
            
            ComputeDiffSet(diffs, clipsToShow, texID);

#ifdef USE_DIFFSET
            meshPtr->ApplyDifferenceSet(diffs);
#else
            if (m_meshNode->m_nbClips > 0)
                {
                m_meshNode->PinUV();
                meshPtr->ApplyClipMesh(diffs);
                //DRange3d range = GetNodeExtent();
                //NEEDS_WORK_SM: Only works when UVs can be calculated from point coordinates e.g. when not using atlas. Need to make sure UVs are properly interpolated for atlas.
                // if (m_meshNode->GetNbUVs() > 0) meshPtr->RecalculateUVs(range);
                m_meshNode->UnPinUV();
                }
#endif
            }
        assert(status == SUCCESS || m_node->size() == 0);

        meshP = meshPtr.get();        
        }

        m_meshNode->UnPinUVsIndices();
        m_meshNode->UnPinUV();        

    m_node->UnPin();
    //if (isStreaming) s_streamingMutex.unlock();
    if (meshP != nullptr && meshP->GetNbFaces() == 0) return nullptr;

    return meshP;
    }

template <class POINT> IScalableMeshMeshPtr ScalableMeshNode<POINT>::_GetMeshByParts(const bset<uint64_t>& clipsToShow, ScalableMeshTextureID texID) const
    {
    LOAD_NODE

        //auto isStreaming = s_useStreamingStore;        

        //if(isStreaming) s_streamingMutex.lock();

        m_node->Pin();
    auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, YProtPtExtentType>, SMPointIndexNode<POINT, YProtPtExtentType>>(m_node);    
    IScalableMeshMeshPtr meshP;
    //NEEDS_WORK_SM_PROGRESSIF : Node header loaded unexpectingly
    if (m_node->size() > 0)
        {
        //auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, YProtPtExtentType>, SMPointIndexNode<POINT, YProtPtExtentType>>(m_node);        
        ScalableMeshMeshPtr meshPtr = ScalableMeshMesh::Create();

        vector<DPoint3d> dataPoints(m_node->size());

        PtToPtConverter converter;

        for (size_t pointInd = 0; pointInd < m_node->size(); pointInd++)
            {
            dataPoints[pointInd] = converter.operator()(m_node->operator[](pointInd));
            }

        int status = meshPtr->AppendMesh(m_node->size(), &dataPoints[0], 0, 0, 0, 0, 0, 0, 0, 0);

        if (texID == IScalableMeshNode::UNTEXTURED_PART)
            {                        
            RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> faceIndexes(m_meshNode->GetPtsIndicePtr());

            if (faceIndexes->size() > 0)
                status = meshPtr->AppendMesh(0, 0, faceIndexes->size(), &(*faceIndexes)[0], 0, 0, 0, 0, nullptr, nullptr);            
            }
        else
            {
            m_meshNode->PinUV();            
            //                   m_meshNode->PinUVsIndices(i);            
            RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> faceIndexes(m_meshNode->GetPtsIndicePtr());

            if (faceIndexes->size() > 0)
                {
                DPoint2d* pUv = m_meshNode->GetUVPtr();
                status = meshPtr->AppendMesh(0, 0, faceIndexes->size(), &(*faceIndexes)[0], 0, 0, 0, m_meshNode->GetNbUVs(), pUv, m_meshNode->GetUVsIndicesPtr(texID - 1));
                }

            // release all            
            //                m_meshNode->UnPinUVsIndices(i);
            m_meshNode->UnPinUV();
            }
        if (meshPtr->GetNbFaces() == 0)
            {            
            m_node->UnPin();
            //if (isStreaming) s_streamingMutex.unlock();
            return nullptr;
            }

        if (clipsToShow.size() > 0 && status == SUCCESS)
            {
            DifferenceSet diffs;

            ComputeDiffSet(diffs, clipsToShow, texID);

#ifdef USE_DIFFSET
            meshPtr->ApplyDifferenceSet(diffs);
#else
            if (m_meshNode->m_nbClips > 0)
                {
                m_meshNode->PinUV();
                meshPtr->ApplyClipMesh(diffs);
                //DRange3d range = GetNodeExtent();
                //NEEDS_WORK_SM: Only works when UVs can be calculated from point coordinates e.g. when not using atlas. Need to make sure UVs are properly interpolated for atlas.
                // if (m_meshNode->GetNbUVs() > 0) meshPtr->RecalculateUVs(range);
                m_meshNode->UnPinUV();
                }
#endif
            }
        assert(status == SUCCESS || m_node->size() == 0);

        meshP = meshPtr.get();        
        }

    m_meshNode->UnPinUVsIndices();
    m_meshNode->UnPinUV();    

    m_node->UnPin();
    //if (isStreaming) s_streamingMutex.unlock();
    if (meshP != nullptr && meshP->GetNbFaces() == 0) return nullptr;

    return meshP;
    }

template <class POINT> int ScalableMeshNode<POINT>::_GetTextureID(size_t texture_id) const
    {
    auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, YProtPtExtentType>, SMPointIndexNode<POINT, YProtPtExtentType>>(m_node);
    if (m_meshNode->m_nodeHeader.m_textureID[texture_id].IsValid()) return m_meshNode->m_nodeHeader.m_textureID[texture_id].m_integerID;
    else return INT_MAX;
    }

template <class POINT> size_t ScalableMeshNode<POINT>::_GetNbTexture() const
{
    auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, YProtPtExtentType>, SMPointIndexNode<POINT, YProtPtExtentType>>(m_node);
    return m_meshNode->GetNbOfTextures();
}

template <class POINT> IScalableMeshTexturePtr ScalableMeshNode<POINT>::_GetTexture(size_t texture_id) const
{
    LOAD_NODE
    
    ScalableMeshTexturePtr textureP;

    if (m_node->size() > 0)
    {
        auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, YProtPtExtentType>, SMPointIndexNode<POINT, YProtPtExtentType>>(m_node);
        //ScalableMeshTexturePtr texturePtr = ScalableMesh::create
        //ScalableMeshMeshPtr meshPtr = ScalableMeshMesh::Create();

        //vector<DPoint3d> dataPoints(m_node->size());

        //PtToPtConverter converter;

        //for (size_t pointInd = 0; pointInd < m_node->size(); pointInd++)
        //{
        //    dataPoints[pointInd] = converter.operator()(m_node->operator[](pointInd));
        //}

        // NEEDS_WORK_SM : texture logique !
        //std::ofstream file_s;
        //file_s.open("C:\\dev\\ContextCapture\\_log.txt", ios_base::app);
        //file_s << "PushIndices etc... -- shit 11" << endl;
        //int32_t* faceIndexes = m_meshNode->GetPtsIndicePtr(0);

        //int status = meshPtr->AppendMesh(m_node->size(), &dataPoints[0], m_node->m_nodeHeader.m_nbFaceIndexes, (int32_t*)&m_node->operator[](m_node->size()), 0, 0, 0);
        //int status = meshPtr->AppendMesh(m_node->size(), &dataPoints[0], m_node->m_nodeHeader.m_nbFaceIndexes, faceIndexes, 0, 0, 0);

        //assert(status == SUCCESS);

        m_meshNode->PinTexture(texture_id);

        uint8_t* data = m_meshNode->GetTexturePtr(texture_id);
        if (data == nullptr) return nullptr;
        // SM_NEEDS_WORK : ugly patch for dimension of JPEG (already in jpeg compression)
        size_t size;
        Point2d dimension;
        int nOfChannels;
        //dimension.x = 1;
        //dimension.y = 1;
        memcpy_s(&dimension.x, sizeof(int), data, sizeof(int));
        memcpy_s(&dimension.y, sizeof(int), (int*)data + 1, sizeof(int));
        memcpy_s(&nOfChannels, sizeof(int), (int*)data + 2, sizeof(int));
        size = dimension.x * dimension.y * nOfChannels;

        /*std::ofstream file_s;
        file_s.open("C:\\Users\\Thomas.Butzbach\\Documents\\data_scalableMesh\\test_SDK\\binary2.txt", ios_base::binary);
        file_s.write((char*)data, size);
        file_s.close();*/

        /*WString fileName = L"file://C:\\dev\\ContextCapture\\texture.bmp";
        HFCPtr<HFCURL> fileUrl(HFCURL::Instanciate(fileName));
        //HFCPtr<HRPPixelType> pImageDataPixelType(new HRPPixelTypeV24R8G8B8());
        HFCPtr<HRPPixelType> pImageDataPixelType(new HRPPixelTypeV24B8G8R8());

        HRFBmpCreator::CreateBmpFileFromImageData(fileUrl,
            dimension.x,
            dimension.y,
            pImageDataPixelType,
            &data[0]);*/

/*        WChar     formatString[128];
        //WChar     formatString2[128];
        BeStringUtilities::Snwprintf(formatString, _countof(formatString), L"%d", m_meshNode->m_nodeHeader.m_textureID.m_integerID);
        //BeStringUtilities::Snwprintf(formatString2, _countof(formatString2), L"%d", i);

        WString fileName = WString(L"file://C:\\dev\\ContextCapture\\BMP_READ\\texture");
        fileName += WString(formatString);
        //fileName += WString(L"_");
        //fileName += WString(formatString2);
        fileName += WString(L".bmp");
        HFCPtr<HFCURL> fileUrl(HFCURL::Instanciate(fileName));
        HFCPtr<HRPPixelType> pImageDataPixelType(new HRPPixelTypeV24B8G8R8());

        HRFBmpCreator::CreateBmpFileFromImageData(fileUrl,
            dimension.x,
            dimension.y,
            pImageDataPixelType,
            &data[0]+2*sizeof(int));*/
        textureP = ScalableMeshTexture::Create(&data[0]/*+sizeof(int)*2*/, size, dimension, m_meshNode->m_nodeHeader.m_textureID[texture_id].m_integerID);

        /*textureP->SetData(data);
        textureP->SetDimension(dimension);
        textureP->SetSize(size);*/
        //delete[] data;
        //textureP = meshPtr.get();

        m_meshNode->UnPinTexture(texture_id);
    }
    else assert(m_node->m_nodeHeader.m_nodeCount == 0);
    return textureP;
}

template <class POINT> IScalableMeshMeshPtr ScalableMeshCachedMeshNode<POINT>::_GetMesh(IScalableMeshMeshFlagsPtr& flags, bvector<bool>& clipsToShow) const
    {        
    
        return __super::_GetMesh(flags, clipsToShow);  
    }

template <class POINT> IScalableMeshMeshPtr ScalableMeshCachedMeshNode<POINT>::_GetMeshByParts(const bvector<bool>& clipsToShow, ScalableMeshTextureID texID) const
    {

    if (m_loadedMeshes.size() > 0)
        {
        return m_loadedMeshes[texID];
        }
    else
        {
        return __super::_GetMeshByParts(clipsToShow, texID);
        }
    }

template <class POINT> IScalableMeshMeshPtr ScalableMeshCachedMeshNode<POINT>::_GetMeshByParts(const bset<uint64_t>& clipsToShow, ScalableMeshTextureID texID) const
    {

    if (m_loadedMeshes.size() > 0)
        {
        return m_loadedMeshes[texID];
        }
    else
        {
        return __super::_GetMeshByParts(clipsToShow, texID);
        }
    }

template <class POINT> IScalableMeshTexturePtr ScalableMeshCachedMeshNode<POINT>::_GetTexture(size_t texture_id) const
    {    
    if (m_loadedTextures.size() > 0)
        {        
        assert(m_loadedTextures[texture_id] != 0);
        return m_loadedTextures[texture_id];        
        }
    else
        {
        return __super::_GetTexture(texture_id);
        }   
    }

//NEEDS_WORK_SM : With new display not necessarily required
template <class POINT> void ScalableMeshCachedMeshNode<POINT>::LoadMeshes(bool loadGraph, const bvector<bool>& clipsToShow, bool applyAllClips)
    {
    //Not implemented yet
    assert(loadGraph == false);
    //NEEDS_WORK_SM : Need to be greater or equal to 0 (SM without texture shouldn't have texture).
    assert(GetNbTexture() >= 1);

    m_loadedMeshes.resize(_GetNbMeshes());
    m_loadedTextures.resize(_GetNbMeshes() - 1);

    if (m_loadedTextures.size() == 0)
        {
        m_loadedMeshes[0] = __super::_GetMeshByParts(clipsToShow, -2);
        }
    else
        {   
        for (size_t meshInd = 0; meshInd < _GetNbMeshes(); meshInd++)
            {   
            int textureId = 0;

            if (meshInd == 0)
                {
                textureId = -1;
                }
            else
                {
                textureId = (int)(meshInd - 1);
                }

            m_loadedMeshes[meshInd] = __super::_GetMeshByParts(clipsToShow, textureId);

            if (textureId >= 0)
                {
                m_loadedTextures[textureId] = __super::_GetTexture(textureId);
                }
            }    
        }
    }


template <class POINT> void ScalableMeshCachedMeshNode<POINT>::LoadMeshes(bool loadGraph, const bset<uint64_t>& clipsToShow) 
    {
    //Not implemented yet
    assert(loadGraph == false);
    //NEEDS_WORK_SM : Need to be greater or equal to 0 (SM without texture shouldn't have texture).
    assert(GetNbTexture() >= 1);

    m_loadedMeshes.resize(_GetNbMeshes());
    m_loadedTextures.resize(_GetNbMeshes() - 1);

    if (m_loadedTextures.size() == 0)
        {
        m_loadedMeshes[0] = __super::_GetMeshByParts(clipsToShow, -2);                   
        }
    else
        {   
        for (size_t meshInd = 0; meshInd < _GetNbMeshes(); meshInd++)
            {   
            int textureId = 0;

            if (meshInd == 0)
                {
                textureId = -1;
                }
            else
                {
                textureId = (int)(meshInd - 1);
                }

            m_loadedMeshes[meshInd] = __super::_GetMeshByParts(clipsToShow, textureId);                   

            if (textureId >= 0)
                {
                m_loadedTextures[textureId] = __super::_GetTexture(textureId);
                }
            }    
        }
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


inline void ApplyClipDiffSetToMesh(DPoint3d*& pointsP, size_t& nbPoints, 
                                   int32_t*&  faceIndexesP, size_t& nbFaceIndexes, 
                                   DPoint2d*& uvP, int32_t*& uvIndexP, size_t& uvCount, 
                                   const DifferenceSet& d)
    {       
    DPoint3d* newPoints = new DPoint3d[d.addedVertices.size() + nbPoints];
    memcpy(&newPoints[nbPoints], &d.addedVertices[0], sizeof(DPoint3d) * d.addedVertices.size());
    if (nbPoints > 0)
        {
        memcpy(newPoints, pointsP, sizeof(DPoint3d) * nbPoints);
        delete[] pointsP;
        }
    pointsP = newPoints;

    if (d.addedUvIndices.size() > 0)
        {
        DPoint2d* newUvs = new DPoint2d[d.addedUvs.size() + uvCount];
        memcpy(&newUvs[uvCount], &d.addedUvs[0], sizeof(DPoint2d) * d.addedUvs.size());
        if (uvCount > 0)
            {
            memcpy(newUvs, uvP, sizeof(DPoint2d) * uvCount);
            delete[] uvP;
            }
        uvP = newUvs;
        for (size_t uvI = 0; uvI < d.addedUvs.size() + uvCount; ++uvI)
            { 
            if (uvP[uvI].x < 0)  uvP[uvI].x = 0;
            if (uvP[uvI].y < 0)  uvP[uvI].y = 0;
            if (uvP[uvI].x > 1)  uvP[uvI].x = 1;
            if (uvP[uvI].y > 1)  uvP[uvI].y = 1;
            }
        }
    else if (uvIndexP)
        {
        delete[] uvIndexP;
        uvIndexP = 0;
        }
    if (d.addedFaces.size() >= 3 && d.addedFaces.size() < 1024 * 1024)
        {
        size_t newMaxNIndexes = d.addedFaces.size();
        int32_t* newfaceIndexes = new int32_t[newMaxNIndexes];
        size_t newNIndexes = 0;
        int32_t* newUvIndices = d.addedUvIndices.size() > 0? new int32_t[newMaxNIndexes] : nullptr;
        for (int i = 0; i + 2 < d.addedFaces.size(); i += 3)
            {
            if (!(d.addedFaces[i] - 1 >= 0 && d.addedFaces[i] - 1 < nbPoints + d.addedVertices.size() && d.addedFaces[i + 1] - 1 >= 0 && d.addedFaces[i + 1] - 1 < nbPoints + d.addedVertices.size()
                && d.addedFaces[i + 2] - 1 >= 0 && d.addedFaces[i + 2] - 1 < nbPoints + d.addedVertices.size()))
                {
#if SM_TRACE_CLIP_MESH
                std::string s;
                s += "INDICES " + std::to_string(d.addedFaces[i]) + " " + std::to_string(d.addedFaces[i + 1]) + " " + std::to_string(d.addedFaces[i + 2]);
#endif
                continue;
                }
             assert(d.addedFaces[i] - 1 >= 0 && d.addedFaces[i] - 1 < nbPoints + d.addedVertices.size() && d.addedFaces[i + 1] - 1 >= 0 && d.addedFaces[i + 1] - 1 < nbPoints + d.addedVertices.size()
            && d.addedFaces[i + 2] - 1 >= 0 && d.addedFaces[i + 2] - 1 < nbPoints + d.addedVertices.size());
          
            for (size_t j = 0; j < 3 && newNIndexes <newMaxNIndexes; ++j)
                {
                int32_t idx = (int32_t)(d.addedFaces[i + j] >= d.firstIndex ? d.addedFaces[i + j] - d.firstIndex + nbPoints + 1 : d.addedFaces[i + j]);
                assert(idx > 0 && idx <= nbPoints + d.addedVertices.size());
                newfaceIndexes[newNIndexes] = idx;
                if(d.addedUvIndices.size() > 0) newUvIndices[newNIndexes] = d.addedUvIndices[i+j]+(int32_t)uvCount;
                newNIndexes++;
                }
            }
        nbFaceIndexes = newNIndexes;
        if (faceIndexesP != nullptr) delete[] faceIndexesP;
        faceIndexesP = newfaceIndexes;
        if (d.addedUvIndices.size() > 0)
            {
            delete[] uvIndexP;
            uvIndexP = newUvIndices;
            }
        }
    else
        {
        nbFaceIndexes = 0;
        if (faceIndexesP != nullptr) delete[] faceIndexesP;
        faceIndexesP = nullptr;
        }
    nbPoints += d.addedVertices.size();
    if (d.addedUvIndices.size() > 0) uvCount += d.addedUvs.size();
    }

typedef struct {float x, y;} FloatXY;
typedef struct {float x, y, z;} FloatXYZ;

inline void ApplyClipDiffSetToMesh(FloatXYZ*& points, size_t& nbPoints, 
                                   int32_t*& faceIndexes, size_t& nbFaceIndexes, 
                                   FloatXY*& pUv, int32_t*& pUvIndex, size_t& uvCount,
                                   FloatXYZ const* inPoints, size_t inNbPoints, 
                                   int32_t const*  inFaceIndexes, size_t inNbFaceIndexes, 
                                   DPoint2d* pInUv, int32_t* pInUvIndex, size_t inUvCount, 
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
                    newUvIndices[newNIndexes] = d.addedUvIndices[i + j] + (int32_t)inUvCount;
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



    template <class POINT> void ScalableMeshCachedDisplayNode<POINT>::LoadMeshes(bool loadGraph, const bvector<bool>& clipsToShow, IScalableMeshDisplayCacheManagerPtr& displayCacheManagerPtr, bool loadTexture, bool applyAllClips)
    {
    static bool s_deactivateTexture = false; 

    //Not implemented yet
    assert(loadGraph == false);

    LOAD_NODE
    
    if (displayCacheManagerPtr != 0 && !m_isLoaded)                
        {
        assert(m_cachedDisplayTextures.size() == 0);

        //NEEDS_WORK_SM_PROGRESSIF : Node header loaded unexpectingly
        if (m_node->size() > 0)
            {
            //NEEDS_WORK_SM : Load texture here
            //m_cachedDisplayTexture

            auto meshNode = dynamic_pcast<SMMeshIndexNode<POINT, YProtPtExtentType>, SMPointIndexNode<POINT, YProtPtExtentType>>(m_node);
            
            m_cachedDisplayMeshes.resize(_GetNbMeshes());

            assert(meshNode->GetNbOfTextures() + 1 == m_cachedDisplayMeshes.size());

            m_cachedDisplayTextures.resize(_GetNbMeshes());

            DRange3d range3D(_GetContentExtent());

            DPoint3d centroid;                                                        
            centroid = DPoint3d::From((range3D.high.x + range3D.low.x) / 2.0, (range3D.high.y + range3D.low.y) / 2.0, (range3D.high.z + range3D.low.z) / 2.0);

            m_node->Pin();

            vector<FloatXYZ> dataPoints(m_node->size());
                        
            for (size_t pointInd = 0; pointInd < m_node->size(); pointInd++)
                {                               
                dataPoints[pointInd].x = (float)(PointOp<POINT>::GetX(m_node->operator[](pointInd)) - centroid.x);
                dataPoints[pointInd].y = (float)(PointOp<POINT>::GetY(m_node->operator[](pointInd)) - centroid.y);
                dataPoints[pointInd].z = (float)(PointOp<POINT>::GetZ(m_node->operator[](pointInd)) - centroid.z);                
                }

            m_node->UnPin();
                                       

//            size_t GetNbOfTextures()
  
            for (size_t meshInd = 0; meshInd < meshNode->GetNbOfTextures() + 1; meshInd++)
                {                                    
                FloatXYZ* toLoadPoints = 0;
                size_t    toLoadNbPoints = 0; 
                int32_t*  toLoadFaceIndexes = 0;
                size_t    toLoadNbFaceIndexes = 0;
                FloatXY*  toLoadUv = 0;
                int32_t*  toLoadUvIndex = 0;
                size_t    toLoadUvCount = 0;      

                if (meshInd == 0 && meshNode->GetNbOfTextures() == 1)
                    continue;

                RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> faceIndexes(meshNode->GetPtsIndicePtr());
                                
                if (faceIndexes == 0)
                    {
                    assert(meshInd == 0 || m_node->size() <= 4);
                    m_cachedDisplayMeshes[meshInd] = 0;
                    }
                else
                    {
                    size_t nbFaceIndices = faceIndexes->size();

                    if (nbFaceIndices == 0)
                        {                        
                        continue;
                        }

                    if (meshInd > 0 && loadTexture)
                        {
                        //                        auto idTexture = GetTextureID(meshInd - 1);

                        IScalableMeshTexturePtr smTexturePtr(GetTexture(meshInd - 1));

                        BentleyStatus status = displayCacheManagerPtr->_CreateCachedTexture(m_cachedDisplayTextures[meshInd],
                                                                                            smTexturePtr->GetDimension().x,
                                                                                            smTexturePtr->GetDimension().y,
                                                                                            false,
                                                                                            QV_RGB_FORMAT,
                                                                                            smTexturePtr->GetData());

                        assert(status == SUCCESS);
                        }

                    DPoint2d* uvPtr = 0;
                    size_t    nbUvs = 0;
                    int32_t*  uvIndicesP = 0;

                    if (m_cachedDisplayTextures[meshInd] != 0)
                        {
                        meshNode->PinUV();
                        uvPtr = meshNode->GetUVPtr();
                        nbUvs = meshNode->GetNbUVs();

                        meshNode->PinUVsIndices(meshInd - 1);
                        uvIndicesP = meshNode->GetUVsIndicesPtr(meshInd - 1);                        
                        }

                    DifferenceSet clipDiffSet;

                    if (meshNode->m_nbClips > 0 && (clipsToShow.size() > 0 || applyAllClips))
                        {
                        ComputeDiffSet(clipDiffSet, clipsToShow, meshInd, applyAllClips);

                        ApplyClipDiffSetToMesh(toLoadPoints, toLoadNbPoints,
                                               toLoadFaceIndexes, toLoadNbFaceIndexes,
                                               toLoadUv, toLoadUvIndex, toLoadUvCount,
                                               &dataPoints[0], dataPoints.size(),
                                               &(*faceIndexes)[0], nbFaceIndices,
                                               uvPtr, uvIndicesP, nbUvs,
                                               clipDiffSet,
                                               centroid);

                        for (size_t ind = 0; ind < toLoadNbFaceIndexes; ind++)
                            {
                            toLoadFaceIndexes[ind] -= 1;
                            }
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

                    bvector<float>          uvCoordinates;
                    float*                  uvCoordinatesP = 0;
                    SmCachedDisplayTexture* cachedTexture = 0;

                    if (m_cachedDisplayTextures[meshInd] != 0 && toLoadUvCount > 0)
                        {
                        cachedTexture = m_cachedDisplayTextures[meshInd];

                        assert(meshNode->GetNbUVs() == m_node->size());

                        //NEEDS_WORK_SM : Can we store UV coordinate as float
                        //uvCoordinates.resize(meshNode->GetNbUVs() * 2);
                        uvCoordinates.resize(/*m_node->size()*/toLoadUvCount * 2);
                        if (toLoadUvCount < toLoadNbPoints)
                            {
                            uvCoordinates.resize(toLoadNbPoints * 2);
                            for (size_t idx = toLoadUvCount * 2; idx < uvCoordinates.size(); ++idx)
                                uvCoordinates[idx] = 0.0;
                            }

                        for (size_t uvCoordInd = 0; uvCoordInd < toLoadNbFaceIndexes; uvCoordInd++)
                            {
                            // if (uvCoordinates[toLoadFaceIndexes[uvCoordInd] * 2] != 0 || uvCoordinates[toLoadFaceIndexes[uvCoordInd] * 2 + 1] != 0)
                            //     continue;

                            assert(toLoadUv[toLoadUvIndex[uvCoordInd] - 1].x <= 1.0 && toLoadUv[toLoadUvIndex[uvCoordInd] - 1].x >= 0.0);
                            assert(toLoadUv[toLoadUvIndex[uvCoordInd] - 1].y <= 1.0 && toLoadUv[toLoadUvIndex[uvCoordInd] - 1].y >= 0.0);

                            uvCoordinates[toLoadFaceIndexes[uvCoordInd] * 2] = toLoadUv[toLoadUvIndex[uvCoordInd] - 1].x;
                            uvCoordinates[toLoadFaceIndexes[uvCoordInd] * 2 + 1] = 1.0 - toLoadUv[toLoadUvIndex[uvCoordInd] - 1].y;
                            }

                        uvCoordinatesP = new float[uvCoordinates.size()];
                        memcpy(uvCoordinatesP, &uvCoordinates[0], sizeof(float)*uvCoordinates.size());

                        meshNode->UnPinUV();
                        meshNode->UnPinUVsIndices(meshInd - 1);
                        }

                    if (s_deactivateTexture)
                        {
                        BentleyStatus status = displayCacheManagerPtr->_CreateCachedMesh(m_cachedDisplayMeshes[meshInd],
                                                                                         toLoadNbPoints,
                                                                                         &centroid,
                                                                                         (float*)toLoadPoints,
                                                                                         0,
                                                                                         (int)toLoadNbFaceIndexes / 3,
                                                                                         toLoadFaceIndexes,
                                                                                         0,
                                                                                         0);

                        assert(status == SUCCESS);
                        }
                    else
                        {
                        BentleyStatus status = displayCacheManagerPtr->_CreateCachedMesh(m_cachedDisplayMeshes[meshInd],
                                                                                         toLoadNbPoints,
                                                                                         &centroid,
                                                                                         (float*)toLoadPoints,
                                                                                         0,
                                                                                         (int)toLoadNbFaceIndexes / 3,
                                                                                         toLoadFaceIndexes,
                                                                                         uvCoordinatesP,
                                                                                         cachedTexture);

                        assert(status == SUCCESS);
                        }
                    if (uvCoordinatesP != 0)
                        delete[]uvCoordinatesP;
                    }                
                                     
                if (meshNode->m_nbClips > 0)
                    {   
                    if (toLoadPoints != 0)
                        delete [] toLoadPoints;
                    
                    if (toLoadUvIndex != 0)
                        delete [] toLoadUvIndex;
                    }
                
                if (toLoadFaceIndexes != 0)
                    delete [] toLoadFaceIndexes;
                
                if (toLoadUv != 0)
                    delete [] toLoadUv;  
                }
            } 

        m_isLoaded = true;
        m_displayCacheManagerPtr = displayCacheManagerPtr;
        }

    m_node->UnPin();
    }


    template <class POINT> void ScalableMeshCachedDisplayNode<POINT>::LoadMeshes(bool loadGraph, const bset<uint64_t>& clipsToShow, IScalableMeshDisplayCacheManagerPtr& displayCacheManagerPtr, bool loadTexture)
    {
    static bool s_deactivateTexture = false; 

    //Not implemented yet
    assert(loadGraph == false);

    LOAD_NODE
    
    if (displayCacheManagerPtr != 0 && !m_isLoaded)                
        {
        assert(m_cachedDisplayTextures.size() == 0);

        //NEEDS_WORK_SM_PROGRESSIF : Node header loaded unexpectingly
        if (m_node->size() > 0)
            {
            //NEEDS_WORK_SM : Load texture here
            //m_cachedDisplayTexture

            auto meshNode = dynamic_pcast<SMMeshIndexNode<POINT, YProtPtExtentType>, SMPointIndexNode<POINT, YProtPtExtentType>>(m_node);

            m_cachedDisplayMeshes.resize(_GetNbMeshes());

            assert(meshNode->GetNbOfTextures() + 1 == m_cachedDisplayMeshes.size());

            m_cachedDisplayTextures.resize(_GetNbMeshes());

            DRange3d range3D(_GetContentExtent());

            DPoint3d centroid;
            centroid = DPoint3d::From((range3D.high.x + range3D.low.x) / 2.0, (range3D.high.y + range3D.low.y) / 2.0, (range3D.high.z + range3D.low.z) / 2.0);

            m_node->Pin();

            vector<FloatXYZ> dataPoints(m_node->size());

            for (size_t pointInd = 0; pointInd < m_node->size(); pointInd++)
                {
                dataPoints[pointInd].x = (float)(PointOp<POINT>::GetX(m_node->operator[](pointInd)) - centroid.x);
                dataPoints[pointInd].y = (float)(PointOp<POINT>::GetY(m_node->operator[](pointInd)) - centroid.y);
                dataPoints[pointInd].z = (float)(PointOp<POINT>::GetZ(m_node->operator[](pointInd)) - centroid.z);
                }

            m_node->UnPin();

            for (size_t meshInd = 0; meshInd < meshNode->GetNbOfTextures() + 1; meshInd++)
                {
                FloatXYZ* toLoadPoints = 0;
                size_t    toLoadNbPoints = 0;
                int32_t*  toLoadFaceIndexes = 0;
                size_t    toLoadNbFaceIndexes = 0;
                FloatXY*  toLoadUv = 0;
                int32_t*  toLoadUvIndex = 0;
                size_t    toLoadUvCount = 0;                

                RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> faceIndexes(meshNode->GetPtsIndicePtr());
                
                if (faceIndexes == 0 || (meshInd == 0 && meshNode->GetNbOfTextures() + 1 > 1))
                    {
                    assert(meshInd == 0 || m_node->size() <= 4);
                    m_cachedDisplayMeshes[meshInd] = 0;
                    }
                else
                    {
                    size_t nbFaceIndices = faceIndexes->size(); 

                    if (nbFaceIndices == 0)
                        {                        
                        continue;
                        }

                    if (meshInd > 0 && loadTexture)
                        {
                        //                        auto idTexture = GetTextureID(meshInd - 1);

                        IScalableMeshTexturePtr smTexturePtr(GetTexture(meshInd - 1));

                        BentleyStatus status = displayCacheManagerPtr->_CreateCachedTexture(m_cachedDisplayTextures[meshInd],
                                                                                            smTexturePtr->GetDimension().x,
                                                                                            smTexturePtr->GetDimension().y,
                                                                                            false,
                                                                                            QV_RGB_FORMAT,
                                                                                            smTexturePtr->GetData());

                        assert(status == SUCCESS);
                        }

                    DPoint2d* uvPtr = 0;
                    size_t    nbUvs = 0;
                    int32_t*  uvIndicesP = 0;

                    if (m_cachedDisplayTextures[meshInd] != 0)
                        {
                        meshNode->PinUV();
                        uvPtr = meshNode->GetUVPtr();
                        nbUvs = meshNode->GetNbUVs();

                        meshNode->PinUVsIndices(meshInd - 1);
                        uvIndicesP = meshNode->GetUVsIndicesPtr(meshInd - 1);
                        }

                    DifferenceSet clipDiffSet;

                    if (meshNode->m_nbClips > 0 && (clipsToShow.size() > 0))
                        {
                        ComputeDiffSet(clipDiffSet, clipsToShow, meshInd);

                        ApplyClipDiffSetToMesh(toLoadPoints, toLoadNbPoints,
                                               toLoadFaceIndexes, toLoadNbFaceIndexes,
                                               toLoadUv, toLoadUvIndex, toLoadUvCount,
                                               &dataPoints[0], dataPoints.size(),
                                               &(*faceIndexes)[0], nbFaceIndices,
                                               uvPtr, uvIndicesP, nbUvs,
                                               clipDiffSet,
                                               centroid);

                        for (size_t ind = 0; ind < toLoadNbFaceIndexes; ind++)
                            {
                            toLoadFaceIndexes[ind] -= 1;
                            }
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

                    bvector<float>          uvCoordinates;
                    float*                  uvCoordinatesP = 0;
                    SmCachedDisplayTexture* cachedTexture = 0;

                    if (m_cachedDisplayTextures[meshInd] != 0 && toLoadUvCount > 0)
                        {
                        cachedTexture = m_cachedDisplayTextures[meshInd];

                        assert(meshNode->GetNbUVs() == m_node->size());

                        //NEEDS_WORK_SM : Can we store UV coordinate as float
                        //uvCoordinates.resize(meshNode->GetNbUVs() * 2);
                        uvCoordinates.resize(/*m_node->size()*/toLoadUvCount * 2);
                        if (toLoadUvCount < toLoadNbPoints)
                            {
                            uvCoordinates.resize(toLoadNbPoints * 2);
                            for (size_t idx = toLoadUvCount * 2; idx < uvCoordinates.size(); ++idx)
                                uvCoordinates[idx] = 0.0;
                            }

                        for (size_t uvCoordInd = 0; uvCoordInd < toLoadNbFaceIndexes; uvCoordInd++)
                            {
                            // if (uvCoordinates[toLoadFaceIndexes[uvCoordInd] * 2] != 0 || uvCoordinates[toLoadFaceIndexes[uvCoordInd] * 2 + 1] != 0)
                            //     continue;

                            assert(toLoadUv[toLoadUvIndex[uvCoordInd] - 1].x <= 1.0 && toLoadUv[toLoadUvIndex[uvCoordInd] - 1].x >= 0.0);
                            assert(toLoadUv[toLoadUvIndex[uvCoordInd] - 1].y <= 1.0 && toLoadUv[toLoadUvIndex[uvCoordInd] - 1].y >= 0.0);

                            uvCoordinates[toLoadFaceIndexes[uvCoordInd] * 2] = toLoadUv[toLoadUvIndex[uvCoordInd] - 1].x;
                            uvCoordinates[toLoadFaceIndexes[uvCoordInd] * 2 + 1] = 1.0 - toLoadUv[toLoadUvIndex[uvCoordInd] - 1].y;
                            }

                        uvCoordinatesP = new float[uvCoordinates.size()];
                        memcpy(uvCoordinatesP, &uvCoordinates[0], sizeof(float)*uvCoordinates.size());

                        meshNode->UnPinUV();
                        meshNode->UnPinUVsIndices(meshInd - 1);
                        }

                    if (s_deactivateTexture)
                        {
                        BentleyStatus status = displayCacheManagerPtr->_CreateCachedMesh(m_cachedDisplayMeshes[meshInd],
                                                                                         toLoadNbPoints,
                                                                                         &centroid,
                                                                                         (float*)toLoadPoints,
                                                                                         0,
                                                                                         (int)toLoadNbFaceIndexes / 3,
                                                                                         toLoadFaceIndexes,
                                                                                         0,
                                                                                         0);

                        assert(status == SUCCESS);
                        }
                    else
                        {
                        BentleyStatus status = displayCacheManagerPtr->_CreateCachedMesh(m_cachedDisplayMeshes[meshInd],
                                                                                         toLoadNbPoints,
                                                                                         &centroid,
                                                                                         (float*)toLoadPoints,
                                                                                         0,
                                                                                         (int)toLoadNbFaceIndexes / 3,
                                                                                         toLoadFaceIndexes,
                                                                                         uvCoordinatesP,
                                                                                         cachedTexture);

                        assert(status == SUCCESS);
                        }
                    if (uvCoordinatesP != 0)
                        delete[]uvCoordinatesP;
                    }                

                if (meshNode->m_nbClips > 0)
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

        m_isLoaded = true;
        m_displayCacheManagerPtr = displayCacheManagerPtr;
        }

    m_node->UnPin();
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

template <class POINT> size_t ScalableMeshNode<POINT>::_GetLevel() const
    {
    LOAD_NODE

    return m_node->m_nodeHeader.m_level;
    }

template <class POINT> DRange3d ScalableMeshNode<POINT>::_GetNodeExtent() const
    {
    LOAD_NODE

    DRange3d range;
    YProtPtExtentType ext = m_node->m_nodeHeader.m_nodeExtent;
    range = DRange3d::From(DPoint3d::From(ExtentOp<YProtPtExtentType>::GetXMin(ext), ExtentOp<YProtPtExtentType>::GetYMin(ext), ExtentOp<YProtPtExtentType>::GetZMin(ext)),
                           DPoint3d::From(ExtentOp<YProtPtExtentType>::GetXMax(ext), ExtentOp<YProtPtExtentType>::GetYMax(ext), ExtentOp<YProtPtExtentType>::GetZMax(ext)));
    return range;
    }

template <class POINT> DRange3d ScalableMeshNode<POINT>::_GetContentExtent() const
    {
    LOAD_NODE

    DRange3d range;
    YProtPtExtentType ext = m_node->m_nodeHeader.m_contentExtent;
    range = DRange3d::From(DPoint3d::From(ExtentOp<YProtPtExtentType>::GetXMin(ext), ExtentOp<YProtPtExtentType>::GetYMin(ext), ExtentOp<YProtPtExtentType>::GetZMin(ext)),
                           DPoint3d::From(ExtentOp<YProtPtExtentType>::GetXMax(ext), ExtentOp<YProtPtExtentType>::GetYMax(ext), ExtentOp<YProtPtExtentType>::GetZMax(ext)));
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
    //NEEDSWORK_SM: GetNbObjects() is sometimes 0 here
    //return m_node->size();
    }  

template <class POINT> bool ScalableMeshNode<POINT>::_IsHeaderLoaded() const
    {            
    return m_node->IsLoaded();
    }

template <class POINT> bool ScalableMeshNode<POINT>::_IsMeshLoaded() const
    {   
    LOAD_NODE

    return !m_node->Discarded();    
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
    auto m_meshNode = dynamic_cast<SMMeshIndexNode<POINT, YProtPtExtentType>*>(m_node.GetPtr());
    if (m_meshNode->m_tileBcDTM.get() != nullptr)
        return m_meshNode->m_tileBcDTM.get();
    else
        {
        s_nMissedDTMs++;
            {
            std::lock_guard<std::mutex> m(m_meshNode->m_dtmLock);
            bvector<bool> clips;
            IScalableMeshMeshFlagsPtr flags = IScalableMeshMeshFlags::Create();
            auto meshP = GetMesh(flags, clips);
            if (meshP == nullptr) return nullptr;
            meshP->GetAsBcDTM(m_meshNode->m_tileBcDTM);
            return m_meshNode->m_tileBcDTM.get();
            }
        }
    }

template <class POINT> void ScalableMeshNode<POINT>::_GetSkirtMeshes(bvector<PolyfaceHeaderPtr>& meshes) const
    {
    auto m_meshNode = dynamic_cast<SMMeshIndexNode<POINT, YProtPtExtentType>*>(m_node.GetPtr());
    for (size_t i = 0; i < m_meshNode->m_nbClips; ++i)
        {
        DifferenceSet d = m_meshNode->GetClipSet(i);
        if (d.toggledForID) continue;
        PolyfaceHeaderPtr mesh = d.ToPolyfaceMesh(&m_node->operator[](0), m_node->size());
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
    auto m_meshNode = dynamic_cast<SMMeshIndexNode<POINT, YProtPtExtentType>*>(m_node.GetPtr());
    if (m_meshNode == nullptr) return false;
    if (m_meshNode->m_nbClips == 0) return false;
    else return m_meshNode->HasClip(clip);
    }

template <class POINT> void ScalableMeshNode<POINT>::_ApplyAllExistingClips() const
    {
    /*ClipRegistry* clipRegistryP(dynamic_cast<SMMeshIndexNode<POINT, YProtPtExtentType>*>(m_node.GetPtr())->GetClipRegistry());
   
    for (size_t i = 0; i < clipRegistryP->GetNbClips(); ++i)
        {
        //NEEDS_WORK_SM : must remove + 1 magic number
        _AddClip(i + 1, true);
        }*/
       
    _RefreshMergedClip();    
    }

template <class POINT> void ScalableMeshNode<POINT>::_RefreshMergedClip() const
    {
    dynamic_cast<SMMeshIndexNode<POINT, YProtPtExtentType>*>(m_node.GetPtr())->BuildSkirts();
    dynamic_cast<SMMeshIndexNode<POINT, YProtPtExtentType>*>(m_node.GetPtr())->ComputeMergedClips();
    }

template <class POINT> bool ScalableMeshNode<POINT>::_AddClip(uint64_t id, bool isVisible) const
    {
    return dynamic_cast<SMMeshIndexNode<POINT, YProtPtExtentType>*>(m_node.GetPtr())->AddClip(id, isVisible);
    }

template <class POINT> bool ScalableMeshNode<POINT>::_AddClipAsync(uint64_t id, bool isVisible) const
    {
    return dynamic_cast<SMMeshIndexNode<POINT, YProtPtExtentType>*>(m_node.GetPtr())->AddClipAsync(id, isVisible);
    }

template <class POINT> bool ScalableMeshNode<POINT>::_ModifyClip(uint64_t id, bool isVisible) const
    {
    return dynamic_cast<SMMeshIndexNode<POINT, YProtPtExtentType>*>(m_node.GetPtr())->ModifyClip(id,  isVisible);
    }

template <class POINT> bool ScalableMeshNode<POINT>::_DeleteClip(uint64_t id, bool isVisible) const
    {
    return dynamic_cast<SMMeshIndexNode<POINT, YProtPtExtentType>*>(m_node.GetPtr())->DeleteClip(id,isVisible);
    }

template <class POINT> StatusInt ScalableMeshNodeEdit<POINT>::_AddMesh(DPoint3d* vertices, size_t nVertices, int32_t* indices, size_t nIndices)
    {
    m_node->clear();
    auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, YProtPtExtentType>, SMPointIndexNode<POINT, YProtPtExtentType>>(m_node);
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
    m_meshNode->push_back(&nodePts[0], nodePts.size());

    m_meshNode->m_nodeHeader.m_nbFaceIndexes = nIndices;
    bvector<int> componentPointsId;
    if (NULL == m_meshNode->GetGraphPtr()) m_meshNode->CreateGraph();
    CreateGraphFromIndexBuffer(m_meshNode->GetGraphPtr(), (const long*)&indicesVec[0], (int)nIndices, (int)nodePts.size(), componentPointsId, &nodePts[0]);
   // m_meshNode->GetGraphPtr()->SortNodesBasedOnLabel(0);
    m_meshNode->SetGraphDirty();
    m_meshNode->StoreGraph();

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

    m_meshNode->PushPtsIndices(0, &indicesVec[0], m_meshNode->m_nodeHeader.m_nbFaceIndexes);    
    m_meshNode->StorePtsIndice(0);
    m_meshNode->IncreaseTotalCount(m_meshNode->size());

    m_meshNode->SetDirty(true);
    return BSISUCCESS;
    }
#pragma optimize("", off)
template <class POINT> StatusInt ScalableMeshNodeEdit<POINT>::_AddTexturedMesh(bvector<DPoint3d>& vertices, bvector<bvector<int32_t>>& ptsIndices, bvector<DPoint2d>& uv, bvector<bvector<int32_t>>& uvIndices, size_t nTexture)
    {
    m_node->clear();
    auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, YProtPtExtentType>, SMPointIndexNode<POINT, YProtPtExtentType>>(m_node);
    m_meshNode->m_nodeHeader.m_arePoints3d = true;
    m_meshNode->m_nodeHeader.m_areTextured = true;

    size_t nIndicesCount = 0;
    vector<POINT> nodePts(vertices.size());

    for (size_t pointInd = 0; pointInd < vertices.size(); pointInd++)
        {
        nodePts[pointInd].x = vertices[pointInd].x;
        nodePts[pointInd].y = vertices[pointInd].y;
        nodePts[pointInd].z = vertices[pointInd].z;
        }
    m_meshNode->push_back(&nodePts[0], nodePts.size());
    m_meshNode->PushUV(&uv[0], uv.size());

    vector<int32_t> indicesLine;

    // Untexturing part : 
    nIndicesCount += ptsIndices[0].size();
    m_meshNode->PushPtsIndices(0, &ptsIndices[0][0], ptsIndices[0].size());    
    m_meshNode->StorePtsIndice(0);
    indicesLine.insert(indicesLine.end(), ptsIndices[0].begin(), ptsIndices[0].end());

    // Texturing points
    for(size_t i = 0; i < nTexture; i++)
        {
        nIndicesCount += ptsIndices[i+1].size();
        m_meshNode->PushPtsIndices(i+1, &ptsIndices[i+1][0], ptsIndices[i+1].size());        
        m_meshNode->StorePtsIndice(i+1);

        m_meshNode->PushUVsIndices(i, &uvIndices[i][0], uvIndices[i].size());
        m_meshNode->SetUVsIndicesDirty(i);
        m_meshNode->StoreUVsIndices(i);

        indicesLine.insert(indicesLine.end(), ptsIndices[i+1].begin(), ptsIndices[i+1].end());
        }
    m_meshNode->SetUVDirty();
    bvector<int> componentPointsId;
    if (NULL == m_meshNode->GetGraphPtr()) m_meshNode->CreateGraph();

    CreateGraphFromIndexBuffer(m_meshNode->GetGraphPtr(), (const long*)&indicesLine[0], (int)indicesLine.size(), (int)nodePts.size(), componentPointsId, &vertices[0]);
    //m_meshNode->GetGraphPtr()->SortNodesBasedOnLabel(0);
    m_meshNode->SetGraphDirty();
    m_meshNode->StoreGraph();

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
    m_meshNode->IncreaseTotalCount(m_meshNode->size());

    m_meshNode->StoreUV();
    m_meshNode->SetDirty(true);

    return BSISUCCESS;
    }
#pragma optimize("", on)
template <class POINT> StatusInt ScalableMeshNodeEdit<POINT>::_AddTextures(bvector<bvector<Byte>>& data, size_t numTextures, bool sibling)
    {
    auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, YProtPtExtentType>, SMPointIndexNode<POINT, YProtPtExtentType>>(m_node);

    if (!m_node->IsTextured())
    {
        // First node, texture are not initialised => save texture parcours all sibling node and save the texture id on it etc...
        for (int i = 0; i < numTextures; i++)
        {
            m_meshNode->PushTexture(i, &data[i][0], data[i].size());
            m_meshNode->SetTextureDirty(i);
            m_meshNode->StoreTexture(i);
        }

        auto nodeParent = dynamic_pcast<SMMeshIndexNode<POINT, YProtPtExtentType>, SMPointIndexNode<POINT, YProtPtExtentType>>(m_meshNode->GetParentNode());
        if (nodeParent != NULL)
        {
            for (int i = 0; i < nodeParent->m_apSubNodes.size(); i++)
            {
                nodeParent->m_apSubNodes[i]->m_nodeHeader.m_areTextured = true;
                nodeParent->m_apSubNodes[i]->m_nodeHeader.m_nbTextures = numTextures;
                nodeParent->m_apSubNodes[i]->m_nodeHeader.m_textureID = m_meshNode->m_nodeHeader.m_textureID;
                nodeParent->m_apSubNodes[i]->SetDirty(true);
            }
        }
    }

    return BSISUCCESS;
    }

template <class POINT> StatusInt ScalableMeshNodeEdit<POINT>::_SetNodeExtent(DRange3d& extent)
    {
    m_node->m_nodeHeader.m_nodeExtent = ExtentOp<YProtPtExtentType>::Create(extent.low.x, extent.low.y, extent.low.z, extent.high.x, extent.high.y, extent.high.z);
    return BSISUCCESS;
    }

template <class POINT> StatusInt ScalableMeshNodeEdit<POINT>::_SetContentExtent(DRange3d& extent)
    {
    m_node->m_nodeHeader.m_contentExtentDefined = true;
    m_node->m_nodeHeader.m_contentExtent = ExtentOp<YProtPtExtentType>::Create(extent.low.x, extent.low.y, extent.low.z, extent.high.x, extent.high.y, extent.high.z);
    return BSISUCCESS;
    }

template <class POINT> StatusInt ScalableMeshNodeEdit<POINT>::_SetArePoints3d(bool arePoints3d)
    {
    m_node->m_nodeHeader.m_arePoints3d = arePoints3d;
    return BSISUCCESS;
    }

template <class POINT> ScalableMeshNodeEdit<POINT>::ScalableMeshNodeEdit(HFCPtr<SMPointIndexNode<POINT, YProtPtExtentType>>& nodePtr)
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
    return m_node->Discarded();    
    }

template <class POINT> void ScalableMeshNodeEdit<POINT>::_LoadHeader() const
    {    
    return m_node->Load();    
    }


template <class POINT> ScalableMeshNodeWithReprojection<POINT>::ScalableMeshNodeWithReprojection(HFCPtr<SMPointIndexNode<POINT, YProtPtExtentType>>& nodePtr, const GeoCoords::Reprojection& reproject)
    :ScalableMeshNode<POINT>(nodePtr), m_reprojectFunction(reproject)
    {
    }

template <class POINT> ScalableMeshNodeWithReprojection<POINT>::ScalableMeshNodeWithReprojection(IScalableMeshNodePtr nodeInfo, const GeoCoords::Reprojection& reproject)
    : m_reprojectFunction(reproject)
    {
    auto nodeInfoImpl = dynamic_cast<ScalableMeshNode<POINT> *>(nodeInfo.get());
    m_node = nodeInfoImpl->GetNodePtr();
    }


template <class POINT> IScalableMeshMeshPtr ScalableMeshNodeWithReprojection<POINT>::_GetMesh(IScalableMeshMeshFlagsPtr& flags, bvector<bool>& clipsToShow) const
    {
    IScalableMeshMeshPtr meshP;
    if (flags->ShouldLoadGraph())
        {
        auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, YProtPtExtentType>, SMPointIndexNode<POINT, YProtPtExtentType>>(m_node);
        if (m_meshNode->GetGraphPtr() == NULL)
            {
            m_meshNode->LoadGraph();
            }
        POINT* pts = new POINT[m_node->size()];
        m_node->get(pts, m_node->size());

        DPoint3d* points = new DPoint3d[m_node->size()];
        m_reprojectFunction.Reproject(points, m_node->size(), points);
        std::transform(&pts[0], &pts[0] + m_node->size(), points, PtToPtConverter());
        meshP = ScalableMeshMeshWithGraph::Create(m_node->size(), points, m_node->m_nodeHeader.m_nbFaceIndexes, (int32_t*)(&pts[0] + m_node->size()), 0, nullptr, nullptr, m_meshNode->GetGraphPtr(), ArePoints3d(), 0, 0, 0);
        delete pts;
        delete points;
        }
    else
        {
        auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, YProtPtExtentType>, SMPointIndexNode<POINT, YProtPtExtentType>>(m_node);
        ScalableMeshMeshPtr meshPtr = ScalableMeshMesh::Create();

        vector<DPoint3d> dataPoints(m_node->size());

        PtToPtConverter converter;

        for (size_t pointInd = 0; pointInd < m_node->size(); pointInd++)
            {
            dataPoints[pointInd] = converter.operator()(m_node->operator[](pointInd));
            }
        m_reprojectFunction.Reproject(&dataPoints[0], m_node->size(), &dataPoints[0]);
        //int status = meshPtr->AppendMesh(m_node->size(), &dataPoints[0], m_node->m_nodeHeader.m_nbFaceIndexes, (int32_t*)&m_node->operator[](m_node->size()), 0, 0, 0);
        // NEEDS_WORK_SM : texture logique !
 /*       std::ofstream file_s;
        file_s.open("C:\\dev\\ContextCapture\\_log.txt", ios_base::app);
        file_s << "PushIndices etc... -- shit 14" << endl;*/
        RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> ptsIndices(m_meshNode->GetPtsIndicePtr());

        int status = meshPtr->AppendMesh(m_node->size(), &dataPoints[0], m_node->m_nodeHeader.m_nbFaceIndexes, &(*ptsIndices)[0], 0, 0, 0, 0, 0, 0);

        assert(status == SUCCESS);

        meshP = meshPtr.get();
        }

    return meshP;
    }

template <class POINT> IScalableMeshMeshPtr ScalableMeshNodeWithReprojection<POINT>::_GetMeshByParts(const bvector<bool>& clipsToShow, ScalableMeshTextureID texID) const
    {
    auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, YProtPtExtentType>, SMPointIndexNode<POINT, YProtPtExtentType>>(m_node);
    ScalableMeshMeshPtr meshPtr = ScalableMeshMesh::Create();

    vector<DPoint3d> dataPoints(m_node->size());

    PtToPtConverter converter;

    for (size_t pointInd = 0; pointInd < m_node->size(); pointInd++)
        {
        dataPoints[pointInd] = converter.operator()(m_node->operator[](pointInd));
        }
    m_reprojectFunction.Reproject(&dataPoints[0], m_node->size(), &dataPoints[0]);
    DPoint2d* pUv = m_meshNode->GetUVPtr();

    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> ptsIndices(m_meshNode->GetPtsIndicePtr());

    int status = meshPtr->AppendMesh(m_node->size(), &dataPoints[0], m_node->m_nodeHeader.m_nbFaceIndexes, &(*ptsIndices)[0], 0, 0, 0, texID >= 1 ? m_meshNode->GetNbUVs() : 0, texID >= 1 ? pUv : 0, texID >= 1 ? m_meshNode->GetUVsIndicesPtr(texID - 1) : 0);

    assert(status == SUCCESS);

    return meshPtr.get();
    }


template <class POINT> IScalableMeshMeshPtr ScalableMeshNodeWithReprojection<POINT>::_GetMeshByParts(const bset<uint64_t>& clipsToShow, ScalableMeshTextureID texID) const
    {
    auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, YProtPtExtentType>, SMPointIndexNode<POINT, YProtPtExtentType>>(m_node);
    ScalableMeshMeshPtr meshPtr = ScalableMeshMesh::Create();

    vector<DPoint3d> dataPoints(m_node->size());

    PtToPtConverter converter;

    for (size_t pointInd = 0; pointInd < m_node->size(); pointInd++)
        {
        dataPoints[pointInd] = converter.operator()(m_node->operator[](pointInd));
        }
    m_reprojectFunction.Reproject(&dataPoints[0], m_node->size(), &dataPoints[0]);
    DPoint2d* pUv = m_meshNode->GetUVPtr();

    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> ptsIndices(m_meshNode->GetPtsIndicePtr());

    int status = meshPtr->AppendMesh(m_node->size(), &dataPoints[0], m_node->m_nodeHeader.m_nbFaceIndexes, &(*ptsIndices)[0], 0, 0, 0, texID >= 1 ? m_meshNode->GetNbUVs() : 0, texID >= 1 ? pUv : 0, texID >= 1 ? m_meshNode->GetUVsIndicesPtr(texID - 1) : 0);

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
 
    HFCPtr<SMPointIndexNode<POINT, YProtPtExtentType>> currentNodeP(nullptr);
    ScalableMeshQuadTreeLevelIntersectIndexQuery<POINT, YProtPtExtentType> query(m_scmIndexPtr->GetContentExtent(), 
                                                                                 scmQueryParamsPtr->GetLevel() == (size_t)-1 ? m_scmIndexPtr->GetDepth() : scmQueryParamsPtr->GetLevel(), 
                                                                                 ray, 
                                                                                 params->Get2d(), 
                                                                                 params->GetDepth(), 
                                                                                 params->Get2d() ? ScalableMeshQuadTreeLevelIntersectIndexQuery<POINT, YProtPtExtentType>::RaycastOptions::FIRST_INTERSECT : ScalableMeshQuadTreeLevelIntersectIndexQuery<POINT, YProtPtExtentType>::RaycastOptions::LAST_INTERSECT);
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

    std::vector<SMPointIndexNode<POINT, YProtPtExtentType>::QueriedNode> nodesP;
    ScalableMeshQuadTreeLevelIntersectIndexQuery<POINT, YProtPtExtentType> query(m_scmIndexPtr->GetContentExtent(), 
                                                                                 scmQueryParamsPtr->GetLevel() == (size_t)-1 ? m_scmIndexPtr->GetDepth() : scmQueryParamsPtr->GetLevel(),
                                                                                 ray, 
                                                                                 params->Get2d(), 
                                                                                 params->GetDepth(), 
                                                                                 ScalableMeshQuadTreeLevelIntersectIndexQuery<POINT, YProtPtExtentType>::RaycastOptions::ALL_INTERSECT);
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

    vector<typename SMPointIndexNode<POINT, YProtPtExtentType>::QueriedNode> nodes;
    ScalableMeshQuadTreeLevelPlaneIntersectIndexQuery<POINT, YProtPtExtentType> query(m_scmIndexPtr->GetContentExtent(), m_scmIndexPtr->GetDepth(), params->GetPlane(), params->GetDepth());
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