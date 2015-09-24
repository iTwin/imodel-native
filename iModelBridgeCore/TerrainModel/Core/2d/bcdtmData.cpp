/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmData.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
#include "bcdtminlines.h" 

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmData_testForValidDtmObjectImportFeatureType(DTMFeatureType dtmFeatureType)
    {
    int  ret=DTM_ERROR ;
    /*
    ** Test For Valid Dtm Object Dtm Feature Type
    */
    if( dtmFeatureType == DTMFeatureType::RandomSpots     ) ret = DTM_SUCCESS ;
    if( dtmFeatureType == DTMFeatureType::GroupSpots      ) ret = DTM_SUCCESS ;
    if( dtmFeatureType == DTMFeatureType::Breakline      ) ret = DTM_SUCCESS ;
    if( dtmFeatureType == DTMFeatureType::GraphicBreak   ) ret = DTM_SUCCESS ;
    if( dtmFeatureType == DTMFeatureType::SoftBreakline      ) ret = DTM_SUCCESS ;
    if( dtmFeatureType == DTMFeatureType::ContourLine    ) ret = DTM_SUCCESS ; 
    if( dtmFeatureType == DTMFeatureType::Void            ) ret = DTM_SUCCESS ;
    if( dtmFeatureType == DTMFeatureType::VoidLine       ) ret = DTM_SUCCESS ; 
    if( dtmFeatureType == DTMFeatureType::Island          ) ret = DTM_SUCCESS ;
    if( dtmFeatureType == DTMFeatureType::Hole            ) ret = DTM_SUCCESS ;
    if( dtmFeatureType == DTMFeatureType::HoleLine       ) ret = DTM_SUCCESS ;
    if( dtmFeatureType == DTMFeatureType::BreakVoid      ) ret = DTM_SUCCESS ;
    if( dtmFeatureType == DTMFeatureType::DrapeVoid      ) ret = DTM_SUCCESS ;
    if( dtmFeatureType == DTMFeatureType::Hull            ) ret = DTM_SUCCESS ;
    if( dtmFeatureType == DTMFeatureType::DrapeHull      ) ret = DTM_SUCCESS ;
    if( dtmFeatureType == DTMFeatureType::HullLine       ) ret = DTM_SUCCESS ;
    if( dtmFeatureType == DTMFeatureType::Region          ) ret = DTM_SUCCESS ;
    if( dtmFeatureType == DTMFeatureType::SlopeToe       ) ret = DTM_SUCCESS ;
    /*
    ** Job Completed
    */
    return(ret) ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmData_testForValidDtmObjectExportFeatureType(DTMFeatureType dtmFeatureType) 
    {
    int ret=DTM_ERROR ;
    /*
    ** Test For Valid DTM Object Export DTM Feature Type
    */
    if     ( dtmFeatureType == DTMFeatureType::RandomSpots         ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::GroupSpots          ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::Spots                ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::FeatureSpot        ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::TinPoint           ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::TinLine            ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::Breakline          ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::Hull                ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::ContourLine        ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::Void                ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::BreakVoid          ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::Island              ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::Hole                ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::Triangle            ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::TriangleEdge       ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::Region              ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::ZeroSlopePolygon  ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::TriangleIndex      ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::TriangleInfo       ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::FlowArrow          ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::GraphicBreak       ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::SoftBreakline          ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::SlopeToe           ) ret = DTM_SUCCESS ;
    /*
    ** Job Completed
    */
    return(ret) ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmData_testForValidPolygonalDtmFeatureType(DTMFeatureType dtmFeatureType) 
    {
    int polygonalFeature = FALSE ; ;
    /*
    ** Test For Valid DTM Polygonal Feature
    */
    if     ( dtmFeatureType == DTMFeatureType::Void       ) polygonalFeature = TRUE ;
    else if( dtmFeatureType == DTMFeatureType::BreakVoid ) polygonalFeature = TRUE ;
    else if( dtmFeatureType == DTMFeatureType::DrapeVoid ) polygonalFeature = TRUE ;
    else if( dtmFeatureType == DTMFeatureType::Hole       ) polygonalFeature = TRUE ;
    else if( dtmFeatureType == DTMFeatureType::Island     ) polygonalFeature = TRUE ;
    else if( dtmFeatureType == DTMFeatureType::Hull       ) polygonalFeature = TRUE ;
    else if( dtmFeatureType == DTMFeatureType::DrapeHull ) polygonalFeature = TRUE ;
    else if( dtmFeatureType == DTMFeatureType::Polygon    ) polygonalFeature = TRUE ;
    else if( dtmFeatureType == DTMFeatureType::Region     ) polygonalFeature = TRUE ;
    /*
    ** Job Completed
    */
    return(polygonalFeature) ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmData_countNumberOfContinuingDtmFeaturesForPointDtmObject(BC_DTM_OBJ *dtmP,long point,long *numFeaturesP)
    /*
    ** This Function Counts The Number Of Continuing Dtm Features For A Tin Point
    */
    {
    long flistPtr ;
    /*
    ** Count Features
    */
    *numFeaturesP = 0 ;
    flistPtr = nodeAddrP(dtmP,point)->fPtr ;
    while ( flistPtr != dtmP->nullPtr )
        {
        if( flistAddrP(dtmP,flistPtr)->nextPnt != dtmP->nullPnt ) ++(*numFeaturesP) ;
        flistPtr = flistAddrP(dtmP,flistPtr)->nextPtr ;
        }
    /*
    ** Job Completed
    */
    return(DTM_SUCCESS) ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmData_deleteAllDtmFeaturesDtmObject(BC_DTM_OBJ *dtmP )
    /*
    ** Remove All Dtm Features 
    */
    {
    int    ret=DTM_SUCCESS ;
    long   dtmFeature ;
    BC_DTM_FEATURE  *dtmFeatureP ;

    /*
    ** Check For Valid Dtm Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Scan For Feature Type
    */
    if( dtmP->numFeatures > 0 ) 
        { 
        for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
            if( dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted )
                {
                if( bcdtmInsert_removeDtmFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ;
                }
            }
        }
    /*
    ** Clean Up
    */
cleanup :
    /*
    ** Job Completed
    */
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmData_deleteAllTinErrorFeaturesDtmObject(BC_DTM_OBJ *dtmP )
    /*
    ** Remove All Dtm Features 
    */
    {
    int    ret=DTM_SUCCESS ;
    long   dtmFeature ;
    int   isModified = FALSE ;
    BC_DTM_FEATURE  *dtmFeatureP ;
    /*
    ** Check For Valid Dtm Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Scan For Feature Type
    */
    if( dtmP->numFeatures > 0 ) 
        { 
        for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
            if( dtmFeatureP->dtmFeatureState == DTMFeatureState::TinError )
                {
                if( dtmFeatureP->dtmFeaturePts.pointsPI != 0) 
                    {
                    bcdtmMemory_free(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI) ;
                    dtmFeatureP->dtmFeaturePts.pointsPI = 0 ;
                    }
                dtmFeatureP->numDtmFeaturePts = 0 ;
                dtmFeatureP->dtmFeatureState  = DTMFeatureState::Deleted ;
                isModified = TRUE;
                }
            }
        }
    /*
    **  Update Modified Time
    */
    if (isModified)
        bcdtmObject_updateLastModifiedTime (dtmP) ;
    /*
    ** Clean Up
    */
cleanup :
    /*
    ** Job Completed
    */
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmData_deleteAllRollBackFeaturesDtmObject(BC_DTM_OBJ *dtmP )
    /*
    ** Remove All Dtm Features 
    */
    {
    int    ret=DTM_SUCCESS ;
    long   dtmFeature ;
    int   isModified = FALSE ;
    BC_DTM_FEATURE  *dtmFeatureP ;
    /*
    ** Check For Valid Dtm Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Scan For Feature Type
    */
    if( dtmP->numFeatures > 0 ) 
        { 
        for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
            if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Rollback )
                {
                if( dtmFeatureP->dtmFeaturePts.pointsPI != 0) 
                    {
                    bcdtmMemory_free(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI) ;
                    dtmFeatureP->dtmFeaturePts.pointsPI = 0 ;
                    }
                dtmFeatureP->numDtmFeaturePts = 0 ;
                dtmFeatureP->dtmFeatureState  = DTMFeatureState::Deleted ;
                isModified = TRUE;
                }
            }
        }
    /*
    **  Update Modified Time
    */
    if (isModified)
        bcdtmObject_updateLastModifiedTime (dtmP) ;
    /*
    ** Clean Up
    */
cleanup :
    /*
    ** Job Completed
    */
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmData_deleteAllOccurrencesOfDtmFeatureTypeDtmObject
    (
    BC_DTM_OBJ *dtmP,
    DTMFeatureType dtmFeatureType 
    )
    /*
    ** Remove All Dtm Features 
    */
    {
 int    ret=DTM_SUCCESS,isModified=FALSE,dbg=DTM_TRACE_VALUE(0) ;
    long   pnt1,pnt2,point,dtmFeature,numPoints,firstPoint,lastPoint,lastCount=0 ;
    BC_DTM_FEATURE  *dtmFeatureP ;
    DTM_TIN_POINT *pointP,*numPointsP ;
    unsigned char   *delP,*delPointsP=NULL ;
    long   *ofsP,*offsCountP=NULL ;

    // Log Entry Parameters

    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Deleting All Dtm Features Of Type") ;
        bcdtmWrite_message(0,0,0,"dtmP           = %p",dtmP) ;
        bcdtmWrite_message(0,0,0,"dtmFeatureType = %8ld",dtmFeatureType) ;
        } 

    // Check For Valid Dtm Object

    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;

    // Check DTM State

    if( dtmP->dtmState != DTMState::Data && dtmP->dtmState != DTMState::Tin )
        {
        bcdtmWrite_message(2,0,0,"Invalid DTM State For Method") ;
        goto errexit ;
        } 

    // Log DTM State

    if( dbg )
        {
        if     ( dtmP->dtmState == DTMState::Data ) bcdtmWrite_message(0,0,0,"dtmState = DTMState::Data") ;
        else if( dtmP->dtmState == DTMState::Tin  ) bcdtmWrite_message(0,0,0,"dtmState = DTMState::Tin ") ;
        } 
    /*
    ** Check For Random Spots
    */
    if( dtmFeatureType == DTMFeatureType::RandomSpots )
        {
        if( dtmP->dtmState == DTMState::Tin )
            {
            if( bcdtmObject_changeStateDtmObject(dtmP,DTMState::Data)) goto errexit ;
            }
        /*
        **  Copy Feature Points Over Random Spots
        */ 
        numPoints = 0 ;
        for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
            firstPoint = dtmFeatureP->dtmFeaturePts.firstPoint ;
            lastPoint  = firstPoint + dtmFeatureP->numDtmFeaturePts - 1 ;
            dtmFeatureP->dtmFeaturePts.firstPoint = numPoints ;
            for( point = firstPoint ; point <= lastPoint ; ++point )
                {
                pointP = pointAddrP(dtmP,point) ;
                numPointsP = pointAddrP(dtmP,numPoints) ;
                *numPointsP = *pointP ;
                ++numPoints ;
                }
            }
        dtmP->numPoints = numPoints ; 
        }
    /*
    ** Scan For Feature Type And mark Points
    */
    else if( dtmP->numFeatures > 0 ) 
        { 

        //  Delete Features From DTMFeatureState::Tin State DTM

        if( dtmP->dtmState == DTMState::Tin )
            {   

            for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
                {
                dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
                if( dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted && dtmFeatureP->dtmFeatureType == dtmFeatureType )
                    {
                    isModified = TRUE;
                    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Removing DTM Feature With Id = %10I64d",dtmFeatureP->dtmFeatureId) ;
                    if( bcdtmInsert_removeDtmFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ;
                    }
                }
            }

        //  Delete Features From DTM State Data

        if( dtmP->dtmState == DTMState::Data )
            { 

            if( dbg ) bcdtmWrite_message(0,0,0,"Deleting From Data State DTM ** dtmP->numPoints = %8ld dtmP->numFeatures = %8ld",dtmP->numPoints,dtmP->numFeatures) ;

            //     Allocate Memory For Deleted Point Flags

            delPointsP = ( unsigned char * ) malloc( ( dtmP->numPoints / 8 + 1 ) * sizeof( char )) ;
            if( delPointsP == NULL )
                {
                bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                goto errexit ;
                }

            //     Initialise Flags

            for( delP = delPointsP ; delP < delPointsP + ( dtmP->numPoints / 8 + 1) ; ++delP ) *delP = ( unsigned char ) 255 ;

            //     Scan Features And Mark Delete Points

            if( dbg ) bcdtmWrite_message(0,0,0,"Marking Deleted Feature Points") ;
            for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
                {
                dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
                if( dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted && dtmFeatureP->dtmFeatureType == dtmFeatureType )
                    {
                    isModified = TRUE;
                    if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Removing DTM Feature[%8ld] With Id = %10I64d",dtmFeature,dtmFeatureP->dtmFeatureId) ;
                    firstPoint = dtmFeatureP->dtmFeaturePts.firstPoint ;
                    lastPoint  = firstPoint + dtmFeatureP->numDtmFeaturePts - 1 ;
                    for( point = firstPoint ; point <= lastPoint ; ++point )
                        {
                        bcdtmFlag_clearFlag(delPointsP,point) ;
                        } 
                    dtmFeatureP->dtmFeatureState = DTMFeatureState::Deleted ;   
                    }
                }

            //     Allocate Memory For Offset Counts

            offsCountP = ( long * ) malloc( dtmP->numPoints * sizeof(long)) ;  
            if( offsCountP == NULL )
                {
                bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                goto errexit ;
                }

            //     Initialise Counts

            for( ofsP = offsCountP ; ofsP < offsCountP + dtmP->numPoints  ; ++ofsP ) *ofsP = 0 ;

            //     Set Counts 

            lastCount = 0 ;
            for( point = 0 ; point < dtmP->numPoints ; ++point )
                {
                if( ! bcdtmFlag_testFlag(delPointsP,point)) 
                    {
                    *(offsCountP+point) = lastCount ;
                    ++lastCount ;
                    }
                else  *(offsCountP+point) = lastCount ; 
                } 

            //     Copy Over Deleted Points

            for( pnt1 = pnt2 = 0 ; pnt2 < dtmP->numPoints ; ++pnt2 )
                {
                if( bcdtmFlag_testFlag(delPointsP,pnt2))
                    {
                    *pointAddrP(dtmP,pnt1) = *pointAddrP(dtmP,pnt2) ;
                    ++pnt1 ; 
                    }
                } 
            dtmP->numPoints = pnt1 ;  

            //     Reset Feature First Points              

            for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
                {
                dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
                if( dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted )
                    {
                    dtmFeatureP->dtmFeaturePts.firstPoint = dtmFeatureP->dtmFeaturePts.firstPoint - *(offsCountP+dtmFeatureP->dtmFeaturePts.firstPoint)  ;
                    } 
                }   

            //     Copy Over Deleted Features

            for( pnt1 = pnt2 = 0 ; pnt2 < dtmP->numFeatures ; ++pnt2 )
                {
                if( ftableAddrP(dtmP,pnt2)->dtmFeatureState != DTMFeatureState::Deleted )
                    {
                    if( pnt1 != pnt2 ) *ftableAddrP(dtmP,pnt1) = *ftableAddrP(dtmP,pnt2) ;
                    ++pnt1 ;
                    }
                }    
            dtmP->numFeatures = pnt1  ;

            //     Resize Array Memory

            if( bcdtmObject_resizeMemoryDtmObject(dtmP)) goto errexit ;

            //     Log Final Number Of Points And Features

            if( dbg ) bcdtmWrite_message(0,0,0,"After Deleting From Data State DTM ** dtmP->numPoints = %8ld dtmP->numFeatures = %8ld",dtmP->numPoints,dtmP->numFeatures) ;

            }
        }   
    /*
    **  Update Modified Time
    */
    if( isModified )
        {
        bcdtmObject_updateLastModifiedTime (dtmP) ;
        } 
    /*
    ** Clean Up
    */
cleanup :
    if( delPointsP != NULL ) free(delPointsP) ;
    /*
    ** Job Completed
    */
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Deleting All Dtm Features Of Type Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Deleting All Dtm Features Of Type Error") ;
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmData_deleteAllOccurrencesOfDtmFeaturesWithUserTagDtmObject(BC_DTM_OBJ *dtmP,DTMUserTag userTag )
    /*
    ** Remove All Dtm Features 
    */
    {
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
    long   dtmFeature,dtmFeature1,dtmFeature2 ;
    BC_DTM_FEATURE  *dtmFeatureP,*dtmFeature1P,*dtmFeature2P ;
    /*
    ** Write Entry Message
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Deleting Features With Usertag") ;
        bcdtmWrite_message(0,0,0,"dtmP      = %p",dtmP) ;
        bcdtmWrite_message(0,0,0,"usertag   = %10I64d",userTag) ;
        }
    /*
    ** Check For Valid Dtm Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Scan For Feature Type
    */
    if( dtmP->numFeatures > 0 ) 
        { 
        for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
            if( dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted && dtmFeatureP->dtmUserTag == userTag )
                {
                if( dbg ) bcdtmWrite_message(0,0,0,"Removing Feature %6ld ** Type = %3ld Usertag = %10I64d",dtmFeature,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag) ;
                if( bcdtmInsert_removeDtmFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ;
                }
            }
        }
    /*
    ** Write Deleted Features
    */
    if( dbg )
        {
        for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
            if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Deleted )
                {
                bcdtmWrite_message(0,0,0,"Deleted Dtm Feature ** dtmFeature = %8ld Type = %8ld userTag = %10I64d featureId = %10I64d",dtmFeature,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId) ;
                }
            } 
        }
    /*
    ** Remove Deleted Feature
    */
    if( dtmP->numFeatures > 0 ) 
        { 
        /*
        **  Copy Over Deleted Features
        */
        for( dtmFeature1 = dtmFeature2 = 0 ; dtmFeature2 < dtmP->numFeatures ; ++dtmFeature2 )
            {
            dtmFeature2P = ftableAddrP(dtmP,dtmFeature2) ;
            if( dtmFeature2P->dtmFeatureState != DTMFeatureState::Deleted )
                {
                if( dtmFeature2 != dtmFeature1 )
                    {
                    dtmFeature1P = ftableAddrP(dtmP,dtmFeature1) ;
                    *dtmFeature1P = *dtmFeature2P ;
                    } 
                ++dtmFeature1 ;  
                } 
            }
        /*
        **  Update Modified Time
        */
        if (dtmP->numFeatures != dtmFeature1)
            bcdtmObject_updateLastModifiedTime (dtmP) ;
        /*
        **  Reset Number Of Features
        */
        dtmP->numFeatures = dtmFeature1 ; 
        }
    /*
    ** Clean Up
    */
cleanup :
    /*
    ** Job Completed
    */
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Deleting Features With Usertag Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Deleting Features With Usertag Error") ;
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmData_deleteAllOccurrencesOfDtmFeaturesWithFeatureIdDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureId userFeatureId )
    /*
    ** Remove All Dtm Features 
    */
    {
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
    long   dtmFeature ;
    int isModifed = FALSE;
    BC_DTM_FEATURE  *dtmFeatureP ;
    /*
    ** Write Entry Message
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Deleting All Dtm Features With Feature Id") ;
        bcdtmWrite_message(0,0,0,"dtmP          = %p",dtmP) ;
        bcdtmWrite_message(0,0,0,"userFeatureId = %8I64d",userFeatureId) ;
        } 
    /*
    ** Check For Valid Dtm Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Scan For Feature Type
    */
    if( dtmP->numFeatures > 0 ) 
        { 
        for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
            if( dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted && memcmp(&dtmFeatureP->dtmFeatureId,&userFeatureId,sizeof(DTMFeatureId)) == 0 )
                {
                if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Feature %8ld State = %2ld ** FeatureID = %10I64d",dtmFeature,dtmFeatureP->dtmFeatureState,dtmFeatureP->dtmFeatureId) ;         
                isModifed = TRUE;
                if( bcdtmInsert_removeDtmFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ;
                }
            }
        }
    /*
    **  Update Modified Time
    */
    if( isModifed )
        {
        bcdtmObject_updateLastModifiedTime (dtmP) ;
        } 
    /*
    ** Clean Up
    */
cleanup :
    /*
    ** Job Completed
    */
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Deleting All Dtm Features With Feature Id Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Deleting All Dtm Features With Feature Id Error") ;
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmData_compactUntriangulatedFeatureTableDtmObject(BC_DTM_OBJ *dtmP )
    /*
    ** Remove All Dtm Features 
    */
    {
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
    long   dtmFeature1,dtmFeature2 ;
    BC_DTM_FEATURE  *dtmFeature1P,*dtmFeature2P ;
    /*
    ** Write Entry Message
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Compacting Untriangulated Feature Table") ;
        bcdtmWrite_message(0,0,0,"dtmP           = %p",dtmP) ;
        bcdtmWrite_message(0,0,0,"dtmP->dtmState = %p",dtmP->dtmState) ;
        } 
    /*
    ** Check For Valid Dtm Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Check DTM Is In Untriangulated State
    */
    if( dtmP->dtmState != DTMState::Data )
        {
        bcdtmWrite_message(2,0,0,"Method Requires Untriangulated DTM") ;
        goto errexit ;
        }
    /*
    ** Scan For Feature Type
    */
    if( dtmP->numFeatures > 0 ) 
        { 
        /*
        **  Copy Over Deleted Features
        */
        for( dtmFeature1 = dtmFeature2 = 0 ; dtmFeature2 < dtmP->numFeatures ; ++dtmFeature2 )
            {
            dtmFeature2P = ftableAddrP(dtmP,dtmFeature2) ;
            if( dtmFeature2P->dtmFeatureState != DTMFeatureState::Deleted )
                {
                if( dtmFeature2 != dtmFeature1 )
                    {
                    dtmFeature1P = ftableAddrP(dtmP,dtmFeature1) ;
                    *dtmFeature1P = *dtmFeature2P ;
                    } 
                ++dtmFeature1 ;  
                } 
            }
        /*
        **  Update Modified Time
        */
        if (dtmP->numFeatures != dtmFeature1)
            bcdtmObject_updateLastModifiedTime (dtmP) ;
        /*
        **  Reset Number Of Features
        */
        dtmP->numFeatures = dtmFeature1 ; 
        /*
        **  Resize Feature Table
        */
        if( bcdtmObject_resizeMemoryDtmObject(dtmP)) goto errexit ;
        }
    /*
    ** Clean Up
    */
cleanup :
    /*
    ** Job Completed
    */
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Compacting Untriangulated Feature Table Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Compacting Untriangulated Feature Table Error") ;
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmData_compactFeatureTableDtmObject(BC_DTM_OBJ *dtmP )
    /*
    ** Removes All Deleted Dtm Features 
    */
    {
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
    /*
    ** Write Entry Message
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Compacting Feature Table") ;
        bcdtmWrite_message(0,0,0,"dtmP           = %p",dtmP) ;
        bcdtmWrite_message(0,0,0,"dtmP->dtmState = %8ld",dtmP->dtmState) ;
        } 
    /*
    ** Check For Valid Dtm Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Compact Depending On State
    */
    if( dtmP->dtmState == DTMState::Data ) if( bcdtmData_compactUntriangulatedFeatureTableDtmObject(dtmP)) goto errexit ;
    if( dtmP->dtmState == DTMState::Tin  ) if( bcdtmTin_compactFeatureTableDtmObject(dtmP)) goto errexit ;
    /*
    ** Clean Up
    */
cleanup :
    /*
    ** Job Completed
    */
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Compacting Feature Table Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Compacting Feature Table Error") ;
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmData_changeAllOccurrencesOfDtmFeatureTypeDtmObject
    (
    BC_DTM_OBJ *dtmP,
    DTMFeatureType dtmFeatureType1,
    DTMFeatureType dtmFeatureType2
    )
    /*
    ** This Function Changes All The Occurrences Of A DTM Feature Type To Another Type
    */
    {
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
    long    dtmFeature ;
    BC_DTM_FEATURE *dtmFeatureP ;
    /*
    ** Write Status Message
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Changing All Occurrences Of DTM Feature Type") ;
    /*
    ** Test For Valid Dtm Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Check For Valid DTM Feature Types
    */
    if( bcdtmData_testForValidDtmObjectImportFeatureType(dtmFeatureType1) || bcdtmData_testForValidDtmObjectImportFeatureType(dtmFeatureType2))
        {
        bcdtmWrite_message(2,0,0,"Invalid DTM Feature Type") ;
        goto errexit ;
        }
    /*
    ** Scan Dtm Features
    */
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
        {
        dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
        if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data || dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin || dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray || dtmFeatureP->dtmFeatureState == DTMFeatureState::OffsetsArray )
            {
            if( dtmFeatureP->dtmFeatureType == dtmFeatureType1 ) dtmFeatureP->dtmFeatureType = (DTMFeatureType)dtmFeatureType2 ; 
            }
        }
    /*
    ** Clean Up
    */
cleanup :
    /*
    ** Job Completed
    */
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Changing All Occurrence Of DTM Feature Type Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Changing All Occurrence Of DTM Feature Type Error") ;
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup  ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int  bcdtmData_moveZDtmObject
    (
    BC_DTM_OBJ *dtmP,
    long       moveOption,
    double     z
    )
    /*
    ** This Function Changes The z Value
    ** moveOption  ==  1  Set To z Value
    **             ==  2  Increment By z Value
    */
    {
    int  ret=DTM_SUCCESS ;
    long point,dtmFeature ;
    DPoint3d  *p3dP,*ptsP ;
    DTM_TIN_POINT  *pointP ;
    BC_DTM_FEATURE *dtmFeatureP ;
    /*
    ** Test For Valid Data Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Only Move For DTM Object In Data Or Tin States
    */
    if( dtmP->dtmState == DTMState::Data || dtmP->dtmState == DTMState::Tin )
        {
        /*
        **  Move Point z Values
        */
        for( point = 0 ; point < dtmP->numPoints ; ++point )
            {
            pointP = pointAddrP(dtmP,point) ;
            if( moveOption == 1 ) pointP->z = z ;
            if( moveOption == 2 ) pointP->z = pointP->z + z ;
            }
        /*
        **  Move Point Array z Values
        */
        for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
            if( dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray )
                {
                ptsP = bcdtmMemory_getPointerP3D(dtmP,dtmFeatureP->dtmFeaturePts.pointsPI) ;
                for( p3dP = ptsP ; p3dP < ptsP + dtmFeatureP->numDtmFeaturePts ; ++p3dP )
                    {
                    if( moveOption == 1 ) p3dP->z = z ;
                    if( moveOption == 2 ) p3dP->z = p3dP->z + z ;
                    }         
                }
            }
        }
    /*
    ** Reset Bounding Cube
    */
    if( bcdtmMath_setBoundingCubeDtmObject(dtmP)) goto errexit ;
    /*
    **  Update Modified Time
    */
    bcdtmObject_updateLastModifiedTime (dtmP) ;
    /*
    ** Clean Up
    */
cleanup :
    /*
    ** Job Completed
    */
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int  bcdtmData_moveZUsingTagDtmObject
    (
    BC_DTM_OBJ   *dtmP,
    DTMUserTag usertag,
    long         moveOption,
    double       z
    )
    /*
    ** This Function Changes The z Value
    ** moveOption  ==  1  Set To z Value
    **             ==  2  Increment By z Value
    */
    {
    int ret=DTM_SUCCESS ;
    long point,dtmFeature ;
    DPoint3d  *p3dP,*ptsP ;
    DTM_TIN_POINT  *pointP ;
    BC_DTM_FEATURE *dtmFeatureP ;
    /*
    ** Test For Valid Data Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Only Move For DTM Object In Data Or Tin States
    */
    if( dtmP->dtmState == DTMState::Data || dtmP->dtmState == DTMState::Tin )
        {
        for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ; 
            if( dtmFeatureP->dtmUserTag == usertag )
                {
                /*
                **        Tin Feature
                */
                if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
                    {
                    point = dtmFeatureP->dtmFeaturePts.firstPoint ;
                    do
                        {
                        pointP = pointAddrP(dtmP,point) ;
                        if( moveOption == 1 ) pointP->z = z ;
                        if( moveOption == 2 ) pointP->z = pointP->z + z ;
                        if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,point,&point)) goto errexit ;           
                        } while ( point != dtmP->nullPnt && point != dtmFeatureP->dtmFeaturePts.firstPoint ) ;
                    }
                /*
                **        Data Feature
                */
                if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data )
                    {
                    point = dtmFeatureP->dtmFeaturePts.firstPoint ;
                    for( point = dtmFeatureP->dtmFeaturePts.firstPoint ; point < dtmFeatureP->dtmFeaturePts.firstPoint + dtmFeatureP->numDtmFeaturePts ; ++point )
                        {
                        pointP = pointAddrP(dtmP,point) ;
                        if( moveOption == 1 ) pointP->z = z ;
                        if( moveOption == 2 ) pointP->z = pointP->z + z ;
                        } 
                    } 
                /*
                **        Point Array Feature
                */
                if( dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray )
                    {
                    ptsP = bcdtmMemory_getPointerP3D(dtmP,dtmFeatureP->dtmFeaturePts.pointsPI) ;
                    for( p3dP = ptsP ; p3dP < ptsP + dtmFeatureP->numDtmFeaturePts ; ++p3dP )
                        {
                        if( moveOption == 1 ) ptsP->z = z ;
                        if( moveOption == 2 ) ptsP->z = ptsP->z + z ;
                        } 
                    } 
                }
            }
        }
    /*
    ** Clean Up
    */
cleanup :
    /*
    ** Job Completed
    */
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int  bcdtmData_moveXYZDtmObject
    (
    BC_DTM_OBJ *dtmP,
    double     x,
    double     y,
    double     z
    )
    /*
    ** This Function Changes The Point XYZ Values
    */
    {
    int  ret=DTM_SUCCESS ;
    long point,dtmFeature ;
    DPoint3d  *p3dP,*ptsP ;
    DTM_TIN_POINT  *pointP ;
    BC_DTM_FEATURE *dtmFeatureP ;
    /*
    ** Test For Valid Data Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Only Move For DTM Object In Data Or Tin States
    */
    if( dtmP->dtmState == DTMState::Data || dtmP->dtmState == DTMState::Tin )
        {
        /*
        **  Move Point XYZ Values
        */
        for( point = 0 ; point < dtmP->numPoints ; ++point )
            {
            pointP = pointAddrP(dtmP,point) ;
            pointP->x = pointP->x + x ;
            pointP->y = pointP->y + y ;
            pointP->z = pointP->z + z ;
            }
        /*
        **  Move Point Array z Values
        */
        for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
            if( dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray )
                {
                ptsP = bcdtmMemory_getPointerP3D(dtmP,dtmFeatureP->dtmFeaturePts.pointsPI) ;
                for( p3dP = ptsP ; p3dP < ptsP + dtmFeatureP->numDtmFeaturePts ; ++p3dP )
                    {
                    p3dP->x = p3dP->x + x ;
                    p3dP->y = p3dP->y + y ;
                    p3dP->z = p3dP->z + z ;
                    }         
                }
            }
        }
    /*
    ** Clean Up
    */
cleanup :
    /*
    ** Job Completed
    */
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int  bcdtmData_moveXYZUsingTagDtmObject
    (
    BC_DTM_OBJ   *dtmP,
    DTMUserTag usertag,
    double       x,
    double       y,
    double       z
    )
    /*
    ** This Function Changes The z Value
    ** moveOption  ==  1  Set To z Value
    **             ==  2  Increment By z Value
    */
    {
    int ret=DTM_SUCCESS ;
    long point,dtmFeature ;
    DPoint3d  *p3dP,*ptsP ;
    DTM_TIN_POINT  *pointP ;
    BC_DTM_FEATURE *dtmFeatureP ;
    /*
    ** Test For Valid Data Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Only Move For DTM Object In Data Or Tin States
    */
    if( dtmP->dtmState == DTMState::Data || dtmP->dtmState == DTMState::Tin )
        {
        for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ; 
            if( dtmFeatureP->dtmUserTag == usertag )
                {
                /*
                **        Tin Feature
                */
                if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
                    {
                    point = dtmFeatureP->dtmFeaturePts.firstPoint ;
                    do
                        {
                        pointP = pointAddrP(dtmP,point) ;
                        pointP->x = pointP->x + x ;
                        pointP->y = pointP->y + y ;
                        pointP->z = pointP->z + z ;
                        if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,point,&point)) goto errexit ;           
                        } while ( point != dtmP->nullPnt && point != dtmFeatureP->dtmFeaturePts.firstPoint ) ;
                    }
                /*
                **        Data Feature
                */
                if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data )
                    {
                    point = dtmFeatureP->dtmFeaturePts.firstPoint ;
                    for( point = dtmFeatureP->dtmFeaturePts.firstPoint ; point < dtmFeatureP->dtmFeaturePts.firstPoint + dtmFeatureP->numDtmFeaturePts ; ++point )
                        {
                        pointP = pointAddrP(dtmP,point) ;
                        pointP->x = pointP->x + x ;
                        pointP->y = pointP->y + y ;
                        pointP->z = pointP->z + z ;
                        } 
                    } 
                /*
                **        Point Array Feature
                */
                if( dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray )
                    {
                    ptsP = bcdtmMemory_getPointerP3D(dtmP,dtmFeatureP->dtmFeaturePts.pointsPI) ;
                    for( p3dP = ptsP ; p3dP < ptsP + dtmFeatureP->numDtmFeaturePts ; ++p3dP )
                        {
                        p3dP->x = p3dP->x + x ;
                        p3dP->y = p3dP->y + y ;
                        p3dP->z = p3dP->z + z ;
                        } 
                    } 
                }
            }
        }
    /*
    ** Clean Up
    */
cleanup :
    /*
    ** Job Completed
    */
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmData_joinAdjacentDtmObjects(DTM_DAT_OBJLIST *joinObjectsP,long numJoinObjects,double xyTolerance,double zTolerance) 
    /*
    ** This Function Joins The Data Objects In The Data Object List If
    ** Points And/Or Lines Are Within The XY & z Tolerances
    **
    ** ArgueMents
    **
    ** joinObjectsP   ==> List Of Data Object Pointers To Be Joined
    ** numJoinObjects ==> Number Of Objects To Be Scanned
    ** xyTolerance    ==> Horizontal Tolerance Between Points And/Or Lines
    ** zTolerance     ==> Vertical Tolerance Between Points And/Or Lines
    **
    ** Return Values
    **
    ** 0 Succesfull
    ** 1 Error Detected 
    **
    ** Author :  Rob Cormack
    ** Date   :  21st january 2002
    **
    */
    {
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
    long  appendFlag,totalPoints,rescanFlag ;
    DTM_DAT_OBJLIST *ol1P,*ol2P ;
    BC_DTM_OBJ *dtmP=NULL ;
    /*
    ** Write Entry Message
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Joining Adjacent Dtm Objects") ;
        bcdtmWrite_message(0,0,0,"Join Objects            = %p",joinObjectsP) ;
        bcdtmWrite_message(0,0,0,"Number Of Join Objects  = %4ld",numJoinObjects) ;
        bcdtmWrite_message(0,0,0,"XY Tolerance            = %10.4lf",xyTolerance) ;
        bcdtmWrite_message(0,0,0,"z  Tolerance            = %10.4lf",zTolerance) ;
        }
    /*
    ** Validate
    */
    if( joinObjectsP == NULL ) { bcdtmWrite_message(2,0,0,"joinObjectsP Set To NULL") ; goto errexit ; }
    if( numJoinObjects <= 0  ) { bcdtmWrite_message(2,0,0,"Invalid Value For numJoinObjects") ; goto errexit ; }
    for( ol1P = joinObjectsP ; ol1P < joinObjectsP + numJoinObjects ; ++ol1P )
        {
        /*
        **  Set Flag To One
        */
        ol1P->ListFlag = 1 ;
        /*
        **  Check For Valid Data Object And Recalulate Bounding Cube
        */
        if( cdbg )
            {
            if( bcdtmObject_testForValidDtmObject((BC_DTM_OBJ *) ol1P->Data)) goto errexit ;
            if( bcdtmMath_setBoundingCubeDtmObject((BC_DTM_OBJ *) ol1P->Data)) goto errexit ;
            }
        } 
    /*
    ** Write Before Join ** Development Only
    */
    if( dbg )
        {
        totalPoints = 0 ;
        bcdtmWrite_message(0,0,0,"Before Join Number Of Data Objects = %6ld",numJoinObjects) ;
        for( ol1P = joinObjectsP ; ol1P < joinObjectsP + numJoinObjects ; ++ol1P )
            {
            dtmP = (BC_DTM_OBJ *) (ol1P->Data) ;
            bcdtmWrite_message(0,0,0,"Object[%6ld] = %p  ListFlag = %1ld numPts = %8ld",(long)(ol1P-joinObjectsP),ol1P->Data,ol1P->ListFlag,dtmP->numPoints) ;
            totalPoints = totalPoints + dtmP->numPoints ;
            } 
        bcdtmWrite_message(0,0,0,"Total Points Before Join = %8ld",totalPoints) ;
        } 
    /*
    ** Append Data Objects
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Appending Data Objects") ;
    for( ol1P = joinObjectsP ; ol1P < joinObjectsP + numJoinObjects ; ++ol1P )
        {
        rescanFlag = 0 ;
        if( ol1P->ListFlag )
            {
            for( ol2P = ol1P + 1 ; ol2P < joinObjectsP + numJoinObjects ; ++ol2P )
                {
                if( ol2P->ListFlag )
                    {
                    if( bcdtmData_appendDtmObjectsIfAdjacentDtmObject((BC_DTM_OBJ *)ol1P->Data,(BC_DTM_OBJ *)ol2P->Data,xyTolerance,zTolerance,&appendFlag) ) goto errexit ;
                    if( appendFlag ) { ol2P->ListFlag = 0 ; rescanFlag = 1 ; } 
                    }
                }
            }
        if( rescanFlag ) --ol1P ;
        }
    /*
    ** Write Append Results ** Development Only
    */
    if( dbg )
        {
        totalPoints = 0 ;
        bcdtmWrite_message(0,0,0,"After Join Number Of Data Objects = %6ld",numJoinObjects) ;
        for( ol1P = joinObjectsP ; ol1P < joinObjectsP + numJoinObjects ; ++ol1P )
            {
            dtmP = (BC_DTM_OBJ *) (ol1P->Data) ;
            bcdtmWrite_message(0,0,0,"Object[%6ld] = %p  ListFlag = %1ld numPts = %8ld",(long)(ol1P-joinObjectsP),ol1P->Data,ol1P->ListFlag,dtmP->numPoints) ;
            totalPoints = totalPoints + dtmP->numPoints ;
            } 
        bcdtmWrite_message(0,0,0,"Total Points After Join = %8ld",totalPoints) ;
        } 
    /*
    ** Clean Up 
    */
cleanup :
    /*
    ** Job Completed
    */
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Joining Adjacent Data Objects Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Joining Adjacent Data Objects Error") ;
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmData_appendDtmObjectsIfAdjacentDtmObject(BC_DTM_OBJ *dtm1P,BC_DTM_OBJ *dtm2P,double xyTolerance,double zTolerance,long *appendFlag) 
    /*
    ** This Function Appends Data Object Two,To Data Object One 
    ** If One Or More Points In Both Data Objects Are Within The
    ** Join Tolerance Or A Point In Either Data Object Is Within The Join 
    ** Tolerance Of A Line In The Other Data Object
    **
    ** ArgueMents
    **
    ** dtm1P          ==> Data Object To Append To
    ** dtm2P          ==> Data Object To Be Appended
    ** xyTolerance    ==> Horizontal Tolerance Between Points And/Or Lines
    ** zTolerance     ==> Vertical Tolerance Between Points And/Or Lines
    ** appendFlag     <==  0  Data Objects Not Appended
    **                <==  1  Data Objects Appended
    **
    ** No Arguement Validation. Assumes Arguements Are Prior Validated 
    **
    ** Return Values
    **
    ** 0 Succesfull
    ** 1 Error Detected 
    **
    ** Author :  Rob Cormack
    ** Date   :  21st january 2002
    **
    */
    {
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ; 
    long   cubeOverlap ;
    double Xmin=0.0,Ymin=0.0,Zmin=0.0,Xmax=0.0,Ymax=0.0,Zmax=0.0 ;
    /*
    ** Write Status message
    */
    if( dbg ) 
        {
        bcdtmWrite_message(0,0,0,"Appending Adjacent Data Objects") ;
        bcdtmWrite_message(0,0,0,"Data One Object = %p",dtm1P) ;
        bcdtmWrite_message(0,0,0,"Data Two Object = %p",dtm2P) ;
        bcdtmWrite_message(0,0,0,"XY Tolerance    = %10.4lf",xyTolerance) ;
        bcdtmWrite_message(0,0,0,"z  Tolerance    = %10.4lf",zTolerance) ;
        }
    /*
    ** Initialise
    */
    *appendFlag = 0 ;
    if( xyTolerance < 0.0 ) xyTolerance = - xyTolerance ;
    if( zTolerance  < 0.0 ) zTolerance  = - zTolerance  ;
    /*
    ** Test For OverLap Of Bounding Cubes
    */
    cubeOverlap = 0 ; 
    if( dtm1P->xMax + xyTolerance >= dtm2P->xMin && dtm1P->xMin - xyTolerance <= dtm2P->xMax  &&
        dtm1P->yMax + xyTolerance >= dtm2P->yMin && dtm1P->yMin - xyTolerance <= dtm2P->yMax  &&
        dtm1P->zMax + zTolerance  >= dtm2P->zMin && dtm1P->zMin - zTolerance  <= dtm2P->zMax      ) cubeOverlap = 1 ;
    if( dbg && ! cubeOverlap ) bcdtmWrite_message(0,0,0,"Bounding Cubes Do Not Overlap") ;
    if( dbg &&   cubeOverlap ) bcdtmWrite_message(0,0,0,"Bounding Cubes Overlap") ;
    /*
    ** If Cube OverLap Get Overlap Cube
    */
    if( cubeOverlap )
        {
        /*
        ** Get Cube Min And Max Values
        */
        if( dtm1P->xMin >= dtm2P->xMin ) Xmin = dtm1P->xMin ;
        else                               Xmin = dtm2P->xMin ;
        if( dtm1P->xMax <= dtm2P->xMax ) Xmax = dtm1P->xMax ;
        else                               Xmax = dtm2P->xMax ;
        if( dtm1P->yMin >= dtm2P->yMin ) Ymin = dtm1P->yMin ;
        else                               Ymin = dtm2P->yMin ;
        if( dtm1P->yMax <= dtm2P->yMax ) Ymax = dtm1P->yMax ;
        else                               Ymax = dtm2P->yMax ;
        if( dtm1P->zMin >= dtm2P->zMin ) Zmin = dtm1P->zMin ;
        else                               Zmin = dtm2P->zMin ;
        if( dtm1P->yMax <= dtm2P->yMax ) Zmax = dtm1P->zMax ;
        else                               Zmax = dtm2P->zMax ;
        if( dbg ) bcdtmWrite_message(0,0,0,"00 Overlap Cube Min = %10.4lf %10.4lf %10.4lf Max = %10.4lf %10.4lf %10.4lf",Xmin,Ymin,Zmin,Xmax,Ymax,Zmax) ;
        /*
        ** Modify Cube Min And Max Values By Tolerances
        */
        Xmin = Xmin - xyTolerance ;
        Ymin = Ymin - xyTolerance ;
        Zmin = Zmin - zTolerance  ;
        Xmax = Xmax + xyTolerance ;
        Ymax = Ymax + xyTolerance ;
        Zmax = Zmax + zTolerance  ;
        if( dbg ) bcdtmWrite_message(0,0,0,"01 Overlap Cube Min = %10.4lf %10.4lf %10.4lf Max = %10.4lf %10.4lf %10.4lf",Xmin,Ymin,Zmin,Xmax,Ymax,Zmax) ;
        } 
    /*
    ** If Cube Overlap Test For Adjacency
    */
    if( cubeOverlap ) if( bcdtmData_testForAdjacenyOfDtmObjectsDtmObject(dtm1P,dtm2P,Xmin,Ymin,Zmin,Xmax,Ymax,Zmax,xyTolerance,zTolerance,appendFlag) ) goto errexit ;
    /*
    ** If Adjacent Append Data Objects
    */
    if( *appendFlag )
        {
        if( bcdtmObject_appendDtmObject(dtm1P,dtm2P)) goto errexit ;
        if( bcdtmObject_initialiseDtmObject(dtm2P)) goto errexit ;    
        }
    /*
    ** Clean Up 
    */
cleanup :
    /*
    ** Job Completed
    */
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Appending Adjacent Data Objects Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Appending Adjacent Data Objects Error") ;
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmData_testForAdjacenyOfDtmObjectsDtmObject(BC_DTM_OBJ *dtm1P,BC_DTM_OBJ *dtm2P,double Xmin,double Ymin,double Zmin,double Xmax,double Ymax,double Zmax,double xyTolerance,double zTolerance,long *adjacencyFlagP) 
    {
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
    long    point,point1,point2,onLine,numMarkPts,numFeaturePts ;
    long    dtmFeature,numPointsCopied,numFeaturesCopied,copyFeature ;
    unsigned char    *mpP,*markPtsP=NULL ;
    double  dx,dy,nd,Xi,Yi,Zi ; 
    DPoint3d     *featurePtsP=NULL ;
    BC_DTM_OBJ      *test1P=NULL,*test2P=NULL ;
    BC_DTM_FEATURE  *dtmFeatureP ;
    DTM_TIN_POINT   *pointP,*point1P,*point2P ;
    DTMFeatureId  nullFeatureId=DTM_NULL_FEATURE_ID  ; 
    /*
    ** Write Status Message
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Testing For Adjacency Of Data Objects") ;
        bcdtmWrite_message(0,0,0,"dtm1P = %p ** dtm1P->numPoints = %6ld",dtm1P,dtm1P->numPoints) ;
        bcdtmWrite_message(0,0,0,"dtm2P = %p ** dtm2P->numPoints = %6ld",dtm2P,dtm2P->numPoints) ;
        }
    /*
    ** Initialise
    */
    *adjacencyFlagP = 0 ;
    /*
    ** Create Data Objects
    */
    if( bcdtmObject_createDtmObject(&test1P)) goto errexit ;
    bcdtmObject_setPointMemoryAllocationParametersDtmObject(test1P,10000,10000) ;
    if( bcdtmObject_createDtmObject(&test2P)) goto errexit ;
    bcdtmObject_setPointMemoryAllocationParametersDtmObject(test2P,10000,10000) ;
    /*
    ** Allocate Memory For Marking Random Points
    */
    numMarkPts = dtm1P->numPoints ;
    markPtsP   = ( unsigned char * ) malloc ( ( numMarkPts / 8 + 1 ) * sizeof(char)) ;
    if( markPtsP == NULL )
        {
        bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
        goto errexit ;
        }
    for( mpP = markPtsP ; mpP < markPtsP + numMarkPts / 8 + 1 ; ++mpP) *mpP = ( char ) 0 ;
    /*
    ** Unmark Feature Points
    */
    for( dtmFeature = 0 ; dtmFeature < dtm1P->numFeatures ; ++dtmFeature )
        {
        dtmFeatureP = ftableAddrP(dtm1P,dtmFeature) ;
        if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data )
            {
            for( point = dtmFeatureP->dtmFeaturePts.firstPoint ; point < dtmFeatureP->dtmFeaturePts.firstPoint + dtmFeatureP->numDtmFeaturePts ; ++point )
                {
                bcdtmFlag_setFlag(markPtsP,point) ;
                }
            }
        }
    /*
    ** Copy Random Points Bounding Cube From dtm1P To test1P
    */
    numPointsCopied = 0 ;
    for( point = 0 ; point < dtm1P->numPoints ; ++point )
        {
        pointP = pointAddrP(dtm1P,point) ;  
        if( ! bcdtmFlag_testFlag(markPtsP,point) && pointP->x >= Xmin && pointP->x <= Xmax && pointP->y >= Ymin && pointP->y <= Ymax  && pointP->z >= Zmin && pointP->z <= Zmax )
            {
            if( bcdtmObject_storeDtmFeatureInDtmObject(test1P,DTMFeatureType::RandomSpots,DTM_NULL_USER_TAG,1,&nullFeatureId,(DPoint3d *)pointP,1)) goto errexit ;
            ++numPointsCopied ;
            }
        }
    /*
    ** Copy DTM Features Within Bounding Cube
    */ 
    numFeaturesCopied = 0 ;
    for( dtmFeature = 0 ; dtmFeature < dtm1P->numFeatures ; ++dtmFeature )
        {
        dtmFeatureP = ftableAddrP(dtm1P,dtmFeature) ;
        if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data )
            {
            copyFeature = FALSE ;
            for( point = dtmFeatureP->dtmFeaturePts.firstPoint ; point < dtmFeatureP->dtmFeaturePts.firstPoint + dtmFeatureP->numDtmFeaturePts && copyFeature == FALSE ; ++point )
                {
                pointP = pointAddrP(dtm1P,point) ;  
                if( pointP->x >= Xmin && pointP->x <= Xmax && pointP->y >= Ymin && pointP->y <= Ymax  && pointP->z >= Zmin && pointP->z <= Zmax ) copyFeature = TRUE ;
                }
            if( copyFeature == TRUE )
                {
                if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtm1P,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
                if( bcdtmObject_storeDtmFeatureInDtmObject(test1P,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,2,&dtmFeatureP->dtmFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
                if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
                ++numFeaturesCopied ;
                }
            }
        }
    /*
    ** Write test1P Statistics ** Development Only
    */
    if( dbg ) 
        {
        bcdtmWrite_message(0,0,0,"Number Of Random   Copied From dtm1P to test1P = %8ld",numPointsCopied) ;
        bcdtmWrite_message(0,0,0,"Number Of Features Copied From dtm1P to test1P = %8ld",numFeaturesCopied) ;
        bcdtmWrite_message(0,0,0,"test1P->numPoints = %6ld",test1P->numPoints) ;
        bcdtmWrite_message(0,0,0,"xMin = %15.6lf yMin = %15.6lf zMin = %15.6lf",test1P->xMin,test1P->yMin,test1P->zMin ) ;
        bcdtmWrite_message(0,0,0,"xMax = %15.6lf yMax = %15.6lf zMax = %15.6lf",test1P->xMax,test1P->yMax,test1P->zMax ) ;
        }
    /*
    ** Allocate Memory For Marking Random Points
    */
    if( markPtsP != NULL ) { free(markPtsP) ; markPtsP = NULL ; }
    numMarkPts = dtm2P->numPoints ;
    markPtsP   = ( unsigned char * ) malloc ( ( numMarkPts / 8 + 1 ) * sizeof(char)) ;
    if( markPtsP == NULL )
        {
        bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
        goto errexit ;
        }
    for( mpP = markPtsP ; mpP < markPtsP + numMarkPts / 8 + 1 ; ++mpP) *mpP = ( char ) 0 ;
    /*
    ** Unmark Feature Points
    */
    for( dtmFeature = 0 ; dtmFeature < dtm2P->numFeatures ; ++dtmFeature )
        {
        dtmFeatureP = ftableAddrP(dtm2P,dtmFeature) ;
        if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data )
            {
            for( point = dtmFeatureP->dtmFeaturePts.firstPoint ; point < dtmFeatureP->dtmFeaturePts.firstPoint + dtmFeatureP->numDtmFeaturePts ; ++point )
                {
                bcdtmFlag_setFlag(markPtsP,point) ;
                }
            }
        }
    /*
    ** Copy Random Points Bounding Cube From dtm2P To test2P
    */
    numPointsCopied = 0 ;
    for( point = 0 ; point < dtm2P->numPoints ; ++point )
        {
        pointP = pointAddrP(dtm2P,point) ;  
        if( ! bcdtmFlag_testFlag(markPtsP,point) && pointP->x >= Xmin && pointP->x <= Xmax && pointP->y >= Ymin && pointP->y <= Ymax  && pointP->z >= Zmin && pointP->z <= Zmax )
            {
            if( bcdtmObject_storeDtmFeatureInDtmObject(test2P,DTMFeatureType::RandomSpots,DTM_NULL_USER_TAG,1,&nullFeatureId,(DPoint3d *) pointP,1)) goto errexit ;
            ++numPointsCopied ;
            }
        }
    /*
    ** Copy DTM Features Within Bounding Cube
    */ 
    numFeaturesCopied = 0 ;
    for( dtmFeature = 0 ; dtmFeature < dtm2P->numFeatures ; ++dtmFeature )
        {
        dtmFeatureP = ftableAddrP(dtm2P,dtmFeature) ;
        if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data )
            {
            copyFeature = FALSE ;
            for( point = dtmFeatureP->dtmFeaturePts.firstPoint ; point < dtmFeatureP->dtmFeaturePts.firstPoint + dtmFeatureP->numDtmFeaturePts && copyFeature == FALSE ; ++point )
                {
                pointP = pointAddrP(dtm2P,point) ;  
                if( pointP->x >= Xmin && pointP->x <= Xmax && pointP->y >= Ymin && pointP->y <= Ymax  && pointP->z >= Zmin && pointP->z <= Zmax ) copyFeature = TRUE ;
                }
            if( copyFeature == TRUE )
                {
                if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtm2P,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
                if( bcdtmObject_storeDtmFeatureInDtmObject(test2P,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,2,&dtmFeatureP->dtmFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
                if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
                ++numFeaturesCopied ;
                }
            }
        }
    /*
    ** Write test2P Statistics ** Development Only
    */
    if( dbg ) 
        {
        bcdtmWrite_message(0,0,0,"Number Of Random   Copied From dtm2P to test2P = %8ld",numPointsCopied) ;
        bcdtmWrite_message(0,0,0,"Number Of Features Copied From dtm2P to test2P = %8ld",numFeaturesCopied) ;
        bcdtmWrite_message(0,0,0,"test2P->numPoints = %6ld",test2P->numPoints) ;
        bcdtmWrite_message(0,0,0,"xMin = %15.6lf yMin = %15.6lf zMin = %15.6lf",test2P->xMin,test2P->yMin,test2P->zMin ) ;
        bcdtmWrite_message(0,0,0,"xMax = %15.6lf yMax = %15.6lf zMax = %15.6lf",test2P->xMax,test2P->yMax,test2P->zMax ) ;
        }
    /*
    ** Check For Adjacency Of Points
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Point Adjacency") ;
    for( point1 = 0 ; point1 < test1P->numPoints && ! *adjacencyFlagP ; ++point1 )
        {
        point1P = pointAddrP(test1P,point1) ;
        for( point2 = 0 ; point2 < test2P->numPoints && ! *adjacencyFlagP ; ++point2 )
            {
            point2P = pointAddrP(test2P,point2) ;
            if( dbg )
                {
                if( fabs(point1P->x-point2P->x) <= xyTolerance && 
                    fabs(point1P->y-point2P->y) <= xyTolerance    )
                    {
                    bcdtmWrite_message(0,0,0,"Point 1 = %6ld  ** %10.4lf %10.4lf %10.4lf",point1,point1P->x,point1P->y,point1P->z) ;
                    bcdtmWrite_message(0,0,0,"Point 2 = %6ld  ** %10.4lf %10.4lf %10.4lf",point2,point2P->x,point2P->y,point2P->z) ;
                    } 
                }
            if( fabs(point1P->x-point2P->x) <= xyTolerance && 
                fabs(point1P->y-point2P->y) <= xyTolerance && 
                fabs(point1P->z-point2P->z) <= zTolerance      )
                {
                dx = point1P->x - point2P->x ;
                dy = point1P->y - point2P->y ;
                if( sqrt(dx*dx+dy*dy) <= xyTolerance )
                    {
                    *adjacencyFlagP = 1 ;
                    if( dbg ) bcdtmWrite_message(0,0,0,"Point To Point Adjacency [%10.4lf,%10.4lf,%10.4lf] [%10.4lf,%10.4lf,%10.4lf] ** DL = %10.4lf DZ = %10.4lf",point1P->x,point1P->y,point1P->z,point2P->x,point2P->y,point2P->z,sqrt(dx*dx+dy*dy),fabs(point1P->z-point2P->z)) ;
                    }
                } 
            }
        }
    /*
    ** Check For Adjacency Of test1P Points To test2P Lines
    */
    if( test1P->numPoints > 0 && test2P->numPoints > 1 && ! *adjacencyFlagP )
        { 
        if( dbg ) bcdtmWrite_message(0,0,0,"Checking test1P Points To test2P Lines Adjacency") ;
        for( point = 0 ; point < test1P->numPoints && ! *adjacencyFlagP ; ++point )
            {
            pointP = pointAddrP(test1P,point) ;
            for( dtmFeature = 0 ; dtmFeature < test2P->numFeatures ; ++dtmFeature )
                {
                dtmFeatureP = ftableAddrP(test2P,dtmFeature) ;
                if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data )
                    for( point1 = dtmFeatureP->dtmFeaturePts.firstPoint ; point1 < dtmFeatureP->dtmFeaturePts.firstPoint + dtmFeatureP->numDtmFeaturePts - 1 && ! *adjacencyFlagP ; ++point1 )
                        {
                        point1P = pointAddrP(test2P,point1) ;  
                        point2P = pointAddrP(test2P,point1+1) ; 
                        Xmin = Xmax = point1P->x ;
                        Ymin = Ymax = point1P->y ;
                        if( point2P->x < Xmin ) Xmin = point2P->x ;
                        if( point2P->x > Xmax ) Xmax = point2P->x ;
                        if( point2P->y < Ymin ) Ymin = point2P->y ;
                        if( point2P->y > Ymax ) Ymax = point2P->y ;
                        Xmin -= xyTolerance ;
                        Xmax += xyTolerance ;
                        Ymin -= xyTolerance ;
                        Ymax += xyTolerance ;
                        if( pointP->x >= Xmin && pointP->y <= Xmax && pointP->y >= Ymin && pointP->y <= Ymax )
                            {
                            nd = bcdtmMath_distanceOfPointFromLine(&onLine,point1P->x,point1P->y,point2P->x,point2P->y,pointP->x,pointP->y,&Xi,&Yi) ;
                            if( onLine && nd <= xyTolerance )
                                {
                                bcdtmMath_interpolatePointOnLine(point1P->x,point1P->y,point1P->z,point2P->x,point2P->y,point2P->z,Xi,Yi,&Zi) ;
                                if( fabs(pointP->z-Zi) <= zTolerance )
                                    {
                                    *adjacencyFlagP = 1 ;
                                    if( dbg ) bcdtmWrite_message(0,0,0,"Point To Line  Adjacency [%10.4lf,%10.4lf,%10.4lf] [%10.4lf,%10.4lf,%10.4lf] ** DL = %10.4lf DZ = %10.4lf",pointP->x,pointP->y,Xi,Yi,Zi,nd,fabs(pointP->z-Zi)) ;
                                    }
                                }
                            } 
                        } 
                }
            }
        }
    /*
    ** Check For Adjacency Of test2P Points To test1P Lines
    */
    if( test2P->numPoints > 0 && test1P->numPoints > 1 && ! *adjacencyFlagP )
        { 
        if( dbg ) bcdtmWrite_message(0,0,0,"Checking test2P Points To test1P Lines Adjacency") ;
        if( dbg ) bcdtmWrite_message(0,0,0,"Checking test1P Points To test2P Lines Adjacency") ;
        for( point = 0 ; point < test2P->numPoints && ! *adjacencyFlagP ; ++point )
            {
            pointP = pointAddrP(test2P,point) ;
            for( dtmFeature = 0 ; dtmFeature < test1P->numFeatures ; ++dtmFeature )
                {
                dtmFeatureP = ftableAddrP(test2P,dtmFeature) ;
                if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data )
                    for( point1 = dtmFeatureP->dtmFeaturePts.firstPoint ; point1 < dtmFeatureP->dtmFeaturePts.firstPoint + dtmFeatureP->numDtmFeaturePts - 1 && ! *adjacencyFlagP ; ++point1 )
                        {
                        point1P = pointAddrP(test1P,point1) ;  
                        point2P = pointAddrP(test1P,point1+1) ; 
                        Xmin = Xmax = point1P->x ;
                        Ymin = Ymax = point1P->y ;
                        if( point2P->x < Xmin ) Xmin = point2P->x ;
                        if( point2P->x > Xmax ) Xmax = point2P->x ;
                        if( point2P->y < Ymin ) Ymin = point2P->y ;
                        if( point2P->y > Ymax ) Ymax = point2P->y ;
                        Xmin -= xyTolerance ;
                        Xmax += xyTolerance ;
                        Ymin -= xyTolerance ;
                        Ymax += xyTolerance ;
                        if( pointP->x >= Xmin && pointP->y <= Xmax && pointP->y >= Ymin && pointP->y <= Ymax )
                            {
                            nd = bcdtmMath_distanceOfPointFromLine(&onLine,point1P->x,point1P->y,point2P->x,point2P->y,pointP->x,pointP->y,&Xi,&Yi) ;
                            if( onLine && nd <= xyTolerance )
                                {
                                bcdtmMath_interpolatePointOnLine(point1P->x,point1P->y,point1P->z,point2P->x,point2P->y,point2P->z,Xi,Yi,&Zi) ;
                                if( fabs(pointP->z-Zi) <= zTolerance )
                                    {
                                    *adjacencyFlagP = 1 ;
                                    if( dbg ) bcdtmWrite_message(0,0,0,"Point To Line  Adjacency [%10.4lf,%10.4lf,%10.4lf] [%10.4lf,%10.4lf,%10.4lf] ** DL = %10.4lf DZ = %10.4lf",pointP->x,pointP->y,Xi,Yi,Zi,nd,fabs(pointP->z-Zi)) ;
                                    }
                                }
                            } 
                        } 
                }
            }
        }
    /*
    ** Write Adjacency
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Adjacency Flag = %1ld",*adjacencyFlagP) ;
    /*
    ** Free Memory
    */
cleanup :
    if( test1P != NULL ) bcdtmObject_destroyDtmObject(&test1P) ;
    if( test2P != NULL ) bcdtmObject_destroyDtmObject(&test2P) ;
    if( markPtsP    != NULL ) { free(markPtsP)    ; markPtsP    = NULL ; }
    if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
    /*
    ** Job Completed
    */
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Testing For Adjacency Of Dtm Objects Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Testing For Adjacency Of Dtm Objects Error") ;
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    *adjacencyFlagP = 0 ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmData_copyDtmFeatureTypeDtmObject
    (
    BC_DTM_OBJ *fromDtmP,               // DTM Pointer To Copy From
    BC_DTM_OBJ *toDtmP,                 // DTM Pointer To Copy To 
    DTMFeatureType dtmFeatureType,                // Dtm Feature Type To Copy
    DTMFeatureType copyDtmFeatureType,            // Dtm Feature Type To Be Assigned To Copied Feature
    long copyOption,                    // Copy Option < 0 No Match > < 1 Match UserTag > < 2 Match FeatureId > < 3 Match Usertag And Feature Id>
    DTMUserTag  dtmUserTag,           // User Tag To Match
    DTMFeatureId dtmFeatureId         // Feature Id to Match
    )
    {
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
    long dtmFeature,copyFeature,numFeaturePts ;
    BC_DTM_FEATURE *dtmFeatureP ;
    DTMFeatureId copyDtmFeatureId ;
    DPoint3d *featurePtsP=NULL ;
    /*
    ** Write Entry Message
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Copying Dtm Feature Type") ;
        bcdtmWrite_message(0,0,0,"fromDtmP           = %p",fromDtmP) ;
        bcdtmWrite_message(0,0,0,"toDtmP             = %p",toDtmP) ;
        bcdtmWrite_message(0,0,0,"dtmFeatureType     = %8ld",dtmFeatureType) ;
        bcdtmWrite_message(0,0,0,"copyDtmFeatureType = %8ld",copyDtmFeatureType) ;
        bcdtmWrite_message(0,0,0,"dtmUserTag         = %8I64d",dtmUserTag) ;
        bcdtmWrite_message(0,0,0,"dtmFeatureId       = %8ld",dtmFeatureId) ;
        }
    /*
    ** Check For Valid DTM
    */
    if( bcdtmObject_testForValidDtmObject(fromDtmP)) goto errexit ;
    if( bcdtmObject_testForValidDtmObject(toDtmP)) goto errexit ;
    /*
    ** Check For Valid Copy Option
    */
    if( copyOption < 0 || copyOption > 3 )
        {
        bcdtmWrite_message(1,0,0,"Invalid Copy Option") ;
        goto errexit ;
        }
    /*
    ** Check For Valid DTM Feature Types
    */
    if( bcdtmData_testForValidDtmObjectExportFeatureType(dtmFeatureType)) goto errexit ;
    if( bcdtmData_testForValidDtmObjectImportFeatureType(copyDtmFeatureType)) goto errexit ;
    /*
    ** Scan DTM Features
    */
    for( dtmFeature = 0 ; dtmFeature < fromDtmP->numFeatures ; ++dtmFeature )
        {
        dtmFeatureP = ftableAddrP(fromDtmP,dtmFeature) ;
        if( dtmFeatureP->dtmFeatureType == dtmFeatureType && ( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data || dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin ))
            {
            copyFeature = TRUE ;
            if( copyOption )
                {
                switch( copyOption )
                    {
                    case 1 :    // Match On User Tag
                        if( dtmFeatureP->dtmUserTag != dtmUserTag ) copyFeature = FALSE ;
                        break  ;

                    case 2 :    // Match On Feature Id   
                        if( dtmFeatureP->dtmFeatureId != dtmFeatureId ) copyFeature = FALSE ;
                        break  ;

                    case 3 :    // Match On User Tag And Feature Id   
                        if( dtmFeatureP->dtmUserTag != dtmUserTag || dtmFeatureP->dtmFeatureId != dtmFeatureId ) copyFeature = FALSE ;
                        break  ;
                    }
                } 
            /*
            **     Get And Copy Feature Points
            */
            if( copyFeature == TRUE )
                {
                if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(fromDtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
                if( bcdtmObject_storeDtmFeatureInDtmObject(toDtmP,copyDtmFeatureType,dtmFeatureP->dtmUserTag,3,&copyDtmFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
                if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
                }    
            }
        }
    /*
    ** Free Memory
    */
cleanup :
    if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
    /*
    ** Job Completed
    */
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying Dtm Feature Type Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying Dtm Feature Type Error") ;
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmData_copyInitialDtmFeaturePointsToPointArrayDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,DPoint3d **featPtsPP,long *numFeatPtsP) 
    /*
    ** This Function Copies Feature Points To A Point Array
    */
    {
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;

    /*
    ** Write Entry Message
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Dtm Feature Points To A Point Array") ; 
    /*
    ** Initialise
    */
    *numFeatPtsP = 0 ;
    if( *featPtsPP != NULL ) { free(*featPtsPP) ; *featPtsPP = NULL ; }
    /*
    ** Test For Valid DTM Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Get Points For Dtm Feature
    */
    if( bcdtmData_getInitialPointsForDtmFeatureDtmObject(dtmP,dtmFeature,(DTM_TIN_POINT **)featPtsPP,numFeatPtsP)) goto errexit ; 
    /*
    ** Clean Up
    */
cleanup :
    /*
    ** Job Completed
    */
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying Dtm Feature Points To A Point Array Completed") ; 
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying Dtm Feature Points To A Point Array Error") ; 
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    } 
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmData_getInitialPointsForDtmFeatureDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,DTM_TIN_POINT **featPtsPP,long *numFeatPtsP) 
    /*
    ** This Function Writes Points For A Dtm Feature 
    */
    {
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
    long n,listPtr,nextPnt=0,firstPnt,point ;
    BC_DTM_FEATURE *dtmFeatureP ;
    DTM_TIN_POINT  *pntP,*pointP ;
    /*
    ** Write Entry Message
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Getting Initial Points For Dtm Feature = %8ld",dtmFeature) ;
    /*
    ** Initialise
    */
    *numFeatPtsP = 0 ;
    if( *featPtsPP != NULL ) { free(*featPtsPP) ; *featPtsPP = NULL ; }
    /*
    ** Validate
    */
    if( dtmFeature < 0 || dtmFeature >= dtmP->numFeatures )
        {
        bcdtmWrite_message(2,0,0,"Dtm Feature Range Error") ;
        goto errexit ;
        }
    /*
    ** Set Feature Address
    */
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted )
        {
        /*
        **  Count Number Of Feature Points For Feature In Tin State 
        */
        if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
            {
            bcdtmList_countNumberOfPointsForDtmTinFeatureDtmObject(dtmP,dtmFeature,numFeatPtsP) ;
            }
        else  *numFeatPtsP = dtmFeatureP->numDtmFeaturePts ;
        if( dbg ) bcdtmWrite_message(0,0,0,"*numFeatPtsP = %8ld",*numFeatPtsP) ;
        /*
        **  Allocate memory To Store Feature Points
        */
        *featPtsPP = ( DTM_TIN_POINT * ) malloc( *numFeatPtsP * sizeof( DTM_TIN_POINT )) ;
        if( *featPtsPP == NULL )
            {
            bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
            goto errexit ;
            }
        /*
        **  Method To Write Points Is Dependent On The Dtm Feature State
        */
        switch( dtmFeatureP->dtmFeatureState )
            {
            /*
            **     Get Points From DTM Points Array
            */ 
            case DTMFeatureState::Data : 
                for( n = 0 , pntP = *featPtsPP ; n < dtmFeatureP->numDtmFeaturePts ; ++n , ++pntP )
                    {
                    point  = dtmFeatureP->dtmFeaturePts.firstPoint+n ;
                    pointP = pointAddrP(dtmP,point) ;
                    *pntP  = *pointP ;
                    } 
                break ;
                /*
                **     Get Points From Feature Points Array
                */ 
            case DTMFeatureState::PointsArray : 
            case DTMFeatureState::TinError    : 
            case DTMFeatureState::Rollback     : 
                memcpy(*featPtsPP,bcdtmMemory_getPointerP3D(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI),dtmFeatureP->numDtmFeaturePts*sizeof(DTM_TIN_POINT)) ;
                break ;
                /*
                **     Get Points From Point Offset Array
                */ 
            case DTMFeatureState::OffsetsArray : 
                for( n = 0 , pntP = *featPtsPP ; n < dtmFeatureP->numDtmFeaturePts ; ++n , ++pntP )
                    {
                    point  = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI)[n] ;
                    pointP = pointAddrP(dtmP,point) ;
                    *pntP  = *pointP ;
                    } 
                break ;
                /*
                **     Get Points From Tin
                */ 
            case DTMFeatureState::Tin  :       // Dtm Feature In Tin 
                if( ( firstPnt = ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ) != dtmP->nullPnt )
                    { 
                    pntP = *featPtsPP ;
                    nextPnt = firstPnt ;
                    listPtr  = nodeAddrP(dtmP,nextPnt)->fPtr ;
                    while ( listPtr != dtmP->nullPtr )
                        {
                        while ( listPtr != dtmP->nullPtr && flistAddrP(dtmP,listPtr)->dtmFeature != dtmFeature ) listPtr = flistAddrP(dtmP,listPtr)->nextPtr ;
                        if( listPtr != dtmP->nullPtr )
                            {
                            if( flistAddrP(dtmP,listPtr)->pntType != 2 )
                                {
                                pointP = pointAddrP(dtmP,nextPnt) ;
                                *pntP  = *pointP ;
                                ++pntP ;
                                }
                            nextPnt = flistAddrP(dtmP,listPtr)->nextPnt ;
                            if( nextPnt == dtmP->nullPnt || nextPnt == firstPnt ) listPtr = dtmP->nullPtr ;
                            else                                                  listPtr  = nodeAddrP(dtmP,nextPnt)->fPtr ;
                            }
                        }
                    if( nextPnt == firstPnt && pntP != *featPtsPP )
                        {
                        *pntP = *(*featPtsPP) ;
                        ++pntP ;
                        }
                    *numFeatPtsP = ( long) ( pntP-*featPtsPP) ;
                    }
                break ;
                /*
                **     Default 
                */
            default :
                bcdtmWrite_message(2,0,0,"Unknown Dtm Feature State %2ld Not Yet Implemented",ftableAddrP(dtmP,dtmFeature)->dtmFeatureState) ;
                goto errexit ; 
                break ;
            } ;
        }
    /*
    ** Clean Up
    */
cleanup :
    /* 
    ** Job Completed
    */
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmData_replaceDtmFeaturePointsDtmObject
    (
    BC_DTM_OBJ     *dtmP,
    DTMFeatureId featureId,
    DPoint3d            *featurePtsP,
    long           numFeaturePts 
    )
    /*
    ** Replace DTM Feature Points 
    */
    {
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
    long   dtmFeature,featureDeleted=FALSE ;
    DTMFeatureType featureType = DTMFeatureType::None, rollBackFeatureType;
    char   dtmFeatureStateName[100],dtmFeatureTypeName[100] ;
    DTMUserTag    featureTag = 0;
    BC_DTM_FEATURE  *dtmFeatureP ;
    /*
    ** Write Entry Message
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Replacing DTM Feature Points") ;
        bcdtmWrite_message(0,0,0,"dtmP          = %p",dtmP) ;
        bcdtmWrite_message(0,0,0,"featureId     = %8I64d",featureId) ;
        bcdtmWrite_message(0,0,0,"featurePtsP   = %p",featurePtsP) ;
        bcdtmWrite_message(0,0,0,"numFeaturePts = %8ld",numFeaturePts) ;
        } 
    /*
    ** Log Current DTM Statistics
    */
    if( dbg == 2 )
        {
        bcdtmWrite_message(0,0,0,"DTM Before Replacement") ;
        bcdtmObject_reportStatisticsDtmObject(dtmP) ;
        }   
    /*
    ** Report Current Features
    */
    if( dbg == 1 )
        {
        bcdtmWrite_message(0,0,0,"Number Of Dtm Features Before Replacement = %8ld",dtmP->numFeatures) ;
        for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
            bcdtmData_getDtmFeatureStateNameFromDtmFeatureState(dtmFeatureP->dtmFeatureState,dtmFeatureStateName) ;
            bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName) ;
            bcdtmWrite_message(0,0,0,"dtmFeature[%8ld] State = %-20s Type = %-30s Id = %10I64d",dtmFeature,dtmFeatureStateName,dtmFeatureTypeName,dtmFeatureP->dtmFeatureId) ;
            } 
        }   
    /*
    ** Check For Valid Dtm Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Check For Points
    */
    if( featurePtsP == NULL || numFeaturePts <= 0 )
        {
        bcdtmWrite_message(1,0,0,"No Replacement Feature Points") ;
        goto errexit ;
        } 
    /*
    ** Scan For Feature Id
    */
    rollBackFeatureType = DTMFeatureType::None;
    if( dtmP->numFeatures > 0 ) 
        { 
        for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
            if( dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted && dtmFeatureP->dtmFeatureId == featureId )
                {
                if( featureDeleted == FALSE )
                    {
                    featureDeleted = TRUE ;
                    featureType = dtmFeatureP->dtmFeatureType ;
                    featureTag  = dtmFeatureP->dtmUserTag ;
                    }
                if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Rollback )
                    {
                    rollBackFeatureType = dtmFeatureP->dtmFeatureType ;
                    featureTag  = dtmFeatureP->dtmUserTag ;
                    }
                if( bcdtmInsert_removeDtmFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ;
                }
            }
        }
    /*
    **  Check Feature Was Deleted
    */
    if( featureDeleted == FALSE )
        {
        bcdtmWrite_message(1,0,0,"Feature Not Found") ;
        goto errexit ;
        } 
    /*
    ** Add Replacement Feature Points
    */
    if( rollBackFeatureType != DTMFeatureType::None ) featureType = rollBackFeatureType ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,featureType,featureTag,2,&featureId,featurePtsP,numFeaturePts)) goto errexit ;     
    /*
    ** Log Current DTM Statistics
    */
    if( dbg == 2 )
        {
        bcdtmWrite_message(0,0,0,"DTM After Replacement") ;
        bcdtmObject_reportStatisticsDtmObject(dtmP) ;
        }   
    /*
    ** Report Current Features
    */
    if( dbg == 1 )
        {
        bcdtmWrite_message(0,0,0,"Number Of Dtm Features After Replacement = %8ld",dtmP->numFeatures) ;
        for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
            bcdtmData_getDtmFeatureStateNameFromDtmFeatureState(dtmFeatureP->dtmFeatureState,dtmFeatureStateName) ;
            bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName) ;
            bcdtmWrite_message(0,0,0,"dtmFeature[%8ld] State = %-20s Type = %-30s Id = %10I64d",dtmFeature,dtmFeatureStateName,dtmFeatureTypeName,dtmFeatureP->dtmFeatureId) ;
            } 
        }   
    /*
    **  Update Modified Time
    */
    bcdtmObject_updateLastModifiedTime(dtmP) ;
    /*
    ** Clean Up
    */
cleanup :
    /*
    ** Job Completed
    */
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Replacing DTM Feature Points Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Replacing DTM Feature Points Error") ;
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmData_replaceDtmFeaturePointsMultipleDtmObject
    (
    BC_DTM_OBJ     *dtmP,
    DTMFeatureId featureId,
    const Bentley::TerrainModel::DtmVectorString& features
    )
    /*
    ** Replace DTM Feature Points
    */
    {
    int    ret = DTM_SUCCESS, dbg = DTM_TRACE_VALUE (0);
    long   dtmFeature,featureDeleted=false ;
    DTMFeatureType featureType, rollBackFeatureType;
    char   dtmFeatureStateName[100],dtmFeatureTypeName[100] ;
    DTMUserTag    featureTag ;
    BC_DTM_FEATURE  *dtmFeatureP ;
    /*
    ** Write Entry Message
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Replacing DTM Feature Points") ;
        bcdtmWrite_message(0,0,0,"dtmP          = %p",dtmP) ;
        bcdtmWrite_message(0,0,0,"featureId     = %8I64d",featureId) ;
        }
    /*
    ** Log Current DTM Statistics
    */
    if( dbg == 2 )
        {
        bcdtmWrite_message(0,0,0,"DTM Before Replacement") ;
        bcdtmObject_reportStatisticsDtmObject(dtmP) ;
        }
    /*
    ** Report Current Features
    */
    if( dbg == 1 )
        {
        bcdtmWrite_message(0,0,0,"Number Of Dtm Features Before Replacement = %8ld",dtmP->numFeatures) ;
        for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
            bcdtmData_getDtmFeatureStateNameFromDtmFeatureState(dtmFeatureP->dtmFeatureState,dtmFeatureStateName) ;
            bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName) ;
            bcdtmWrite_message(0,0,0,"dtmFeature[%8ld] State = %-20s Type = %-30s Id = %10I64d",dtmFeature,dtmFeatureStateName,dtmFeatureTypeName,dtmFeatureP->dtmFeatureId) ;
            }
        }
    /*
    ** Check For Valid Dtm Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Check For Points
    */
    if( features.size() == 0 )
        {
        bcdtmWrite_message(1,0,0,"No Replacement Feature Points") ;
        goto errexit ;
        }
    /*
    ** Scan For Feature Id
    */
    rollBackFeatureType = DTMFeatureType::None;
    if( dtmP->numFeatures > 0 )
        {
        for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
            if( dtmFeatureP->dtmFeatureState != DTMFeatureState::Deleted && dtmFeatureP->dtmFeatureId == featureId )
                {
                if( featureDeleted == false )
                    {
                    featureDeleted = true ;
                    featureType = dtmFeatureP->dtmFeatureType ;
                    featureTag  = dtmFeatureP->dtmUserTag ;
                    }
                if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Rollback )
                    {
                    rollBackFeatureType = dtmFeatureP->dtmFeatureType ;
                    featureTag  = dtmFeatureP->dtmUserTag ;
                    }
                if( bcdtmInsert_removeDtmFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ;
                }
            }
        }
    /*
    **  Check Feature Was Deleted
    */
    if( featureDeleted == false )
        {
        bcdtmWrite_message(1,0,0,"Feature Not Found") ;
        goto errexit ;
        }
    /*
    ** Add Replacement Feature Points
    */
    if( rollBackFeatureType != DTMFeatureType::None ) featureType = rollBackFeatureType ;

    for (size_t i = 0; i < features.size(); i++)
        {
        if ( bcdtmObject_storeDtmFeatureInDtmObject (dtmP, featureType, featureTag, 2, &featureId, (DPoint3d*)features[i].data(), (int)features[i].size())) goto errexit;
        }
    /*
    ** Log Current DTM Statistics
    */
    if( dbg == 2 )
        {
        bcdtmWrite_message(0,0,0,"DTM After Replacement") ;
        bcdtmObject_reportStatisticsDtmObject(dtmP) ;
        }
    /*
    ** Report Current Features
    */
    if( dbg == 1 )
        {
        bcdtmWrite_message(0,0,0,"Number Of Dtm Features After Replacement = %8ld",dtmP->numFeatures) ;
        for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
            bcdtmData_getDtmFeatureStateNameFromDtmFeatureState(dtmFeatureP->dtmFeatureState,dtmFeatureStateName) ;
            bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName) ;
            bcdtmWrite_message(0,0,0,"dtmFeature[%8ld] State = %-20s Type = %-30s Id = %10I64d",dtmFeature,dtmFeatureStateName,dtmFeatureTypeName,dtmFeatureP->dtmFeatureId) ;
            }
        }
    /*
    **  Update Modified Time
    */
    bcdtmObject_updateLastModifiedTime(dtmP) ;
    /*
    ** Clean Up
    */
cleanup :
    /*
    ** Job Completed
    */
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Replacing DTM Feature Points Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Replacing DTM Feature Points Error") ;
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmData_removeExternalSliverTrianglesDtmObject
    (
    BC_DTM_OBJ *dtmP 
    ) 
    {
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
    long dtmFeature,hullFeature=FALSE ;
    BC_DTM_FEATURE *dtmFeatureP ;
    /*
    ** Write Entry Message
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Removing External Sliver Triangles") ;
        bcdtmWrite_message(0,0,0,"dtmP    = %p",dtmP) ;
        } 
    /*
    ** Check For Valid Dtm Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Check If DTM Is In Tin State
    */
    if( dtmP->dtmState != DTMState::Tin )
        {
        bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
        goto errexit ;
        }
    /*
    ** Check For Hull Feature
    */  
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures && hullFeature == FALSE ; ++dtmFeature )
        {
        dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
        if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeatureType == DTMFeatureType::Hull ) hullFeature = TRUE ;
        }
    /*
    ** Only Remove External Sliver Triangles If There Is No Hull Feature
    */
    if( hullFeature == FALSE )
        {
        if( bcdtmTin_removeExternalSliverTrianglesDtmObject(dtmP)) goto errexit ;
        if( bcdtmTin_compactCircularListDtmObject(dtmP)) goto errexit ;
        }    
    /*
    ** Clean Up
    */
cleanup :
    /*
    ** Job Completed
    */
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing External Sliver Triangles Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing External Sliver Triangles Error") ;
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmData_removeExternalMaxSideTrianglesDtmObject
    (
    BC_DTM_OBJ *dtmP ,
    double      maxSide 
    ) 
    {
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
    long dtmFeature,hullFeature=FALSE ;
    BC_DTM_FEATURE *dtmFeatureP ;
    /*
    ** Write Entry Message
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Removing External Max Side Triangles") ;
        bcdtmWrite_message(0,0,0,"dtmP     = %p",dtmP) ;
        bcdtmWrite_message(0,0,0,"maxSide  = %8.3lf",maxSide) ;
        } 
    /*
    ** Check For Valid Dtm Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Check If DTM Is In Tin State
    */
    if( dtmP->dtmState != DTMState::Tin )
        {
        bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
        goto errexit ;
        }
    /*
    ** Validate Max Side Value
    */
    if( maxSide <= 0.0 )
        {
        bcdtmWrite_message(0,0,0,"Invalid Max Side Value") ;
        goto errexit ;
        }   
    /*
    ** Check For Hull Feature
    */  
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures && hullFeature == FALSE ; ++dtmFeature )
        {
        dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
        if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeatureType == DTMFeatureType::Hull ) hullFeature = TRUE ;
        }
    /*
    ** Only Remove External Max Side Triangles If There Is No Hull Feature
    */
    if( hullFeature == FALSE )
        {
        if( bcdtmTin_removeExternalMaxSideTrianglesDtmObject(dtmP,maxSide)) goto errexit ;
        if( bcdtmTin_compactCircularListDtmObject(dtmP)) goto errexit ;
        }    
    /*
    ** Clean Up
    */
cleanup :
    /*
    ** Job Completed
    */
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing External Max Side Triangles Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing External Max Side Triangles Error") ;
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmData_getDtmFeatureStateNameFromDtmFeatureState(DTMFeatureState dtmFeatureState,char *dtmFeatureStateName)
    {
    int  ret=DTM_ERROR ;
    /*
    ** Initialise
    */
    *dtmFeatureStateName = 0 ;
    /*
    ** Assign Name From Type
    */
    if( dtmFeatureState == DTMFeatureState::Unused        ) strcpy(dtmFeatureStateName,"DTMFeatureState::Unused") ;
    if( dtmFeatureState == DTMFeatureState::Data          ) strcpy(dtmFeatureStateName,"DTMFeatureState::Data") ;
    if( dtmFeatureState == DTMFeatureState::PointsArray  ) strcpy(dtmFeatureStateName,"DTMFeatureState::PointsArray") ;
    if( dtmFeatureState == DTMFeatureState::OffsetsArray ) strcpy(dtmFeatureStateName,"DTMFeatureState::OffsetsArray") ;
    if( dtmFeatureState == DTMFeatureState::Tin           ) strcpy(dtmFeatureStateName,"DTMFeatureState::Tin") ;
    if( dtmFeatureState == DTMFeatureState::TinError     ) strcpy(dtmFeatureStateName,"DTMFeatureState::TinError") ;
    if( dtmFeatureState == DTMFeatureState::Deleted       ) strcpy(dtmFeatureStateName,"DTMFeatureState::Deleted") ; 
    if( dtmFeatureState == DTMFeatureState::Rollback      ) strcpy(dtmFeatureStateName,"DTMFeatureState::Rollback") ;
    /*
    ** Set Return Value
    */
    if( *dtmFeatureStateName != 0 ) ret = DTM_SUCCESS ; 
    /*
    ** Job Completed
    */
    return(ret) ;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmData_moveTaggedDtmFeatureTypeOccurrencesToDtmObject
    (
    BC_DTM_OBJ   *dtm1P,
    BC_DTM_OBJ   *dtm2P,
    DTMFeatureType dtmFeatureType,
    DTMUserTag userTag
    ) 
    /*
    ** This Function Moves Occurrences Of A DTM Feature Type From One DTM Object To Another DTM Object
    ** An Occurrence Of A Dtm Feature Type Is Only Moved If Its User Tag Matches The User Tag Value
    ** All Occurences Are Moved If UserTagValue Is Set To -DTM_NULL_USER_TAG
    **
    */
    {
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
    long dtmFeature,numFeaturePts ;
    DPoint3d  *featurePtsP=NULL ;
    BC_DTM_FEATURE  *dtmFeatureP ;
    /*
    ** Write Status Message
    */
    if( dbg ) 
        {
        bcdtmWrite_message(0,0,0,"Moving Tagged DTM Feature Types") ;
        bcdtmWrite_message(0,0,0,"dtm1P          = %p",dtm1P) ;
        bcdtmWrite_message(0,0,0,"dtm2P          = %p",dtm2P) ;
        bcdtmWrite_message(0,0,0,"dtmFeatureType = %8ld",dtmFeatureType) ;
        bcdtmWrite_message(0,0,0,"userTag        = %8I64d",userTag) ;
        } 
    /*
    ** Test For Valid DTM Objects
    */
    if( bcdtmObject_testForValidDtmObject(dtm1P)) goto errexit ;
    if( bcdtmObject_testForValidDtmObject(dtm2P)) goto errexit ;
    /*
    ** Scan Features And Move
    */
    for( dtmFeature = 0 ; dtmFeature < dtm1P->numFeatures ; ++dtmFeature )
        {
        dtmFeatureP = ftableAddrP(dtm1P,dtmFeature) ;
        if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data || dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
            {
            if( dtmFeatureP->dtmFeatureType == dtmFeatureType && ( userTag == -dtm1P->nullUserTag || userTag == dtmFeatureP->dtmUserTag ))
                {
                if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtm1P,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
                if( bcdtmObject_storeDtmFeatureInDtmObject(dtm2P,dtmFeatureType,dtmFeatureP->dtmUserTag,1,&dtmFeatureP->dtmFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
                if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
                if( bcdtmInsert_removeDtmFeatureFromDtmObject(dtm1P,dtmFeature)) goto errexit ;
                } 
            }
        } 
    /*
    ** Clean Up
    */
cleanup :
    /*
    ** Job Completed
    */
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmData_copyTaggedDtmFeatureTypeOccurrencesToDtmObject
    (
    BC_DTM_OBJ   *dtm1P,
    BC_DTM_OBJ   *dtm2P,
    DTMFeatureType dtmFeatureType,
    DTMUserTag userTag
    ) 
    /*
    ** This Function Copies Occurrences Of A DTM Feature Type From One DTM Object To Another Dtm Object
    ** An Occurrence Of A Dtm Feature Type Is Only Copied If Its User Tag Matches The userTag
    ** All Occurences Are Copied If userTagValue Is Set To -DTM_NULL_USER_TAG
    */
    {
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
    long dtmFeature,numFeaturePts ;
    DPoint3d  *featurePtsP=NULL ;
    BC_DTM_FEATURE  *dtmFeatureP ;
    /*
    ** Write Status Message
    */
    if( dbg ) 
        {
        bcdtmWrite_message(0,0,0,"Copying Tagged DTM Feature Types") ;
        bcdtmWrite_message(0,0,0,"dtm1P          = %p",dtm1P) ;
        bcdtmWrite_message(0,0,0,"dtm2P          = %p",dtm2P) ;
        bcdtmWrite_message(0,0,0,"dtmFeatureType = %8ld",dtmFeatureType) ;
        bcdtmWrite_message(0,0,0,"userTag        = %8I64d",userTag) ;
        } 
    /*
    ** Test For Valid DTM Objects
    */
    if( bcdtmObject_testForValidDtmObject(dtm1P)) goto errexit ;
    if( bcdtmObject_testForValidDtmObject(dtm2P)) goto errexit ;
    /*
    ** Scan Features And Copy
    */
    for( dtmFeature = 0 ; dtmFeature < dtm1P->numFeatures ; ++dtmFeature )
        {
        dtmFeatureP = ftableAddrP(dtm1P,dtmFeature) ;
        if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data || dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
            {
            if( dtmFeatureP->dtmFeatureType == dtmFeatureType && ( userTag == -dtm1P->nullUserTag || userTag == dtmFeatureP->dtmUserTag ))
                {
                if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtm1P,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
                if( bcdtmObject_storeDtmFeatureInDtmObject(dtm2P,dtmFeatureType,dtmFeatureP->dtmUserTag,1,&dtmFeatureP->dtmFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
                if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
                } 
            }
        } 
    /*
    ** Clean Up
    */
cleanup :
    if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
    /*
    ** Job Completed
    */
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmData_moveToP3DArrayDtmFeatureTypeDtmObject
    (
    BC_DTM_OBJ     *dtmP,
    DTMFeatureType dtmFeatureType,
    DTMUserTag   *userTagP,
    DTMFeatureId *featureIdP,
    DPoint3d            **featurePtsPP,
    long           *numFeaturePtsP
    )
    /*
    ** This Function Moves A DTM Feature Type From A DTM Object To A Point Array
    */
    {
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
    long scan,dtmFeature ;
    BC_DTM_FEATURE  *dtmFeatureP ;
    /*
    ** Write Status Message
    */
    if( dbg ) 
        {
        bcdtmWrite_message(0,0,0,"Moving To DPoint3d Array Dtm Feature Type") ;
        bcdtmWrite_message(0,0,0,"dtmP           = %p",dtmP) ;
        bcdtmWrite_message(0,0,0,"dtmFeatureType = %8ld",dtmFeatureType) ;
        } 
    /*
    ** Initialise
    */
    *userTagP       = DTM_NULL_USER_TAG ;
    *featureIdP     = DTM_NULL_FEATURE_ID ;
    *numFeaturePtsP = 0 ;
    if( *featurePtsPP != NULL ) { free(*featurePtsPP) ; *featurePtsPP = NULL ; }
    /*
    ** Test For Valid Dtm Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP) )  goto errexit ; 
    /*
    ** Scan Features And Copy
    */
    scan = TRUE ;
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures && scan == TRUE ; ++dtmFeature )
        {
        dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
        if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data || dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
            {
            if( dtmFeatureP->dtmFeatureType == dtmFeatureType )
                {
                if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,featurePtsPP,numFeaturePtsP)) goto errexit ;
                *userTagP    = dtmFeatureP->dtmUserTag ;
                *featureIdP  = dtmFeatureP->dtmFeatureId ;
                if( bcdtmInsert_removeDtmFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ;
                scan = FALSE ;
                } 
            }
        } 
    /*
    ** Clean Up
    */
cleanup :
    /*
    ** Job Completed
    */
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    ret = DTM_ERROR ;
    if( *featurePtsPP != NULL ) { free(*featurePtsPP) ; *featurePtsPP = NULL ; }
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmData_validatePolygonalDtmFeaturesDtmObject
    (
    BC_DTM_OBJ *dtmP
    )
    /*
    ** This Function Validates Polygonal DTM Features
    */
    {
 long  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ; 
    long  numErrors ;
    /*
    ** Write Entry Message
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Validating Polygonal Dtm Features") ;
    /*
    ** Check For Valid DTM Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Check For Untriangulated DTM
    */
    if( dtmP->dtmState != DTMState::Data )
        {
        bcdtmWrite_message(1,0,0,"Method Requires Untriangulated DTM") ;
        goto errexit ;
        }
    /*
    ** Intersect DTM Features In Dtm Object
    */
    /*
    Robc - Not Necessary For Validation Purpose - Leave Port Of bcdtmData_intersectDtmFeaturesDtmObject Till Later

    if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting DTM Features") ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Points Before Intersecting Features = %7ld",Data->numPts) ;   
    if( bcdtmData_intersectDtmFeaturesDtmObject(dtmP,1000.0)) goto errexit  ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Points After  Intersecting Features = %7ld",Data->numPts) ;   
    if( dbg ) bcdtmWrite_message(0,0,0,"DTM Features Intersected") ;
    */
    /*
    ** Validate Voids
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Validating Voids") ;
    if( bcdtmClean_validateDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Void,dtmP->ppTol,&numErrors)) goto errexit  ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Voids Validated") ;
    /*
    ** Validate Voids
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Validating Holes") ;
    if( bcdtmClean_validateDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Hole,dtmP->ppTol,&numErrors)) goto errexit  ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Holes Validated") ;
    /*
    ** Validate Break Voids
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Validating Break Voids") ;
    if( bcdtmClean_validateDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::BreakVoid,dtmP->ppTol,&numErrors)) goto errexit  ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Break Voids Validated") ;
    /*
    ** Validate Islands
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Validating Islands") ;
    if( bcdtmClean_validateDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Island,dtmP->ppTol,&numErrors)) goto errexit  ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Islands Validated") ;
    /*
    ** Validate Drape Voids
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Validating Drape Voids") ;
    if( bcdtmClean_validateDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::DrapeVoid,dtmP->ppTol,&numErrors)) goto errexit  ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Drape Voids Validated") ;
    /*
    ** Validate Regions
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Validating Regions") ;
    if( bcdtmClean_validateDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Region,dtmP->ppTol,&numErrors)) goto errexit  ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Regions Validated") ;
    /*
    ** Validate Boundary Polygon
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Validating Boundary Polygon") ;
    if( bcdtmClean_validateDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Hull,dtmP->ppTol,&numErrors)) goto errexit  ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Boundary Polygon Validated") ;
    /*
    ** Validate Drape Boundary Polygon
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Validating Drape Boundary Polygon") ;
    if( bcdtmClean_validateDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::DrapeHull,dtmP->ppTol,&numErrors)) goto errexit  ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Drape Boundary Polygon Validated") ;
    /*
    ** Clean Up
    */
cleanup :
    /*
    ** Return
    */
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Validating Polygonal Dtm Features Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Validating Polygonal Dtm Features Error") ;
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmData_resolveIntersectingPolygonalDtmFeatureTypeDtmObject
    (
    BC_DTM_OBJ *dtmP,
    DTMFeatureType dtmFeatureType
    ) 
    /*
    ** This Function Resolves Intersecting Polygonal Features
    */
    {
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
    long   dtmFeature,numFeatureTypes=0,numFeaturePts=0 ;  
    DPoint3d    *featurePtsP=NULL ; 
    BC_DTM_OBJ *polyDtmP=NULL ;
    BC_DTM_FEATURE *dtmFeatureP ;
    /*
    ** Write Entry Message
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Resolving Intersecting Polygonal DTM Feature Type") ;
        bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
        bcdtmWrite_message(0,0,0,"dtmFeatureType  = %8ld",dtmFeatureType) ;
        }  
    /*
    ** Check For Valid DTM Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Check For Untriangulated DTM
    */
    if( dtmP->dtmState != DTMState::Data )
        {
        bcdtmWrite_message(1,0,0,"Method Requires Untriangulated DTM") ;
        goto errexit ;
        }
    /*
    ** Check For Valid Feature Types
    */
    switch( dtmFeatureType )
        {
        case DTMFeatureType::Void :
        case DTMFeatureType::BreakVoid :
        case DTMFeatureType::DrapeVoid :
        case DTMFeatureType::Island :
            break ;

        default :
            bcdtmWrite_message(2,0,0,"Invalid Feature Type For Method") ;
            goto errexit ;
            break ;  
        }  ;
    /*
    ** Count Number Of Feature Types In DTM
    */
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
        {
        dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
        if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data && dtmFeatureP->dtmFeatureType == dtmFeatureType )
            {
            ++numFeatureTypes ;
            }
        }
    /*
    ** Only Process If There Are More Than One Occurrence Of The Feature Type
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"numFeatureTypes = %8ld",numFeatureTypes) ;
    if( numFeatureTypes > 1 )
        {
        /*
        **  Create Temporary Object To Store Feature Occurrences
        */
        if( bcdtmObject_createDtmObject(&polyDtmP)) goto errexit ;
        bcdtmObject_setPointMemoryAllocationParametersDtmObject(polyDtmP,10000,10000) ;
        polyDtmP->ppTol = dtmP->ppTol ; 
        polyDtmP->plTol = dtmP->plTol ;
        /*
        **  Move Features To Temporary DTM
        */
        for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
            if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data && dtmFeatureP->dtmFeatureType == dtmFeatureType )
                {
                if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
                if( bcdtmObject_storeDtmFeatureInDtmObject(polyDtmP,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,2,&dtmFeatureP->dtmFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
                if( bcdtmInsert_removeDtmFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ;
                if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
                }
            }
        /*
        **  Remove Deleted Features
        */
        if( bcdtmData_compactFeatureTableDtmObject(dtmP)) goto errexit ;  
        /*
        **  Get Hulls For Intersecting Polygonal Feature Type
        */
        if( bcdtmData_getHullsForIntersectingPolyonalFeaturesDtmObject(polyDtmP,dtmFeatureType)) goto errexit ;
        /*
        **  Copy Intersected Features To DTM
        */
        for( dtmFeature = 0 ; dtmFeature < polyDtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(polyDtmP,dtmFeature) ;
            if( ( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data && dtmFeatureP->dtmFeatureType == dtmFeatureType ) || 
                ( dtmFeatureP->dtmFeatureType  == DTMFeatureType::Island     && dtmFeatureType              == DTMFeatureType::Void      ) ||
                ( dtmFeatureP->dtmFeatureType  == DTMFeatureType::Void       && dtmFeatureType              == DTMFeatureType::Island    )     )
                {
                if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(polyDtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
                if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,2,&dtmFeatureP->dtmFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
                if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
                } 
            }
        }
    /*
    ** Clean Up
    */
cleanup :
    if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
    if( polyDtmP    != NULL )   bcdtmObject_destroyDtmObject(&polyDtmP) ; 
    /*
    ** Return
    */
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Resolving Intersecting Polygonal DTM Feature Type Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Resolving Intersecting Polygonal DTM Feature Type Error") ;
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmData_getHullsForIntersectingPolyonalFeaturesDtmObject
    (
    BC_DTM_OBJ *polygonalDtmP,
    DTMFeatureType dtmFeatureType 
    )
    /*
    ** This Function Resolves Intersecting Occurrences Of Polygonal DTM Feature Types
    ** Assumes The Polygonal DTM Feature Type Occurences Are Clean And Closed
    */
    {
 int      ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long     sp, np, hp, ss, numFeatures = 0;
  DTMFeatureType compDtmFeatureType;
    long     dtmFeature,numStartFeatures,coincidentVoid ;
    long     firstPoint,knotFound,numFeaturesWithKnots ;
    long     numBreakFeatures,numDtmFeatures ;
    long     numP3dPts,numVoids=0,numIslands=0,numPts=0,closeFlag=0;
    DTMDirection direction ;
    double   area ; 
    char     dtmFeatureTypeName[50],compDtmFeatureTypeName[50] ;
    DPoint3d      *p3dPtsP=NULL ;
    BC_DTM_OBJ      *dtmP=NULL ;
    BC_DTM_FEATURE  *dtmFeatureP ; 
    DTM_TIN_POINT   *pntP ;
    /*
    ** Write Entry Message
    */
    if( dbg ) 
        {
        bcdtmWrite_message(0,0,0,"Resolving Polygonal Dtm Feature Occurrences") ;
        bcdtmWrite_message(0,0,0,"polygonalDtmP  = %p",polygonalDtmP) ;
        bcdtmWrite_message(0,0,0,"dtmFeatureType = %8ld",dtmFeatureType) ;
        }
    /*
    ** Test For Valid Polygonal Feature Type
    */
    if( ! bcdtmData_testForValidPolygonalDtmFeatureType(dtmFeatureType) )
        {
        bcdtmWrite_message(2,0,0,"Invalid Polygonal Feature Type") ;
        goto errexit ;
        }
    /*
    ** Set Complimentary DTM Feature Type
    */
    compDtmFeatureType = DTMFeatureType::None;
    if     ( dtmFeatureType == DTMFeatureType::Void  || dtmFeatureType == DTMFeatureType::BreakVoid ) compDtmFeatureType = DTMFeatureType::Island ;
    else if( dtmFeatureType == DTMFeatureType::Island ) compDtmFeatureType = DTMFeatureType::Void ; 
    /*
    ** Get Feature Names
    */ 
    if( dbg )
        {
        bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureType,dtmFeatureTypeName) ;
        if( compDtmFeatureType != DTMFeatureType::None ) bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(compDtmFeatureType,compDtmFeatureTypeName) ;
        else                                               compDtmFeatureTypeName[0] = (char )0 ; 
        bcdtmWrite_message(0,0,0,"Dtm Feature Type = %s ** Complimentary Dtm Feature Type = %s",dtmFeatureTypeName,compDtmFeatureTypeName) ;
        } 
    /*
    ** Check For Knots In Source Features
    */
    if( cdbg == 1 )
        {
        bcdtmWrite_message(0,0,0,"Checking For Knots In Polygonal Features") ;
        numFeaturesWithKnots = 0 ;
        for( dtmFeature = 0 ; dtmFeature < polygonalDtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(polygonalDtmP,dtmFeature) ;
            if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data && dtmFeatureP->dtmFeatureType == dtmFeatureType )
                {
                if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(polygonalDtmP,dtmFeature,&p3dPtsP,&numP3dPts)) goto errexit ;
                if( bcdtmData_checkForKnots(p3dPtsP,numP3dPts,&knotFound)) goto errexit ; 
                if( knotFound )
                    {
                    ++numFeaturesWithKnots ;
                    bcdtmWrite_message(0,0,0,"Knot Found In DTM Feature %8ld",dtmFeature) ;
                    } 
                }
            } 
        bcdtmWrite_message(0,0,0,"Number Of Features With Knots = %8ld",numFeaturesWithKnots) ;
        if( numFeaturesWithKnots )
            {
            bcdtmWrite_message(1,0,0,"Knots In Source Features") ;
            goto errexit ;  
            }
        }
    /*
    ** Set Direction Of Polygonal DTM Features AntiClockwise
    */
    if( bcdtmClean_setDtmPolygonalFeatureTypeAntiClockwiseDtmObject(polygonalDtmP,dtmFeatureType)) goto errexit ;
    /*
    ** Create DTM Object
    */
    if( bcdtmObject_createDtmObject(&dtmP)) goto errexit ;
    bcdtmObject_setPointMemoryAllocationParametersDtmObject(dtmP,2*polygonalDtmP->numPoints,polygonalDtmP->numPoints) ;
    /*
    **  Write DTM Feature Type To DTM Object As Breaks
    */
    numVoids = 0 ;
    for( dtmFeature = 0 ; dtmFeature < polygonalDtmP->numFeatures ; ++dtmFeature )
        {
        dtmFeatureP = ftableAddrP(polygonalDtmP,dtmFeature) ;
        if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data && dtmFeatureP->dtmFeatureType == dtmFeatureType )
            {
            if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(polygonalDtmP,dtmFeature,&p3dPtsP,&numP3dPts)) goto errexit ;
            if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Breakline,dtmFeatureP->dtmUserTag,1,&dtmFeatureP->dtmFeatureId,p3dPtsP,numP3dPts)) goto errexit ; 
            ++numVoids ;
            }
        }
    /*
    ** Write Features To File
    */
    if( dbg == 1 ) bcdtmWrite_geopakDatFileFromDtmObject(dtmP,L"unresolved.dat") ;
    /*
    ** Triangulate 
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Creating Break Tin ** Number Of Breaks = %8ld",dtmP->numFeatures) ;
    dtmP->ppTol = 0.0 ;
    dtmP->plTol = 0.0 ;  
    if( bcdtmObject_createTinDtmObject(dtmP,1,0.0, false)) goto errexit ;
    /*
    ** Log Number Of Intersected Features
    */
    if( dbg == 2 )
        {
        for( sp = 0 ; sp < dtmP->numPoints ; ++sp )
            {
            if( bcdtmList_countNumberOfDtmFeaturesForPointDtmObject(dtmP,sp,&numFeatures)) goto errexit ;
            if( numFeatures > 1 ) 
                {
                pntP = pointAddrP(dtmP,sp) ;
                bcdtmWrite_message(0,0,0,"numFeatures = %8ld ** %12.5lf %12.5lf %10.4lf",numFeatures,pntP->x,pntP->y,pntP->z) ;
                }
            } 
        } 
    /*
    ** Remove None Feature Hull Lines
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Removing Non Feature Hull Lines") ;
    if( bcdtmList_removeNoneFeatureHullLinesDtmObject(dtmP)) goto errexit ;
    if( dbg ) bcdtmWrite_toFileDtmObject(dtmP,L"voidHulls.bcdtm") ;
    /*
    ** Report DTM Stats
    */
    if( dbg == 2 ) bcdtmObject_reportStatisticsDtmObject(dtmP) ;
    /*
    ** Set Pointer To Last Feature In Tin
    */
    numStartFeatures = dtmP->numFeatures ;
    /*
    ** Scan Tin Hull To Get Voids
    */
    if( dbg )bcdtmWrite_message(0,0,0,"Scanning For External %s",dtmFeatureTypeName) ;
    numVoids = 0 ;
    sp = dtmP->hullPoint ;
    do
        {
        np = nodeAddrP(dtmP,sp)->hPtr ;
        if( bcdtmList_testForBreakLineDtmObject(dtmP,sp,np) )
            {
            /*
            **     Only Proces If Point Has Not Been Already Included In A Void
            */
            if( ! bcdtmList_testForDtmFeatureTypeLineDtmObject(dtmP,sp,np,dtmFeatureType))
                {
                /*
                **        Scan Around External Edge Of Break Lines
                */
                hp = sp ;
                nodeAddrP(dtmP,sp)->tPtr = np ;
                do
                    { 
                    if( ( hp = bcdtmList_nextAntDtmObject(dtmP,np,hp)) < 0 ) goto errexit ;
                    while ( ! bcdtmList_testForBreakLineDtmObject(dtmP,np,hp))
                        {
                        if( ( hp = bcdtmList_nextAntDtmObject(dtmP,np,hp)) < 0 ) goto errexit ;
                        }
                    nodeAddrP(dtmP,np)->tPtr = hp ;
                    ss = hp ;
                    hp = np ;
                    np = ss ;
                    } while ( hp != sp ) ;
                    /*
                    **        Check Connectivity
                    */
                    if( bcdtmList_checkConnectivityTptrListDtmObject(dtmP,sp,1)) goto errexit ;
                    if( dbg == 2 ) bcdtmList_writeTptrListDtmObject(dtmP,sp) ;
                    /*
                    **        Store Void Feature In Tin
                    */
                    if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,dtmFeatureType,dtmP->nullUserTag,dtmP->nullFeatureId,sp,1)) goto errexit ; 
                    ++numVoids ;
                }
            }
        sp = np ; 
        } while ( sp != dtmP->hullPoint ) ;
        /*
        ** Log Number Of External Features
        */
        if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Number Of External %20s = %6ld",dtmFeatureTypeName,numVoids) ;   
        /*
        ** Get Complementary Feature Type
        */
        if( compDtmFeatureType != DTMFeatureType::None )
            {
            if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For Internal %s Between %s",compDtmFeatureTypeName,dtmFeatureTypeName) ;
            numIslands = 0 ;
            for( dtmFeature = 0 ; dtmFeature < numStartFeatures ; ++dtmFeature )
                {
                if( dbg == 2 )
                    {
                    if( bcdtmList_countNumberOfDtmFeaturePointsDtmObject(dtmP,dtmFeature,&numPts,&closeFlag)) goto errexit ;
                    bcdtmWrite_message(0,0,0,"dtmFeature[%5ld] ** state = %2ld numPts = %8ld closeFlag = %2ld",dtmFeature,ftableAddrP(dtmP,dtmFeature)->dtmFeatureState,numPts,closeFlag) ;
                    }
                /*
                **     Copy Feature To Sptr List
                */          
                if( bcdtmList_copyDtmFeatureToSptrListDtmObject(dtmP,dtmFeature,&firstPoint)) goto errexit ;
                if( dbg == 2 ) bcdtmList_writeSptrListDtmObject(dtmP,firstPoint) ;
                /*
                **     Log Intersect Points On Feature
                */
                if( dbg == 2 )
                    {
                    sp = firstPoint ;    
                    do
                        {
                        bcdtmList_countNumberOfHardBreakFeaturesForPointDtmObject(dtmP,sp,&numFeatures) ;  
                        if( numFeatures > 1 )
                            {
                            bcdtmWrite_message(0,0,0,"dtmFeature = %5ld ** numFeatures = %2ld sp = %8ld",dtmFeature,numFeatures,sp) ;
                            } 
                        sp = nodeAddrP(dtmP,sp)->sPtr ; ;  
                        } while( sp != firstPoint && sp != dtmP->nullPnt) ;
                    }   
                /*
                **     Scan To Intersect Point Not On A External DTM Feature
                */
                sp = firstPoint ;    
                do
                    {
                    np = nodeAddrP(dtmP,sp)->sPtr ;
                    bcdtmList_countNumberOfHardBreakFeaturesForPointDtmObject(dtmP,sp,&numBreakFeatures) ;     
                    bcdtmList_countNumberOfDtmFeaturesForPointDtmObject(dtmP,sp,&numDtmFeatures) ;     
                    if( numBreakFeatures > 1 && numDtmFeatures == numBreakFeatures )
                        { 
                        if( dbg == 1 ) bcdtmWrite_message(0,0,0,"dtmFeature = %4ld ** numBreakFeatures = %2ld numDtmFeatures = %2ld ** sp = %8ld ** %12.4lf %12.4lf %10.4lf",dtmFeature,numBreakFeatures,numFeatures,sp,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z) ;
                        /*
                        **           Scan Around External Edge Of Break Lines
                        */
                        ss = sp ;
                        nodeAddrP(dtmP,sp)->tPtr = np ;
                        do
                            {
                            if( ( hp = bcdtmList_nextAntDtmObject(dtmP,np,sp)) < 0 ) goto errexit ;
                            while ( ! bcdtmList_testForBreakLineDtmObject(dtmP,np,hp))
                                {
                                if( ( hp = bcdtmList_nextAntDtmObject(dtmP,np,hp)) < 0 ) goto errexit ;
                                }
                            nodeAddrP(dtmP,np)->tPtr = hp ;
                            sp = np ;
                            np = hp ;
                            } while( sp != ss ) ;
                            /*
                            **           Check Connectivity
                            */
                            if( bcdtmList_checkConnectivityTptrListDtmObject(dtmP,sp,1)) goto errexit ;
                            if( dbg == 1 ) bcdtmList_writeTptrListDtmObject(dtmP,sp) ;
                            /*
                            **           Check For Coincident Features
                            */
                            coincidentVoid = FALSE ;
                            hp = sp ;
                            do
                                {
                                np =  nodeAddrP(dtmP,hp)->tPtr ;
                                if( compDtmFeatureType == DTMFeatureType::Island )
                                    {
                                    if( bcdtmList_testForVoidHullLineDtmObject(dtmP,hp,np)) coincidentVoid = TRUE ;
                                    if( bcdtmList_testForVoidHullLineDtmObject(dtmP,np,hp)) coincidentVoid = TRUE ;
                                    } 
                                if( compDtmFeatureType == DTMFeatureType::Void )
                                    {
                                    if( bcdtmList_testForIslandHullLineDtmObject(dtmP,hp,np)) coincidentVoid = TRUE ;
                                    if( bcdtmList_testForIslandHullLineDtmObject(dtmP,np,hp)) coincidentVoid = TRUE ;
                                    } 
                                hp = np ;
                                } while ( hp != sp && coincidentVoid == FALSE ) ;   
                                /*
                                **           Only Process If There Are No Coincident Features Already Resolved
                                */
                                if( coincidentVoid == FALSE )
                                    {
                                    /*
                                    **              Check Direction
                                    */
                                    bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,sp,&area,&direction) ;
                                    if( dbg == 1 ) bcdtmWrite_message(0,0,0,"area = %12.4lf ** direction = %2ld",area,direction) ;
                                    /*
                                    **              If Direction Clockwise Store As Island
                                    */
                                    if( direction == DTMDirection::Clockwise )
                                        { 
                                        /*
                                        **                 Set Tptr Polygon Anti Clockwise
                                        */
                                        bcdtmList_reverseTptrPolygonDtmObject(dtmP,sp) ;
                                        if( dbg == 2 ) bcdtmList_writeTptrListDtmObject(dtmP,sp) ;
                                        /*
                                        **                 Store Complimentary Feature In DTM
                                        */
                                        if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,compDtmFeatureType,dtmP->nullUserTag,dtmP->nullFeatureId,sp,1)) goto errexit ; 
                                        ++numIslands ;
                                        }
                                    }
                                /*
                                **           Reset Np
                                */
                                np = nodeAddrP(dtmP,ss)->sPtr ;
                        }
                    if (dbg && np == dtmP->nullPnt)
                        bcdtmWrite_message (0,0,0, "The feature doesn't close");
                    sp = np ;
                    } while( sp != firstPoint && sp != dtmP->nullPnt) ;
                    /*
                    **     Null sPtr List
                    */
                    if( bcdtmList_nullSptrListDtmObject(dtmP,firstPoint)) goto errexit ;
                }
            /*
            ** Log Number Of Internal Features
            */
            if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Number Of Internal %20s = %6ld",compDtmFeatureTypeName,numIslands) ;   
            }
        /*
        ** Initialise Polygonal Features DTM
        */
        bcdtmObject_initialiseDtmObject(polygonalDtmP) ;
        /*
        ** Copy Features To Polygonal Features DTM
        */
        numVoids   = 0 ;
        numIslands = 0 ;
        for( dtmFeature = numStartFeatures ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
            if( dtmFeatureP->dtmFeatureType == dtmFeatureType || dtmFeatureP->dtmFeatureType == compDtmFeatureType )
                {
                if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&p3dPtsP,&numP3dPts)) goto errexit ;
                if( dtmFeatureP->dtmFeatureType == dtmFeatureType ) 
                    {
                    if( bcdtmObject_storeDtmFeatureInDtmObject(polygonalDtmP,dtmFeatureType,dtmFeatureP->dtmUserTag,1,&dtmFeatureP->dtmFeatureId,p3dPtsP,numP3dPts)) goto errexit ; 
                    ++numVoids ;
                    } 
                if( dtmFeatureP->dtmFeatureType == compDtmFeatureType )
                    { 
                    if( bcdtmObject_storeDtmFeatureInDtmObject(polygonalDtmP,compDtmFeatureType,dtmFeatureP->dtmUserTag,1,&dtmFeatureP->dtmFeatureId,p3dPtsP,numP3dPts)) goto errexit ; 
                    ++numIslands ;
                    }
                }
            }
        /*
        ** Write Number Of Resolved Polygons
        */
        if( dbg )
            {
            bcdtmWrite_message(0,0,0,"Number Of %20s  Written = %6ld",dtmFeatureTypeName,numVoids) ;
            if (compDtmFeatureType != DTMFeatureType::None) bcdtmWrite_message (0, 0, 0, "Number Of %20s  Written = %6ld", compDtmFeatureTypeName, numIslands);
            if( bcdtmWrite_geopakDatFileFromDtmObject(polygonalDtmP,L"resolved.dat")) goto errexit ;
            }
        /*
        ** Clean Up
        */
cleanup :
        //DTM_NORMALISE_OPTION = TRUE ;
        if( p3dPtsP   != NULL ) free(p3dPtsP) ;
        if( dtmP      != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ;
        /*
        ** Job Completed
        */
        if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Resolving Polygonal Dtm Feature Occurrences Completed") ;
        if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Resolving Polygonal Dtm Feature Occurrences Error") ;
        return(ret) ;
        /*
        ** Error Exit
        */
errexit :
        if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
        goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmData_resolveDataStateIslandsVoidsAndHolesDtmObject
    (
    BC_DTM_OBJ *dtmP
    ) 
    /*
    ** This Function Resolves Intersecting Polygonal Features
    */
    {
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
    long   dtmFeature,numFeaturePts=0 ;  
    long   numVoids,numIslands,startPnt ;
    DPoint3d    *featurePtsP=NULL ; 
    BC_DTM_OBJ *featureDtmP=NULL,*tempDtmP=NULL ;
    BC_DTM_FEATURE *dtmFeatureP ;
    DTMFeatureId nullFeatureId=DTM_NULL_FEATURE_ID ;
    /*
    ** Write Entry Message
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Resolving Data State Islands Voids And Holes") ;
        bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
        }  
    /*
    ** Check For Valid DTM Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Check For Untriangulated DTM
    */
    if( dtmP->dtmState != DTMState::Data )
        {
        bcdtmWrite_message(1,0,0,"Method Requires Untriangulated DTM") ;
        goto errexit ;
        }
    /*
    ** Count Number Of Feature Types In DTM
    */
    numVoids = numIslands = 0 ;
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
        {
        dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
        if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data )
            {
            switch( dtmFeatureP->dtmFeatureType )
                {
                case DTMFeatureType::Void :
                case DTMFeatureType::BreakVoid :
                case DTMFeatureType::DrapeVoid :
                case DTMFeatureType::Hole :
                    ++numVoids ;
                    break ;   

                case DTMFeatureType::Island :
                    ++numIslands ;
                    break ;   
                }
            }
        } 
    /*
    ** Only Process If There Is More Than One Occurrence Of Each Feature Type
    */
    if( numVoids > 1 && numIslands > 1 )
        {
        /*
        **  Create Temporary Object To Store Feature Occurrences
        */
        if( bcdtmObject_createDtmObject(&featureDtmP)) goto errexit ;
        bcdtmObject_setPointMemoryAllocationParametersDtmObject(featureDtmP,10000,10000) ;
        /*
        **  Move Features To Temporary Features DTM
        */
        for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
            if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data )
                {
                switch( dtmFeatureP->dtmFeatureType )
                    {
                    case DTMFeatureType::Void :
                    case DTMFeatureType::BreakVoid :
                    case DTMFeatureType::DrapeVoid :
                    case DTMFeatureType::Hole :
                    case DTMFeatureType::Island :
                        ++numIslands ;
                        if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
                        if( bcdtmObject_storeDtmFeatureInDtmObject(featureDtmP,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,2,&dtmFeatureP->dtmFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
                        if( bcdtmInsert_removeDtmFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ;
                        if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
                        break ;   
                    }
                }    
            }
        /*
        **  Remove Deleted Features
        */
        if( bcdtmData_compactFeatureTableDtmObject(dtmP)) goto errexit ;  
        /*
        **  Create DTM For Triangulating The Features As Hard Breaks 
        */
        if( bcdtmObject_createDtmObject(&tempDtmP)) goto errexit ; 
        bcdtmObject_setPointMemoryAllocationParametersDtmObject(tempDtmP,10000,10000) ;
        tempDtmP->ppTol = tempDtmP->plTol = 0.0 ;
        /*
        **  Store Features In DTM As Hard Breaks
        */
        for( dtmFeature = 0 ; dtmFeature < featureDtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(featureDtmP,dtmFeature) ;
            if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(featureDtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
            if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::Breakline,DTM_NULL_USER_TAG,1,&nullFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
            if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
            }
        /*
        **  Triangulate DTM  
        */
        if( bcdtmObject_triangulateDtmObject(tempDtmP)) goto errexit ;
        /*
        **  Add Features To Triangulated DTM
        */
        for( dtmFeature = 0 ; dtmFeature < featureDtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(featureDtmP,dtmFeature) ;
            if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(featureDtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
            if( bcdtmInsert_internalStringIntoDtmObject(tempDtmP,1,2,featurePtsP,numFeaturePts,&startPnt)) goto errexit ;
            if( bcdtmInsert_addDtmFeatureToDtmObject(tempDtmP,NULL,0,dtmFeatureP->dtmFeatureType,dtmP->nullUserTag,dtmP->nullFeatureId,startPnt,1)) goto errexit  ;          
            }
        /*
        **  Resolve Voids And Islands In Triangualted DTM   
        */
        if( featureDtmP != NULL )   bcdtmObject_destroyDtmObject(&featureDtmP) ; 
        if( bcdtmData_resolveTinStateIslandsVoidsAndHolesDtmObject(tempDtmP,&featureDtmP)) goto errexit ;
        /*
        **  Copy Voids And Islands To Starting DTM
        */
        for( dtmFeature = 0 ; dtmFeature < tempDtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(tempDtmP,dtmFeature) ;
            if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && ( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void && dtmFeatureP->dtmFeatureType == DTMFeatureType::Island ))
                { 
                if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(tempDtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
                if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,1,&dtmFeatureP->dtmFeatureId,featurePtsP,numFeaturePts)) goto errexit  ;          
                }
            }   
        }
    /*
    ** Clean Up
    */
cleanup :
    if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
    if( featureDtmP != NULL )   bcdtmObject_destroyDtmObject(&featureDtmP) ; 
    if( tempDtmP    != NULL )   bcdtmObject_destroyDtmObject(&tempDtmP) ; 
    /*
    ** Return
    */
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Resolving Data State Islands Voids And Holes Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Resolving Data State Islands Voids And Holes Error") ;
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmData_directionallyMarkLinesInternalToTptrPolygonDtmObject
    (
    BC_DTM_OBJ *dtmP,
    long       sPnt,
    long       *tinLinesP,
    long       mark,
    long       *numMarkedP
    ) 
    /*
    ** This Function Directionally Marks Lines Internal To a Tptr Polygon
    ** Directional means That Line P1P2 is different To Line P2P1
    ** This Is Used For Resolving Overlapping Polygonal DTM Features .
    ** The Void Lines Are Marked In This Manner For Polygonisation Purposes
    **
    ** Rob Cormack October 2011
    ** 
    */
    {
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
    long npnt,ppnt,lpnt,clp,clc,pnt,fPnt,lPnt,offset,numPtsMarked=0 ;
    DTMDirection direction=DTMDirection::Clockwise; 
    double area=0.0 ;
    unsigned long mask=0x0 ;
    /*
    ** Write Entry Message
    */
    if( dbg ) 
        {
        bcdtmWrite_message(0,0,0,"Directionally Marking Lines Internal To A Tptr Polygon") ;
        bcdtmWrite_message(0,0,0,"dtmP       = %p",dtmP) ;
        bcdtmWrite_message(0,0,0,"sPnt       = %8ld",sPnt) ;
        bcdtmWrite_message(0,0,0,"tinLinesP  = %8ld",tinLinesP) ;
        bcdtmWrite_message(0,0,0,"mark       = %8ld",mark) ;
        bcdtmWrite_message(0,0,0,"numMarkedP = %8ld",*numMarkedP) ;
        } 
    /*
    ** Check For Valid DTM Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Check For Triangulated DTM
    */
    if( dtmP->dtmState != DTMState::Tin )
        {
        bcdtmWrite_message(1,0,0,"Method Requires Triangulated DTM") ;
        goto errexit ;
        }
    /*
    ** Initialise
    */
    *numMarkedP = 0 ;
    fPnt = lPnt = dtmP->nullPnt ;
    /*
    ** Set Mask Bit
    */
    if( mark == 1 )  mask=0x1 ;
    if( mark == 2 )  mask=0x2 ;
    if( mark == 4 )  mask=0x4 ;
    /*
    ** Check Triangulation
    */
    if( cdbg )
        {
        bcdtmWrite_message(0,0,0,"Checking Triangulation") ;
        if( bcdtmCheck_tinComponentDtmObject(dtmP))
            {
            bcdtmWrite_message(2,0,0,"Triangulation Invalid") ;
            goto errexit ;
            }
        bcdtmWrite_message(0,0,0,"Triangulation Valid") ;
        }
    /*
    ** Check Connectivity Of Tptr Polygon
    */
    if( cdbg )
        {
        if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,sPnt,0))
            {
            bcdtmWrite_message(2,0,0,"Connectivity Error In Tptr Polygon") ;
            goto errexit ;
            }
        } 
    /*
    ** Check Direction Of Tptr Polygon
    */
    bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,sPnt,&area,&direction) ;
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"area = %12.4lf ** direction = %2ld",area,direction) ;
    if (direction == DTMDirection::Clockwise)
        {
        bcdtmWrite_message(2,0,0,"Tptr Polygon Must Have A Counter Clockwise Direction") ;
        goto errexit ;
        }
    /*
    ** Scan Around Tptr Polygon And Mark Immediately Internal Points And Create Internal Tptr List
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Immediately Internal To Tptr Polygon") ;
    bcdtmList_nullSptrValuesDtmObject(dtmP) ;
    ppnt = sPnt ;
    pnt  = nodeAddrP(dtmP,sPnt)->tPtr ; 
    do
        {
        clp = npnt = nodeAddrP(dtmP,pnt)->tPtr ;
        if(( clp = bcdtmList_nextAntDtmObject(dtmP,pnt,clp)) < 0 ) goto errexit ; 
        /*
        **  Scan Anti Clockwise From Next Point On Tptr Polygon To Prior Point
        */
        while ( clp != ppnt )
            {
            if( nodeAddrP(dtmP,clp)->sPtr == dtmP->nullPnt && nodeAddrP(dtmP,clp)->tPtr == dtmP->nullPnt )
                {
                if( fPnt == dtmP->nullPnt ) { fPnt = lPnt = clp ;  }
                else                        { nodeAddrP(dtmP,lPnt)->sPtr = clp ; lPnt = clp ; }
                nodeAddrP(dtmP,clp)->sPtr = -(clp+1) ;
                ++numPtsMarked ;
                if( dbg == 2 ) bcdtmWrite_message(0,0,0,"**00** Marking Point %6ld ** %10.4lf %10.4lf %10.4lf Internal To %6ld ** fPnt = %9ld lPnt = %9ld",clp,pointAddrP(dtmP,clp)->x,pointAddrP(dtmP,clp)->y,pointAddrP(dtmP,clp)->z,pnt,fPnt,lPnt) ;
                }
            if(( clp = bcdtmList_nextAntDtmObject(dtmP,pnt,clp)) < 0 ) goto errexit ; ;
            }
        /*
        **   Reset For Next Point On Tptr Polygon
        */
        ppnt = pnt ;  
        pnt  = npnt ; 
        } while ( ppnt != sPnt ) ;
        /*
        ** Log Initial Internal Point List
        */
        if( cdbg )
            {
            clp = fPnt ;
            bcdtmWrite_message(0,0,0,"numPtsMarked   = %8ld",numPtsMarked) ;
            bcdtmWrite_message(0,0,0,"fpnt        = %8ld lpnt       = %8ld",fPnt,lPnt) ;
            if( fPnt != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"fpnt->sPtr  = %8ld lpnt->sPtr = %8ld",nodeAddrP(dtmP,fPnt)->sPtr,nodeAddrP(dtmP,lPnt)->sPtr)  ;
            if( clp != dtmP->nullPnt )
                { 
                do
                    {
                    bcdtmWrite_message(0,0,0,"InternalPoint[%8ld] = %12.5lf %12.5lf %12.5lf",clp,pointAddrP(dtmP,clp)->x,pointAddrP(dtmP,clp)->y,pointAddrP(dtmP,clp)->z) ;
                    clp = nodeAddrP(dtmP,clp)->sPtr ;
                    if( clp != dtmP->nullPnt && clp < 0 ) clp = dtmP->nullPnt ;
                    } while ( clp != dtmP->nullPnt ) ;
                }
            }   
        /*
        ** Scan Internal Sptr List And Mark Points Connected To Marked Points
        */
        if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Connected To Internal Marked Points") ;
        if( fPnt != dtmP->nullPnt )
            {
            pnt = fPnt ;
            do
                {
                if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Marking Points Connected To Point %6ld  ** numPtsMarked = %8ld",pnt,numPtsMarked) ;
                clc = nodeAddrP(dtmP,pnt)->cPtr ;
                while( clc != dtmP->nullPtr )
                    {
                    clp = clistAddrP(dtmP,clc)->pntNum ;
                    clc = clistAddrP(dtmP,clc)->nextPtr ;
                    if( nodeAddrP(dtmP,clp)->sPtr == dtmP->nullPnt && nodeAddrP(dtmP,clp)->tPtr == dtmP->nullPnt ) 
                        { 
                        nodeAddrP(dtmP,lPnt)->sPtr = clp ; 
                        lPnt = clp ; 
                        ++numPtsMarked ;
                        if( dbg == 2 ) bcdtmWrite_message(0,0,0,"**01** Marking Point %6ld ** %10.4lf %10.4lf %10.4lf Internal To %6ld ** fPnt = %9ld lPnt = %9ld",clp,pointAddrP(dtmP,clp)->x,pointAddrP(dtmP,clp)->y,pointAddrP(dtmP,clp)->z,pnt,fPnt,lPnt) ;
                        }
                    }
                npnt = nodeAddrP(dtmP,pnt)->sPtr ;
                pnt = npnt ;
                if( pnt != dtmP->nullPnt && pnt < 0 ) pnt = dtmP->nullPnt ;  // Last Point Has Been Processed
                } while ( pnt != dtmP->nullPnt ) ;
            }
        /*
        ** Mark Lines Connected To Marked Points
        */
        if( dbg ) bcdtmWrite_message(0,0,0,"Marking Lines Connected To Marked Points ") ;
        if( fPnt != dtmP->nullPnt )
            {
            if( dbg == 2 ) bcdtmWrite_message(0,0,0,"fPnt = %9ld lPnt = %9ld",fPnt,lPnt) ;
            pnt = fPnt ;
            do
                {
                if( dbg == 2  ) bcdtmWrite_message(0,0,0,"Marking Lines Connected To Point %6ld",pnt) ;
                lpnt = pnt ;
                clc = nodeAddrP(dtmP,pnt)->cPtr ;
                while( clc != dtmP->nullPtr )
                    {
                    clp  = clistAddrP(dtmP,clc)->pntNum ;
                    clc = clistAddrP(dtmP,clc)->nextPtr ;
                    if( clp > pnt && nodeAddrP(dtmP,clp)->sPtr != dtmP->nullPnt )
                        { 
                        if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,pnt,clp)) goto errexit ;
                        *(tinLinesP+offset) = *(tinLinesP+offset) | mask ;
                        if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,clp,pnt)) goto errexit ;
                        *(tinLinesP+offset) = *(tinLinesP+offset) | mask ;
                        ++*numMarkedP ; 
                        } 
                    }
                npnt = nodeAddrP(dtmP,pnt)->sPtr ;
                pnt = npnt ;
                if( pnt != dtmP->nullPnt && pnt < 0 ) pnt = dtmP->nullPnt ;  // Last Point Has Been Processed
                } while ( pnt != dtmP->nullPnt ) ;
            }
        if( dbg ) bcdtmWrite_message(0,0,0,"Number Marked = %6ld",*numMarkedP) ;
        /*
        ** Scan Tptr Polygon And Mark Lines Immediately Internal To Tptr Polygon
        */
        if( dbg ) bcdtmWrite_message(0,0,0,"Marking Lines Internal To Tptr Polygon") ;
        ppnt = sPnt ;
        pnt = nodeAddrP(dtmP,ppnt)->tPtr ; 
        do
            {
            clp = npnt = nodeAddrP(dtmP,pnt)->tPtr ;
            if(( clp = bcdtmList_nextAntDtmObject(dtmP,pnt,clp)) < 0 ) goto errexit ; 
            /*
            **  Scan Anti Clockwise From Next Point On Tptr Polygon To Prior Point
            */
            while ( clp != ppnt )
                {

                //     Only Mark Lines That Have Not Already Been Marked

                if( nodeAddrP(dtmP,clp)->sPtr == dtmP->nullPnt )
                    { 
                    if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,pnt,clp)) goto errexit ;
                    *(tinLinesP+offset) = *(tinLinesP+offset) | mask ;
                    if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,clp,pnt)) goto errexit ;
                    *(tinLinesP+offset) = *(tinLinesP+offset) | mask ;
                    ++*numMarkedP ; 
                    } 
                if(( clp = bcdtmList_nextAntDtmObject(dtmP,pnt,clp)) < 0 ) goto errexit ; ;
                }
            ppnt = pnt ;
            pnt = npnt ;
            } while ( ppnt!= sPnt ) ;
            /*
            ** Scan Tptr Polygon And Mark Inside Of Tptr Polygon Lines 
            */
            if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Edge Tptr Polygon") ;
            pnt = sPnt ;
            do
                {
                npnt = nodeAddrP(dtmP,pnt)->tPtr ; 
                if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,npnt,pnt)) goto errexit ;
                *(tinLinesP+offset) = *(tinLinesP+offset) | mask ;
                ++*numMarkedP ; 
                pnt = npnt ;
                } while ( pnt!= sPnt ) ;
                if( dbg ) bcdtmWrite_message(0,0,0,"Number Marked = %6ld",*numMarkedP) ;
                /*
                ** Null Out Internal Sptr List
                */
                if( fPnt != dtmP->nullPnt )
                    {
                    pnt = fPnt ;
                    if( dbg ) bcdtmWrite_message(0,0,0,"Nulling Out Internal Sptr List") ;
                    do
                        {
                        npnt = nodeAddrP(dtmP,pnt)->sPtr ;
                        nodeAddrP(dtmP,pnt)->sPtr = dtmP->nullPnt ;
                        pnt = npnt ;
                        if( pnt != dtmP->nullPnt && pnt < 0 ) pnt = dtmP->nullPnt ;  // Last Point Has Been Processed
                        } while ( pnt != dtmP->nullPnt ) ;
                    }
                /*
                ** Check For None Null Sptr Values
                */
                if( cdbg )
                    {
                    bcdtmList_checkForNoneNullSptrValuesDtmObject(dtmP,&numPtsMarked) ;
                    if( numPtsMarked )
                        {
                        bcdtmWrite_message(1,0,0,"None Null Sptr Values") ;
                        goto errexit ;
                        }
                    //    bcdtmList_reportAndSetToNullNoneNullSptrValuesDtmObject(dtmP,1) ;
                    }   
                /*
                ** Job Completed
                */
cleanup :
                if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Directionally Marking Lines Internal To A Tptr Polygon Completed") ;
                if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Directionally Marking Lines Internal To A Tptr Polygon Error") ;
                return(ret) ;
                /*
                ** Error Exit
                */
errexit :
                if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
                goto cleanup ;
    } 
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmData_resolveTinStateIslandsVoidsAndHolesDtmObject
    (
    BC_DTM_OBJ *dtmP,
    BC_DTM_OBJ **resolvedDtmPP
    )
    /*
    ** This Function Resolves Overlapping Islands Voids And Holes
    */
    {
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
    long    p1,p2,p3,clPtr,offset1,offset2,offset3 ;
    long    fpnt,dtmFeature,numMarked,numFeaturePts ;
    long    *lineP,*tinLinesP=NULL,*featureLinesP=NULL,numTinLines ;
    long    numNotMarked,numIslandsMarked,numVoidsMarked,numHolesMarked,numMultiMarked;
    long    numFeatures, dtmFeatureMarkType;
    DTMFeatureType dtmFeatureType;
    char    dtmFeatureTypeName[50] ;
    DPoint3d     *featurePtsP=NULL ;
    BC_DTM_FEATURE *dtmFeatureP ;
    DTMFeatureId nullFeatureId=DTM_NULL_FEATURE_ID ;
    /*
    ** Write Entry Message
    */
    if( dbg ) 
        {
        bcdtmWrite_message(0,0,0,"Resolving Tin State Islands Voids And Holes") ;
        bcdtmWrite_message(0,0,0,"dtmP          = %p",dtmP) ;
        bcdtmWrite_message(0,0,0,"resolvedDtmPP = %p",*resolvedDtmPP) ;
        } 
    /*
    ** Check For Valid DTM Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Check For Triangulated DTM
    */
    if( dtmP->dtmState != DTMState::Tin )
        {
        bcdtmWrite_message(1,0,0,"Method Requires Triangulated DTM") ;
        goto errexit ;
        }
    /*
    ** Check Triangulation
    */
    if( cdbg )
        {
        bcdtmWrite_message(0,0,0,"Checking Triangulation Before Resolving Voids And Islands") ;
        if( bcdtmCheck_tinComponentDtmObject(dtmP))
            {
            bcdtmWrite_message(2,0,0,"Triangulation Invalid") ;
            goto errexit ;
            }
        bcdtmWrite_message(0,0,0,"Triangulation Valid") ;
        }
    /*
    ** Allocate Memory For Tin Line And Feature Assignments
    */
    numTinLines = dtmP->cListPtr ;
    tinLinesP = ( long * ) malloc( numTinLines * sizeof(long)) ;
    if( tinLinesP == NULL ) 
        { 
        bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
        goto errexit ; 
        }
    /*
    ** Mark Initial Tin Values
    */
    for( lineP = tinLinesP ; lineP < tinLinesP + numTinLines ; ++lineP ) *lineP = 0 ; 
    /*
    ** Mark Internal Feature Lines
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Feature Lines") ;
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature ) 
        {
        dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
        if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin ) 
            {
            /*
            **     Directionally Mark Internal Lines
            */
            if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Island ) dtmFeatureMarkType = 1 ;
            else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void   ) dtmFeatureMarkType = 2 ;
            else if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Hole   ) dtmFeatureMarkType = 4 ;
            else dtmFeatureMarkType = 0 ;
            /*
            **     Marked Features
            */
            if( dtmFeatureMarkType )
                {
                if( dbg ) bcdtmWrite_message(0,0,0,"Marking Feature %8ld of %8ld ** Type %4ld",dtmFeature,dtmP->numFeatures,dtmFeatureP->dtmFeatureType) ;
                /*
                **        Copy Feature To Tptr List
                */
                if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,dtmFeature,&fpnt)) goto errexit ; 
                /*
                **        Mark Internal Tptr Tin Lines
                */ 
                numMarked = 0 ;
                if( bcdtmData_directionallyMarkLinesInternalToTptrPolygonDtmObject(dtmP,fpnt,tinLinesP,dtmFeatureMarkType,&numMarked)) goto errexit ;
                if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Feature Lines Marked = %6ld",numMarked) ;
                }
            }
        }   
    /*
    ** Check That All Triangle Edges Have All Been Marked With The Same Value
    */
    if( cdbg )
        {
        bcdtmWrite_message(0,0,0,"Checking Triangle Marking") ;
        for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
            {
            clPtr = nodeAddrP(dtmP,p1)->cPtr ;
            if( clPtr != dtmP->nullPtr )
                {
                if(( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum )) < 0 ) goto errexit ;
                while( clPtr != dtmP->nullPtr )
                    {
                    p3    = clistAddrP(dtmP,clPtr)->pntNum ;
                    clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
                    if( p2 > p1 && p3 > p1 && nodeAddrP(dtmP,p1)->hPtr != p2 )
                        {
                        if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset1,p1,p2)) goto errexit ;
                        if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset2,p2,p3)) goto errexit ;
                        if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset3,p3,p1)) goto errexit ;
                        if( *(tinLinesP+offset1) != *(tinLinesP+offset2) || *(tinLinesP+offset1) != *(tinLinesP+offset3))
                            {
                            bcdtmWrite_message(0,0,0,"Incorrectly Marked Triangle %8ld %8ld %8ld ** %8ld %8ld %8ld",p1,p2,p3,*(tinLinesP+offset1),*(tinLinesP+offset2),*(tinLinesP+offset3)) ;
                            if( *(tinLinesP+offset1) == *(tinLinesP+offset2) ) *(tinLinesP+offset3) = *(tinLinesP+offset1) ;
                            if( *(tinLinesP+offset1) == *(tinLinesP+offset3) ) *(tinLinesP+offset2) = *(tinLinesP+offset1) ;
                            if( *(tinLinesP+offset2) == *(tinLinesP+offset3) ) *(tinLinesP+offset1) = *(tinLinesP+offset2) ;
                            }
                        }
                    p2 = p3 ;  
                    }
                }
            } 
        bcdtmWrite_message(0,0,0,"Checking Triangle Marking") ;
        for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
            {
            clPtr = nodeAddrP(dtmP,p1)->cPtr ;
            if( clPtr != dtmP->nullPtr )
                {
                if(( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum )) < 0 ) goto errexit ;
                while( clPtr != dtmP->nullPtr )
                    {
                    p3    = clistAddrP(dtmP,clPtr)->pntNum ;
                    clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
                    if( p2 > p1 && p3 > p1 && nodeAddrP(dtmP,p1)->hPtr != p2 )
                        {
                        if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset1,p1,p2)) goto errexit ;
                        if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset2,p2,p3)) goto errexit ;
                        if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset3,p3,p1)) goto errexit ;
                        if( *(tinLinesP+offset1) != *(tinLinesP+offset2) || *(tinLinesP+offset1) != *(tinLinesP+offset3))
                            {
                            bcdtmWrite_message(0,0,0,"Incorrectly Marked Triangle %8ld %8ld %8ld ** %8ld %8ld %8ld",p1,p2,p3,*(tinLinesP+offset1),*(tinLinesP+offset2),*(tinLinesP+offset3)) ;
                            if( *(tinLinesP+offset1) == *(tinLinesP+offset2) ) *(tinLinesP+offset3) = *(tinLinesP+offset1) ;
                            if( *(tinLinesP+offset1) == *(tinLinesP+offset3) ) *(tinLinesP+offset2) = *(tinLinesP+offset1) ;
                            if( *(tinLinesP+offset2) == *(tinLinesP+offset3) ) *(tinLinesP+offset1) = *(tinLinesP+offset2) ;
                            }
                        }
                    p2 = p3 ;  
                    }
                }
            } 
        }   
    /*
    ** Log Number Of Different Tin Lines Marked
    */
    if( dbg == 1 )
        {
        numNotMarked = numIslandsMarked = numVoidsMarked = numHolesMarked = numMultiMarked = 0 ;
        for( lineP = tinLinesP ; lineP < tinLinesP + numTinLines ; ++lineP ) 
            {
            switch ( *lineP )
                {
                case 0  : ++numNotMarked     ; break ;
                case 1  : ++numIslandsMarked ; break ;
                case 2  : ++numVoidsMarked   ; break ;
                case 4  : ++numHolesMarked   ; break ;
                default : ++numMultiMarked   ; break ;
                } ;
            }
        bcdtmWrite_message(0,0,0,"Number Not     Marked = %8ld",numNotMarked) ;  
        bcdtmWrite_message(0,0,0,"Number Islands Marked = %8ld",numIslandsMarked) ; 
        bcdtmWrite_message(0,0,0,"Number Voids   Marked = %8ld",numVoidsMarked) ;  
        bcdtmWrite_message(0,0,0,"Number Holes   Marked = %8ld",numHolesMarked) ;  
        bcdtmWrite_message(0,0,0,"Number Multi   Marked = %8ld",numMultiMarked) ;  
        }   
    /*
    ** Polygonise Dtm Features
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Polygonising Tin Feature Lines") ;
    numFeatures = dtmP->numFeatures ;
    if( bcdtmClean_polygoniseTinLinesDtmObject(dtmP,tinLinesP)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Polygonised Features = %6ld",dtmP->numFeatures-numFeatures) ;
    /*
    ** Extract Polygonised DTM Features
    */
    if( numFeatures < dtmP->numFeatures )
        {
        /*
        **  Create Dtm Object
        */
        if( bcdtmObject_createDtmObject(resolvedDtmPP)) goto errexit ;
        bcdtmObject_setPointMemoryAllocationParametersDtmObject(*resolvedDtmPP,10000,10000) ; 
        /*
        **  Extract Features From Tin
        */
        for( dtmFeature = numFeatures ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
            if( dbg ) 
                {
                bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName) ;
                bcdtmWrite_message(0,0,0,"Feature Type %4ld ** %20s  UserTag = %4I64d",dtmFeatureP->dtmFeatureType,dtmFeatureTypeName,dtmFeatureP->dtmUserTag ) ;
                } 
            if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeatureType == DTMFeatureType::Polygon )
                {
                /*
                **        Assign Feature Type
                */
                dtmFeatureType = DTMFeatureType::Island ;
                if     ( ftableAddrP(dtmP,dtmFeature)->dtmUserTag == 1 ) dtmFeatureType = DTMFeatureType::Island ;
                else if( ftableAddrP(dtmP,dtmFeature)->dtmUserTag == 2 ) dtmFeatureType = DTMFeatureType::Void   ;
                else if( ftableAddrP(dtmP,dtmFeature)->dtmUserTag == 3 ) dtmFeatureType = DTMFeatureType::Hole   ;
                /*
                **        Store Feature
                */
                if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ; 
                if( bcdtmObject_storeDtmFeatureInDtmObject(*resolvedDtmPP,dtmFeatureType,DTM_NULL_USER_TAG,1,&nullFeatureId,featurePtsP,numFeaturePts)) goto errexit ;
                if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
                } 
            }
        } 
    /*
    ** Clean Up
    */
cleanup :
    if( featurePtsP   != NULL ) { free(featurePtsP)   ; featurePtsP   = NULL ; }
    if( tinLinesP     != NULL ) { free(tinLinesP)     ; tinLinesP     = NULL ; }
    if( featureLinesP != NULL ) { free(featureLinesP) ; featureLinesP = NULL ; }
    /*
    ** Job Completed
    */
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Resolving Tin State Islands Voids And Holes Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Resolving Tin State Islands Voids And Holes Error") ;
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmData_validateMnDotVoidsDtmObject
    (
    BC_DTM_OBJ *dtmP 
    )
    /*
    ** This Function Validates Void And Island Data As Collected By Mn Dot
    */
    {
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
    long numErrors=0 ;
    /*
    ** Check For Valid DTM Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Check For Untriangulated DTM
    */
    if( dtmP->dtmState != DTMState::Data )
        {
        bcdtmWrite_message(1,0,0,"Method Requires Untriangulated DTM") ;
        goto errexit ;
        }
    /*
    ** Validate Voids
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Validating Voids") ;
    if( bcdtmClean_validateDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Void,dtmP->ppTol,&numErrors)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Voids Validated") ;
    /*
    ** Validate Break Voids
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Validating Break Voids") ;
    if( bcdtmClean_validateDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::BreakVoid,dtmP->ppTol,&numErrors)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Break Voids Validated") ;
    /*
    ** Validate Drape Voids
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Validating Drape Voids") ;
    if( bcdtmClean_validateDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::DrapeVoid,dtmP->ppTol,&numErrors)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Drape Voids Validated") ;
    /*
    ** Validate Islands
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Validating Islands") ;
    if( bcdtmClean_validateDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Island,dtmP->ppTol,&numErrors)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Islands Validated") ;
    /*
    ** Resolve Intersecting Voids
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Resolving Intersecting Voids") ;
    if( bcdtmData_resolveIntersectingPolygonalDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Void)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting Voids Resolved") ;
    /*
    ** Resolve Intersecting Islands
    */
    /*
    if( dbg ) bcdtmWrite_message(0,0,0,"Resolving Intersecting Islands") ;
    if( bcdtmData_resolveIntersectingPolygonalDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Island)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting Island Resolved") ;
    */ 
    /*
    ** Log Stats
    /*
    ** Clean Up
    */
cleanup :
    /*
    ** Job Completed
    */
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmData_bulkFeatureIdDeleteDtmFeaturesDtmObject
    (
    BC_DTM_OBJ     *dtmP,
    DTMFeatureId *dtmFeatureIdP,
    long           numDtmFeatureId
    )
    /*
    ** Remove All Dtm Features With the Nominated Feature Id's 
    */
    {
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
    long   offset=0,dtmFeature,numDtmFeatureIndex,isModifed=FALSE ;
    DTM_FEATURE_INDEX *dtmFeatureIndexP=NULL ;
    DTMFeatureId *featIdP,*featId1P ;
    BC_DTM_FEATURE *dtmFeatureP ;
    /*
    ** Write Entry Message
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Bulk FeatureId Deleting Dtm Features") ;
        bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
        bcdtmWrite_message(0,0,0,"dtmFeatureId    = %p",dtmFeatureIdP) ;
        bcdtmWrite_message(0,0,0,"numDtmFeatureId = %8ld",numDtmFeatureId) ;
        } 
    /*
    ** Log Feature Ids To Be Deleted
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Number Of Feature Ids To Be Deleted = %8ld",numDtmFeatureId) ;
        for( offset = 0 ; offset < numDtmFeatureId ; ++offset )
            {
            bcdtmWrite_message(0,0,0,"Delete Feature Id[%8ld] = %10I64d",offset,*(dtmFeatureIdP+offset)) ;
            }
        }   
    /*
    ** Check For Valid Dtm Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Only Process If Delete Feature Ids Exist
    */
    if( numDtmFeatureId > 0 && dtmFeatureIdP != NULL )
        { 
        /*
        **  Write Feature Ids
        */
        if( dbg == 1 )
            {
            bcdtmWrite_message(0,0,0,"Number Of Initial DTM Features = %8ld",dtmP->numFeatures) ;
            for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
                {
                dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ; 
                bcdtmWrite_message(0,0,0,"dtmFeature[%8ld] ** FeatureId = %10I64d State = %2ld",dtmFeature,dtmFeatureP->dtmFeatureId,dtmFeatureP->dtmFeatureState) ;
                }
            }   
        /*
        **  Remove Duplicate Delete Id's
        */
        if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Before Removing Duplicate Delete Ids ** Num Ids = %8ld",numDtmFeatureId) ;
        qsortCPP(dtmFeatureIdP,numDtmFeatureId,sizeof(DTMFeatureId),(int(*)(const void*, const void*))bcdtmData_featureIdCompareFunction) ;   
        featIdP = dtmFeatureIdP ;
        for(  featId1P = dtmFeatureIdP + 1 ; featId1P < dtmFeatureIdP + numDtmFeatureId ; ++featId1P )
            {
            if( *featId1P != *featIdP )
                {
                ++featIdP ;
                *featIdP = *featId1P ;
                }
            }
        numDtmFeatureId = (long)(featIdP-dtmFeatureIdP) + 1 ;  
        if( dbg == 1 ) bcdtmWrite_message(0,0,0,"After  Removing Duplicate Delete Ids ** Num Ids = %8ld",numDtmFeatureId) ;
        /*
        **  Create Feature Id Index
        */
        if( dtmP->numFeatures > 0 ) 
            { 
            if( bcdtmData_createFeatureIdIndexDtmObject(dtmP,&dtmFeatureIndexP,&numDtmFeatureIndex)) goto errexit ; 
            if( dbg == 2 ) 
                {
                bcdtmWrite_message(0,0,0,"Number Of Feature Index = %8ld",numDtmFeatureIndex) ;
                for( offset = 0 ; offset < numDtmFeatureIndex ; ++offset )
                    {
                    bcdtmWrite_message(0,0,0,"Feature Index[%8ld] ** Id = %8I64ld dtmFeature = %8ld",offset,(dtmFeatureIndexP+offset)->index,(dtmFeatureIndexP+offset)->dtmFeature) ;
                    }
                } 
            /*
            **     Scan And Delete Features
            */
            for( featIdP = dtmFeatureIdP ; featIdP < dtmFeatureIdP + numDtmFeatureId ; ++featIdP )
                {
                if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Deleting Feature Index = %8I64d",*featIdP) ;
                if( bcdtmData_findFirstFeatureIndexOccurrence((int64_t)*featIdP,dtmFeatureIndexP,numDtmFeatureIndex,&offset)) 
                    {
                    if( dbg == 1 ) bcdtmWrite_message(0,0,0,"First DTM Feature Offset = %8ld",offset) ;
                    while( offset <  numDtmFeatureIndex && (dtmFeatureIndexP+offset)->index  == *featIdP )
                        {
                        dtmFeature = (dtmFeatureIndexP+offset)->dtmFeature ;
                        if( ftableAddrP(dtmP,dtmFeature)->dtmFeatureState != DTMFeatureState::Deleted )
                            {
                            isModifed = TRUE;
                            if( bcdtmInsert_removeDtmFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ;
                            }
                        ++offset ;  
                        } 
                    }
                } 
            }   
        }      
    /*
    **  Update Modified Time
    */
    if( isModifed == TRUE )
        {
        if( bcdtmData_compactFeatureTableDtmObject(dtmP)) goto errexit ;
        bcdtmObject_updateLastModifiedTime (dtmP) ;
        } 
    /*
    ** Log Remaining Features
    */
    if( dbg == 2 )
        {
        bcdtmWrite_message(0,0,0,"Number Of Remaining DTM Features = %8ld",dtmP->numFeatures) ;
        for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ; 
            bcdtmWrite_message(0,0,0,"dtmFeature = %8ld ** Type = %4ld Usertag = %10I64d FeatureId = %10I64d",dtmFeature,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId) ;
            }
        }    
    /*
    ** Clean Up
    */
cleanup :
    if( dtmFeatureIndexP != NULL ) free(dtmFeatureIndexP) ;
    /*
    ** Job Completed
    */
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Bulk FeatureId Deleting Dtm Features Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Bulk FeatureId Deleting Dtm Features Error") ;
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmData_bulkUserTagDeleteDtmFeaturesDtmObject
    (
    BC_DTM_OBJ     *dtmP,
    DTMUserTag   *dtmUserTagP,
    long           numDtmUserTag
    )
    /*
    ** Remove All Dtm Features 
    */
    {
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
    long   offset,dtmFeature,numDtmFeatureIndex,isModifed=FALSE ;
    DTM_FEATURE_INDEX *dtmFeatureIndexP=NULL ;
    DTMUserTag *userTagP,*userTag1P ;
    BC_DTM_FEATURE *dtmFeatureP ;
    /*
    ** Write Entry Message
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Bulk UserTag Deleting Dtm Features") ;
        bcdtmWrite_message(0,0,0,"dtmP           = %p",dtmP) ;
        bcdtmWrite_message(0,0,0,"dtmFeatureId   = %p",dtmUserTagP) ;
        bcdtmWrite_message(0,0,0,"numDtmUserTag  = %8ld",numDtmUserTag) ;
        } 
    /*
    ** Check For Valid Dtm Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Only Process If Delete Feature Ids Exist
    */
    if( numDtmUserTag > 0 && dtmUserTagP != NULL )
        { 
        /*
        **  Remove Duplicate Delete Id's
        */
        if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Before Removing Duplicate Delete Ids ** Num UserTags = %8ld",numDtmUserTag) ;
        qsortCPP(dtmUserTagP,numDtmUserTag,sizeof(DTMUserTag),(int(*)(const void*, const void*))bcdtmData_userTagCompareFunction) ;   
        userTagP = dtmUserTagP ;
        for(  userTag1P = dtmUserTagP + 1 ; userTag1P < dtmUserTagP + numDtmUserTag ; ++userTag1P )
            {
            if( *userTag1P != *userTagP )
                {
                ++userTagP ;
                *userTagP = *userTag1P ;
                }
            }
        numDtmUserTag = (long)(userTagP-dtmUserTagP) + 1 ;  
        if( dbg == 1 ) bcdtmWrite_message(0,0,0,"After  Removing Duplicate Delete Ids ** Num UserTags = %8ld",numDtmUserTag) ;
        /*
        **  Create Feature Id Index
        */
        if( dtmP->numFeatures > 0 ) 
            { 
            if( bcdtmData_createUserTagIndexDtmObject(dtmP,&dtmFeatureIndexP,&numDtmFeatureIndex)) goto errexit ; 
            /*
            **     Scan And Delete Features
            */
            for( userTagP = dtmUserTagP ; userTagP < dtmUserTagP + numDtmUserTag ; ++userTagP )
                {
                if( bcdtmData_findFirstFeatureIndexOccurrence((int64_t)*userTagP,dtmFeatureIndexP,numDtmFeatureIndex,&offset)) 
                    {
                    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"First DTM Feature = %8ld",offset) ;
                    while( offset <  numDtmFeatureIndex && (dtmFeatureIndexP+offset)->index  == *userTagP )
                        {
                        dtmFeature = (dtmFeatureIndexP+offset)->dtmFeature ;
                        if( ftableAddrP(dtmP,dtmFeature)->dtmFeatureState != DTMFeatureState::Deleted )
                            {
                            if( bcdtmInsert_removeDtmFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ;
                            }
                        isModifed = TRUE;
                        ++offset ;  
                        } 
                    }
                } 
            }   
        }      
    /*
    **  Update Modified Time
    */
    if( isModifed == TRUE )
        {
        if( bcdtmData_compactFeatureTableDtmObject(dtmP)) goto errexit ;
        bcdtmObject_updateLastModifiedTime (dtmP) ;
        } 
    /*
    ** Log Remaining Features
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Number Of Remaining DTM Features = %8ld",dtmP->numFeatures) ;
        for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
            {
            dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ; 
            bcdtmWrite_message(0,0,0,"dtmFeature = %8ld ** Type = %4ld Usertag = %10I64d FeatureId = %10I64d",dtmFeature,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId) ;
            }
        }    
    /*
    ** Clean Up
    */
cleanup :
    if( dtmFeatureIndexP != NULL ) free(dtmFeatureIndexP) ;
    /*
    ** Job Completed
    */
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Bulk UserTag Deleting Dtm Features Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Bulk UserTag Deleting Dtm Features Error") ;
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmData_featureIdCompareFunction
    (
    const DTMFeatureId *id1P ,
    const DTMFeatureId *id2P 
    )
    {
    if( *id1P <  *id2P ) return(-1) ;
    if( *id1P >  *id2P ) return( 1) ;
    return(0) ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmData_userTagCompareFunction
    (
    const DTMUserTag *ut1P ,
    const DTMUserTag *ut2P 
    )
    {
    if( *ut1P <  *ut2P ) return(-1) ;
    if( *ut1P >  *ut2P ) return( 1) ;
    return(0) ;
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmData_featureIndexCompareFunction
    (
    const DTM_FEATURE_INDEX *id1P ,
    const DTM_FEATURE_INDEX *id2P 
    )
    {
    if( id1P->index      <  id2P->index      ) return(-1) ;
    if( id1P->index      >  id2P->index      ) return( 1) ;
    if( id1P->dtmFeature <  id2P->dtmFeature ) return(-1) ;
    if( id1P->dtmFeature >  id2P->dtmFeature ) return( 1) ;
    return(0) ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmData_createFeatureIdIndexDtmObject
    (
    BC_DTM_OBJ        *dtmP,
    DTM_FEATURE_INDEX **dtmFeatureIndexPP,
    long              *numDtmFeatureIndexP
    )
    /*
    ** Remove All Dtm Features 
    */
    {
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
    long   dtmFeature ;
    DTM_FEATURE_INDEX *indexP ;
    /*
    ** Write Entry Message
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Creating Feature Id Index") ;
        bcdtmWrite_message(0,0,0,"dtmP                = %p",dtmP) ;
        bcdtmWrite_message(0,0,0,"dtmFeatureIndexPP   = %p",*dtmFeatureIndexPP) ;
        } 
    /*
    ** Initialise
    */
    *numDtmFeatureIndexP = 0 ;
    if( *dtmFeatureIndexPP != NULL )
        {
        free(*dtmFeatureIndexPP) ;
        *dtmFeatureIndexPP = NULL ;
        }
    /*
    ** Check For Valid Dtm Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Only Create Index If Features Exist
    */
    if( dtmP->numFeatures > 0 )
        {
        /*
        **  Allocate Memory For Feature Index
        */ 
        *numDtmFeatureIndexP = dtmP->numFeatures ;
        *dtmFeatureIndexPP = ( DTM_FEATURE_INDEX * ) malloc(*numDtmFeatureIndexP * sizeof( DTM_FEATURE_INDEX)) ;
        if( *dtmFeatureIndexPP == NULL )
            {
            bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
            goto errexit ;
            }
        /*
        **  Populate Feature Index
        */
        for( dtmFeature = 0 , indexP = *dtmFeatureIndexPP ; dtmFeature < dtmP->numFeatures ; ++dtmFeature , ++indexP )
            {    
            indexP->index = (int64_t) ftableAddrP(dtmP,dtmFeature)->dtmFeatureId ;
            indexP->dtmFeature = dtmFeature ;
            }
        /*
        **  Sort Index
        */
        qsortCPP(*dtmFeatureIndexPP,*numDtmFeatureIndexP,sizeof(DTM_FEATURE_INDEX),(int(*)(const void*, const void*))bcdtmData_featureIndexCompareFunction) ;
        /*
        **  Log First 100 Indecies
        */
        if( dbg == 1 )
            {
            for( indexP = *dtmFeatureIndexPP ; indexP < *dtmFeatureIndexPP + dtmP->numFeatures ; ++indexP )
                {
                bcdtmWrite_message(0,0,0,"Index[%8ld] = %8I64d ** %8ld",(long)(indexP-*dtmFeatureIndexPP),indexP->index,indexP->dtmFeature) ;
                }
            }
        } 
    /*
    ** Clean Up
    */
cleanup :
    /*
    ** Job Completed
    */
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Feature Id Index Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating Feature Id Index Error") ;
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmData_createUserTagIndexDtmObject
    (
    BC_DTM_OBJ        *dtmP,
    DTM_FEATURE_INDEX **dtmFeatureIndexPP,
    long              *numDtmFeatureIndexP
    )
    /*
    ** Remove All Dtm Features 
    */
    {
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
    long   dtmFeature ;
    DTM_FEATURE_INDEX *indexP ;
    /*
    ** Write Entry Message
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Creating User Tag Index") ;
        bcdtmWrite_message(0,0,0,"dtmP                = %p",dtmP) ;
        bcdtmWrite_message(0,0,0,"dtmFeatureIndexPP   = %p",*dtmFeatureIndexPP) ;
        } 
    /*
    ** Initialise
    */
    *numDtmFeatureIndexP = 0 ;
    if( *dtmFeatureIndexPP != NULL )
        {
        free(*dtmFeatureIndexPP) ;
        *dtmFeatureIndexPP = NULL ;
        }
    /*
    ** Check For Valid Dtm Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Only Create Index If Features Exist
    */
    if( dtmP->numFeatures > 0 )
        {
        /*
        **  Allocate Memory For Feature Index
        */ 
        *numDtmFeatureIndexP = dtmP->numFeatures ;
        *dtmFeatureIndexPP = ( DTM_FEATURE_INDEX * ) malloc(*numDtmFeatureIndexP * sizeof( DTM_FEATURE_INDEX)) ;
        if( *dtmFeatureIndexPP == NULL )
            {
            bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
            goto errexit ;
            }
        /*
        **  Populate Feature Index
        */
        for( dtmFeature = 0 , indexP = *dtmFeatureIndexPP ; dtmFeature < dtmP->numFeatures ; ++dtmFeature , ++indexP )
            {    
            indexP->index = (int64_t) ftableAddrP(dtmP,dtmFeature)->dtmUserTag ;
            indexP->dtmFeature = dtmFeature ;
            }
        /*
        **  Sort Index
        */
        qsortCPP(*dtmFeatureIndexPP,*numDtmFeatureIndexP,sizeof(DTM_FEATURE_INDEX),(int(*)(const void*, const void*))bcdtmData_featureIndexCompareFunction) ;
        } 
    /*
    ** Clean Up
    */
cleanup :
    /*
    ** Job Completed
    */
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating User Tag Index Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Creating User Tag Index Error") ;
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmData_findFirstFeatureIndexOccurrence
    (
    int64_t           featureValue,
    DTM_FEATURE_INDEX *dtmFeatureIndexP,
    long              numDtmFeatureIndex,
    long              *offsetP
    )
    {
    /*
    ** Binary Search Index 
    */
 int  dbg=DTM_TRACE_VALUE(0) ;
    long top,bot,mid ;
    DTM_FEATURE_INDEX *botP,*topP,*midP ;
    /*
    ** Write Entry Message
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Find First Feature Index Occurrence") ;
        bcdtmWrite_message(0,0,0,"featureValue       = %8I64d",featureValue) ;
        bcdtmWrite_message(0,0,0,"dtmFeatureIndexP   = %p",dtmFeatureIndexP) ;
        bcdtmWrite_message(0,0,0,"numDtmFeatureIndex = %8ld",numDtmFeatureIndex) ;
        } 
    /*
    ** Check Range
    */
    if( numDtmFeatureIndex <= 0 || dtmFeatureIndexP == NULL ) return(0) ; 
    /*
    ** Set Binary Search Range
    */
    botP = dtmFeatureIndexP ;
    if( botP->index == featureValue )
        {
        *offsetP = botP->dtmFeature ;
        return(1) ;
        }
    if( numDtmFeatureIndex == 1 ) return(0) ;  
    topP =  dtmFeatureIndexP + numDtmFeatureIndex - 1 ;  
    if( topP->index == featureValue )
        {
        while( topP >= dtmFeatureIndexP && topP->index == featureValue )
            {
            --topP ;
            }
        ++topP ;
        *offsetP = (long)(topP - dtmFeatureIndexP) ;  
        return(1) ;
        }
    /*
    ** Binary Scan Index
    */
    bot = 0 ;
    top = numDtmFeatureIndex - 1 ;
    while( top - bot  > 1 )
        {
        mid = ( top + bot ) / 2 ;
        midP = dtmFeatureIndexP + mid ;
        if     ( midP->index >  featureValue ) top = mid ;
        else if( midP->index <  featureValue ) bot = mid ;
        else if( midP->index == featureValue )
            {
            while( midP >= dtmFeatureIndexP && midP->index == featureValue ) --midP ;
            ++midP ;
            *offsetP = (long)(midP-dtmFeatureIndexP) ;  
            return(1) ;
            }
        }
    /*
    **  Not Found
    */
    return(0) ;    
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(DTMFeatureType dtmFeatureType,char *dtmFeatureTypeName)
    {
    int  ret=DTM_ERROR ;
    /*
    ** Initialise
    */
    *dtmFeatureTypeName = 0 ;
    /*
    ** Assign Name From Type
    */
    if( dtmFeatureType == DTMFeatureType::RandomSpots     ) strcpy(dtmFeatureTypeName,"DTMFeatureType::RandomSpots") ;
    if( dtmFeatureType == DTMFeatureType::GroupSpots      ) strcpy(dtmFeatureTypeName,"DTMFeatureType::GroupSpots") ;
    if( dtmFeatureType == DTMFeatureType::Breakline      ) strcpy(dtmFeatureTypeName,"DTMFeatureType::Breakline") ;
    if( dtmFeatureType == DTMFeatureType::SoftBreakline      ) strcpy(dtmFeatureTypeName,"DTMFeatureType::SoftBreakline") ;
    if( dtmFeatureType == DTMFeatureType::DrapeLine      ) strcpy(dtmFeatureTypeName,"DTMFeatureType::DrapeLine") ;
    if( dtmFeatureType == DTMFeatureType::GraphicBreak   ) strcpy(dtmFeatureTypeName,"DTMFeatureType::GraphicBreak") ;
    if( dtmFeatureType == DTMFeatureType::ContourLine    ) strcpy(dtmFeatureTypeName,"DTMFeatureType::ContourLine") ; 
    if( dtmFeatureType == DTMFeatureType::Void            ) strcpy(dtmFeatureTypeName,"DTMFeatureType::Void") ;
    if( dtmFeatureType == DTMFeatureType::VoidLine       ) strcpy(dtmFeatureTypeName,"DTMFeatureType::VoidLine") ; 
    if( dtmFeatureType == DTMFeatureType::Island          ) strcpy(dtmFeatureTypeName,"DTMFeatureType::Island") ;
    if( dtmFeatureType == DTMFeatureType::Hole            ) strcpy(dtmFeatureTypeName,"DTMFeatureType::Hole") ;
    if( dtmFeatureType == DTMFeatureType::HoleLine       ) strcpy(dtmFeatureTypeName,"DTMFeatureType::HoleLine") ;
    if( dtmFeatureType == DTMFeatureType::BreakVoid      ) strcpy(dtmFeatureTypeName,"DTMFeatureType::BreakVoid") ;
    if( dtmFeatureType == DTMFeatureType::DrapeVoid      ) strcpy(dtmFeatureTypeName,"DTMFeatureType::DrapeVoid") ;
    if( dtmFeatureType == DTMFeatureType::Hull            ) strcpy(dtmFeatureTypeName,"DTMFeatureType::Hull") ;
    if( dtmFeatureType == DTMFeatureType::DrapeHull      ) strcpy(dtmFeatureTypeName,"DTMFeatureType::DrapeHull") ;
    if( dtmFeatureType == DTMFeatureType::HullLine       ) strcpy(dtmFeatureTypeName,"DTMFeatureType::HullLine") ;
    if( dtmFeatureType == DTMFeatureType::TinHull        ) strcpy(dtmFeatureTypeName,"DTMFeatureType::TinHull") ;
    if( dtmFeatureType == DTMFeatureType::Triangle        ) strcpy(dtmFeatureTypeName,"DTMFeatureType::Triangle") ;
    if( dtmFeatureType == DTMFeatureType::TriangleEdge   ) strcpy(dtmFeatureTypeName,"DTMFeatureType::TriangleEdge") ;
    if( dtmFeatureType == DTMFeatureType::Lattice         ) strcpy(dtmFeatureTypeName,"DTMFeatureType::Lattice") ;
    if( dtmFeatureType == DTMFeatureType::LatticeEdge    ) strcpy(dtmFeatureTypeName,"DTMFeatureType::LatticeEdge") ;
    if( dtmFeatureType == DTMFeatureType::LatticeXLine   ) strcpy(dtmFeatureTypeName,"DTMFeatureType::LatticeXLine") ;
    if( dtmFeatureType == DTMFeatureType::LatticeYLine   ) strcpy(dtmFeatureTypeName,"DTMFeatureType::LatticeYLine") ;
    if( dtmFeatureType == DTMFeatureType::LatticePoint   ) strcpy(dtmFeatureTypeName,"DTMFeatureType::LatticePoint") ;
    if( dtmFeatureType == DTMFeatureType::Contour        ) strcpy(dtmFeatureTypeName,"DTMFeatureType::Contour") ;
    if( dtmFeatureType == DTMFeatureType::ZeroSlopeLine ) strcpy(dtmFeatureTypeName,"DTMFeatureType::ZeroSlopeLine") ;
    if( dtmFeatureType == DTMFeatureType::ISOLine        ) strcpy(dtmFeatureTypeName,"DTMFeatureType::ISOLine") ;
    if( dtmFeatureType == DTMFeatureType::ISOCell        ) strcpy(dtmFeatureTypeName,"DTMFeatureType::ISOCell") ;
    if( dtmFeatureType == DTMFeatureType::Theme           ) strcpy(dtmFeatureTypeName,"DTMFeatureType::Theme") ;
    if( dtmFeatureType == DTMFeatureType::SlopeToe       ) strcpy(dtmFeatureTypeName,"DTMFeatureType::SlopeToe") ;
    if( dtmFeatureType == DTMFeatureType::LowPoint       ) strcpy(dtmFeatureTypeName,"DTMFeatureType::LowPoint") ;
    if( dtmFeatureType == DTMFeatureType::HighPoint      ) strcpy(dtmFeatureTypeName,"DTMFeatureType::HighPoint") ;
    if( dtmFeatureType == DTMFeatureType::SumpLine       ) strcpy(dtmFeatureTypeName,"DTMFeatureType::SumpLine") ;
    if( dtmFeatureType == DTMFeatureType::RidgeLine      ) strcpy(dtmFeatureTypeName,"DTMFeatureType::RidgeLine") ;
    if( dtmFeatureType == DTMFeatureType::DescentTrace   ) strcpy(dtmFeatureTypeName,"DTMFeatureType::DescentTrace") ;
    if( dtmFeatureType == DTMFeatureType::AscentTrace    ) strcpy(dtmFeatureTypeName,"DTMFeatureType::AscentTrace") ;
    if( dtmFeatureType == DTMFeatureType::ZeroSlopeTriangle ) strcpy(dtmFeatureTypeName,"DTMFeatureType::ZeroSlopeTriangle") ;
    if( dtmFeatureType == DTMFeatureType::Catchment       ) strcpy(dtmFeatureTypeName,"DTMFeatureType::Catchment") ;
    if( dtmFeatureType == DTMFeatureType::LowPointPond  ) strcpy(dtmFeatureTypeName,"DTMFeatureType::LowPointPond") ;
    if( dtmFeatureType == DTMFeatureType::VisibleLine    ) strcpy(dtmFeatureTypeName,"DTMFeatureType::VisibleLine") ;
    if( dtmFeatureType == DTMFeatureType::InvisibleLine  ) strcpy(dtmFeatureTypeName,"DTMFeatureType::InvisibleLine") ;
    if( dtmFeatureType == DTMFeatureType::SlopeLine      ) strcpy(dtmFeatureTypeName,"DTMFeatureType::SlopeLine") ;
    if( dtmFeatureType == DTMFeatureType::Polygon         ) strcpy(dtmFeatureTypeName,"DTMFeatureType::Polygon") ;
    if( dtmFeatureType == DTMFeatureType::GradeSlope     ) strcpy(dtmFeatureTypeName,"DTMFeatureType::GradeSlope") ;
    if( dtmFeatureType == DTMFeatureType::Region          ) strcpy(dtmFeatureTypeName,"DTMFeatureType::Region") ;
    /*
    ** Set Return Value
    */
    if( *dtmFeatureTypeName != 0 ) ret = DTM_SUCCESS ; 
    /*
    ** Job Completed
    */
    return(ret) ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmData_getDtmFeatureTypeFromDtmFeatureTypeName (char *dtmFeatureTypeName, DTMFeatureType* dtmFeatureTypeP)
    {
    int  ret=DTM_SUCCESS ;
    /*
    ** Initialise
    */
    *dtmFeatureTypeP = DTMFeatureType::None;
    /*
    ** Assign Type From Name
    */
    if     ( strcmp(dtmFeatureTypeName,"DTMFeatureType::RandomSpots") == 0 )   *dtmFeatureTypeP = DTMFeatureType::RandomSpots ;   
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::GroupSpots") == 0 )    *dtmFeatureTypeP = DTMFeatureType::GroupSpots ;   
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::DatPoint") == 0 )     *dtmFeatureTypeP = DTMFeatureType::DatPoint  ;   
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::TinPoint") == 0 )     *dtmFeatureTypeP = DTMFeatureType::TinPoint  ;   
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::TinLine") == 0 )      *dtmFeatureTypeP = DTMFeatureType::TinLine   ;   
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::Breakline") == 0 )    *dtmFeatureTypeP = DTMFeatureType::Breakline ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::SoftBreakline") == 0 )    *dtmFeatureTypeP = DTMFeatureType::SoftBreakline ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::DrapeLine") == 0 )    *dtmFeatureTypeP = DTMFeatureType::DrapeLine ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::GraphicBreak") == 0 ) *dtmFeatureTypeP = DTMFeatureType::GraphicBreak ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::ContourLine") == 0 )  *dtmFeatureTypeP = DTMFeatureType::ContourLine ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::Void") == 0 )          *dtmFeatureTypeP = DTMFeatureType::Void   ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::VoidLine") == 0 )     *dtmFeatureTypeP = DTMFeatureType::VoidLine ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::Island") == 0 )        *dtmFeatureTypeP = DTMFeatureType::Island   ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::Hole") == 0 )          *dtmFeatureTypeP = DTMFeatureType::Hole  ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::HoleLine") == 0 )     *dtmFeatureTypeP = DTMFeatureType::HoleLine    ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::BreakVoid") == 0 )    *dtmFeatureTypeP = DTMFeatureType::BreakVoid;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::DrapeVoid") == 0 )    *dtmFeatureTypeP = DTMFeatureType::DrapeVoid ;  
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::Hull") == 0 )          *dtmFeatureTypeP = DTMFeatureType::Hull   ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::HullLine") == 0 )     *dtmFeatureTypeP = DTMFeatureType::HullLine ;  
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::Triangle") == 0 )      *dtmFeatureTypeP = DTMFeatureType::Triangle  ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::TriangleEdge") == 0 ) *dtmFeatureTypeP = DTMFeatureType::TriangleEdge  ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::Lattice") == 0 )       *dtmFeatureTypeP = DTMFeatureType::Lattice   ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::LatticeEdge") == 0 )  *dtmFeatureTypeP = DTMFeatureType::LatticeEdge ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::LatticeXLine") == 0 ) *dtmFeatureTypeP = DTMFeatureType::LatticeXLine ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::LatticeYLine") == 0 ) *dtmFeatureTypeP = DTMFeatureType::LatticeYLine ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::LatticePoint") == 0 ) *dtmFeatureTypeP = DTMFeatureType::LatticePoint ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::Contour") == 0 )      *dtmFeatureTypeP = DTMFeatureType::Contour   ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::ZeroSlopeLine")== 0) *dtmFeatureTypeP = DTMFeatureType::ZeroSlopeLine   ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::ISOLine") == 0 )      *dtmFeatureTypeP = DTMFeatureType::ISOLine  ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::ISOCell") == 0 )      *dtmFeatureTypeP = DTMFeatureType::ISOCell  ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::Theme") == 0 )         *dtmFeatureTypeP = DTMFeatureType::Theme   ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::SlopeToe") == 0 )     *dtmFeatureTypeP = DTMFeatureType::SlopeToe ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::LowPoint") == 0 )     *dtmFeatureTypeP = DTMFeatureType::LowPoint  ; 
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::HighPoint") == 0 )    *dtmFeatureTypeP = DTMFeatureType::HighPoint  ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::SumpLine") == 0 )     *dtmFeatureTypeP = DTMFeatureType::SumpLine   ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::RidgeLine") == 0 )    *dtmFeatureTypeP = DTMFeatureType::RidgeLine   ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::DescentTrace") == 0 ) *dtmFeatureTypeP = DTMFeatureType::DescentTrace ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::AscentTrace") == 0 )  *dtmFeatureTypeP = DTMFeatureType::AscentTrace ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::Catchment") == 0 )     *dtmFeatureTypeP = DTMFeatureType::Catchment  ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::LowPointPond") == 0 )*dtmFeatureTypeP = DTMFeatureType::LowPointPond ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::VisibleLine") == 0 )  *dtmFeatureTypeP = DTMFeatureType::VisibleLine ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::InvisibleLine") == 0 )*dtmFeatureTypeP = DTMFeatureType::InvisibleLine ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::SlopeLine") == 0 )    *dtmFeatureTypeP = DTMFeatureType::SlopeLine  ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::Polygon") == 0 )       *dtmFeatureTypeP = DTMFeatureType::Polygon     ;
    else if( strcmp(dtmFeatureTypeName,"DTMFeatureType::GradeSlope") == 0 )   *dtmFeatureTypeP = DTMFeatureType::GradeSlope  ;
    /*
    ** Set Return Value
    */
    if (*dtmFeatureTypeP == DTMFeatureType::None) ret = DTM_ERROR;
    /*
    ** Job Completed
    */
    return(ret) ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmData_testForValidDataObjectDtmFeatureType(DTMFeatureType dtmFeatureType)
    {
    int  ret=DTM_ERROR ;
    /*
    ** Test For Valid Data Object Dtm Feature Type
    */
    if( dtmFeatureType == DTMFeatureType::RandomSpots     ) ret = DTM_SUCCESS ;
    if( dtmFeatureType == DTMFeatureType::GroupSpots      ) ret = DTM_SUCCESS ;
    //TODO if( dtmFeatureType == DTMFeatureType::DatPoint       ) ret = DTM_SUCCESS ;
    if( dtmFeatureType == DTMFeatureType::Breakline      ) ret = DTM_SUCCESS ;
    if( dtmFeatureType == DTMFeatureType::SoftBreakline      ) ret = DTM_SUCCESS ;
    if( dtmFeatureType == DTMFeatureType::GraphicBreak   ) ret = DTM_SUCCESS ;
    if( dtmFeatureType == DTMFeatureType::ContourLine    ) ret = DTM_SUCCESS ; 
    if( dtmFeatureType == DTMFeatureType::Void            ) ret = DTM_SUCCESS ;
    if( dtmFeatureType == DTMFeatureType::VoidLine       ) ret = DTM_SUCCESS ; 
    if( dtmFeatureType == DTMFeatureType::Island          ) ret = DTM_SUCCESS ;
    if( dtmFeatureType == DTMFeatureType::Hole            ) ret = DTM_SUCCESS ;
    if( dtmFeatureType == DTMFeatureType::HoleLine       ) ret = DTM_SUCCESS ;
    if( dtmFeatureType == DTMFeatureType::BreakVoid      ) ret = DTM_SUCCESS ;
    if( dtmFeatureType == DTMFeatureType::DrapeVoid      ) ret = DTM_SUCCESS ;
    if( dtmFeatureType == DTMFeatureType::Hull            ) ret = DTM_SUCCESS ;
    if( dtmFeatureType == DTMFeatureType::HullLine       ) ret = DTM_SUCCESS ;
    if( dtmFeatureType == DTMFeatureType::Polygon         ) ret = DTM_SUCCESS ;
    if( dtmFeatureType == DTMFeatureType::Region          ) ret = DTM_SUCCESS ;
    /*
    ** Job Completed
    */
    return(ret) ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmData_testForValidTinObjectDtmFeatureType(DTMFeatureType dtmFeatureType) 
    {
    int ret=DTM_ERROR ;
    /*
    ** Test For Correct Tin Object Feature Type
    */
    if     ( dtmFeatureType == DTMFeatureType::RandomSpots        ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::GroupSpots         ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::TinPoint          ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::Breakline         ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::Hull               ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::ContourLine       ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::Void               ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::Island             ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::Hole               ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::Triangle           ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::TriangleEdge      ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::Polygon            ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::Region             ) ret = DTM_SUCCESS ;
    else if( dtmFeatureType == DTMFeatureType::ZeroSlopePolygon ) ret = DTM_SUCCESS ;
    /*
    ** Job Completed
    */
    return(ret) ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmData_testForValidLatticeObjectDtmFeatureType(DTMFeatureType dtmFeatureType) 
    {
    int ret=DTM_ERROR ;
    /*
    ** Test For Correct Lattice Object DTM Feature Type
    */
    if( dtmFeatureType == DTMFeatureType::LatticePoint ) ret = DTM_SUCCESS ;
    if( dtmFeatureType == DTMFeatureType::Lattice       ) ret = DTM_SUCCESS ;
    if( dtmFeatureType == DTMFeatureType::LatticeEdge  ) ret = DTM_SUCCESS ;
    if( dtmFeatureType == DTMFeatureType::LatticeXLine ) ret = DTM_SUCCESS ;
    if( dtmFeatureType == DTMFeatureType::LatticeYLine ) ret = DTM_SUCCESS ;
    /*
    ** Job Completed
    */
    return(ret) ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmData_copyParallelPointArrayPolygon
    (
    double    offset,                                  // ==> Copy offset
    DPoint3d       *p3dPtsP,                                // ==> Pointer To String Points
    long      numP3dPts,                               // ==> Number Of String Points
    DPoint3d       **parallelPtsPP,                         // <== Pointer To Parallel Points
    long      *numParallelPtsP                         // <== Number Of Parallel Points
    )
    /*
    ** This Function Parallel Copies a DPoint3d String
    **
    ** If The String Closes  A Positive offset Copies Copy Externally. A Negative offset Copies Copy Internally.
    ** If The String Is Open A Positive offset Copies Copy To The Right And A Negative offset Copies Copy To the Left
    **
    ** Author : Rob Cormack
    ** Date   : 11 March 2002
    ** 
    */
    {
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),sideof ;
    DTMDirection direction;
    long    numCopyPts,memPts=0,memPtsInc=100 ;
    double  pang,nang,bang,area,x,y,z ;
    DPoint3d     *p3d,*pp,*cp,*np,*copyPtsP=NULL ;
    double  ppTol=0.0001 ;
    /*
    ** Write Status Message
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Copying Parallel DPoint3d Polygon") ;
        bcdtmWrite_message(0,0,0,"offset         = %10.4lf",offset) ;
        bcdtmWrite_message(0,0,0,"p3dPtsP         = %p",p3dPtsP) ;
        bcdtmWrite_message(0,0,0,"numP3dPts      = %6ld",numP3dPts) ;
        for( p3d = p3dPtsP ; p3d < p3dPtsP + numP3dPts ; ++p3d )
            {
            bcdtmWrite_message(0,0,0,"Point[%6ld] = %10.4lf %10.4lf %8.4lf",(long)(p3d-p3dPtsP),p3d->x,p3d->y,p3d->z) ;
            }
        }
    /*
    ** Validate
    */
    if( p3dPtsP == NULL )   { bcdtmWrite_message(2,0,0,"Null String Points") ; goto errexit ; }
    if( *parallelPtsPP != NULL ) { bcdtmWrite_message(2,0,0,"Copy Parallel Points Not Null") ; goto errexit ; }
    if( numP3dPts < 2 ) { bcdtmWrite_message(2,0,0,"Less Than 2 Points In String") ; goto errexit ; }
    /*
    ** Remove Duplicate Points From String
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicate Points") ;
    for( pp = cp = p3dPtsP ; cp < p3dPtsP + numP3dPts ; ++cp )
        {
        if( pp->x != cp->x || pp->y != cp->y )
            {
            ++pp ;
            if( pp != cp ) *pp = *cp ;
            }
        }
    numP3dPts = (long) ( pp - p3dPtsP + 1) ;
    /*
    ** Check There Are More Than Two Points In Polygon
    */
    if( numP3dPts < 3 ) { bcdtmWrite_message(1,0,0,"Less Than 3 Points In Polygon") ; goto errexit ; }
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Number Of Points After Duplicate Removal = %6ld",numP3dPts) ;
        for( p3d = p3dPtsP ; p3d < p3dPtsP + numP3dPts ; ++p3d )
            {
            bcdtmWrite_message(0,0,0,"Point[%6ld] = %10.4lf %10.4lf %8.4lf",(long)(p3d-p3dPtsP),p3d->x,p3d->y,p3d->z) ;
            }
        }
    /*
    ** Test For Closure
    */
    if( p3dPtsP->x != (p3dPtsP+numP3dPts-1)->x || p3dPtsP->y != (p3dPtsP+numP3dPts-1)->y )
        { bcdtmWrite_message(1,0,0,"Polygon Does Not Close") ;  goto errexit ; }
    /*
    **  Set Polygon Anti Clockwise
    */
    bcdtmMath_getPolygonDirectionP3D(p3dPtsP,numP3dPts,&direction,&area) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Polygon direction = 1") ;
    if (direction == DTMDirection::Clockwise) bcdtmMath_reversePolygonDirectionP3D (p3dPtsP, numP3dPts);
    /*
    ** Scan Points And Copy Copy
    */
    numCopyPts = 0 ;
    pp = p3dPtsP + numP3dPts - 2 ;
    for( cp = p3dPtsP  ; cp < p3dPtsP + numP3dPts  ; ++cp )
        {
        np = cp + 1 ;
        if( np >= p3dPtsP + numP3dPts ) np = p3dPtsP + 1 ;  
        /*
        if( dbg ) bcdtmWrite_message(0,0,0,"Parallel offsetting Point %6ld ** %10.4lf %10.4lf %10.4lf",(long)(cp-p3dPtsP),cp->x,cp->y,cp->z) ;
        */
        pang = bcdtmMath_getAngle(cp->x,cp->y,pp->x,pp->y) ;
        nang = bcdtmMath_getAngle(cp->x,cp->y,np->x,np->y) ;
        sideof = bcdtmMath_sideOf(pp->x,pp->y,np->x,np->y,cp->x,cp->y) ;
        /*
        if( sideof <= INT_EPILSON ) sideof = 0 ;
        */
        /*
        ** Zero offset
        */
        if    ( offset == 0.0 )
            {
            x = cp->x ;
            y = cp->y ;
            z = cp->z ;
            if( bcdtmData_storePointP3D(x,y,z,&copyPtsP,&numCopyPts,&memPts,memPtsInc)) goto errexit ;
            }
        /*
        ** offset To Right Or External
        */
        else if( offset > 0.0 )
            {
            /*
            if( dbg ) bcdtmWrite_message(0,0,0,"offsetting To Right") ;
            */
            if( nang < pang ) nang += DTM_2PYE ;
            bang = ( pang + nang ) / 2.0 ; 
            if( nang > DTM_2PYE ) nang -= DTM_2PYE ;
            if( sideof < 0 )
                {
                x = cp->x + offset * cos(pang+DTM_PYE/2.0 ) ;
                y = cp->y + offset * sin(pang+DTM_PYE/2.0 ) ;
                z = cp->z ;
                if( bcdtmData_storePointP3D(x,y,z,&copyPtsP,&numCopyPts,&memPts,memPtsInc)) goto errexit ;
                x = cp->x + offset * cos(bang) ;
                y = cp->y + offset * sin(bang) ;
                z = cp->z ;
                if( bcdtmData_storePointP3D(x,y,z,&copyPtsP,&numCopyPts,&memPts,memPtsInc)) goto errexit ;
                x = cp->x + offset * cos(nang-DTM_PYE/2.0 ) ;
                y = cp->y + offset * sin(nang-DTM_PYE/2.0 ) ;
                z = cp->z ;
                if( bcdtmData_storePointP3D(x,y,z,&copyPtsP,&numCopyPts,&memPts,memPtsInc)) goto errexit ;
                }
            else if( sideof == 0 )
                {
                x = cp->x + offset * cos(bang) ;
                y = cp->y + offset * sin(bang) ;
                z = cp->z ;
                if( bcdtmData_storePointP3D(x,y,z,&copyPtsP,&numCopyPts,&memPts,memPtsInc)) goto errexit ;
                }
            else  if( sideof > 0 )
                {
                x = cp->x + offset * cos(pang+DTM_PYE/2.0 ) ;
                y = cp->y + offset * sin(pang+DTM_PYE/2.0 ) ;
                z = cp->z ;
                if( bcdtmData_storePointP3D(x,y,z,&copyPtsP,&numCopyPts,&memPts,memPtsInc)) goto errexit ;
                x = cp->x + offset * cos(nang-DTM_PYE/2.0 ) ;
                y = cp->y + offset * sin(nang-DTM_PYE/2.0 ) ;
                z = cp->z ;
                if( bcdtmData_storePointP3D(x,y,z,&copyPtsP,&numCopyPts,&memPts,memPtsInc)) goto errexit ;
                }
            }
        /*
        ** offset To Left Or Internal
        */
        else  if( offset < 0.0 )
            {
            if( dbg ) bcdtmWrite_message(0,0,0,"offsetting To Left ** sideof = %2d",sideof) ;
            if( pang < nang ) pang += DTM_2PYE ;
            bang = ( pang + nang ) / 2.0 ; 
            if( pang > DTM_2PYE ) pang -= DTM_2PYE ;
            if( dbg ) bcdtmWrite_message(0,0,0,"Pang = %12.8lf Bang = %12.8lf Nang = %12.8lf",pang,bang,nang) ;
            if( sideof < 0 )
                {
                x = cp->x - offset * cos(pang-DTM_PYE/2.0 ) ;
                y = cp->y - offset * sin(pang-DTM_PYE/2.0 ) ;
                z = cp->z ;
                if( bcdtmData_storePointP3D(x,y,z,&copyPtsP,&numCopyPts,&memPts,memPtsInc)) goto errexit ;
                x = cp->x - offset * cos(nang+DTM_PYE/2.0 ) ;
                y = cp->y - offset * sin(nang+DTM_PYE/2.0 ) ;
                z = cp->z ;
                if( bcdtmData_storePointP3D(x,y,z,&copyPtsP,&numCopyPts,&memPts,memPtsInc)) goto errexit ;
                }
            else if( sideof == 0 )
                {
                x = cp->x - offset * cos(bang) ;
                y = cp->y - offset * sin(bang) ;
                z = cp->z ;
                if( bcdtmData_storePointP3D(x,y,z,&copyPtsP,&numCopyPts,&memPts,memPtsInc)) goto errexit ;
                }
            else  if( sideof > 0 )
                {
                x = cp->x - offset * cos(pang-DTM_PYE/2.0 ) ;
                y = cp->y - offset * sin(pang-DTM_PYE/2.0 ) ;
                z = cp->z ;
                if( bcdtmData_storePointP3D(x,y,z,&copyPtsP,&numCopyPts,&memPts,memPtsInc)) goto errexit ;
                x = cp->x - offset * cos(bang) ;
                y = cp->y - offset * sin(bang) ;
                z = cp->z ;
                if( bcdtmData_storePointP3D(x,y,z,&copyPtsP,&numCopyPts,&memPts,memPtsInc)) goto errexit ;
                x = cp->x - offset * cos(nang+DTM_PYE/2.0 ) ;
                y = cp->y - offset * sin(nang+DTM_PYE/2.0 ) ;
                z = cp->z ;
                if( bcdtmData_storePointP3D(x,y,z,&copyPtsP,&numCopyPts,&memPts,memPtsInc)) goto errexit ;
                }
            }
        pp = cp ;
        }
    /*
    ** Write Copy Points
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Number Of Copy Points = %6ld",numCopyPts) ; 
        for( p3d = copyPtsP ; p3d < copyPtsP + numCopyPts ; ++p3d )
            {
            bcdtmWrite_message(0,0,0,"Point[%6ld] = %10.4lf %10.4lf %8.4lf",(long)(p3d-copyPtsP),p3d->x,p3d->y,p3d->z) ;
            }
        }
    /*
    ** Set Number Of Points
    */
    while ( numCopyPts > 0 && copyPtsP->x != (copyPtsP+numCopyPts-1)->x && copyPtsP->y != (copyPtsP+numCopyPts-1)->y ) --numCopyPts ;
    if( dbg ) 
        {
        bcdtmWrite_message(0,0,0,"Number Of Copy Points = %6ld",numCopyPts) ;
        bcdtmWrite_message(0,0,0,"Start Point = %10.4lf %10.4lf %10.4lf",copyPtsP->x,copyPtsP->y,copyPtsP->z) ;
        bcdtmWrite_message(0,0,0,"End   Point = %10.4lf %10.4lf %10.4lf",(copyPtsP+numCopyPts-1)->x,(copyPtsP+numCopyPts-1)->y,(copyPtsP+numCopyPts-1)->z) ;
        }
    /*
    ** Clean Polygon Points
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Cleaning Object Polygon Points") ;   
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Points Before Clean = %6ld",numCopyPts) ;
    if( bcdtmClean_internalPointArrayPolygon(&copyPtsP,&numCopyPts,ppTol)) goto errexit ;  
    if( bcdtmClean_externalPointArrayPolygon(&copyPtsP,&numCopyPts,ppTol)) goto errexit ;  
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Points After  Clean = %6ld",numCopyPts) ;
    *parallelPtsPP = copyPtsP ;
    *numParallelPtsP = numCopyPts ;
    copyPtsP = NULL ;
    /*
    ** Clean Up
    */
cleanup :
    if( copyPtsP != NULL ) { free(copyPtsP) ; copyPtsP = NULL ; }
    /*
    ** Job Completed
    */
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying Parallel DPoint3d Polygon Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying Parallel DPoint3d Polygon Error") ;
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmData_storePointP3D(double x,double y,double z,DPoint3d **Points,long *numPts,long *memPts,long memPtsInc) 
    {
 long dbg=DTM_TRACE_VALUE(0) ;
    /*
    ** Check For Memory Allocation
    */
    if( *numPts == *memPts )
        {
        *memPts = *memPts + memPtsInc ;
        if( *Points == NULL ) *Points = (DPoint3d * ) malloc ( *memPts * sizeof(DPoint3d)) ;
        else                  *Points = (DPoint3d * ) realloc( *Points,*memPts * sizeof(DPoint3d)) ;
        if( *Points == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
        }
    /*
    ** Store Point
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Storing Point %10.4lf %10.4lf %10.4lf",x,y,z) ;
    (*Points+*numPts)->x = x ;
    (*Points+*numPts)->y = y ;
    (*Points+*numPts)->z = z ;
    ++*numPts ;
    /*
    ** Job Completed
    */
    return(0) ;
    /*
    ** Error Exit
    */
errexit :
    return(1) ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmData_checkPolygonForKnots(DPoint3d *PolygonumPolyPts,long NumPolygonumPolyPts,long *KnotFlag)
    /*
    **
    ** This Function Validates A Polygon 
    **
    **         = 1   Knot In Polygon
    **         = 2   Coincident Point
    **         = 1|2 Both Knot In Polygon and Coincident Point
    **
    */
    {
 long   ofs,dbg=DTM_TRACE_VALUE(0) ;
    double Xi,Yi ;
    DPoint3d    *p3d1,*p3d2 ;
    /*
    ** Check For Coincident Points On Polygon
    */
    *KnotFlag = 0 ;
    for( p3d1 = PolygonumPolyPts ; p3d1 < PolygonumPolyPts + NumPolygonumPolyPts - 1 ; ++p3d1 )
        {
        for( p3d2 = p3d1 + 1 ; p3d2 < PolygonumPolyPts + NumPolygonumPolyPts - 1 ; ++p3d2 )
            {
            if( p3d1->x == p3d2->x && p3d1->y == p3d2->y ) 
                {
                if( dbg ) bcdtmWrite_message(0,0,0,"Coincident Point Knot At %10.4lf %10.4lf %10.4lf",p3d1->x,p3d1->y,p3d1->z) ; 
                *KnotFlag |= 2 ; 
                }
            }  
        }
    /*
    ** Check For Crossing Polygon Lines
    */
    ofs = 2 ;
    for( p3d1 = PolygonumPolyPts ; p3d1 < PolygonumPolyPts + NumPolygonumPolyPts - 2 ; ++p3d1 )
        {
        for( p3d2 = p3d1 + 2 ; p3d2 < PolygonumPolyPts + NumPolygonumPolyPts - ofs ; ++p3d2 )
            {
            if(bcdtmMath_intersectCordLines(p3d1->x,p3d1->y,(p3d1+1)->x,(p3d1+1)->y,p3d2->x,p3d2->y,(p3d2+1)->x,(p3d2+1)->y,&Xi,&Yi))  
                { 
                if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting Lines Knot At %10.4lf %10.4lf",Xi,Yi) ; 
                *KnotFlag |= 1 ; 
                }
            }  
        ofs = 1 ; 
        }
    /*
    ** Job Completed
    */
    return(0) ;
    }
