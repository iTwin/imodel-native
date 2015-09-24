/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/MrDTMUtilityFunctions.h $
|    $RCSfile: MrDTMUtilityFunctions.h,v $
|   $Revision: 1.15 $
|       $Date: 2013/03/27 15:53:36 $
|     $Author: Jean-Francois.Cote $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|																		|
|	MrDTMFileCreator.h    		  	    		(C) Copyright 2001.		|
|												BCIVIL Corporation.		|
|												All Rights Reserved.	|
|                                                                       |
+----------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <TerrainModel/TerrainModel.h>
#include <TerrainModel/Core/bcDTMClass.h>

#include <ScalableTerrainModel/IMrDTMQuery.h>
#include <ScalableTerrainModel/IMrDTMClipContainer.h>

USING_NAMESPACE_BENTLEY_MRDTM

BENTLEYSTM_EXPORT int GetShapeInFrontOfProjectivePlane(vector<DPoint3d>&       shapeInFrontOfProjectivePlane, 
                                                 double&                 ratioShapeInFrontToTile,
                                                 const vector<DPoint3d>& tileborderPoints, 
                                                 const double            rootToViewMatrix[][4]);

BENTLEYSTM_EXPORT bool GetVisibleAreaForView(DPoint3d**   fencePt, 
                                            int&           nbPts, 
                                            const DPoint3d viewBox[],                                         
                                            DRange3d&      dtmRange, 
                                            DRange3d&      dtmIntersectionRange);
/*
static HFCPtr<HVE2DPolygonOfSegments> ComputeConvexHull    (const DPoint3d*      inputPts,
                                                            size_t          inputPtsQty)
*/


template<class EXTENT> bool GetVisibleExtent(EXTENT&        po_rVisibleExtent, 
                                             const EXTENT&  pi_rExtentInMeters, 
                                             const DPoint3d pi_pViewBox[]);

template<class EXTENT> bool GetVisibleExtent(EXTENT&        po_rVisibleExtent, 
                                             const EXTENT&  pi_rExtent,
                                             const DPoint3d pi_pViewBox[])
    {   
    bool isVisible = false;

    //::DPoint3d** fencePt
    int  nbPts;
                
    DRange3d dtmIntersectionRange;
    DRange3d extentRange;
        
    extentRange.low.x = ExtentOp<EXTENT>::GetXMin(pi_rExtent);
    extentRange.low.y = ExtentOp<EXTENT>::GetYMin(pi_rExtent);
    extentRange.low.z = ExtentOp<EXTENT>::GetZMin(pi_rExtent);

    extentRange.high.x = ExtentOp<EXTENT>::GetXMax(pi_rExtent);
    extentRange.high.y = ExtentOp<EXTENT>::GetYMax(pi_rExtent);
    extentRange.high.z = ExtentOp<EXTENT>::GetZMax(pi_rExtent);

    isVisible = GetVisibleAreaForView(0, 
                                      nbPts, 
                                      pi_pViewBox,                                       
                                      extentRange, 
                                      dtmIntersectionRange);
    
    if (isVisible == true)
        {
        DPoint3d lowPt(dtmIntersectionRange.low);
        DPoint3d highPt(dtmIntersectionRange.high);  
                          
        ExtentOp<EXTENT>::SetXMin(po_rVisibleExtent, lowPt.x);
        ExtentOp<EXTENT>::SetYMin(po_rVisibleExtent, lowPt.y);
        ExtentOp<EXTENT>::SetZMin(po_rVisibleExtent, lowPt.z);

        ExtentOp<EXTENT>::SetXMax(po_rVisibleExtent, highPt.x);
        ExtentOp<EXTENT>::SetYMax(po_rVisibleExtent, highPt.y);
        ExtentOp<EXTENT>::SetZMax(po_rVisibleExtent, highPt.z);        
        }        
    
    return isVisible;  
    }

BENTLEYSTM_EXPORT int CreateBcDTM(BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& dtmPtr);

BENTLEYSTM_EXPORT int SetClipsToDTM (BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& dtmPtr,
                               const DRange3d&                      dtmRange,
                               const IMrDTMClipContainerPtr&        clips);

BENTLEYSTM_EXPORT int SetClipsToDTM (BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& dtmPtr,
                               const DRange3d&                      dtmRange,
                               const vector<DPoint3d>&              regionPoints,
                               const IMrDTMClipContainerPtr&        clips);

BENTLEYSTM_EXPORT int TriangulateDTM(BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr&                     dtmPtr, 
                               const Bentley::MrDTM::IMrDTMQueryParametersPtr& mrDTMQueryParamsPtr);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AlainRobert  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BENTLEYSTM_EXPORT bool GCSMathematicalDomainsOverlap(Bentley::GeoCoordinates::BaseGCSPtr& sourceGCSPtr,
                                               Bentley::GeoCoordinates::BaseGCSPtr& targetGCSPtr);

typedef vector<DPoint3d> MaskPoints;
typedef list<MaskPoints> MaskList;

BENTLEYSTM_EXPORT bool AddExtentAndTestMatch(bool&           shouldAddMatchedCachedTile,
                                       MaskList&       maskList,
                                       bool            reset,                                        
                                       const DRange2d& extentToCover,
                                       const DRange2d& matchedCachedTileExtent);



typedef int (*checkTriangulationStopCallbackFP)(DTMFeatureType dtmFeatureType);

checkTriangulationStopCallbackFP GetTriangulationTerminationCallback();

BENTLEYSTM_EXPORT bool SetTriangulationTerminationCallback(checkTriangulationStopCallbackFP callbackFP);


class DTMLinearFeature;

typedef int (*addLinearsForPresentationModeFP)(const BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr&  dtmPtr,                                                 
                                               DTMLinearFeature*            linearList,
                                               unsigned int                 nbLinearListElems,                                               
                                               size_t                       maxNumberOfPoints);
                                
addLinearsForPresentationModeFP GetLinearsForPresentationModeCallback();

BENTLEYSTM_EXPORT bool SetLinearsForPresentationModeCallback(addLinearsForPresentationModeFP callbackFP);
