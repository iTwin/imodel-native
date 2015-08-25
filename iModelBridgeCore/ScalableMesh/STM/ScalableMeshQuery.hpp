/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshQuery.hpp $
|    $RCSfile: ScalableMeshQuery.hpp,v $
|   $Revision: 1.23 $
|       $Date: 2012/11/29 17:30:45 $
|     $Author: Mathieu.St-Pierre $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

// This define does not work when it is set to 10000 and a dataset of 120000 is used
#define MAX_POINTS_PER_DTM 10000
#define MAX_FEATURES_PER_DTM 10000


static HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment s_queryMemoryManager(20, 1000*sizeof(DPoint3d));

template<class EXTENT> EXTENT ScalableMeshQuery::GetExtentFromClipShape(const DPoint3d* pClipShapePts, 
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

template <class POINT> int ScalableMeshQuery::AddPoints(const DTMPtr&      dtmPtr, 
                                                 const list<POINT>& pointList) const
    {         
    assert((dtmPtr != 0) && (dtmPtr->GetBcDTM() != 0) && (dtmPtr->GetBcDTM()->GetTinHandle()));
               
    DPoint3d* pPt = new DPoint3d[pointList.size()];
    std::transform(pointList.begin(), pointList.end(), pPt, ToBcPtConverter());
 
    BC_DTM_OBJ* dtmObjP(dtmPtr->GetBcDTM()->GetTinHandle());
    
    int status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMF_RANDOM_SPOT, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, pPt, pointList.size());    

    delete pPt;

    return status;
    }


template <class POINT> int ScalableMeshQuery::AddPoints(const DTMPtr&                          dtmPtr, 
                                                 const HPMMemoryManagedVector<POINT>& pointList) /*const*/
    {        
    assert((dtmPtr != 0) && (dtmPtr->GetBcDTM() != 0) && (dtmPtr->GetBcDTM()->GetTinHandle()));
               
    DPoint3d* pPt = new DPoint3d[pointList.size()];
    std::transform(pointList.begin(), pointList.end(), pPt, ToBcPtConverter());
 
    BC_DTM_OBJ* dtmObjP(dtmPtr->GetBcDTM()->GetTinHandle());

    int status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::RandomSpots, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, pPt, (long)pointList.size());
    
    delete pPt;

    return status;
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
    m_scmPointIndexPtr = pointIndexPtr;    
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
template <class POINT> int ScalableMeshFullResolutionPointQuery<POINT>::_Query(Bentley::TerrainModel::DTMPtr&           dtmPtr, 
                                                                        const DPoint3d*                 pQueryShapePts, 
                                                                        int                             nbQueryShapePts, 
                                                                        const IScalableMeshQueryParametersPtr& scmQueryParamsPtr) const
    {        
    assert(scmQueryParamsPtr != 0);

    // list<POINT> pointList;    
    HPMMemoryManagedVector<POINT> pointList(&s_queryMemoryManager);

    int         status;

    YProtPtExtentType contentExtent(m_scmPointIndexPtr->GetContentExtent());        
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
        queryExtent.xMin = ExtentOp<YProtPtExtentType>::GetXMin(contentExtent);
        queryExtent.xMax = ExtentOp<YProtPtExtentType>::GetXMax(contentExtent);
        queryExtent.yMin = ExtentOp<YProtPtExtentType>::GetYMin(contentExtent);
        queryExtent.yMax = ExtentOp<YProtPtExtentType>::GetYMax(contentExtent);
        queryExtent.zMin = ExtentOp<YProtPtExtentType>::GetZMin(contentExtent);
        queryExtent.zMax = ExtentOp<YProtPtExtentType>::GetZMax(contentExtent);
        
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

        m_scmPointIndexPtr->Query(pointQueryP.get(), pointList);

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
    
    if ((status == S_SUCCESS) && (dtmPtr == 0))
        {
        status = CreateBcDTM(dtmPtr);
        }

    if (status == S_SUCCESS)
        {
        status = AddPoints<POINT>(dtmPtr, pointList);
        }
        
    if ((status == S_SUCCESS) && (scmQueryParamsPtr->GetTriangulationState() == true))
        {
        // Since triangulation was requested, the process needs to be completed. This means that current query result
        // must be clipped according to current query object clips then triangulated.

        if ((m_clips != 0) && (m_clips->GetNbClips() > 0))
            {
            status = TriangulateDTM(dtmPtr, scmQueryParamsPtr);

            // Set clip shape to DTM
            DRange3d drange;
                                         
            drange.low.x = contentExtent.xMin;
            drange.low.y = contentExtent.yMin;
            drange.low.z = contentExtent.zMin;
            drange.high.x = contentExtent.xMax;
            drange.high.y = contentExtent.yMax;
            drange.high.z = contentExtent.zMax;           
                
            SetClipsToDTM(dtmPtr, drange, m_clips);
            }

        // Triangulate
        status = TriangulateDTM(dtmPtr, scmQueryParamsPtr);        
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
    m_scmPointIndexPtr = pointIndexPtr;
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
template <class POINT> int ScalableMeshViewDependentPointQuery<POINT>::_Query(Bentley::TerrainModel::DTMPtr&            dtmPtr, 
                                                                       const DPoint3d*                  pQueryShapePts, 
                                                                       int                              nbQueryShapePts, 
                                                                       const IScalableMeshQueryParametersPtr&  scmQueryParamsPtr) const
    {    
    //MST More validation is required here.
    assert(scmQueryParamsPtr != 0);

    // list<POINT> pointList;  
    HPMMemoryManagedVector<POINT> pointList(&s_queryMemoryManager);    

    int            status;

    YProtPtExtentType contentExtent(m_scmPointIndexPtr->GetContentExtent());    

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

        viewDependentQueryP->SetUseSameResolutionWhenCameraIsOff(queryParams->GetUseSameResolutionWhenCameraIsOff());

        viewDependentQueryP->SetUseSplitThresholdForLevelSelection(queryParams->GetUseSplitThresholdForLevelSelection());

        viewDependentQueryP->SetUseSplitThresholdForTileSelection(queryParams->GetUseSplitThresholdForTileSelection());

        //MS : Might need to be done at the ScalableMeshReprojectionQuery level.
        if ((scmQueryParamsPtr->GetSourceGCS() != 0) && (scmQueryParamsPtr->GetTargetGCS() != 0))
            {
            BaseGCSPtr sourcePtr = scmQueryParamsPtr->GetSourceGCS();
            BaseGCSPtr targetPtr = scmQueryParamsPtr->GetTargetGCS();
            viewDependentQueryP->SetReprojectionInfo(sourcePtr, targetPtr);
            }
            
            pointQueryP = viewDependentQueryP;            
        }

    DRange3d spatialIndexRange;

    YProtPtExtentType ExtentPoints = m_scmPointIndexPtr->GetContentExtent();
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
                    
        m_scmPointIndexPtr->Query(pointQueryP.get(), pointList);
       


        status = 0;

        if ((g_checkIndexingStopCallbackFP != 0) && (g_checkIndexingStopCallbackFP(0) != 0))
            {
            status = 1; 
            }




        }
    catch (...)
        {
        status = 1;
        }
   
    //TEMP    
    if ((status == SUCCESS) && (dtmPtr == 0))
        {
        status = CreateBcDTM(dtmPtr);
        }

    if (status == SUCCESS)
        {        
 
        status = AddPoints(dtmPtr, pointList);                                                 

        }
        
    if ((status == SUCCESS) && (scmQueryParamsPtr->GetTriangulationState() == true))
        {
        // Since triangulation was requested, the process needs to be completed. This means that current query result
        // must be clipped according to current query object clips then triangulated.

        if ((m_clips != 0) && (m_clips->GetNbClips() > 0))
            {
            status = TriangulateDTM(dtmPtr, scmQueryParamsPtr);


            // Set clip shape to DTM
            SetClipsToDTM(dtmPtr, spatialIndexRange, m_clips);
            }

        // Triangulate
        status = TriangulateDTM(dtmPtr, scmQueryParamsPtr);
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
    m_scmPointIndexPtr = pointIndexPtr;    
    }

template <class POINT> ScalableMeshFixResolutionViewPointQuery<POINT>::~ScalableMeshFixResolutionViewPointQuery()
    {
    }
    
template <class POINT> int ScalableMeshFixResolutionViewPointQuery<POINT>::_Query(Bentley::TerrainModel::DTMPtr&            dtmPtr, 
                                                                           const DPoint3d*                  pQueryShapePts, 
                                                                           int                              nbQueryShapePts, 
                                                                           const IScalableMeshQueryParametersPtr&  scmQueryParamsPtr) const
    {
    //MST More validation is required here.
    assert(scmQueryParamsPtr != 0);
    //MST Not supported.
    assert(pQueryShapePts == 0);

    int         status = SUCCESS;

    // list<POINT> pointList;  
    HPMMemoryManagedVector<POINT> pointList(&s_queryMemoryManager);   


    if (dynamic_cast<IScalableMeshFixResolutionIndexQueryParams*>(scmQueryParamsPtr.get()) != 0)
        {
        IScalableMeshFixResolutionIndexQueryParams* queryParams(dynamic_cast<IScalableMeshFixResolutionIndexQueryParams*>(scmQueryParamsPtr.get()));               

        dtmPtr = new ScalableMeshSingleResolutionPointIndexView<POINT>(m_scmPointIndexPtr, queryParams->GetResolutionIndex(), m_pointIndexGCS);       
        }
    else    
        {
        assert(dynamic_cast<IScalableMeshFixResolutionMaxPointsQueryParams*>(scmQueryParamsPtr.get()) != 0);

        IScalableMeshFixResolutionMaxPointsQueryParams* queryParams(dynamic_cast<IScalableMeshFixResolutionMaxPointsQueryParams*>(scmQueryParamsPtr.get()));       
          
        StatusInt status = SUCCESS;
                                          
        if (m_scmPointIndexPtr->IsEmpty() == false)
            {                    
            __int64 nbObjectsForLevel;            
            int     resolutionIndex = 0;
            
            size_t  nbResolution = m_scmPointIndexPtr->GetDepth();        
            
            for (; resolutionIndex <= (int)nbResolution; resolutionIndex++)
                    {
                    nbObjectsForLevel = m_scmPointIndexPtr->GetNbObjectsAtLevel(resolutionIndex);
                    
                    if (nbObjectsForLevel > queryParams->GetMaxNumberPoints())
                        {                
                        break;
                        }
                    }   

            resolutionIndex--; 

            if (resolutionIndex == -1)
                {
                status = ERROR;
                }
            else
                {
                dtmPtr = new ScalableMeshSingleResolutionPointIndexView<POINT>(m_scmPointIndexPtr, resolutionIndex, m_pointIndexGCS);
                }   
            }  
        else
            {
            status =  ERROR;
            }                   
        }

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
    m_scmPointIndexPtr = pointIndexPtr;    
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

    // list<IDTMFile::Point3d64f> pointList;  
    HPMMemoryManagedVector<POINT> pointList(&s_queryMemoryManager);    

    int            status;

    YProtPtExtentType contentExtent(m_scmPointIndexPtr->GetContentExtent());    

    double minZ = ExtentOp<YProtPtExtentType>::GetZMin(contentExtent);
    double maxZ = ExtentOp<YProtPtExtentType>::GetZMax(contentExtent);
        
    YProtPtExtentType queryExtent(ScalableMeshQuery::GetExtentFromClipShape<YProtPtExtentType>(pQueryExtentPts,
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
                                                                                                 scmViewDependentParamsPtr->GetGatherQueriedNodeBoundaries(),
                                                                                                 scmViewDependentParamsPtr->GetViewClipVector(),
                                                                                                 maxNbPoints));                        

    // viewDependentQueryP->SetTracingXMLFileName(AString("E:\\MyDoc\\SS3 - Iteration 17\\STM\\Bad Resolution Selection\\visitingNodes.xml"));

    viewDependentQueryP->SetMeanScreenPixelsPerPoint(scmViewDependentParamsPtr->GetMinScreenPixelsPerPoint());

    viewDependentQueryP->SetUseSameResolutionWhenCameraIsOff(scmViewDependentParamsPtr->GetUseSameResolutionWhenCameraIsOff());

    viewDependentQueryP->SetUseSplitThresholdForLevelSelection(scmViewDependentParamsPtr->GetUseSplitThresholdForLevelSelection());

    viewDependentQueryP->SetUseSplitThresholdForTileSelection(scmViewDependentParamsPtr->GetUseSplitThresholdForTileSelection());

    //MS : Might need to be done at the ScalableMeshReprojectionQuery level.
    
    if ((scmQueryParamsPtr->GetSourceGCS() != 0) && (scmQueryParamsPtr->GetTargetGCS() != 0))
        {
        BaseGCSPtr sourcePtr = scmQueryParamsPtr->GetSourceGCS();
        BaseGCSPtr targetPtr = scmQueryParamsPtr->GetTargetGCS();
        viewDependentQueryP->SetReprojectionInfo(sourcePtr, targetPtr);
        }
        
    pointQueryP = viewDependentQueryP;                    

    DRange3d spatialIndexRange;

    YProtPtExtentType ExtentPoints = m_scmPointIndexPtr->GetContentExtent();
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
         
            
        if (m_scmPointIndexPtr->Query(pointQueryP.get(), returnMeshPtr.get()))
            {       
            meshPtr = returnMeshPtr;

            if (scmViewDependentParamsPtr->GetGatherQueriedNodeBoundaries())
                {                                        
            //    mrDTMQueryParamsPtr->SetQueriedNodeBoundaries(viewDependentQueryP->GetListOfTileBreaklines());
                }

            status = SUCCESS;
            }
        else
            {
            status = ERROR;
            }

        if ((g_checkIndexingStopCallbackFP != 0) && (g_checkIndexingStopCallbackFP(0) != 0))
            {
            status = 1; 
            }
        }
    catch (...)
        {
        status = 1;
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

    // list<IDTMFile::Point3d64f> pointList;  
    HPMMemoryManagedVector<POINT> pointList(&s_queryMemoryManager);    

    int            status;

    YProtPtExtentType contentExtent(m_scmPointIndexPtr->GetContentExtent());    

    double minZ = ExtentOp<YProtPtExtentType>::GetZMin(contentExtent);
    double maxZ = ExtentOp<YProtPtExtentType>::GetZMax(contentExtent);
        
    YProtPtExtentType queryExtent(ScalableMeshQuery::GetExtentFromClipShape<YProtPtExtentType>(pQueryExtentPts,
        nbQueryExtentPts,
                                                                                        minZ,
                                                                                        maxZ));

    HAutoPtr<ISMPointIndexQuery<POINT, YProtPtExtentType>> pointQueryP;
             
    //MS Need to be removed
    double viewportRotMatrix[3][3];        
    double rootToViewMatrix[4][4];        

    IScalableMeshViewDependentMeshQueryParamsPtr scmViewDependentParamsPtr = IScalableMeshViewDependentMeshQueryParamsPtr(dynamic_cast<ScalableMeshViewDependentMeshQueryParams*>(scmQueryParamsPtr.get()));

    memcpy(rootToViewMatrix, scmViewDependentParamsPtr->GetRootToViewMatrix(), sizeof(double) * 4 * 4);

    static size_t maxNbPoints = 150000000000;
                    
    ScalableMeshQuadTreeViewDependentMeshQuery<POINT, YProtPtExtentType>* viewDependentQueryP(new ScalableMeshQuadTreeViewDependentMeshQuery<POINT, YProtPtExtentType>
                                                                                                (queryExtent, 
                                                                                                 rootToViewMatrix,                   
                                                                                                 viewportRotMatrix,             
                                                                                                 scmViewDependentParamsPtr->GetViewBox(),
                                                                                                 scmViewDependentParamsPtr->GetGatherQueriedNodeBoundaries(),
                                                                                                 scmViewDependentParamsPtr->GetViewClipVector(),
                                                                                                 maxNbPoints));                        

    // viewDependentQueryP->SetTracingXMLFileName(AString("E:\\MyDoc\\SS3 - Iteration 17\\STM\\Bad Resolution Selection\\visitingNodes.xml"));

    viewDependentQueryP->SetMeanScreenPixelsPerPoint(scmViewDependentParamsPtr->GetMinScreenPixelsPerPoint());

    viewDependentQueryP->SetUseSameResolutionWhenCameraIsOff(scmViewDependentParamsPtr->GetUseSameResolutionWhenCameraIsOff());

    viewDependentQueryP->SetUseSplitThresholdForLevelSelection(scmViewDependentParamsPtr->GetUseSplitThresholdForLevelSelection());

    viewDependentQueryP->SetUseSplitThresholdForTileSelection(scmViewDependentParamsPtr->GetUseSplitThresholdForTileSelection());

    //MS : Might need to be done at the ScalableMeshReprojectionQuery level.
    
    if ((scmQueryParamsPtr->GetSourceGCS() != 0) && (scmQueryParamsPtr->GetTargetGCS() != 0))
        {
        BaseGCSPtr sourcePtr = scmQueryParamsPtr->GetSourceGCS();
        BaseGCSPtr targetPtr = scmQueryParamsPtr->GetTargetGCS();
        viewDependentQueryP->SetReprojectionInfo(sourcePtr, targetPtr);
        }
        
    pointQueryP = viewDependentQueryP;                    

    DRange3d spatialIndexRange;

    YProtPtExtentType ExtentPoints = m_scmPointIndexPtr->GetContentExtent();
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

        if (m_scmPointIndexPtr->Query(pointQueryP.get(), returnedMeshNodes))
            {           
            meshNodes.resize(returnedMeshNodes.size());
            for (size_t nodeInd = 0; nodeInd < returnedMeshNodes.size(); nodeInd++)
                {
                meshNodes[nodeInd] = new ScalableMeshNode<POINT>(returnedMeshNodes[nodeInd].m_indexNode);
                }            

            if (scmViewDependentParamsPtr->GetGatherQueriedNodeBoundaries())
                {                                        
            //    mrDTMQueryParamsPtr->SetQueriedNodeBoundaries(viewDependentQueryP->GetListOfTileBreaklines());
                }

            status = SUCCESS;
            }
        else
            {
            status = ERROR;
            }

        if ((g_checkIndexingStopCallbackFP != 0) && (g_checkIndexingStopCallbackFP(0) != 0))
            {
            status = 1; 
            }
        }
    catch (...)
        {
        status = 1;
        }

    return status;    
    }

template <class POINT> ScalableMeshFullResolutionMeshQuery<POINT>::ScalableMeshFullResolutionMeshQuery(const HFCPtr<SMPointIndex<POINT, YProtPtExtentType>>& pointIndexPtr)
    {
    m_scmPointIndexPtr = pointIndexPtr;
    }


template <class POINT> ScalableMeshFullResolutionMeshQuery<POINT>::~ScalableMeshFullResolutionMeshQuery()
    {}


template <class POINT> int ScalableMeshFullResolutionMeshQuery<POINT>::_Query(IScalableMeshMeshPtr&                                meshPtr,
                                                                       const DPoint3d*                               pQueryExtentPts,
                                                                       int                                           nbQueryExtentPts,
                                                                       const IScalableMeshMeshQueryParamsPtr&  scmQueryParamsPtr) const
    {
    YProtPtExtentType contentExtent(m_scmPointIndexPtr->GetContentExtent());

    double minZ = ExtentOp<YProtPtExtentType>::GetZMin(contentExtent);
    double maxZ = ExtentOp<YProtPtExtentType>::GetZMax(contentExtent);

    YProtPtExtentType queryExtent(ScalableMeshQuery::GetExtentFromClipShape<YProtPtExtentType>(pQueryExtentPts,
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
    ScalableMeshQuadTreeLevelMeshIndexQuery<POINT, YProtPtExtentType>* meshQueryP(new ScalableMeshQuadTreeLevelMeshIndexQuery<POINT, YProtPtExtentType>(queryExtent, m_scmPointIndexPtr->GetDepth(), box));

    try
        {
        ScalableMeshMeshPtr returnMeshPtr(ScalableMeshMesh::Create());


        if (m_scmPointIndexPtr->Query(meshQueryP, returnMeshPtr.get()))
            {
            meshPtr = returnMeshPtr;

            status = SUCCESS;
            }
        else
            {
            status = ERROR;
            }

        if ((g_checkIndexingStopCallbackFP != 0) && (g_checkIndexingStopCallbackFP(0) != 0))
            {
            status = 1;
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
    YProtPtExtentType contentExtent(m_scmPointIndexPtr->GetContentExtent());

    double minZ = ExtentOp<YProtPtExtentType>::GetZMin(contentExtent);
    double maxZ = ExtentOp<YProtPtExtentType>::GetZMax(contentExtent);

    YProtPtExtentType queryExtent(ScalableMeshQuery::GetExtentFromClipShape<YProtPtExtentType>(pQueryExtentPts,
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
    ScalableMeshQuadTreeLevelMeshIndexQuery<POINT, YProtPtExtentType>* meshQueryP(new ScalableMeshQuadTreeLevelMeshIndexQuery<POINT, YProtPtExtentType>(queryExtent, m_scmPointIndexPtr->GetDepth(), box));
    try
        {

        vector<typename SMPointIndexNode<POINT, YProtPtExtentType>::QueriedNode> returnedMeshNodes;

        if (m_scmPointIndexPtr->Query(meshQueryP, returnedMeshNodes))
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

        if ((g_checkIndexingStopCallbackFP != 0) && (g_checkIndexingStopCallbackFP(0) != 0))
            {
            status = 1;
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
                                               m_scmPointIndexPtr(indexPtr)
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
        HFCPtr<HVE2DShape> pReprojectedShape = ReprojectShapeDomainLimited((BaseGCSPtr&)m_sourceGCS.GetGeoRef().GetBasePtr(),
                                                                           (BaseGCSPtr&)m_targetGCS.GetGeoRef().GetBasePtr(), pQueryExtentPts, nbQueryExtentPts);

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
        HFCPtr<HVE2DShape> pReprojectedShape = ReprojectShapeDomainLimited((BaseGCSPtr&)m_sourceGCS.GetGeoRef().GetBasePtr(),
                                                                           (BaseGCSPtr&)m_targetGCS.GetGeoRef().GetBasePtr(), pQueryExtentPts, nbQueryExtentPts);

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
    GeoCoordinates::BaseGCSPtr targetGCSP = &*m_targetGCS.GetGeoRef().GetBasePtr();
    GeoCoordinates::BaseGCSPtr sourceGCSP = &*m_sourceGCS.GetGeoRef().GetBasePtr();
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
    m_scmPointIndexPtr = pointIndexPtr;
    }

template <class POINT> ScalableMeshNodeRayQuery<POINT>::~ScalableMeshNodeRayQuery()
    {
    }

template <class POINT> ScalableMeshNodePlaneQuery<POINT>::ScalableMeshNodePlaneQuery(const HFCPtr<SMPointIndex<POINT, YProtPtExtentType>>& pointIndexPtr)
    {
    m_scmPointIndexPtr = pointIndexPtr;
    }

template <class POINT> ScalableMeshNodePlaneQuery<POINT>::~ScalableMeshNodePlaneQuery()
    {}

template <class POINT> ScalableMeshNode<POINT>::ScalableMeshNode(HFCPtr<SMPointIndexNode<POINT, YProtPtExtentType>>& nodePtr)
    {
    if (!nodePtr->IsLoaded())
        nodePtr->Load();
    m_node = nodePtr;
    }

template <class POINT> bool ScalableMeshNode<POINT>::_ArePoints3d() const
    {
    return m_node->m_nodeHeader.m_arePoints3d;
    }

template <class POINT> bool ScalableMeshNode<POINT>::_ArePointsFullResolution() const
    {
    return m_node->m_nodeHeader.m_IsLeaf;
    }

template <class POINT> IScalableMeshMeshPtr ScalableMeshNode<POINT>::_GetMesh(bool loadGraph) const
    {
    m_node->Pin();

    IScalableMeshMeshPtr meshP;
    if (loadGraph)
        {
        auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, YProtPtExtentType>, SMPointIndexNode<POINT, YProtPtExtentType>>(m_node);
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
        vector<DPoint3d> dataPoints(m_node->size());

        ToBcPtConverter converter;
        
        for (size_t pointInd = 0; pointInd < m_node->size(); pointInd++)
            {
            dataPoints[pointInd] = converter.operator()(m_node->operator[](pointInd));
            }
        ScalableMeshMeshWithGraphPtr meshPtr = ScalableMeshMeshWithGraph::Create(m_meshNode->GetGraphPtr(), ArePoints3d());
        int status = meshPtr->AppendMesh(m_node->size(), &dataPoints[0], m_node->m_nodeHeader.m_nbFaceIndexes, (int32_t*)&m_node->operator[](m_node->size()), 0, 0, 0);
        assert(status == SUCCESS);
        meshP = meshPtr.get();
        }
    else
        {
        ScalableMeshMeshPtr meshPtr = ScalableMeshMesh::Create();
        
        vector<DPoint3d> dataPoints(m_node->size());

        ToBcPtConverter converter; 

        for (size_t pointInd = 0; pointInd < m_node->size(); pointInd++)
            {
            dataPoints[pointInd] = converter.operator()(m_node->operator[](pointInd));                                            
            }

        int status = meshPtr->AppendMesh(m_node->size(), &dataPoints[0], m_node->m_nodeHeader.m_nbFaceIndexes, (int32_t*)&m_node->operator[](m_node->size()), 0, 0, 0);
        
        assert(status == SUCCESS);        

        meshP = meshPtr.get();
        }

    m_node->UnPin();

    return meshP;    
    }

template <class POINT>bvector<IScalableMeshNodePtr> ScalableMeshNode<POINT>::_GetNeighborAt(char relativePosX, char relativePosY, char relativePosZ) const
    {
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
    return m_node->m_nodeHeader.m_level;
    }

template <class POINT> DRange3d ScalableMeshNode<POINT>::_GetNodeExtent() const
    {
    DRange3d range;
    YProtPtExtentType ext = m_node->m_nodeHeader.m_nodeExtent;
    range = DRange3d::From(DPoint3d::From(ExtentOp<YProtPtExtentType>::GetXMin(ext), ExtentOp<YProtPtExtentType>::GetYMin(ext), ExtentOp<YProtPtExtentType>::GetZMin(ext)),
                           DPoint3d::From(ExtentOp<YProtPtExtentType>::GetXMax(ext), ExtentOp<YProtPtExtentType>::GetYMax(ext), ExtentOp<YProtPtExtentType>::GetZMax(ext)));
    return range;
    }

template <class POINT> DRange3d ScalableMeshNode<POINT>::_GetContentExtent() const
    {
    DRange3d range;
    YProtPtExtentType ext = m_node->m_nodeHeader.m_contentExtent;
    range = DRange3d::From(DPoint3d::From(ExtentOp<YProtPtExtentType>::GetXMin(ext), ExtentOp<YProtPtExtentType>::GetYMin(ext), ExtentOp<YProtPtExtentType>::GetZMin(ext)),
                           DPoint3d::From(ExtentOp<YProtPtExtentType>::GetXMax(ext), ExtentOp<YProtPtExtentType>::GetYMax(ext), ExtentOp<YProtPtExtentType>::GetZMax(ext)));
    return range;
    }

template <class POINT> __int64 ScalableMeshNode<POINT>::_GetNodeId() const
    {
    return m_node->GetBlockID().m_integerID;
    }

template <class POINT> size_t ScalableMeshNode<POINT>::_GetPointCount() const
    {
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
    return m_node->Discarded();    
    }

template <class POINT> void ScalableMeshNode<POINT>::_LoadHeader() const
    {    
    return m_node->Load();    
    }

template <class POINT> StatusInt ScalableMeshNodeEdit<POINT>::_AddMesh(DPoint3d* vertices, size_t nVertices, int32_t* indices, size_t nIndices)
    {
    m_node->clear();
    auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, YProtPtExtentType>, SMPointIndexNode<POINT, YProtPtExtentType>>(m_node);
    m_meshNode->m_nodeHeader.m_arePoints3d = true;
    vector<POINT> nodePts(nVertices);

    for (size_t pointInd = 0; pointInd < nVertices; pointInd++)
        {
        nodePts[pointInd].x = vertices[pointInd].x;
        nodePts[pointInd].y = vertices[pointInd].y;
        nodePts[pointInd].z = vertices[pointInd].z;
        }

    m_meshNode->push_back(&nodePts[0], nodePts.size());

    m_meshNode->m_nodeHeader.m_nbFaceIndexes = nIndices;
    bvector<int> componentPointsId;
    if (NULL == m_meshNode->GetGraphPtr()) m_meshNode->CreateGraph();
    CreateGraphFromIndexBuffer(m_meshNode->GetGraphPtr(), (const long*)indices, (int)nIndices, (int)nodePts.size(), componentPointsId, vertices);
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

    //NEEDS_WORK_SM - Ugly hack, piggyback the face indexes at the end of the points
    size_t nbPointsForFaceInd = (size_t)ceil((m_meshNode->m_nodeHeader.m_nbFaceIndexes * (double)sizeof(int32_t)) / (double)sizeof(POINT));

    POINT* pPiggyBackMeshIndexes = new POINT[nbPointsForFaceInd];

    memcpy(pPiggyBackMeshIndexes, indices, m_meshNode->m_nodeHeader.m_nbFaceIndexes * sizeof(int32_t));

    m_meshNode->push_back(pPiggyBackMeshIndexes, nbPointsForFaceInd);

    m_meshNode->setNbPointsUsedForMeshIndex(nbPointsForFaceInd);
                        
    delete[] pPiggyBackMeshIndexes;
    m_meshNode->IncreaseTotalCount(m_meshNode->size());

    m_meshNode->SetDirty(true);
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


template <class POINT> ScalableMeshNodeEdit<POINT>::ScalableMeshNodeEdit(HFCPtr<SMPointIndexNode<POINT, YProtPtExtentType>>& nodePtr)
    {
    if (!nodePtr->IsLoaded())
        nodePtr->Load();
    m_node = nodePtr;
    }

template <class POINT> bool ScalableMeshNodeEdit<POINT>::_ArePoints3d() const
    {
    return m_node->m_nodeHeader.m_arePoints3d;
    }

template <class POINT> bool ScalableMeshNodeEdit<POINT>::_ArePointsFullResolution() const
    {
    return m_node->m_nodeHeader.m_IsLeaf;
    }

template <class POINT> IScalableMeshMeshPtr ScalableMeshNodeEdit<POINT>::_GetMesh(bool loadGraph) const
    {
    IScalableMeshMeshPtr meshP;
    if (loadGraph)
        {
        auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, YProtPtExtentType>, SMPointIndexNode<POINT, YProtPtExtentType>>(m_node);
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
        vector<DPoint3d> dataPoints(m_node->size());

        ToBcPtConverter converter;

        for (size_t pointInd = 0; pointInd < m_node->size(); pointInd++)
            {
            dataPoints[pointInd] = converter.operator()(m_node->operator[](pointInd));
            }
        ScalableMeshMeshWithGraphPtr meshPtr = ScalableMeshMeshWithGraph::Create(m_meshNode->GetGraphPtr(), ArePoints3d());
        int status = meshPtr->AppendMesh(m_node->size(), &dataPoints[0], m_node->m_nodeHeader.m_nbFaceIndexes, (int32_t*)&m_node->operator[](m_node->size()), 0, 0, 0);
        assert(status == SUCCESS);
        meshP = meshPtr.get();
        }
    else
        {
        ScalableMeshMeshPtr meshPtr = ScalableMeshMesh::Create();

        vector<DPoint3d> dataPoints(m_node->size());

        ToBcPtConverter converter;

        for (size_t pointInd = 0; pointInd < m_node->size(); pointInd++)
            {
            dataPoints[pointInd] = converter.operator()(m_node->operator[](pointInd));
            }

        int status = meshPtr->AppendMesh(m_node->size(), &dataPoints[0], m_node->m_nodeHeader.m_nbFaceIndexes, (int32_t*)&m_node->operator[](m_node->size()), 0, 0, 0);

        assert(status == SUCCESS);

        meshP = meshPtr.get();
        }

    return meshP;
    }

template <class POINT>bvector<IScalableMeshNodePtr> ScalableMeshNodeEdit<POINT>::_GetNeighborAt(char relativePosX, char relativePosY, char relativePosZ) const
    {
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

template <class POINT> DRange3d ScalableMeshNodeEdit<POINT>::_GetNodeExtent() const
    {
    DRange3d range;
    YProtPtExtentType ext = m_node->m_nodeHeader.m_nodeExtent;
    range = DRange3d::From(DPoint3d::From(ExtentOp<YProtPtExtentType>::GetXMin(ext), ExtentOp<YProtPtExtentType>::GetYMin(ext), ExtentOp<YProtPtExtentType>::GetZMin(ext)),
                           DPoint3d::From(ExtentOp<YProtPtExtentType>::GetXMax(ext), ExtentOp<YProtPtExtentType>::GetYMax(ext), ExtentOp<YProtPtExtentType>::GetZMax(ext)));
    return range;
    }

template <class POINT> DRange3d ScalableMeshNodeEdit<POINT>::_GetContentExtent() const
    {
    DRange3d range;
    YProtPtExtentType ext = m_node->m_nodeHeader.m_contentExtent;
    range = DRange3d::From(DPoint3d::From(ExtentOp<YProtPtExtentType>::GetXMin(ext), ExtentOp<YProtPtExtentType>::GetYMin(ext), ExtentOp<YProtPtExtentType>::GetZMin(ext)),
                           DPoint3d::From(ExtentOp<YProtPtExtentType>::GetXMax(ext), ExtentOp<YProtPtExtentType>::GetYMax(ext), ExtentOp<YProtPtExtentType>::GetZMax(ext)));
    return range;
    }

template <class POINT> __int64 ScalableMeshNodeEdit<POINT>::_GetNodeId() const
    {
    return m_node->GetBlockID().m_integerID;
    }

template <class POINT> size_t ScalableMeshNodeEdit<POINT>::_GetLevel() const
    {
    return m_node->m_nodeHeader.m_level;
    }

template <class POINT> size_t ScalableMeshNodeEdit<POINT>::_GetPointCount() const
    {
    //return m_node->GetNbObjects();
    //NEEDSWORK_SM: GetNbObjects() is sometimes 0 here
    return m_node->size();
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


template <class POINT> IScalableMeshMeshPtr ScalableMeshNodeWithReprojection<POINT>::_GetMesh(bool loadGraph) const
    {
    IScalableMeshMeshPtr meshP;
    if (loadGraph)
        {
        auto m_meshNode = dynamic_pcast<SMMeshIndexNode<POINT, YProtPtExtentType>, SMPointIndexNode<POINT, YProtPtExtentType>>(m_node);
        if (m_meshNode->GetGraphPtr() == NULL)
            {
            m_meshNode->LoadGraph();
            }
        POINT* pts = new POINT[m_node->sizeTotal()];
        m_node->get(pts, m_node->sizeTotal());

        DPoint3d* points = new DPoint3d[m_node->size()];
        m_reprojectFunction.Reproject(points, m_node->size(), points);
        std::transform(&pts[0], &pts[0] + m_node->size(), points, ToBcPtConverter());
        meshP = ScalableMeshMeshWithGraph::Create(m_node->size(), points, m_node->m_nodeHeader.m_nbFaceIndexes, (int32_t*)(&pts[0] + m_node->size()), 0, nullptr, nullptr, m_meshNode->GetGraphPtr(), ArePoints3d());
        delete pts;
        delete points;
        }
    else
        {
        ScalableMeshMeshPtr meshPtr = ScalableMeshMesh::Create();

        vector<DPoint3d> dataPoints(m_node->size());

        ToBcPtConverter converter;

        for (size_t pointInd = 0; pointInd < m_node->size(); pointInd++)
            {
            dataPoints[pointInd] = converter.operator()(m_node->operator[](pointInd));
            }
        m_reprojectFunction.Reproject(&dataPoints[0], m_node->size(), &dataPoints[0]);
        int status = meshPtr->AppendMesh(m_node->size(), &dataPoints[0], m_node->m_nodeHeader.m_nbFaceIndexes, (int32_t*)&m_node->operator[](m_node->size()), 0, 0, 0);

        assert(status == SUCCESS);

        meshP = meshPtr.get();
        }

    return meshP;
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
    ScalableMeshQuadTreeLevelIntersectIndexQuery<POINT, YProtPtExtentType> query(m_scmPointIndexPtr->GetContentExtent(), m_scmPointIndexPtr->GetDepth(), ray, params->Get2d(), params->GetDepth(), params->Get2d() ? ScalableMeshQuadTreeLevelIntersectIndexQuery<POINT, YProtPtExtentType>::RaycastOptions::FIRST_INTERSECT : ScalableMeshQuadTreeLevelIntersectIndexQuery<POINT, YProtPtExtentType>::RaycastOptions::LAST_INTERSECT);
    m_scmPointIndexPtr->Query(&query, currentNodeP);
    if (currentNodeP == nullptr) return ERROR;
    nodePtr = IScalableMeshNodePtr(new ScalableMeshNode<POINT>(currentNodeP));
    return SUCCESS;
    }

template <class POINT> int ScalableMeshNodePlaneQuery<POINT>::_Query(bvector<IScalableMeshNodePtr>&                       meshNodesPtr,
                                                                   const DPoint3d*                              pQueryExtentPts,
                                                                   int                                          nbQueryExtentPts,
                                                                   const IScalableMeshMeshQueryParamsPtr& scmQueryParamsPtr) const
    {
    ScalableMeshNodePlaneQueryParams* params = (ScalableMeshNodePlaneQueryParams*)scmQueryParamsPtr.get();

    vector<typename SMPointIndexNode<POINT, YProtPtExtentType>::QueriedNode> nodes;
    ScalableMeshQuadTreeLevelPlaneIntersectIndexQuery<POINT, YProtPtExtentType> query(m_scmPointIndexPtr->GetContentExtent(), m_scmPointIndexPtr->GetDepth(), params->GetPlane(), params->GetDepth());
    m_scmPointIndexPtr->Query(&query, nodes);
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