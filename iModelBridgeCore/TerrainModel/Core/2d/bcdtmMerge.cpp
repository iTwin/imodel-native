/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmMerge.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
#include "bcdtminlines.h" 

thread_local static double areaPerimeterRatio = 1.0;
thread_local static double minImpliedVoidArea = 5.0;
/*==============================================================================*//**
* @memo   Merge Dtm Files
* @doc    Merge Dtm Files 
* @notes  Merges Dtm File 2 Into Dtm File 1 and writes the merged Dtm File as Dtm File 3
* param dtmFile1P      ==>  Dtm File Name To Be Merged Into
* param dtmFile2P      ==>  Dtm File Name To Be Merged 
* param dtmFile3P      ==>  Dtm File Name To Be Created For The Merged File 
* @return DTM_SUCCESS or DTM_ERROR
* @author Rob Cormack   rob.cormack@bentley.con
* @version 
* @see None
*===============================================================================*/
BENTLEYDTM_EXPORT int bcdtmMerge_dtmFiles
(
 WCharCP dtmFile1P,      /* ==>  Dtm File Name To Be Merged Into             */
 WCharCP dtmFile2P,      /* ==>  Dtm File Name To Be Merged                  */
 WCharCP dtmFile3P       /* ==>  Dtm File To Be Created For The Merged Files */
)
/*
** This Function Merges Dtm Files
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 BC_DTM_OBJ *dtm1P=NULL,*dtm2P=NULL ;
/*
** Write Status Message
*/
 if( dbg )
   { 
    bcdtmWrite_message(0,0,0,"Merging Dtm Files") ; 
    bcdtmWrite_message(0,0,0,"Dtm File1 = %s",dtmFile1P) ;
    bcdtmWrite_message(0,0,0,"Dtm File2 = %s",dtmFile2P) ;
    bcdtmWrite_message(0,0,0,"Dtm File3 = %s",dtmFile3P) ;
   }
/*
** Check File Name Three Has Been Provided
*/
 if( *dtmFile3P == 0 )
   {
    bcdtmWrite_message(1,0,0,"Merge File Name Not Provided") ;
    goto errexit ;
   }
/*
** Check For Different Files
*/
 if( wcscmp(dtmFile1P,dtmFile2P) == 0 ) 
   { 
    bcdtmWrite_message(1,0,0,"Dtm Files The Same") ; goto errexit ; 
   } 
/*
** Read Dtm Files Into Memory
*/
 if( bcdtmRead_fromFileDtmObject(&dtm1P,dtmFile1P)) goto errexit ;
 if( bcdtmRead_fromFileDtmObject(&dtm2P,dtmFile2P)) goto errexit ; 
/*
** Check For Old Dtm Files
*/
 if( dtm1P->ppTol == 0.0 ) { bcdtmUtl_setCurrentTinFileName(dtmFile1P) ; bcdtmWrite_message(1,0,0,"Convert Dtm File %s",dtmFile1P) ; bcdtmObject_destroyDtmObject(&dtm1P) ; bcdtmObject_destroyDtmObject(&dtm2P) ; return(20) ; }
 if( dtm2P->ppTol == 0.0 ) { bcdtmUtl_setCurrentTinFileName(dtmFile2P) ; bcdtmWrite_message(1,0,0,"Convert Dtm File %s",dtmFile2P) ; bcdtmObject_destroyDtmObject(&dtm1P) ; bcdtmObject_destroyDtmObject(&dtm2P) ; return(20) ; }
 if( dtm1P->plTol == 0.0 ) dtm1P->plTol = dtm1P->ppTol ;
 if( dtm2P->plTol == 0.0 ) dtm2P->plTol = dtm2P->ppTol ;
/*
** Merge Dtm Objects
*/
 if( bcdtmMerge_dtmObjects(dtm1P,dtm2P,0)) goto errexit ;
/*
** Set Currency
*/
 if( bcdtmUtility_setCurrentDtmObject(dtm1P,dtmFile3P)) goto errexit ; 
/*
** Write Dtm File
*/
 if( bcdtmWrite_toFileDtmObject(dtm1P,dtmFile3P)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
 if( dtm2P != NULL ) bcdtmObject_destroyDtmObject(&dtm2P) ; 
/*
** Return
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( dtm1P != NULL ) bcdtmObject_destroyDtmObject(&dtm1P) ; 
 if( dtm2P != NULL ) bcdtmObject_destroyDtmObject(&dtm2P) ; 
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmMerge_addVoidAreaDtmObject (
 BC_DTM_OBJ *dtm1P,       /* ==> Pointer To dtm1P                                */
 BC_DTM_OBJ *dtm2P,       /* ==> Pointer To dtm2P                                */
 long* internalFlagP      /* ==> Designates if dtm2P is internal to dtm1P <0,1,2> */
)
{
 int  ret=DTM_SUCCESS ;
 long numHullPts=0;
 DPoint3d* hullPtsP=nullptr;
 BC_DTM_OBJ *tempDtmP = nullptr;
 DTMFeatureId featureId;
/*
** Initialise
*/
 *internalFlagP = 0 ;

 /*
  * Create temp DTM.
  */
 if( bcdtmObject_createDtmObject(&tempDtmP))  goto errexit ;
/*
** Extract Dtm Hull From Dtm Objects
*/
 if( bcdtmList_extractHullDtmObject(dtm1P,&hullPtsP,&numHullPts)) goto errexit ;
 if (bcdtmObject_storeDtmFeatureInDtmObject (tempDtmP, DTMFeatureType::Void, DTM_NULL_USER_TAG, 3, &featureId,hullPtsP, numHullPts)) goto errexit;
 if( hullPtsP != NULL ) {free(hullPtsP) ; hullPtsP = NULL; }

 if( bcdtmList_extractHullDtmObject(dtm2P,&hullPtsP,&numHullPts)) goto errexit ;
 if (bcdtmObject_storeDtmFeatureInDtmObject (tempDtmP, DTMFeatureType::Void, DTM_NULL_USER_TAG, 3, &featureId,hullPtsP, numHullPts)) goto errexit;
 if( hullPtsP != NULL ) {free(hullPtsP) ; hullPtsP = NULL; }
/*
** Check Intersection
*/
 if ( bcdtmObject_triangulateDtmObject (tempDtmP)) goto errexit;

/*
 * Remove all void on the boundary, eg leaving what is between the two dtms
 */
 if ( bcdtmEdit_removeInsertedVoidsOnTinHullDtmObject (tempDtmP, 0)) goto errexit;
/*
 * Extract the hull which is the area between the two.
 */
 if( bcdtmList_extractHullDtmObject(tempDtmP,&hullPtsP,&numHullPts)) goto errexit ;
/*
 * Delete the temporary dtm and create a new one with a void between the two.
 */
 if( tempDtmP != NULL ) bcdtmObject_destroyDtmObject(&tempDtmP) ;
 if( bcdtmObject_createDtmObject(&tempDtmP))  goto errexit ;
 if (bcdtmObject_storeDtmFeatureInDtmObject (tempDtmP, DTMFeatureType::Void, DTM_NULL_USER_TAG, 3, &featureId,hullPtsP, numHullPts)) goto errexit;
 if( hullPtsP != NULL ) {free(hullPtsP) ; hullPtsP = NULL; }

 if ( bcdtmObject_triangulateDtmObject (tempDtmP)) goto errexit;
/*
 * Merge this void dtm to the first dtm.
 */
 if ( bcdtmMerge_dtmObjects (dtm1P, tempDtmP, 2, true)) goto errexit;
/*
 * Update the internalFlag.
 */
 if( bcdtmMerge_checkTinsIntersectDtmObject(dtm1P,dtm2P,internalFlagP)) goto errexit  ;
/*
** Clean Up
*/
 cleanup :
 if( hullPtsP != NULL ) free(hullPtsP) ;
 if( tempDtmP != NULL ) bcdtmObject_destroyDtmObject(&tempDtmP) ;
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

/*==============================================================================*//**
* @memo   Merge Dtm Objects
* @doc    Merge Dtm Objects 
* @notes  Merges dtm2P Into dtm1P
* @notes  interflag designates if dtm2P is totally or partially internal to dtm1P
* @notes  If dtm2P is totally internal to dtm1P set internalFlag to 1
* @notes  If dtm2P is both external and internal to dtm1P set internalFlag to 2       
* @notes  If unsure set internalFlag to 0 and the merge function will determine.
* @param  dtm1P             ==> Pointer To dtm1P Object        
* @param  dtm2P             ==> Pointer To dtm2P Object        
* @param  internalFlag      ==> Designates if dtm2P is internal to dtm1P <0,1,2>         
* @return DTM_SUCCESS or DTM_ERROR
* @author Rob Cormack rob.cormack@bentley.con
* @version 
* @see None
*===============================================================================*/
BENTLEYDTM_EXPORT int bcdtmMerge_dtmObjects
(
 BC_DTM_OBJ *dtm1P,       /* ==> Pointer To dtm1P                                */
 BC_DTM_OBJ *dtm2P,       /* ==> Pointer To dtm2P                                */
 long internalFlag,        /* ==> Designates if dtm2P is internal to dtm1P <0,1,2> */
 bool failOnNonIntersecting
)
/*
** Initialise
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0),tdbg=DTM_TIME_VALUE(0),fifodbg=DTM_TRACE_VALUE(0) ;
 long startPnt,numDtm2HullPts=0 ; 
 long mstart,start=0 ;
 DPoint3d  *tin2hullPtsPP=NULL ;
 static long  mrgSeqNum=1 ;
 wchar_t *cP,mergeFile[FILENAME_MAX + 1]  ;
/*
** Write Status Message
*/
// bcdtmWrite_message(0,0,0,"Merging Dtm Objects") ;
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Merging Dtm Objects") ;
    bcdtmWrite_message(0,0,0,"dtm1P        = %p",dtm1P) ;
    bcdtmWrite_message(0,0,0,"dtm2P        = %p",dtm2P) ;
    bcdtmWrite_message(0,0,0,"internalFlag = %8ld",internalFlag) ;
   } 
/*
** Test For Valid DTM Objects
*/
 if( bcdtmObject_testForValidDtmObject(dtm1P)) goto errexit  ;
 if( bcdtmObject_testForValidDtmObject(dtm2P)) goto errexit  ;
 if( dtm1P->dtmState != DTMState::Tin || dtm2P->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Write Dtm Objects To File While FIFO Processing For Debugging Purposes
*/
 mstart = bcdtmClock() ;
 if( fifodbg )
   {
    mergeFile[0] = 0 ;
    swprintf(mergeFile,128,L"mrg%4ld_1.tin",mrgSeqNum) ;
    cP = mergeFile ;
    while( *cP != 0 ) { if( *cP == ' ' ) *cP = '0' ; ++cP ; }
    bcdtmWrite_toFileDtmObject(dtm1P,mergeFile) ; 
    mergeFile[0] = 0 ;
    swprintf(mergeFile,128,L"mrg%4ld_2.tin",mrgSeqNum );
    cP = mergeFile ;
    while( *cP != 0 ) { if( *cP == ' ' ) *cP = '0' ; ++cP ; }
    bcdtmWrite_toFileDtmObject(dtm2P,mergeFile) ; 
    ++mrgSeqNum ;
   }
/*
** Check For Old Dtm Files
*/
 if( dtm1P->ppTol == 0.0 || dtm2P->ppTol == 0.0 ) 
   {
    bcdtmWrite_message(1,0,0,"Convert Dtm File(s)") ;
    goto errexit ;
   }
/*
** A Fix For Old Dtm files Where plTol Was Not Set
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"dtm1P ** ppTol = %20.15lf plTol = %20.15lf mppTol = %20.15lf",dtm1P->ppTol,dtm1P->plTol,dtm1P->mppTol) ;
    bcdtmWrite_message(0,0,0,"dtm2P ** ppTol = %20.15lf plTol = %20.15lf mppTol = %20.15lf",dtm2P->ppTol,dtm2P->plTol,dtm2P->mppTol) ;
   }
 if( dtm1P->plTol != dtm1P->ppTol )  dtm1P->plTol = dtm1P->ppTol ;
 if( dtm2P->plTol != dtm2P->ppTol )  dtm2P->plTol = dtm2P->ppTol ;
/*
** Write Dtm Statistics
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Dtm  - 1") ;
    bcdtmUtility_writeStatisticsDtmObject(dtm1P) ;
    bcdtmWrite_message(0,0,0,"Dtm  - 2") ;
    bcdtmUtility_writeStatisticsDtmObject(dtm2P) ;
    bcdtmWrite_message(0,0,0,"dtm1P ** ppTol = %20.15lf plTol = %20.15lf mppTol = %20.15lf",dtm1P->ppTol,dtm1P->plTol,dtm2P->mppTol) ;
    bcdtmWrite_message(0,0,0,"dtm2P ** ppTol = %20.15lf plTol = %20.15lf mppTol = %20.15lf",dtm2P->ppTol,dtm2P->plTol,dtm2P->mppTol) ;
   }
/*
** Check Tins 
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Tin Component dtm1P") ;
    if( bcdtmCheck_tinComponentDtmObject(dtm1P)) goto errexit  ;
    bcdtmWrite_message(0,0,0,"Checking Tin Component dtm2P") ;
    if( bcdtmCheck_tinComponentDtmObject(dtm2P)) goto errexit  ;
   }
/*
** Set Tolerances For Merge
*/
 if( dtm1P->ppTol > dtm2P->ppTol ) { dtm1P->ppTol = dtm1P->plTol = dtm2P->ppTol ; }
 else                              { dtm2P->ppTol = dtm2P->plTol = dtm1P->ppTol ; }  
 if( dtm1P->ppTol < dtm1P->mppTol * 10000.0 ) dtm1P->ppTol = dtm1P->plTol = dtm1P->mppTol * 10000.0 ;                         
/*
** Log Tolerances
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Adjusted Merge Tolerances") ;
    bcdtmWrite_message(0,0,0,"dtm1P ** ppTol = %16.14lf plTol = %16.14lf mppTol = %16.14lf",dtm1P->ppTol,dtm1P->plTol,dtm1P->mppTol) ;
    bcdtmWrite_message(0,0,0,"dtm2P ** ppTol = %16.14lf plTol = %16.14lf mppTol = %16.14lf",dtm2P->ppTol,dtm2P->plTol,dtm2P->mppTol) ;
   } 
/*
** Clean DTM Features ** Fix For Older Dtm Versions
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Cleaning DTM Features") ;
 bcdtmList_cleanDtmFeatureListsDtmObject(dtm1P) ;
 bcdtmList_cleanDtmFeatureListsDtmObject(dtm2P) ;
/*
**  Remove Tin Error And Roll Back Features
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Error And Rollback Features") ;
 if( bcdtmData_deleteAllTinErrorFeaturesDtmObject(dtm1P)) goto errexit ;
 if( bcdtmData_deleteAllRollBackFeaturesDtmObject(dtm1P)) goto errexit ;
 if( bcdtmList_cleanDtmObject(dtm1P)) goto errexit ;
/*
** Check Tins Intersect
*/
 if( ! internalFlag )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Tins Intersect") ;
    if( dbg ) start = bcdtmClock() ;
    if( bcdtmMerge_checkTinsIntersectDtmObject(dtm1P,dtm2P,&internalFlag)) goto errexit  ;
    if( internalFlag == 0 && !failOnNonIntersecting)
        {
        if (bcdtmMerge_addVoidAreaDtmObject (dtm1P, dtm2P, &internalFlag)) goto errexit;
        }
    if( internalFlag == 0 ) { bcdtmWrite_message(1,0,0,"Merge Tins Do Not Intersect") ; goto errexit  ; }
    if( internalFlag == 3 ) { bcdtmWrite_message(1,0,0,"Merge Dtm Encloses Old Tin") ; goto errexit  ; }
    if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Check Tins Intersect = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),start)) ; 
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Internal Flag = %2ld",internalFlag) ;
/*
** Check For And Extract Holes From dtm1P
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For And Merging Holes") ;
 if( bcdtmMerge_insertHolesIntoDtmObject(dtm1P,dtm2P,internalFlag)) goto errexit  ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For And Merging Holes Comleted") ;
/*
** Extract dtm2P Hull into DPoint3d Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Extracting dtm2P Hull") ;
 if( bcdtmList_extractHullDtmObject(dtm2P,&tin2hullPtsPP,&numDtm2HullPts)) goto errexit  ;
/*
** Internally Clip Update Tin
*/
 start = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Internally Clipping Update Dtm") ;
 if( bcdtmMerge_internallyClipDtmObject(dtm1P,internalFlag,tin2hullPtsPP,numDtm2HullPts,&startPnt)) goto errexit  ; 
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Internally Clip Dtm = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),start)) ;
/*
** Insert Merge Dtm Into Update 
*/
 start = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Merge Dtm") ;
 if( bcdtmMerge_insertMergeDtmObject(dtm1P,dtm2P,internalFlag,startPnt)) goto errexit  ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Insert Merge Dtm = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),start)) ; 
/*
** Write Merged Dtm Statistics
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Merged Dtm") ;
    bcdtmUtility_writeStatisticsDtmObject(dtm1P) ;
    bcdtmWrite_message(0,0,0,"dtm1P ** ppTol = %15.10lf plTol = %15.10lf",dtm1P->ppTol,dtm1P->plTol) ;
   }
/*
** Check Merged Dtm 
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Merged Tin") ;
    if( bcdtmCheck_tinComponentDtmObject(dtm1P)) goto errexit  ;
   } 
/*
**  Update Modified Time
 */
 bcdtmObject_updateLastModifiedTime (dtm1P) ;
/*
** Write Status Message
*/
 cleanup :
 if( dtm2P->dtmState == DTMState::Tin ) bcdtmList_reportAndSetToNullNoneNullSptrValuesDtmObject(dtm2P,0) ;
 if( tin2hullPtsPP != NULL ) free(tin2hullPtsPP) ;
/*
** Job Completed
*/
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Merge Tins = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),mstart)) ; 
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Merging Dtm Objects Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Merging Dtm Objects Error") ;
// if( ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Merging Dtm Objects Completed") ;
// if( ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Merging Dtm Objects Error") ;
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
BENTLEYDTM_EXPORT int  bcdtmMerge_checkTinsIntersectDtmObject(BC_DTM_OBJ *dtm1P,BC_DTM_OBJ *dtm2P,long *internalFlagP)
/*
** This Function Tests The Intersection Of dtm2P Hull With dtm1P Hull
** internalFlag = 0  No Intersection
** internalFlag = 1  Dtm Hulls Intersect
** internalFlag = 2  dtm2P Hull Totally Within dtm1P Hull
** internalFlag = 3  dtm1P Hull Totally Within dtm2P HULL
*/
{
 int  ret=DTM_SUCCESS ;
 long numHullPts1=0,numHullPts2=0,intersectFlag ;
 DPoint3d  *hullPts1P=NULL,*hullPts2P=NULL ;
/*
** Initialise 
*/
 *internalFlagP = 0 ;
/*
** Extract Dtm Hull From Dtm Objects
*/
 if( bcdtmList_extractHullDtmObject(dtm1P,&hullPts1P,&numHullPts1)) goto errexit ;
 if( bcdtmList_extractHullDtmObject(dtm2P,&hullPts2P,&numHullPts2)) goto errexit ;
/*
** Check Intersection
*/
 if( bcdtmClip_checkPolygonsIntersect(hullPts1P,numHullPts1,hullPts2P,numHullPts2,&intersectFlag) ) goto errexit ;
 if     ( intersectFlag == 0 ) *internalFlagP = 0 ;
 else if( intersectFlag == 1 ) *internalFlagP = 2 ;
 else if( intersectFlag == 2 ) *internalFlagP = 3 ;
 else if( intersectFlag == 3 ) *internalFlagP = 1 ;
/*
** Clean Up
*/
 cleanup :
 if( hullPts1P != NULL ) free(hullPts1P) ;
 if( hullPts1P != NULL ) free(hullPts2P) ; 
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
BENTLEYDTM_Private int bcdtmMerge_insertHolesIntoDtmObject(BC_DTM_OBJ *dtm1P,BC_DTM_OBJ *dtm2P,long internalFlag)
/*
** This Function Extracts The Holes From dtm1P And Stores Them In Dtm Objects
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long  dtmFeature,numHullPts,numHoles,numHolePts,holeInternalFlag ;
 DPoint3d   *p3dP,*hullPtsP=NULL,*holePtsP=NULL ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Holes Into Dtm Object %p",dtm1P) ;
/*
** Initialise
*/
 numHoles = 0 ;
/*
** Count Number Of Holes
*/
 for( dtmFeature = 0 ; dtmFeature < dtm2P->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtm2P,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeaturePts.firstPoint != dtm2P->nullPnt && dtmFeatureP->dtmFeatureType == DTMFeatureType::Hole )
      {
       ++numHoles ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Feature = %4ld Type = %4ld DTMFeatureType::Hole = %4ld Dtm Feature State = %2ld",dtmFeature,dtmFeatureP->dtmFeatureType,DTMFeatureType::Hole,dtmFeatureP->dtmFeatureState) ;
      }
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Holes = %8ld",numHoles) ; 
/*
** Only Process If Holes Present
*/
 if( numHoles )
   {    
/*
**  Extract Dtm Hull To DPoint3d Array
*/
    if( bcdtmList_extractHullDtmObject(dtm1P,&hullPtsP,&numHullPts)) goto errexit ;
/*
**  Scan Features For Holes
*/ 
    for( dtmFeature = 0 ; dtmFeature < dtm2P->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtm2P,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeaturePts.firstPoint != dtm2P->nullPnt && dtmFeatureP->dtmFeatureType == DTMFeatureType::Hole )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"** Processing Hole Feature = %4ld",dtmFeature) ;
/*
**        Extract Hole To Point Array
*/
          if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtm2P,dtmFeature,&holePtsP,&numHolePts)) goto errexit ;
/*
**        Write Hole Points
*/
          if( dbg == 2 )
            { 
             bcdtmWrite_message(0,0,0,"Number Of Hole Points = %8ld",numHolePts) ;
             for( p3dP = holePtsP ; p3dP < holePtsP + numHolePts ; ++p3dP )       
               {
                bcdtmWrite_message(0,0,0,"holePoint[%6ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-holePtsP),p3dP->x,p3dP->y,p3dP->z) ;
               }
            }
/*
**        Check Hole Is Internal To dtm1P Hull
*/
          if( internalFlag == 1 ) holeInternalFlag = 1 ;
          else
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Checking Hole Is Internal To Dtm") ;
             if( bcdtmMerge_checkHoleInternalToDtmHull(hullPtsP,numHullPts,holePtsP,numHolePts,&holeInternalFlag) ) goto errexit ;
            } 
          if( dbg ) bcdtmWrite_message(0,0,0,"holeInternalFlag = %2ld",holeInternalFlag) ;
/*
**       Set Hole To Void If Hole Not Totally Internal To dtm1P
*/
          if( ! holeInternalFlag ) dtmFeatureP->dtmFeatureType = DTMFeatureType::Void ;
          else
            {
/*
**           Remove Hole Feature From dtm2P
*/
             if( dbg ) bcdtmWrite_message(0,0,0,"Removing Hole From DTM2") ;
             if( bcdtmInsert_removeDtmTinFeatureFromDtmObject(dtm2P,dtmFeature)) goto errexit ;
/*
**           Extract Hole Data From dtm1P And Insert Into dtm2P
*/ 
             if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Hole Into DTM1") ;
             if( bcdtmMerge_extractAndInsertHoleDtmObject(dtm1P,dtm2P,holePtsP,numHolePts)) goto errexit ;
/*
**           Check Hole Has Been Inserted Correctly
*/
             if( cdbg )
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"Checking Tin") ;
                if( bcdtmCheck_tinComponentDtmObject(dtm2P) ) 
                  {
                   if( dbg )bcdtmWrite_message(0,0,0,"Tin Invalid") ;
                   goto errexit ;
                  }
                if( dbg ) bcdtmWrite_message(0,0,0,"Tin Valid") ;
               }
/*
**           Reset Scan To Start Because Of Clean Operation
*/
             dtmFeature = -1 ;
            } 
/*
**        Free Hole Points Memory
*/
          free(holePtsP) ; holePtsP = NULL ;  
     
         }
      }
   } 
/*
** Flag Holes Detected
*/
 if( dbg && numHoles )
   {
    bcdtmWrite_message(0,0,0,"Holes Detected And Inserted") ;
    bcdtmWrite_toFileDtmObject(dtm2P,L"holesInserted.dtm") ;
   }
/*
** Clean Up
*/
 cleanup :
 if( hullPtsP != NULL ) free(hullPtsP) ;
 if( holePtsP != NULL ) free(holePtsP) ; 
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
BENTLEYDTM_Public int bcdtmMerge_checkHoleInternalToDtmHull(DPoint3d *hullPtsP,long numHullPts,DPoint3d *holePtsP,long numHolePts,long *holeInternalFlag)
/*
** This Function Get The Intesection Polygon of Two Polygons
** Return Values for holeInternalFlag == 1 Hole Internal To Hull
**                                    == 0 Hole Not Internal To Hull
** Notes
**  1. Direction Of hullPtsP & holePtsP must be anticlockwise
*/
{
 DPoint3d    *p3dP1,*p3dP2 ;
 double Xi,Yi ;
/*
** Initialise Values
*/
 *holeInternalFlag = 0 ;
/*
** Check For Coincident Points 
*/
 for( p3dP1 = hullPtsP ; p3dP1 < hullPtsP + numHullPts ; ++p3dP1 )
   {
    for( p3dP2 = holePtsP ; p3dP2 < holePtsP + numHolePts ; ++p3dP2 )
      {
       if( p3dP1->x == p3dP2->x && p3dP1->y == p3dP2->y ) return(0) ;
      }
   }
/*
** Check For Intersecting Lines
*/
 for( p3dP1 = hullPtsP ; p3dP1 < hullPtsP + numHullPts - 1 ; ++p3dP1 )
   {
    for( p3dP2 = holePtsP ; p3dP2 < holePtsP + numHolePts - 1 ; ++p3dP2 )
      {
       if(bcdtmMath_intersectCordLines(p3dP1->x,p3dP1->y,(p3dP1+1)->x,(p3dP1+1)->y,p3dP2->x,p3dP2->y,(p3dP2+1)->x,(p3dP2+1)->y,&Xi,&Yi)) return(0) ;
      }
   }
/*
**  Check Hole Is Internal To Hull
*/
 if( bcdtmClip_pointInPointArrayPolygon(hullPtsP,numHullPts,holePtsP->x,holePtsP->y )) *holeInternalFlag = 1 ;
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmMerge_extractAndInsertHoleDtmObject(BC_DTM_OBJ *dtm1P,BC_DTM_OBJ *dtm2P,DPoint3d *holePtsP,long numHolePts) 
/*
** This Function Extracts The Hole Data From dtm1P and Stores The Hole Data In dtm2P
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   startPnt,numHullPts ;
 DPoint3d    *hullPtsP=NULL ;
 BC_DTM_OBJ *dtmP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Hole Data Into Tin") ;
/*
** Clone dtm1P Object
*/
 if( bcdtmObject_cloneDtmObject(dtm1P,&dtmP)) goto errexit ;
/*
**  Insert Hole Points Into Tin
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Hole Points Into Tin") ;
 if( bcdtmInsert_rigidInternalStringIntoDtmObject(dtmP,2,2,holePtsP,numHolePts,&startPnt))  goto errexit ;
/*
** Check Connectivity Of Inserted Hole Points Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Connectivity Of Inserted Hole Points") ;
 if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,startPnt,0))  goto errexit ;
/*
** Clip Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Clipping Tin External To Hole") ;
 if( bcdtmClip_externalToTptrPolygonDtmObject(dtmP,startPnt) )  goto errexit ;
/*
** Check Clipped Tin
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Cleaning Tin") ;
 if( bcdtmList_cleanDtmObject(dtmP)) goto errexit ;
/*
** Check Clipped Dtm ** Development Only
*/
 if( cdbg )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Triangles") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP) ) goto errexit ;
   }
/*
** Extract Clipped Dtm Hull
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Extracting Clipped DTM Hull") ;
 if( bcdtmList_extractHullDtmObject(dtmP,&hullPtsP,&numHullPts)) goto errexit ;
/*
**  Insert Hole Points Into Tin
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Hole Points Into Tin") ;
 if( bcdtmInsert_colinearStringDtmObject(dtm2P,hullPtsP,numHullPts,&startPnt))  goto errexit ;
/*
** Check Connectivity Of Inserted Hole Points Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Connectivity Of Tptr Polygon") ;
 if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtm2P,startPnt,0))  goto errexit ; 
/*
** Internally Clip Hole Triangles
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Internally Clipping Hole Polygon") ;
 if( bcdtmClip_internalToTptrPolygonDtmObject(dtm2P,startPnt,0)) goto errexit ;
/*
** Merge Hole Dtm Into dtm2P Hole
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Merge DTM") ;
 if( bcdtmMerge_insertMergeDtmObject(dtm2P,dtmP,1,startPnt)) goto errexit ;
/*
** Check Merged Hole ** Development Only
*/
 if( cdbg )if( bcdtmCheck_tinComponentDtmObject(dtm2P) ) goto errexit ;
/*
** Clean Up
*/
 cleanup :
 if( dtmP     != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ;
 if( hullPtsP != NULL ) free(hullPtsP) ;
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Inserting Hole Data Into Dtm Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Inserting Hole Data Into Dtm Error") ;
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
BENTLEYDTM_Private int bcdtmMerge_internallyClipDtmObject
(
 BC_DTM_OBJ *dtmP,            /* ==> Pointer To Dtm Object          */
 long internalFlag,           /* ==> internalFlag <1,2>             */
 DPoint3d *clipPtsP,               /* ==> Pointer To Polygon Point Array */
 long numClipPts,             /* ==> Number Of Poygon Points        */
 long *firstPntP              /* <== First Point On Inserted Clip Tptr Polygon */
)
/*
**
** Notes :-
** If Internal Flag == 1 The clip polygon is totally internal to the Tin
**                  == 2 The clip polgon is both internal and external to the Dtm 
**
*/
{
 int      ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long     sp,np,startPnt=0,dtmFeature,fpts,numPolyPts,saveIncPoints ;
 DTMDirection direction;
 double   polyArea=0.0,insertArea=0.0 ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg )
   { 
    bcdtmWrite_message(0,0,0,"Internally Clipping Dtm Object") ;
    bcdtmWrite_message(0,0,0,"dtmP         = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"internalFlag = %8ld",internalFlag) ;
    bcdtmWrite_message(0,0,0,"clipPtsP     = %p",clipPtsP) ;
    bcdtmWrite_message(0,0,0,"numClipPts   = %6ld",numClipPts) ; 
   }
/*
** Perform Checks
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Dtm Points  = %20ld",dtmP->numPoints) ;
    bcdtmMath_getPolygonDirectionP3D(clipPtsP,numClipPts,&direction,&polyArea) ;
    bcdtmWrite_message(0,0,0,"Clipping Polygon Area = %20.10lf",polyArea) ;
    bcdtmWrite_message(0,0,0,"Checking Dtm Feature Topology") ;
    if( bcdtmCheck_topologyDtmFeaturesDtmObject(dtmP,1) ){  bcdtmWrite_message(0,0,0,"Feature Topology Invalid") ; goto errexit ; }
    else                                                    bcdtmWrite_message(0,0,0,"Feature Topology Valid") ;
   }
/*
** Insert Polygon Into Tin
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Merge Dtm Hull Into Update Tin") ;
 saveIncPoints = dtmP->incPoints ;
 if( numClipPts * 5 > dtmP->incPoints ) dtmP->incPoints = numClipPts * 5 ;
 if( internalFlag == 1 ) if( bcdtmInsert_rigidInternalStringIntoDtmObject(dtmP,2,2,clipPtsP,numClipPts,&startPnt))  goto errexit ;
 if( internalFlag == 2 ) if( bcdtmInsert_rigidExternalStringIntoDtmObject(dtmP,2,2,clipPtsP,numClipPts,&startPnt))  goto errexit ;
 dtmP->incPoints = saveIncPoints ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Merge Dtm Hull Into Update Dtm Completed") ;
/*
**  Perform Checks
*/
 if( cdbg )
   {
    bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPnt,&insertArea,&direction) ;
    bcdtmWrite_message(0,0,0,"Area Of Inserted Hull = %20.10lf",insertArea) ;
    if( fabs (insertArea-polyArea) > 0.0001 )
      {
       bcdtmWrite_message(2,0,0,"Large Difference %20.10lf Between Polygon And Insert Areas",fabs(insertArea-polyArea)) ;
       goto errexit ;
      }  
    if( internalFlag == 1 )
      {
       bcdtmWrite_message(0,0,0,"Checking Dtm Topology") ;
       if( bcdtmCheck_topologyDtmObject(dtmP,0)) { bcdtmWrite_message(0,0,0,"Dtm Topology Invalid") ; goto errexit ; }
       else                                        bcdtmWrite_message(0,0,0,"Dtm Topology Valid") ;
       bcdtmWrite_message(0,0,0,"Checking Dtm Precision") ;
       if( bcdtmCheck_precisionDtmObject(dtmP,0))  { bcdtmWrite_message(0,0,0,"Dtm Precision Invalid") ; goto errexit ; }
       else                                          bcdtmWrite_message(0,0,0,"Dtm Precision Valid") ;
      }
    bcdtmWrite_message(0,0,0,"Checking Dtm Feature Topology") ;
    if( bcdtmCheck_topologyDtmFeaturesDtmObject(dtmP,0)) { bcdtmWrite_message(0,0,0,"Dtm Feature Topology Invalid") ; goto errexit ; }
    else                                                   bcdtmWrite_message(0,0,0,"DTM Feature Topology Valid") ;
   }
/*
** Bring Points On Insert Hull Back To Tolerances
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For And Merging Close Points On Insert Hull") ;
 if( bcdtmMerge_mergeClosePointsOnInsertBoundaryDtmObject(dtmP,internalFlag,startPnt,clipPtsP,numClipPts)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For And Merging Close Points On Insert Hull Completed") ;
/*
**  Perform Checks
*/
 if( cdbg )
   {
    if( internalFlag == 1 )
      {
       bcdtmWrite_message(0,0,0,"Checking Dtm Topology") ;
       if( bcdtmCheck_topologyDtmObject(dtmP,0)) { bcdtmWrite_message(0,0,0,"Dtm Topology Invalid") ; goto errexit ; }
       else                                        bcdtmWrite_message(0,0,0,"Dtm Topology Valid") ;
       bcdtmWrite_message(0,0,0,"Checking Dtm Precision") ;
       if( bcdtmCheck_precisionDtmObject(dtmP,1)) { bcdtmWrite_message(0,0,0,"Dtm Precision Invalid") ; goto errexit ; }
       else                                         bcdtmWrite_message(0,0,0,"Dtm Precision Valid") ;
      }
    bcdtmWrite_message(0,0,0,"Checking Dtm Feature Topology") ;
    if( bcdtmCheck_topologyDtmFeaturesDtmObject(dtmP,1)) { bcdtmWrite_message(0,0,0,"Dtm Feature Topology Invalid") ; goto errexit ; }
    else                                                   bcdtmWrite_message(0,0,0,"DTM Feature Topology Valid") ;
   }
/*
** Clean Tptr Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Cleaning Tptr Polygon") ;
 if( bcdtmList_cleanTptrPolygonDtmObject(dtmP,startPnt) ) goto errexit ;
/*
** Remove Sliver Triangles On Insert Hull
*/
// if( dbg ) bcdtmWrite_message(0,0,0,"Removing Sliver Triangles On Inserted Hull") ;
// if( bcdtmMerge_removeTrianglesOnInsertHullDtmObject(dtmP,startPnt) ) goto errexit ;
// if( dbg ) bcdtmWrite_message(0,0,0,"Removing Sliver Triangles On Inserted Hull Completed") ;
/*
** Null Out Any Void Bits On Insert Hull
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Clearing Void Bits On Insert Hull") ;
 sp = startPnt ;
 do 
   { 
    bcdtmFlag_clearVoidBitPCWD(&nodeAddrP(dtmP,sp)->PCWD) ; 
    sp = nodeAddrP(dtmP,sp)->tPtr ; 
   } while( sp != startPnt ) ;
/*
**  Internally Clip Update Tin
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Internally Clipping Update Tin") ;
 if( bcdtmClip_internalToTptrPolygonDtmObject(dtmP,startPnt,0) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Internally Clipping Update Dtm Completed") ;
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Dtm Feature Topology") ;
    if( bcdtmCheck_topologyDtmFeaturesDtmObject(dtmP,1) ){  bcdtmWrite_message(0,0,0,"Feature Topology Invalid") ; goto errexit ; }
    else                                                    bcdtmWrite_message(0,0,0,"Feature Topology Valid") ;
   }
/*
** Remove Voids Coincident With Tptr Polygon
** Added 30 May 2002 - Rob Cormack
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Voids Coincident With Insert Dtm Hull") ;
//TODO if( dbg ) bcdtmUtl_writeStatisticsDtmObject(dtmP) ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeaturePts.firstPoint != dtmP->nullPnt && dtmFeatureP->dtmFeatureType == DTMFeatureType::Void )
      {
       fpts = numPolyPts = 0 ;
       sp = dtmFeatureP->dtmFeaturePts.firstPoint ;
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"dtmFeature = %8ld of = %8ld type = %4ld sp = %9ld",dtmFeature,dtmP->numFeatures,dtmFeatureP->dtmFeatureType,sp) ; 
       ++fpts ;
       if( nodeAddrP(dtmP,sp)->tPtr != dtmP->nullPnt ) ++numPolyPts ;
       bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sp,&np) ;
       while ( np != sp )
         {
          ++fpts ;
          if( nodeAddrP(dtmP,np)->tPtr != dtmP->nullPnt ) ++numPolyPts ;
          bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,np,&np) ;
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"np = %6ld sp = %6ld",np,sp) ;
         } 
       if( fpts == numPolyPts  )
         {
          if( bcdtmInsert_removeDtmTinFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ;
         } 
      }
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Voids Coincident With Insert Dtm Hull Completed") ;
/*
** Set Return values
*/
 *firstPntP = startPnt ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Internally Clipping Dtm Object Completed") ;
 if( dbg && ret != DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Internally Clipping Dtm Object Error") ;
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
BENTLEYDTM_Private int bcdtmMerge_mergeClosePointsOnInsertBoundaryDtmObject(BC_DTM_OBJ *dtmP,long internalFlag,long startPnt,DPoint3d *hullPtsP,long numHullPts )
/*
** This Function Merges Closes Points On The Insert Boundary
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   pp,np,sp,f1,f2,p1,p2,nnp,cp1,cp2,node,process,mergeFlag,moveFlag,*pointListP=NULL ;
 double cd,dd,dx,dy  ;
 DPoint3d    *p3dP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Merging Close Points On Insert Boundary") ;
    bcdtmWrite_message(0,0,0,"dtmP             = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"internalFlag     = %8ld",internalFlag) ;
    bcdtmWrite_message(0,0,0,"startPnt         = %8ld",startPnt) ;
    bcdtmWrite_message(0,0,0,"hullPtsP         = %p",hullPtsP) ;
    bcdtmWrite_message(0,0,0,"numHullPts       = %8ld",numHullPts) ;
   }
/*
** Check Feature Lists
*/
 if( cdbg ) 
   {
    bcdtmWrite_message(0,0,0,"Checking DTM Feature Lists Tin") ;
    if( bcdtmCheck_tinFeatureListsDtmObject(dtmP,1) )
      {
       bcdtmWrite_message(0,0,0,"Error In DTM Feature Lists") ; 
       goto errexit ;
      }
   }
/*
** Copy Tptr Values To Point List
*/
 if( bcdtmList_copyTptrValuesToPointListDtmObject(dtmP,&pointListP)) goto errexit ; 
/*
** Mark Insert Hull Points That Cannot be Merged
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points That Can Not Be Merged") ;
 np = startPnt ;
 for( node = 0 ; node < dtmP->numPoints ; ++node  ) nodeAddrP(dtmP,node)->PRGN = 0 ;
 for( p3dP = hullPtsP ; p3dP < hullPtsP + numHullPts ; ++p3dP )
   {
    sp = np ;
    do
      {
       if( p3dP->x == pointAddrP(dtmP,sp)->x && p3dP->y == pointAddrP(dtmP,sp)->y ) 
         { 
          nodeAddrP(dtmP,sp)->PRGN = 1 ; 
          np = sp  ;
         }
       else sp = nodeAddrP(dtmP,sp)->tPtr ;
      } while ( sp != np ) ;  
   }
/*
** Mark Dtm Hull Points That Cannot be Merged
*/
 sp = dtmP->hullPoint ;
 do
   {
    nodeAddrP(dtmP,sp)->PRGN = 1 ;
    sp = nodeAddrP(dtmP,sp)->hPtr ;
   } while( sp != dtmP->hullPoint) ;
/*
** Write Out Points Closer Than ppTol
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Points On Insert Hull Closer Than ppTol of %20.15lf",dtmP->ppTol) ;
    sp = startPnt  ;
    do
      {
       np = nodeAddrP(dtmP,sp)->tPtr ;
       dd = bcdtmMath_pointDistanceDtmObject(dtmP,sp,np) ;
       if( dd < dtmP->ppTol )
         {
          bcdtmWrite_message(0,0,0,"Close Points %9ld %9ld ** Distance = %20.15lf",sp,np,dd) ;
         }
       sp = np ;
      } while( sp != startPnt ) ;
   }
/*
** Merge Close Points
*/  
 if( dbg ) bcdtmWrite_message(0,0,0,"Merging Close Points") ;
 pp = startPnt ; 
 process = 1 ;
 while( process )
   {
    process = 0 ;
    sp = startPnt ;
/*
**  Get Closest Points On Tptr Polygon
*/ 
    cp1 =  cp2 = dtmP->nullPnt ;
    cd = bcdtmMath_distance(dtmP->xMin,dtmP->yMin,dtmP->xMax,dtmP->yMax) ;    
    do
      {
       np = nodeAddrP(dtmP,sp)->tPtr ;
       if(nodeAddrP(dtmP,sp)->PRGN == 0 || nodeAddrP(dtmP,np)->PRGN == 0 )
         {
          dx = fabs(pointAddrP(dtmP,sp)->x - pointAddrP(dtmP,np)->x ) ;
          dy = fabs(pointAddrP(dtmP,sp)->y - pointAddrP(dtmP,np)->y ) ;
          if( dx <= cd && dy <= cd )
            {
             if(( dd = sqrt(dx*dx + dy*dy) ) < cd )
               { cd = dd ; cp1 = sp ; cp2 = np ; }
            }
         }
        sp = np ;
       } while ( sp != startPnt ) ;
/*
**   Merge Points
*/
     if( cd < dtmP->ppTol  )
       {
        if( dbg )
          {
           bcdtmWrite_message(0,0,0,"Merging cp1 = %6ld PRGN = %2ld ** %10.4lf %10.4lf %10.4lf",cp1,nodeAddrP(dtmP,cp1)->PRGN,pointAddrP(dtmP,cp1)->x,pointAddrP(dtmP,cp1)->y,pointAddrP(dtmP,cp1)->z) ;
           bcdtmWrite_message(0,0,0,"        cp2 = %6ld PRGN = %2ld ** %10.4lf %10.4lf %10.4lf",cp2,nodeAddrP(dtmP,cp2)->PRGN,pointAddrP(dtmP,cp2)->x,pointAddrP(dtmP,cp2)->y,pointAddrP(dtmP,cp2)->z) ;
          } 
        f1 = nodeAddrP(dtmP,cp1)->PRGN ; 
        f2 = nodeAddrP(dtmP,cp2)->PRGN ; 
        if( ! f1 || ! f2 )
          {
           nnp = nodeAddrP(dtmP,cp2)->tPtr ; 
           bcdtmList_getTptrPriorAndNextPointsDtmObject(dtmP,cp1,&pp,&p1) ;
           if( dbg ) 
             {
              bcdtmWrite_message(0,0,0,"Distance = %20.15lf",bcdtmMath_distance(pointAddrP(dtmP,cp1)->x,pointAddrP(dtmP,cp1)->y,pointAddrP(dtmP,cp2)->x,pointAddrP(dtmP,cp2)->y)) ;
              bcdtmWrite_message(0,0,0,"pp = %6ld cp1 = %6ld cp2 = %6ld nnp = %6ld",pp,cp1,cp2,nnp) ;
              bcdtmWrite_message(0,0,0,"pp->Tptr = %6ld cp1->Tptr = %6ld cp2->Tptr = %6ld nnp-Tptr = %6ld",nodeAddrP(dtmP,pp)->tPtr,nodeAddrP(dtmP,cp1)->tPtr,nodeAddrP(dtmP,cp2)->tPtr,nodeAddrP(dtmP,nnp)->tPtr) ;
             }

           p1 = cp1 ; p2 = cp2 ; 
           if( f2 || (nodeAddrP(dtmP,p2)->hPtr != dtmP->nullPnt &&  ! f1) ) { p1 = cp2 ; p2 = cp1 ; }
          
           if( nodeAddrP(dtmP,p2)->PRGN ) { sp = p1 ; p1 = p2 ; p2 = sp ; }

           if( bcdtmMerge_checkPointCanBeMergedDtmObject(dtmP,p1,p2,&mergeFlag)) goto errexit ;
           if( ! mergeFlag && nodeAddrP(dtmP,p1)->PRGN == 0 )
             {
              sp = p1 ; p1 = p2 ; p2 = sp ;
              if( bcdtmMerge_checkPointCanBeMergedDtmObject(dtmP,p1,p2,&mergeFlag)) goto errexit ;
             }

           if( ! mergeFlag ) 
             {
              if( dbg ) bcdtmWrite_message(0,0,0,"Points Cannot Be Merged") ;
              if( nodeAddrP(dtmP,p1)->PRGN == 0 ) nodeAddrP(dtmP,p1)->PRGN = 2 ;
              if( nodeAddrP(dtmP,p2)->PRGN == 0 ) nodeAddrP(dtmP,p2)->PRGN = 2 ;
             } 
           else
             {
              nodeAddrP(dtmP,p2)->hPtr = dtmP->nullPnt ;

//              if( dbg ) bcdtmWrite_message(0,0,0,"Fixing Feature List For Last Point %6ld %6ld",p2,p1) ;
//              if( bcdtmMerge_fixFeatureListForLastPointDtmObject(dtmP,p2,p1)) goto errexit ;
 
              if( dbg ) bcdtmWrite_message(0,0,0,"Copying Feature List %6ld %6ld",p2,p1) ;
              if( bcdtmMerge_copyFeatureListFromPointToPointDtmObject(dtmP,p2,p1) ) goto errexit ;

              if( dbg ) bcdtmWrite_message(0,0,0,"Merging Connected Points %6ld %6ld",p2,p1) ;
              if( bcdtmMerge_connectedPointsDtmObject(dtmP,p2,p1) ) goto errexit ;
              if( dbg ) bcdtmWrite_message(0,0,0,"Points Merged") ;

              if( p1 == cp1 ) *(pointListP+cp1) = nnp ; 
              else            *(pointListP+pp)  = cp2 ;    

              if( dbg ) bcdtmWrite_message(0,0,0,"pp = %6ld cp1 = %6ld cp2 = %6ld nnp = %6ld",pp,cp1,cp2,nnp) ;
              if( dbg ) bcdtmWrite_message(0,0,0,"pp->Tptr = %6ld cp1->Tptr = %6ld cp2->Tptr = %6ld nnp-Tptr = %6ld",nodeAddrP(dtmP,pp)->tPtr,nodeAddrP(dtmP,cp1)->tPtr,nodeAddrP(dtmP,cp2)->tPtr,nodeAddrP(dtmP,nnp)->tPtr) ;

              process = 1 ;
/*
**            Check Feature Lists
*/
              if( cdbg ) 
                {
                 bcdtmWrite_message(0,0,0,"Checking DTM Feature Lists Tin") ;
                 if( bcdtmCheck_tinFeatureListsDtmObject(dtmP,1) )
                   {
                    bcdtmWrite_message(0,0,0,"Error In DTM Feature Lists After Merging Point %9ld To Point %9ld",p2,p1) ; 
                    goto errexit ;
                   }
                }
/*
**            Rebuild Tptr Polygon
*/
              sp = startPnt ;
              do
                {
                 nodeAddrP(dtmP,sp)->tPtr = *(pointListP+sp) ;
                 sp = *(pointListP+sp) ;
                } while ( sp != startPnt  ) ;
/*
**            Check Connectivity Of Tptr Polygon 
*/

              if( cdbg )
                {
                 if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,startPnt,1))
                   { 
                    bcdtmList_writeTptrListDtmObject(dtmP,startPnt) ;
                    goto errexit ; 
                   }
                }
             }
          }
       }
    } 
/*
** Remove Slivers On Insert Line
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Merging Slivers") ;
 sp = startPnt ;
 do
   {
    np = nodeAddrP(dtmP,sp)->tPtr ;
    if( ( p1 = bcdtmList_nextClkDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
    if( bcdtmList_testLineDtmObject(dtmP,p1,np) && nodeAddrP(dtmP,p1)->tPtr == dtmP->nullPnt )
      { 
       dd = bcdtmMath_distanceOfPointFromLine(&f1,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,np)->x,pointAddrP(dtmP,np)->y,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,&dx,&dy) ;
       if( dd > dtmP->plTol ) f1 = 0 ;
       if(nodeAddrP(dtmP,p1)->fPtr != dtmP->nullPtr ) f1 = 0 ; 
       if( f1 )
         {
          if( dbg )bcdtmWrite_message(0,0,0,"Removing Sliver On Insert Hull %6ld %6ld %6ld",sp,np,p1) ;
          if( ( moveFlag = bcdtmInsert_checkPointCanBeMovedOnToLineDtmObject(dtmP,p1,np,sp,dx,dy)) == 1 ) goto errexit ;
          if( ! moveFlag )
            {
             if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
             if( bcdtmList_testLineDtmObject(dtmP,p2,np) )
               { 
                if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p1,p2,sp)) goto errexit ;
                if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p2,p1,np)) goto errexit ;
                if( bcdtmList_deleteLineDtmObject(dtmP,sp,np)) goto errexit ;
                nodeAddrP(dtmP,sp)->tPtr = p1 ;
                pointAddrP(dtmP,p1)->x = dx ;
                pointAddrP(dtmP,p1)->y = dy ; 
                nodeAddrP(dtmP,p1)->tPtr = np ; 
               }
             else
               {
                if( bcdtmList_deleteLineDtmObject(dtmP,sp,np)) goto errexit ;
                nodeAddrP(dtmP,sp)->tPtr = p1 ;
                pointAddrP(dtmP,p1)->x = dx ;
                pointAddrP(dtmP,p1)->y = dy ; 
                nodeAddrP(dtmP,p1)->tPtr = np ; 
               }
            }
          if( dbg ) bcdtmWrite_message(0,0,0,"Sliver Removed") ;
         }
      }
    sp = np ; 
   } while ( sp != startPnt ) ;

/*
** Check Feature Topology - Development Only
*/ 
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Dtm Feature Topology") ;
    if( bcdtmCheck_topologyDtmFeaturesDtmObject(dtmP,1) ){  bcdtmWrite_message(0,0,0,"Feature Topology Corrupted") ; goto errexit ; }
    else                                                    bcdtmWrite_message(0,0,0,"Feature Topology OK") ;
   }
/*
** Clean Up
*/
 cleanup :
 if( pointListP != NULL ) free(pointListP) ;
 for( node = 0 ; node < dtmP->numPoints ; ++node  ) nodeAddrP(dtmP,node)->PRGN = 0 ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Merging Close Points On Insert Boundary Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Merging Close Points On Insert Boundary Error") ;
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
BENTLEYDTM_Public int bcdtmMerge_fixFeatureListForLastPointDtmObject(BC_DTM_OBJ *dtmP,long pnt1,long pnt2)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long firstPnt,lastPnt,priorPnt,dtmFeature,pnt2OnFeature ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Fixing Feature List For Last Point ** pnt1 = %6ld pnt2 = %6ld",pnt1,pnt2) ;
/*
** Find All Features For Which pnt1 Is Last Point
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeaturePts.firstPoint != dtmP->nullPnt )
      {
       bcdtmList_getFirstAndLastPointForDtmFeatureDtmObject(dtmP,dtmFeature,&firstPnt,&lastPnt) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Feat = %6ld Fp = %6ld Lp = %6ld",dtmFeature,firstPnt,lastPnt) ;
/*
**     Remove Feature If Only pnt1 And pnt2 Exist On Feature
*/
       if( ( pnt1 == firstPnt && pnt2 == lastPnt ) || ( pnt1 == lastPnt && pnt2 == firstPnt ) )
         {
          if( bcdtmInsert_removeDtmTinFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ;          
         }
/*
**     Remove pnt1 From Feature If pnt1 Is The Last Point
*/
       else if( pnt1 == lastPnt && pnt1 != firstPnt )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"pnt1 %9ld is last Point of DTM Feature",pnt1,dtmFeature) ;
          bcdtmList_testForPointOnTinFeatureDtmObject(dtmP,dtmFeature,pnt2,&pnt2OnFeature) ;
          bcdtmList_getPriorPointForDtmFeatureDtmObject(dtmP,dtmFeature,pnt1,&priorPnt) ;
          if( pnt2OnFeature )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"pnt2 %9ld On Feature Where pnt1 %9ld is last Point",pnt2,pnt1) ;
             if( dbg ) bcdtmWrite_message(0,0,0,"Removing Point %9ld From Dtm Feature %9ld",pnt1,dtmFeature) ;
             if( priorPnt == pnt2 ) { if( bcdtmInsert_removePointFromDtmFeatureDtmObject(dtmP,pnt1,dtmFeature)) goto errexit ; }       
             else           
               { 
                if( dbg ) bcdtmWrite_message(0,0,0,"Removing Dtm Feature %9ld",dtmFeature) ;
                if( bcdtmInsert_removeDtmTinFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ; 
               }
            } 
         }
      } 
   }
/*
** Claen Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Fixing Feature List For Last Point ** pnt1 = %6ld pnt2 = %6ld Completed",pnt1,pnt2) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Fixing Feature List For Last Point ** pnt1 = %6ld pnt2 = %6ld Error",pnt1,pnt2) ;
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
BENTLEYDTM_Public int bcdtmMerge_copyFeatureListFromPointToPointDtmObject(BC_DTM_OBJ *dtmP,long pnt1,long pnt2)
/*
**
** This Function Copies The Feature List Pointers From pnt1 to pnt2
** This Is Done Prior To Merging pnt1 To pnt2
**
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  clc1,clc2,clc3,process,dtmFeature,priorPnt,nextPnt,pnt2OnFeature;
/*
** Test For pnt1 Having A Feature List
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying Feature List From Point %9ld to Point %9ld",pnt1,pnt2) ;
/*
** Only Copy If pnt1 Has A Feature List
*/
 if( nodeAddrP(dtmP,pnt1)->fPtr != dtmP->nullPtr ) 
   {
/*
**  Remove pnt1 From Dtm Features Lists That Include pnt2
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Removing pnt1 %9ld From Feature Lists That Include pnt2 %9ld",pnt1,pnt2) ;
    process = 1 ;
    while ( process )
      {
       process = 0 ;
       clc1 = nodeAddrP(dtmP,pnt1)->fPtr ;
       while( clc1 != dtmP->nullPtr )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"pnt1 = %6ld dtmFeature = %6ld nextPnt = %9ld nextPtr = %9ld",pnt1,flistAddrP(dtmP,clc1)->dtmFeature,flistAddrP(dtmP,clc1)->nextPnt,flistAddrP(dtmP,clc1)->nextPtr ) ;
          if( ftableAddrP(dtmP,flistAddrP(dtmP,clc1)->dtmFeature)->dtmFeaturePts.firstPoint != dtmP->nullPnt )
            {
             bcdtmList_testForPointOnTinFeatureDtmObject(dtmP,flistAddrP(dtmP,clc1)->dtmFeature,pnt2,&pnt2OnFeature) ;
             if( pnt2OnFeature )
               {
                if( dbg ) 
                  {
                   bcdtmWrite_message(0,0,0,"pnt1 = %6ld pnt2 = %6ld On Feature %6ld",pnt1,pnt2,flistAddrP(dtmP,clc1)->dtmFeature) ;
//                   bcdtmList_writePointsForDtmFeatureDtmObject(dtmP,flistAddrP(dtmP,clc1)->dtmFeature) ;
                  }
                if( bcdtmList_getPriorPointForDtmFeatureDtmObject(dtmP,flistAddrP(dtmP,clc1)->dtmFeature,pnt1,&priorPnt)) goto errexit  ;
                if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,flistAddrP(dtmP,clc1)->dtmFeature,pnt1,&nextPnt)) goto errexit  ;
                if( dbg ) bcdtmWrite_message(0,0,0,"pnt1 ** nextPoint = %9ld priorPoint = %9ld",nextPnt,priorPnt) ;
                if( pnt2 == nextPnt || pnt2 == priorPnt ) 
                  { 
                   if( bcdtmInsert_removePointFromDtmFeatureDtmObject(dtmP,pnt1,flistAddrP(dtmP,clc1)->dtmFeature)) goto errexit  ;
                   process = 1 ; 
                  }
                else  
                  {                     
                   if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Feature %6ld",flistAddrP(dtmP,clc1)->dtmFeature) ;
                   if( bcdtmInsert_removeDtmTinFeatureFromDtmObject(dtmP,flistAddrP(dtmP,clc1)->dtmFeature)) goto errexit  ; 
                  }
               } 
            }
/*
**        Get Next Feature For Point
*/
          if( ! process ) clc1 = flistAddrP(dtmP,clc1)->nextPtr ; 
          else            clc1 = dtmP->nullPtr ;
         }
      }
/*
**   Test For pnt1 Having A Feature List
*/
    if( nodeAddrP(dtmP,pnt1)->fPtr  != dtmP->nullPtr ) 
      {
/*
**     Copy pnt1 Features To pnt2
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Copying pnt1 Features To pnt2") ;
       while ( ( clc1 = nodeAddrP(dtmP,pnt1)->fPtr ) != dtmP->nullPtr )
         {
          dtmFeature = flistAddrP(dtmP,clc1)->dtmFeature ; 
          if( bcdtmList_getPriorPointForDtmFeatureDtmObject(dtmP,dtmFeature,pnt1,&priorPnt)) goto errexit  ;
/*
**        Update Prior Point In Feature List
*/
          if( priorPnt == dtmP->nullPnt ) ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint = pnt2 ;
          else
            {
             clc3 = dtmP->nullPtr ;
             clc2 = nodeAddrP(dtmP,priorPnt)->fPtr ;
             while( clc2 != dtmP->nullPtr && clc3 == dtmP->nullPtr ) 
               { 
                if( flistAddrP(dtmP,clc2)->dtmFeature == dtmFeature ) clc3 = clc2 ; 
                clc2 = flistAddrP(dtmP,clc2)->nextPtr ;
               }
             if( clc3 == dtmP->nullPtr ) { bcdtmWrite_message(2,0,0,"Cannot Find Feature In Prior Point ") ; goto errexit  ; }
             flistAddrP(dtmP,clc3)->nextPnt = pnt2 ;
            }  
/*
**        Detach Feature List Entry From pnt1 And Attach To pnt2
*/
          nodeAddrP(dtmP,pnt1)->fPtr = flistAddrP(dtmP,clc1)->nextPtr ;  ;
          clc2 = nodeAddrP(dtmP,pnt2)->fPtr ;  
          if ( clc2 == dtmP->nullPtr ) nodeAddrP(dtmP,pnt2)->fPtr = clc1 ;
          else
            {  
             while( flistAddrP(dtmP,clc2)->nextPtr != dtmP->nullPtr ) clc2 = flistAddrP(dtmP,clc2)->nextPtr ; 
             flistAddrP(dtmP,clc2)->nextPtr = clc1 ;
            }
          flistAddrP(dtmP,clc1)->nextPtr = dtmP->nullPtr ;
/*
**        Set First Pointer For Feature
*/
          if( ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint == pnt1 ) ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint = pnt2 ; 
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
BENTLEYDTM_Public int bcdtmMerge_insertMergeDtmObject(BC_DTM_OBJ *dtm1P,BC_DTM_OBJ *dtm2P,long mergeFlag,long firstPnt)
/*
** This Function Inserts The Merge Dtm Points And Lines Into The Update Tin
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long    ofs,newPnt,clc,pp,p1,p2,p3,p4,np1,np2,np3,saveIncPoints,numListPts ;
 DPoint3d     *p3dP,*listPtsP=NULL ;
 DTM_TIN_NODE   *nodeP ;
 DTM_TIN_POINT  *pointP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Merge Dtm") ;
/*
** Initialise
*/
 saveIncPoints = dtm1P->incPoints ;
/*
** Copy Insert Boundary Tptr List To DPoint3d List
*/
 if( bcdtmList_copyTptrListToPointArrayDtmObject(dtm1P,firstPnt,&listPtsP,&numListPts)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Insert Hull Points = %8ld",numListPts) ;
/*
** Compact DTM 1 Arrays
*/
 if( dbg )bcdtmWrite_message(0,0,0,"Compacting dtm1P Point And List Structures") ;
 if( bcdtmTin_compactCircularListDtmObject(dtm1P))       goto errexit  ;
 if( bcdtmTin_compactFeatureTableDtmObject(dtm1P))       goto errexit  ;
 if( bcdtmTin_compactFeatureListDtmObject(dtm1P))        goto errexit  ;
 if( bcdtmTin_compactPointAndNodeTablesDtmObject(dtm1P)) goto errexit  ;
 if( bcdtmTin_resortTinStructureDtmObject(dtm1P))        goto errexit  ;
/*
** Copy DPoint3d List To Tptr List
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying DPoint3d List To Tptr List") ;
 pp = dtm1P->nullPnt ;
 for ( p3dP = listPtsP ; p3dP < listPtsP + numListPts ; ++p3dP )
   {
    bcdtmFind_closestPointDtmObject(dtm1P,p3dP->x,p3dP->y,&newPnt) ;
    if( pointAddrP(dtm1P,newPnt)->x != p3dP->x || pointAddrP(dtm1P,newPnt)->y != p3dP->y ) 
      {
       bcdtmWrite_message(2,0,0,"Cannot Find dtm1P Point Number For DPoint3d List Point") ;
       bcdtmWrite_message(0,0,0,"DPoint3d  List  Point[%6ld] ** %15.7lf %15.7lf %10.4lf",(long)(p3dP-listPtsP),p3dP->x,p3dP->y,p3dP->z) ;
       bcdtmWrite_message(0,0,0,"dtm1P New  Point[%6ld] ** %15.7lf %15.7lf %10.4lf",newPnt,pointAddrP(dtm1P,newPnt)->x,pointAddrP(dtm1P,newPnt)->y,pointAddrP(dtm1P,newPnt)->z) ;
       goto errexit ;
      }
    if( pp != dtm1P->nullPnt ) nodeAddrP(dtm1P,pp)->tPtr = newPnt ;
    pp = newPnt ;
   }
/*
**  Check Connectivity Of Tptr List
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Connectivity Of Tptr List After Coping DPoint3d To Tptr List") ;
    if( bcdtmList_checkConnectivityTptrListDtmObject(dtm1P,newPnt,1)) goto errexit ;
   }
/*
**  Free memory For DPoint3d List
*/
 if( listPtsP != NULL ) { free(listPtsP) ; listPtsP = NULL ; }
/*
** Get TIN1 Point Numbers For TIN2 Hull Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Assigning dtm1P Point Numbers To dtm2P Hull Points") ;
 p2 = dtm2P->hullPoint ;
 do
   {
    bcdtmFind_closestPointDtmObject(dtm1P,pointAddrP(dtm2P,p2)->x,pointAddrP(dtm2P,p2)->y,&newPnt) ;
    if( pointAddrP(dtm1P,newPnt)->x != pointAddrP(dtm2P,p2)->x ||pointAddrP(dtm1P,newPnt)->y != pointAddrP(dtm2P,p2)->y ) 
      {
       bcdtmWrite_message(2,0,0,"Cannot Find dtm1P Point Number For dtm2P Hull Point") ;
       bcdtmWrite_message(0,0,0,"dtm2P Hull Point[%6ld] ** %15.7lf %15.7lf %10.4lf",p2,pointAddrP(dtm2P,p2)->x,pointAddrP(dtm2P,p2)->y,pointAddrP(dtm2P,p2)->z) ;
       bcdtmWrite_message(0,0,0,"dtm1P New  Point[%6ld] ** %15.7lf %15.7lf %10.4lf",newPnt,pointAddrP(dtm1P,newPnt)->x,pointAddrP(dtm1P,newPnt)->y,pointAddrP(dtm1P,newPnt)->z) ;
       goto errexit ;
      }
    nodeAddrP(dtm2P,p2)->sPtr = newPnt ; 
    p2 = nodeAddrP(dtm2P,p2)->hPtr ;
   } while ( p2 != dtm2P->hullPoint ) ;
/*
** Merge DTM 2 Points Into DTM 1
*/
 dtm1P->incPoints = dtm2P->numPoints ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Merging DTM 2 Points Into DTM 1") ;
 for( ofs = 0 ; ofs < dtm2P->numPoints ; ++ofs)
   {
    nodeP  = nodeAddrP(dtm2P,ofs) ;
    pointP = pointAddrP(dtm2P,ofs) ;
    newPnt = dtm1P->nullPnt ;
    if( nodeP->hPtr == dtm2P->nullPnt )
      {
       if( bcdtmInsert_addPointToDtmObject(dtm1P,pointP->x,pointP->y,pointP->z,&newPnt))  goto errexit  ; 
       if( bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtm2P,ofs)->PCWD) ) bcdtmFlag_setVoidBitPCWD(&nodeAddrP(dtm1P,newPnt)->PCWD) ; 
       nodeP->sPtr = newPnt ;
      }  
   } 
/*
** Write Out dtm2P Point Numbers
*/
 if( dbg == 2 )
   {
    for( p1 = 0 ; p1 < dtm2P->numPoints ; ++p1 )
      {
       bcdtmWrite_message(0,0,0,"dtm2P[%10ld] fTableP = %10ld New Point = %10ld",p1,nodeAddrP(dtm2P,p1)->hPtr,nodeAddrP(dtm2P,p1)->sPtr) ;
      }
   }
/*
** Merge Internal DTM 2 Lines Into DTM 1
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Merging Internal DTM 2 Lines Into DTM 1 ") ;
 for( p1 = 0 ; p1 < dtm2P->numPoints ; ++p1 )
   {
    if( ( clc = nodeAddrP(dtm2P,p1)->cPtr ) != dtm2P->nullPtr )
      {
       np1 = nodeAddrP(dtm2P,p1)->sPtr ;
       if( nodeAddrP(dtm1P,np1)->cPtr == dtm1P->nullPtr )
         {
          np3 = dtm1P->nullPnt ;
          while( clc != dtm2P->nullPtr )
            {
             p2  = clistAddrP(dtm2P,clc)->pntNum ;
             clc = clistAddrP(dtm2P,clc)->nextPtr ;
             np2 = nodeAddrP(dtm2P,p2)->sPtr ;
             if( bcdtmList_insertLineAfterPointDtmObject(dtm1P,np1,np2,np3)) goto errexit  ; 
             np3 = np2 ;
            }
         } 
      }
   }
/*
**  Merge DTM 2 Hull Lines Into DTM 1
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Merging dtm2P Hull Lines Into dtm1P") ;
 p1 = dtm2P->hullPoint ;
 do
   { 
    p3  = nodeAddrP(dtm2P,p1)->hPtr  ;
    np1 = nodeAddrP(dtm2P,p1)->sPtr  ;
    np3 = nodeAddrP(dtm1P,np1)->tPtr ;
    if(( p2 = bcdtmList_nextAntDtmObject(dtm2P,p1,p3)) < 0 )  goto errexit  ; 
    while ( nodeAddrP(dtm2P,p2)->hPtr != p1 )
      {
       np2 = nodeAddrP(dtm2P,p2)->sPtr  ;
       if( bcdtmList_insertLineBeforePointDtmObject(dtm1P,np1,np2,np3)) goto errexit ;
       np3 = np2 ; 
       if(( p2 = bcdtmList_nextAntDtmObject(dtm2P,p1,p2)) < 0 ) goto errexit  ; 
      }
    p1 = nodeAddrP(dtm2P,p1)->hPtr  ;
   } while ( p1 != dtm2P->hullPoint ) ;
/*
** Set Convex Hull For dtm1P
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Setting Convex Hull") ;
 bcdtmList_setConvexHullDtmObject(dtm1P) ;
/*
** Stitch Tins Together
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Stitching TINS") ;
 p1 = dtm2P->hullPoint ;
 do
   {
    p2 = nodeAddrP(dtm2P,p1)->hPtr ;
    if( nodeAddrP(dtm1P,nodeAddrP(dtm2P,p1)->sPtr)->tPtr  != nodeAddrP(dtm2P,p2)->sPtr )
      {
       pp = nodeAddrP(dtm2P,p1)->sPtr ;
       p3 = nodeAddrP(dtm1P,pp)->tPtr ;
       if( ( p4 = bcdtmList_nextAntDtmObject(dtm1P,pp,p3)) < 0 ) goto errexit  ; 
       while ( p3 != nodeAddrP(dtm2P,p2)->sPtr )
         {
          if( bcdtmList_insertLineAfterPointDtmObject(dtm1P,p3,p4,pp))  goto errexit  ; 
          if( bcdtmList_insertLineBeforePointDtmObject(dtm1P,p4,p3,pp)) goto errexit  ; 
          pp = p3 ;
          p3 = nodeAddrP(dtm1P,p3)->tPtr ;
         }
      }
    p1 = p2 ;
   } while ( p1 != dtm2P->hullPoint ) ;
/*
** Detect And Insert Implied Voids
*/
 if( mergeFlag == 2 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Detecting And Inserting Implied Voids") ;
    if( bcdtmMerge_detectAndInsertImpliedVoidsDtmObject(dtm1P,dtm2P) ) goto errexit  ; 
    bcdtmList_setConvexHullDtmObject(dtm1P) ; 
   }
/*
** Null Out Tptr Array
*/
 if( bcdtmList_nullTptrListDtmObject(dtm1P,nodeAddrP(dtm2P,dtm2P->hullPoint)->sPtr))  goto errexit  ; 
/*
** Check And Fix Any Precision Problems
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking And Fixing Precision For Triangles") ;
 if( bcdtmMerge_checkAndFixPrecisionForTrianglesDtmObject(dtm1P) ) goto errexit ;
/*
** Copy Dtm Features To Merged 
*/ 
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying DTM Features To Merge Tin") ;
 if( bcdtmMerge_copyDtmFeaturesToDtmObject(dtm1P,dtm2P)) goto errexit  ; 
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Checking Dtm Feature Topology") ;
    if( bcdtmCheck_topologyDtmFeaturesDtmObject(dtm1P,1) ){ bcdtmWrite_message(0,0,0,"Feature Topology Corrupted") ; goto errexit  ; }
    else                                                    bcdtmWrite_message(0,0,0,"Feature Topology OK") ;
   }
/*
** Resolve Adjoining Voids
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Resolving Adjoining Voids") ;
 if( bcdtmClip_resolveAdjoiningPolygonalFeaturesDtmObject(dtm1P))  goto errexit  ; 
/*
** Clean Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Cleaning Dtm Object") ;
 if( bcdtmList_cleanDtmObject(dtm1P)) goto errexit ;
/*
** Cleanup
*/
 cleanup :
 if( listPtsP != NULL ) free(listPtsP) ;
 dtm1P->incPoints = saveIncPoints ;
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Inserting Merge Dtm Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Inserting Merge Dtm Error") ;
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
BENTLEYDTM_Private int bcdtmMerge_copyDtmFeaturesToDtmObject(BC_DTM_OBJ *dtm1P,BC_DTM_OBJ *dtm2P)
/*
** This Function Copies The DTM Features From dtm2P To dtm1P
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    sp,np,clc,spnt,lpnt,fspnt,dtmFeature,process,addflag ;
 char    dtmFeatureTypeName[50] ;
 BC_DTM_FEATURE   *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying DTM Features To Merge Dtm") ;
/*
**  Only Process If Features Present
*/
 if( dtm2P->numFeatures > 0 ) 
   {
/*
** Look For dtm1P Points Without Any Circular List . 
** This is caused by the precision fix prior to copying dtmFeatures
** Code Added  8 April 2004 - Rob Cormack
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Looking For dtm1P Points Without Any Feature List") ;
    for( sp = 0 ; sp < dtm2P->numPoints ; ++sp )
      {
       if( nodeAddrP(dtm2P,sp)->sPtr >= 0 && nodeAddrP(dtm2P,sp)->sPtr < dtm1P->numPoints )
         { 
          if( nodeAddrP(dtm1P,nodeAddrP(dtm2P,sp)->sPtr)->cPtr == dtm1P->nullPtr )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"dtm2PPoint[%6ld] dtm1PPoint[%6ld] = %12.6lf %12.6lf %12.6lf ** Has No Circular List",sp,nodeAddrP(dtm2P,sp)->sPtr,pointAddrP(dtm2P,sp)->x,pointAddrP(dtm2P,sp)->y,pointAddrP(dtm2P,sp)->z) ;
             bcdtmFind_closestPointDtmObject(dtm1P,pointAddrP(dtm2P,sp)->x,pointAddrP(dtm2P,sp)->y,&np) ;
             if( dbg ) bcdtmWrite_message(0,0,0,"Closest Point = %6ld  ** %12.6lf %12.6lf %12.6lf",np,pointAddrP(dtm1P,np)->x,pointAddrP(dtm1P,np)->y,pointAddrP(dtm1P,np)->z) ;
             nodeAddrP(dtm2P,sp)->sPtr = np ;
            }
         }
      }
/*
**  Null Out Tptr Values
*/
    bcdtmList_nullTptrValuesDtmObject(dtm1P) ;
/*
**  Scan And Copy Features
*/
    for( dtmFeature = 0 ; dtmFeature <  dtm2P->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtm2P,dtmFeature) ;
       dtmFeatureP->internalToDtmFeature = dtm2P->nullPnt;
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeaturePts.firstPoint != dtm2P->nullPnt )
         {
          if( dtmFeatureP->dtmFeatureType != DTMFeatureType::Hull )
            {
             fspnt = lpnt = spnt = dtmFeatureP->dtmFeaturePts.firstPoint    ;
             clc   = nodeAddrP(dtm2P,spnt)->fPtr ;
             if( dbg ) 
               {
                bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName) ;
                bcdtmWrite_message(0,0,0,"Copying Feature  %6ld of %6ld ** Fpnt = %6ld Type = %s",dtmFeature,dtm2P->numFeatures,fspnt,dtmFeatureTypeName) ;
               }
/*
**           Scan Feature List Pointers
*/
             process = 1 ;
             if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Feature List") ; 
             while ( clc != dtm2P->nullPtr && process )
               {
                while ( clc != dtm2P->nullPtr  && flistAddrP(dtm2P,clc)->dtmFeature != dtmFeature ) clc = flistAddrP(dtm2P,clc)->nextPtr ;
                if( clc != dtm2P->nullPtr )
                  {
                   spnt = flistAddrP(dtm2P,clc)->nextPnt ;
                   if( spnt != dtm2P->nullPnt )
                     {
                      if(  dtmFeatureP->dtmFeatureType == DTMFeatureType::GroupSpots || bcdtmList_testLineDtmObject(dtm1P,nodeAddrP(dtm2P,lpnt)->sPtr,nodeAddrP(dtm2P,spnt)->sPtr)) 
                        {
                         nodeAddrP(dtm1P,nodeAddrP(dtm2P,lpnt)->sPtr)->tPtr = nodeAddrP(dtm2P,spnt)->sPtr ;
                        }
                      else
                        {
                         sp = nodeAddrP(dtm1P,nodeAddrP(dtm2P,spnt)->sPtr)->tPtr ;
                         nodeAddrP(dtm1P,nodeAddrP(dtm2P,spnt)->sPtr)->tPtr = dtm1P->nullPnt ;
                         if( bcdtmInsert_lineBetweenPointsDtmObject(dtm1P,nodeAddrP(dtm2P,lpnt)->sPtr,nodeAddrP(dtm2P,spnt)->sPtr,1,2)) process = 0 ;
                         else nodeAddrP(dtm1P,nodeAddrP(dtm2P,spnt)->sPtr)->tPtr = sp ;
                        } 
                      lpnt = spnt ;
                      if( clc != dtm2P->nullPtr ) clc = nodeAddrP(dtm2P,spnt)->fPtr ;
                     } 
                   if( spnt == dtm2P->nullPnt || spnt == fspnt )  process = 0 ; 
                  }
               }
             if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Feature List Completed") ; 
/*
**           Check Connectivity Of Tptr List
*/
             addflag = 1 ;
             switch( dtmFeatureP->dtmFeatureType )
               {
                case  DTMFeatureType::Breakline   :
                case  DTMFeatureType::ContourLine : 
                  if( bcdtmList_checkConnectivityTptrListDtmObject(dtm1P,nodeAddrP(dtm2P,fspnt)->sPtr,0))
                    {
                     if( dbg ) bcdtmWrite_message(0,0,0,"Scanning And Fixing String List") ; 
                     if( bcdtmInsert_scanAndInsertBrokenTptrLinksDtmObject(dtm1P,nodeAddrP(dtm2P,fspnt)->sPtr,2,2)) goto errexit ;
                     if( bcdtmList_checkConnectivityTptrListDtmObject(dtm1P,nodeAddrP(dtm2P,fspnt)->sPtr,0))
                       {
                        addflag = 0 ;  
                       }
                    }
                break ;
 
                case  DTMFeatureType::Void   :
                case  DTMFeatureType::Island :  
                case  DTMFeatureType::Hole   :
                case  DTMFeatureType::Polygon  :
                case  DTMFeatureType::Region  :
                  if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtm1P,nodeAddrP(dtm2P,fspnt)->sPtr,0)) 
                    {
                     if( dbg ) bcdtmWrite_message(0,0,0,"Scanning And Fixing Polygon List") ; 
                     if( bcdtmInsert_scanAndInsertBrokenTptrLinksDtmObject(dtm1P,nodeAddrP(dtm2P,fspnt)->sPtr,2,2)) goto errexit ;
                     if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtm1P,nodeAddrP(dtm2P,fspnt)->sPtr,0))
                       {
                        addflag = 0 ;  
                       }
                    } 
                break ;

                default :
                break  ;
               } ; 
/*
**          Add Feature To Merge Tin
*/
            if( addflag ) 
              {
               dtmFeatureP->dtmFeatureId = dtm1P->dtmFeatureIndex ;
               ++dtm1P->dtmFeatureIndex ;
               if( bcdtmInsert_addDtmFeatureToDtmObject(dtm1P,NULL,0,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,dtmFeatureP->dtmFeatureId,nodeAddrP(dtm2P,fspnt)->sPtr,1) ) goto errexit ; 
               if( bcdtmList_checkConnectivityOfDtmFeatureDtmObject(dtm1P,dtm1P->numFeatures-1,1))
                 {
                  bcdtmList_writePointsForDtmFeatureDtmObject(dtm1P,dtm1P->numFeatures-1) ;
                  goto errexit ;
                 }
               bcdtmList_nullTptrListDtmObject(dtm1P,nodeAddrP(dtm2P,fspnt)->sPtr) ; 
              }
            else if( fspnt != dtm1P->nullPnt ) if( bcdtmList_nullTptrListDtmObject(dtm1P,fspnt)) goto errexit ;
           } 
        }
     }
  } 
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying DTM Features To Merge Dtm Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Copying DTM Features To Merge Dtm Error") ;
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
BENTLEYDTM_Public int bcdtmMerge_detectAndInsertImpliedVoidsDtmObject(BC_DTM_OBJ *dtm1P,BC_DTM_OBJ *dtm2P)
/*
** This Function Detects And Inserts Implied Voids Between The Dtm Hulls
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   p1,p2,p3,p4,tp,np ;
 double x,y,area ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Detecting And Inserting Implied Voids") ;
/*
** Scan Tptr List And Detect Implied Voids
*/
 p1 = nodeAddrP(dtm2P,dtm2P->hullPoint)->sPtr ;
 do
   {
    p2 = nodeAddrP(dtm1P,p1)->tPtr ;
    if( ( p3 = bcdtmList_nextClkDtmObject(dtm1P,p1,p2)) < 0 )  goto errexit ; 
    if( nodeAddrP(dtm1P,p3)->tPtr != p1 )
      { 
       if( ! bcdtmList_testLineDtmObject(dtm1P,p2,p3) )
         {
          p3 = p1 ;
          p4 = p2 ; 
          np = 1  ;
          area = 0.0 ;
          do
            {
             ++np ;
             x  = pointAddrP(dtm1P,p4)->x - pointAddrP(dtm1P,p3)->x  ;
             y  = pointAddrP(dtm1P,p4)->y - pointAddrP(dtm1P,p3)->y  ;
             area = area + x * y / 2.0 + x * pointAddrP(dtm1P,p3)->y ;
             tp = p4 ;
             if(( p4 = bcdtmList_nextAntDtmObject(dtm1P,p4,p3)) < 0 ) goto errexit ; 
             p3 = tp ; 
            } while ( p3 != p1 ) ;
/*
**        Insert Implied Void If Area Detected Between Voids
*/
          if( area > 0.0 ) 
            { 
             if( bcdtmMerge_insertImpliedVoidDtmObject(dtm1P,p1,np))  goto errexit ; 
            } 
         }
      } 
    p1 = p2 ;
   } while ( p1 != nodeAddrP(dtm2P,dtm2P->hullPoint)->sPtr ) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Detecting And Inserting Implied Voids Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Detecting And Inserting Implied Voids Error") ;
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
BENTLEYDTM_Public int bcdtmMerge_insertImpliedVoidDtmObject(BC_DTM_OBJ *dtmP,long point,long numPoints)
/*
** This Function Processes Implied Voids
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    np,sp,pp,cp,rsp,rcp,rnp,numPts ;
 DTMDirection direction;
 DPoint3d     *p3dP,*pointsP=NULL ; 
 double  area,perimeter ;
 BC_DTM_OBJ  *tempDtmP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Inserting Implied Void") ;
    bcdtmWrite_message(0,0,0,"dtmP      = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"point     = %8ld",point) ;
    bcdtmWrite_message(0,0,0,"numPoints = %8ld",numPoints) ;
   }
/*
** Copy Tptr List To Hptr List
*/
 if( bcdtmList_copyTptrListToHptrListDtmObject(dtmP,point)) goto errexit ; 
/*
** Create Dtm Object
*/
 if( bcdtmObject_createDtmObject(&tempDtmP))  goto errexit ;
/*
** Set Memory Allocation Parameters For tempDtmP Object
*/
 if( bcdtmObject_setPointMemoryAllocationParametersDtmObject(tempDtmP,numPoints,numPoints) )  goto errexit ;
/*
** Allocate Memory To Store Void Boundary Points
*/
 pointsP = ( DPoint3d * ) malloc( numPoints * 2 * sizeof(DPoint3d)) ;
 if( pointsP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Store Void Boundary Points
*/
 p3dP = pointsP ;
 sp = point ;
 np = nodeAddrP(dtmP,sp)->tPtr ;
 do
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Implied Void Point = %6ld ** %12.10lf %12.10lf",sp,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y) ;
    p3dP->x = pointAddrP(dtmP,sp)->x ;
    p3dP->y = pointAddrP(dtmP,sp)->y ;
    p3dP->z = ( double ) sp ;
    ++p3dP ;
    if(( pp = bcdtmList_nextAntDtmObject(dtmP,np,sp)) < 0 ) goto errexit ;
    nodeAddrP(dtmP,np)->tPtr = pp ;
    sp = np ;
    np = pp ;
   } while ( sp != point ) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Implied Void Point = %6ld ** %12.10lf %12.10lf",sp,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y) ;
 p3dP->x = pointAddrP(dtmP,sp)->x ;
 p3dP->y = pointAddrP(dtmP,sp)->y ;
 p3dP->z = ( double ) sp ;
 ++p3dP ;
 numPts = (long)(p3dP-pointsP) ;
/*
**  Store Void Points In tempDtmP Object As Break Lines
*/
 if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::Breakline,tempDtmP->nullUserTag,1,&tempDtmP->nullFeatureId,pointsP,numPts)) goto errexit ;
/*
** Triangulate tempDtmP Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating Temporary Dtm Object") ;
 DTM_NORMALISE_OPTION = FALSE ;              // To Inhibit Normalisation Of Coordinates - function 
 tempDtmP->ppTol = tempDtmP->plTol = 0.0 ;  
 if( bcdtmObject_createTinDtmObject(tempDtmP,1,0.0)) goto errexit ;
 DTM_NORMALISE_OPTION = TRUE ;
/*
** Remove None Feature Hull Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing None Feature Tin Hull Lines") ;
 if( bcdtmList_removeNoneFeatureHullLinesDtmObject(tempDtmP)) goto errexit ;
/*
** Insert Internal Lines Into Tptr Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Internal Lines") ;
 pp  = tempDtmP->hullPoint ;
 sp  = nodeAddrP(tempDtmP,pp)->hPtr ;
 rsp = (long)pointAddrP(tempDtmP,sp)->z ;
  do
   { 
    np  = nodeAddrP(tempDtmP,sp)->hPtr ;
    rnp = (long)pointAddrP(tempDtmP,np)->z ;
    if(( cp = bcdtmList_nextClkDtmObject(tempDtmP,sp,pp)) < 0 ) goto errexit ;
    while ( cp != np )
      {
       rcp = (long)pointAddrP(tempDtmP,cp)->z ;
       if( bcdtmList_insertLineBeforePointDtmObject(dtmP,rsp,rcp,rnp)) goto errexit  ;
       if(( cp = bcdtmList_nextClkDtmObject(tempDtmP,sp,cp)) < 0 ) goto errexit  ;
      }
    pp  = sp ;
    sp  = np ;
    rsp = rnp ;
   } while ( pp != tempDtmP->hullPoint ) ;
/*
** Check For Implied Void
*/
 if( bcdtmMath_calculateDirectionAreaAndPerimeterHptrPolygonDtmObject(tempDtmP,tempDtmP->hullPoint,&direction,&area,&perimeter) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Area = %10.4lf Perimeter = %10.4lf  minImpliedVoidArea = %10.4lf Area/Perimeter = %10.4lf areaPerimeterRatio = %10.4lf",area,perimeter,minImpliedVoidArea,area/perimeter,areaPerimeterRatio) ;
 if( area > minImpliedVoidArea && area/perimeter > areaPerimeterRatio )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Adding Implied Void") ;
/*
**  Reverse Implied Void Direction
*/
    if( bcdtmList_reverseTptrPolygonDtmObject(dtmP,point)) goto errexit ;
/*
**  Insert Implied Void Into Dtm Object
*/
    if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,DTMFeatureType::Void,dtmP->nullUserTag,dtmP->nullFeatureId,point,1)) goto errexit ;
   }
/*
** Null Out Tptr List
*/
 if( bcdtmList_nullTptrListDtmObject(dtmP,point)) goto errexit ;
/*
** Copy Hptr List To Tptr List
*/
 if( bcdtmList_copyHptrListToTptrListDtmObject(dtmP,point)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
 DTM_NORMALISE_OPTION = TRUE ;
 if( pointsP  != NULL ) free(pointsP) ;
 if( tempDtmP != NULL ) bcdtmObject_destroyDtmObject(&tempDtmP)  ;  
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Implied Void Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Implied Void Error") ;
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
BENTLEYDTM_Public int bcdtmMerge_checkAndFixPrecisionForTrianglesDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Fixes The Precision Of Internal Triangles
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   p1,p2,p3,clPtr,loop,process,sd1,sd2,sd3,nip,insertLine  ;
 double d12,d13,d23 ;
 DTM_TIN_NODE  *nodeP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking And Fixing Triangle Precision") ;
/*
** Scan Internal Points And Remove Internal Slivers Less Than < Pltol
*/
 loop = 0 ;
 process = 1 ;
 while ( process && loop < 20 )
   {
    nip = 0 ;
    process = 0 ;
    for( p1 = 0  ; p1 < dtmP->numPoints ; ++p1 )
      {
       nodeP = nodeAddrP(dtmP,p1) ;
       if( nodeP->cPtr  != dtmP->nullPtr  ) 
         {
          clPtr = nodeP->cPtr ;
          if(( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
          while ( clPtr != dtmP->nullPtr )
            {
             p3  = clistAddrP(dtmP,clPtr)->pntNum ;
             clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
             if( p2 > p1 && p3 > p1 && nodeAddrP(dtmP,p1)->hPtr != p2 )
               {
                sd1 = bcdtmMath_pointSideOfDtmObject(dtmP,p1,p2,p3) ;
                sd2 = bcdtmMath_pointSideOfDtmObject(dtmP,p2,p3,p1) ;
                sd3 = bcdtmMath_pointSideOfDtmObject(dtmP,p3,p1,p2) ;
                if( sd1 >= 0 || sd2 >= 0 || sd3 >= 0 )
                  {
                   if( dbg ) bcdtmWrite_message(0,0,0,"Precision Error ** Triangle [%6ld,%6ld,%6ld] ** sd1 = %2ld sd2 = %2ld sd3 = %2ld",p1,p2,p3,sd1,sd2,sd3) ;
                   d12 = bcdtmMath_pointDistanceDtmObject(dtmP,p1,p2) ;
                   d13 = bcdtmMath_pointDistanceDtmObject(dtmP,p1,p3) ;
                   d23 = bcdtmMath_pointDistanceDtmObject(dtmP,p2,p3) ;
                   if      ( d12 <= d13 && d12 <= d23 ) 
                     { 
                      insertLine = bcdtmList_testForDtmFeatureLineDtmObject(dtmP,p1,p2) ;
                      if( dbg ) bcdtmWrite_message(0,0,0,"01 ** p1 = %6ld p2 = %6ld p3 = %6ld ** Insert Line = %2ld",p1,p2,p3,insertLine) ;
                      if( bcdtmMerge_fixFeatureListForLastPointDtmObject(dtmP,p2,p1)) goto errexit ;
                      if( bcdtmMerge_copyFeatureListFromPointToPointDtmObject(dtmP,p2,p1) ) goto errexit ;
                      if( bcdtmMerge_connectedPointsDtmObject(dtmP,p2,p1)) goto errexit ; 
                     }
                   else if ( d13 <= d12 && d13 <= d23 ) 
                     { 
                      insertLine = bcdtmList_testForDtmFeatureLineDtmObject(dtmP,p1,p3) ;
                      if( dbg ) bcdtmWrite_message(0,0,0,"02 ** p1 = %6ld p2 = %6ld p3 = %6ld ** Insert Line = %2ld",p1,p2,p3,insertLine) ;
                      if( bcdtmMerge_fixFeatureListForLastPointDtmObject(dtmP,p3,p1)) goto errexit ;
                      if( bcdtmMerge_copyFeatureListFromPointToPointDtmObject(dtmP,p3,p1) ) goto errexit ;
                      if( bcdtmMerge_connectedPointsDtmObject(dtmP,p3,p1)) goto errexit ; 
                     }
                   else if ( d23 <= d12 && d23 <= d13 ) 
                     { 
                      insertLine = bcdtmList_testForDtmFeatureLineDtmObject(dtmP,p2,p3) ;
                      if( dbg ) bcdtmWrite_message(0,0,0,"02 ** p1 = %6ld p2 = %6ld p3 = %6ld ** Insert Line = %2ld",p1,p2,p3,insertLine) ;
                      if( bcdtmMerge_fixFeatureListForLastPointDtmObject(dtmP,p3,p2)) goto errexit ;
                      if( bcdtmMerge_copyFeatureListFromPointToPointDtmObject(dtmP,p3,p2) ) goto errexit ;
                      if( bcdtmMerge_connectedPointsDtmObject(dtmP,p3,p2)) goto errexit ; 
                     }
                   clPtr = dtmP->nullPtr ;
//TODO                   --node ;
                   process = 1 ;
                   ++nip ;
                  } 
               }
             p2 = p3 ;
            }
         }
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"Loop = %6ld  Nip = %6ld",loop,nip) ;
    ++loop ;
  } 
/*
** Check Topology
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Dtm Topology") ;
    if( bcdtmCheck_topologyDtmObject(dtmP,0)) 
      { 
       bcdtmWrite_message(1,0,0,"Dtm Topology Corrupted While Adjusting Precision Of Triangles ") ;
       goto errexit ; 
      }
    else  bcdtmWrite_message(0,0,0,"Topology OK") ;
    bcdtmWrite_message(0,0,0,"Checking Dtm Feature Topology") ;
    if( bcdtmCheck_topologyDtmFeaturesDtmObject(dtmP,1) )
      { 
       bcdtmWrite_message(1,0,0,"Feature Topology Corrupted While Adjusting Precision Of Triangle ") ;
       goto errexit ;
      }
    else  bcdtmWrite_message(0,0,0,"Feature Topology OK") ;
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
BENTLEYDTM_Public int bcdtmMerge_writeTinLinesDtmObject(BC_DTM_OBJ *dtmP,WCharCP dataFileP)
/*
** This Function Writes The Dtm Lines To A Data File As Break Lines
*/
{
 int   ret=DTM_SUCCESS ;
 long  p1,p2,clPtr ;
 DPoint3d   linePts[2] ;
 BC_DTM_OBJ *dataP=NULL ;
 DTM_CIR_LIST *clistP ;
 DTM_TIN_POINT *p1P,*p2P ;
/*
** Create Data Object
*/
 if( bcdtmObject_createDtmObject(&dataP)) goto errexit ;
 bcdtmObject_setPointMemoryAllocationParametersDtmObject(dataP,dtmP->numPoints*6,1000) ;
/*
** Store Dtm Lines In Data Object As Breaks
*/
 for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
   {
    if( ( clPtr = nodeAddrP(dtmP,p1)->cPtr ) != dtmP->nullPtr ) 
      {
       while ( clPtr != dtmP->nullPtr )
         {
          p1P = pointAddrP(dtmP,p1) ;
          linePts[0].x = p1P->x ;
          linePts[0].y = p1P->y ;
          linePts[0].z = p1P->z ;
          clistP = clistAddrP(dtmP,clPtr) ;
          p2     = clistP->pntNum ;
          clPtr  = clistP->nextPtr ; 
          if( p2 > p1 )
            {
             p2P = pointAddrP(dtmP,p2) ;
             linePts[1].x = p2P->x ;
             linePts[1].y = p2P->y ;
             linePts[1].z = p2P->z ;
             if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::TinLine,dtmP->nullUserTag,1,&dtmP->nullFeatureId,linePts,2)) goto errexit ;
            }
         }
      }
   }
/*
** Write Data Object
*/
 if( bcdtmWrite_toFileDtmObject(dataP,dataFileP)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
 bcdtmObject_destroyDtmObject(&dataP) ;  
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
BENTLEYDTM_Public int bcdtmMerge_checkPointCanBeMergedDtmObject(BC_DTM_OBJ *dtmP,long pnt1,long pnt2,long *mergeFlagP)
/*
** This Function Checks If Point pnt2 Can Be Merged With pnt1 And Lines Connected To
** pnt2 Can be Connected To pnt1
*/
{
 int  ret=DTM_SUCCESS,sideof ;
 long dbg=DTM_TRACE_VALUE(0),sp,np,scanPnt,fPtr,nextPnt,priorPnt,pnt1OnFeature ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking If Point %9ld Can Be Merged With Point %9ld",pnt2,pnt1) ;
/*
** Initialise
*/
 *mergeFlagP = 1 ;
/*
** Get Start Circular Scan Point About pnt2 From pnt1
*/
 if( ( scanPnt = bcdtmList_nextAntDtmObject(dtmP,pnt2,pnt1)) < 0 ) goto errexit ;
/*
** Scan About pnt2 And Check Precision
*/
 sp = scanPnt ;
 if( (np = bcdtmList_nextAntDtmObject(dtmP,pnt2,sp)) < 0 ) goto errexit ;
 while ( np != pnt1 && *mergeFlagP )
   {
    sideof = bcdtmMath_pointSideOfDtmObject(dtmP,pnt1,sp,np) ;
    if( sideof <= 0 ) *mergeFlagP = 0 ;
    sp = np ;
    if( (np = bcdtmList_nextAntDtmObject(dtmP,pnt2,sp)) < 0 ) goto errexit ;
   }
/*
** Check Merging Points Wont Cause A Knot In Feature Lists
*/
 if( *mergeFlagP )
   {
    fPtr = nodeAddrP(dtmP,pnt2)->fPtr ;
    while( fPtr != dtmP->nullPtr && *mergeFlagP )
      {
       bcdtmList_testForPointOnTinFeatureDtmObject(dtmP,flistAddrP(dtmP,fPtr)->dtmFeature,pnt1,&pnt1OnFeature) ;
       if( pnt1OnFeature )
         { 
          if( bcdtmList_getPriorPointForDtmFeatureDtmObject(dtmP,flistAddrP(dtmP,fPtr)->dtmFeature,pnt2,&priorPnt)) goto errexit  ;
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,flistAddrP(dtmP,fPtr)->dtmFeature,pnt2,&nextPnt)) goto errexit  ;
          if( pnt1 != nextPnt && pnt1 != priorPnt ) *mergeFlagP = 0 ;
         }
       fPtr = flistAddrP(dtmP,fPtr)->nextPtr ;
      } 
   }
   
/*
** Job Completed
*/
 cleanup :
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Checking If Point %9ld Can Be Merged With Point %9ld Completed",pnt2,pnt1) ;
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Checking If Point %9ld Can Be Merged With Point %9ld Error",pnt2,pnt1) ;
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
BENTLEYDTM_Public int bcdtmMerge_connectedPointsDtmObject
(
 BC_DTM_OBJ *dtmP,
 long       mrgPnt1,
 long       mrgPnt2
)
{
/*
** This Function Merges Point mrgPnt1 To mrgPnt2
*/
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long np,sp,p1,p2,clc,startPnt,endPnt,hullPnt ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Merging Connected Points mrgPnt2 = %6ld mrgPnt1 = %6ld",mrgPnt2,mrgPnt1) ; 
    bcdtmWrite_message(0,0,0,"mrgPnt2 = %6ld Fptr = %9ld  ** %20.15lf %20.15lf %10.4lf",mrgPnt2,nodeAddrP(dtmP,mrgPnt2)->hPtr,pointAddrP(dtmP,mrgPnt2)->x,pointAddrP(dtmP,mrgPnt2)->y,pointAddrP(dtmP,mrgPnt2)->z ) ;
    bcdtmWrite_message(0,0,0,"mrgPnt1 = %6ld Fptr = %9ld  ** %20.15lf %20.15lf %10.4lf",mrgPnt1,nodeAddrP(dtmP,mrgPnt1)->hPtr,pointAddrP(dtmP,mrgPnt1)->x,pointAddrP(dtmP,mrgPnt1)->y,pointAddrP(dtmP,mrgPnt1)->z ) ;
    bcdtmList_writeCircularListForPointDtmObject(dtmP,mrgPnt2) ;
    bcdtmList_writeCircularListForPointDtmObject(dtmP,mrgPnt1) ;
   }
/*
** Initialise
*/
 if( nodeAddrP(dtmP,mrgPnt2)->hPtr == dtmP->nullPnt || nodeAddrP(dtmP,mrgPnt1)->hPtr == dtmP->nullPnt ) hullPnt = dtmP->nullPnt ;
 else                                                                                                   hullPnt = 1 ;
 if( hullPnt != dtmP->nullPnt )
   {
    if( nodeAddrP(dtmP,mrgPnt2)->hPtr != mrgPnt1 && nodeAddrP(dtmP,mrgPnt1)->hPtr != mrgPnt2 )
      { nodeAddrP(dtmP,mrgPnt1)->tPtr = dtmP->nullPnt ; return(0) ; }
    if( nodeAddrP(dtmP,mrgPnt1)->hPtr != mrgPnt2 ) { sp = mrgPnt2 ; mrgPnt2 = mrgPnt1 ; mrgPnt1 = sp ; }
    if(( hullPnt = bcdtmList_nextClkDtmObject(dtmP,mrgPnt1,mrgPnt2)) < 0 ) goto errexit ; 
   }
 else
   {
    if( nodeAddrP(dtmP,mrgPnt1)->hPtr != dtmP->nullPnt ) { sp = mrgPnt2 ; mrgPnt2 = mrgPnt1 ; mrgPnt1 = sp ; }
   }
/*
** Get Circular Points About mrgPnt2 From mrgPnt1
*/
 if( ( startPnt = bcdtmList_nextClkDtmObject(dtmP,mrgPnt1,mrgPnt2)) < 0 ) goto errexit ;
 if( ( endPnt = bcdtmList_nextAntDtmObject(dtmP,mrgPnt1,mrgPnt2))   < 0 ) goto errexit ;
/*
** Set Points Connected To mrgPnt1 In tPtr Array
*/
 sp = startPnt ;
 if( ( np = bcdtmList_nextClkDtmObject(dtmP,mrgPnt1,sp)) < 0 ) goto errexit ;
 while ( np !=  mrgPnt2 )
   {
    nodeAddrP(dtmP,sp)->tPtr = np ;
    sp = np ;
    if( ( np = bcdtmList_nextClkDtmObject(dtmP,mrgPnt1,sp)) < 0 ) goto errexit ;
   }
/*
** Insert Hull Point Into List
*/
 if( hullPnt != dtmP->nullPnt )
   {
    if(( startPnt = bcdtmList_nextAntDtmObject(dtmP,mrgPnt2,mrgPnt1)) < 0 ) goto errexit ;
    nodeAddrP(dtmP,startPnt)->tPtr = hullPnt ;
   }
/*
** Delete Lines Already Connected To mrgPnt2
*/
 sp = startPnt ;
 while ( ( np = nodeAddrP(dtmP,sp)->tPtr ) != endPnt )
   {
    if( bcdtmList_testLineDtmObject(dtmP,mrgPnt2,np))
      {
       if(( p1 = bcdtmList_nextAntDtmObject(dtmP,mrgPnt2,np))   < 0 ) goto errexit ;
       if(( p2 = bcdtmList_nextClkDtmObject(dtmP,mrgPnt2,np)) < 0 ) goto errexit ;
       if( bcdtmList_deleteLineDtmObject(dtmP,mrgPnt2,np)) goto errexit ; 
       if( bcdtmList_insertLineBeforePointDtmObject(dtmP,p1,p2,mrgPnt2))  goto errexit ;
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p2,p1,mrgPnt2)) goto errexit ; 
      }
    sp = np ; 
   }
/*
** Delete Point mrgPnt1
*/
 clc = nodeAddrP(dtmP,mrgPnt1)->cPtr ;
 while ( clc != dtmP->nullPtr )
   {
    np  = clistAddrP(dtmP,clc)->pntNum ;
    clc = clistAddrP(dtmP,clc)->nextPtr ;
    if( bcdtmList_deleteLineDtmObject(dtmP,mrgPnt1,np)) goto errexit ;
   } 
 nodeAddrP(dtmP,mrgPnt1)->hPtr = dtmP->nullPnt ;
 nodeAddrP(dtmP,mrgPnt1)->tPtr = dtmP->nullPnt ;
 nodeAddrP(dtmP,mrgPnt1)->cPtr = dtmP->nullPtr ;
 nodeAddrP(dtmP,mrgPnt1)->fPtr = dtmP->nullPtr ;
/*
** Connect Deleted Lines To mrgPnt2
*/
 sp = startPnt ;
 while ( ( np = nodeAddrP(dtmP,sp)->tPtr ) != endPnt )
   {
    if( bcdtmList_insertLineAfterPointDtmObject(dtmP,mrgPnt2,np,sp))  goto errexit ;
    if ( np == hullPnt ) {  if( bcdtmList_insertLineAfterPointDtmObject(dtmP,np,mrgPnt2,nodeAddrP(dtmP,np)->tPtr)) goto errexit ; }
    else            
      {  
       if( bcdtmList_insertLineBeforePointDtmObject(dtmP,np,mrgPnt2,sp)) goto errexit ; 
      }
    nodeAddrP(dtmP,sp)->tPtr = dtmP->nullPnt ;
    sp = np ; 
   }
/*
** Reset Hull Pointer
*/
 if( hullPnt != dtmP->nullPnt ) nodeAddrP(dtmP,hullPnt)->hPtr = mrgPnt2 ;
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
BENTLEYDTM_Private int bcdtmMerge_removeTrianglesOnInsertHullDtmObject
(
 BC_DTM_OBJ *dtmP,                 /* ==> Pointer To Dtm Object          */
 long   startPnt                  /* ==> Start Point On Tptr Polygon    */
)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long sPnt,nPnt,cPnt,ncPnt,scan  ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Triangles On Inserted Hull") ;
/*
** Initialise
*/
 sPnt = startPnt ;
 do
   {
    nPnt = nodeAddrP(dtmP,sPnt)->tPtr ;
    if( ( cPnt = bcdtmList_nextClkDtmObject(dtmP,sPnt,nPnt)) < 0 ) goto errexit ;
/*
**  One Triangle At sPnt
*/
    if( nodeAddrP(dtmP,cPnt)->tPtr == sPnt )
      {
       if( bcdtmList_testLineDtmObject(dtmP,cPnt,nPnt) )
         {
          if( ! bcdtmList_testForDtmFeatureLineDtmObject(dtmP,cPnt,nPnt) )
            { 
             if( ( ncPnt = bcdtmList_nextClkDtmObject(dtmP,cPnt,nPnt) ) < 0 ) goto errexit ;
             if( bcdtmList_testLineDtmObject(dtmP,ncPnt,nPnt) )
               {
                if( bcdtmMath_pointSideOfDtmObject(dtmP,sPnt,ncPnt,cPnt) < 0 &&
                    bcdtmMath_pointSideOfDtmObject(dtmP,sPnt,ncPnt,nPnt) > 0     )
                  {
                   if( dbg )
                     {
                      bcdtmWrite_message(0,0,0,"Swapping Line cPnt-nPnt") ;
                      bcdtmWrite_message(0,0,0,"sPnt  ** %8ld ** %12.5lf %12.5lf %10.4lf",sPnt,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y,pointAddrP(dtmP,sPnt)->z) ;
                      bcdtmWrite_message(0,0,0,"nPnt  ** %8ld ** %12.5lf %12.5lf %10.4lf",nPnt,pointAddrP(dtmP,nPnt)->x,pointAddrP(dtmP,nPnt)->y,pointAddrP(dtmP,nPnt)->z) ;
                      bcdtmWrite_message(0,0,0,"cPnt  ** %8ld ** %12.5lf %12.5lf %10.4lf",cPnt,pointAddrP(dtmP,cPnt)->x,pointAddrP(dtmP,cPnt)->y,pointAddrP(dtmP,cPnt)->z) ;
                      bcdtmWrite_message(0,0,0,"ncPnt ** %8ld ** %12.5lf %12.5lf %10.4lf",ncPnt,pointAddrP(dtmP,ncPnt)->x,pointAddrP(dtmP,ncPnt)->y,pointAddrP(dtmP,ncPnt)->z) ;
                     }
                   if( bcdtmList_deleteLineDtmObject(dtmP,cPnt,nPnt)) goto errexit ;
                   if( bcdtmList_insertLineAfterPointDtmObject(dtmP,sPnt,ncPnt,nPnt)) goto errexit ;
                   if( bcdtmList_insertLineAfterPointDtmObject(dtmP,ncPnt,sPnt,cPnt)) goto errexit ;
                  }
               }
            }
         }
      }
/*
**  Multiple Triangles At sPnt       
*/
    else
      {
       scan = 1 ;
       while ( scan )
         {
          scan = 0 ;            
          nPnt = nodeAddrP(dtmP,sPnt)->tPtr ;
          if( ( cPnt = bcdtmList_nextClkDtmObject(dtmP,sPnt,nPnt)) < 0 ) goto errexit ;
          while ( nodeAddrP(dtmP,cPnt)->tPtr != sPnt )  
            {
             if( dbg )
               {
                bcdtmWrite_message(0,0,0,"***********************") ;
                bcdtmWrite_message(0,0,0,"sPnt  ** %8ld ** %12.5lf %12.5lf %10.4lf",sPnt,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y,pointAddrP(dtmP,sPnt)->z) ;
                bcdtmWrite_message(0,0,0,"nPnt  ** %8ld ** %12.5lf %12.5lf %10.4lf",nPnt,pointAddrP(dtmP,nPnt)->x,pointAddrP(dtmP,nPnt)->y,pointAddrP(dtmP,nPnt)->z) ;
                bcdtmWrite_message(0,0,0,"cPnt  ** %8ld ** %12.5lf %12.5lf %10.4lf",cPnt,pointAddrP(dtmP,cPnt)->x,pointAddrP(dtmP,cPnt)->y,pointAddrP(dtmP,cPnt)->z) ;
               }
             if( nodeAddrP(dtmP,cPnt)->tPtr != dtmP->nullPnt )
               {
                if( bcdtmList_testLineDtmObject(dtmP,cPnt,nPnt) )
                  { 
                   if( ( ncPnt = bcdtmList_nextClkDtmObject(dtmP,sPnt,cPnt)) < 0 ) goto errexit ;
                   if( dbg )   bcdtmWrite_message(0,0,0,"ncPnt ** %8ld ** %12.5lf %12.5lf %10.4lf",ncPnt,pointAddrP(dtmP,ncPnt)->x,pointAddrP(dtmP,ncPnt)->y,pointAddrP(dtmP,ncPnt)->z) ;
                   if( nodeAddrP(dtmP,ncPnt)->tPtr == dtmP->nullPnt )
                     { 
                      if( bcdtmList_testLineDtmObject(dtmP,ncPnt,cPnt) )
                        {
                         if( ! bcdtmList_testForDtmFeatureLineDtmObject(dtmP,cPnt,nPnt) )
                           {
                            if( bcdtmMath_pointSideOfDtmObject(dtmP,nPnt,ncPnt,sPnt) < 0 &&
                                bcdtmMath_pointSideOfDtmObject(dtmP,nPnt,ncPnt,cPnt) > 0     )
                              {
                               if( dbg )
                                 {
                                  bcdtmWrite_message(0,0,0,"Swapping Lines") ;
                                  bcdtmWrite_message(0,0,0,"sPnt  ** %8ld ** %12.5lf %12.5lf %10.4lf",sPnt,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y,pointAddrP(dtmP,sPnt)->z) ;
                                  bcdtmWrite_message(0,0,0,"nPnt  ** %8ld ** %12.5lf %12.5lf %10.4lf",nPnt,pointAddrP(dtmP,nPnt)->x,pointAddrP(dtmP,nPnt)->y,pointAddrP(dtmP,nPnt)->z) ;
                                  bcdtmWrite_message(0,0,0,"cPnt  ** %8ld ** %12.5lf %12.5lf %10.4lf",cPnt,pointAddrP(dtmP,cPnt)->x,pointAddrP(dtmP,cPnt)->y,pointAddrP(dtmP,cPnt)->z) ;
                                  bcdtmWrite_message(0,0,0,"ncPnt ** %8ld ** %12.5lf %12.5lf %10.4lf",ncPnt,pointAddrP(dtmP,ncPnt)->x,pointAddrP(dtmP,ncPnt)->y,pointAddrP(dtmP,ncPnt)->z) ;
                                 }
                               if( bcdtmList_deleteLineDtmObject(dtmP,sPnt,cPnt)) goto errexit ;
                               if( bcdtmList_insertLineAfterPointDtmObject(dtmP,nPnt,ncPnt,cPnt)) goto errexit ;
                               if( bcdtmList_insertLineAfterPointDtmObject(dtmP,ncPnt,nPnt,sPnt)) goto errexit ;
                               cPnt = ncPnt ;
                               scan = 1 ;
                              }
                           }
                        } 
                     }
                  }
               } 
             nPnt = cPnt ;
             if( ( cPnt = bcdtmList_nextClkDtmObject(dtmP,sPnt,cPnt)) < 0 ) goto errexit ;
            } 
         }
      }
    sPnt = nodeAddrP(dtmP,sPnt)->tPtr ;  
   } while ( sPnt != startPnt ) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Triangles On Inserted Hull Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Triangles On Inserted Hull Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
