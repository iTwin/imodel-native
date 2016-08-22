/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/MrDTMQuery.hpp $
|    $RCSfile: MrDTMQuery.hpp,v $
|   $Revision: 1.23 $
|       $Date: 2012/11/29 17:30:45 $
|     $Author: Mathieu.St-Pierre $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|																		|
|	MrDTMNewFileCreator.h    		  	    		(C) Copyright 2001.		|
|												BCIVIL Corporation.		|
|												All Rights Reserved.	|
|                                                                       |
+----------------------------------------------------------------------*/

#pragma once

#include "MrDTMQuery.h"
#include "InternalUtilityFunctions.h"
#include <ImagePP/all/h/HPMPooledVector.h>

// This define does not work when it is set to 10000 and a dataset of 120000 is used
#define MAX_POINTS_PER_DTM 10000
#define MAX_FEATURES_PER_DTM 10000


static HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment s_queryMemoryManager(20, 1000*sizeof(DPoint3d));

template<class EXTENT> EXTENT MrDTMQuery::GetExtentFromClipShape(const DPoint3d* pClipShapePts, 
                                                                 int             nbClipShapePts, 
                                                                 double          zMin, 
                                                                 double          zMax) const
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

template <class POINT> int MrDTMQuery::AddPoints(const DTMPtr&      dtmPtr, 
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


template <class POINT> int MrDTMQuery::AddPoints(const DTMPtr&                          dtmPtr, 
                                                 const HPMMemoryManagedVector<POINT>& pointList) const
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
|MrDTMFullResolutionPointQuery Method Definition Section - Begin
+----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------+
|MrDTMFullResolutionPointQuery::MrDTMFullResolutionPointQuery
+----------------------------------------------------------------------------*/
template <class POINT> MrDTMFullResolutionPointQuery<POINT>::MrDTMFullResolutionPointQuery(const HFCPtr<HGFPointIndex<POINT, YProtPtExtentType>>& pointIndexPtr, 
                                                                                           int                                              resolutionIndex)
    {        
    m_mrDTMPointIndexPtr = pointIndexPtr;    
    m_resolutionIndex = resolutionIndex;
    }
/*----------------------------------------------------------------------------+
|MrDTMFullResolutionPointQuery::~MrDTMFullResolutionPointQuery
+----------------------------------------------------------------------------*/
template <class POINT> MrDTMFullResolutionPointQuery<POINT>::~MrDTMFullResolutionPointQuery()
    {
    }
   
/*----------------------------------------------------------------------------+
|MrDTMFullResolutionPointQuery::Query
+----------------------------------------------------------------------------*/
template <class POINT> int MrDTMFullResolutionPointQuery<POINT>::_Query(Bentley::TerrainModel::DTMPtr&           dtmPtr, 
                                                                        const DPoint3d*                 pQueryShapePts, 
                                                                        int                             nbQueryShapePts, 
                                                                        const IMrDTMQueryParametersPtr& mrDTMQueryParamsPtr) const
    {        
    assert(mrDTMQueryParamsPtr != 0);

    // list<POINT> pointList;    
    HPMMemoryManagedVector<POINT> pointList(&s_queryMemoryManager);

    int         status;

    YProtPtExtentType contentExtent(m_mrDTMPointIndexPtr->GetContentExtent());        
    YProtPtExtentType queryExtent;
    
    HFCPtr<HVEShape> clipShapePtr;
    HAutoPtr<IHGFPointIndexQuery<POINT, YProtPtExtentType>> pointQueryP;

    assert(dynamic_cast<IMrDTMFullResolutionQueryParams*>(mrDTMQueryParamsPtr.get()) != 0);
    IMrDTMFullResolutionQueryParams* queryParams(dynamic_cast<IMrDTMFullResolutionQueryParams*>(mrDTMQueryParamsPtr.get()));        

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
        //pointQueryP->SetTracingXMLFileName(AString("E:\\MyDoc\\SS3 - Iteration 1\\MrDTM\\New 3D Query\\Log\\CheckedNodesDuringLastQuery.xml"));                 
    #endif                          

        m_mrDTMPointIndexPtr->Query(pointQueryP.get(), pointList);

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
        
    if ((status == S_SUCCESS) && (mrDTMQueryParamsPtr->GetTriangulationState() == true))
        {
        // Since triangulation was requested, the process needs to be completed. This means that current query result
        // must be clipped according to current query object clips then triangulated.

        if ((m_clips != 0) && (m_clips->GetNbClips() > 0))
            {
            status = TriangulateDTM(dtmPtr, mrDTMQueryParamsPtr);

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
        status = TriangulateDTM(dtmPtr, mrDTMQueryParamsPtr);        
        }
   
    return status;
    }

/*----------------------------------------------------------------------------+
|MrDTMFullResolutionPointQuery Method Definition Section - End
+----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------+
|MrDTMViewDependentPointQuery Method Definition Section - Begin
+----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------+
|MrDTMViewDependentPointQuery::MrDTMViewDependentPointQuery
+----------------------------------------------------------------------------*/
template <class POINT> MrDTMViewDependentPointQuery<POINT>::MrDTMViewDependentPointQuery(const HFCPtr<HGFPointIndex<POINT, YProtPtExtentType>>& pointIndexPtr)
    {        
    m_mrDTMPointIndexPtr = pointIndexPtr;
    m_resolutionIndex    = INT_MAX;          
    }       

/*----------------------------------------------------------------------------+
|MrDTMViewDependentPointQuery::~MrDTMViewDependentPointQuery
+----------------------------------------------------------------------------*/
template <class POINT> MrDTMViewDependentPointQuery<POINT>::~MrDTMViewDependentPointQuery()
    {
    }

/*----------------------------------------------------------------------------+
|MrDTMViewDependentPointQuery::SetResolutionIndex
+----------------------------------------------------------------------------*/
template <class POINT> void MrDTMViewDependentPointQuery<POINT>::SetResolutionIndex(int resolutionIndex)
    {
    m_resolutionIndex = resolutionIndex;
    }
   
/*----------------------------------------------------------------------------+
|MrDTMViewDependentPointQuery::Query
+----------------------------------------------------------------------------*/
template <class POINT> int MrDTMViewDependentPointQuery<POINT>::_Query(Bentley::TerrainModel::DTMPtr&            dtmPtr, 
                                                                       const DPoint3d*                  pQueryShapePts, 
                                                                       int                              nbQueryShapePts, 
                                                                       const IMrDTMQueryParametersPtr&  mrDTMQueryParamsPtr) const
    {    
    //MST More validation is required here.
    assert(mrDTMQueryParamsPtr != 0);

    // list<POINT> pointList;  
    HPMMemoryManagedVector<POINT> pointList(&s_queryMemoryManager);    

    int            status;

    YProtPtExtentType contentExtent(m_mrDTMPointIndexPtr->GetContentExtent());    

    double minZ = ExtentOp<YProtPtExtentType>::GetZMin(contentExtent);
    double maxZ = ExtentOp<YProtPtExtentType>::GetZMax(contentExtent);
    
    YProtPtExtentType queryExtent(GetExtentFromClipShape<YProtPtExtentType>(pQueryShapePts, 
                                                                            nbQueryShapePts, 
                                                                            minZ,
                                                                            maxZ));

    HAutoPtr<IHGFPointIndexQuery<POINT, YProtPtExtentType>> pointQueryP;

    if (m_resolutionIndex != INT_MAX)
        {
        assert(dynamic_cast<ISrDTMViewDependentQueryParams*>(mrDTMQueryParamsPtr.get()) != 0);
        assert((mrDTMQueryParamsPtr->GetSourceGCS() == 0) && (mrDTMQueryParamsPtr->GetTargetGCS() == 0));

        ISrDTMViewDependentQueryParams* queryParams(dynamic_cast<ISrDTMViewDependentQueryParams*>(mrDTMQueryParamsPtr.get()));        

        pointQueryP = new MrDTMQuadTreeLevelPointIndexQuery<POINT, YProtPtExtentType>(queryExtent, 
                                                                                      m_resolutionIndex,
                                                                                      queryParams->GetViewBox());
        }
    else        
        {
        assert(dynamic_cast<IMrDTMViewDependentQueryParams*>(mrDTMQueryParamsPtr.get()) != 0);

        IMrDTMViewDependentQueryParams* queryParams(dynamic_cast<IMrDTMViewDependentQueryParams*>(mrDTMQueryParamsPtr.get()));       

        //MS Need to be removed
        double viewportRotMatrix[3][3];        
        double rootToViewMatrix[4][4];        

        memcpy(rootToViewMatrix, queryParams->GetRootToViewMatrix(), sizeof(double) * 4 * 4);
                        
        MrDTMQuadTreeViewDependentPointQuery<POINT, YProtPtExtentType>* viewDependentQueryP(new MrDTMQuadTreeViewDependentPointQuery<POINT, YProtPtExtentType>
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

        //MS : Might need to be done at the MrDTMReprojectionQuery level.
        if ((mrDTMQueryParamsPtr->GetSourceGCS() != 0) && (mrDTMQueryParamsPtr->GetTargetGCS() != 0))
            {
            BaseGCSPtr sourcePtr = mrDTMQueryParamsPtr->GetSourceGCS();
            BaseGCSPtr targetPtr = mrDTMQueryParamsPtr->GetTargetGCS();
            viewDependentQueryP->SetReprojectionInfo(sourcePtr, targetPtr);
            }
            
            pointQueryP = viewDependentQueryP;            
        }

    DRange3d spatialIndexRange;

    YProtPtExtentType ExtentPoints = m_mrDTMPointIndexPtr->GetContentExtent();
    spatialIndexRange.low.x = ExtentOp<YProtPtExtentType>::GetXMin(ExtentPoints);
    spatialIndexRange.high.x = ExtentOp<YProtPtExtentType>::GetXMax(ExtentPoints);
    spatialIndexRange.low.y = ExtentOp<YProtPtExtentType>::GetYMin(ExtentPoints);
    spatialIndexRange.high.y = ExtentOp<YProtPtExtentType>::GetYMax(ExtentPoints);               

    HFCPtr<HVEShape> clipShapePtr = CreateClipShape(spatialIndexRange);

    if ((clipShapePtr != 0) && (clipShapePtr->IsEmpty() == false))
        {
        IHGFPointIndexSpatialLimitWrapQuery<POINT, YProtPtExtentType>* wrappedShapedQuery;
        HFCPtr<HVE2DShape> clipShape = static_cast<HVE2DShape*>(clipShapePtr->GetShapePtr()->Clone());
        wrappedShapedQuery = new IHGFPointIndexSpatialLimitWrapQuery<POINT, YProtPtExtentType>(clipShape, pointQueryP.release());

        pointQueryP = wrappedShapedQuery;
        }  
            
    try   
        {           
    #ifdef ACTIVATE_NODE_QUERY_TRACING            
        //queryObject.SetTracingXMLFileName(AString("E:\\MyDoc\\SS3 - Iteration 1\\MrDTM\\New 3D Query\\Log\\CheckedNodesDuringLastQuery.xml"));                 
    #endif                          
                    
        m_mrDTMPointIndexPtr->Query(pointQueryP.get(), pointList);
       
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
        
    if ((status == SUCCESS) && (mrDTMQueryParamsPtr->GetTriangulationState() == true))
        {
        // Since triangulation was requested, the process needs to be completed. This means that current query result
        // must be clipped according to current query object clips then triangulated.

        if ((m_clips != 0) && (m_clips->GetNbClips() > 0))
            {
            status = TriangulateDTM(dtmPtr, mrDTMQueryParamsPtr);


            // Set clip shape to DTM
            SetClipsToDTM(dtmPtr, spatialIndexRange, m_clips);
            }

        // Triangulate
        status = TriangulateDTM(dtmPtr, mrDTMQueryParamsPtr);
        }               

    return status;
    }

/*----------------------------------------------------------------------------+
|MrDTMViewDependentPointQuery Method Definition Section - End
+----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------+
|MrDTMFixResolutionViewPointQuery Method Definition Section - Begin
+----------------------------------------------------------------------------*/
template <class POINT> MrDTMFixResolutionViewPointQuery<POINT>::MrDTMFixResolutionViewPointQuery(const HFCPtr<HGFPointIndex<POINT, YProtPtExtentType>>& pointIndexPtr, 
                                                                                                 const GeoCoords::GCS&                                        pointIndexGCS)
: m_pointIndexGCS(pointIndexGCS)
    {    
    m_mrDTMPointIndexPtr = pointIndexPtr;    
    }

template <class POINT> MrDTMFixResolutionViewPointQuery<POINT>::~MrDTMFixResolutionViewPointQuery()
    {
    }
    
template <class POINT> int MrDTMFixResolutionViewPointQuery<POINT>::_Query(Bentley::TerrainModel::DTMPtr&            dtmPtr, 
                                                                           const DPoint3d*                  pQueryShapePts, 
                                                                           int                              nbQueryShapePts, 
                                                                           const IMrDTMQueryParametersPtr&  mrDTMQueryParamsPtr) const
    {
    //MST More validation is required here.
    assert(mrDTMQueryParamsPtr != 0);
    //MST Not supported.
    assert(pQueryShapePts == 0);

    int         status = SUCCESS;

    // list<POINT> pointList;  
    HPMMemoryManagedVector<POINT> pointList(&s_queryMemoryManager);   


    if (dynamic_cast<IMrDTMFixResolutionIndexQueryParams*>(mrDTMQueryParamsPtr.get()) != 0)
        {
        IMrDTMFixResolutionIndexQueryParams* queryParams(dynamic_cast<IMrDTMFixResolutionIndexQueryParams*>(mrDTMQueryParamsPtr.get()));               

        dtmPtr = new MrDTMSingleResolutionPointIndexView<POINT>(m_mrDTMPointIndexPtr, queryParams->GetResolutionIndex(), m_pointIndexGCS);       
        }
    else    
        {
        assert(dynamic_cast<IMrDTMFixResolutionMaxPointsQueryParams*>(mrDTMQueryParamsPtr.get()) != 0);

        IMrDTMFixResolutionMaxPointsQueryParams* queryParams(dynamic_cast<IMrDTMFixResolutionMaxPointsQueryParams*>(mrDTMQueryParamsPtr.get()));       
          
        status = SUCCESS;
                                          
        if (m_mrDTMPointIndexPtr->IsEmpty() == false)
            {                    
            __int64 nbObjectsForLevel;            
            int     resolutionIndex = 0;
            
            size_t  nbResolution = m_mrDTMPointIndexPtr->GetDepth();        
            
            for (; resolutionIndex <= (int)nbResolution; resolutionIndex++)
                    {
                    nbObjectsForLevel = m_mrDTMPointIndexPtr->GetNbObjectsAtLevel(resolutionIndex);
                    
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
                dtmPtr = new MrDTMSingleResolutionPointIndexView<POINT>(m_mrDTMPointIndexPtr, resolutionIndex, m_pointIndexGCS);
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
|MrDTMFixResolutionViewPointQuery Method Definition Section - End
+----------------------------------------------------------------------------*/