/*--------------------------------------------------------------------------------------+
|
|     $Source: Drainage/bcdtmDrainage.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//

#include "bcdtmDrainage.h"

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
DTMDrainageTables* dtmDrainageTablesP = nullptr;

DTMStatusInt BcDTMDrainage::CreateDrainageTables
    (
    BcDTMP   dtm,
    DTMDrainageTables*&     dtmDrainageTablesPP 
    ) 
    {

    BC_DTM_OBJ* dtmP = dtm->GetTinHandle() ;

    if( dtmDrainageTablesPP == nullptr || ( dtmDrainageTablesPP != nullptr && dtmDrainageTablesP->CheckForDtmChange(dtmP->numPoints,dtmP->numLines,dtmP->numTriangles,dtmP->numFeatures,dtmP->modifiedTime)) )
        {

        // Destroy Current Instance Of Drainage Tables

        if( dtmDrainageTablesPP != nullptr )
            {
            delete dtmDrainageTablesPP ;
            dtmDrainageTablesPP = nullptr ;
            }

        // Create New Drainage Tables  

        int startTime=bcdtmClock() ;
        bcdtmWrite_message(0,0,0,"Creating Drainage Tables") ;
        dtmDrainageTablesPP = new DTMDrainageTables(dtmP) ;
        bcdtmWrite_message(0,0,0,"Time To Calculate Drainage Tables = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;

        // Calculate Pond Tables

        if (dtmDrainageTablesPP != nullptr )
            {
            startTime=bcdtmClock() ;
            bcdtmWrite_message(0,0,0,"Creating Drainage Pond Tables") ;
            bcdtmDrainage_determinePondsDtmObject(dtmP,dtmDrainageTablesP,nullptr,false,true,nullptr) ;
            bcdtmWrite_message(0,0,0,"Time To Calculate Drainage Pond Tables = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
            }

        }

    if (dtmDrainageTablesPP == nullptr)
        return DTM_ERROR ;
    else
        return DTM_SUCCESS ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
DTMStatusInt BcDTMDrainage::DeterminePonds
    (
    BcDTMP dtm,
    DTMDrainageTables*     dtmDrainageTablesP,
    DTMFeatureCallback     callBackFunctionP,
    void*                  userP
    )
    {
    //  Check Memory Mode

    if( dtm->SetMemoryAccess(DTMAccessMode::Temporary) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is Read Only");
        return DTM_ERROR;
        }

    // Call Core Function
    int  tdbg=DTM_TIME_VALUE(0) ;
    long startTime=bcdtmClock() ;
    DTMStatusInt status = (DTMStatusInt)bcdtmDrainage_determinePondsDtmObject (dtm->GetTinHandle(), dtmDrainageTablesP, callBackFunctionP, true, false, userP) ;
    if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Calculate Drainage Pond Tables = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
    return status ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/

DTMStatusInt BcDTMDrainage::TraceMaximumDescent
    (
    BcDTMP   dtm, 
    DTMDrainageTables*       dtmDrainageTablesP, 
    double                   minDepth, 
    double                   x, 
    double                   y, 
    DTMFeatureCallback       callBackFunctionP,
    void*                    userP
    )
    {

    //  Check Memory Mode

    if( dtm->SetMemoryAccess(DTMAccessMode::Temporary) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is Read Only");
        return DTM_ERROR;
        }

    // Create Drainage Class

    DTMDrainage  dtmDrainage ;

    // Convert Trace Start Point

    //    bcDTMTransformHelper helper (dtm);
    DPoint3d cPt ;
    cPt.x = x ;
    cPt.y = y ;
    cPt.z = 0.0 ;
    //    helper.convertPointToDTM (cPt);
    //    minDepth = helper.convertDistanceToDTM (minDepth);

    BC_DTM_OBJ *dtmP = dtm->GetTinHandle() ;

    // Call Core DTM Method

    //    bcDTMTransformHelper::PFBrowseFeatureCallbackWrapper data;
    //    data.callback = (PFBrowseFeatureCallback)callBackFunctionP;
    //    data.userP = userP;
    //    data.wrapper = &helper;

    return (DTMStatusInt)bcdtmDrainage_traceMaximumDescentDtmObject( dtmP,dtmDrainageTablesP,( DTMFeatureCallback ) callBackFunctionP,minDepth,cPt.x,cPt.y,userP);
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/

DTMStatusInt BcDTMDrainage::TraceMaximumAscent
    (
    BcDTMP  dtm, 
    DTMDrainageTables*      dtmDrainageTablesP,
    double                  minDepth, 
    double                  x, 
    double                  y, 
    DTMFeatureCallback      callBackFunctionP,
    void*                   userP
    )
    {

    //  Set Memory Mode

    if( dtm->SetMemoryAccess(DTMAccessMode::Temporary) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is Read Only");
        return DTM_ERROR;
        }

    // Convert Trace Start Point

    //    bcDTMTransformHelper helper (dtm);
    DPoint3d cPt ;
    cPt.x = x ;
    cPt.y = y ;
    cPt.z = 0.0 ;
    //    helper.convertPointToDTM (cPt);
    //    minDepth = helper.convertDistanceToDTM (minDepth);

    // Calculate Drainage Tables If They Dont Exist Or The DTM Has Changed

    BC_DTM_OBJ *dtmP = dtm->GetTinHandle() ;


    // Call Core DTM Method

    //    bcDTMTransformHelper::PFBrowseFeatureCallbackWrapper data;
    //    data.callback = (PFBrowseFeatureCallback)callBackFunctionP;
    //    data.userP = userP;
    //    data.wrapper = &helper;

    return (DTMStatusInt)bcdtmDrainage_traceMaximumAscentDtmObject (dtmP, dtmDrainageTablesP, (DTMFeatureCallback)callBackFunctionP, minDepth, cPt.x, cPt.y, userP);
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
DTMStatusInt bcdtmDrainage_calculatePondCallBack
    (
    DTMFeatureType dtmFeatureType, 
    DTMUserTag     userTag, 
    DTMFeatureId   featureId, 
    DPoint3d*      pointsP, 
    int            numPoints, 
    void*          userArgP
    )
    {
    if( userArgP != nullptr )
        {
        Bentley::TerrainModel::DTMFeatureBuffer* buffer = (Bentley::TerrainModel::DTMFeatureBuffer*) userArgP ; 
        buffer->AddDtmFeatureToBuffer(dtmFeatureType,userTag,featureId,pointsP,numPoints);
        }

    //   Return

    return DTM_SUCCESS;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
DTMStatusInt BcDTMDrainage::CalculatePondForPoint
    (
    BcDTMP dtm, 
    double                 x, 
    double                 y, 
    double                 minDepth, 
    bool&                  pondCalculated,
    double&                pondElevation,
    double&                pondDepth,
    double&                pondArea,
    double&                pondVolume,
    Bentley::TerrainModel::DTMDynamicFeatureArray& pondFeatures
    )
    {
    DTMStatusInt ret = DTM_SUCCESS;
    void*    userP = nullptr;
    Bentley::TerrainModel::DTMFeatureBuffer buffer;

    //  Initialise

    pondCalculated = false;

    //  Set Memory Mode
    if (dtm->SetMemoryAccess (DTMAccessMode::Temporary) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is Read Only");
        return DTM_ERROR;
        }

    // Convert Trace Start Point

    //    bcDTMTransformHelper helper (dtm);
    //    helper.convertPointToDTM (x, y);

    DTMStatusInt status = (DTMStatusInt) bcdtmDrainage_calculatePondDtmObject (dtm->GetTinHandle(), x, y, minDepth, (DTMFeatureCallback)bcdtmDrainage_calculatePondCallBack, true, &pondCalculated, &pondElevation, &pondDepth, &pondArea, &pondVolume, &buffer); 

    //  Get Pond Features From Buffer 
    //    if (pondElevationP) *pondElevationP = helper.convertElevationFromDTM (*pondElevationP);
    //    if (pondDepthP) *pondDepthP = helper.convertDistanceFromDTM (*pondDepthP);
    //    if (pondAreaP) *pondAreaP = helper.convertAreaFromDTM (*pondAreaP);
    //    if (pondVolumeP) *pondVolumeP = helper.convertVolumeFromDTM (*pondVolumeP);

    if( status == DTM_SUCCESS && pondCalculated ) 
        buffer.GetDtmDynamicFeaturesArray (pondFeatures, dtm->GetTransformHelper());

    //   Return

    return status;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/

DTMStatusInt BcDTMDrainage::TraceCatchmentForPoint
    (
    BcDTMP  dtm, 
    DPoint3d   tracePoint,
    double     maxPondDepth, 
    bool&      catchmentDetermined,
    DPoint3d&  sumpPoint,
    bvector<DPoint3d>& catchmentPts
    )
    {
    DTMStatusInt ret = DTM_SUCCESS ;
    long catchmentClosure = 0;

    //  Initialise

    catchmentDetermined = false ;
    sumpPoint.x = sumpPoint.y = sumpPoint.z = 0.0;

    //  Set Memory Mode
    if (dtm->SetMemoryAccess (DTMAccessMode::Temporary) != DTM_SUCCESS)
        {
        bcdtmWrite_message (1,0,0, "DTM is Read Only");
        return DTM_ERROR;
        }

    // Convert Trace Start Point

    //    bcDTMTransformHelper helper (dtm);
    //    helper.convertPointToDTM (tracePoint.x, tracePoint.y);

    //  Call Core DTM Function 
    //     int status = bcdtmDrainage_traceCatchmentForPointDtmObject(dtm->GetTinHandle(),tracePoint.x,tracePoint.y, helper.convertDistanceToDTM (maxPondDepth),&catchmentDetermined,&catchmentClosure,(DPoint3d **)catchmentPtsPP,numCatchmentPtsP,(DPoint3d *)sumpPointP) ;
    DPoint3d* catchmentPtsP = nullptr;
    long numCatchmentPts;
    DTMStatusInt status = (DTMStatusInt) bcdtmDrainage_traceCatchmentForPointDtmObject (dtm->GetTinHandle(), tracePoint.x, tracePoint.y, maxPondDepth, &catchmentDetermined, &catchmentClosure, &catchmentPtsP, &numCatchmentPts, &sumpPoint) ;

    //  Encode Catchment Points 

    if (status == DTM_SUCCESS && catchmentDetermined) 
        {
        catchmentPts.resize (numCatchmentPts);
        memcpy (&catchmentPts[0], catchmentPtsP, sizeof (DPoint3d) * numCatchmentPts);
        //        helper.convertPointsFromDTM (*catchmentPtsPP, *numCatchmentPtsP);
        //        helper.convertPointFromDTM (*sumpPointP);
        }

    if (catchmentPtsP)
        free (catchmentPtsP);

    //   Return
    return status;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
DTMStatusInt BcDTMDrainage::CreateDepressionDtm
    (
    BcDTMP dtmP,               // ==> Pointer to Dtm object     
    BC_DTM_OBJ*&          depressionDtmP,     // <== Depression Dtm    
    DTMFeatureCallback    loadFunctionP,      // <== Call Back Function For Check Stop Purposes
    void*                 userP               // <== Pointer To User Argument For Check Stop Purposes
    )
    {
    return (DTMStatusInt)bcdtmDrainage_createDepressionDtmObject(dtmP->GetTinHandle(), &depressionDtmP, loadFunctionP, userP) ;     
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
DTMStatusInt BcDTMDrainage::CreateAndCheckDrainageTables
    (
    BcDTMP dtmP               // ==> Pointer to Dtm object     
    )
    {
    return (DTMStatusInt)bcdtmTables_createAndCheckDrainageTablesDtmObject (dtmP->GetTinHandle()) ;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
DTMStatusInt BcDTMDrainage::ReturnLowPoints
    (
    BcDTMP                dtmP,              // ==> Pointer to Dtm object     
    DTMFeatureCallback                   loadFunctionP,     // ==> Call Back Function 
    DTMFenceParamsCR  fence,             // ==> Fence Parameters
    void*                                userP              // <==> Pointer To User Call Back Function
    )
    {
    int numLowPoints=0,numFencePts=0; 
    DTMFenceOption fenceOption = DTMFenceOption::Inside;
    DTMFenceType fenceType = DTMFenceType::Block;
    bool useFence=false ;
    DPoint3dCP fencePtsP=nullptr ;
    if( fence.fenceType != DTMFenceType::None )
        {
        useFence    = true ;  
        fencePtsP   = fence.points ;
        numFencePts = fence.numPoints ;
        fenceOption = fence.fenceOption ;
        fenceType   = fence.fenceType ;
        if( fencePtsP == nullptr || numFencePts <= 3 )
            {
            useFence = false ;      
            }
        else if( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )
            {
            useFence = false ; 
            }
        } 
    return (DTMStatusInt)bcdtmDrainage_returnLowPointsDtmObject (dtmP->GetTinHandle(),loadFunctionP,useFence,fenceType,fenceOption,fencePtsP,numFencePts,userP,numLowPoints) ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
DTMStatusInt BcDTMDrainage::ReturnNoneFalseLowPoints
    (
    BcDTMP                dtmP,              // ==> Pointer to Dtm object   
    double                               falseLowDepth,     // ==> Depth Must Be greater To return As a Low Point  
    DTMFeatureCallback                   loadFunctionP,     // ==> Call Back Function 
    DTMFenceParamsCR  fence,             // ==> Fence Parameters
    void*                                userP              // <==> Pointer To User Call Back Function
    )
    {
    int numLowPoints=0,numFencePts=0; 
    DTMFenceOption fenceOption = DTMFenceOption::Inside;
    DTMFenceType fenceType = DTMFenceType::Block;
    bool useFence = false;
    DPoint3dCP fencePtsP=nullptr ;
    if( fence.fenceType != DTMFenceType::None )
        {
        useFence    = true ;  
        fencePtsP   = fence.points ;
        numFencePts = fence.numPoints ;
        fenceOption = fence.fenceOption ;
        fenceType   = fence.fenceType ;
        if( fencePtsP == nullptr || numFencePts <= 3 )
            {
            useFence = false ;      
            }
        else if( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )
            {
            useFence = false ; 
            }
        } 
     return (DTMStatusInt)bcdtmDrainage_returnNoneFalseLowPointsDtmObject(dtmP->GetTinHandle(),falseLowDepth,loadFunctionP,useFence,fenceType,fenceOption,fencePtsP,numFencePts,userP,numLowPoints) ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
DTMStatusInt BcDTMDrainage::ReturnHighPoints
    (
    BcDTMP                dtmP,              // ==> Pointer to Dtm object     
    DTMFeatureCallback                   loadFunctionP,     // ==> Call Back Function 
    DTMFenceParamsCR  fence,             // ==> Fence Parameters
    void*                                userP              // <==> Pointer To User Call Back Function
    )
    {
    int numHighPoints=0,numFencePts=0; 
    DTMFenceOption fenceOption = DTMFenceOption::Inside;
    DTMFenceType fenceType = DTMFenceType::Block;

    bool useFence=false ;
    DPoint3dCP fencePtsP=nullptr ;
    if( fence.fenceType != DTMFenceType::None )
        {
        useFence    = true ;  
        fencePtsP   = fence.points ;
        numFencePts = fence.numPoints ;
        fenceOption = fence.fenceOption ;
        fenceType   = fence.fenceType ;
        if( fencePtsP == nullptr || numFencePts <= 3 )
            {
            useFence = false ;      
            }
        else if( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )
            {
            useFence = false ; 
            }
        } 
    return (DTMStatusInt)bcdtmDrainage_returnHighPointsDtmObject (dtmP->GetTinHandle(),loadFunctionP,useFence,fenceType,fenceOption,fencePtsP,numFencePts,userP,numHighPoints) ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
DTMStatusInt BcDTMDrainage::ReturnSumpLines
    (
    BcDTMP                dtmP,              // ==> Pointer to Dtm object     
    DTMFeatureCallback                   loadFunctionP,     // ==> Call Back Function 
    DTMFenceParamsCR  fence,             // ==> Fence Parameters
    void*                                userP              // <==> Pointer To User Call Back Function
    )
    {
    int numSumpLines=0,numFencePts=0; 
    DTMFenceOption fenceOption = DTMFenceOption::Inside;
    DTMFenceType fenceType = DTMFenceType::Block;
    bool useFence=false ;
    DPoint3dCP fencePtsP=nullptr ;
    if( fence.fenceType != DTMFenceType::None )
        {
        useFence    = true ;  
        fencePtsP   = fence.points ;
        numFencePts = fence.numPoints ;
        fenceOption = fence.fenceOption ;
        fenceType   = fence.fenceType ;
        if( fencePtsP == nullptr || numFencePts <= 3 )
            {
            useFence = false ;      
            }
        else if( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )
            {
            useFence = false ; 
            }
        } 
    return (DTMStatusInt)bcdtmDrainage_returnSumpLinesDtmObject (dtmP->GetTinHandle(),loadFunctionP,useFence,fenceType,fenceOption,fencePtsP,numFencePts,userP,numSumpLines) ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
DTMStatusInt BcDTMDrainage::ReturnZeroSlopeSumpLines
    (
    BcDTMP                dtmP,              // ==> Pointer to Dtm object     
    DTMFeatureCallback                   loadFunctionP,     // ==> Call Back Function 
    DTMFenceParamsCR  fence,             // ==> Fence Parameters
    void*                                userP              // <==> Pointer To User Call Back Function
    )
    {
    int numSumpLines=0,numFencePts=0; 
    DTMFenceOption fenceOption = DTMFenceOption::Inside;
    DTMFenceType fenceType = DTMFenceType::Block;
    bool useFence = false;
    DPoint3dCP fencePtsP=nullptr ;
    if( fence.fenceType != DTMFenceType::None )
        {
        useFence    = true ;  
        fencePtsP   = fence.points ;
        numFencePts = fence.numPoints ;
        fenceOption = fence.fenceOption ;
        fenceType   = fence.fenceType ;
        if( fencePtsP == nullptr || numFencePts <= 3 )
            {
            useFence = false ;      
            }
        else if( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )
            {
            useFence = false ; 
            }
        } 
    return (DTMStatusInt)bcdtmDrainage_returnZeroSlopeSumpLinesDtmObject (dtmP->GetTinHandle(),loadFunctionP,useFence,fenceType,fenceOption,fencePtsP,numFencePts,userP,numSumpLines) ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
DTMStatusInt BcDTMDrainage::ReturnRidgeLines
    (
    BcDTMP                dtmP,              // ==> Pointer to Dtm object     
    DTMFeatureCallback                   loadFunctionP,     // ==> Call Back Function 
    DTMFenceParamsCR  fence,             // ==> Fence Parameters
    void*                                userP              // <==> Pointer To User Call Back Function
    )
    {
    int numRidgeLines=0,numFencePts=0; 
    DTMFenceOption fenceOption = DTMFenceOption::Inside;
    DTMFenceType fenceType = DTMFenceType::Block;
    bool useFence = false;
    DPoint3dCP fencePtsP=nullptr ;
    if( fence.fenceType != DTMFenceType::None )
        {
        useFence    = true ;  
        fencePtsP   = fence.points ;
        numFencePts = fence.numPoints ;
        fenceOption = fence.fenceOption ;
        fenceType   = fence.fenceType ;
        if( fencePtsP == nullptr || numFencePts <= 3 )
            {
            useFence = false ;      
            }
        else if( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )
            {
            useFence = false ; 
            }
        } 
    return (DTMStatusInt)bcdtmDrainage_returnRidgeLinesDtmObject (dtmP->GetTinHandle(),loadFunctionP,useFence,fenceType,fenceOption,fencePtsP,numFencePts,userP,numRidgeLines) ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
DTMStatusInt BcDTMDrainage::ReturnZeroSlopePolygons
    (
    BcDTMP                dtmP,              // ==> Pointer to Dtm object     
    DTMFeatureCallback                   loadFunctionP,     // ==> Call Back Function 
    DTMFenceParamsCR  fence,             // ==> Fence Parameters
    void*                                userP              // <==> Pointer To User Call Back Function
    )
    {
    int numZeroSlopePolygons=0,numFencePts=0; 
    DTMFenceOption fenceOption = DTMFenceOption::Inside;
    DTMFenceType fenceType = DTMFenceType::Block;
    bool useFence = false;
    DPoint3dCP fencePtsP=nullptr ;
    if( fence.fenceType != DTMFenceType::None )
        {
        useFence    = true ;  
        fencePtsP   = fence.points ;
        numFencePts = fence.numPoints ;
        fenceOption = fence.fenceOption ;
        fenceType   = fence.fenceType ;
        if( fencePtsP == nullptr || numFencePts <= 3 )
            {
            useFence = false ;      
            }
        else if( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )
            {
            useFence = false ; 
            }
        } 
    return (DTMStatusInt)bcdtmDrainage_returnZeroSlopePolygonsDtmObject (dtmP->GetTinHandle(),loadFunctionP,useFence,fenceType,fenceOption,fencePtsP,numFencePts,userP,numZeroSlopePolygons) ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
DTMStatusInt BcDTMDrainage::CreateRefinedDrainageDtm
    (
    BcDTMP                dtmP,              // ==> Pointer to Dtm     
    DTMFenceParamsCR  fence,             // ==> Fence Parameters
    Bentley::TerrainModel::BcDTMPtr*              refinedDtmP        // <== Pointer To Refined DTM 
    )
    {
    int status=DTM_SUCCESS,numFencePts=0 ; 
    DTMFenceOption fenceOption = DTMFenceOption::Inside;
    DTMFenceType fenceType = DTMFenceType::Block;
    bool useFence = false;
    DPoint3dCP fencePtsP=nullptr ;
    BC_DTM_OBJ *drainageDtmP=nullptr ;

    // Check Fence Parameters 

    if( fence.fenceType != DTMFenceType::None )
        {
        useFence    = true ;  
        fencePtsP   = fence.points ;
        numFencePts = fence.numPoints ;
        fenceOption = fence.fenceOption ;
        fenceType   = fence.fenceType ;
        if( fencePtsP == nullptr || numFencePts <= 3 )
            {
            useFence = false ;      
            }
        else if( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )
            {
            useFence = false ; 
            }
        } 

    // Check For Null Refined Drainage DTM

    if( *refinedDtmP != nullptr )
        {
         bcdtmWrite_message(1,0,0,"Method Requires Null Dtm Pointer") ;
         status = DTM_ERROR ;
        }  

    //  Clone DTM Object

    if( status == DTM_SUCCESS )
        {
        status =  bcdtmObject_cloneDtmObject(dtmP->GetTinHandle(),&drainageDtmP) ;
        } 

    //  Refine DTM

    if( status == DTM_SUCCESS )
        {
        status = bcdtmDrainage_refineTinForDrainageDtmObject(dtmP->GetTinHandle(),useFence,fenceType,fenceOption,fencePtsP,numFencePts) ;
        } 

    //  Create IbcDTM

    if( status == DTM_SUCCESS )
        {
         *refinedDtmP = Bentley::TerrainModel::BcDTM::CreateFromDtmHandle(drainageDtmP);
        } 
  
    return  ( DTMStatusInt ) status ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
DTMStatusInt BcDTMDrainage::ReturnCatchments
    (
    BcDTMP                dtmP,     // ==> Pointer to Dtm     
    class DTMDrainageTables*             drainageTablesP,   // ==> Pointer To Drainage Tables                  
    double                               falseLowDepth,     // ==> Downstrean Trace Will Flow Out Of Ponds Less Than this Depth
    bool                                 refineOption,      // ==> If True Refine TM For Catchment Delineation 
    DTMFeatureCallback                   loadFunctionP,     // ==> Call Back Function 
    DTMFenceParamsCR  fence,             // ==> Fence Parameters
    void*                                userP              // <==> Pointer To User Call Back Function
    )
    {
    int numCatchments=0,numFencePts=0; 
    DTMFenceOption fenceOption = DTMFenceOption::Inside;
    DTMFenceType fenceType = DTMFenceType::Block;
    bool useFence = false;
    DPoint3dCP fencePtsP=nullptr ;

    // Check Fence Parameters 

    if( fence.fenceType != DTMFenceType::None )
        {
        useFence    = true ;  
        fencePtsP   = fence.points ;
        numFencePts = fence.numPoints ;
        fenceOption = fence.fenceOption ;
        fenceType   = fence.fenceType ;
        if( fencePtsP == nullptr || numFencePts <= 3 )
            {
            useFence = false ;      
            }
        else if( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )
            {
            useFence = false ; 
            }
        } 


    //  Return DTM Catchments

    return  ( DTMStatusInt ) bcdtmDrainage_traceCatchmentsDtmObject(dtmP->GetTinHandle(),loadFunctionP,drainageTablesP,falseLowDepth,refineOption,useFence,fenceType,fenceOption,fencePtsP,numFencePts,userP,numCatchments) ;
    
	}
