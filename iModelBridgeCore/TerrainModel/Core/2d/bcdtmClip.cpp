/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmClip.cpp $
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
BENTLEYDTM_EXPORT int bcdtmClip_toPolygonDtmFile
(
 WCharCP dtmFileP,
 WCharCP clipDtmFileP,
 DPoint3d *clipPtsP,
 long     numClipPts,
 DTMClipOption  clipOption
)
/*
** This the Controlling Function For Clipping a Tin File
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 BC_DTM_OBJ *dtmP=NULL ;
/*
** Initialise Variables
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Clipping Dtm File To Polygon")  ;
/*
** Test If Requested Dtm File Is Current Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Reading Dtm File %ws",dtmFileP)  ;
 if( bcdtmUtility_testForAndSetCurrentDtmObject(&dtmP,dtmFileP)) goto errexit ;
/*
** Clip Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Clipping Dtm Object %p",dtmP)  ;
 if( bcdtmClip_toPolygonDtmObject(dtmP,clipPtsP,numClipPts,clipOption)) goto errexit ;
/*
** Check Clipped DTM
*/
 if( cdbg )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Clipped Dtm Object %p",dtmP) ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP) )
      {
       bcdtmWrite_message(2,0,0,"Dtm Object Corrupted") ;
       goto errexit ;
      } 
    else bcdtmWrite_message(2,0,0,"Dtm Object OK") ;
   }
/*
** Write Clipped Dtm File 
*/
 if( clipDtmFileP[0] != 0 ) 
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Writing Clipped Dtm Object %p To File %s",dtmP,clipDtmFileP)  ;
    if( bcdtmWrite_toFileDtmObject(dtmP,clipDtmFileP)) goto errexit ; 
   }
/*
** Reset Name Of Current DTM Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Resetting Name Of Current Dtm Object File To %s",clipDtmFileP)  ;
 //wcscpy(DTM_CDTM_FILE,clipDtmFileP) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Clipping Dtm File To Polygon Completed")  ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Clipping Dtm File To Polygon Error")  ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( dtmP != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmClip_toPolygonDtmObject
(
 BC_DTM_OBJ *dtmP,               /* ==> Pointer To Dtm Object                       */
 DPoint3d *ptsP,                 /* ==> Pointer To Clipping Polygon Points          */
 long numPts,                    /* ==> Number Of Points In Clipping Polygon        */
 DTMClipOption clipOption                 /* ==> Clipping Option <DTMClipOption::Internal,DTMClipOption::External> */
)
/*
** This Function Clips a Dtm Object To A DPoint3d Polygon
*/
{
 int              ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long     polyOffset,startPnt=0,intersectFlag,numClipPts,numHullPts,dtmFeature  ;
 DTMDirection     direction;
 long             clipTime=bcdtmClock(),startTime ;
 double           area,savePpTol=0.0,savePlTol=0.0  ;
 DPoint3d         *p3dP,*clipPtsP=NULL,*hullPtsP=NULL ;
 DTM_POLYGON_OBJ  *polyP=NULL ;
 DTM_POLYGON_LIST *polyListP ;
 BC_DTM_FEATURE   *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Clipping Dtm Object To Polygon") ; 
    bcdtmWrite_message(0,0,0,"dtmP         = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"ptsP         = %p",ptsP) ;
    bcdtmWrite_message(0,0,0,"numPts       = %8ld",numPts) ;
    bcdtmWrite_message(0,0,0,"clipOption   = %8ld",clipOption) ;
    bcdtmWrite_message(0,0,0,"dtmP->ppTol  = %20.16lf",dtmP->ppTol) ;
    bcdtmWrite_message(0,0,0,"dtmP->plTol  = %20.16lf",dtmP->plTol) ;
    bcdtmWrite_message(0,0,0,"dtmP->mppTol = %20.16lf",dtmP->mppTol) ;
    if( dbg == 2 )
      {
/*
**     Write Clipping Polygon Points To Log File
*/
       for( p3dP = ptsP ; p3dP < ptsP + numPts ; ++p3dP ) 
         {
          bcdtmWrite_message(0,0,0,"Clip Polygon Point [%4ld] = %10.4lf %10.4lf %10.4lf",(long)(p3dP-ptsP),p3dP->x,p3dP->y,p3dP->z) ;
         }
/*
**     Write Clipping Polygon To XYZ File
*/
       if( bcdtmWrite_binaryFileP3D(L"clipPoly.xyz",ptsP,numPts)) goto errexit ;
/*
**     Write Clipping Polygon To Dat File 
*/
       bcdtmWrite_dataFileFromP3DArray(L"clipPoly.dat",ptsP,numPts,DTMFeatureType::Breakline) ;
      } 
   }
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
** Check If DTM Is In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"DTM Object %p Not Triangulated",dtmP) ;
    goto errexit ;
   }
/*
** Test For Valid Clip Option
*/
 if( clipOption != DTMClipOption::Internal && clipOption != DTMClipOption::External) 
   { 
    bcdtmWrite_message(2,0,0,"Invalid Clip Option") ;
    goto errexit ; 
   }
/*
** Check User Clip Polygon
*/
 if( ptsP == NULL || numPts <= 0 ) 
   { 
    bcdtmWrite_message(1,0,0,"Error In Clip Polygon") ;
    goto errexit ;
   }
/*
** Test For Pre 98 Dtm File
*/
 if( dtmP->ppTol == 0.0 ) 
   { 
    bcdtmWrite_message(1,0,0,"Pre 1998 Dtm - Convert Tin") ;
    ret = 20 ;
    goto errexit ;
   }
/*
** Log All Features
*/
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Number Of DTM Features = %8ld",dtmP->numFeatures) ;
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       bcdtmWrite_message(0,0,0,"dtmFeature[%8ld] ** Type = %4ld  State = %4ld",dtmFeature,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmFeatureState) ;
      } 
   } 
/*
** Delete All RollBack And Tin Error Features
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Deleting All Roll Back And Tin Error Features") ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::TinError || dtmFeatureP->dtmFeatureState == DTMFeatureState::Rollback )
      {
       if( dtmFeatureP->dtmFeaturePts.pointsPI != 0)
         {
          bcdtmMemory_free(dtmP, dtmFeatureP->dtmFeaturePts.pointsPI) ;
          dtmFeatureP->dtmFeaturePts.pointsPI = 0 ;
         }
       dtmFeatureP->dtmFeatureState = DTMFeatureState::Deleted ; 
      }
   } 
/*
**  Check Tolerance Settings
*/
 savePpTol = dtmP->ppTol ; 
 savePlTol = dtmP->plTol ; 
 if( dtmP->ppTol > dtmP->mppTol * 10000.0 ) dtmP->ppTol = dtmP->plTol = dtmP->mppTol * 10000.0 ;
 if( dtmP->plTol > dtmP->mppTol * 10000.0 ) dtmP->ppTol = dtmP->plTol = dtmP->mppTol * 10000.0 ;
/*
** Write Dtm Statistics
*/
 if( cdbg == 2 ) bcdtmUtility_writeStatisticsDtmObject(dtmP) ;
/*
** Create A Local Copy Of The User Clip Polygon
*/
 numClipPts = numPts ;
 if( bcdtmUtl_copy3DTo3D(ptsP,numClipPts,&clipPtsP)) goto errexit ;
/*
** Validate Clip Polgon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Validating Clip Polygon") ;
 if( bcdtmClean_validatePointArrayPolygon(&clipPtsP,&numClipPts,1,dtmP->ppTol) ) goto errexit ; 
/*
**  Extract Dtm Hull
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Extracting Dtm Hull") ;
 if( bcdtmList_extractHullDtmObject(dtmP,&hullPtsP,&numHullPts)) goto errexit ;
/*
** Intersect Dtm Hull And Clip Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting Dtm Hull And Clip Polygon") ;
 startTime = bcdtmClock() ;
 if( bcdtmPolygon_intersectPolygons(hullPtsP,numHullPts,clipPtsP,numClipPts,&intersectFlag,&polyP,dtmP->ppTol,dtmP->plTol)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Intersect Flag = %1ld Number Of Polygons = %4ld",intersectFlag,polyP->numPolygons) ;
 if( intersectFlag == 0 ) { bcdtmWrite_message(1,0,0,"Clip Polygon and Dtm Hull Do Not Intersect") ; goto errexit ; }
 if( tdbg ) bcdtmWrite_message(0,0,0,"**** Time To Intersect Dtm Hull And Clip Polygon = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Free UnWanted Memory
*/
 if( hullPtsP != NULL ) { free(hullPtsP) ; hullPtsP = NULL ; }
 if( clipPtsP != NULL ) { free(clipPtsP) ; clipPtsP = NULL ; }
/*
** If Clip Polygon Intersects Dtm Hull Or Is Internal To Dtm Hull Then Clip Tin
*/
 if( intersectFlag == 1 || intersectFlag == 3 ) 
   {
/*
**  If More Than One Intersect Polygon Get And Use Largest Area Intersect Polygon For Clip
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Intersect Polygons = %2ld",polyP->numPolygons) ;
    polyOffset = 0 ;
    area = (polyP->polyListP)->area ; 
    if( dbg ) bcdtmWrite_message(0,0,0,"Polygon[  0] Area = %12.4lf",area) ;
    for( polyListP = polyP->polyListP + 1 ; polyListP < polyP->polyListP + polyP->numPolygons ; ++polyListP )
      { 
       if( dbg ) bcdtmWrite_message(0,0,0,"Polygon[%3ld] Area = %12.4lf",(long)(polyListP-polyP->polyListP),polyListP->area) ;
       if( polyListP->area > area ) { polyOffset = (long)(polyListP-polyP->polyListP) ; area = polyListP->area ; } 
      }
/*
**  Get Clip Polygon Vertices
*/
    if( bcdtmPolygon_copyPolygonObjectPolygonToPointArrayPolygon(polyP,polyOffset,&clipPtsP,&numClipPts)) goto errexit ;
    if( cdbg )
      {
       bcdtmMath_getPolygonDirectionP3D(clipPtsP,numClipPts,&direction,&area) ;
       bcdtmWrite_message(0,0,0,"Clipping Polygon Direction = %12ld",direction) ;
       bcdtmWrite_message(0,0,0,"Clipping Polygon Area      = %12.4lf",area) ;
       bcdtmMath_calculateAreaAndDirectionHptrPolygonDtmObject(dtmP,dtmP->hullPoint,&area,&direction) ;
       bcdtmWrite_message(0,0,0,"Tin Hull Area              = %12.4lf",area) ;
       for( p3dP = clipPtsP ; p3dP < clipPtsP +numClipPts ; ++p3dP ) 
         {
          bcdtmWrite_message(0,0,0,"Clip Point[%5ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-clipPtsP),p3dP->x,p3dP->y,p3dP->z) ;
         }
      }
/*
**  Insert Clip Polygon Into Tin
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Clip Polygon Into Dtm Object") ;
/*
**  Check Tolerance Settings
*/
    savePpTol = dtmP->ppTol ; 
    savePlTol = dtmP->plTol ; 
    if( dtmP->ppTol > dtmP->mppTol * 10000.0 ) dtmP->ppTol = dtmP->plTol = dtmP->mppTol * 10000.0 ;
    if( dtmP->plTol > dtmP->mppTol * 10000.0 ) dtmP->ppTol = dtmP->plTol = dtmP->mppTol * 10000.0 ;
/*
**  Insert Clipping Polygon Into Dtm
*/
    startTime = bcdtmClock() ;
    if( bcdtmInsert_internalStringIntoDtmObject(dtmP,1,2,clipPtsP,numClipPts,&startPnt)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Clip Polygon Into Dtm Object Completed ** startPnt = %6ld",startPnt) ;
    if( tdbg ) bcdtmWrite_message(0,0,0,"**** Time To Insert Clip Polygon = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;

    if( dtmP->extended && dtmP->extended->rollBackInfoP && bcdtmObject_testApiCleanUpDtmObject (dtmP, DTMCleanupFlags::Changes) && bcdtmInsert_rollBackDtmFeaturesExternalToTptrPolygonDtmObject(dtmP,startPnt)) goto errexit ;
/*
**  Check DTM After Inserting Internal String
*/
    if( cdbg )
      {
/*
**     Check Connectivity Of Tptr Polygon
*/
       bcdtmWrite_message(0,0,0,"Checking Connectivity Tptr Polygon") ;
       if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,startPnt,0)) 
         {
          bcdtmWrite_message(2,0,0,"Connectivity Errors In Tptr Polygon") ;
          goto errexit ;
         }
       bcdtmWrite_message(0,0,0,"No Connectivity Errors In Tptr Polygon") ;
/*
**     Check Area And Direction Of Inserted Clipping Polygon
*/
       bcdtmWrite_message(0,0,0,"Checking Area And Direction Of Inserted Clipping Polygon") ;
       if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPnt,&area,&direction)) goto errexit ;
       bcdtmWrite_message(0,0,0,"Tptr Polygon Direction     = %12ld",direction) ;
       bcdtmWrite_message(0,0,0,"Tptr Polygon Area          = %12.4lf",area) ;
/*
**     Check Dtm After Inserting Clipping Polygon
*/
       bcdtmWrite_message(0,0,0,"Checking Dtm After Inserting Clip Polygon") ;
       if( bcdtmCheck_tinComponentDtmObject(dtmP)) { bcdtmWrite_message(0,0,0,"DTM Corrupted") ; goto errexit ; }
       else                                          bcdtmWrite_message(0,0,0,"DTM OK") ;
      }
/*
**  Free Memory
*/
    free(clipPtsP) ;
    clipPtsP = NULL  ; 
/*
**  Clip Dtm Object
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Clipping Dtm Object To Tptr Polygon") ;
    startTime = bcdtmClock() ;
    if( bcdtmClip_toTptrPolygonDtmObject(dtmP,startPnt,clipOption)) goto errexit ;
    if( tdbg ) bcdtmWrite_message(0,0,0,"**** Time To Clip To Tptr Polygon = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
**  Check Dtm Integrity After Clipping Polygon
*/
    if( cdbg ) 
      { 
       if( dbg ) bcdtmWrite_message(0,0,0,"Checking Dtm After Clipping Dtm") ;
       if( bcdtmCheck_tinComponentDtmObject(dtmP)) { bcdtmWrite_message(0,0,0,"DTM Corrupted") ; goto errexit ; }
       else                                          bcdtmWrite_message(0,0,0,"DTM OK") ;
      }
/*
**  Check Area Of Clipped Dtm Hull - Development Only
*/
    if( cdbg )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Checking Area And Direction Of Clipped Tin Hull") ;
       if( bcdtmMath_calculateAreaAndDirectionHptrPolygonDtmObject(dtmP,0,&area,&direction)) goto errexit ;
       bcdtmWrite_message(0,0,0,"Tin Hull Direction         = %12ld",direction) ;
       bcdtmWrite_message(0,0,0,"Tin Hull Area              = %12.4lf",area) ;
      }
/*
**  Write Dtm Statistics
*/
    if( cdbg == 2 ) bcdtmUtility_writeStatisticsDtmObject(dtmP) ;
   }
/*
**  Update Modified Time
 */
   bcdtmObject_updateLastModifiedTime (dtmP) ;
/*
** Clean Up
*/
 cleanup :
 if( hullPtsP != NULL ) { free(hullPtsP) ; hullPtsP = NULL ; }
 if( clipPtsP != NULL ) { free(clipPtsP) ; clipPtsP = NULL ; }
 if( polyP    != NULL ) bcdtmPolygon_deletePolygonObject(&polyP) ;
 if( savePpTol != 0.0 ) dtmP->ppTol = savePpTol ;
 if( savePlTol != 0.0 ) dtmP->plTol = savePlTol ;
/*
** Job Completed
*/
 if( tdbg ) bcdtmWrite_message(0,0,0,"**** Time To Clip Dtm Object = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),clipTime)) ;
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Clipping Dtm Object To Polygon Completed") ; 
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Clipping Dtm Object To Polygon Error") ; 
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
BENTLEYDTM_EXPORT int bcdtmClip_cloneAndClipToPolygonDtmObject
(
 BC_DTM_OBJ *dtmP,             /* ==> Pointer To Dtm Object                       */
 BC_DTM_OBJ **cloneDtmPP,      /* <== Pointer To Cloned And Clipped Dtm Object    */
 DPoint3d *ptsP,                    /* ==> Pointer To Clipping Polygon Points          */
 long numPts,                  /* ==> Number Of Points In Clipping Polygon        */
 DTMClipOption clipOption               /* ==> Clipping Option <DTMClipOption::Internal,DTMClipOption::External> */
)
/*
** This Function Clones And Then Clips The Cloned Dtm Object To A Polygon
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 DTMDirection direction=DTMDirection::Unknown ;
 double area=0.0 ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Cloning And Clipping Dtm Object %p To Polygon",dtmP) ; 
    bcdtmWrite_message(0,0,0,"dtmP         = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"*cloneDtmPP  = %p",*cloneDtmPP) ;
    bcdtmWrite_message(0,0,0,"ptsP         = %p",ptsP) ;
    bcdtmWrite_message(0,0,0,"numPts       = %8ld",numPts) ;
    bcdtmWrite_message(0,0,0,"clipOption   = %8ld",clipOption) ;
   }
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
** Check If DTM Is In Dtm State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"DTM Object %p Not Triangulated",dtmP) ;
    goto errexit ;
   }
/*
** Check Clone Dtm Is Null
*/
 if( *cloneDtmPP != NULL )
   {
    bcdtmWrite_message(2,0,0,"None Null Clone Dtm Object %p",*cloneDtmPP) ;
    goto errexit ;
   }
/*
** Check Area And Direction Of Clipping Polygon
*/
 if( cdbg )
   {
     bcdtmWrite_message(0,0,0,"Checking Direction Of Clip Polygon") ;   
     if( bcdtmMath_getPolygonDirectionP3D(ptsP,numPts,&direction,&area)) goto errexit ; 
     bcdtmWrite_message(0,0,0,"Clip Polygon Area = %12.3lf ** direction = %2ld",area,direction) ;
     if (direction == DTMDirection::Clockwise) bcdtmWrite_message (0, 0, 0, "Polygon direction Clockwise");
     if (direction == DTMDirection::AntiClockwise) bcdtmWrite_message (0, 0, 0, "Polygon direction Anti Clockwise");
   }
/*
** Clone Dtm Object
*/
 if( bcdtmObject_cloneDtmObject(dtmP,cloneDtmPP)) goto errexit ;
/*
** Clip Cloned Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Clipping Cloned Dtm Object To Polygon") ;
 if( bcdtmClip_toPolygonDtmObject(*cloneDtmPP,ptsP,numPts,clipOption)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Cloning And Clipping Dtm Object To Polygon Completed") ; 
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Cloning And Clipping Dtm Object To Polygon Error") ; 
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( *cloneDtmPP != NULL ) bcdtmObject_destroyDtmObject(cloneDtmPP) ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*==============================================================================*//**
* @memo   Clips A Tin With A User Defined Horizontal And Vertical Offset
* @doc    Clips A Tin With A User Defined Horizontal And Vertical Offset
* @notes  
* 
* @author Rob Cormack 15 April 2003  Rob.Cormack@Bentley.com
*
* @param  *dtmP,            ==>  Dtm Object To Be Clipped                               
* @param  **clipDtmPP,      <==  The Clipped Dtm Object                              
* @param  *clipPtsP,        ==>  An Array Of Points Representating The Clip Polygon  
* @param  numClipPts,       ==>  Size of the Array of clip points                      
* @param  clipOption,       ==>  Clip Otion , 1 = Clip Internally 2 = Clip Externally 
* @param  isHorOffset,      ==>  Flag To Horizontal Offset Clip Polygon , TRUE = Horizontal Offset , FALSE = Don't Horizontal Offset 
* @param  horOffset,        ==>  Horizontal Offset Value . < 0.0 Horizontal Offset Internal Amount > 0.0 Horizontal Offset External Amount 
* @param  isVertOffset,     ==>  Flag To Vertical Offset Clipped Tin. TRUE = Vertical Offset , FALSE = Do Not Vertical Offset              
* @param  vertOffset        ==>  Vertical Offset Amount                                                                                    
*
* @return DTM_SUCCESS or DTM_ERROR
* @version 
* @see None
*===============================================================================*/
BENTLEYDTM_EXPORT int bcdtmClip_usingOffsetsDtmObject
(
 BC_DTM_OBJ *dtmP,          /*  ==>  Pointer To Dtm Object To Be Clipped                                                                                   */
 BC_DTM_OBJ **clipDtmPP,    /*  <==  Pointer To The Clipped Dtm Object                                                                                     */
 DPoint3d *userClipPtsP,         /*  ==>  An Array Of Points Representating The Clip Polygon                                                         */
 long numUserClipPts,       /*  ==>  Size of the Array of clip points                                                                           */
 DTMClipOption clipOption,           /*  ==>  Clip Otion , 1 = Clip Internally 2 = Clip Externally                                                       */
 long isHorOffset,          /*  ==>  Flag To Horizontal Offset Clip Polygon , TRUE = Horizontal Offset , FALSE = Don't Horizontal Offset        */
 double horOffset,          /*  ==>  Horizontal Offset Value . < 0.0 Horizontal Offset Internal Amount > 0.0 Horizontal Offset External Amount  */
 long isVertOffset,         /*  ==>  Flag To Vertical Offset Clipped Tin. TRUE = Vertical Offset , FALSE = Do Not Vertical Offset               */
 double vertOffset          /*  ==>  Vertical Offset Amount                                                                                     */
)
/*
** This Is A Special Purpose Modeller Function For Clipping A DTM 
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   pnt,numClipPts,numParallelPts=0 ;
 DPoint3d    *p3dP,*p3dP1,*p3dP2,*clipPtsP=NULL,*parallelPts=NULL ;
/*
** Initialise Variables
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Clipping Dtm Object Using Offsets") ; 
    bcdtmWrite_message(0,0,0,"dtmP           = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"clipDtmPP      = %p",*clipDtmPP) ;
    bcdtmWrite_message(0,0,0,"userClipPtsP   = %p",userClipPtsP) ;
    bcdtmWrite_message(0,0,0,"numUserClipPts = %6ld",numUserClipPts) ;
    bcdtmWrite_message(0,0,0,"clipOption     = %2ld",clipOption) ;
    bcdtmWrite_message(0,0,0,"isHorOffset    = %2ld",isHorOffset) ;
    bcdtmWrite_message(0,0,0,"horOffset      = %10.4lf",horOffset) ;
    bcdtmWrite_message(0,0,0,"isVertOffset   = %2ld",isVertOffset) ;
    bcdtmWrite_message(0,0,0,"vertOffset     = %10.4lf",vertOffset) ;
    for( p3dP = userClipPtsP ; p3dP < userClipPtsP + numUserClipPts ; ++p3dP)
      {
       bcdtmWrite_message(0,0,0,"Clip Point[%6ld] = %10.4lf %10.4lf %8.4lf",(long)(p3dP-userClipPtsP),p3dP->x,p3dP->y,p3dP->z) ;
      } 
   }
/*
** Validate Parameters
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
 if( *clipDtmPP != NULL )
   {
    if( bcdtmObject_testForValidDtmObject(*clipDtmPP)) bcdtmObject_destroyDtmObject(clipDtmPP) ;
    else                                              *clipDtmPP = NULL ;
   }
 if( userClipPtsP == NULL || numUserClipPts < 4 ) { bcdtmWrite_message(1,0,0,"Invalid Clip Polygon") ; goto errexit ; }
 if( horOffset  == 0.0 ) isHorOffset = FALSE ;
 if( vertOffset == 0.0 ) vertOffset  = FALSE ;
/*
** Make  A Local Copy Of The Clip Points
*/
 numClipPts = numUserClipPts ;
 clipPtsP   = ( DPoint3d * ) malloc( numClipPts * sizeof(DPoint3d)) ;
 if( clipPtsP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
 for( p3dP1 = clipPtsP , p3dP2 = userClipPtsP ; p3dP1 < clipPtsP + numClipPts ; ++p3dP1 , ++p3dP2 ) *p3dP1 = *p3dP2 ;
/*
** Validate Clip Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Validating Clip Polygon") ; 
 if( bcdtmMath_validatePointArrayPolygon(&clipPtsP,&numClipPts,dtmP->ppTol) ) goto errexit ;
/*
** Horizontal Offset Clip Polygon
*/
 if( isHorOffset == TRUE )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Parallel Clip Polygon") ;
    if( bcdtmData_copyParallelPointArrayPolygon(horOffset,clipPtsP,numClipPts,&parallelPts,&numParallelPts)) goto errexit ;
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"Number Of Parallel Points = %6ld",numParallelPts) ;
       for( p3dP = parallelPts ; p3dP < parallelPts + numParallelPts ; ++p3dP)
         {
          bcdtmWrite_message(0,0,0,"Parallel Point[%6ld] = %10.4lf %10.4lf %8.4lf",(long)(p3dP-parallelPts),p3dP->x,p3dP->y,p3dP->z) ;
         } 
      }
   }
 else
   {
    parallelPts    = clipPtsP ;
    numParallelPts = numClipPts ; 
   }
/*
** Clip The Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Clipping From Polygon Dtm Object") ;
 if( bcdtmClip_cloneAndClipToPolygonDtmObject(dtmP,clipDtmPP,parallelPts,numParallelPts,clipOption)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Clipping From Polygon Dtm Object Completed") ;
/*
** Vertical Offset The Clip Dtm Object
*/
 if( isVertOffset == TRUE ) 
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Vertical Offseting Clipped Dtm Object Points") ;
    for( pnt = 0 ; pnt < (*clipDtmPP)->numPoints ; ++pnt )
      {
       pointAddrP(*clipDtmPP,pnt)->z += vertOffset ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( clipPtsP    != NULL ) free(clipPtsP) ;
 if( parallelPts != NULL && parallelPts != clipPtsP ) free(parallelPts) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Clipping Dtm Object Using Offsets Completed") ; 
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Clipping Dtm Object Using Offsets Error") ; 
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = DTM_ERROR  ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmClip_toPolygonDtmObjects
(
 BC_DTM_OBJ *dtm1P,
 BC_DTM_OBJ *dtm2P,
 DPoint3d *userClipPtsP,
 long numUserClipPts,
 BC_DTM_OBJ **dtm3PP,
 BC_DTM_OBJ **dtm4PP
) 
/*
** This Function Clips Two Dtm Objects To A Common Polygon
*/
{
 int      ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long     numHullPts1,numHullPts2,numClipPts,intersectFlag,polyofs;
 DTMDirection direction;
 double   area,ppTol,plTol ;
 DPoint3d      *clipPtsP=NULL,*hullPts1P=NULL,*hullPts2P=NULL ;
 DTM_POLYGON_OBJ  *poly1P=NULL,*poly2P=NULL ;
 DTM_POLYGON_LIST   *pl ;
/*
**  Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Clip From Polygon Dtm Objects") ; 
/*
**  Validate Dtm Object For Storing Clips
*/
 if( *dtm3PP != NULL ) { if( bcdtmObject_testForValidDtmObject(*dtm3PP)) *dtm3PP = NULL ; if( *dtm3PP != NULL ) bcdtmObject_destroyDtmObject(dtm3PP) ; }
 if( *dtm4PP != NULL ) { if( bcdtmObject_testForValidDtmObject(*dtm4PP)) *dtm4PP = NULL ; if( *dtm4PP != NULL ) bcdtmObject_destroyDtmObject(dtm4PP) ; }
/*
** Test For Valid Dtm Objects
*/
 if( bcdtmObject_testForValidDtmObject(dtm1P)) goto errexit ;
 if( bcdtmObject_testForValidDtmObject(dtm2P)) goto errexit ;
/*
** Test For Pre 98 Tin Files
*/
 if( dtm1P->ppTol == 0.0 || dtm2P->ppTol == 0.0 ) 
   {
    bcdtmWrite_message(1,0,0,"Convert Tin File(s)") ;
    ret = 20 ;
    goto errexit ;
   }
/*
** Copy Clip Polygon
*/
 numClipPts = 0 ;
 if( numUserClipPts > 0 && userClipPtsP != NULL )
   {
    numClipPts = numUserClipPts ;
    if( bcdtmUtl_copy3DTo3D(userClipPtsP,numUserClipPts,&clipPtsP)) goto errexit ;
/*
** Validate Clip Polgon
*/
    if( bcdtmMath_validatePointArrayPolygon(&clipPtsP,&numClipPts,dtm1P->ppTol) ) goto errexit ; 
   }
/*
**  Extract Tin Hulls
*/
 if( bcdtmList_extractHullDtmObject(dtm1P,&hullPts1P,&numHullPts1)) goto errexit ;
 if( bcdtmList_extractHullDtmObject(dtm2P,&hullPts2P,&numHullPts2)) goto errexit ;
/*
** Set Tolerances
*/
 if( dtm1P->ppTol <= dtm2P->ppTol ) ppTol = dtm1P->ppTol ;
 else                               ppTol = dtm2P->ppTol ;
 if( dtm1P->plTol <= dtm2P->plTol ) plTol = dtm1P->plTol ;
 else                               plTol = dtm2P->plTol ;
/*
** Intersect Tin Hulls
*/
 if( bcdtmPolygon_intersectPointArrayPolygons(hullPts1P,numHullPts1,hullPts2P,numHullPts2,&intersectFlag,&poly1P,ppTol,plTol)) goto errexit ;
 if( intersectFlag == 0 ) 
   { 
    bcdtmWrite_message(1,0,0,"Tin Hulls Do Not Intersect") ;
    goto errexit ;
   }
/*
** Free UnWanted Memory
*/
 if( hullPts1P != NULL ) { free(hullPts1P) ; hullPts1P = NULL ; }
 if( hullPts2P != NULL ) { free(hullPts2P) ; hullPts2P = NULL ; }
/*
** Intersect Clip Polygon With Intersected Tin Hull Polygons
*/
 if( numClipPts > 0 )
   {
    bcdtmPolygon_intersectPointArrayPolygonWithPolgyonObject(clipPtsP,numClipPts,poly1P,ppTol,plTol,&intersectFlag,&poly2P) ;
    if( intersectFlag == 0 ) { bcdtmWrite_message(1,0,0,"Clip Polygon And Tin Hulls Do Not Intersect") ; goto errexit ; }
    bcdtmPolygon_deletePolygonObject(&poly1P) ; 
    poly1P = poly2P ;
    poly2P = NULL  ;
   }
/*
** Free UnWanted Memory
*/
 if( clipPtsP != NULL ) { free(clipPtsP) ; clipPtsP = NULL ; }
/*
** If More Than One Intersect Polygon Get And Use Largest Area Intersect Polygon For Clip
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Intersect Polygons = %2ld",poly1P->numPolygons) ;
 polyofs = 0 ;
 area = (poly1P->polyListP)->area ; 
 for( pl = poly1P->polyListP + 1 ; pl < poly1P->polyListP + poly1P->numPolygons ; ++pl )
   {
    if( pl->area > area ) { polyofs = (long)(pl-poly1P->polyListP) ; area = pl->area ; }
   }
/*
** Get Clip Polygon Vertices
*/
 if( bcdtmPolygon_copyPolygonObjectPolygonToPointArrayPolygon(poly1P,polyofs,&clipPtsP,&numClipPts)) goto errexit ;
/*
**  Clone And Clip dtm1P Object
*/
 if( bcdtmClip_cloneAndClipToPolygonDtmObject(dtm1P,dtm3PP,clipPtsP,numClipPts,DTMClipOption::External)) goto errexit ;
/*
**  Clone And Clip dtm2P Object
*/
 if (bcdtmClip_cloneAndClipToPolygonDtmObject (dtm2P, dtm4PP, clipPtsP, numClipPts, DTMClipOption::External)) goto errexit;
/*
**  Check Tin Hull Areas Are The Same - Development Only
*/
 if( cdbg )
   {
    if( bcdtmList_extractHullDtmObject(*dtm3PP,&hullPts1P,&numHullPts1)) goto errexit ;
    if( bcdtmList_extractHullDtmObject(*dtm4PP,&hullPts2P,&numHullPts2)) goto errexit ;
    bcdtmMath_getPolygonDirectionP3D(hullPts1P,numHullPts1,&direction,&area) ;
    bcdtmWrite_message(0,0,0,"Area Of dtm3PP = %12.4lf",area) ; 
    bcdtmMath_getPolygonDirectionP3D(hullPts2P,numHullPts2,&direction,&area) ;
    bcdtmWrite_message(0,0,0,"Area Of dtm4PP = %12.4lf",area) ; 
    free(hullPts1P) ; hullPts1P = NULL ; 
    free(hullPts2P) ; hullPts2P = NULL ; 
   } 
/*
** Clean Up
*/
 cleanup :
 if( clipPtsP  != NULL ) { free(clipPtsP)  ; clipPtsP  = NULL ; }
 if( hullPts1P != NULL ) { free(hullPts1P) ; hullPts1P = NULL ; }
 if( hullPts2P != NULL ) { free(hullPts2P) ; hullPts2P = NULL ; }
 if( poly1P    != NULL ) bcdtmPolygon_deletePolygonObject(&poly1P) ;
 if( poly2P    != NULL ) bcdtmPolygon_deletePolygonObject(&poly2P) ;
 /*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Clip From Polygon Dtm Objects Completed") ; 
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Clip From Polygon Dtm Objects Error") ; 
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( *dtm3PP != NULL ) bcdtmObject_destroyDtmObject(dtm3PP) ;
 if( *dtm4PP != NULL ) bcdtmObject_destroyDtmObject(dtm4PP) ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmClip_toTptrPolygonDtmObject(BC_DTM_OBJ *dtmP,long startPnt,DTMClipOption clipOption)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 DTMDirection direction ;
 long   dtmFeature ;
 double area ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Clipping Dtm Object To Tptr Polygon") ;
    bcdtmWrite_message(0,0,0,"dtmP       = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"startPnt   = %8ld",startPnt) ;
    bcdtmWrite_message(0,0,0,"clipOption = %8ld",clipOption) ;
   } 
/*
** Test For Valid Clip Option
*/
 if( clipOption != DTMClipOption::Internal && clipOption != DTMClipOption::External ) 
   { 
    bcdtmWrite_message(2,0,0,"Invalid Clip Option") ;
    goto errexit ; 
   }
/*
** Test For Valid Dtm  Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check If DTM Is In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"DTM Object %p Not Triangulated",dtmP) ;
    goto errexit ;
   }
/*
** Check For Valid Tptr Polygon Start Point
*/
 if( startPnt < 0 || startPnt >= dtmP->numPoints || nodeAddrP(dtmP,startPnt)->tPtr < 0 || nodeAddrP(dtmP,startPnt)->tPtr >= dtmP->numPoints )  
   { 
    bcdtmWrite_message(2,0,0,"Invalid Start Point For Clip Tptr Polygon") ;
    goto errexit ; 
   }
/*
** Check Connectivity Of Tptr Polygon - Development Only
*/
 if( cdbg )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Connectivity Tptr Polygon") ;
    if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,startPnt,0)) goto errexit ;
   }
/*
** Check Direction Of Tptr Polygon And If Clockwise Set Direction Anti Clockwise
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Direction Tptr Polygon") ;
 if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPnt,&area,&direction) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Tptr Polygon Area = %15.4lf Direction = %2ld",area,direction) ;
 if( direction == DTMDirection::Clockwise ) 
   {
    if( bcdtmList_reverseTptrPolygonDtmObject(dtmP,startPnt)) goto errexit ;
   }
/*
** Check Integrity Of Dtm Object - Development Only
*/
 if( cdbg ) 
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Dtm") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP)) { bcdtmWrite_message(0,0,0,"DTM Corrupted") ; goto errexit ; }
    else                                          bcdtmWrite_message(0,0,0,"DTM OK") ;
   }
/*
** Clip Internal To Tptr Polygon
*/
 if( clipOption == DTMClipOption::Internal ) 
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Clipping Dtm Object Internal") ;
    if( bcdtmClip_internalToTptrPolygonDtmObject(dtmP,startPnt,1)) goto errexit ; 
   }
/*
** Clip External To Tptr Polygon
*/
 if( clipOption == DTMClipOption::External ) 
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Clipping Dtm Object External") ;
    if( bcdtmClip_externalToTptrPolygonDtmObject(dtmP,startPnt)) goto errexit ; 
   }
/*
** Remove All But Tin Features
*/ 
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing All Features Except Tin State Features") ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState != DTMFeatureState::Tin )
      {
       if( bcdtmInsert_removeDtmFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ; 
      }
   }  
/*
** Clean Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Cleaning Dtm Object ** dtmP->numFeatures = %8ld",dtmP->numFeatures) ;
 if( bcdtmList_cleanDtmObject(dtmP)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Cleaning Dtm Object Completed ** dtmP->numFeatures = %8ld",dtmP->numFeatures) ;
/*
** Check Integrity Of Dtm Object - Development Only
*/
 if( cdbg ) 
   {
    bcdtmWrite_message(0,0,0,"Checking Clipped Dtm") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP)) 
      {
       bcdtmWrite_message(0,0,0,"DTM Corrupted After Clip") ;
       goto errexit ; 
      } 
    else bcdtmWrite_message(0,0,0,"DTM OK") ;
   } 
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Clipping Dtm Object To Tptr Polygon Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Clipping Dtm Object To Tptr Polygon Error") ;
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
BENTLEYDTM_Public int bcdtmClip_findAndMergeAdjoiningVoidsDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature)
/*
** This Function Looks For Adjoining Voids
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    pp,cp,np,sp,asp,anp,adjacentFeature,adjacentFlag ;
 DPoint3d     linePts[2] ;
 BC_DTM_OBJ *tempDtmP=NULL ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Finding And Merging Adjoining Voids") ;
/*
** Initialise
*/
 adjacentFlag = 0 ;
/*
** Scan Dtm Feature Looking For Adjoining Voids
*/
 pp = dtmP->nullPnt ;
 sp = ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ;
 do
   {
    if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sp,&np)) goto errexit  ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Testing For Adjacent dtmFeature") ;
    if( bcdtmClip_testForStartOfAdjoingFeatureDtmObject(dtmP,dtmFeature,pp,sp,np,DTMFeatureType::Void,&adjacentFeature)) goto errexit  ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Adjacent dtmFeature = %9ld",adjacentFeature) ;
    if( adjacentFeature != dtmP->nullPnt )
      {
        if( ! adjacentFlag )
         {
          adjacentFlag = 1 ;
/*
**        Create Dtm Object
*/
          if( bcdtmObject_createDtmObject(&tempDtmP)) goto errexit  ; 
          bcdtmObject_setPointMemoryAllocationParametersDtmObject(tempDtmP,2000,2000) ;
         }
/*
**     Copy Adjacent Dtm Feature To Dtm As Break Lines
*/ 
       asp = ftableAddrP(dtmP,adjacentFeature)->dtmFeaturePts.firstPoint ;
       do
         {
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,adjacentFeature,asp,&anp)) goto errexit  ;
          memcpy(&linePts[0],pointAddrP(dtmP,asp),sizeof(DPoint3d)) ;
          memcpy(&linePts[1],pointAddrP(dtmP,anp),sizeof(DPoint3d)) ;
          if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::Breakline,tempDtmP->nullUserTag,2,&tempDtmP->nullFeatureId,linePts,2) ) goto errexit ;
          asp = anp ;
         } while ( asp != ftableAddrP(dtmP,adjacentFeature)->dtmFeaturePts.firstPoint ) ;
/*
**     Delete  Adjacent dtmFeature
*/
       if( bcdtmInsert_removeDtmTinFeatureFromDtmObject(dtmP,adjacentFeature)) goto errexit  ; 
      }  
    pp = sp ; sp = np ;
   } while ( sp != ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ) ;
/*
** Process If Adjacent Features Detected
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Adjacent Flag = %2ld",adjacentFlag) ;
 if( adjacentFlag ) 
   {
/*
**  Copy Dtm Feature To Dtm As Break Lines
*/ 
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Dtm Feature To Dtm Object") ;
    sp = ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ;
    do
      {
       memcpy(&linePts[0],pointAddrP(dtmP,sp),sizeof(DPoint3d)) ;
       memcpy(&linePts[1],pointAddrP(dtmP,np),sizeof(DPoint3d)) ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::Breakline,tempDtmP->nullUserTag,2,&tempDtmP->nullFeatureId,linePts,2) ) goto errexit ;
       sp = np ;
      } while ( sp != ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ) ;
/*
**  Delete Dtm Feature 
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Dtm Feature From Dtm Object") ;
    if( bcdtmInsert_removeDtmTinFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit  ; 
/*
**  Triangulate Dtm Object
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating Dtm Object") ;
    if( bcdtmObject_createTinDtmObject(tempDtmP,1,0.0) ) goto errexit ;
/*
**  Remove None dtmFeature Hull Lines
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Removing Non dtmFeature Hull Lines") ;
    if( bcdtmList_removeNoneFeatureHullLinesDtmObject(tempDtmP)) goto errexit  ; 
/*
**  Copy Dtm Hull To Tptr Polygon
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Dtm Hull To Tptr Polygon") ;
    sp = tempDtmP->hullPoint ;
    cp = asp = (long)pointAddrP(tempDtmP,sp)->z ;
    do
      {
       np = nodeAddrP(tempDtmP,sp)->hPtr ;
       pp = (long)pointAddrP(tempDtmP,np)->z ;
       nodeAddrP(dtmP,cp)->tPtr = pp  ;
       bcdtmFlag_clearVoidBitPCWD(&nodeAddrP(dtmP,cp)->PCWD) ;
       sp = np ;
       cp = pp ;
      } while ( sp != tempDtmP->hullPoint ) ;  
/*
**  Delete Temporary Objects
*/
    bcdtmObject_destroyDtmObject(&tempDtmP) ;
/*
**  Check Connectivity Of Tptr List
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Connectivity Tptr List") ;
    if( bcdtmList_checkConnectivityTptrListDtmObject(dtmP,asp,0)) goto errexit  ; 
/*
**  Add dtmFeature To Tin
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Void Feature") ;
    if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,DTMFeatureType::Void,dtmP->nullUserTag,dtmP->nullFeatureId,asp,1)) goto errexit  ;
/*
**  Mark Internal Void Points
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Void Points") ;
    if( bcdtmMark_pointsInternalToVoidDtmObject(dtmP,dtmP->numFeatures-1) ) goto errexit  ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Void Points Completed") ;
   }
/*
** Clean Up
*/
 cleanup :
 if( tempDtmP != NULL ) bcdtmObject_destroyDtmObject(&tempDtmP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Finding And Merging Adjoining Voids Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Finding And Merging Adjoining Voids Error") ;
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
BENTLEYDTM_Public int bcdtmClip_testForStartOfAdjoingFeatureDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,long priorPnt,long thisPnt,long nextPnt,DTMFeatureType dtmFeatureType,long *adjacentFeatureP)
/*
** This Function Tests For The Start Of An Adjoining Dtm Feature Type
*/
{
 long flPtr,fl1Ptr,isw,process ;
/*
** Initialise
*/
 *adjacentFeatureP = dtmP->nullPnt ;
 flPtr = nodeAddrP(dtmP,nextPnt)->fPtr ;
/*
** Scan For thisPnt
*/ 
 process = 0 ;
 while ( flPtr != dtmP->nullPtr && process )
   {
    if( flistAddrP(dtmP,flPtr)->nextPnt == thisPnt )
      {
       if( flistAddrP(dtmP,flPtr)->dtmFeature != dtmFeature ) 
         {
          if( ftableAddrP(dtmP,flistAddrP(dtmP,flPtr)->dtmFeature)->dtmFeatureType == dtmFeatureType )
            { 
             isw = 1 ; 
             fl1Ptr = nodeAddrP(dtmP,thisPnt)->fPtr ;
             while ( fl1Ptr != dtmP->nullPtr && isw )
               {
                if( flistAddrP(dtmP,fl1Ptr)->nextPnt == priorPnt && flistAddrP(dtmP,fl1Ptr)->dtmFeature == flistAddrP(dtmP,flPtr)->dtmFeature ) isw = 0 ;
                fl1Ptr = flistAddrP(dtmP,fl1Ptr)->nextPtr ;                
               }
             if( isw ) 
               { 
                *adjacentFeatureP = flistAddrP(dtmP,flPtr)->dtmFeature ;
                return(0) ;
               }
            }
         }
      }
    flPtr = flistAddrP(dtmP,flPtr)->nextPtr ;
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
BENTLEYDTM_Public int bcdtmClip_checkForInsertedVoidInternalToVoidDtmObject
(
 BC_DTM_OBJ *dtmP
 )
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long sp,np,feature,numVoidPts,numFeaturePts ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Voids Internal To Other Voids") ;
/*
** Scan Features For Void Features
*/
 for( feature = 0 ; feature < dtmP->numFeatures ; ++feature )
   {
    dtmFeatureP = ftableAddrP(dtmP,feature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeatureType == DTMFeatureType::Void ) 
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Void Feature %4ld Type = %4ld Fpnt = %6ld",feature,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmFeaturePts.firstPoint) ;
       if( dbg ) bcdtmList_writePointsForDtmFeatureDtmObject(dtmP,feature) ;
       numVoidPts = numFeaturePts = 0 ;
/*
**     Scan Void And Test For Void Bits On Void Hull
*/
       sp = dtmFeatureP->dtmFeaturePts.firstPoint ;
       np = sp ;
       do
         {
/*
**        Test If Point In Void
*/
          ++numFeaturePts ;
          if( bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,np)->PCWD) )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Void Point Found On Void Hull ** Ndp = %6ld P = %6ld  %10.4lf %10.4lf %10.4lf",dtmP->numSortedPoints,np,pointAddrP(dtmP,np)->x,pointAddrP(dtmP,np)->y,pointAddrP(dtmP,np)->z) ;
             ++numVoidPts ; 
            }
/*
**        Get Next Point On Feature
*/
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,feature,np,&np)) goto errexit ;
         } while ( np != sp ) ;
/*
**     Delete Feature
*/
       if( numVoidPts == numFeaturePts )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Void Feature %6ld",feature) ;
          if( bcdtmInsert_removeDtmTinFeatureFromDtmObject(dtmP,feature)) goto errexit ;
         }
      } 
   }
/*
** Clean Up
*/
 cleanup :
/*
** Normal return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking For Voids Internal To Other Voids Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking For Voids Internal To Other Voids Error") ;
 return(ret) ;
/*
** Error return
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
BENTLEYDTM_Public int bcdtmClip_dtmFeaturesInternalToTptrPolygonDtmObject(BC_DTM_OBJ *dtmP,long startPnt,long mark)
/*
** This Function Clips Dtm Features Internal To Polygon
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  clPtr,nextPnt,firstPnt,dtmFeature,numFeatureTable,lastPnt  ;
 DTMFeatureType  dtmFeatureType;
 long  numMarked,numPts,numCoincident,numClipPts ;
 char  dtmFeatureTypeName[100] ;
 DPoint3d   *clipPtsP=NULL ;
 BC_DTM_FEATURE   *dtmFeatureP ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Internally Clipping DTM Features") ;
/*
** Copy The Clip Boundary To A DPoint3d Array
*/
 if( bcdtmList_copyTptrListToPointArrayDtmObject(dtmP,startPnt,&clipPtsP,&numClipPts)) goto errexit ;
/*
** Copy Tptr List To Sptr and Null Out Tptr
*/
 if( bcdtmList_nullSptrValuesDtmObject(dtmP))                  goto errexit ;
 if( bcdtmList_copyTptrListToSptrListDtmObject(dtmP,startPnt)) goto errexit ;
 if( bcdtmList_reverseSptrPolygonDtmObject(dtmP,startPnt))     goto errexit ;
 if( bcdtmList_nullTptrListDtmObject(dtmP,startPnt))           goto errexit ;
/*
** Scan Feature Lists And Detect And Fix Lines That Span Concave Clip Boundaries
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning And Fixing Concave Sections") ;
 if( bcdtmClip_scanFeatureListAndDetectAndFixConcaveSpansDtmObject(dtmP,mark) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Concave Sections Scanned And Fixed") ;
/*
** Scan Feature Table And Look For Marked Features
*/
 numFeatureTable = dtmP->numFeatures ;
 for( dtmFeature = 0 ; dtmFeature < numFeatureTable ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureType != DTMFeatureType::Hull )
      {
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeaturePts.firstPoint != dtmP->nullPnt )
         {
          dtmFeatureType = dtmFeatureP->dtmFeatureType ;
/*
**        Write Feature
*/
          if( dbg == 1  ) 
            {
             bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureType,dtmFeatureTypeName) ;
             bcdtmWrite_message(0,0,0,"Feature = %6ld of %6ld ** Type = %6ld Name = %s",dtmFeature,numFeatureTable,dtmFeatureType,dtmFeatureTypeName) ;
            }
/*
**        Initialise Scan Variables
*/
          numMarked = numPts = numCoincident = 0 ; 
          firstPnt  = nextPnt = lastPnt = dtmFeatureP->dtmFeaturePts.firstPoint  ;
          ++numPts ;
          if( nodeAddrP(dtmP,nextPnt)->tPtr == mark ) ++numMarked ;
/*
**        Scan Point Feature List Pointers
*/
          if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Point Feature Lists") ;
          clPtr = nodeAddrP(dtmP,nextPnt)->fPtr ;
          while ( clPtr != dtmP->nullPtr )
            {
             while ( clPtr != dtmP->nullPtr  && flistAddrP(dtmP,clPtr)->dtmFeature != dtmFeature ) clPtr = flistAddrP(dtmP,clPtr)->nextPtr ;
             if( clPtr != dtmP->nullPtr )
               {
                nextPnt = flistAddrP(dtmP,clPtr)->nextPnt ;
                if( nextPnt != dtmP->nullPnt )
                  {
                   ++numPts ; 
                   if     ( nodeAddrP(dtmP,nextPnt)->tPtr == mark ) ++numMarked ;
                   else if( nodeAddrP(dtmP,nextPnt)->sPtr != dtmP->nullPnt ) ++numCoincident ;
                   clPtr = nodeAddrP(dtmP,nextPnt)->fPtr ;
                   lastPnt = nextPnt ;
                  }                
                if( nextPnt == dtmP->nullPnt || nextPnt == firstPnt ) clPtr = dtmP->nullPtr ;
               }
            }
          if( dbg ) bcdtmWrite_message(0,0,0,"**** numPts = %6ld NumMarked = %6ld NumCoincident = %6ld",numPts,numMarked,numCoincident) ;
/*
**        Test If Feature Clipped
*/ 
          if( numMarked == 0 && ( numCoincident > 0 && ( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void || dtmFeatureP->dtmFeatureType == DTMFeatureType::Island || dtmFeatureP->dtmFeatureType == DTMFeatureType::Hole ))) numMarked = numCoincident ;
/*
**        Process Clipped Feature
*/ 
          if( numMarked > 0 )
            { 
/*
**           Write Feature
*/
             if( dbg ) 
               {
                bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureType,dtmFeatureTypeName) ;
                bcdtmWrite_message(0,0,0,"Clipping Feature = %6ld of %6ld ** Type = %6ld Name = %s",dtmFeature,numFeatureTable,dtmFeatureType,dtmFeatureTypeName) ;
               }
/*
**           Delete Origonal Feature From Dtm Object If All Points Marked
*/
             if( numPts == numMarked || ( dtmFeatureType != DTMFeatureType::GroupSpots && numPts == numMarked + 1) )
               {  
                if( dbg ) bcdtmWrite_message(0,0,0,"**** Removing Feature") ;
                if( bcdtmInsert_removeDtmTinFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ; 
               }
/*
**           Delete Clipped Sections Of Feature Only
*/
             else 
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"**** Clipping Feature") ;
                switch ( dtmFeatureType ) 
                  {
                   case DTMFeatureType::GroupSpots :
                   if( bcdtmClip_groupSpotDtmFeatureDtmObject(dtmP,dtmFeature,mark)) goto errexit ;
                   break ;

 
                   case DTMFeatureType::Region :
                   case DTMFeatureType::Polygon :
                   case DTMFeatureType::Void    :
                   case DTMFeatureType::Island  :
                   case DTMFeatureType::Hole    :
                   if( bcdtmClip_internalPolygonalDtmFeatureDtmObject(dtmP,dtmFeature,clipPtsP,numClipPts)) goto errexit ;
                   break ;
               
                   case DTMFeatureType::Breakline :
                   case DTMFeatureType::ContourLine :
                   case DTMFeatureType::SoftBreakline : 
                   if( bcdtmClip_linearDtmFeatureDtmObject(dtmP,dtmFeature,mark)) goto errexit ;
                   break ;

                   default :
                   bcdtmWrite_message(2,0,0,"Unknown DTM Feature Type") ;
                   goto errexit ; 
                   break ;
                  } ; 
               }
            }
         }
      }
   }
/*
** Remove Voids On Island Boundaries
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Voids On Island Hulls") ;
 if( bcdtmClip_modifyIslandHullsForExternalVoidsDtmObject(dtmP)) goto errexit ;
/*
**  Copy Sptr List To Tptr 
*/
 if( bcdtmList_reverseSptrPolygonDtmObject(dtmP,startPnt))     goto errexit ;
 if( bcdtmList_copySptrListToTptrListDtmObject(dtmP,startPnt)) goto errexit ;
 if( bcdtmList_nullSptrListDtmObject(dtmP,startPnt))           goto errexit ;
/*
** Clean Up
*/
 cleanup :
 if( clipPtsP != NULL ) free(clipPtsP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Internally Clipping DTM Features Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Internally Clipping DTM Features Error") ;
 return(ret) ;
/*
** Errexit
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
BENTLEYDTM_Public int bcdtmClip_groupSpotDtmFeatureDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,long mark)
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    sp, np, lnp, process, numPts, firstPnt, newFirstPnt;
 DTMFeatureType dtmFeatureType;
 DTMUserTag userTag   ;
 DTMFeatureId     userFeatureId  ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Internally Clipping Dtm Feature %6ld ** Feature Type = DTMFeatureType::GroupSpots",dtmFeature) ;
/*
** Initialise
*/
 firstPnt       = ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ;
 dtmFeatureType = ftableAddrP(dtmP,dtmFeature)->dtmFeatureType ;
 userTag        = ftableAddrP(dtmP,dtmFeature)->dtmUserTag  ;
 userFeatureId  = ftableAddrP(dtmP,dtmFeature)->dtmFeatureId ;
/*
** Count Number Of Points
*/
 if( dbg )
   {
    numPts = 0 ;
    sp = firstPnt ;
    do
      {
       ++numPts ;
       if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sp,&sp)) goto errexit ; 
      } while ( sp != dtmP->nullPnt && sp != firstPnt ) ;
    if( sp == firstPnt ) ++numPts ;
    bcdtmWrite_message(0,0,0,"Number Of Points In DTMFeatureType::GroupSpots = %6ld",numPts) ;  
   }
/*
** Scan Feature For Non Clipped Sections
*/
 sp          = firstPnt ;
 process     = 1 ;
 lnp         = dtmP->nullPnt ;
 newFirstPnt = dtmP->nullPnt ;
 while ( process )
   { 
/*
**  Scan To First Unmarked Point On Feature
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Scanning To First Unmarked Point") ;
    while ( process && nodeAddrP(dtmP,sp)->tPtr == mark )
      {
       if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sp,&sp)) goto errexit ; 
       if( sp == dtmP->nullPnt || sp == firstPnt ) process = 0 ;
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"First Unmarked Point = %8ld",sp) ;
/*
**  Scan To Next Mark Point
*/
    if( process )  	   
      {
/*
**     Set New First Point
*/
       if( newFirstPnt == dtmP->nullPnt ) newFirstPnt = sp ;
       else                               nodeAddrP(dtmP,lnp)->tPtr = sp ;
/*
**     Scan To Next Marked Point
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Scanning To Next Marked Point") ;
       np = sp ;
       process = 1 ;
       while ( process && nodeAddrP(dtmP,np)->tPtr != mark )
         {
          lnp = np ;
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,np,&np)) goto errexit ; 
          if( np == dtmP->nullPnt || np == firstPnt ) process = 0 ;
          else                                        nodeAddrP(dtmP,lnp)->tPtr = np ;                                     
         }
/*
**     Check For Closure
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Last Unmarked Point = %8ld",lnp) ;
       if( np == firstPnt ) nodeAddrP(dtmP,lnp)->tPtr = np ; 
       sp = np ;
      }
   }
/*
** Add Feature To Dtm 
*/
 if( newFirstPnt != dtmP->nullPnt )
   {
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"Adding Feature To DTM") ;  
       numPts = 0 ;
       sp = newFirstPnt ;
       do
         {
          ++numPts ;
          sp = nodeAddrP(dtmP,sp)->tPtr ;
          if( sp != dtmP->nullPnt ) if( nodeAddrP(dtmP,sp)->tPtr == mark ) sp = mark ;
         } while ( sp != dtmP->nullPnt && sp != newFirstPnt && sp != mark) ;
       bcdtmWrite_message(0,0,0,"Number Of Points In DTMFeatureType::GroupSpots = %6ld",numPts) ;  
      }
/*
**  Check Last Point Is Not A Mark
*/
    sp = newFirstPnt ;
    do
      {
       np = nodeAddrP(dtmP,sp)->tPtr ;
       if( np != dtmP->nullPnt )
         {
          if( nodeAddrP(dtmP,np)->tPtr == mark )
            {
             nodeAddrP(dtmP,sp)->tPtr = dtmP->nullPnt ;
             np = mark ;
            } 
         }
       sp = np ; 
      } while ( sp != dtmP->nullPnt && sp != newFirstPnt && sp != mark) ;
/*
**  Add Feature To Tin
*/
    if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,DTMFeatureType::GroupSpots,userTag,userFeatureId,newFirstPnt,1)) goto errexit ;
   }
/*
** Delete Origonal Feature From Dtm Object
*/
 if( bcdtmInsert_removeDtmTinFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
** Normal Exit
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Internally Clipping Dtm Feature %6ld ** Feature Type = %2ld Completed",dtmFeature,ftableAddrP(dtmP,dtmFeature)->dtmFeatureType) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Internally Clipping Dtm Feature %6ld ** Feature Type = %2ld Error",dtmFeature,ftableAddrP(dtmP,dtmFeature)->dtmFeatureType) ;
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
BENTLEYDTM_Private int bcdtmClip_internalPolygonalDtmFeatureDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,DPoint3d *clipPtsP,long numClipPts)
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    sp,np,firstPnt,startPnt,numFeaturePts,intersectFlag,polyError ;
 DTMFeatureType dtmFeatureType, insertType;
 DTMDirection direction;
 DPoint3d     *p3d,*featurePtsP=NULL ;
 double  area,perimeter ;
 DTM_POLYGON_OBJ *polyP=NULL    ;
 DTM_POLYGON_LIST  *pl            ;
 DTMUserTag userTag   ;
 DTMFeatureId     userFeatureId  ;
/*
** Write Entry Message
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Internally Clipping Dtm Feature %6ld ** Feature Type = %2ld",dtmFeature,ftableAddrP(dtmP,dtmFeature)->dtmFeatureType) ;
    bcdtmMath_getPolygonDirectionP3D(clipPtsP,numClipPts,&direction,&area) ;
    bcdtmWrite_message(0,0,0,"Clip Polygon ** Area = %10.4lf Direction = %2ld",area,direction) ;
    bcdtmWrite_message(0,0,0,"Number Of Clip Polygon Points = %6ld",numClipPts) ;
    if( dbg == 2 )
      { 
       for( p3d = clipPtsP ; p3d < clipPtsP + numClipPts ; ++p3d )
         {
          bcdtmWrite_message(0,0,0,"Clip Polygon Point [%4ld] = %10.4lf %10.4lf %10.4lf",(long)(p3d-clipPtsP),p3d->x,p3d->y,p3d->z) ;
         }
      } 
   }
/*
** Initialise
*/
 firstPnt       = ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ;
 dtmFeatureType = ftableAddrP(dtmP,dtmFeature)->dtmFeatureType ;
 userTag        = ftableAddrP(dtmP,dtmFeature)->dtmUserTag  ;
 userFeatureId       = ftableAddrP(dtmP,dtmFeature)->dtmFeatureId ;
/*
** Count Number Of Points In Dtm Feature
*/
 numFeaturePts = 0 ;
 sp = firstPnt ;
 do
   {
    ++numFeaturePts ;
    bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sp,&sp) ;
   } while ( sp != firstPnt && sp != dtmP->nullPnt ) ;
 ++numFeaturePts ;
/*
** Allocate Memory For Feature Points
*/
 featurePtsP = ( DPoint3d *) malloc(numFeaturePts * sizeof(DPoint3d)) ;
 if( featurePtsP == NULL ) 
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ; 
   }
/*
** Copy Feature Points To Point Array
*/
 sp = firstPnt ;
 p3d = featurePtsP ;
 do
   {
    p3d->x = pointAddrP(dtmP,sp)->x ;
    p3d->y = pointAddrP(dtmP,sp)->y ;
    p3d->z = (double) sp ;
    ++p3d ;
    bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sp,&sp) ;
   } while ( sp != firstPnt && sp != dtmP->nullPnt ) ;
 p3d->x = pointAddrP(dtmP,sp)->x ;
 p3d->y = pointAddrP(dtmP,sp)->y ;
 p3d->z = (double) sp ;
/*
** Intersect DPoint3d Polygons
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting Polygons") ;
 if( bcdtmClip_exclusivelyPolygonWithClipPolygonDtmObject(featurePtsP,numFeaturePts,clipPtsP,numClipPts,&intersectFlag,&polyP)) goto errexit ;
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Polygons Intersected") ;
    bcdtmWrite_message(0,0,0,"intersectFlag = %1ld",intersectFlag) ;
    bcdtmWrite_message(0,0,0,"Number Of Intersected Polygons = %4ld",polyP->numPolygons) ;
    for( pl = polyP->polyListP ; pl < polyP->polyListP + polyP->numPolygons ; ++pl )
      {
       bcdtmWrite_message(0,0,0,"Polygon [%4ld] ** Area = %10.4lf",(long)(pl-polyP->polyListP),pl->area) ;
       for( p3d = polyP->polyPtsP + pl->firstPnt ; p3d <= polyP->polyPtsP + pl->lastPnt ; ++p3d )
         {
          bcdtmWrite_message(0,0,0,"Poly Point [%4ld] = %10.4lf %10.4lf %10.4lf",(long)(p3d-polyP->polyPtsP),p3d->x,p3d->y,p3d->z) ;
         }
      }
   }
/*
** Free Memory
*/
 free(featurePtsP) ; 
 featurePtsP = NULL ;
 numFeaturePts = 0 ;
/*
** Extract Intersected Polygons From Polygon Object And Store Them As Features
*/
 if( intersectFlag == 1 )
   {
    for( pl = polyP->polyListP ; pl < polyP->polyListP + polyP->numPolygons ; ++pl )
      {
       startPnt = dtmP->nullPnt ;
       polyError = 0 ;
       if( bcdtmPolygon_copyPolygonObjectPolygonToPointArrayPolygon(polyP,(long)(pl-polyP->polyListP),&featurePtsP,&numFeaturePts)) goto errexit ;
       if( dbg )
         {
          bcdtmWrite_message(0,0,0,"Number Of Clipped Feature Points = %6ld Area = %15.8lf",numFeaturePts,pl->area) ;
          for( p3d = featurePtsP ; p3d < featurePtsP + numFeaturePts ; ++p3d )  
            {
             bcdtmWrite_message(0,0,0,"Clipped Feature Point [%4ld] = %10.4lf %10.4lf %10.4lf",(long)(p3d-featurePtsP),p3d->x,p3d->y,p3d->z) ;
            } 
         }
/*
** Determine Insert Type
*/
       insertType = DTMFeatureType::Polygon ;
       if( ! polyError )
         {
          if     ( dtmFeatureType == DTMFeatureType::Polygon ) insertType = DTMFeatureType::Polygon ;
          else if ( dtmFeatureType == DTMFeatureType::Polygon ) insertType = DTMFeatureType::Region ;
          else if( dtmFeatureType == DTMFeatureType::Void   && pl->userTag == 1 ) insertType = DTMFeatureType::Void ;
          else if( dtmFeatureType == DTMFeatureType::Void   && pl->userTag == 2 ) insertType = DTMFeatureType::Island ;
          else if( dtmFeatureType == DTMFeatureType::Island && pl->userTag == 1 ) insertType = DTMFeatureType::Island ;
          else if( dtmFeatureType == DTMFeatureType::Island && pl->userTag == 2 ) insertType = DTMFeatureType::Void ;
          else if( dtmFeatureType == DTMFeatureType::Hole   && pl->userTag == 1 ) insertType = DTMFeatureType::Void ;
          else if( dtmFeatureType == DTMFeatureType::Hole   && pl->userTag == 2 ) insertType = DTMFeatureType::Island ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Type = %4ld pl->userTag = %2ld insertType = %2ld",dtmFeatureType,pl->userTag,insertType) ;
          if( insertType == DTMFeatureType::Island &&  dtmFeatureType != DTMFeatureType::Island ) polyError = 1 ;
          if( insertType == DTMFeatureType::Void   &&  dtmFeatureType != DTMFeatureType::Void && dtmFeatureType != DTMFeatureType::Hole ) polyError = 1 ;
         }
/*
**  Check Feature Point Numbers
**  Added 4 Feb 2005 Rob Cormack
*/
       if( ! polyError )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Checking Clipped Feature Point Numbers") ;
          for( p3d = featurePtsP ; p3d < featurePtsP + numFeaturePts ; ++p3d )  
            {
             sp = (long)p3d->z ;
             if( (double)sp != p3d->z )
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"Point Error ** Point = %8ld z = %12.5lf",sp,p3d->z) ;
                bcdtmFind_closestPointDtmObject(dtmP,p3d->x,p3d->y,&sp) ;
                p3d->z = (double) sp ;
               }
            }
         } 
/*
** Copy Polygon Hull To Tptr List
*/
       if( ! polyError )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Copying Feature To Tptr Polygon") ;
          startPnt = (long) featurePtsP->z ;
          for( p3d = featurePtsP + 1 ; p3d < featurePtsP + numFeaturePts ; ++p3d )  
            {
             if( nodeAddrP(dtmP,startPnt)->tPtr != dtmP->nullPnt )
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"New Feature Contains Marked Point %6ld ** %12.5lf %12.5lf %10.4lf",startPnt,pointAddrP(dtmP,startPnt)->x,pointAddrP(dtmP,startPnt)->y,pointAddrP(dtmP,startPnt)->z) ;
                polyError = 1 ;  
                p3d  = featurePtsP + numFeaturePts ;
               }
             else
               { 
                nodeAddrP(dtmP,startPnt)->tPtr = (long)(p3d->z)  ;
                startPnt = (long)(p3d->z) ; 
               }
            }
         }
/*
**  Check Connectivity Of Tptr List
*/
        if( ! polyError )
          { 
           if( dbg ) bcdtmWrite_message(0,0,0,"Checking Connectivity Tptr Polygon") ;
           if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,startPnt,0)) 
             {
              for( p3d = featurePtsP ; p3d < featurePtsP + numFeaturePts - 1 ; ++p3d )  
                {
                 sp = (long)p3d->z ;
                 np = (long)(p3d+1)->z ;
                 if( sp != np )
                   { 
                    if( ! bcdtmList_testLineDtmObject(dtmP,sp,np) )
                      {
                       if( dbg ) 
                         {
                          bcdtmWrite_message(0,0,0,"Feature Points Not Connected") ;   
                          bcdtmWrite_message(0,0,0,"Sp = %8ld ** %12.5lf %12.5lf %10.4lf",sp,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z) ;   
                          bcdtmWrite_message(0,0,0,"Np = %8ld ** %12.5lf %12.5lf %10.4lf",np,pointAddrP(dtmP,np)->x,pointAddrP(dtmP,np)->y,pointAddrP(dtmP,np)->z) ;   
                         }
                       nodeAddrP(dtmP,sp)->tPtr = dtmP->nullPnt ;
                       if( bcdtmInsert_lineBetweenPointsDtmObject(dtmP,sp,np,1,2)) goto errexit ;
                      }
                   }
                }
              if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,startPnt,0)) 
                {
                 polyError = 1 ;  
                 if( dbg ) bcdtmWrite_message(0,0,0,"Connectivity Error In Feature Being Inserted") ;
                }
             }
          }
/*
**     Check Direction
*/
        if( ! polyError )
          {
           if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Direction Area And Perimeter Tptr Polygon") ;
           bcdtmMath_calculateDirectionAreaAndPerimeterTptrPolygonDtmObject(dtmP,startPnt,&direction,&area,&perimeter) ;
           if( dbg ) bcdtmWrite_message(0,0,0,"Direction = %2ld Area = %15.5lf Perimeter = %15.5lf Area/Perimeter = %15.10lf",direction,area,perimeter,area/perimeter) ; 
           if( area < 0.1 || area / perimeter < 0.00001 ) polyError = 1 ;
           else if( direction == DTMDirection::Clockwise )
             {
              if( bcdtmList_reverseTptrPolygonDtmObject(dtmP,startPnt)) goto errexit ;
             }  
          }
/*
**     Store New Feature In DTMFeatureState::Tin
*/
        if( ! polyError )
          {
           if( dbg ) bcdtmWrite_message(0,0,0,"Inserting %2ld Feature",insertType) ;
           if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,insertType,userTag,userFeatureId,startPnt,1)) goto errexit ; 
          }
        else 
          {
           if( startPnt != dtmP->nullPnt && nodeAddrP(dtmP,startPnt)->tPtr >= 0 && nodeAddrP(dtmP,startPnt)->tPtr < dtmP->numPoints ) 
             {
              bcdtmList_nullTptrListDtmObject(dtmP,startPnt) ;  
             }
          }
/*
** Free Memory For Feature Points
*/
       if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; } 
      }
   }
/*
** Delete Origonal Feature From Dtm Object
*/
 if( bcdtmInsert_removeDtmTinFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
 if( featurePtsP != NULL ) free(featurePtsP) ;
 if( polyP       != NULL ) bcdtmPolygon_deletePolygonObject(&polyP) ;
/*
** Normal Exit
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 ret = 1 ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmClip_externalPolygonalDtmFeatureDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,DPoint3d *clipPtsP,long numClipPts)
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    sp, firstPnt, startPnt, numFeaturePts, intersectFlag;
 DTMFeatureType dtmFeatureType;
 DTMDirection direction;
 DPoint3d     *p3dP,*featurePtsP=NULL ;
 double  area ;
 DTM_POLYGON_OBJ   *polyP=NULL ;
 DTM_POLYGON_LIST  *plistP ;
 DTMUserTag      userTag  ;
 DTMFeatureId    userFeatureId ;
 BC_DTM_FEATURE    *dtmFeatureP ;
/*
** Initialise
*/
 dtmFeatureP    = ftableAddrP(dtmP,dtmFeature) ;
 firstPnt       = dtmFeatureP->dtmFeaturePts.firstPoint ;
 dtmFeatureType = dtmFeatureP->dtmFeatureType ;
 userTag        = dtmFeatureP->dtmUserTag ;
 userFeatureId  = dtmFeatureP->dtmFeatureId ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Externally Clipping Feature %6ld ** dtmFeatureType = %2ld  userTag = %10I64",dtmFeature,dtmFeatureType,userTag) ;
    bcdtmMath_getPolygonDirectionP3D(clipPtsP,numClipPts,&direction,&area) ;
    bcdtmWrite_message(0,0,0,"Clip Polygon ** Area = %10.4lf Direction = %2ld",area,direction) ;
    bcdtmWrite_message(0,0,0,"Number Of Clip Polygon Points = %6ld",numClipPts) ;
    if( dbg == 2 )
      {
       for( p3dP = clipPtsP ; p3dP < clipPtsP + numClipPts ; ++p3dP )
         {
          bcdtmWrite_message(0,0,0,"Clip Point [%4ld] = %10.4lf %10.4lf %10.4lf",(long)(p3dP-clipPtsP),p3dP->x,p3dP->y,p3dP->z) ;
         }
      }
   }
/*
** Count Number Of Points In Feature
*/
 numFeaturePts = 0 ;
 sp = firstPnt ;
 do
   {
    ++numFeaturePts ;
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Sp = %6ld Tptr = %9ld ** %10.4lf %10.4lf %10.4lf",sp,nodeAddrP(dtmP,sp)->tPtr,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z) ;  
    bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sp,&sp) ;
   } while ( sp != firstPnt && sp != dtmP->nullPnt ) ;
 if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Sp = %6ld Tptr = %9ld ** %10.4lf %10.4lf %10.4lf",sp,nodeAddrP(dtmP,sp)->tPtr,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z) ;  
 ++numFeaturePts ;
/*
** Allocate Memory For Feature Points
*/
 featurePtsP = ( DPoint3d *) malloc(numFeaturePts * sizeof(DPoint3d)) ;
 if( featurePtsP == NULL ) 
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ; 
   }
/*
** Copy Feature Points To DPoint3d Array
*/
 p3dP = featurePtsP ;
 sp = firstPnt ;
 do
   {
    p3dP->x = pointAddrP(dtmP,sp)->x ;
    p3dP->y = pointAddrP(dtmP,sp)->y ;
    p3dP->z = (double) sp ;
    ++p3dP ;
    bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sp,&sp) ;
   } while ( sp != firstPnt && sp != dtmP->nullPnt ) ;
 p3dP->x = pointAddrP(dtmP,sp)->x ;
 p3dP->y = pointAddrP(dtmP,sp)->y ;
 p3dP->z = (double) sp ;
/*
** Write Area Direction And Feature Polygon Points
*/
 if( dbg )
   {
    bcdtmMath_getPolygonDirectionP3D(featurePtsP,numFeaturePts,&direction,&area) ;
    bcdtmWrite_message(0,0,0,"Feature Polygon ** numPts = %6ld ** Area = %10.4lf Direction = %2ld",numFeaturePts,area,direction) ;
    if( dbg == 2 )
      { 
       for( p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )
         {
          bcdtmWrite_message(0,0,0,"Feature Point[%8ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-featurePtsP),p3dP->x,p3dP->y,p3dP->z) ;
         } 
      }
   }
/*
** Intersect DPoint3d Polygons
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting Polygons") ;
 if( bcdtmPolygon_intersectPolygons(clipPtsP,numClipPts,featurePtsP,numFeaturePts,&intersectFlag,&polyP,0.0,0.0)) goto errexit ;
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Polygons Intersected") ;
    bcdtmWrite_message(0,0,0,"intersectFlag = %1ld",intersectFlag) ;
    bcdtmWrite_message(0,0,0,"Number Of Intersected Polygons = %4ld",polyP->numPolygons) ;
    if( dbg == 1 )
      { 
       for( plistP = polyP->polyListP ; plistP < polyP->polyListP + polyP->numPolygons ; ++plistP )
         {
          bcdtmWrite_message(0,0,0,"Polygon [%4ld] ** Area = %10.4lf",(long)(plistP-polyP->polyListP),plistP->area) ;
          if( dbg == 2 )
            { 
             for( p3dP = polyP->polyPtsP + plistP->firstPnt ; p3dP <= polyP->polyPtsP + plistP->lastPnt ; ++p3dP )
               {
                bcdtmWrite_message(0,0,0,"Poly Point [%4ld] = %10.4lf %10.4lf %10.4lf",(long)(p3dP-polyP->polyPtsP),p3dP->x,p3dP->y,p3dP->z) ;
               } 
            }
         }   
      }
   }
/*
** Free Memory
*/
 free(featurePtsP) ; 
 featurePtsP = NULL ;
 numFeaturePts = 0 ;
/*
** Extract Intersected Polygons From Polygon Object And Store Them As Features
*/
 for( plistP = polyP->polyListP ; plistP < polyP->polyListP + polyP->numPolygons ; ++plistP )
   {
    if( bcdtmPolygon_copyPolygonObjectPolygonToPointArrayPolygon(polyP,(long)(plistP-polyP->polyListP),&featurePtsP,&numFeaturePts)) goto errexit ;
    if( dbg == 1 )
      {
       bcdtmWrite_message(0,0,0,"Number Of Feature Points = %6ld",numFeaturePts) ;
       if( dbg == 2 )
         {
          for( p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )  
            {
             bcdtmWrite_message(0,0,0,"Feature Point [%4ld] = %10.4lf %10.4lf %10.4lf",(long)(p3dP-featurePtsP),p3dP->x,p3dP->y,p3dP->z) ;
            } 
         } 
      }
/*
**  Get Tin Points For Polygon Points
*/
    for( p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )  
      {
       bcdtmFind_closestPointDtmObject(dtmP,p3dP->x,p3dP->y,&startPnt) ;
       p3dP->z = (double) startPnt  ;
       nodeAddrP(dtmP,startPnt)->tPtr = dtmP->nullPnt ;
      } 
/*
**  Write Feature Points
*/
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"Number Of Feature Points = %6ld",numFeaturePts) ;
       for( p3dP = featurePtsP ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )  
         {
          bcdtmWrite_message(0,0,0,"Feature Point [%4ld] = %10.4lf %10.4lf ** %10ld",(long)(p3dP-featurePtsP),p3dP->x,p3dP->y,(long)p3dP->z) ;
         } 
      }
/*
**  Copy Polygon Hull To Tptr List
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Copying Polygon Hull To Tptr List") ;
    startPnt = (long) featurePtsP->z ;
    for( p3dP = featurePtsP + 1 ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )  
      {
       if( ! bcdtmList_testLineDtmObject(dtmP,startPnt,(long)p3dP->z)) 
         {
          if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Inserting Line Between %8ld %8ld",startPnt,(long)p3dP->z) ;
          if( bcdtmInsert_lineBetweenPointsDtmObject(dtmP,startPnt,(long)p3dP->z,1,2)) goto errexit ;
         }
       else  nodeAddrP(dtmP,startPnt)->tPtr = (long)(p3dP->z)  ;
       startPnt = (long)(p3dP->z) ; 
      } 
/*
**  Check Connectivity Of Tptr Polygon
*/
    if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,startPnt,0)) 
      {
       bcdtmWrite_message(2,0,0,"Connectivity Error In Clipped Dtm Feature") ;
       goto errexit ;
      }
/*
**  Store New Feature In DTMFeatureState::Tin
*/
    if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,dtmFeatureType,userTag,userFeatureId,startPnt,1)) goto errexit ; 
/*
**  Free Memory For Feature Points
*/
    free(featurePtsP) ;
    featurePtsP = NULL ; 
   }
/*
** Delete Origonal Feature From Dtm Object
*/
 if( bcdtmInsert_removeDtmTinFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
 if( featurePtsP != NULL ) free(featurePtsP) ;
 if( polyP       != NULL ) bcdtmPolygon_deletePolygonObject(&polyP) ;
/*
** Normal Exit
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
BENTLEYDTM_Public int bcdtmClip_exclusivelyPolygonWithClipPolygonDtmObject(DPoint3d *poly1PtsP,long numPoly1Pts,DPoint3d *poly2PtsP,long numPoly2Pts,long *clipResult,DTM_POLYGON_OBJ **polyPP )
/*
**
** This Function Exclusively Clips A Polygon With A Clipping Polygon.
** Segments Of Poly 1 internal to Poly2 are clipped
**
** Poly 1 - Polygon To Be Exclusively Clipped
** Poly 2 - Clipping Polygon
**
** Return Values for clipResult == 0 Polygons dont intersect
**                              == 1 Polygons intersect
**                              == 2 Poly1 is totally inside Poly 2
**                              == 3 Poly2 is totally inside Poly 1
**                              == 4 Polygons are coincident
** Notes :-
**
**  1. Assumes Poly1 And Poly2 have been validated and set anti-clockwise
**  2. Direction Of intersected polygons is set anti-clockwise
**
*/
{
 int     ret=DTM_SUCCESS ;
 long    sp,fPtr,numHullPts,numPoly1DtmPts,numPoly2DtmPts  ;
 DPoint3d     *p3d1P,*p3d2P,linePts[2] ; 
 BC_DTM_OBJ *dtmP=NULL   ;
/*
** Initialise
*/
 *clipResult = 0 ;
/*
** Create Polygon Object If Necessary
*/
 if( *polyPP == NULL ) 
   { 
    if( bcdtmPolygon_createPolygonObject(polyPP)) goto errexit ; 
   }
/*
** Create Data Object
*/
 if( bcdtmObject_createDtmObject(&dtmP)) goto errexit ;
 bcdtmObject_setPointMemoryAllocationParametersDtmObject(dtmP,numPoly1Pts+numPoly2Pts,100) ;
/*
** Write Polygons To Data Object As Break Lines
*/
 for( p3d1P = poly1PtsP , p3d2P = poly1PtsP + 1 ; p3d2P < poly1PtsP + numPoly1Pts ; ++p3d1P , ++p3d2P )
   {
    memcpy(&linePts[0],p3d1P,sizeof(DPoint3d)) ;
    memcpy(&linePts[1],p3d2P,sizeof(DPoint3d)) ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Breakline,1,1,&dtmP->nullFeatureId,linePts,2) ) goto errexit ;
   }
 for( p3d1P = poly2PtsP , p3d2P = poly2PtsP + 1 ; p3d2P < poly2PtsP + numPoly2Pts ; ++p3d1P , ++p3d2P )
   {
    memcpy(&linePts[0],p3d1P,sizeof(DPoint3d)) ;
    memcpy(&linePts[1],p3d2P,sizeof(DPoint3d)) ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Breakline,2,1,&dtmP->nullFeatureId,linePts,2) ) goto errexit ;
   }
/*
** Calculate Machine Tolerance
*/
 if( bcdtmMath_calculateMachinePrecisionForDtmObject(dtmP)) goto errexit ;
 dtmP->ppTol = dtmP->plTol = dtmP->mppTol * 1000.0 ;

/*
**  Triangulate Dtm Object
*/
 if( bcdtmObject_createTinDtmObject(dtmP,1,0.0) ) goto errexit ;
/*
** Remove None Feature Hull Lines
*/
 if( bcdtmList_removeNoneFeatureHullLinesDtmObject(dtmP)) goto errexit ;
/*
** Get Intersect Polygons
*/
 if( bcdtmClip_getExclusiveIntersectPolygonsDtmObject(dtmP,*polyPP) ) goto errexit ;
/*
** Set Return Values
*/
 if( (*polyPP)->numPolygons > 0 ) *clipResult = 1 ;
/*
** Count Hull Points
*/
 else
   {
    numHullPts = numPoly1DtmPts = numPoly2DtmPts = 0 ;
    sp = dtmP->hullPoint ;
    do
      {
       ++numHullPts ;
       fPtr = nodeAddrP(dtmP,sp)->fPtr ;
       while( fPtr != dtmP->nullPtr )
         {
          if(ftableAddrP(dtmP,flistAddrP(dtmP,fPtr)->dtmFeature)->dtmUserTag == 1 ) ++numPoly1DtmPts ;
          if(ftableAddrP(dtmP,flistAddrP(dtmP,fPtr)->dtmFeature)->dtmUserTag == 2 ) ++numPoly2DtmPts ;
          fPtr = flistAddrP(dtmP,fPtr)->nextPtr ;
         } 
       sp = nodeAddrP(dtmP,sp)->hPtr ;
      } while ( sp != dtmP->hullPoint ) ;
/*
**  Check For Polygon One Totally Within Polygon Two
*/
    if( numPoly2DtmPts == numHullPts && numPoly1DtmPts != numHullPts )
      {
       *clipResult = 2 ; 
       if( bcdtmPolygon_storePointArrayPolygonInPolygonObject(*polyPP,poly1PtsP,numPoly1Pts,1)) goto errexit ;
      } 
/*
**  Check For Polygon Two Totally Within Polygon One
*/
    if( numPoly1DtmPts == numHullPts && numPoly2DtmPts != numHullPts )
      {
       *clipResult = 3 ; 
       if( bcdtmPolygon_storePointArrayPolygonInPolygonObject(*polyPP,poly2PtsP,numPoly2Pts,2)) goto errexit ;
      } 
/*
**  Check For Coincident Polygons
*/
    if( numPoly1DtmPts == numHullPts && numPoly2DtmPts == numHullPts )
      {
       *clipResult = 4 ; 
       if( bcdtmPolygon_storePointArrayPolygonInPolygonObject(*polyPP,poly2PtsP,numPoly2Pts,2) ) goto errexit ;
      } 
   }
/*
** Cleanup
*/
 cleanup :
 if( dtmP  != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ; 
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 if( *polyPP != NULL ) bcdtmPolygon_deletePolygonObject(polyPP) ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmClip_getExclusiveIntersectPolygonsDtmObject(BC_DTM_OBJ *dtmP,DTM_POLYGON_OBJ *polyP) 
/*
** This Function Gets The Exclusive Intersect Polygons 
** And Copies Them To The Polygon Object
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ; 
 long    pp,sp,np,np1,np2,spnt,clc,feature,numFeatureTable ;
 DTMUserTag usertag,usertag1,usertag2 ;
/*
** Write Status Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Getting DTM Feature Intersect Polygons") ;
    bcdtmWrite_toFileDtmObject(dtmP,L"intersect.tin") ;
   }
/*
** Initialise
*/
 usertag  = dtmP->nullUserTag ;
 usertag1 = dtmP->nullUserTag ;
 usertag2 = dtmP->nullUserTag ;
/*
** Scan Hull Points For More Than One Feature
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Hull Points") ;
 spnt = dtmP->hullPoint ;
 do
   {
    if( nodeAddrP(dtmP,spnt)->cPtr != dtmP->nullPtr && ! nodeAddrP(dtmP,spnt)->PRGN ) 
      {
/*
**     Determine Number Of Features On Point
*/
       bcdtmList_countNumberOfDtmFeaturesForPointDtmObject(dtmP,spnt,&numFeatureTable) ;
/*
**     If More Than One Feature At Point Then Polygons Intersect
*/
       if( numFeatureTable > 1 ) 
         {
          if( dbg )
            {
             if( nodeAddrP(dtmP,spnt)->hPtr == dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"Internal Point = %6ld Number Features = %6ld",spnt,numFeatureTable) ;
             else                                        bcdtmWrite_message(0,0,0,"External Point = %6ld Number Features = %6ld",spnt,numFeatureTable) ;
             bcdtmWrite_message(0,0,0,"spnt = %6ld ** %10.4lf %10.4lf %8.4lf",spnt,pointAddrP(dtmP,spnt)->x,pointAddrP(dtmP,spnt)->y,pointAddrP(dtmP,spnt)->z) ;
             sp = nodeAddrP(dtmP,spnt)->hPtr ;
             bcdtmWrite_message(0,0,0,"fTableP = %6ld ** %10.4lf %10.4lf %8.4lf",sp,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z) ;
            }
/*
**        Get Next Point For Features
*/
          clc = nodeAddrP(dtmP,spnt)->fPtr ;
          np1 = np2 = dtmP->nullPnt ;
          while( clc != dtmP->nullPtr && np2 == dtmP->nullPnt )
            {
             feature = flistAddrP(dtmP,clc)->dtmFeature ;
             usertag = ftableAddrP(dtmP,feature)->dtmUserTag ; 
             bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,feature,spnt,&np) ;
             if( np1 == dtmP->nullPnt ) { np1 = np ; usertag1 = usertag ; }
             else                     { np2 = np ; usertag2 = usertag ; }
             clc = flistAddrP(dtmP,clc)->nextPtr ;
            }
          if( dbg )
            {
             bcdtmWrite_message(0,0,0,"Hull Point Intersection") ;
             bcdtmWrite_message(0,0,0,"Np1 = %6ld ** %10.4lf %10.4lf %8.4lf",np1,pointAddrP(dtmP,np1)->x,pointAddrP(dtmP,np1)->y,pointAddrP(dtmP,np1)->z) ;
             bcdtmWrite_message(0,0,0,"Np2 = %6ld ** %10.4lf %10.4lf %8.4lf",np2,pointAddrP(dtmP,np2)->x,pointAddrP(dtmP,np2)->y,pointAddrP(dtmP,np2)->z) ;
             bcdtmWrite_message(0,0,0,"UserTag 1 = %1I64ld UserTag2 = %1I64ld",usertag1,usertag2) ;
            }
/*
**        External Point
*/
          if( np1 != np2 ) 
            { 
/*
**           Determine Next Point
*/
             np = dtmP->nullPnt ;
             if( usertag1 == 1 ) { np = np1 ; usertag = usertag1 ; } 
             if( usertag2 == 1 ) { np = np2 ; usertag = usertag2 ; } 
/*
**           Scan Back To Start Point
*/
             if( np != dtmP->nullPnt && nodeAddrP(dtmP,spnt)->hPtr == np )
               { 
                if( dbg )
                  {
                   bcdtmWrite_message(0,0,0,"Next Point = %6ld ** %10.4lf %10.4lf %8.4lf",np,pointAddrP(dtmP,np)->x,pointAddrP(dtmP,np)->y,pointAddrP(dtmP,np)->z) ;
                   bcdtmWrite_message(0,0,0,"UserTag    = %1ld ",usertag) ;
                  } 
                sp = spnt ;
                do
                  { 
                   nodeAddrP(dtmP,sp)->tPtr = np ;
                   if( dbg ) bcdtmWrite_message(0,0,0,"sp = %6ld ** %10.4lf %10.4lf %10.4lf",sp,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z) ;
                   pp = sp ; sp = np ; np = pp ;
                   if( (np = bcdtmList_nextClkDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
                   while( ! bcdtmList_testForLineOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Breakline,sp,np))
                     { if( (np = bcdtmList_nextClkDtmObject(dtmP,sp,np)) < 0 ) goto errexit ; }
                  } while ( sp != spnt ) ;
/*
**              Copy Internal Tptr Polygon To Polygon Object
*/
                if( bcdtmPolygon_storeTptrPolygonInPolygonObjectDtmObject(dtmP,polyP,spnt,(long)usertag)) goto errexit ;
/*
**              Mark Tptr Polygon To Polygon Object
*/
                sp = spnt ;
                do
                  {
                   nodeAddrP(dtmP,sp)->PRGN = 1 ;
                   sp = nodeAddrP(dtmP,sp)->tPtr ;
                  } while ( sp != spnt ) ;
/*
**               Null Out Tptr Polygon
*/         
                bcdtmList_nullTptrListDtmObject(dtmP,spnt) ;
               }
            }
         }
      }
/*
**  Get Next Point On Hull
*/
    spnt = nodeAddrP(dtmP,spnt)->hPtr ;     
   } while ( spnt != dtmP->hullPoint ) ;
/*
** Scan Internal Points For More Than One Feature
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Internal Points") ;
 for( spnt = 0 ; spnt < dtmP->numPoints ; ++spnt )
   {
    if( nodeAddrP(dtmP,spnt)->hPtr == dtmP->nullPnt && nodeAddrP(dtmP,spnt)->cPtr != dtmP->nullPtr && nodeAddrP(dtmP,spnt)->PRGN != 2 ) 
      {
/*
**     Determine Number Of Features On Point
*/
       bcdtmList_countNumberOfDtmFeaturesForPointDtmObject(dtmP,spnt,&numFeatureTable) ;
/*
**     If More Than One Feature At Point Then Polygons Intersect
*/
       if( numFeatureTable > 1 ) 
         {
          if( dbg )
            {
             bcdtmWrite_message(0,0,0,"spnt = %6ld fTableP = %9ld ** %10.4lf %10.4lf %8.4lf",spnt,nodeAddrP(dtmP,spnt)->hPtr,pointAddrP(dtmP,spnt)->x,pointAddrP(dtmP,spnt)->y,pointAddrP(dtmP,spnt)->z) ;
            }
/*
**        Get Next Point For Features
*/
          clc = nodeAddrP(dtmP,spnt)->fPtr ;
          np1 = np2 = dtmP->nullPnt ;
          while( clc != dtmP->nullPtr && np2 == dtmP->nullPnt )
            {
             feature = flistAddrP(dtmP,clc)->dtmFeature ;
             usertag = ftableAddrP(dtmP,feature)->dtmUserTag ; 
             bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,feature,spnt,&np) ;
             if( np1 == dtmP->nullPnt ) { np1 = np ; usertag1 = usertag ; }
             else                     { np2 = np ; usertag2 = usertag ; }
             clc = flistAddrP(dtmP,clc)->nextPtr ;
            }
          if( dbg )
            {
             bcdtmWrite_message(0,0,0,"Internal Point Intersection") ;
             bcdtmWrite_message(0,0,0,"Np1 = %6ld Np2 = %6ld",np1,np2) ;
             bcdtmWrite_message(0,0,0,"Np1 = %6ld ** %10.4lf %10.4lf %8.4lf",np1,pointAddrP(dtmP,np1)->x,pointAddrP(dtmP,np1)->y,pointAddrP(dtmP,np1)->z) ;
             bcdtmWrite_message(0,0,0,"Np2 = %6ld ** %10.4lf %10.4lf %8.4lf",np2,pointAddrP(dtmP,np2)->x,pointAddrP(dtmP,np2)->y,pointAddrP(dtmP,np2)->z) ;
             bcdtmWrite_message(0,0,0,"UserTag 1 = %1I64ld UserTag2 = %1I64ld",usertag1,usertag2) ;
            }
/*
**        Determine Next Point
*/
          np = dtmP->nullPnt ;
          if( usertag1 == 2 ) { np = np1 ; usertag = usertag1 ; } 
          if( usertag2 == 2 ) { np = np2 ; usertag = usertag2 ; } 
          if( np != dtmP->nullPnt ) bcdtmList_countNumberOfDtmFeaturesForPointDtmObject(dtmP,np,&numFeatureTable) ;
/*
**        Scan Back To Start Point
*/
          if( np != dtmP->nullPnt && ! nodeAddrP(dtmP,np)->PRGN && numFeatureTable == 1 )
            { 
             if( dbg )
               {
                bcdtmWrite_message(0,0,0,"Next Point = %6ld ** %10.4lf %10.4lf %8.4lf",np,pointAddrP(dtmP,np)->x,pointAddrP(dtmP,np)->y,pointAddrP(dtmP,np)->z) ;
                bcdtmWrite_message(0,0,0,"UserTag    = %1ld ",usertag) ;
               } 
             sp = spnt ;
             do
               { 
                nodeAddrP(dtmP,sp)->tPtr = np ;
                if( dbg ) bcdtmWrite_message(0,0,0,"sp = %6ld ** %10.4lf %10.4lf %10.4lf",sp,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z) ;
                pp = sp ; sp = np ; np = pp ;
                if( (np = bcdtmList_nextAntDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
                while( ! bcdtmList_testForLineOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Breakline,sp,np))
                  { if( (np = bcdtmList_nextAntDtmObject(dtmP,sp,np)) < 0 ) goto errexit ; }
               } while ( sp != spnt ) ;
/*
**           Reverse Direction
*/
             if( bcdtmList_reverseTptrPolygonDtmObject(dtmP,spnt)) goto errexit ;
/*
**           Copy Internal Tptr Polygon To Polygon Object
*/
             if( bcdtmPolygon_storeTptrPolygonInPolygonObjectDtmObject(dtmP,polyP,spnt,(long)usertag)) goto errexit ;
/*
**           Mark Tptr Polygon To Polygon Object
*/
             sp = spnt ;
             do
               {
                nodeAddrP(dtmP,sp)->PRGN = 2 ;
                sp = nodeAddrP(dtmP,sp)->tPtr ;
               } while ( sp != spnt ) ;
/*
**           Null Out Tptr Polygon
*/         
             bcdtmList_nullTptrListDtmObject(dtmP,spnt) ;
            }
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job completed
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
BENTLEYDTM_Public int bcdtmClip_modifyIslandHullsForExternalVoidsDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Resolves Adjoining Polygonal Features
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   sp,np,pp,ssp,lp,islandFeature,voidFeature,feature ;
 DTMDirection direction;
 long   islandStartPnt = 0,islandPnt,voidPnt ;
 double area ;
 DTMUserTag    userTag ;
 DTMFeatureId  userFeatureId ;
 BC_DTM_FEATURE  *dtmFeatureP  ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Voids On Island Hulls") ;
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Island Features") ;
    for( islandFeature = 0 ; islandFeature < dtmP->numFeatures ; ++islandFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,islandFeature) ;
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeatureType == DTMFeatureType::Island )
         {
          bcdtmWrite_message(0,0,0,"Feature = %6ld firstPnt = %6ld TYPE = %4ld userTag = %9ld",islandFeature,dtmFeatureP->dtmFeaturePts.firstPoint,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag) ;
         }
      }
   } 
/*
** Scan Island Features
*/
 for( islandFeature = 0 ; islandFeature < dtmP->numFeatures ; ++islandFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,islandFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeatureType == DTMFeatureType::Island )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Island Feature = %6ld firstPnt = %6ld TYPE = %4ld userTag = %9ld",islandFeature,dtmFeatureP->dtmFeaturePts.firstPoint,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag) ;
       sp = dtmFeatureP->dtmFeaturePts.firstPoint ;
       userTag  = dtmFeatureP->dtmUserTag ;
       userFeatureId = dtmFeatureP->dtmFeatureId ;
       if( bcdtmList_getPriorPointForDtmFeatureDtmObject(dtmP,islandFeature,sp,&pp)) goto errexit ;
       do
         {
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,islandFeature,sp,&np)) goto errexit ;
          if( dbg ) bcdtmWrite_message(0,0,0,"sp = %6ld np = %6ld",sp,np) ;
/*
**        Test For Internal Void Hull
*/
          voidFeature = dtmP->nullPnt ;
          if( ( lp = bcdtmList_nextAntDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
          while( lp != pp && ! bcdtmClip_testForVoidOrHoleHullLineDtmObject(dtmP,sp,lp,&voidFeature) )
            {
             if( ( lp = bcdtmList_nextAntDtmObject(dtmP,sp,lp)) < 0 ) goto errexit ;
            }
/*
**        Test For Internal Void On Island Hull
*/
          if( voidFeature != dtmP->nullPnt )  
            {
             if( dbg ) 
               {
                bcdtmWrite_message(0,0,0,"Adjoining Void Feature = %6ld firstPnt = %6ld TYPE = %4ld userTag = %9ld",voidFeature,ftableAddrP(dtmP,voidFeature)->dtmFeaturePts.firstPoint,ftableAddrP(dtmP,voidFeature)->dtmFeatureType,ftableAddrP(dtmP,voidFeature)->dtmUserTag) ;
                bcdtmWrite_message(0,0,0,"pp = %6ld ** %10.4lf %10.4lf %10.4lf",pp,pointAddrP(dtmP,pp)->x,pointAddrP(dtmP,pp)->y,pointAddrP(dtmP,pp)->z) ;
                bcdtmWrite_message(0,0,0,"sp = %6ld ** %10.4lf %10.4lf %10.4lf",sp,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z) ;
                bcdtmWrite_message(0,0,0,"np = %6ld ** %10.4lf %10.4lf %10.4lf",np,pointAddrP(dtmP,np)->x,pointAddrP(dtmP,np)->y,pointAddrP(dtmP,np)->z) ;
               }
/*
**           Copy Island Hull To Tptr Polygon
*/            
             if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,islandFeature,&islandPnt) ) goto errexit ;
             if( dbg )
               {   
                if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,islandPnt,&area,&direction) ) goto errexit ;
                bcdtmWrite_message(0,0,0,"Island Polygon Area = %15.4lf Direction = %2ld",area,direction) ;
               }
/*
**          Scan In Reverse Direction Around Void HULL
*/
             ssp = sp ;
             do
               {
                if( ( lp = bcdtmList_nextAntDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
                while( ! bcdtmClip_testForVoidOrHoleHullLineDtmObject(dtmP,sp,lp,&feature) )
                  {
                   if( ( lp = bcdtmList_nextAntDtmObject(dtmP,sp,lp)) < 0 ) goto errexit ;
                  }
                if( nodeAddrP(dtmP,lp)->tPtr != sp) { nodeAddrP(dtmP,sp)->tPtr = lp ; islandStartPnt = lp ; }
                np = sp ;
                sp = lp ; 
               } while ( sp != ssp ) ; 
/*
**          Check Connectivity Of Inserted Tptr Polygon
*/
            if( dbg )
              {
               bcdtmWrite_message(0,0,0,"Checking Connectivity Of Void Polygon") ;
               if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,islandStartPnt,0)) 
                 {
                  bcdtmWrite_message(0,0,0,"Connectivity Error In Void Polygon") ;
                  bcdtmList_writeTptrListDtmObject(dtmP,islandStartPnt) ; 
                  goto errexit ;  
                 }
               if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,islandStartPnt,&area,&direction) ) goto errexit ;
               bcdtmWrite_message(0,0,0,"Intersected Island Polygon Area = %15.4lf Direction = %2ld",area,direction) ;
              }
/*
**          Insert Modified Island Feature
*/
            if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,DTMFeatureType::Island,userTag,userFeatureId,islandStartPnt,1)) goto errexit ; 
/*
**          Delete Island
*/
            if( bcdtmInsert_removeDtmTinFeatureFromDtmObject(dtmP,islandFeature)) goto errexit ;
/*
**          Copy Void Hull To Tptr List
*/
            if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,voidFeature,&voidPnt)) goto errexit ;
/*
**          Delete Void Feature
*/
            if( bcdtmInsert_removeDtmTinFeatureFromDtmObject(dtmP,voidFeature)) goto errexit ;
/*
**          Null Out Tptr List
*/
            bcdtmList_nullTptrListDtmObject(dtmP,voidPnt) ;
/*
**          Set Np To Stop Scan
*/
             np = dtmFeatureP->dtmFeaturePts.firstPoint ;
            }
          pp = sp ;
          sp = np ;
         } while ( sp != dtmFeatureP->dtmFeaturePts.firstPoint ) ;  
      } 
   }
/*
** Compact Features
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Compacting Feature Table") ;
 if( bcdtmTin_compactFeatureTableDtmObject(dtmP)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Compacting Feature List") ;
 if( bcdtmTin_compactFeatureListDtmObject(dtmP) ) goto errexit ;
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
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR  ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmClip_linearDtmFeatureDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,long mark)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   nextPnt, priorPoint, unMarkedPoint, markedPoint, process;
 DTMFeatureType dtmFeatureType;
 DTMUserTag userTag  ;
 DTMFeatureId     userFeatureId ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Internally Clipping Linear Dtm Feature %6ld",dtmFeature) ;
/*
** Initialise Scan Of Feature
*/
 if( bcdtmClip_initialiseMarkScanDtmObject(dtmP,dtmFeature,mark) ) goto errexit ;
/*
** Initialise Varaibles
*/
 nextPnt         = ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ; 
 dtmFeatureType  = ftableAddrP(dtmP,dtmFeature)->dtmFeatureType ;
 userTag         = ftableAddrP(dtmP,dtmFeature)->dtmUserTag  ;
 userFeatureId        = ftableAddrP(dtmP,dtmFeature)->dtmFeatureId ;
 priorPoint      = dtmP->nullPnt ;
/*
** Scan And Extract Clipped Sections Of Feature
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning And Extracting Clipped Sections Of Feature") ;
 process = 1 ;
 while ( process )
   {
    if( bcdtmClip_scanToMarkedPointDtmFeatureDtmObject(dtmP,dtmFeature,nextPnt,dtmP->nullPnt,&unMarkedPoint,&priorPoint)) goto errexit ;
    if( unMarkedPoint == dtmP->nullPnt ) process = 0 ;
    else
      {
       bcdtmClip_scanToMarkedPointDtmFeatureDtmObject(dtmP,dtmFeature,unMarkedPoint,mark,&markedPoint,&priorPoint) ;
       if( unMarkedPoint != priorPoint )
         { 
          if( bcdtmClip_placeLinearDtmFeatureSectionIntoTptrListDtmObject(dtmP,dtmFeature,unMarkedPoint,priorPoint)) goto errexit ; 
          if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,dtmFeatureType,userTag,userFeatureId,unMarkedPoint,1)) goto errexit ; 
         }
       if( markedPoint == dtmP->nullPnt ) process = 0 ;
       else                              nextPnt = markedPoint ;
      } 
   }  
/*
** Delete Origonal Feature From Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Dtm Feature") ;
 if( bcdtmInsert_removeDtmTinFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
** Normal Exit
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Internally Clipping Linear Dtm Feature %6ld Completed",dtmFeature) ; 
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Internally Clipping Linear Dtm Feature %6ld Error",dtmFeature) ; 
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
BENTLEYDTM_Private int bcdtmClip_initialiseMarkScanDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,long mark )
/*
** This Function Initialises The Scan For A Closed Dtm Feature.
** It Sets The First Point Of A Closed Feature To A Marked Point
**
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long nextPnt,firstPnt,clPtr ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Initialising Mark Scan For dtmFeature = %6ld",dtmFeature) ;
/*
** Initialise
*/
 nextPnt = firstPnt = ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ; 
 if( nextPnt == dtmP->nullPnt ) { bcdtmWrite_message(1,0,0,"Error Initialising Scan Of Dtm Feature") ; goto errexit ; }
/*
** Only Scan If First Point Not Marked
*/
 if( nodeAddrP(dtmP,firstPnt)->tPtr != mark ) 
   { 
/*
** Check For A Closed Dtm Feature
*/
    clPtr = nodeAddrP(dtmP,nextPnt)->fPtr ;
    while ( clPtr != dtmP->nullPtr )
      {
       while ( clPtr != dtmP->nullPtr  && flistAddrP(dtmP,clPtr)->dtmFeature != dtmFeature ) clPtr = flistAddrP(dtmP,clPtr)->nextPtr ;
       if( clPtr != dtmP->nullPtr )
         {
          nextPnt = flistAddrP(dtmP,clPtr)->nextPnt ;
          if( nextPnt != dtmP->nullPnt ) clPtr = nodeAddrP(dtmP,nextPnt)->fPtr ;
          if( nextPnt == dtmP->nullPnt || nextPnt == firstPnt ) clPtr = dtmP->nullPtr ;
         }
      }
/*
** Write Dtm Feature Close Status
*/ 
    if( dbg )
      {
       if( nextPnt == firstPnt ) bcdtmWrite_message(0,0,0,"Dtm Feature Closes") ;
       else                      bcdtmWrite_message(0,0,0,"Dtm Feature Open") ;
      } 
/*
** If Dtm Feature Closes Scan To First Marked Point
*/
    if( nextPnt == firstPnt ) 
      { 
       clPtr = nodeAddrP(dtmP,nextPnt)->fPtr ;
       while ( clPtr != dtmP->nullPtr )
         {
          while ( clPtr != dtmP->nullPtr  && flistAddrP(dtmP,clPtr)->dtmFeature != dtmFeature ) clPtr = flistAddrP(dtmP,clPtr)->nextPtr ;
          if( clPtr != dtmP->nullPtr )
            {
             nextPnt = flistAddrP(dtmP,clPtr)->nextPnt ;
             clPtr = nodeAddrP(dtmP,nextPnt)->fPtr ;
             if( nodeAddrP(dtmP,nextPnt)->tPtr == mark ) clPtr = dtmP->nullPtr ;
            }
         }
/*
** Set Dtm Feature First Point To The Marked Point
*/
       ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint = nextPnt ;
/*
** Write New Start Point
*/
        if( dbg ) bcdtmWrite_message(0,0,0,"Dtm Feature Start Point = %8ld",ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint) ;
       }
    }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Initialising Mark Scan For dtmFeature = %6ld Completed",dtmFeature) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Initialising Mark Scan For dtmFeature = %6ld Error",dtmFeature) ;
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
BENTLEYDTM_Private int bcdtmClip_scanToMarkedPointDtmFeatureDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,long startPnt,long mark,long *markPoint,long *priorPoint)
/*
** This Function Scans To The First Marked Or Unmarked Point In The Dtm Feature List
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;  
 long nextPnt,firstPnt,lastPnt,clPtr ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning To Marked Point Dtm Feature = %6ld",dtmFeature) ;
/*
** Initialise
*/
 *markPoint = dtmP->nullPnt ;
 lastPnt    = dtmP->nullPnt ;
/*
** Test For Second Time Trough Closed Dtm Feature
*/
 if( ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint == startPnt && *priorPoint != dtmP->nullPnt ) *priorPoint = dtmP->nullPnt ;
/*
** Test For First Point
*/
 else
   {
    nextPnt  = startPnt  ;
    if( nodeAddrP(dtmP,nextPnt)->tPtr == mark ) 
      { 
       *markPoint  = nextPnt ; 
       *priorPoint = dtmP->nullPnt ;
      }
/*
** Initialise
*/
    else
      { 
       clPtr  = nodeAddrP(dtmP,nextPnt)->fPtr ;
       firstPnt = ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ;
/*
** Scan dtmFeature List Pointers
*/
       while ( clPtr != dtmP->nullPtr )
         {
          while ( clPtr != dtmP->nullPtr  && flistAddrP(dtmP,clPtr)->dtmFeature != dtmFeature ) clPtr = flistAddrP(dtmP,clPtr)->nextPtr ;
          if( clPtr != dtmP->nullPtr )
            {
             lastPnt = nextPnt ;
             nextPnt = flistAddrP(dtmP,clPtr)->nextPnt ;
             if( nextPnt != dtmP->nullPnt )
               {  
                clPtr = nodeAddrP(dtmP,nextPnt)->fPtr ;
                if( nodeAddrP(dtmP,nextPnt)->tPtr == mark ) 
                  { 
                   *markPoint  = nextPnt  ; 
                   *priorPoint = lastPnt  ; 
                   clPtr = dtmP->nullPtr ; 
                  }
               } 
             if( nextPnt == dtmP->nullPnt || nextPnt == firstPnt ) clPtr = dtmP->nullPtr ;
            }
         }
/*
** Set Prior Point
*/
       *priorPoint = lastPnt ; 
      }
   } 
/*
** Job Completed
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning To Marked Point Dtm Feature = %6ld Completed",dtmFeature) ;
 return(ret) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmClip_externalToTptrPolygonDtmObject(BC_DTM_OBJ *dtmP,long startPnt)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   sp,np,cp,lp,clc,numMarked,mark=-98989898 ;
 long   numPts,numLines;
 DTMDirection direction;
 double area ;
 DTM_TIN_NODE *nodeP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Clipping External To Tptr Polygon") ;
/*
** Check direction Of Tptr Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking direction Of Tptr Polygon") ;
 bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPnt,&area,&direction) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Tptr Polygon direction = %2ld area = %12.4lf",direction,area) ;
 if( direction == DTMDirection::Clockwise ) if( bcdtmList_reverseTptrPolygonDtmObject(dtmP,startPnt)) goto errexit ;
/*
** Mark Points External To Tptr Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points External To Clip Polygon") ;
 bcdtmMark_externalTptrPolygonPointsDtmObject(dtmP,startPnt,mark,&numMarked) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of External Points = %6ld",numMarked) ;
/*
** Clip Dtm Features External To Tptr Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Clipping DTM Features External To Clip Polygon")  ;
 if( bcdtmClip_dtmFeaturesExternalToTptrPolygonDtmObject(dtmP,startPnt,mark)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Clipping DTM Features External To Clip Polygon Completed")  ;
/*
** Check Topology Of DTM Features - Development Only
*/
 if( cdbg )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking DTM Feature Topology") ;
    if( bcdtmCheck_topologyDtmFeaturesDtmObject(dtmP,0)) { bcdtmWrite_message(0,0,0,"Topology Error In DTM Features") ; goto errexit ; }
    else                                                   bcdtmWrite_message(0,0,0,"Topology DTM Features OK") ;
   }
/*
** Delete All Lines Connecting To External Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Lines Connecting To External Points") ;
 numPts   = 0 ;
 numLines = 0 ;
 for( sp = 0 ; sp < dtmP->numPoints ; ++sp )
   {
    if( nodeAddrP(dtmP,sp)->tPtr == mark )
      {
       clc = nodeAddrP(dtmP,sp)->cPtr ;
       while ( clc != dtmP->nullPtr )
         {
          cp  = clistAddrP(dtmP,clc)->pntNum ;
          clc = clistAddrP(dtmP,clc)->nextPtr ;
          if( bcdtmList_deleteLineDtmObject(dtmP,sp,cp) ) goto errexit ;
         ++numLines ;
         }
       nodeP = nodeAddrP(dtmP,sp) ; 
       nodeP->hPtr = dtmP->nullPnt ;
       nodeP->tPtr = dtmP->nullPnt ;
       nodeP->sPtr = dtmP->nullPnt ;
       nodeP->cPtr = dtmP->nullPtr ;
       nodeP->fPtr = dtmP->nullPtr ;
       ++numPts ;
      }
   }
/*
** Write Statistics
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Points Deleted = %6ld",numPts) ;
    bcdtmWrite_message(0,0,0,"Number Of Lines  Deleted = %6ld",numLines) ;
   }
/*
** Delete All External Lines Connected To Tptr Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Deleting Lines External To tPtr Polygon") ;
 numLines = 0 ;
 sp = startPnt ; 
 do
   {
    np = nodeAddrP(dtmP,sp)->tPtr ;
    if( ( cp = bcdtmList_nextClkDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
    while ( nodeAddrP(dtmP,cp)->tPtr != sp )
      {
       if(( lp = bcdtmList_nextClkDtmObject(dtmP,sp,cp)) < 0 ) goto errexit ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Deleting %10ld %10ld",sp,cp) ;      
       if( bcdtmList_deleteLineDtmObject(dtmP,sp,cp)) goto errexit ;
       ++numLines ;
       cp = lp ;
      }
    sp = np ; 
   } while( sp != startPnt ) ;
/*
** Write Statistics
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Lines Deleted = %6ld",numLines) ;
/*
** Copy Tptr List To Hptr List
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying Tptr List To Hptr List") ;
 if( bcdtmList_copyTptrListToHptrListDtmObject(dtmP,startPnt)) goto errexit ;
 dtmP->hullPoint = startPnt ; 
 dtmP->nextHullPoint = nodeAddrP(dtmP,startPnt)->hPtr ;
/*
** Null Out Tptr List
*/ 
 if( dbg ) bcdtmWrite_message(0,0,0,"Nulling Out Tptr List") ;
 if( bcdtmList_nullTptrListDtmObject(dtmP,startPnt)) goto errexit ;
/*
** Clean Polygonal Dtm Features On Dtm Hull
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Cleaning Polygonal Features On Dtm Hull") ;
 if( bcdtmClip_externalCleanPolygonalFeaturesDtmObject(dtmP) ) goto errexit ; 
/*
** Check Tin
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking DTM After External Clip") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP)) 
      {
       bcdtmWrite_message(0,0,0,"DTM Corrupted After External Clip") ;
       goto errexit  ;
      }
    else bcdtmWrite_message(0,0,0,"DTM OK After External Clip") ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Clipping External To Tptr Polygon Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Clipping External To Tptr Polygon Error") ;
 return(ret) ;
/*
** Error Return
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
BENTLEYDTM_Public int bcdtmClip_dtmFeaturesExternalToTptrPolygonDtmObject(BC_DTM_OBJ *dtmP,long startPnt,long mark)
/*
** This Function Clips Dtm Features External To Clip Polygon
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ; 
 long    clc,spnt,fspnt,dtmFeature,numFeatures ;
 DTMFeatureType dtmFeatureType;
 long    numMarked,numPts,numClipPts ;
 char    dtmFeatureTypeName[50] ;
 DPoint3d     *clipPtsP=NULL ;
 BC_DTM_FEATURE  *dtmFeatureP ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Externally Clipping DTM Features") ;
/*
** Copy The Clip Boundary To A DPoint3d Array
*/
 if( bcdtmList_copyTptrListToPointArrayDtmObject(dtmP,startPnt,&clipPtsP,&numClipPts)) goto errexit ;
/*
**  Copy Tptr List To Sptr and Null Out Tptr
*/
 if( bcdtmList_nullSptrValuesDtmObject(dtmP)) goto errexit ;
 if( bcdtmList_copyTptrListToSptrListDtmObject(dtmP,startPnt)) goto errexit ;
 if( bcdtmList_nullTptrListDtmObject(dtmP,startPnt)) goto errexit ;
/*
** Scan Feature Lists And Detect And Fix Feature Lines That Span Concave Clip Boundaries
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Clipping And Fixing Concave Sections") ;
// if( bcdtmClip_scanFeatureListAndDetectAndFixConcaveSpansDtmObject(dtmP,mark) ) goto errexit ;
 if( bcdtmClip_scanFeatureListAndMarkConcaveSpansDtmObject(dtmP,mark)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Clipping And Fixing Concave Sections Completed") ;
/*
** Scan Feature Table And Look For Clipped Linear Features
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Feature Table For Clipped Features") ;
 numFeatures = dtmP->numFeatures ;
 for( dtmFeature = 0 ; dtmFeature < numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeaturePts.firstPoint != dtmP->nullPnt )
      { 
       dtmFeatureType = dtmFeatureP->dtmFeatureType ;
/*
**     Initialise Scan Variables
*/
       numPts = 1 ; 
       numMarked = 0 ; 
       fspnt = spnt  = dtmFeatureP->dtmFeaturePts.firstPoint  ;
       if( dbg ) bcdtmWrite_message(0,0,0,"dtmFeature = %8ld ** dtmFeatureType = %2ld firstPoint = %8ld",dtmFeature,dtmFeatureType,fspnt) ;
       clc   = nodeAddrP(dtmP,fspnt)->fPtr ;
       if( nodeAddrP(dtmP,fspnt)->tPtr == mark ) ++numMarked ;
/*
**     Scan Feature List Pointers
*/
       while ( clc != dtmP->nullPtr )
         {
          while ( clc != dtmP->nullPtr  && flistAddrP(dtmP,clc)->dtmFeature != dtmFeature ) clc = flistAddrP(dtmP,clc)->nextPtr ;
          if( clc != dtmP->nullPtr )
            {
             spnt = flistAddrP(dtmP,clc)->nextPnt ;
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"dtmFeatureType = %2ld firstPoint = %8ld nextPoint = %8ld",dtmFeatureType,fspnt,spnt) ;
             if( spnt != dtmP->nullPnt )
               {
                ++numPts ; 
                if( nodeAddrP(dtmP,spnt)->tPtr == mark ) ++numMarked ;
                clc = nodeAddrP(dtmP,spnt)->fPtr ;
               }
             if( spnt == dtmP->nullPnt || spnt == fspnt ) clc = dtmP->nullPtr ;
            }
         }
/*
**     Clip Feature
*/ 
       if( numMarked > 0 )
         { 
/*
**        Write Feature
*/
          if( dbg ) 
            {
             bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureType,dtmFeatureTypeName) ;
             bcdtmWrite_message(0,0,0,"Clipping Feature = %6ld of %6ld ** Type = %6ld Name = %s",dtmFeature,numFeatures,dtmFeatureType,dtmFeatureTypeName) ;
             bcdtmWrite_message(0,0,0,"numPts = %6ld numMarked = %6ld",numPts,numMarked) ;
            }
/*
**        Delete Origonal Feature From Dtm Object If All Feature Points marked
*/
          if( numPts == numMarked || ( dtmFeatureType != DTMFeatureType::GroupSpots && numPts == numMarked + 1) )
            {  
             if( dbg ) bcdtmWrite_message(0,0,0,"**** Removing Feature") ;
             if( bcdtmInsert_removeDtmTinFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ; 
            }

/*
**        Delete Clipped Sections Of Feature Only
*/
          else 
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"**** Clipping Feature %6ld",dtmFeature) ;
             switch ( dtmFeatureType ) 
               {
                case DTMFeatureType::GroupSpots :
                if( bcdtmClip_groupSpotDtmFeatureDtmObject(dtmP,dtmFeature,mark)) goto errexit ;
                break ;

                case DTMFeatureType::Polygon :
                case DTMFeatureType::Region  :
                case DTMFeatureType::Void    :
                case DTMFeatureType::Island  :
                case DTMFeatureType::Hole    :
                if( bcdtmClip_externalPolygonalDtmFeatureDtmObject(dtmP,dtmFeature,clipPtsP,numClipPts)) goto errexit ;
                break ;
               
                case DTMFeatureType::Breakline :
                case DTMFeatureType::ContourLine : 
                case DTMFeatureType::SoftBreakline :
                if( bcdtmClip_linearDtmFeatureDtmObject(dtmP,dtmFeature,mark)) goto errexit ;
                break ;

                default :
                bcdtmWrite_message(2,0,0,"Unknown DTM Feature Type = %8d",dtmFeatureType) ;
                goto errexit ; 
                break ;
               } ; 
            }
         }
      }
   }
/*
**  Copy Tptr List To Sptr and Null Out Sptr
*/
 if( bcdtmList_copySptrListToTptrListDtmObject(dtmP,startPnt)) goto errexit ;
 if( bcdtmList_nullSptrListDtmObject(dtmP,startPnt)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
 if( clipPtsP != NULL ) free(clipPtsP) ;
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Externally Clipping DTM Features Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Externally Clipping DTM Features Error") ;
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
BENTLEYDTM_Public int bcdtmClip_scanFeatureListAndMarkConcaveSpansDtmObject
(
 BC_DTM_OBJ *dtmP,
 long       mark
) 
/*
** Scan Feature Lists And Mark Feature Lines That Span Concave Clip Boundaries
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   lp,numMarked,outside,clPtr,nextPnt,firstPnt,lastPnt,dtmFeature ;
 BC_DTM_FEATURE  *dtmFeatureP ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking DTM Feature Concave Spans") ;
/*
** Check For Existence Of Features
*/
 numMarked = 0 ;
 if( dtmP->numFeatures > 0 ) 
   {
/*
** Scan Feature Lists
*/
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeatureType != DTMFeatureType::GroupSpots )
         {
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Dtm Feature = %6ld Type = %4ld Tag = %9I64d",dtmFeature,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag) ;
/*
**        Process Feature Marking External Parts
*/ 
          nextPnt = firstPnt = lastPnt = dtmFeatureP->dtmFeaturePts.firstPoint ;
          clPtr = nodeAddrP(dtmP,nextPnt)->fPtr ;
          while ( clPtr != dtmP->nullPtr )
            {
             while ( clPtr != dtmP->nullPtr  && flistAddrP(dtmP,clPtr)->dtmFeature != dtmFeature ) clPtr = flistAddrP(dtmP,clPtr)->nextPtr ;
             if( clPtr != dtmP->nullPtr )
               {
/*
**              Set Feature Line Points
*/
                lastPnt = nextPnt ;
                nextPnt = flistAddrP(dtmP,clPtr)->nextPnt ;
                if( nextPnt != dtmP->nullPnt )
                  {
/*
**                 Check If Feature Line End Points Are Not marked
*/ 
                   if( nodeAddrP(dtmP,lastPnt)->tPtr == dtmP->nullPnt && nodeAddrP(dtmP,nextPnt)->tPtr == dtmP->nullPnt )
                     {
/*
**                    Check The Feature Segment End Points Are On The Clip Polygon
*/
                      if( nodeAddrP(dtmP,lastPnt)->sPtr != dtmP->nullPnt && nodeAddrP(dtmP,nextPnt)->sPtr != dtmP->nullPnt )
                        {
/*
**                       Check The Feature Segment Is Not Coincident With The Clip Boundary
*/
                         if( nodeAddrP(dtmP,lastPnt)->sPtr != nextPnt || nodeAddrP(dtmP,nextPnt)->sPtr != lastPnt )
                           {                         
/*
**                          Check Feature Line Is Outside OF Clip Boundary
*/
                            outside = 0 ;
                            if( ( lp = bcdtmList_nextClkDtmObject(dtmP,lastPnt,nodeAddrP(dtmP,lastPnt)->sPtr)) < 0 ) 
                              {
                               bcdtmWrite_message(0,0,0,"Processing DTM Feature %8ld",dtmFeature) ;
                               goto errexit  ;
                              } 
                            while( nodeAddrP(dtmP,lp)->sPtr != lastPnt && ! outside )
                              {
                               if( lp == nextPnt ) outside = 1 ;  
                               else if( ( lp = bcdtmList_nextClkDtmObject(dtmP,lastPnt,lp)) < 0 ) 
                                 {
                                  bcdtmWrite_message(0,0,0,"Processing DTM Feature %8ld",dtmFeature) ;
                                  goto errexit  ;
                                 } 
                              }
                              
                            if( outside )
                              {
                               nodeAddrP(dtmP,lastPnt)->tPtr = mark ;
                               nodeAddrP(dtmP,nextPnt)->tPtr = mark ;
                               numMarked = numMarked + 2 ;
                              }
                           }
                        }   
                     }
                   clPtr   = nodeAddrP(dtmP,nextPnt)->fPtr ;
                  }
                if( nextPnt == dtmP->nullPnt || nextPnt == firstPnt )  clPtr = dtmP->nullPtr ; 
               }
            } 
         }
      } 
   }
/*
** Log Number Marked
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Features Marked = %8ld",numMarked) ;   
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking DTM Feature Concave Spans Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking DTM Feature Concave Spans Error") ;
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
BENTLEYDTM_Public int bcdtmClip_scanFeatureListAndDetectAndFixConcaveSpansDtmObject(BC_DTM_OBJ *dtmP,long Mark) 
/*
** Scan Feature Lists And Detect And Fix Lines That Span Concave Clip Boundaries
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   ap,cp,np,lp,outside,clPtr,nextPnt,firstPnt,lastPnt,dtmFeature ;
 double x,y,z ;
 BC_DTM_FEATURE  *dtmFeatureP ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Fixing DTM Feature Concave Spans") ;
/*
** Check For Existence Of Features
*/
 if( dtmP->numFeatures > 0 ) 
   {
/*
** Scan Feature Lists
*/
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeaturePts.firstPoint != dtmP->nullPnt && dtmFeatureP->dtmFeatureType != DTMFeatureType::GroupSpots )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Dtm Feature = %6ld Type = %4ld Tag = %9I64d",dtmFeature,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag) ;
/*
**        Process Feature Marking External Parts
*/ 
          nextPnt = firstPnt = lastPnt = dtmFeatureP->dtmFeaturePts.firstPoint ;
          clPtr = nodeAddrP(dtmP,nextPnt)->fPtr ;
          while ( clPtr != dtmP->nullPtr )
            {
             while ( clPtr != dtmP->nullPtr  && flistAddrP(dtmP,clPtr)->dtmFeature != dtmFeature ) clPtr = flistAddrP(dtmP,clPtr)->nextPtr ;
             if( clPtr != dtmP->nullPtr )
               {
/*
**              Set Feature Line Points
*/
                lastPnt = nextPnt ;
                nextPnt = flistAddrP(dtmP,clPtr)->nextPnt ;
                if( nextPnt != dtmP->nullPnt )
                  {
/*
**                 Check If Feature Line End Points Are Not Marked
*/ 
                   if( nodeAddrP(dtmP,lastPnt)->tPtr == dtmP->nullPnt && nodeAddrP(dtmP,nextPnt)->tPtr == dtmP->nullPnt )
                     {
/*
**                    Check Feature Line End Points Are On Clip Boundary
*/
                      if(nodeAddrP(dtmP,lastPnt)->sPtr != dtmP->nullPnt && nodeAddrP(dtmP,nextPnt)->sPtr != dtmP->nullPnt )
                        { 
/*
**                       Check Feature Line Is Not Coincident With Clip Boundary
*/
                         if(nodeAddrP(dtmP,lastPnt)->sPtr != nextPnt && nodeAddrP(dtmP,nextPnt)->sPtr != lastPnt )
                           {
                            outside = 0 ;
                            if( ( lp = bcdtmList_nextClkDtmObject(dtmP,lastPnt,nodeAddrP(dtmP,lastPnt)->sPtr)) < 0 ) goto errexit  ;
                            while( nodeAddrP(dtmP,lp)->sPtr != lastPnt && ! outside )
                              {
                               if( lp == nextPnt ) outside = 1 ;  
                               else if( ( lp = bcdtmList_nextClkDtmObject(dtmP,lastPnt,lp)) < 0 ) goto errexit  ;
                              }
                            if( outside )
                              {
                               if(( ap = bcdtmList_nextAntDtmObject(dtmP,lastPnt,nextPnt)) < 0 ) goto errexit  ;
                               if( ! bcdtmList_testLineDtmObject(dtmP,ap,nextPnt)) ap = dtmP->nullPnt ;
                               if(( cp = bcdtmList_nextClkDtmObject(dtmP,lastPnt,nextPnt)) < 0 ) goto errexit  ;
                               if( ! bcdtmList_testLineDtmObject(dtmP,cp,nextPnt)) cp = dtmP->nullPnt ;
                               x = (pointAddrP(dtmP,lastPnt)->x + pointAddrP(dtmP,nextPnt)->x ) / 2.0 ;
                               y = (pointAddrP(dtmP,lastPnt)->y + pointAddrP(dtmP,nextPnt)->y ) / 2.0 ;
                               z = (pointAddrP(dtmP,lastPnt)->z + pointAddrP(dtmP,nextPnt)->z ) / 2.0 ;
                               if( bcdtmInsert_addPointToDtmObject(dtmP,x,y,z,&np) ) goto errexit  ;
                               if( bcdtmList_deleteLineDtmObject(dtmP,lastPnt,nextPnt)) goto errexit  ;
                               if( ap != dtmP->nullPnt && cp != dtmP->nullPnt )
                                 { 
                                  if( bcdtmList_insertLineAfterPointDtmObject(dtmP,lastPnt,np,ap) ) goto errexit  ;   
                                  if( bcdtmList_insertLineAfterPointDtmObject(dtmP,np,lastPnt,dtmP->nullPnt) ) goto errexit  ;   
                                  if( bcdtmList_insertLineAfterPointDtmObject(dtmP,ap,np,nextPnt)) goto errexit  ; 
                                  if( bcdtmList_insertLineAfterPointDtmObject(dtmP,np,ap,lastPnt)) goto errexit  ; 
                                  if( bcdtmList_insertLineAfterPointDtmObject(dtmP,nextPnt,np,cp)) goto errexit  ; 
                                  if( bcdtmList_insertLineAfterPointDtmObject(dtmP,np,nextPnt,ap)) goto errexit  ; 
                                  if( bcdtmList_insertLineAfterPointDtmObject(dtmP,cp,np,lastPnt)) goto errexit  ; 
                                  if( bcdtmList_insertLineAfterPointDtmObject(dtmP,np,cp,nextPnt)) goto errexit  ; 
                                 }
                               if( ap != dtmP->nullPnt && cp == dtmP->nullPnt )
                                 { 
                                  if( bcdtmList_insertLineAfterPointDtmObject(dtmP,lastPnt,np,ap) ) goto errexit  ;   
                                  if( bcdtmList_insertLineAfterPointDtmObject(dtmP,np,lastPnt,dtmP->nullPnt) ) goto errexit  ;   
                                  if( bcdtmList_insertLineAfterPointDtmObject(dtmP,ap,np,nextPnt)) goto errexit  ; 
                                  if( bcdtmList_insertLineAfterPointDtmObject(dtmP,np,ap,lastPnt)) goto errexit  ; 
                                  if( bcdtmList_insertLineBeforePointDtmObject(dtmP,nextPnt,np,ap)) goto errexit  ; 
                                  if( bcdtmList_insertLineAfterPointDtmObject(dtmP,np,nextPnt,ap)) goto errexit  ; 
                                 }
                               if( ap == dtmP->nullPnt && cp != dtmP->nullPnt )
                                 { 
                                  if( bcdtmList_insertLineBeforePointDtmObject(dtmP,lastPnt,np,cp) ) goto errexit  ;   
                                  if( bcdtmList_insertLineAfterPointDtmObject(dtmP,np,lastPnt,dtmP->nullPnt) ) goto errexit  ;   
                                  if( bcdtmList_insertLineAfterPointDtmObject(dtmP,nextPnt,np,cp))   goto errexit  ; 
                                  if( bcdtmList_insertLineAfterPointDtmObject(dtmP,np,nextPnt,lastPnt)) goto errexit  ; 
                                  if( bcdtmList_insertLineAfterPointDtmObject(dtmP,cp,np,lastPnt)) goto errexit  ; 
                                  if( bcdtmList_insertLineAfterPointDtmObject(dtmP,np,cp,nextPnt)) goto errexit  ; 
                                 }
                               if( bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(dtmP,lastPnt,nextPnt,np)) goto errexit  ;                 
                               nodeAddrP(dtmP,np)->tPtr = Mark ;
                              }
                           }  
                        }
                     }
                   clPtr   = nodeAddrP(dtmP,nextPnt)->fPtr ;
                  }
                if( nextPnt == dtmP->nullPnt || nextPnt == firstPnt )  clPtr = dtmP->nullPtr ; 
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
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Fixing DTM Feature Concave Spans Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Fixing DTM Feature Concave Spans Error") ;
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
BENTLEYDTM_Private int bcdtmClip_placeLinearDtmFeatureSectionIntoTptrListDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature,long startPnt,long endPnt)
/*
**
** This Function Places A section Of The Dtm dtmFeature Into The Tptr List 
**
*/
{  
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long firstPnt,nextPnt,lastPnt,sectionStartPnt,clPtr ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Placing Linear Dtm dtmFeature Section Into Tptr List") ;
/*
** Test Start And End Points Are Unique
*/
 if( startPnt == endPnt )
   {
    bcdtmWrite_message(2,0,0,"Start Point In Dtm Feature List Is Equal To Last Point ") ;
    goto errexit ;
   } 
/*
** Initialise
*/
 firstPnt = ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ;
 sectionStartPnt = lastPnt = dtmP->nullPnt ;
/*
** Scan Dtm Feature List To Section Start Point
*/
 if( startPnt == firstPnt ) sectionStartPnt = firstPnt ;
 else
   {
    clPtr = nodeAddrP(dtmP,firstPnt)->fPtr ;
    while ( clPtr != dtmP->nullPtr )
      {
       while ( clPtr != dtmP->nullPtr  && flistAddrP(dtmP,clPtr)->dtmFeature != dtmFeature ) clPtr = flistAddrP(dtmP,clPtr)->nextPtr ;
       if( clPtr != dtmP->nullPtr )
         {
          nextPnt = flistAddrP(dtmP,clPtr)->nextPnt ;
          if( nextPnt != dtmP->nullPnt )
            { 
             if( nextPnt == startPnt ) { sectionStartPnt = nextPnt ; nextPnt = dtmP->nullPnt ; }
             else                      clPtr = nodeAddrP(dtmP,nextPnt)->fPtr ;
            } 
          if( nextPnt == dtmP->nullPnt || nextPnt == firstPnt ) clPtr = dtmP->nullPtr ;
         }
      }
/*
** Test Start Point Found In Dtm Feature
*/
    if( sectionStartPnt == dtmP->nullPnt )
      {
       bcdtmWrite_message(2,0,0,"Cannot Find Start Point In Dtm Feature List") ;
       goto errexit ;
      }    
   }
/*
** Scan From Start Point To End Point And Store In Tptr List
*/
 firstPnt = lastPnt = sectionStartPnt ;
 clPtr  = nodeAddrP(dtmP,firstPnt)->fPtr ;
 while ( clPtr != dtmP->nullPtr )
   {
    while ( clPtr != dtmP->nullPtr  && flistAddrP(dtmP,clPtr)->dtmFeature != dtmFeature ) clPtr = flistAddrP(dtmP,clPtr)->nextPtr ;
    if( clPtr != dtmP->nullPtr )
      {
       nextPnt = flistAddrP(dtmP,clPtr)->nextPnt ;
       nodeAddrP(dtmP,lastPnt)->tPtr = nextPnt ;
       lastPnt = nextPnt ;
       if( nextPnt != dtmP->nullPnt )
         { 
          clPtr = nodeAddrP(dtmP,nextPnt)->fPtr ;
          if( nextPnt == endPnt ) nodeAddrP(dtmP,nextPnt)->tPtr = dtmP->nullPnt ; 
         }
       if( nextPnt == dtmP->nullPnt || nextPnt == firstPnt || nextPnt == endPnt ) clPtr = dtmP->nullPtr ;
      }
   }
/*
** Cleanup
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Placing Linear Dtm dtmFeature Section Into Tptr List Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Placing Linear Dtm dtmFeature Section Into Tptr List Error") ;
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
BENTLEYDTM_Public int bcdtmClip_resolveAdjoiningPolygonalFeaturesDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Resolves Adjoining Polygonal Features
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  dtmFeature,numVoid=0,numIsland=0,numHole=0 ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Resolving Adjoining Polygonal Features") ;
/*
** Count Number Of Polygonal Features In Tin
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void   && dtmFeatureP->dtmFeaturePts.firstPoint != dtmP->nullPnt )  ++numVoid ;
    if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Island && dtmFeatureP->dtmFeaturePts.firstPoint != dtmP->nullPnt )  ++numIsland ;
    if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Hole   && dtmFeatureP->dtmFeaturePts.firstPoint != dtmP->nullPnt )  ++numHole ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"numVoids = %6ld numIslands = %6ld numHoles = %6ld",numVoid,numIsland,numHole) ;
/*
** Only Process If Polygonal Features Present
*/
 if( numVoid > 1 || numIsland != 0 || numHole != 0 ) 
   {
/*
**  Look For Adjoining Voids
*/
    if( numVoid > 1 )
      {
       for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
          if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
            {
             if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void )
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"Finding And Merging Adjoining Voids To Void Feature %6ld",dtmFeature) ;
                if( bcdtmClip_findAndMergeAdjoiningVoidsDtmObject(dtmP,dtmFeature)) goto errexit ;
               }
            }
         } 
      } 
/*
**  Compact Features
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Compacting Feature Table") ;
    if( bcdtmTin_compactFeatureTableDtmObject(dtmP)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Compacting Feature List") ;
    if( bcdtmTin_compactFeatureListDtmObject(dtmP) ) goto errexit ;
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
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR  ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int  bcdtmClip_checkTinHullsIntersectDtmObject(BC_DTM_OBJ *dtm1P,BC_DTM_OBJ *dtm2P,long *intersectResultP)
/*
** This Function Tests The Intersection Of dtm2P Hull With dtm1P Hull
**
** intersectResultP = 0  No Intersection
** intersectResultP = 1  Tin Hulls Intersect
** intersectResultP = 2  dtm2P Hull Totally Within dtm1P Hull
** intersectResultP = 3  dtm1P Hull Totally Within dtm2P HULL
**
*/
{
 int  ret=DTM_SUCCESS ;
 long numhullPts1P=0,numhullPts2P=0,intersectFlag ;
 DPoint3d  *hullPts1P=NULL,*hullPts2P=NULL ;
/*
** Initialise 
*/
 *intersectResultP = 0 ;
/*
** Extract Tin Hull From Tin Objects
*/
 if( bcdtmList_extractHullDtmObject(dtm1P,&hullPts1P,&numhullPts1P)) goto errexit ;
 if( bcdtmList_extractHullDtmObject(dtm2P,&hullPts2P,&numhullPts2P)) goto errexit ; 
/*
** Check Intersection
*/
 bcdtmClip_checkPolygonsIntersect(hullPts1P,numhullPts1P,hullPts2P,numhullPts2P,&intersectFlag) ;
 if     ( intersectFlag == 0 ) *intersectResultP = 0 ;
 else if( intersectFlag == 1 ) *intersectResultP = 2 ;
 else if( intersectFlag == 2 ) *intersectResultP = 3 ;
 else if( intersectFlag == 3 ) *intersectResultP = 1 ;
/*
** Cleanup
*/
 cleanup :
 if( hullPts1P != NULL ) free(hullPts1P) ;  
 if( hullPts2P != NULL ) free(hullPts2P) ;  
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
BENTLEYDTM_Private int bcdtmClip_copyTptrListToPointArrayDtmObject(BC_DTM_OBJ *dtmP,long startPnt,DPoint3d **tptrPtsPP,long *numTptrPtsP)
/*
** This Function Is A Specific Purpose Clipping Function
** And Copies A Tptr List To A DPoint3d Array With
** The Point Number Subsituted For The z Value
**
*/
{
 int  ret=DTM_SUCCESS ;
 long sp ;
 DPoint3d  *p3dP ;
/*
** Initialise
*/
 *numTptrPtsP = 0 ;
 if( *tptrPtsPP != NULL ) { free(*tptrPtsPP) ; *tptrPtsPP = NULL ; }
/*
** Validate
*/
 if( nodeAddrP(dtmP,startPnt)->tPtr == dtmP->nullPnt )
   {
    bcdtmWrite_message(2,0,0,"Start Point Has A Null Tptr Value") ;
    goto errexit ;
   }
/*
** Scan Tptr List And Count Number Of Points
*/
 sp = startPnt ;
 do
   {
    ++*numTptrPtsP ; 
    sp = nodeAddrP(dtmP,sp)->tPtr ;
   } while ( sp != startPnt && nodeAddrP(dtmP,sp)->tPtr != dtmP->nullPnt ) ;
/*
** Check For Closure
*/
 if( sp == startPnt ) ++*numTptrPtsP ;
/*
** Allocate memory For DPoint3d Array
*/ 
 *tptrPtsPP = ( DPoint3d * ) malloc( *numTptrPtsP * sizeof(DPoint3d)) ;
 if( *tptrPtsPP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
** Scan Tptr List And Copy Points To DPoint3d Array
*/
 p3dP = *tptrPtsPP ;
 sp = startPnt ;
 do
   { 
    p3dP->x = pointAddrP(dtmP,sp)->x  ;
    p3dP->y = pointAddrP(dtmP,sp)->y  ;
    p3dP->z = (double) sp  ;
    ++p3dP ;
    sp = nodeAddrP(dtmP,sp)->tPtr   ;
   } while ( sp != startPnt && nodeAddrP(dtmP,sp)->tPtr != dtmP->nullPnt ) ;
/*
** Check For Closure
*/
if( sp == startPnt )
  {
   p3dP->x = pointAddrP(dtmP,sp)->x  ;
   p3dP->y = pointAddrP(dtmP,sp)->y  ;
   p3dP->z = (double) sp  ;
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
BENTLEYDTM_Private int bcdtmClip_testForVoidOrHoleHullLineDtmObject(BC_DTM_OBJ *dtmP,long pnt1,long pnt2,long *featureP)
/*
** This Function Is A Specific Purpose Clipping Function Tests If The Line pnt1-pnt2 is A Void Or Hole Line
*/
{
 long flPtr ;
/*
** Initialise
*/
 *featureP = dtmP->nullPnt ;
/*
** Test For Void Hull Line From pnt1 To pnt2
*/
 flPtr = nodeAddrP(dtmP,pnt1)->fPtr ;
 while ( flPtr != dtmP->nullPtr )
   {
    if(flistAddrP(dtmP,flPtr)->nextPnt == pnt2 )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,flPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Void ||
           ftableAddrP(dtmP,flistAddrP(dtmP,flPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Hole    ) { *featureP = flistAddrP(dtmP,flPtr)->dtmFeature ; return(1) ; }
      }
    flPtr = flistAddrP(dtmP,flPtr)->nextPtr ;  
   } 
/*
** Test For Void Hull Line From pnt2 To pnt1
*/
 flPtr = nodeAddrP(dtmP,pnt2)->fPtr ;
 while ( flPtr != dtmP->nullPtr )
   {
    if(flistAddrP(dtmP,flPtr)->nextPnt == pnt1 )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,flPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Void ||
           ftableAddrP(dtmP,flistAddrP(dtmP,flPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Hole    ) { *featureP = flistAddrP(dtmP,flPtr)->dtmFeature ; return(1) ; }
      }
    flPtr = flistAddrP(dtmP,flPtr)->nextPtr ;  
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
BENTLEYDTM_Private int bcdtmClip_testForIslandHullLineDtmObject(BC_DTM_OBJ *dtmP,long pnt1,long pnt2,long *featureP)
/*
** This Function Is A Specific Purpose Clipping Function Tests If The Line pnt1-pnt2 is A Void Or Hole Line
*/
{
 long flPtr ;
/*
** Initialise
*/
 *featureP = dtmP->nullPnt ;
/*
** Test For Void Hull Line From pnt1 To pnt2
*/
 flPtr = nodeAddrP(dtmP,pnt1)->fPtr ;
 while ( flPtr != dtmP->nullPtr )
   {
    if(flistAddrP(dtmP,flPtr)->nextPnt == pnt2 )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,flPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Island ) { *featureP = flistAddrP(dtmP,flPtr)->dtmFeature ; return(1) ; }
      }
    flPtr = flistAddrP(dtmP,flPtr)->nextPtr ;  
   } 
/*
** Test For Void Hull Line From pnt2 To pnt1
*/
 flPtr = nodeAddrP(dtmP,pnt2)->fPtr ;
 while ( flPtr != dtmP->nullPtr )
   {
    if(flistAddrP(dtmP,flPtr)->nextPnt == pnt1 )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,flPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Island ) { *featureP = flistAddrP(dtmP,flPtr)->dtmFeature ; return(1) ; }
      }
    flPtr = flistAddrP(dtmP,flPtr)->nextPtr ;  
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
BENTLEYDTM_Private int bcdtmClip_testForPointOnVoidHullDtmObject(BC_DTM_OBJ *dtmP,long point)
/*
** This Function Tests If Point P1 Is On A Void Hull
*/
{
 long flPtr ;
/*
** Scan Feature List For Point
*/
 flPtr = nodeAddrP(dtmP,point)->fPtr ;
 while ( flPtr != dtmP->nullPtr )
   {
    if( ftableAddrP(dtmP,flistAddrP(dtmP,flPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Void ||
        ftableAddrP(dtmP,flistAddrP(dtmP,flPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Hole    )  return(1) ; 
    flPtr = flistAddrP(dtmP,flPtr)->nextPtr ;  
   } 
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
BENTLEYDTM_Private int bcdtmClip_externalCleanPolygonalFeaturesDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Cleans Polygonal dtmFeatures
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    sp,np,ssp,snp,lp,dtmFeature,feature,connectError=0 ;
 DTMDirection direction;
 double  area ;
 DTMUserTag userTag ;
 DTMFeatureId    userFeatureId ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Cleaning External Polygonal Dtm Features") ;
 if( dbg == 2 )
   {
    bcdtmWrite_message(0,0,0,"Void Features") ;
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       if( ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint != dtmP->nullPnt  && ftableAddrP(dtmP,dtmFeature)->dtmFeatureType == DTMFeatureType::Void )
         {
          bcdtmWrite_message(0,0,0,"dtmFeature = %6ld firstPnt = %6ld TYPE = %4ld userTag = %9ld",dtmFeature,ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint,ftableAddrP(dtmP,dtmFeature)->dtmFeatureType,ftableAddrP(dtmP,dtmFeature)->dtmUserTag) ;
         }
      }
    bcdtmWrite_message(0,0,0,"Island Features") ;
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       if( ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint != dtmP->nullPnt  && ftableAddrP(dtmP,dtmFeature)->dtmFeatureType == DTMFeatureType::Island )
         {
          bcdtmWrite_message(0,0,0,"dtmFeature = %6ld firstPnt = %6ld TYPE = %4ld userTag = %9ld",dtmFeature,ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint,ftableAddrP(dtmP,dtmFeature)->dtmFeatureType,ftableAddrP(dtmP,dtmFeature)->dtmUserTag) ;
         }
      }
   } 
/*
**  Clear Tin Hull Markers
*/
 sp = dtmP->hullPoint ;
 do
   { 
    nodeAddrP(dtmP,sp)->PRGN = 0 ; 
    sp = nodeAddrP(dtmP,sp)->hPtr ;
   } while ( sp != dtmP->hullPoint ) ;
/*
**  Scan Tin Hull Looking For Voids Or Holes On Tin Hull
*/
 sp = dtmP->hullPoint ;
 do
   {
    np = nodeAddrP(dtmP,sp)->hPtr ;
    if( bcdtmClip_testForVoidOrHoleHullLineDtmObject(dtmP,sp,np,&dtmFeature) && ! nodeAddrP(dtmP,sp)->PRGN && ! nodeAddrP(dtmP,np)->PRGN && ! bcdtmClip_testForIslandHullLineDtmObject(dtmP,sp,np,&feature) )
      {
       if( dbg ) 
         {
          bcdtmWrite_message(0,0,0,"Void dtmFeature %6ld Fpnt = %6ld On Tin Hull %6ld %6ld",dtmFeature,ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint,sp,np) ;
          bcdtmWrite_message(0,0,0,"Sp = %6ld Fptr = %6ld ** %10.4lf %10.4lf %10.4lf",sp,nodeAddrP(dtmP,sp)->hPtr,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z) ;
          bcdtmWrite_message(0,0,0,"Np = %6ld Fptr = %6ld ** %10.4lf %10.4lf %10.4lf",np,nodeAddrP(dtmP,np)->hPtr,pointAddrP(dtmP,np)->x,pointAddrP(dtmP,np)->y,pointAddrP(dtmP,np)->z) ;
         } 
       connectError = 0 ;  
       ssp = sp ; snp = np ;
/*
**     Get Void dtmFeature Tag
*/
       userTag  = ftableAddrP(dtmP,dtmFeature)->dtmUserTag ;
       userFeatureId = ftableAddrP(dtmP,dtmFeature)->dtmFeatureId    ;
/*
**     Scan Anti-Clockwise Around Inside Of Hull back To Start Point
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Anti Clockwise Around Inside") ; 
       nodeAddrP(dtmP,sp)->tPtr = np ; 
        do
          {   
           if(( lp = bcdtmList_nextClkDtmObject(dtmP,np,sp)) < 0 ) goto errexit ;
           while ( ! bcdtmClip_testForVoidOrHoleHullLineDtmObject(dtmP,np,lp,&feature) &&
                   ! bcdtmClip_testForIslandHullLineDtmObject(dtmP,np,lp,&feature)         )
             {
              if(( lp = bcdtmList_nextClkDtmObject(dtmP,np,lp)) < 0 ) goto errexit ;
             }
           sp = np ; np = lp ;
           nodeAddrP(dtmP,sp)->tPtr = np ; 
          } while ( sp != ssp ) ;
/*
**     Check Connectivity Of Inserted Tptr Polygon
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"Checking Connectivity Of Void Polygon") ;
       if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,ssp,0)) 
         {
          if( dbg == 2 ) bcdtmList_writeTptrListDtmObject(dtmP,ssp) ; 
          connectError = 1 ;
         } 
/*
**     Insert New Feature If No Connect Error  
*/
       if( ! connectError )
         {  
          if( dbg )
            {  
             if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,ssp,&area,&direction) ) goto errexit ;
             bcdtmWrite_message(0,0,0,"Void Polygon area = %15.4lf direction = %2ld",area,direction) ;
            } 
/*
**        Mark Hull Points
*/
          sp = ssp ;
          do 
            {
             nodeAddrP(dtmP,sp)->PRGN = 1  ;
             sp = nodeAddrP(dtmP,sp)->tPtr ;
            } while ( sp != ssp ) ; 
/*
**        Insert Void In Tin
*/
          if( dbg ) bcdtmWrite_message(0,0,0,"Inserting New Feature") ;
          if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,DTMFeatureType::Void,userTag,userFeatureId,ssp,1)) goto errexit ; 
         }   
/*
**     Mark Origonal Void dtmFeature For Deletion
*/
       if( ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint >= 0 ) ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint = - (ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint + 1 ) ;
/*
**     Reset 
*/
       sp = ssp ;
       np = snp ;
      } 
    sp = np ;
   } while ( sp != dtmP->hullPoint ) ;
/*
** Delete Void Features
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    if( ftableAddrP(dtmP,dtmFeature)->dtmFeatureState == DTMFeatureState::Tin )
      { 
        if( ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint != dtmP->nullPnt )
          {
           if( ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint < 0 )
             {
              ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint = -ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint - 1 ;
              if( dbg ) bcdtmWrite_message(0,0,0,"Deleting dtmFeature = %6ld Fpnt = %6ld",dtmFeature,ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ) ;
              if( bcdtmInsert_removeDtmTinFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ;
             }
          }
      }  
   }
/*
** Clear Hull Markers
*/
 sp = dtmP->hullPoint ;
 do { nodeAddrP(dtmP,sp)->PRGN = 0 ; sp = nodeAddrP(dtmP,sp)->hPtr ; } while ( sp != dtmP->hullPoint ) ;
/*
** Look For Void Bits On Tin Hull Without An Associated Void
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Looking For Void Points On Tin Hull") ;
 sp = dtmP->hullPoint ;
 do 
   { 
    if( ! bcdtmClip_testForPointOnVoidHullDtmObject(dtmP,sp) )
      {
       if( bcdtmFlag_testVoidBitPCWD(&nodeAddrP(dtmP,sp)->PCWD) ) 
         {
          ssp = sp ;
          np  = nodeAddrP(dtmP,sp)->hPtr ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Void Hull Sp Point %6ld ** Fptr = %9ld %10.4lf %10.4lf %10.4lf",sp,nodeAddrP(dtmP,sp)->hPtr,pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z) ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Void Hull Np Point %6ld ** Fptr = %9ld %10.4lf %10.4lf %10.4lf",np,nodeAddrP(dtmP,np)->hPtr,pointAddrP(dtmP,np)->x,pointAddrP(dtmP,np)->y,pointAddrP(dtmP,np)->z) ;
/*
**        Scan Anti Clockwise And Extract Void
*/
          nodeAddrP(dtmP,sp)->tPtr = np ; 
          do
            {   
             if(( lp = bcdtmList_nextClkDtmObject(dtmP,np,sp)) < 0 ) goto errexit ;
             while ( nodeAddrP(dtmP,np)->hPtr != lp && ! bcdtmClip_testForIslandHullLineDtmObject(dtmP,lp,np,&feature) )
               {
                if( (lp = bcdtmList_nextClkDtmObject(dtmP,np,lp)) < 0 ) goto errexit ;
               }
             if( dbg ) bcdtmWrite_message(0,0,0,"Lp Point %6ld ** Fptr = %9ld %10.4lf %10.4lf %10.4lf",lp,nodeAddrP(dtmP,lp)->hPtr,pointAddrP(dtmP,lp)->x,pointAddrP(dtmP,lp)->y,pointAddrP(dtmP,lp)->z) ;
             sp = np ; np = lp ;
             nodeAddrP(dtmP,sp)->tPtr = np ; 
            } while ( sp != ssp ) ;
/*
**        Check Connectivity Of Inserted Tptr Polygon
*/
          if( dbg )
            {
             bcdtmWrite_message(0,0,0,"Checking Connectivity Of Void Polygon") ;
             if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,ssp,0)) 
               {
                bcdtmList_writeTptrListDtmObject(dtmP,ssp) ; 
                goto errexit ;  
               }
             if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,ssp,&area,&direction) ) goto errexit ;
             bcdtmWrite_message(0,0,0,"Void Polygon area = %15.4lf direction = %2ld",area,direction) ;
            } 
/*
**        Insert Void In Tin
*/
          if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,DTMFeatureType::Void,dtmP->nullUserTag,dtmP->nullFeatureId,ssp,1)) goto errexit ; 
/*
**        Reset
*/       
          sp = ssp ;
         }
      }
    sp = nodeAddrP(dtmP,sp)->hPtr ;
   } while ( sp != dtmP->hullPoint ) ;
/*
**  Scan Tin Hull Looking For Islands
*/
 sp = dtmP->hullPoint ;
 do
   {
    np = nodeAddrP(dtmP,sp)->hPtr ;
    if( bcdtmClip_testForIslandHullLineDtmObject(dtmP,sp,np,&dtmFeature) )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Island On Tin Hull %6ld %6ld",sp,np) ;
/*
**     Delete Island Feature From Tin
*/
       if( bcdtmInsert_removeDtmTinFeatureFromDtmObject(dtmP,dtmFeature)) goto errexit ;
      } 
    sp = np ;
   } while ( sp != dtmP->hullPoint ) ;
/*
** Compact Feature Arrays
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Compacting dtmFeature Table") ;
// if( bcdtmTin_compactFeatureTableDtmObject(dtmP)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Compacting dtmFeature List") ;
// if( bcdtmTin_compactFeatureListDtmObject(dtmP) ) goto errexit ;
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
BENTLEYDTM_Public int bcdtmClip_fillTptrPolygonWithTrianglesDtmObject(BC_DTM_OBJ *dtmP,long point)
/*
** This Function Fills A Tptr Polygon With Triangles
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long  cp,pp,sp,np,rsp,rcp,rnp,numPoints,numTinErrors ;
 DPoint3d   *pointsP=NULL ;
 BC_DTM_OBJ  *tempDtmP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Filling Tptr Polygon With Triangles") ;
/*
** Check Line Connections
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Reverse Line Connections") ;
    if( bcdtmClip_checkReverseLineConnectionsDtmObject(dtmP)) goto errexit ;
   }
/*
** Write Out Tptr Polygon
*/
 if( dbg ) bcdtmList_writeTptrListDtmObject(dtmP,point) ;
/*
** Copy Tptr Polygon To Point Array
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying Tptr Polygon To Point Array") ;
 if( bcdtmClip_copyTptrListToPointArrayDtmObject(dtmP,point,&pointsP,&numPoints)) goto errexit  ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Tptr Polygon Points = %8ld",numPoints) ;
/*
** Create Dtm Object
*/
 if( bcdtmObject_createDtmObject(&tempDtmP)) goto errexit ;
/*
** Set Memory Allocation Parameters For tempDtmP Object
*/
 if( bcdtmObject_setPointMemoryAllocationParametersDtmObject(tempDtmP,numPoints,numPoints)) goto errexit ;
/*
**  Store Polygon In tempDtmP Object As Break Lines
*/
 if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::Breakline,tempDtmP->nullUserTag,1,&tempDtmP->nullFeatureId,pointsP,numPoints)) goto errexit ;
/*
** Triangulate tempDtmP Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating Temporary Dtm Object") ;
 DTM_NORMALISE_OPTION  = FALSE ;             // To Inhibit Normalisation Of Coordinates - function 
 DTM_DUPLICATE_OPTION = FALSE ;             // To Inhibit Removal Of None Identical Points
 tempDtmP->ppTol = tempDtmP->plTol = 0.0 ;  
 if( bcdtmObject_createTinDtmObject(tempDtmP,1,0.0)) goto errexit ;
 DTM_NORMALISE_OPTION  = TRUE ;
 DTM_DUPLICATE_OPTION = TRUE ;
/*
** Check For Triangulation Errors
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking For Tin Feature Errors") ; 
    if( bcdtmCheck_forTinFeatureErrorsDtmObject(tempDtmP,1,&numTinErrors)) goto errexit ;
    bcdtmWrite_message(0,0,0,"Number Of Tin Feature Errors = %6ld",numTinErrors) ; 
   }
/*
** Remove None Feature Hull Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing None Feature Tin Hull Lines") ;
 if( bcdtmList_removeNoneFeatureHullLinesDtmObject(tempDtmP)) goto errexit ;
/*
** Check Tin
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Fill DTM %p",tempDtmP) ;
/*
**   Check Topology
*/
    bcdtmWrite_message(0,0,0,"Checking Topology") ;
    if( bcdtmCheck_topologyDtmObject(tempDtmP,1))
      { 
       bcdtmWrite_message(1,0,0,"Fill DTM Topology Corrupted") ;
       goto errexit ; 
      }
    bcdtmWrite_message(0,0,0,"Fill DTM Topology Valid") ;
/*
**  Check Precision
*/
    bcdtmWrite_message(0,0,0,"Checking Precision") ;
    if( bcdtmCheck_precisionDtmObject(tempDtmP,1)) 
      { 
       bcdtmWrite_message(1,0,0,"Fill DTM Precision Corrupted") ;
       goto errexit ;
      }
    bcdtmWrite_message(0,0,0,"Fill DTM Precision OK") ;
/*
**  Check Consistency Of Related Points
*/
    bcdtmWrite_message(0,0,0,"Checking Consistency Of Related Points") ;
    sp  = tempDtmP->hullPoint ;
    do
      {
       np  = nodeAddrP(tempDtmP,sp)->hPtr ;
       rsp = (long)pointAddrP(tempDtmP,sp)->z ;
       rnp = (long)pointAddrP(tempDtmP,np)->z ;
       if( nodeAddrP(dtmP,rsp)->tPtr != rnp )
         {
          bcdtmWrite_message(0,0,0,"Inconsistency In Related Points ** sp = %8ld np = %8ld ** rsp = %8ld rnp = %8ld",sp,np,rsp,rnp) ;
          bcdtmWrite_message(1,0,0,"Related Points Error") ;
          goto errexit ;
         }  
       sp = np ;
      } while( sp != tempDtmP->hullPoint ) ;
    bcdtmWrite_message(0,0,0,"Checking Consistency Of Related Points OK") ;
   }
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
** Check Line Connections
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Reverse Line Connections") ;
    if( bcdtmClip_checkReverseLineConnectionsDtmObject(dtmP)) goto errexit ;
   }
/*
** Clean Up
*/
 cleanup :
 DTM_NORMALISE_OPTION  = TRUE ;
 DTM_DUPLICATE_OPTION = TRUE ;
 if( pointsP  != NULL ) free(pointsP) ;
 if( tempDtmP != NULL ) bcdtmObject_destroyDtmObject(&tempDtmP)  ;  
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Filling Tptr Polygon With Triangles Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Filling Tptr Polygon With Triangles Error") ;
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
BENTLEYDTM_Private int bcdtmClip_checkReverseLineConnectionsDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Fills A Tptr Polygon With Triangles
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  point,clPnt,clPtr ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Reverse Line Connections") ;
/*
** Scan Points
*/
 for( point = 0 ; point < dtmP->numPoints ; ++point )
   {
    if( ( clPtr = nodeAddrP(dtmP,point)->cPtr ) != dtmP->nullPtr )
      {
       while( clPtr != dtmP->nullPtr )
         {
          clPnt = clistAddrP(dtmP,clPtr)->pntNum ;
          clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
          if( ! bcdtmList_testLineDtmObject(dtmP,clPnt,point)) 
            {
             bcdtmWrite_message(0,0,0,"No Reverse Connection For Tin Line %8ld %8ld",point,clPnt) ; 
             ret = DTM_ERROR ;
            }
         }
      }
   }
/*
** If Errors Detected Error Exit
*/
 if( ret == DTM_ERROR ) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Reverse Line Connections Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Reverse Line Connections Error") ;
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
BENTLEYDTM_EXPORT int bcdtmClip_buildClippingTinFromFencePointsDtmObject(BC_DTM_OBJ **dtmPP,DPoint3dCP fencePtsP,long numFencePts)
/*
** This Function Builds A Clipping Tin From Fence Points
*/
{
 int         ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long        edgeOption=1 ;
 double      maxSide=0.0 ;
 DPoint3dCP  p3dP ;
 DTMFeatureId dtmFeatureId=0 ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Building Clipping Tin") ;
    bcdtmWrite_message(0,0,0,"dtmPP           = %p",*dtmPP) ;
    bcdtmWrite_message(0,0,0,"fencePtsP       = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts     = %8ld",numFencePts) ;
    if( fencePtsP != NULL && numFencePts > 0 )
      {
       for( p3dP = fencePtsP ; p3dP < fencePtsP + numFencePts ; ++p3dP )
         {
          bcdtmWrite_message(0,0,0,"fencePts[%4ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-fencePtsP),p3dP->x,p3dP->y,p3dP->z) ;
         } 
      } 
   }
/*
** Validate Point Array
*/
 if( fencePtsP == NULL || numFencePts < 3 )
   {
    bcdtmWrite_message(2,0,0,"Invalid Clipping Fence") ;
    goto errexit ;
   }
/*
** Delete Clip Tin Object If It Exists
*/
 if( *dtmPP != NULL ) bcdtmObject_destroyDtmObject(dtmPP) ;
/*
** Create Dtm Object For Clipping Feature
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Dtm Object") ;
 if( bcdtmObject_createDtmObject(dtmPP)) goto errexit ;
 bcdtmObject_setPointMemoryAllocationParametersDtmObject(*dtmPP,numFencePts*2,numFencePts) ;
/*
** Store Fence Points As Break Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Storing Fence Points ** dtmFeatureId = %I64d",dtmFeatureId) ;
 if( bcdtmObject_storeDtmFeatureInDtmObject(*dtmPP,DTMFeatureType::Breakline,DTM_NULL_USER_TAG,1,&dtmFeatureId,fencePtsP,numFencePts)) goto errexit ;
/*
** Triangulate Data Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating") ;
 if( bcdtmObject_createTinDtmObject(*dtmPP,edgeOption,maxSide))
/*
** Remove None Feature Hull Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing None Feature hull Lines") ;
 if( bcdtmList_removeNoneFeatureHullLinesDtmObject(*dtmPP)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Building Clipping Tin Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Building Clipping Tin Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 if( *dtmPP != NULL ) bcdtmObject_destroyDtmObject(dtmPP) ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmClip_featurePointArrayToTinHullDtmObject
(
 BC_DTM_OBJ *dtmP,
 DTMFenceOption clipOption,
 DPoint3d  *featPtsP,
 long numFeatPts,
 long *clipResultP,                  // <  0  Clip = None , 1 Clip = Feature Pts , 2 Clip = Clip Arrays  > 
 DTM_POINT_ARRAY ***clipArraysPPP,
 long *numClipArraysP
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long   num, numClipPts, numDrapePts = 0, numPtsOnDrape;
 DTMFenceOption featureExtent;
 long   memPointArrays=0,memPointArraysInc=10 ;
 double xMin,yMin,xMax,yMax ;
 DPoint3d    *ptsP,*clipPtsP=NULL,*p3dP,*p3d1P,*p3d2P ;
 DTM_DRAPE_POINT *drapeP,*drape1P,*drape2P=NULL,*drapePtsP=NULL ;
 long startTime = bcdtmClock() ;

/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Clipping Feature To Tin Hull") ;
/*
** Initialise
*/
 *clipResultP = 0 ;
/*
** Validate Point Array Arguments
*/
 *numClipArraysP = 0 ;
 if( *clipArraysPPP != NULL ) 
   {
    bcdtmWrite_message(2,0,0,"Pointer Array Is Not Null") ;
    goto errexit ;
   }
/*
** Check Clipping Tin
*/
 if( dtmP == NULL )
   {
    bcdtmWrite_message(2,0,0,"Null Clipping Tin") ;
    goto errexit ;
   }
/*
** Get Bounding Rectangle For Feature
*/
 xMin = xMax = featPtsP->x ;
 yMin = yMax = featPtsP->y ;
 for( ptsP = featPtsP + 1 ; ptsP < featPtsP + numFeatPts ; ++ptsP )
   {
    if( ptsP->x < xMin ) xMin = ptsP->x ;
    if( ptsP->x > xMax ) xMax = ptsP->x ;
    if( ptsP->y < yMin ) yMin = ptsP->y ;
    if( ptsP->y > yMax ) yMax = ptsP->y ;
   }
/*
** Write Stats On Feature Extent
*/
 if( dbg == 1 )
   {
    featureExtent = DTMFenceOption::Outside ;
    if( xMax >= dtmP->xMin && xMin <= dtmP->xMax && yMax >= dtmP->yMin && yMin <= dtmP->yMax ) featureExtent = DTMFenceOption::Overlap ;
    if( featureExtent == DTMFenceOption::Outside ) bcdtmWrite_message(0,0,0,"Feature External To Clip") ;
    else                               bcdtmWrite_message(0,0,0,"Feature Overlaps Clip") ;
   }
/*
** Determine Feature Extent In Relation To Tin Hull
*/
 featureExtent = DTMFenceOption::Outside ;
 if( xMax >= dtmP->xMin && xMin <= dtmP->xMax && yMax >= dtmP->yMin && yMin <= dtmP->yMax )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Draping Feature On Clip Tin") ;
/*
**  Drape Feature Points On Clipping Tin
*/
    if( bcdtmDrape_stringDtmObject(dtmP,featPtsP,numFeatPts,FALSE,&drapePtsP,&numDrapePts)) goto errexit ;
/*
**  Write Drape Points
*/
    if( dbg == 1 )
      {
       bcdtmWrite_message(0,0,0,"Number Of Drape Points = %6ld",numDrapePts) ;
       for( drapeP = drapePtsP ; drapeP < drapePtsP + numDrapePts ; ++drapeP )  
         {
          bcdtmWrite_message(0,0,0,"Drape Point[%6ld]  L = %4ld T = %2ld ** %10.4lf %10.4lf",(long)(drapeP-drapePtsP),drapeP->drapeLine,drapeP->drapeType,drapeP->drapeX,drapeP->drapeY ) ;
         }
      } 
/*
**  Determine Feature Extent
*/
    numPtsOnDrape = 0 ; 
    for( drapeP = drapePtsP ; drapeP < drapePtsP + numDrapePts ; ++drapeP )  
      {
       if( drapeP->drapeType != DTMDrapedLineCode::External ) ++numPtsOnDrape ;
      }
    if     ( numPtsOnDrape == numDrapePts ) featureExtent = DTMFenceOption::Inside  ;
    else if( numPtsOnDrape == 0           ) featureExtent = DTMFenceOption::Outside ;
    else                                    featureExtent = DTMFenceOption::Overlap ;
   }
/*
** Write Feature Extent
*/
 if( dbg ) if( featureExtent == DTMFenceOption::Overlap ) bcdtmWrite_message(0,0,0,"featureExtent = %2ld ** clipOption = %2ld",featureExtent,clipOption) ;
/*
** If Feature Extent Satifies Clip Option 
*/
 if ((clipOption == DTMFenceOption::Inside  && featureExtent == DTMFenceOption::Inside) ||
     (clipOption == DTMFenceOption::Outside && featureExtent == DTMFenceOption::Outside) ||
     (clipOption == DTMFenceOption::Overlap && featureExtent == DTMFenceOption::Overlap) ||
     (clipOption == DTMFenceOption::Overlap && featureExtent == DTMFenceOption::Inside)) *clipResultP = 1;
/*
**  Overlapping Featuring Clip Inside Or OutSide Sections
*/
 else if (featureExtent == DTMFenceOption::Overlap && (clipOption == DTMFenceOption::Inside || clipOption == DTMFenceOption::Outside))
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Feature Overlaps Fence") ;
/*
**  Write Feature Points
*/
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"Number Of Feature Points = %6ld",numFeatPts) ;
       for( ptsP = featPtsP ; ptsP < featPtsP + numFeatPts ; ++ptsP )
         {
          bcdtmWrite_message(0,0,0,"Feature Point[%6ld]  ** %12.5lf %12.5lf",(long)(ptsP-featPtsP),ptsP->x,ptsP->y,ptsP->z ) ;
         } 
      }
/*
**  Write Drape Points
*/
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"Number Of Drape Points = %6ld",numDrapePts) ;
       for( drapeP = drapePtsP ; drapeP < drapePtsP + numDrapePts ; ++drapeP )  
         {
          bcdtmWrite_message(0,0,0,"Drape Point[%6ld]  L = %4ld T = %2ld ** %10.4lf %10.4lf",(long)(drapeP-drapePtsP),drapeP->drapeLine-1,drapeP->drapeType,drapeP->drapeX,drapeP->drapeY ) ;
         }
      } 
/*
**  Set Clip Result To Arrays
*/ 
    *clipResultP = 2  ; 
/*
**  Scan Drape Line Points And Write Clipped Feature Sections To Point Arrays
*/
    drape1P = drapePtsP ;
    while ( drape1P < drapePtsP + numDrapePts )
      {
/*
**     Get First And Last Drape Point For Inside Segment
*/
       if( clipOption == DTMFenceOption::Inside )
         {
          while (  drape1P < drapePtsP + numDrapePts && drape1P->drapeType == DTMDrapedLineCode::External) ++drape1P ;
          if( drape1P >= drapePtsP + numDrapePts ) --drape1P ;       
          drape2P = drape1P + 1 ;  
          while (drape2P < drapePtsP + numDrapePts &&   drape2P->drapeType != DTMDrapedLineCode::External) ++drape2P;
          --drape2P ;   
         }
/*
**     Get First And Last Drape Point For Outside Segment
*/
       if( clipOption == DTMFenceOption::Outside )
         {
         while (drape1P < drapePtsP + numDrapePts &&  drape1P->drapeType != DTMDrapedLineCode::External) ++drape1P;
          if( drape1P >= drapePtsP + numDrapePts ) --drape1P ;       
          else if( drape1P > drapePtsP ) --drape1P ;
          drape2P = drape1P + 1 ;  
          while (drape2P < drapePtsP + numDrapePts &&  drape2P->drapeType == DTMDrapedLineCode::External) ++drape2P;
          if( drape2P >= drapePtsP + numDrapePts ) --drape2P ;       
         }
       if( dbg )
         {
          bcdtmWrite_message(0,0,0,"drape1P = %p ** ofs = %8ld line = %8ld",drape1P,(long)(drape1P-drapePtsP),drape1P->drapeLine) ;
          bcdtmWrite_message(0,0,0,"drape2P = %p ** ofs = %8ld line = %8ld",drape2P,(long)(drape2P-drapePtsP),drape2P->drapeLine) ;
         }
/*
**     Get Offsets To Clipped Feature Sections
*/
       if( drape1P < drape2P )
         {
/*
**       Allocate Memory For Clip Points
*/
         numClipPts = drape2P->drapeLine - drape1P->drapeLine + 2 ;
         if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Clip Points = %8ld",numClipPts) ;
         clipPtsP  = ( DPoint3d * ) malloc( numClipPts * sizeof(DPoint3d)) ;
         if( clipPtsP == NULL )
           {
            bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
            goto errexit ;
           } 
/*
**       Copy Feature Points To Clip Points
*/
         ptsP  = clipPtsP ;
         p3d1P = featPtsP + drape1P->drapeLine - 1 ; 
         p3d2P = featPtsP + drape2P->drapeLine - 1 ;
/*
**       Copy First Feature Point
*/  
         if( drape1P != drapePtsP )
           {  
            bcdtmMath_interpolatePointOnLine(p3d1P->x,p3d1P->y,p3d1P->z,(p3d1P+1)->x,(p3d1P+1)->y,(p3d1P+1)->z,drape1P->drapeX,drape1P->drapeY,&drape1P->drapeZ) ;
            ptsP->x = drape1P->drapeX ;
            ptsP->y = drape1P->drapeY ;
            ptsP->z = drape1P->drapeZ ;
           }
         else
           {
            ptsP->x = featPtsP->x ;
            ptsP->y = featPtsP->y ;
            ptsP->z = featPtsP->z ;
           }  
         ++ptsP ;
/*
**       Copy Intermediate Points
*/  
         for( p3dP = p3d1P + 1 ; p3dP <= p3d2P ; ++p3dP )
           {
            ptsP->x = p3dP->x ;
            ptsP->y = p3dP->y ;
            ptsP->z = p3dP->z ;
            ++ptsP ;
           } 
/*
**       Copy Last Feature Point
*/  
         if( drape2P != drapePtsP + numDrapePts - 1 )
           { 
            bcdtmMath_interpolatePointOnLine(p3d2P->x,p3d2P->y,p3d2P->z,(p3d2P+1)->x,(p3d2P+1)->y,(p3d2P+1)->z,drape2P->drapeX,drape2P->drapeY,&drape2P->drapeZ) ;
            ptsP->x = drape2P->drapeX ;
            ptsP->y = drape2P->drapeY ;
            ptsP->z = drape2P->drapeZ ;
           }
         else
           {
            ptsP->x = (featPtsP+numFeatPts-1)->x ;
            ptsP->y = (featPtsP+numFeatPts-1)->y ;
            ptsP->z = (featPtsP+numFeatPts-1)->z ;
           } 
         ++ptsP ;
/*
**       Check Number Of Points Copied Is Same As Number Of Clip Points
*/
         if( (long)(ptsP-clipPtsP) != numClipPts )
           {
            bcdtmWrite_message(2,0,0,"numClipPts = %4ld ** number Copied = %4ld",numClipPts,(long)(ptsP-clipPtsP)) ;
            goto errexit ;
           }
/*
**       Allocate Memory For Pointer Array 
*/
          if( *numClipArraysP == memPointArrays )
            {
             if( bcdtmMem_allocatePointerArrayToPointArrayMemory(clipArraysPPP,*numClipArraysP,&memPointArrays,memPointArraysInc)) goto errexit ;
            }
/*
**        Store Clip Points In Pointer Array
*/
          if( bcdtmMem_storePointsInPointerArray(*clipArraysPPP,*numClipArraysP,&clipPtsP,numClipPts)) goto errexit ;
          ++*numClipArraysP ;
         }
/*
**        Set Drape 1 Pointer
*/ 
        drape1P = drape2P + 1 ;
      }
   }
/*
** Realloc Pointer Array To Point Arrays
*/
 if( *numClipArraysP < memPointArrays )
   {
    *clipArraysPPP = ( DTM_POINT_ARRAY ** ) realloc ( *clipArraysPPP, *numClipArraysP * sizeof( DTM_POINT_ARRAY * )) ;
   }
/*
** Write Out Clipped Point Arrays
*/
 if( dbg && featureExtent == DTMFenceOption::Overlap && ( clipOption == DTMFenceOption::Inside || clipOption == DTMFenceOption::Outside ) )
   {
    bcdtmWrite_message(0,0,0,"Number Of Clipped Point Arrays = %6ld",*numClipArraysP) ;
    for( num = 0 ; num < *numClipArraysP ; ++num )
      {
       bcdtmWrite_message(0,0,0,"Point Array[%4ld] = %p %p ** %p %4ld",num,(*clipArraysPPP+num),*(*clipArraysPPP+num),(*(*clipArraysPPP+num))->pointsP,(*(*clipArraysPPP+num))->numPoints) ; 
      }
   } 
/*
** Clean Up
*/
 cleanup :
 if( clipPtsP  != NULL ) free(clipPtsP) ;
 if( drapePtsP != NULL ) bcdtmDrape_freeDrapePointMemory(&drapePtsP,&numDrapePts) ;
/*
** Job Completed
*/
 if( tdbg ) bcdtmWrite_message(0,0,0,"ClipResult = %2ld ** Clip Time  = %7.4lf Seconds",*clipResultP,bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Clipping Feature To Tin Hull Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Clipping Feature To Tin Hull Error") ;
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
BENTLEYDTM_Public int bcdtmClip_determineFeatureExtentWithFenceDtmObject
(
 BC_DTM_OBJ *tinP,                   /* ==> Pointer To Tin Object            */
 DPoint3d *featurePtsP,                   /* ==> Pointer To Feature Points        */
 long numFeaturePts,                 /* ==> Number Of Feature Points         */
 DTMFenceOption *featureExtentP                /* <== Feature Extent                   */
 )
/*
** This Function Determines A Features Extent In Relation To A Clipping Tin Hull
**
** Do Not Use Tins With Voids Or Islands
**
** Possible Extent Values   =  DTMFenceOption::Inside   If Feature Is Totally Internal To Hull
**                          =  DTMFenceOption::Overlap  If Feature Overlaps Hull
**                          =  DTMFenceOption::Outside  If Feature Is Totally External To Hull   
**
*/
{
 int             ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long            trgPnt1,trgPnt2,trgPnt3,pntFnd,numDrapePts,numPtsOnDrape ;
 double          xMin,xMax,yMin,yMax ; 
 DPoint3d             *p3dP ;
 DTM_DRAPE_POINT *drapeP,*drapePtsP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Determining Feature Extent") ;
/*
** Initialise
*/
 *featureExtentP = DTMFenceOption::Inside ;
/*
** Only  One Feature Point
*/
 if( numFeaturePts == 1 )
   {
/*
**  Check Bounding Rectangle
*/
    if( featurePtsP->x < tinP->xMin || featurePtsP->x > tinP->xMax  || 
        featurePtsP->y < tinP->yMin || featurePtsP->y > tinP->yMax     ) *featureExtentP = DTMFenceOption::Outside ;
/*
**  Check If Point Is Internal To Tin
*/
    else 
      {
       if( bcdtmFind_triangleDtmObject(tinP,featurePtsP->x,featurePtsP->y,&pntFnd,&trgPnt1,&trgPnt2,&trgPnt3) ) goto errexit ;
       if( ! pntFnd ) *featureExtentP = DTMFenceOption::Outside ;
      }
   }
/*
**  More Than One Feature Point
*/
 if( numFeaturePts > 1 )
   {
/*
** Get Bounding Rectangle For Feature
*/
    xMin = xMax = featurePtsP->x ;
    yMin = yMax = featurePtsP->y ;
    for( p3dP = featurePtsP + 1 ; p3dP < featurePtsP + numFeaturePts ; ++p3dP )
      {
       if( p3dP->x < xMin ) xMin = p3dP->x ;
       if( p3dP->x > xMax ) xMax = p3dP->x ;
       if( p3dP->y < yMin ) yMin = p3dP->y ;
       if( p3dP->y > yMax ) yMax = p3dP->y ;
      }
/*
**  Check Bounding Rectangle
*/
    if( xMax < tinP->xMin || xMin > tinP->xMax  || 
        yMax < tinP->yMin || yMin > tinP->yMax     ) *featureExtentP = DTMFenceOption::Outside ;
/*
**  Drape Feature On Tin
*/
    else
      {
       if( bcdtmDrape_stringDtmObject(tinP,featurePtsP,numFeaturePts,FALSE,&drapePtsP,&numDrapePts)) goto errexit ;
       numPtsOnDrape = 0 ; 
       for( drapeP = drapePtsP ; drapeP < drapePtsP + numDrapePts ; ++drapeP )  
         {
         if (drapeP->drapeType != DTMDrapedLineCode::External) ++numPtsOnDrape;
         }
       if     ( numPtsOnDrape == numDrapePts ) *featureExtentP = DTMFenceOption::Inside  ;
       else if( numPtsOnDrape == 0           ) *featureExtentP = DTMFenceOption::Outside ;
       else                                    *featureExtentP = DTMFenceOption::Overlap ;
      } 
   }
/*
** Clean Up
*/
 cleanup :
 if( drapePtsP != NULL )  bcdtmDrape_freeDrapePointMemory(&drapePtsP,&numDrapePts) ;
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Determining Feature Extent Completed") ;
 if( dbg && ret == DTM_ERROR   ) bcdtmWrite_message(0,0,0,"Determining Feature Extent Error") ;
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
BENTLEYDTM_EXPORT int bcdtmClip_polygonToTinHullDtmObject
(
 BC_DTM_OBJ *dtmP,                  // ==> Dtm Object
 DPoint3d *polygonPtsP,                  // ==> Feature Points
 long numPolygonPts,                // ==> Number Of Feature Points
 DTM_POINT_ARRAY ***polygonsPPP,    // <== Pointer To Polygon Arrays
 long   *numPolygnsP                // <== Number Of Polygons  
) 
/*
** This Function Clips A Polygon To The Tin Hull
** It May Result In None or One Or More Polgons
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   ofs,numHullPts,numPolyPts,intersectFlag;
 DPoint3d    *p3dP,*polyPtsP=NULL,*hullPtsP=NULL ; 
 DTM_POLYGON_OBJ *polygonP=NULL ;
 DTM_POINT_ARRAY *pArrayP=NULL,**pArrayPP ;
/*
** Write Status Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Clipping Polygon To Tin Hull") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"polygonPtsP     = %p",polygonPtsP) ;
    bcdtmWrite_message(0,0,0,"numPolygonPts   = %8ld",numPolygonPts) ;
    for( p3dP = polygonPtsP ; p3dP < polygonPtsP + numPolygonPts ; ++p3dP )
      {
       bcdtmWrite_message(0,0,0,"Point[%4ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-polygonPtsP),p3dP->x,p3dP->y,p3dP->z) ;
      }
   }
/*
** Check For Valid DTM Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check DTM Is Triangulated
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Initialise
*/
 *numPolygnsP = 0 ; 
 if( *polygonsPPP != NULL ) 
   {
    bcdtmWrite_message(2,0,0,"None Null Pointer") ;
    goto errexit ;
   }
/*
** Validate Polygon
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Validating Polygon") ; 
// if( bcdtmMath_validatePointArrayPolygon(&polyPtsP,&numPolyPts,ppTol) ) goto errexit ;
/*
**  Intersect Region With Tin Hull
*/
 if( bcdtmPolygon_intersectPolygonAndTinHullDtmObject(dtmP,polygonPtsP,numPolygonPts,&polygonP,&intersectFlag)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"intersectFlag = %2ld",intersectFlag) ;
/*
**  Tin Hull Totally Internal To Polygon -  Clip Polygon To Tin Hull
*/
 if( intersectFlag == 1 )
   {
    *numPolygnsP = 1 ;
    *polygonsPPP = ( DTM_POINT_ARRAY **) malloc(sizeof(DTM_POINT_ARRAY *)) ;
    if( *polygonsPPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }  
    **polygonsPPP = NULL ;
    pArrayP = ( DTM_POINT_ARRAY *) malloc(sizeof(DTM_POINT_ARRAY)) ;
    if( pArrayP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      } 
    pArrayP->pointsP = NULL ;  
    pArrayP->numPoints = 0 ; 
    **polygonsPPP = pArrayP ;
    pArrayP = NULL ;
    if( bcdtmList_extractHullDtmObject(dtmP,&hullPtsP,&numHullPts)) goto errexit ;
    (**polygonsPPP)->pointsP   = hullPtsP ;
    (**polygonsPPP)->numPoints = numHullPts ;
    hullPtsP = NULL ; 
   }  
/*
**  Tin Hull And Polygon Intersect In One Or More Intersections
*/
 if( intersectFlag == 2 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Polygons = %2ld",polygonP->numPolygons) ;
    *numPolygnsP = polygonP->numPolygons ;
    *polygonsPPP = ( DTM_POINT_ARRAY **) malloc(*numPolygnsP*sizeof(DTM_POINT_ARRAY *)) ;
    if( *polygonsPPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      } 
/*
**  Allocate Point Array Memory
*/
    for( pArrayPP = *polygonsPPP ; pArrayPP < *polygonsPPP + *numPolygnsP ; ++pArrayPP )
      {
       *pArrayPP = NULL ;
       pArrayP = ( DTM_POINT_ARRAY *) malloc(sizeof(DTM_POINT_ARRAY)) ;
       if( pArrayP == NULL )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         } 
       pArrayP->pointsP = NULL ;  
       pArrayP->numPoints = 0 ; 
       *pArrayPP = pArrayP ;
       pArrayP = NULL ;
      } 
/*
**  Get Intersect Polygons
*/
    for( ofs = 0 ; ofs < *numPolygnsP ; ++ofs )
      {
       if( bcdtmPolygon_copyPolygonObjectPolygonToPointArrayPolygon(polygonP,ofs,&polyPtsP,&numPolyPts))goto errexit ;
       (*(*polygonsPPP+ofs))->pointsP   = polyPtsP ;
       (*(*polygonsPPP+ofs))->numPoints = numPolyPts ;
       polyPtsP = NULL ;
      }  
   }
/*
** Clean Up
*/
 cleanup :
 if( pArrayP    != NULL ) free(pArrayP) ;
 if( hullPtsP   != NULL ) free(hullPtsP) ;
 if( polyPtsP   != NULL ) free(polyPtsP) ;      
 if( polygonP   != NULL ) bcdtmPolygon_deletePolygonObject(&polygonP) ;
/*
** Job Completed
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if(*polygonsPPP != NULL ) bcdtmMem_freePointerArrayToPointArrayMemory(polygonsPPP,*numPolygnsP) ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmClip_validateVoidIslandClippingBoundariesDtmObject
(
 BC_DTM_OBJ *dtmP,               // ==> Pointer To Dtm Object To Insert Bounadries  
 BC_DTM_OBJ *bndyDtmP,           // ==> Pointer To Dtm Object Containing Boundaries 
 BC_DTM_OBJ *clipDtmP            // ==> Pointer To Dtm Object Containing Validated Boundaries
)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long poly,dtmFeature,numHullPts,numClipPts,intersectFlag ;
 DTMDirection direction;
 long numStartFeatures,numVoids,numIslands ;
 DPoint3d  *p3dP,*hullPtsP=NULL,*clipPtsP=NULL ;
 double area ;
 char   dtmFeatureTypeName[256] ;
 BC_DTM_FEATURE  *dtmFeatureP ;
 DTM_POLYGON_OBJ *polyP=NULL ;
 DTMFeatureId  nullFeatureId=DTM_NULL_FEATURE_ID  ; 
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Validating Island And Void Clipping Boundaries") ; 
    bcdtmWrite_message(0,0,0,"dtmP       = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"bndyDtmP   = %p",bndyDtmP) ;
    bcdtmWrite_message(0,0,0,"clipDtmP   = %p",clipDtmP) ;
   }
/*
** Get Hull Points
*/
 if( bcdtmList_extractHullDtmObject(dtmP,&hullPtsP,&numHullPts)) goto errexit ;
/*
** Scan Void Island Features
*/
 for( dtmFeature = 0 ; dtmFeature < bndyDtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(bndyDtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void || dtmFeatureP->dtmFeatureType == DTMFeatureType::Island )
      {

//     Log Feature Type
      
       if( dbg == 1 )
         {
          bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName);
          bcdtmWrite_message(0,0,0,"Validating %20s Feature Id = %10I64d",dtmFeatureTypeName,dtmFeatureP->dtmFeatureId ) ;
         } 
       if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(bndyDtmP,dtmFeature,&clipPtsP,&numClipPts)) goto errexit ;
/*
**     Validate Clipping polygon
*/
       if( bcdtmClean_validatePointArrayPolygon(&clipPtsP,&numClipPts,1,dtmP->mppTol*10000.0)) goto errexit ;
/*
**     Log Area Before Intersection With Tin Hull
*/
       if( dbg == 2 )
         {
          bcdtmMath_getPolygonDirectionP3D(clipPtsP,numClipPts,&direction,&area) ;
          bcdtmWrite_message(0,0,0,"Polygon Before Hull Intersection ** Direction = %2ld Area = %12.4lf",direction,area) ;
         } 
/*
**     Intersect Clipping Polygon With Tin Hull
*/
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Intersecting Dtm Hull And Clip Polygon") ;
       if( bcdtmPolygon_intersectPolygons(hullPtsP,numHullPts,clipPtsP,numClipPts,&intersectFlag,&polyP,0.0,0.0)) goto errexit ;
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Intersect Flag = %1ld Number Of Clipping Polygons = %4ld",intersectFlag,polyP->numPolygons) ;
       if( intersectFlag == 0 ) 
         { 
          bcdtmWrite_message(1,0,0,"Clip Polygon and Dtm Hull Do Not Intersect") ;
          goto errexit ;
         }
/*
**     Store Clipping Polygons
*/
       for( poly = 0 ; poly < polyP->numPolygons ; ++poly )
         { 
          if( bcdtmPolygon_copyPolygonObjectPolygonToPointArrayPolygon(polyP,poly,&clipPtsP,&numClipPts)) goto errexit ;
/*
**        Log Area After Intersection With Tin Hull
*/
          if( dbg == 2 )
            {
             bcdtmMath_getPolygonDirectionP3D(clipPtsP,numClipPts,&direction,&area) ;
             bcdtmWrite_message(0,0,0,"Polygon After  Hull Intersection ** Direction = %2ld Area = %12.4lf",direction,area) ;
            } 
/*
**        Log Points
*/            
          if( dbg == 2 )
            {
             bcdtmWrite_message(0,0,0,"Number Of Clip Feature Points = %8ld",numClipPts) ;
             for( p3dP = clipPtsP ; p3dP < clipPtsP +numClipPts ; ++p3dP ) 
               {
                bcdtmWrite_message(0,0,0,"Clip Point[%5ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-clipPtsP),p3dP->x,p3dP->y,p3dP->z) ;
               }
            }
          if( bcdtmObject_storeDtmFeatureInDtmObject(clipDtmP,dtmFeatureP->dtmFeatureType,DTM_NULL_USER_TAG,1,&nullFeatureId,clipPtsP,numClipPts)) goto errexit ;
         }
       if( polyP != NULL ) bcdtmPolygon_deletePolygonObject(&polyP) ;
      }
   }
/*
** Set Number Of Start Features
*/   
 numStartFeatures = clipDtmP->numFeatures ; 
/*
** Count Validated Void And Island Occurrences
*/
 numVoids = numIslands = 0 ;
 for( dtmFeature = 0 ; dtmFeature < clipDtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(clipDtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void   ) ++numVoids ;
    if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Island ) ++numIslands ;
   } 
 if( dbg ) bcdtmWrite_message(0,0,0,"numStartFeatures = %8ld numVoids = %8ld numIslands = %8ld",numStartFeatures,numVoids,numIslands) ;  
/*
**  Resolve Voids
*/
 if( numVoids > 1 && numIslands == 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Resolving Intersecting Voids") ;
    if( bcdtmData_resolveIntersectingPolygonalDtmFeatureTypeDtmObject(clipDtmP,DTMFeatureType::Void)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting Voids Resolved") ;
   }  
/*
**  Resolve Islands
*/
 else if( numIslands > 1 && numVoids == 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Resolving Intersecting Islands") ;
    if( bcdtmData_resolveIntersectingPolygonalDtmFeatureTypeDtmObject(clipDtmP,DTMFeatureType::Island)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting Islands Resolved") ;
   }  
/*
**  Resolve Voids And Islands
*/
 else if( numIslands > 0 && numVoids > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Resolving Intersecting Islands And Voids") ;
//    if( bcdtmData_resolveIntersectingPolygonalDtmFeatureTypeDtmObject(clipDtmP,DTMFeatureType::Island)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting Islands And Voids Resolved") ;
   }  
/*
** Clean Up
*/
 cleanup :
 if( hullPtsP    != NULL ) free(hullPtsP) ;
 if( clipPtsP    != NULL ) free(clipPtsP) ;
 if( polyP       != NULL ) bcdtmPolygon_deletePolygonObject(&polyP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Validating Island And Void Clipping Boundaries Completed") ; 
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Validating Island And Void Clipping Boundaries Error") ; 
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
BENTLEYDTM_Private int bcdtmClip_resolveVoidIslandClippingBoundariesDtmObject
(
 BC_DTM_OBJ *dtmP,               // ==> Pointer To Dtm Object To Insert Bounadries  
 long       numStartFeatures     // ==> Number Of Feature Before Inserting Void Island Clipping Features
)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 long dtmFeature ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Resolving Void Island Clipping Boundaries") ; 
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"numStartFeatures  = %8ld",numStartFeatures) ;
    bcdtmWrite_message(0,0,0,"dtmP->numFeatures = %8ld",dtmP->numFeatures) ;
   }
/*
** Scan Inserted Features
*/
 for( dtmFeature = numStartFeatures ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
/*
**  Void All Points Internal To Void
*/    
    if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void )
      {
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Voiding Points Internal To Void Feature %8ld",dtmFeature) ;
       if( bcdtmMark_pointsInternalToVoidDtmObject(dtmP,dtmFeature)) goto errexit ;
      }
/*
**  Void All Points External To Island
*/    
    if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Island )
      {
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Voiding Points External To Island Feature %8ld",dtmFeature) ;
       if( bcdtmMark_pointsExternalToIslandDtmObject(dtmP,dtmFeature)) goto errexit ;
      }  
   }
    
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Validating Island And Void Clipping Boundaries Completed") ; 
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Validating Island And Void Clipping Boundaries Error") ; 
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
BENTLEYDTM_EXPORT int bcdtmClip_cloneAndInsertIslandAndVoidBoundariesDtmObject
(
 BC_DTM_OBJ *dtmP,               // ==> Pointer To Dtm Object  
 BC_DTM_OBJ *bndyDtmP,           // ==> Pointer To Dtm Object Containing Boundaries
 BC_DTM_OBJ **clonedDtmPP        // ==> Pointer To Cloned Dtm Object With Inserted Boundaries 
)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Cloning And Inserting Island And Void Boundaries") ; 
    bcdtmWrite_message(0,0,0,"dtmP         = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"bndyDtmP     = %p",bndyDtmP) ;
    bcdtmWrite_message(0,0,0,"clonedDtmPP  = %p",clonedDtmPP) ;
   }
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
 if( bcdtmObject_testForValidDtmObject(bndyDtmP)) goto errexit  ;
/*
** Check If DTM Is In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Check If Boundary DTM Is In Data State
*/
 if( bndyDtmP->dtmState != DTMState::Data )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Un-Triangulated DTM For Boundaries") ;
    goto errexit ;
   }
/*
** Check Cloned DTM Is Null
*/
 if( *clonedDtmPP != NULL )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Null Pointer For Clone DTM ") ;
    goto errexit ;
   }  
/*
**  Clone DTM
*/
 if( bcdtmObject_cloneDtmObject(dtmP,clonedDtmPP)) goto errexit ;
/*
** Insert Island And Void Boundaries
*/ 
 if( bcdtmClip_insertIslandAndVoidBoundariesDtmObject(*clonedDtmPP,bndyDtmP)) goto errexit ;     
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Cloning And Inserting Island And Void Clipping Boundaries Completed") ; 
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Cloning And Inserting Island And Void Clipping Boundaries Error") ; 
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
BENTLEYDTM_EXPORT int bcdtmClip_insertIslandAndVoidBoundariesDtmObject
(
 BC_DTM_OBJ *dtmP,               // ==> Pointer To Dtm Object To Insert Bounadries  
 BC_DTM_OBJ *bndyDtmP            // ==> Pointer To Dtm Object Containing Boundaries
)
/*
** This Function Inserts Island And Void Boundaries Into A Triangulated DTM
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long numVoids,numIslands,dtmFeature,numFeatures,numFeaturePts ;
 long startPnt,numStartFeatures;
 DTMDirection direction;
 long clipTime=bcdtmClock() ;
 double area ;
 DPoint3d  *featurePtsP=NULL ;
 BC_DTM_FEATURE *dtmFeatureP ;
 BC_DTM_OBJ *clipDtmP=NULL ;
 double savePptol=dtmP->ppTol,savePltol=dtmP->plTol ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Inserting Island And Void Boundaries") ; 
    bcdtmWrite_message(0,0,0,"dtmP         = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"bndyDtmP     = %p",bndyDtmP) ;
   }
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
 if( bcdtmObject_testForValidDtmObject(bndyDtmP)) goto errexit  ;
/*
** Check If DTM Is In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Check If Boundary DTM Is In Data State
*/
 if( bndyDtmP->dtmState != DTMState::Data )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Un-Triangulated DTM") ;
    goto errexit ;
   }
/*
** Count Number Of Clip Features
*/
 numVoids = numIslands = numFeatures = 0 ;
 for( dtmFeature = 0 ; dtmFeature < bndyDtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(bndyDtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void   ) ++numVoids ;
    if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Island ) ++numIslands ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Before Validating ** numVoids = %8ld numIslands = %8ld ** numFeatures = %8ld ",numVoids,numIslands,bndyDtmP->numFeatures) ;
/*
** Create DTM For Storing Validated Clipping Boundaries
*/   
 if( bcdtmObject_createDtmObject(&clipDtmP)) goto errexit ;
/*
** Validate Clipping Boundaries   
*/   
 if( bcdtmClip_validateVoidIslandClippingBoundariesDtmObject(dtmP,bndyDtmP,clipDtmP)) goto errexit ;
/*
** Count Number Of Clip Feature Boundaries After Validating
*/
 numVoids = numIslands = numFeatures = 0 ;
 for( dtmFeature = 0 ; dtmFeature < clipDtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(clipDtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void   ) ++numVoids ;
    if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Island ) ++numIslands ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"After  Validating ** numVoids = %8ld numIslands = %8ld ** numFeatures = %8ld ",numVoids,numIslands,clipDtmP->numFeatures) ;
/*
** Process If Clip Features Are Present
*/
 if( numVoids > 0 || numIslands > 0 )
   {
/*
**  If There is Only One Clip Boundary Use Core Clipping Method
*/
    if( ( numVoids == 1 && numIslands == 0 ) || ( numVoids == 0 && numIslands == 1 ))
      {
       for( dtmFeature = 0 ; dtmFeature < clipDtmP->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(clipDtmP,dtmFeature) ;
          if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void || dtmFeatureP->dtmFeatureType == DTMFeatureType::Island ) 
            {
             if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(clipDtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
             if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void )
               {
                if( bcdtmClip_toPolygonDtmObject(dtmP,featurePtsP,numFeaturePts,DTMClipOption::Internal)) goto errexit ;
               }
             else if ( dtmFeatureP->dtmFeatureType == DTMFeatureType::Island )  
               {
                if( bcdtmClip_toPolygonDtmObject(dtmP,featurePtsP,numFeaturePts,DTMClipOption::External)) goto errexit ;
               }
            }
         }    
      }
/*
**  Multiple Clip Boundaries So Insert As Voids and Islands
*/
    else if( numVoids >= 1 || numIslands >= 1 )  
      {
/*
**     Log Validated Clipping Boundaries
*/
       if( dbg == 1 ) bcdtmObject_reportStatisticsDtmObject(clipDtmP) ;  
/*
**     Remove Tin Error And Roll Back Features
*/
       if( bcdtmData_deleteAllTinErrorFeaturesDtmObject(dtmP)) goto errexit ;
       if( bcdtmData_deleteAllRollBackFeaturesDtmObject(dtmP)) goto errexit ;
       if( bcdtmList_cleanDtmObject(dtmP)) goto errexit ;
/*
**     Set Insert Tolerance Settings
*/
       if( dtmP->ppTol > dtmP->mppTol * 10000.0 ) dtmP->ppTol = dtmP->plTol = dtmP->mppTol * 10000.0 ;
       if( dtmP->plTol > dtmP->mppTol * 10000.0 ) dtmP->ppTol = dtmP->plTol = dtmP->mppTol * 10000.0 ;
/*
**     Insert Clipping Boundaries Into Tin
*/
       numStartFeatures = dtmP->numFeatures ;
       for( dtmFeature = 0 ; dtmFeature < clipDtmP->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(clipDtmP,dtmFeature) ;
          if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(clipDtmP,dtmFeature,&featurePtsP,&numFeaturePts)) goto errexit ;
          if( bcdtmInsert_internalStringIntoDtmObject(dtmP,1,2,featurePtsP,numFeaturePts,&startPnt)) goto errexit ;
          if( dbg == 1 )
            {
             if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPnt,&area,&direction)) goto errexit ;
             bcdtmWrite_message(0,0,0,"Tptr Polygon Direction     = %12ld",direction) ;
             bcdtmWrite_message(0,0,0,"Tptr Polygon Area          = %12.4lf",area) ;
            } 
          if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,dtmFeatureP->dtmFeatureType,dtmP->nullUserTag,dtmP->nullFeatureId,startPnt,1)) goto errexit  ;          
         }
       if( dbg ) bcdtmWrite_message(0,0,0,"numStartFeatures = %8ld dtmP->numFeatures = %8ld",numStartFeatures,dtmP->numFeatures) ;              
/*
**     Resolve Void Island Clipping
*/
//       if( bcdtmClip_resolveVoidIslandClippingBoundariesDtmObject(dtmP,numStartFeatures)) goto errexit ; 
       if( clipDtmP    != NULL ) bcdtmObject_destroyDtmObject(&clipDtmP) ; 
       if( bcdtmData_resolveTinStateIslandsVoidsAndHolesDtmObject(dtmP,&clipDtmP)) goto errexit ;
      }
/*
**  Check Triangulation
*/
    if( cdbg  )
      {
       bcdtmWrite_message(0,0,0,"Checking Clipped DTM") ;
       if( bcdtmCheck_tinComponentDtmObject(dtmP)) 
         {
          bcdtmWrite_message(0,0,0,"Clipped DTM Invalid") ;
          goto errexit ;
         }
       else  bcdtmWrite_message(0,0,0,"Clipped DTM Valid") ; 
      }
   }      
/*
** Clean Up
*/
 cleanup :
 dtmP->ppTol = savePptol ;
 dtmP->plTol = savePltol ;
 if( featurePtsP != NULL ) free(featurePtsP) ;
 if( clipDtmP    != NULL ) bcdtmObject_destroyDtmObject(&clipDtmP) ; 
/*
** Job Completed
*/
 if( tdbg ) bcdtmWrite_message(0,0,0,"**** Time To Clip Dtm Object = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),clipTime)) ;
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Island And Void Boundaries Completed") ; 
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Island And Void Boundaries Error") ; 
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
 bcdtmList_nullSptrValuesDtmObject(dtmP) ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmClip_checkPolygonsIntersect(DPoint3d *Poly1,long numPts1,DPoint3d *Poly2,long numPts2,long *IntersectFlag)
/*
** This Function Get The Intesection Polygon of Two Polygons
** Return Values for IntersectFlag == 0 No Intersection
**                                 == 1 Intesection
**                                 == 2 Poly1 is inside Poly2
**                                 == 3 Poly2 is inside Poly3
** Notes
**  1. Direction Of Poly1 & Poly2 must be anticlockwise
**  2. Direction Of Poly3 is set anticlockwise
*/
{
 DPoint3d    *p3d1,*p3d2 ;
 long   coinflag,intflag  ;
 double Xi,Yi ;
/*
** Initialise Values
*/
 *IntersectFlag = 0 ;
 coinflag = intflag = 0 ;
/*
** Check For Coincident Points On Polygons
*/
 for( p3d1 = Poly1 ; p3d1 < Poly1 + numPts1 && ! coinflag ; ++p3d1 )
   {
    for( p3d2 = Poly2 ; p3d2 < Poly2 + numPts2 && ! coinflag ; ++p3d2 )
      {
       if( p3d1->x == p3d2->x && p3d1->y == p3d2->y ) 
         { coinflag = 1 ; *IntersectFlag = 1 ; }
      }
   }
/*
** Check For Intersecting Lines
*/
 if( ! coinflag )
   {
    for( p3d1 = Poly1 ; p3d1 < Poly1 + numPts1 - 1 && ! intflag ; ++p3d1 )
      {
       for( p3d2 = Poly2 ; p3d2 < Poly2 + numPts2 - 1 && ! intflag ; ++p3d2 )
         {
          if(bcdtmMath_intersectCordLines(p3d1->x,p3d1->y,(p3d1+1)->x,(p3d1+1)->y,p3d2->x,p3d2->y,(p3d2+1)->x,(p3d2+1)->y,&Xi,&Yi))  
            { intflag = 1 ; *IntersectFlag = 1 ; }
         }
      }
    if( ! intflag )
      {
/*
**  Check If Polygon 1 is inside Polygon 2
*/
       if     ( bcdtmClip_pointInPointArrayPolygon(Poly2,numPts2,Poly1->x,Poly1->y )) *IntersectFlag = 2 ; 
/*
**  Check If Polygon 2 is inside Polygon 1
*/
       else if( bcdtmClip_pointInPointArrayPolygon(Poly1,numPts1,Poly2->x,Poly2->y )) *IntersectFlag = 3 ; 
      }
   }
/*
**  Check If Polygons Intersect
*/
 if( coinflag || intflag ) *IntersectFlag = 1 ;
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
BENTLEYDTM_EXPORT int bcdtmClip_pointInPointArrayPolygon(DPoint3d *PolyPts,long NumPolyPts,double Xp,double Yp)
/*
** This Function Tests if the Point (Xp,Yp) lies in a polygon
**
** Return Value = 0 Point not in Polygon
**              = 1 Point in Polygon
**
*/
{
 long    Nints,Int ;
 DPoint3d     *p3dP ;
 double  xPmin,xPmax,yPmin,yPmax,xOut,yOut,Xc,Yc ;
/*
** Test for Number of Points
*/
 if( NumPolyPts <= 0 ) return(0) ;
/*
** Calculate Minimium Bounding Rectangle
*/
 xPmin = xPmax = PolyPts->x ;
 yPmin = yPmax = PolyPts->y ;
 for( p3dP = PolyPts + 1 ; p3dP < PolyPts + NumPolyPts ; ++p3dP )
   {
    if( xPmin > p3dP->x ) xPmin = p3dP->x ;
    if( xPmax < p3dP->x ) xPmax = p3dP->x ;
    if( yPmin > p3dP->y ) yPmin = p3dP->y ;
    if( yPmax < p3dP->y ) yPmax = p3dP->y ;
   }
 xOut = 2.0 * (xPmax - xPmin) + xPmin ;
 yOut = 2.0 * (yPmax - yPmin) + yPmin ;
/*
**  Count number of IntercePts of Line (Xp,Yp),(xOut,yOut)
*/
 Nints = 0 ;
 for( p3dP = PolyPts + 1 ; p3dP < PolyPts + NumPolyPts ; ++p3dP )
   {
    Int   = bcdtmMath_intersectCordLines( p3dP->x,p3dP->y,(p3dP-1)->x,(p3dP-1)->y,Xp,Yp,xOut,yOut,&Xc,&Yc) ;
    Nints = Nints + Int ;
    if( Xc == p3dP->x && Yc == p3dP->y && Int )
      {
       if( p3dP == PolyPts + NumPolyPts - 1 ) --Nints ;
       else                                   ++p3dP  ;
      }
   }
/*
** If Odd  Number of Intercepts Point in Polygon
** If Even Number of Intercepts Point not in Polygon
*/
 if( Nints % 2 ) return(1) ; else return(0) ;
}
