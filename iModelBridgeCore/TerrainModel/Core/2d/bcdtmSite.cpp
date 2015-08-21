/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmSite.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
#include "bcdtminlines.h" 
//#pragma optimize( "p", on )
/*==============================================================================*//**
* @memo   Test For DTM Dtm Features Internal To Tin Hull
* @doc    Test For DTM Dtm Features Internal To Tin Hull 
* @notes  None
* @author Rob Cormack 1 May 2003 rob@geopak.com
* @param  *dtmP                ==> Pointer To DTM Object        
* @param  *internalFeaturesP  <== Set To TRUE For Internal dtmFeatures Found, Else set to FALSE  
* @return DTM_SUCCESS or DTM_ERROR
* @version 
* @see None
*===============================================================================*/
BENTLEYDTM_EXPORT int bcdtmSite_testForDtmFeaturesInternalToTinHullDtmObject
(
 BC_DTM_OBJ  *dtmP,              /* ==> Pointer To DTM object                        */
 long        *internalFeaturesP  /* <== Set To TRUE If Internal dtmFeatures else FALSE  */
)  
{
 int  ret=DTM_SUCCESS ;
 long np,sp,fpnt,dtmFeature ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Initialise
*/
 *internalFeaturesP = FALSE ;
/*
** Check For Valid DTM Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP) ) goto errexit ;
/*
** Check DTM Is Triangulated
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Scan dtmFeatures 
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures && *internalFeaturesP == FALSE ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
/*
**  Check For Valid DTMFeatureState::Tin Feature
*/
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
      {
/*
**     Scan dtmFeature Points And Test For dtmFeature Lines Internal To Tin Hull
*/
       sp = fpnt = dtmFeatureP->dtmFeaturePts.firstPoint ;
       if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sp,&np)) goto errexit ;
       while( np != fpnt && np != dtmP->nullPnt && *internalFeaturesP == FALSE )
         {
          if     ( nodeAddrP(dtmP,sp)->hPtr == dtmP->nullPnt || nodeAddrP(dtmP,np)->hPtr == dtmP->nullPnt ) *internalFeaturesP = TRUE ;
          else if( nodeAddrP(dtmP,sp)->hPtr != np && nodeAddrP(dtmP,np)->hPtr != sp ) *internalFeaturesP = TRUE ;
          sp = np ;
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sp,&np)) goto errexit ;
         }
/*
**     Check Last Line Of Closed DTM dtmFeature
*/
       if( *internalFeaturesP == FALSE && np == fpnt )
         {
          if     ( nodeAddrP(dtmP,sp)->hPtr == dtmP->nullPnt || nodeAddrP(dtmP,np)->hPtr == dtmP->nullPnt ) *internalFeaturesP = TRUE ;
          else if( nodeAddrP(dtmP,sp)->hPtr != np && nodeAddrP(dtmP,np)->hPtr != sp ) *internalFeaturesP = TRUE ;
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
 return( ret) ;
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
BENTLEYDTM_Public int bcdtmSite_internalStringIntoDtmObject
(
 BC_DTM_OBJ  *dtmP,
 long        drapeOption,
 DPoint3d         *stringPtsP,
 long        numStringPts,
 long        *startPntP,
 long        *knotDetectedP
)
/*
**
** This Function Inserts A String Into A Tin Object
** Assumes String Is Internal To Tin And Has Been Validated
**
** drapeOption    = 1   Drape Intersect Vertices On Tin Surface
**                = 2   Break Intersect Vertices On Tin Surface 
** insertOption   = 1   Move DTMFeatureState::Tin Lines That Are Not Linear Features
**                = 2   Intersect Tin Lines
**
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long  pntNum,*pntsP,*pointsP=NULL,knotPoint ;
 DPoint3d   *p3dP ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Internal String Into Tin Object") ;
/*
** Initialise
*/
 *startPntP = dtmP->nullPnt ;
 *knotDetectedP = 0 ;
  bcdtmList_nullTptrValuesDtmObject(dtmP) ;
/*
** Allocate Memory To Hold Point Numbers
*/ 
 pointsP = ( long * ) malloc ( numStringPts * sizeof(long)) ;
 if( pointsP == NULL ) 
  { 
   bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
   goto errexit ;
  }
/*
** Store Points In Tin Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting pointsP") ;
 for( p3dP = stringPtsP , pntsP = pointsP ; p3dP < stringPtsP + numStringPts ; ++p3dP , ++pntsP)
   {
    if( bcdtmInsert_storePointInDtmObject(dtmP,drapeOption,1,p3dP->x,p3dP->y,p3dP->z,&pntNum)) goto errexit ;
    *pntsP = pntNum ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Tin pointsP = %6ld",dtmP->numPoints) ;
/*
** Check Tin Integrity - Development Only
*/
 if( cdbg ) 
   { 
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Tin After Inserting Points") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP)) goto errexit ; 
    if( dbg ) bcdtmWrite_message(0,0,0,"Tin OK") ;
   }
/*
** Store Lines In Tin Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Lines") ;
 for( pntsP = pointsP + 1 ; pntsP < pointsP + numStringPts ; ++pntsP )
   {
    if( *(pntsP-1) != *pntsP )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Line Between %6ld %6ld",*(pntsP-1),*pntsP) ;
       if( ( ret = bcdtmSite_lineBetweenPointsDtmObject(dtmP,*(pntsP-1),*pntsP,drapeOption,&knotPoint))!= DTM_SUCCESS ) 
         {
          if( dbg )
            {
             bcdtmWrite_message(0,0,0,"Line Insert Error") ;
             bcdtmWrite_message(0,0,0,"P1 = %6ld ** %10.4lf %10.4lf %8.4lf",*(pntsP-1),pointAddrP(dtmP,*(pntsP-1))->x,pointAddrP(dtmP,*(pntsP-1))->y,pointAddrP(dtmP,*(pntsP-1))->z ) ;
             bcdtmWrite_message(0,0,0,"P2 = %6ld ** %10.4lf %10.4lf %8.4lf",*pntsP,pointAddrP(dtmP,*pntsP)->x,pointAddrP(dtmP,*pntsP)->y,pointAddrP(dtmP,*pntsP)->z ) ;
            }
         }
       if( knotPoint != dtmP->nullPnt ) *knotDetectedP = 1 ;
      }
   } 
/*
** Check Tin Integrity - Development Only
*/
 if( cdbg ) 
   { 
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Tin After Inserting Lines") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP)) goto errexit ; 
    if( dbg ) bcdtmWrite_message(0,0,0,"Tin OK") ;
   }
/*
** Free memory
*/
 cleanup :
 *startPntP = *pointsP ;
 if( pointsP != NULL ) { free(pointsP) ; pointsP = NULL ; }
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Inserting Internal String Into Tin Object Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Inserting Internal String Into Tin Object Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == 0 ) ret = 1 ;
 goto cleanup ;
}  
/*-----------------------------------------------------------+
|                                                            |
|                                                            |
|                                                            |
+-----------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmSite_lineBetweenPointsDtmObject(BC_DTM_OBJ *dtmP,long firstPoint,long lastPoint,long drapeOption,long *knotPointP )
/*
** This Function Inserts A Line Between Two Points In a DTM Object
**
** dtmP         =     Pointer To DTM Object
** firstPoint   =     First Point Of Line
** lastPoint    =     Last Point Of Line
** drapeOption  = 1   Drape Vertices On Tin Surface
**              = 2   Break Vertices On Tin Surface 
**  
*/
{
 int    ret=DTM_SUCCESS,bkp,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   p1,p2,p3,p4,startPnt,endPnt,insertLine,voidLine ;
 double xc,yc,zc=0.0 ;
/*
** Initialise
*/
 p1 = p2 = p3 = dtmP->nullPnt ;
 startPnt = firstPoint ; endPnt = lastPoint ;
 *knotPointP = dtmP->nullPnt ;
/*
** Insert And Intersect Tin Lines 
*/
 if( dbg )
   { 
    bcdtmWrite_message(0,0,0,"startPnt = %6ld tPtr = %9ld ** %12.5lf %12.5lf",startPnt,nodeAddrP(dtmP,startPnt)->tPtr,pointAddrP(dtmP,+startPnt)->x,pointAddrP(dtmP,+startPnt)->y) ;
    bcdtmList_writeCircularListForPointDtmObject(dtmP,startPnt) ;
    bcdtmWrite_message(0,0,0,"endPnt   = %6ld tPtr = %9ld ** %12.5lf %12.5lf",endPnt,nodeAddrP(dtmP,endPnt)->tPtr,pointAddrP(dtmP,+endPnt)->x,pointAddrP(dtmP,+endPnt)->y) ;
    bcdtmList_writeCircularListForPointDtmObject(dtmP,endPnt) ;
   }
/*
** Process Until First Point Equals Last Point
*/
 while ( firstPoint != lastPoint )
   {
/*
**  Check For Knot If So Return
*/
    if( nodeAddrP(dtmP,firstPoint)->tPtr != dtmP->nullPnt ) 
      {
       if( dbg )
         { 
          bcdtmWrite_message(0,0,0,"Start Knot Detected firstPoint = %6ld lastPoint = %6ld",firstPoint,lastPoint) ;
          bcdtmWrite_message(0,0,0,"firstPoint = %6ld tPtr = %9ld ** %10.4lf %10.4lf %10.4lf",firstPoint,nodeAddrP(dtmP,startPnt)->tPtr,pointAddrP(dtmP,+firstPoint)->x,pointAddrP(dtmP,+firstPoint)->y,pointAddrP(dtmP,+firstPoint)->z) ;
          bcdtmWrite_message(0,0,0,"lastPoint = %6ld tPtr = %9ld ** %10.4lf %10.4lf %10.4lf",lastPoint,nodeAddrP(dtmP,endPnt)->tPtr,pointAddrP(dtmP,+lastPoint)->x,pointAddrP(dtmP,+lastPoint)->y,pointAddrP(dtmP,+lastPoint)->z) ;
         } 
       *knotPointP = firstPoint ;
      }
/*
**  Get Next Point
*/
    if( dbg )
      {
       bcdtmWrite_message(0,0,0,"=== firstPoint = %6ld ** %12.5lf %12.5lf",firstPoint,pointAddrP(dtmP,+firstPoint)->x,pointAddrP(dtmP,+firstPoint)->y) ;
       bcdtmWrite_message(0,0,0,"=== lastPoint  = %6ld ** %12.5lf %12.5lf",lastPoint,pointAddrP(dtmP,+lastPoint)->x,pointAddrP(dtmP,+lastPoint)->y) ;
      } 
    bkp = bcdtmInsert_getIntersectPointDtmObject(dtmP,firstPoint,lastPoint,p3,&insertLine,&p1,&p2,&p3,&xc,&yc) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"bkp = %6ld P1 = %6ld P2 = %9ld P3 = %9ld",bkp,p1,p2,p3)  ;
    if ( bkp == 0 ) goto errexit  ;
    if ( bkp == 8 ) { if( dbg ) bcdtmWrite_message(0,0,0,"Knot Detected") ; return(2) ; }
/*
**  Get z Value Of Intecept Point
*/
    if( bkp == 1 || bkp == 2 || bkp == 3 )
      {
       if( bkp == 1 )      { xc = pointAddrP(dtmP,+p1)->x ; yc = pointAddrP(dtmP,+p1)->y ; } 
       if( drapeOption == 1 )  bcdtmInsert_getZvalueDtmObject(dtmP,p1,p2,xc,yc,&zc) ;
       else                    bcdtmInsert_getZvalueDtmObject(dtmP,startPnt,endPnt,xc,yc,&zc) ;
      }
/*
**  Passes Through DTM Point
*/ 
    if( bkp == 1 ) 
      { 
       nodeAddrP(dtmP,firstPoint)->tPtr = p1 ; 
       pointAddrP(dtmP,+p1)->z = zc ; 
       firstPoint = p1 ; 
      }
/*
**  Intersects Internal Line
*/
    if( bkp == 2 || bkp == 3 )
      {
/*
**   Check For Knot If So Return
*/
       if( nodeAddrP(dtmP,p1)->tPtr == p2 || nodeAddrP(dtmP,p2)->tPtr == p1  ) 
         { 
          bcdtmWrite_message(0,0,0,"Knot Detected p1 = %6ld p2 = %6ld",p1,p2) ;
          bcdtmWrite_message(0,0,0,"p1 = %6ld tPtr = %6ld ** %10.4lf %10.4lf %10.4lf",p1,nodeAddrP(dtmP,p1)->tPtr,pointAddrP(dtmP,+p1)->x,pointAddrP(dtmP,+p1)->y,pointAddrP(dtmP,+p1)->z) ;
          bcdtmWrite_message(0,0,0,"p2 = %6ld tPtr = %6ld ** %10.4lf %10.4lf %10.4lf",p2,nodeAddrP(dtmP,p2)->tPtr,pointAddrP(dtmP,+p2)->x,pointAddrP(dtmP,+p2)->y,pointAddrP(dtmP,+p2)->z) ;
          return(2) ;
         } 
/*
**   Check For Void Line
*/
       bcdtmList_testForVoidLineDtmObject(dtmP,p1,p2,&voidLine) ;
       if( bcdtmList_deleteLineDtmObject(dtmP,p1,p2) ) goto errexit  ;
       if( bcdtmInsert_addPointToDtmObject(dtmP,xc,yc,zc,&p4) ) goto errexit  ;
       if( voidLine ) bcdtmFlag_setVoidBitPCWD(&nodeAddrP(dtmP,p4)->PCWD) ; 
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,firstPoint,p4,p1)) goto errexit  ;
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p4,firstPoint,dtmP->nullPnt)) goto errexit  ;
       if( bcdtmList_insertLineBeforePointDtmObject(dtmP,p1,p4,firstPoint)) goto errexit  ;
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p4,p1,firstPoint) ) goto errexit ;
       if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p2,p4,firstPoint) ) goto errexit ;
       if( bcdtmList_insertLineBeforePointDtmObject(dtmP,p4,p2,firstPoint)) goto errexit  ;
       if( p3 != dtmP->nullPnt )
         {
          if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p3,p4,p2)) goto errexit  ;
          if( bcdtmList_insertLineAfterPointDtmObject(dtmP,p4,p3,p1)) goto errexit  ;
         }  
/*
**     If Intersecting Tin Hull Update Hull Pointers
*/   
       if( bkp == 3 )
         {
          if(nodeAddrP(dtmP,p1)->hPtr == p2 ) { nodeAddrP(dtmP,p1)->hPtr = p4 ;nodeAddrP(dtmP,p4)->hPtr = p2 ; }
          if(nodeAddrP(dtmP,p2)->hPtr == p1 ) { nodeAddrP(dtmP,p2)->hPtr = p4 ;nodeAddrP(dtmP,p4)->hPtr = p1 ; }
         } 
/*
**     If Intersecting Inserted Line Feature Update Feature List Structure
*/
       if( insertLine ) if( bcdtmInsert_pointIntoAllDtmFeaturesDtmObject(dtmP,p1,p2,p4)) goto errexit  ; 
/*
**     Update Temporary Pointer  Array
*/
       nodeAddrP(dtmP,firstPoint)->tPtr = p4 ; 
       firstPoint = p4 ; 
      }
/*
**  Check Tin Precision
*/
    if( cdbg )
      {
       if( bcdtmCheck_precisionDtmObject(dtmP,1) ) 
         { 
          bcdtmWrite_message(1,0,0,"Tin Precision Invalid") ;
          goto errexit ;
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
/*-----------------------------------------------------------+
|                                                            |
|                                                            |
|                                                            |
+-----------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmSite_cleanTptrListDtmObject(BC_DTM_OBJ *dtmP,long startPnt)
/*
** This Function Cleans the tPtr Values After A Knot
** Has Been Detected Inserting A String Into The DTM
**
** dtmp      => Pointer To Dtm Object
** startPnt  => Start Point Of Tptr List
**
** Author :  Rob Cormack
** Date   :  15 March 2002
**
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  sp,lp,hp,numPts,*pntListPt=NULL,*pl ;
/*
** Write Status Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Cleaning Tptr List") ;
    bcdtmWrite_message(0,0,0,"dtmP      =  %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"startPnt  =  %ld",startPnt) ;
   }
/*
** Valiadte
*/
 if( nodeAddrP(dtmP,startPnt)->tPtr == dtmP->nullPnt ) goto cleanup ;
/*
** Count Number Of Points In Tptr List
*/
 numPts = 0 ;
 sp = lp = hp = startPnt ;
 do
   {
    ++numPts ;
    sp = nodeAddrP(dtmP,sp)->tPtr ; 
    if( sp < lp ) lp = sp ;
    if( sp > hp ) hp = sp ;
   } while ( sp != startPnt && sp != dtmP->nullPnt ) ;
 if( sp == startPnt ) ++numPts ;
 if( dbg ) bcdtmWrite_message(0,0,0,"numPts = %6ld lastPoint = %6ld Hp = %6ld",numPts,lp,hp) ;
/*
** Allocate Memory To Store Tptr List Points
*/
 pntListPt = ( long *) malloc( numPts * sizeof(long)) ;
 if( pntListPt == NULL ) { bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ; goto errexit ; }
/*
** Copy Tptr List Points To Point List
*/
 sp = startPnt ;
 pl = pntListPt ;
 do
   {
    *pl = sp ;
    ++pl ;
    sp = nodeAddrP(dtmP,sp)->tPtr ; 
   } while ( sp != startPnt && sp != dtmP->nullPnt ) ;
 if( sp == startPnt ) *pl = sp ;
/*
** Null Node tPtr Values
*/
 bcdtmList_rangeNullTptrValuesDtmObject(dtmP,lp,hp) ;
/*
** Copy Tptr List Points To Point List
*/
 for( pl = pntListPt ; pl < pntListPt + numPts - 1 ; ++pl ) nodeAddrP(dtmP,*pl)->tPtr = *(pl+1) ;
/*
** Clean Up
*/
 cleanup :
 if( pntListPt != NULL ) free(pntListPt) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Cleaning Tptr List Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Cleaning Tptr List Error") ;
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
BENTLEYDTM_EXPORT int bcdtmSite_createDisplayDtmObject
(
 BC_DTM_OBJ  *dtmP,              /* ==> Pointer To Tin Onject                               */
 BC_DTM_OBJ  **displayDtmPP,     /* <== Pointer To Display Tin                              */
 BC_DTM_OBJ  *dataP,             /* ==> Pointer To dataP Object With Site Object Boundaries */
 long         displayDtmOption   /* ==> Option For Creating A Display Tin
                                     ==   1  Create A Display Tin Of Site Objects Only
                                     ==   2  Create A Display Tin Excluding Site Objects
                                 */
)
/*
** This Function Creates A Display DTMFeatureState::Tin For Site Modeler
** The Display DTMFeatureState::Tin Cannot be used For Any Other Purposes Than Display
**
** Author : Rob Cormack
** Date     13 March 2002
** Updated  30 October 2006
** Converted To Partitioned DTM June 2008
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   startPnt,dtmFeature,numPolyPts,numFeatures,numObjects,knotDetected ;
 DTMFeatureType dtmFeatureType;
 long   numVoids=0,numIslands=0,numVoidPts=0 ;
 DTMDirection direction;
 double area ;
 DPoint3d    *p3dP,*polyPtsP=NULL ;
 BC_DTM_OBJ       *voidsP=NULL ;
 BC_DTM_FEATURE   *dtmFeatureP ;
/*
** Write Status Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Creating Display Dtm") ;
    bcdtmWrite_message(0,0,0,"dtmP             = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"displayDtmPP     = %p",*displayDtmPP) ;
    bcdtmWrite_message(0,0,0,"dataP            = %p",dataP) ;
    bcdtmWrite_message(0,0,0,"displayDtmOption = %8ld",displayDtmOption) ;
    if( displayDtmOption == 1 )
      {
       bcdtmWrite_toFileDtmObject(dtmP,L"displayType1.tin") ;
       bcdtmWrite_geopakDatFileFromDtmObject(dataP,L"displayType1Voids.dat") ;
      } 
    if( displayDtmOption == 2 )
      {
       bcdtmWrite_toFileDtmObject(dtmP,L"displayType2.tin") ;
       bcdtmWrite_geopakDatFileFromDtmObject(dataP,L"displayType2Voids.dat") ;
      } 
   }
/*
** Copy Object Boundaries To Temporary Object
*/
 if( bcdtmObject_cloneDtmObject(dataP,&voidsP)) goto errexit ;
/*
** Copy dtmP To Display dtmP
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Copying dtmP To Display dtmP") ;
 if( bcdtmObject_cloneDtmObject(dtmP,displayDtmPP)) goto errexit ;
/*
** Save Number Of Features In Display dtmP
*/
 numFeatures = (*displayDtmPP)->numFeatures ;
/*
** Resolve Intersecting And Internal Voids In Data Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Resolving Intersecting And Internal Voids") ;
 if( bcdtmSite_resolveVoidsDtmObject(dataP) ) goto errexit ;
 if( dbg ) bcdtmWrite_toFileDtmObject(dataP,L"voidsWithinObject.dat") ;
/*
** Count Number Of Islands And Voids
*/
 numVoids = numIslands = 0 ;
 for( dtmFeature = 0 ; dtmFeature < dataP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dataP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data && dtmFeatureP->dtmFeatureType == DTMFeatureType::Void   ) ++numVoids ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data && dtmFeatureP->dtmFeatureType == DTMFeatureType::Island ) ++numIslands ;
   } 
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Number Of Voids   = %6ld",numVoids) ;
    bcdtmWrite_message(0,0,0,"Number Of Islands = %6ld",numIslands) ;
   }
/*
** Resolve Intersecting And Internal Voids Within Islands
*/
 if( numIslands > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Resolving Intersecting And Internal Voids Within Islands") ;
    if( bcdtmSite_resolveVoidsWithinIslandsDtmObject(dataP,voidsP) ) goto errexit ;
    if( dbg ) 
      {
       numVoids = numIslands = 0 ;
       for( dtmFeature = 0 ; dtmFeature < dataP->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(dataP,dtmFeature) ;
          if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data && dtmFeatureP->dtmFeatureType == DTMFeatureType::Void   ) ++numVoids ;
          if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data && dtmFeatureP->dtmFeatureType == DTMFeatureType::Island ) ++numIslands ;
         } 
       bcdtmWrite_message(0,0,0,"Number Of Voids   = %6ld",numVoids) ;
       bcdtmWrite_message(0,0,0,"Number Of Islands = %6ld",numIslands) ;
       bcdtmWrite_toFileDtmObject(dataP,L"voidsWithinIslands.dat") ;
      }
   }
/*
** Delete Copy Of Voids Data Object
*/
 if( voidsP != NULL ) bcdtmObject_destroyDtmObject(&voidsP) ;
/*
**  Objects Only Exclude Model
*/
 if( displayDtmOption == 1 )
   {
/*
**  Clear ALL Tptr Values
*/
    bcdtmList_nullTptrValuesDtmObject(*displayDtmPP) ;
/*
**  Store DTMFeatureState::Tin Hull As Void Feature
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Storing Tin Hull As Void") ;
    if( bcdtmList_copyHptrListToTptrListDtmObject(*displayDtmPP,(*displayDtmPP)->hullPoint) ) goto errexit ;
    if( bcdtmInsert_addDtmFeatureToDtmObject(*displayDtmPP,NULL,0,DTMFeatureType::Void,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,(*displayDtmPP)->hullPoint,1)) goto errexit ;
/*
**  Insert Site Object Polygons As Islands
*/
    numObjects = 0 ;
    for( dtmFeature = 0 ; dtmFeature < dataP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dataP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data && ( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void || dtmFeatureP->dtmFeatureType == DTMFeatureType::Island ))
         {
          ++numObjects ;
          dtmFeatureType = dtmFeatureP->dtmFeatureType ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Site Object %4ld Hull Into Display dtmP",numObjects) ;   
/*
**        Get Points For DTM Feature
*/
          if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dataP,dtmFeature,&polyPtsP,&numPolyPts)) goto errexit ;
/*
**        Get Area And Direction Of Polygon
*/
          bcdtmMath_getPolygonDirectionP3D(polyPtsP,numPolyPts,&direction,&area) ;
          if( dbg ) 
            {
             bcdtmWrite_message(0,0,0,"Polygon Direction = %2ld Area = %20.10lf",direction,area) ;
             if( area < 0.001 ) 
               {
                bcdtmWrite_message(0,0,0,"Polygon Direction = %2ld Area = %20.10lf",direction,area) ;
                bcdtmWrite_message(0,0,0,"Number Of Poly Pts = %6ld",numPolyPts) ;
                for( p3dP = polyPtsP ; p3dP < polyPtsP + numPolyPts ; ++p3dP )
                  {
                   bcdtmWrite_message(0,0,0,"Point[%4ld] = %20.10lf %20.10lf %10.4lf",(long)(p3dP-polyPtsP),p3dP->x,p3dP->y,p3dP->z) ;   
                  }
               } 
            }
/*
**        Clean Polygon Points
*/
          if( area > 0.01 )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Cleaning Object Polygon Points") ;   
             if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Points Before Clean = %6ld",numPolyPts) ;
             if( bcdtmClean_internalPointArrayPolygon(&polyPtsP,&numPolyPts,(*displayDtmPP)->ppTol)) goto errexit ;
             if( bcdtmClean_externalPointArrayPolygon(&polyPtsP,&numPolyPts,(*displayDtmPP)->ppTol)) goto errexit ;
             if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Points After  Clean = %6ld",numPolyPts) ;
/*
**           Store Polygon Boundary As Island In Display dtmP
*/
             if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Object Boundary Into Display dtmP") ;   
             if( bcdtmSite_internalStringIntoDtmObject(*displayDtmPP,1,polyPtsP,numPolyPts,&startPnt,&knotDetected)) goto errexit ;
             if( knotDetected ) if( bcdtmSite_cleanTptrListDtmObject(*displayDtmPP,startPnt)) goto errexit ;
             if( dbg ) bcdtmWrite_message(0,0,0,"Storing Object Boundary As Island Feature") ;   
             if( bcdtmInsert_addDtmFeatureToDtmObject(*displayDtmPP,NULL,0,dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,startPnt,1)) goto errexit ;
            }
/*
**        Free Object Points
*/ 
          if( polyPtsP != NULL ) { free(polyPtsP) ; polyPtsP = NULL ; } 
         }
      }
/*
**   Count Number Of Voids And Islands In Display Tin
*/
    if( dbg )
      {
       numVoids = numIslands = 0 ;
       for( dtmFeature = 0 ; dtmFeature < (*displayDtmPP)->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(*displayDtmPP,dtmFeature) ;
          if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeatureType == DTMFeatureType::Void   ) ++numVoids ;
          if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeatureType == DTMFeatureType::Island ) ++numIslands ;
         } 
       bcdtmWrite_message(0,0,0,"Number Of Voids   = %6ld",numVoids) ;
       bcdtmWrite_message(0,0,0,"Number Of Islands = %6ld",numIslands) ;
      }
/*
**  Set All Void Points
*/
    if( bcdtmMark_setVoidPointsDtmObject(*displayDtmPP,FALSE,numFeatures,&numVoidPts)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"numVoidPts = %8ld of %8ld",numVoidPts,(*displayDtmPP)->numPoints) ;
   }
/*
**  Model Only Exclude Objects
*/
 if( displayDtmOption == 2 )
   {
/*
**  Clear ALL Tptr Values
*/
    bcdtmList_nullTptrValuesDtmObject(*displayDtmPP) ;
/*
**  Insert Site Object Polygons As Voids
*/
    numObjects = 0 ;
    for( dtmFeature = 0 ; dtmFeature < dataP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dataP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data && ( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void || dtmFeatureP->dtmFeatureType == DTMFeatureType::Island ))
         {
          ++numObjects ;
          dtmFeatureType = dtmFeatureP->dtmFeatureType ;
          if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Site Object %4ld Hull Into Display dtmP",numObjects) ;   
/*
**        Get Points For DTM Feature
*/
          if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dataP,dtmFeature,&polyPtsP,&numPolyPts)) goto errexit ;
/*
**        Get Area And Direction Of Polygon
*/
          bcdtmMath_getPolygonDirectionP3D(polyPtsP,numPolyPts,&direction,&area) ;
          if( dbg ) 
            {
             bcdtmWrite_message(0,0,0,"Polygon Direction = %2ld Area = %20.10lf",direction,area) ;
             if( area < 0.001 ) 
               {
                bcdtmWrite_message(0,0,0,"Number Of Poly Pts = %6ld",numPolyPts) ;
                for( p3dP = polyPtsP ; p3dP < polyPtsP + numPolyPts ; ++p3dP )
                  {
                   bcdtmWrite_message(0,0,0,"Point[%4ld] = %20.10lf %20.10lf %10.4lf",(long)(p3dP-polyPtsP),p3dP->x,p3dP->y,p3dP->z) ;   
                  }
               } 
            }
/*
**       Clean Polygon Points
*/
         if( area > 0.01 )
           {
            if( dbg ) bcdtmWrite_message(0,0,0,"Cleaning Object Polygon Points") ;   
            if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Points Before Clean = %6ld",numPolyPts) ;
            if( bcdtmClean_internalPointArrayPolygon(&polyPtsP,&numPolyPts,(*displayDtmPP)->ppTol)) goto errexit ;
            if( bcdtmClean_externalPointArrayPolygon(&polyPtsP,&numPolyPts,(*displayDtmPP)->ppTol)) goto errexit ;
            if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Points After  Clean = %6ld",numPolyPts) ;
/*
**          Store Polygon Boundary As Void In Display Tin
*/
            if( dbg ) bcdtmWrite_message(0,0,0,"Inserting Object Boundary Into Display dtmP") ;   
            if( bcdtmSite_internalStringIntoDtmObject(*displayDtmPP,1,polyPtsP,numPolyPts,&startPnt,&knotDetected)) goto errexit ;
            if( knotDetected ) if( bcdtmSite_cleanTptrListDtmObject(*displayDtmPP,startPnt)) goto errexit ;
            if( dbg ) bcdtmWrite_message(0,0,0,"Storing Object Boundary As Void Feature") ;   
            if( bcdtmInsert_addDtmFeatureToDtmObject(*displayDtmPP,NULL,0,dtmFeatureType,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,startPnt,1)) goto errexit ;
           }
/*
**       Free Object Points
*/
         if( polyPtsP != NULL ) { free(polyPtsP) ; polyPtsP = NULL ; } 
        }
     }
/*
** Set All Void Points
*/
   if( bcdtmMark_setVoidPointsDtmObject(*displayDtmPP,FALSE,numFeatures,&numVoidPts)) goto errexit ;
  }
/*
** Write Display Tin - Development Only
*/
 if( dbg ) bcdtmWrite_toFileDtmObject(*displayDtmPP,L"Display.tin") ;
/*
** Clean Up
*/
 cleanup :
 if( voidsP   != NULL ) bcdtmObject_destroyDtmObject(&voidsP) ;
 if( polyPtsP != NULL ) { free(polyPtsP) ; polyPtsP = NULL ; } 
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Creating Display Dtm Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Creating Display Dtm Error") ;
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
BENTLEYDTM_Public int bcdtmSite_resolveVoidsDtmObject(BC_DTM_OBJ *voidsP)
/*
** This Function Resolves Intersecting Voids
** Robc 12/11/2007. Modified This Function To Go down One More Level With Islands That
** Are Detected Within Intersecting Voids. Functions Needs To Be Made Recursive To
** Keep Going Down Until No More Voids Are Detected Within Islands.
*/
{
 int      ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long     sp,np,hp,ss,spf,npf,offset,clPtr,startPoint ;
 long     dtmFeature,breakFeature,numStartFeatures,islandBoundary ;
 long     firstPoint,numMarked,mark=-999999 ;
 long     numP3dPts,numVoids,numIslands;
 DTMDirection direction;
 double   area ; 
 unsigned char     *voidLineP=NULL ;
 DPoint3d      *p3dPtsP=NULL ;
 BC_DTM_OBJ      *dtmP=NULL ;
 BC_DTM_FEATURE  *dtmFeatureP ; 
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Resolving Voids") ;
    if( bcdtmWrite_toFileDtmObject(voidsP,L"unresolvedVoids.dat")) goto errexit ;
   }
/*
** Set Direction Of Voids AntiClockwise
*/
 if( bcdtmClean_setDtmPolygonalFeatureTypeAntiClockwiseDtmObject(voidsP,DTMFeatureType::Void)) goto errexit ;
/*
** Create DTM Object
*/
 if( bcdtmObject_createDtmObject(&dtmP)) goto errexit ;
 bcdtmObject_setPointMemoryAllocationParametersDtmObject(dtmP,2*voidsP->numPoints,voidsP->numPoints) ;
/*
** Scan Voids Object And Write To Data Object As Breaks
*/
 numVoids = 0 ;
 for( dtmFeature = 0 ; dtmFeature < voidsP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(voidsP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data && dtmFeatureP->dtmFeatureType == DTMFeatureType::Void )
      {
       if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(voidsP,dtmFeature,&p3dPtsP,&numP3dPts)) goto errexit ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Breakline,dtmFeatureP->dtmUserTag,1,&dtmFeatureP->dtmFeatureId,p3dPtsP,numP3dPts)) goto errexit ; 
       ++numVoids ;
      }
   }
/*
** Triangulate 
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Voids Tin") ;
 DTM_NORMALISE_OPTION  = FALSE ;             // To Inhibit Normalisation Of Coordinates - function 
 DTM_DUPLICATE_OPTION = FALSE ;             // To Inhibit Removal Of None Identical Points
 dtmP->ppTol = 0.0 ;
 dtmP->plTol = 0.0 ;  
 if( bcdtmObject_createTinDtmObject(dtmP,1,0.0)) goto errexit ;
 DTM_NORMALISE_OPTION  = TRUE ;
 DTM_DUPLICATE_OPTION = TRUE ;
/*
** Remove None Feature Hull Lines
*/
 if(dbg) bcdtmWrite_message(0,0,0,"Removing Non Feature Hull Lines") ;
 if( bcdtmList_removeNoneFeatureHullLinesDtmObject(dtmP)) goto errexit ;
 if( dbg ) bcdtmWrite_toFileDtmObject(dtmP,L"voidHulls.tin") ;
/*
** Mark Internal Void Tin Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Void Display Lines") ;
 if( bcdtmSite_markInternalFeatureLinesDtmObject(dtmP,&voidLineP)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Void Display Lines Completed") ;
/*
** Set Pointer To Last Feature In Tin
*/
 numStartFeatures = dtmP->numFeatures ;
/*
** Scan Tin Hull To Get Voids
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For External Voids") ;
 numVoids = 0 ;
 sp = dtmP->hullPoint ;
 do
   {
    np = nodeAddrP(dtmP,sp)->hPtr ;
/*
**  Only Proces If Point Has Not Been Already Included In A Void
*/
    if( nodeAddrP(dtmP,sp)->sPtr == dtmP->nullPnt )
      { 
       if( bcdtmList_testForBreakLineDtmObject(dtmP,sp,np))
         {
/*
**        Scan Around External Edge Of Break Lines
*/
          hp = sp ;
          nodeAddrP(dtmP,sp)->tPtr = np ;
          nodeAddrP(dtmP,sp)->sPtr = np ;
          do
            { 
             if( ( hp = bcdtmList_nextAntDtmObject(dtmP,np,hp)) < 0 ) goto errexit ;
             while ( ! bcdtmList_testForBreakLineDtmObject(dtmP,np,hp))
               {
                if( ( hp = bcdtmList_nextAntDtmObject(dtmP,np,hp)) < 0 ) goto errexit ;
               }
             nodeAddrP(dtmP,np)->tPtr = hp ;
             nodeAddrP(dtmP,np)->sPtr = hp ;
             ss = hp ;
             hp = np ;
             np = ss ;
            } while ( hp != sp ) ;
/*
**         Store Void Feature In Tin
*/
          if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,DTMFeatureType::Void,dtmP->nullUserTag,dtmP->nullFeatureId,sp,1)) goto errexit ; 
          ++numVoids ;
         }
      }
    sp = np ; 
   } while ( sp != dtmP->hullPoint ) ;
/*
** Get Internal Islands
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For Internal Islands") ;
 numIslands = 0 ;
 for( sp = 0 ; sp < dtmP->numPoints ; ++sp )
   {
/*
**  Only Proces If Point Has Not Been Already Included In A Island
*/
    if( nodeAddrP(dtmP,sp)->sPtr == dtmP->nullPnt ) 
      {
       clPtr = nodeAddrP(dtmP,sp)->cPtr ;
       islandBoundary = FALSE ;
       while ( clPtr != dtmP->nullPtr && islandBoundary == FALSE )
         {
          np    = clistAddrP(dtmP,clPtr)->pntNum ; 
          clPtr = clistAddrP(dtmP,clPtr)->nextPtr ; 
          if( np > sp && nodeAddrP(dtmP,sp)->hPtr != np && nodeAddrP(dtmP,np)->hPtr != sp )
            {
/*
**           Check For Island Boundary Line
*/
             spf = npf = 0 ;
             if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,sp,np)) goto errexit ;
             if( bcdtmFlag_testFlag(voidLineP,offset) ) spf = 1 ;
             if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,np,sp)) goto errexit ;
             if( bcdtmFlag_testFlag(voidLineP,offset) ) npf = 1 ;
             if( ( spf == 1 && npf == 0 ) || ( spf == 0 && npf == 1 )) islandBoundary = TRUE ;
/*
**           Island Boundary Line Detected
*/
             if( islandBoundary == TRUE ) 
               {
/*
**              Scan Clockwise Around Island Boundary Starting From Sp
*/ 
                if( npf == 1 )
                  {
                   hp = sp ;
                   nodeAddrP(dtmP,sp)->tPtr = np ;
                   nodeAddrP(dtmP,sp)->sPtr = np ;
                   do
                     { 
                      if( ( hp = bcdtmList_nextClkDtmObject(dtmP,np,hp)) < 0 ) goto errexit ;
                      while ( ! bcdtmList_testForBreakLineDtmObject(dtmP,np,hp))
                        {
                         if( ( hp = bcdtmList_nextClkDtmObject(dtmP,np,hp)) < 0 ) goto errexit ;
                        }
                      nodeAddrP(dtmP,np)->tPtr = hp ;
                      nodeAddrP(dtmP,np)->sPtr = hp ;
                      ss = hp ;
                      hp = np ;
                      np = ss ;
                     } while ( hp != sp ) ;
                  }  
/*
**              Scan  Clockwise Around Island Boundary Starting From Np
*/ 
                if( spf == 1 ) 
                  {
                   hp = np ;
                   nodeAddrP(dtmP,np)->tPtr = sp ;
                   nodeAddrP(dtmP,np)->sPtr = sp ;
                   do
                     { 
                      if( ( hp = bcdtmList_nextClkDtmObject(dtmP,sp,hp)) < 0 ) goto errexit ;
                      while ( ! bcdtmList_testForBreakLineDtmObject(dtmP,sp,hp))
                        {
                         if( ( hp = bcdtmList_nextClkDtmObject(dtmP,sp,hp)) < 0 ) goto errexit ;
                        }
                      nodeAddrP(dtmP,sp)->tPtr = hp ;
                      nodeAddrP(dtmP,sp)->sPtr = hp ;
                      ss = hp ;
                      hp = sp ;
                      sp = ss ;
                     } while ( hp != np ) ;
                  }
/*
**              False Island If Direction Is Clockwise
*/
                bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,sp,&area,&direction) ;
                if( direction == DTMDirection::AntiClockwise )
                  {
                   if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,DTMFeatureType::Island,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,sp,1)) goto errexit ; 
                   ++numIslands ;
                  }
                else bcdtmList_nullTptrListDtmObject(dtmP,sp) ; 
               }
            }
         }
      }
   }
/*
** Get Voids Internal To Islands
*/
 if( numIslands > 0 ) 
   { 
    if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For Voids Internal To Islands") ;
      {
       for( dtmFeature = numStartFeatures ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
          if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Island )
            {
             if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,dtmFeature,&startPoint)) goto errexit ;
             if( bcdtmMark_internalTptrPolygonPointsDtmObject(dtmP,startPoint,mark,&numMarked) ) goto errexit ;
/*
**           Scan For Marked Void Features
*/
             for( sp = 0 ; sp < dtmP->numPoints ; ++sp )
               {
                if( nodeAddrP(dtmP,sp)->tPtr == mark )
                  {
                   if( bcdtmList_testForBreakPointDtmObject(dtmP,sp))
                     {
/*
**                    Place Tptr List  
*/         
                      breakFeature = flistAddrP(dtmP,nodeAddrP(dtmP,sp)->fPtr)->dtmFeature ;
                      if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,breakFeature,&firstPoint)) goto errexit ;
                      if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,DTMFeatureType::Void,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,firstPoint,1)) goto errexit ; 
                     }
                  }
               } 
            } 
         }
      }
   } 
/*
** Write Number Of Resolved Polygons
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Voids   = %6ld",numVoids) ;
    bcdtmWrite_message(0,0,0,"Number Of Islands = %6ld",numIslands) ;
   }
/*
** Initialise Voids Data Object
*/
 bcdtmObject_initialiseDtmObject(voidsP) ;
/*
** Copy Features From Tin To Voids Object
*/
 numVoids   = 0 ;
 numIslands = 0 ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void || dtmFeatureP->dtmFeatureType == DTMFeatureType::Island )
      {
       if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&p3dPtsP,&numP3dPts)) goto errexit ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(voidsP,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,1,&dtmFeatureP->dtmFeatureId,p3dPtsP,numP3dPts)) goto errexit ; 
       if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void   ) ++numVoids   ;
       if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Island ) ++numIslands ;
      }
   }
/*
** Write Number Of Resolved Polygons
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Voids   Written = %6ld",numVoids) ;
    bcdtmWrite_message(0,0,0,"Number Of Islands Written = %6ld",numIslands) ;
   }
/*
** Write Resolved Voids 
*/
 if( dbg ) if( bcdtmWrite_toFileDtmObject(voidsP,L"resolvedvoids.dat")) goto errexit ;
/*
** Clean Up
*/
 cleanup :
 DTM_NORMALISE_OPTION = TRUE ;
 DTM_DUPLICATE_OPTION = TRUE ;
 if( p3dPtsP   != NULL ) free(p3dPtsP) ;
 if( voidLineP != NULL ) free(voidLineP) ;
 if( dtmP      != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ;
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Resolving Voids Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Resolving Voids Error") ;
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
BENTLEYDTM_Public int bcdtmSite_resolveIslandsDtmObject(BC_DTM_OBJ *islandP )
/*
** This Function Resolves Intersecting Islands
*/
{
 int      ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long     sp,np,hp,ss,spf,npf,offset,clPtr ;
 long     dtmFeature,numStartFeatures,voidBoundary ;
 long     numP3dPts,numVoids,numIslands;
 DTMDirection direction;
 double   area ; 
 unsigned char     *islandLineP=NULL ;
 DPoint3d      *p3dPtsP=NULL ;
 BC_DTM_OBJ      *dtmP=NULL ;
 BC_DTM_FEATURE  *dtmFeatureP ; 
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Resolving Islands") ;
    if( bcdtmWrite_toFileDtmObject(islandP,L"unresolvedIslands.dat")) goto errexit ;
   }
/*
** Set Direction Of Voids AntiClockwise
*/
 if( bcdtmClean_setDtmPolygonalFeatureTypeAntiClockwiseDtmObject(islandP,DTMFeatureType::Island)) goto errexit ;
/*
** Create DTM Object
*/
 if( bcdtmObject_createDtmObject(&dtmP)) goto errexit ;
 bcdtmObject_setPointMemoryAllocationParametersDtmObject(dtmP,2*islandP->numPoints,islandP->numPoints) ;
/*
** Scan Islands Object And Write To Data Object As Breaks
*/
 numIslands = 0 ;
 for( dtmFeature = 0 ; dtmFeature < islandP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(islandP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Data && dtmFeatureP->dtmFeatureType == DTMFeatureType::Void )
      {
       if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(islandP,dtmFeature,&p3dPtsP,&numP3dPts)) goto errexit ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(dtmP,DTMFeatureType::Breakline,dtmFeatureP->dtmUserTag,1,&dtmFeatureP->dtmFeatureId,p3dPtsP,numP3dPts)) goto errexit ; 
       ++numIslands ;
      }
   }
/*
** Triangulate 
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Creating Islands Tin") ;
 DTM_NORMALISE_OPTION  = FALSE ;             // To Inhibit Normalisation Of Coordinates - function 
 DTM_DUPLICATE_OPTION = FALSE ;             // To Inhibit Removal Of None Identical Points
 dtmP->ppTol = 0.0 ;
 dtmP->plTol = 0.0 ;  
 if( bcdtmObject_createTinDtmObject(dtmP,1,0.0)) goto errexit ;
 DTM_NORMALISE_OPTION = TRUE ;
 DTM_DUPLICATE_OPTION = TRUE ;
/*
** Remove None Feature Hull Lines
*/
 if(dbg) bcdtmWrite_message(0,0,0,"Removing Non Feature Hull Lines") ;
 if( bcdtmList_removeNoneFeatureHullLinesDtmObject(dtmP)) goto errexit ;
 if( dbg ) bcdtmWrite_toFileDtmObject(dtmP,L"hullIslands.tin") ;
/*
** Mark Internal Island Tin Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Island Lines") ;
 if( bcdtmSite_markInternalFeatureLinesDtmObject(dtmP,&islandLineP)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Island Lines Completed") ;
/*
** Set Offset To Last Feature In Tin
*/
 numStartFeatures = dtmP->numFeatures ;
/*
** Scan Tin Hull To Get External Islands
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For External Islands") ;
 numIslands = 0 ;
 sp = dtmP->hullPoint ;
 do
   {
    np = nodeAddrP(dtmP,sp)->hPtr ;
/*
**  Only Proces If Point Has Not Been Already Included In A Void
*/
    if( nodeAddrP(dtmP,sp)->sPtr == dtmP->nullPnt )
      { 
       if( bcdtmList_testForBreakLineDtmObject(dtmP,sp,np))
         {
/*
**        Scan Around External Edge Of Break Lines
*/
          hp = sp ;
          nodeAddrP(dtmP,sp)->tPtr = np ;
          nodeAddrP(dtmP,sp)->sPtr = np ;
          do
            { 
             if( ( hp = bcdtmList_nextAntDtmObject(dtmP,np,hp)) < 0 ) goto errexit ;
             while ( ! bcdtmList_testForBreakLineDtmObject(dtmP,np,hp))
               {
                if( ( hp = bcdtmList_nextAntDtmObject(dtmP,np,hp)) < 0 ) goto errexit ;
               }
             nodeAddrP(dtmP,np)->tPtr = hp ;
             nodeAddrP(dtmP,np)->sPtr = hp ;
             ss = hp ;
             hp = np ;
             np = ss ;
            } while ( hp != sp ) ;
/*
**        Store Island Feature In Tin
*/
          if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,DTMFeatureType::Island,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,sp,1)) goto errexit ; 
          ++numIslands ;
         }
      }
    sp = np ; 
   } while ( sp != dtmP->hullPoint ) ;
/*
** Scan For Internal Internal Voids
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For Internal Voids") ;
 numVoids = 0 ;
 for( sp = 0 ; sp < dtmP->numPoints ; ++sp )
   {
    if( nodeAddrP(dtmP,sp)->sPtr == dtmP->nullPnt ) 
      {
       clPtr = nodeAddrP(dtmP,sp)->cPtr ;
       voidBoundary = FALSE ;
       while ( clPtr != dtmP->nullPtr && voidBoundary == FALSE )
         {
          np    = clistAddrP(dtmP,clPtr)->pntNum ; 
          clPtr = clistAddrP(dtmP,clPtr)->nextPtr ; 
          if( np > sp && nodeAddrP(dtmP,sp)->hPtr != np && nodeAddrP(dtmP,np)->hPtr != sp )
            {
/*
**           Check For Void Boundary Line
*/
             spf = npf = 0 ;
             if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,sp,np)) goto errexit ;
             if( bcdtmFlag_testFlag(islandLineP,offset) ) spf = 1 ;
             if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,np,sp)) goto errexit ;
             if( bcdtmFlag_testFlag(islandLineP,offset) ) npf = 1 ;
             if( ( spf == 1 && npf == 0 ) || ( spf == 0 && npf == 1 )) voidBoundary = TRUE ;
/*
**           Island Boundary Line Detected
*/
             if( voidBoundary == TRUE ) 
               {
/*
**              Scan Clockwise Around Void Boundary Starting From Sp
*/ 
                if( npf == 1 )
                  {
                   hp = sp ;
                   nodeAddrP(dtmP,sp)->tPtr = np ;
                   nodeAddrP(dtmP,sp)->sPtr = np ;
                   do
                     { 
                      if( ( hp = bcdtmList_nextClkDtmObject(dtmP,np,hp)) < 0 ) goto errexit ;
                      while ( ! bcdtmList_testForBreakLineDtmObject(dtmP,np,hp))
                        {
                         if( ( hp = bcdtmList_nextClkDtmObject(dtmP,np,hp)) < 0 ) goto errexit ;
                        }
                      nodeAddrP(dtmP,np)->tPtr = hp ;
                      nodeAddrP(dtmP,np)->sPtr = hp ;
                      ss = hp ;
                      hp = np ;
                      np = ss ;
                     } while ( hp != sp ) ;
                  }  
/*
**              Scan  Clockwise Around Void Boundary Starting From Np
*/ 
                if( spf == 1 ) 
                  {
                   hp = np ;
                   nodeAddrP(dtmP,np)->tPtr = sp ;
                   nodeAddrP(dtmP,np)->sPtr = sp ;
                   do
                     { 
                      if( ( hp = bcdtmList_nextClkDtmObject(dtmP,sp,hp)) < 0 ) goto errexit ;
                      while ( ! bcdtmList_testForBreakLineDtmObject(dtmP,sp,hp))
                        {
                         if( ( hp = bcdtmList_nextClkDtmObject(dtmP,sp,hp)) < 0 ) goto errexit ;
                        }
                      nodeAddrP(dtmP,sp)->tPtr = hp ;
                      nodeAddrP(dtmP,sp)->sPtr = hp ;
                      ss = hp ;
                      hp = sp ;
                      sp = ss ;
                     } while ( hp != np ) ;
                  }
/*
**              False Void If Direction Is Clockwise
*/
                bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,sp,&area,&direction) ;
                if( direction == DTMDirection::AntiClockwise )
                  {
                   if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,DTMFeatureType::Void,DTM_NULL_USER_TAG,DTM_NULL_FEATURE_ID,sp,1)) goto errexit ; 
                   ++numVoids ;
                  }
                else bcdtmList_nullTptrListDtmObject(dtmP,sp) ; 
               }
            }
         }
      }
   }
/*
** Write Number Of Resolved Polygons
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Islands = %6ld",numVoids) ;
    bcdtmWrite_message(0,0,0,"Number Of Voids   = %6ld",numIslands) ;
   }
/*
** Initialise Islands Object - Remove All Data
*/
 bcdtmObject_initialiseDtmObject(islandP) ;
/*
** Copy Features From Tin To Voids Object
*/
 numVoids      = 0 ;
 numIslands    = 0 ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(islandP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void || dtmFeatureP->dtmFeatureType == DTMFeatureType::Island )
      {
       if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&p3dPtsP,&numP3dPts)) goto errexit ;
       if( bcdtmObject_storeDtmFeatureInDtmObject(islandP,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmUserTag,1,&dtmFeatureP->dtmFeatureId,p3dPtsP,numP3dPts)) goto errexit ; 
       if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Void   ) ++numVoids   ;
       if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Island ) ++numIslands ;
      }
   }
/*
** Write Number Of Resolved Polygons
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Islands  Written = %6ld",numIslands) ;
    bcdtmWrite_message(0,0,0,"Number Of Voids    Written = %6ld",numVoids) ;
   }
/*
** Write Resolved Islands 
*/
 if( dbg ) if( bcdtmWrite_toFileDtmObject(islandP,L"resolvedIslands.dat")) goto errexit ;
/*
** Clean Up
*/
 cleanup :
 DTM_NORMALISE_OPTION = TRUE ;
 DTM_DUPLICATE_OPTION = TRUE ;
 if( p3dPtsP     != NULL ) free(p3dPtsP) ;
 if( islandLineP != NULL ) free(islandLineP) ;
 if( dtmP        != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ;
/*
** Job Completed
*/
 if( dbg && ! ret ) bcdtmWrite_message(0,0,0,"Resolving Islands Completed") ;
 if( dbg &&   ret ) bcdtmWrite_message(0,0,0,"Resolving Islands Error") ;
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
BENTLEYDTM_Private int bcdtmSite_markInternalFeatureLinesDtmObject(BC_DTM_OBJ *dtmP,unsigned char **featureLinePP)
/*
** This Function Marks All Internal Feature Lines
** This Is Not A Generic Function And Is For DTM Internal Use Only
** Assumes All Tin Features Are Closed
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long cp,sp,np,pp,lp,hp,clPtr,offset,dtmFeature,startPoint,mark=-989898,numMarked ;
 long count,count0,count1,count2 ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Feature Lines") ;
/*
** Allocate Memory For Markers
*/
 if( *featureLinePP != NULL ) free(*featureLinePP) ;
 *featureLinePP = ( unsigned char * ) malloc ( (dtmP->cListPtr/8+1) * sizeof(char)) ;
 if( *featureLinePP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ; 
   }
/*
** Initialise Markers
*/
 memset(*featureLinePP,0,(dtmP->cListPtr/8+1)) ;
/*
** Scan Features And Mark Tin Lines Internal To Features
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Tin Features") ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Feature %6ld of %6ld",dtmFeature+1,dtmP->numFeatures) ;
    if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,dtmFeature,&startPoint)) goto errexit ;
/*
** Get Lowest And Highest Numbered Points On Tptr Polygon
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Getting Low And High Points") ; 
    lp = hp = sp = startPoint ;
    do
      {
       sp = nodeAddrP(dtmP,sp)->tPtr ;
        if( sp < lp ) lp = sp ;
        if( sp > hp ) hp = sp ;
      } while ( sp != startPoint ) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"lp = %8ld  hp = %8ld",lp,hp) ;
/*
**  Mark Internal Tptr Polygon Points
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Tptr Polygon Points") ; 
    if( bcdtmMark_internalTptrPolygonPointsDtmObject(dtmP,startPoint,mark,&numMarked ) ) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Marked = %6ld",numMarked) ;
/*
**  Mark Internal Lines Connected To Marked Points
*/
    if( numMarked > 0 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Marking Internal Lines Connected To Marked Points") ; 
       for( sp = lp ; sp != hp  ; ++sp )
         { 
          if( nodeAddrP(dtmP,sp)->tPtr == mark )
            {
             clPtr = nodeAddrP(dtmP,sp)->cPtr ;
             while( clPtr != dtmP->nullPtr )
                {
                np    = clistAddrP(dtmP,clPtr)->pntNum ;
                  clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
                if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,sp,np)) goto errexit ;
                bcdtmFlag_setFlag(*featureLinePP,offset) ;
                if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,np,sp)) goto errexit ;
                bcdtmFlag_setFlag(*featureLinePP,offset) ;
               }
            }
         }
      }
/*
**  Mark Lines On Internal Edge Of Tptr Polygon Not Connected To A Marked Point
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Lines Internal To Tptr Polygon") ; 
    pp = startPoint ; 
    sp = nodeAddrP(dtmP,pp)->tPtr ;
    np = nodeAddrP(dtmP,sp)->tPtr ;
    do
      {
       cp = np ;
       if(( cp = bcdtmList_nextAntDtmObject(dtmP,sp,cp)) < 0 ) goto errexit ;
       while ( cp != pp )
         {
          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,sp,cp)) goto errexit ;
          bcdtmFlag_setFlag(*featureLinePP,offset) ;
          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,cp,sp)) goto errexit ;
          bcdtmFlag_setFlag(*featureLinePP,offset) ;
          if(( cp = bcdtmList_nextAntDtmObject(dtmP,sp,cp)) < 0 ) goto errexit ;
         }
       pp = sp ;
       sp = np ;
       np = nodeAddrP(dtmP,sp)->tPtr  ;
      } while ( pp != startPoint ) ;
/*
**  Mark Tptr Polygon Edge Lines
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Tptr Polygon Lines") ; 
    sp = startPoint ; 
    do
      {
       np = nodeAddrP(dtmP,sp)->tPtr ;
       if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,sp,np)) goto errexit ;
       bcdtmFlag_setFlag(*featureLinePP,offset) ;
       sp = np ; 
      }  while ( sp != startPoint ) ;
/*
**  Un Mark Marked Points
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Unmarking Marked Points") ; 
    for( sp = lp ; sp <= hp ; ++sp )
      {
       if( nodeAddrP(dtmP,sp)->tPtr == mark ) nodeAddrP(dtmP,sp)->tPtr = dtmP->nullPnt ;
      }
/*
**  Null Out Tptr Polygon
*/
    if( bcdtmList_nullTptrListDtmObject(dtmP,startPoint)) goto errexit ; 
   }
/*
** Write Diagnostics
*/
 if( dbg )
   {
    count0 = count1 = count2 = 0 ;
    for( sp = 0 ; sp < dtmP->numPoints ; ++sp )
      {
       clPtr = nodeAddrP(dtmP,sp)->cPtr ;
       while( clPtr != dtmP->nullPtr )
         {
          np    = clistAddrP(dtmP,clPtr)->pntNum ;
          clPtr = clistAddrP(dtmP,clPtr)->nextPtr ; 
          if( np > sp && nodeAddrP(dtmP,sp)->hPtr != np && nodeAddrP(dtmP,np)->hPtr != sp )
            {
             count=0 ;
             if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,sp,np)) goto errexit ;
             if( bcdtmFlag_testFlag(*featureLinePP,offset) ) ++count ;
             if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,np,sp)) goto errexit ;
             if( bcdtmFlag_testFlag(*featureLinePP,offset) ) ++count ;
             if( count == 0 )  ++count0 ;
             if( count == 1 )  ++count1 ;
             if( count == 2 )  ++count2 ;
            } 
         }
      }
    bcdtmWrite_message(0,0,0,"Number With Zero Count = %6ld",count0) ;
    bcdtmWrite_message(0,0,0,"Number With One  Count = %6ld",count1) ;
    bcdtmWrite_message(0,0,0,"Number With Two  Count = %6ld",count2) ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Internal Feature Lines Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Internal Feature Lines Error") ;
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
BENTLEYDTM_Public int bcdtmSite_resolveVoidsWithinIslandsDtmObject
(
 BC_DTM_OBJ *dataP,               /* ==> Object With Islands And Voids                    */
 BC_DTM_OBJ *voidsP               /* ==> Append Resolved Islands And Voids To This Object */
) 
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long    islandFeature,voidFeature,numIslandPts,numVoidPts,intResult,numInternalVoids ;
 DPoint3d     *p3dP,*islandPtsP=NULL,*voidPtsP=NULL ;
 double  ixMin,iyMin,ixMax,iyMax,vxMin,vyMin,vxMax,vyMax ;
 DTM_POLYGON_OBJ *polyP=NULL ; 
 BC_DTM_OBJ      *voidDataP=NULL ;
 BC_DTM_FEATURE  *islandFeatureP,*voidFeatureP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Resolving Voids Within Islands") ;
 if( dbg == 2 ) if( bcdtmWrite_toFileDtmObject(voidsP,L"unresolvedVoidsWithinIslands.dat")) goto errexit ;
/*
** Scan Object For Islands 
*/
 for( islandFeature = 0 ; islandFeature < dataP->numFeatures ; ++islandFeature )
   {
    islandFeatureP = ftableAddrP(dataP,islandFeature) ;
    if( islandFeatureP->dtmFeatureState == DTMFeatureState::Data && islandFeatureP->dtmFeatureType == DTMFeatureType::Island )
      {
       if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dataP,islandFeature,&islandPtsP,&numIslandPts)) goto errexit ;
/*
**     Get Bounding Rectangle For Island Feature
*/
       ixMin = ixMax = islandPtsP->x ;
       iyMin = iyMax = islandPtsP->y ;
       for( p3dP = islandPtsP ; p3dP <= islandPtsP + numIslandPts ; ++p3dP ) 
         { 
          if( p3dP->x < ixMin ) ixMin = p3dP->x ;
          if( p3dP->x > ixMin ) ixMin = p3dP->x ;
          if( p3dP->y < iyMin ) iyMin = p3dP->y ;
          if( p3dP->y > iyMax ) iyMax = p3dP->y ;
         }
/*
**     Set Monitor Flag For Internal Voids 
*/
       numInternalVoids = 0 ;
/*
**     Scan For voidsP
*/
       for( voidFeature = 0 ; voidFeature < dataP->numFeatures ; ++voidFeature )
         {
          voidFeatureP = ftableAddrP(dataP,voidFeature) ;
          if( voidFeatureP->dtmFeatureState == DTMFeatureState::Data && voidFeatureP->dtmFeatureType == DTMFeatureType::Void )
            {
             if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dataP,voidFeature,&voidPtsP,&numVoidPts)) goto errexit ;
/*
**           Get Bounding Rectangle For Void Feature
*/
             vxMin = vxMax = voidPtsP->x ;
             vyMin = vyMax = voidPtsP->y ;
             for( p3dP = voidPtsP  ; p3dP <= voidPtsP + numVoidPts ; ++p3dP ) 
               {
                if( p3dP->x < vxMin ) vxMin = p3dP->x ;
                if( p3dP->x > vxMin ) vxMin = p3dP->x ;
                if( p3dP->y < vyMin ) vyMin = p3dP->y ;
                if( p3dP->y > vyMax ) vyMax = p3dP->y ;
               }
/*
**           Check For Overlap Of Bounding Cubes
*/
             if( vxMax >= ixMin && vxMin <= ixMax && vyMax >= iyMin && vyMin <= iyMax )
               {
/*
**              Test For Void Internal To Island
*/
                if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting 3D Polygons") ;
                if( bcdtmPolygon_intersectPointArrayPolygons(islandPtsP,numIslandPts,voidPtsP,numVoidPts,&intResult,&polyP,0.000001,0.000001)) goto errexit ;
                if( polyP != NULL ) bcdtmPolygon_deletePolygonObject(&polyP) ;
                if( dbg ) bcdtmWrite_message(0,0,0,"Intersecting 3D Polygons Completed") ;
/*
**              If Void Polygon Is Inside Island Polygon Copy Void Polygon To Data Object
*/
                if( intResult == 3 )
                  {
                   ++numInternalVoids ; 
                   if( voidDataP == NULL ) 
                     {
                      if( bcdtmObject_createDtmObject(&voidDataP)) goto errexit ;
                      bcdtmObject_setPointMemoryAllocationParametersDtmObject(voidDataP,numVoidPts+numIslandPts,numVoidPts+numIslandPts) ;
                     }
                   if( bcdtmObject_storeDtmFeatureInDtmObject(voidDataP,DTMFeatureType::Void,voidDataP->nullUserTag,1,&voidDataP->nullFeatureId,voidPtsP,numVoidPts)) goto errexit ;
                   free(voidPtsP) ; voidPtsP = NULL ;
                  }             
               }
            }
         }
/*
**     Resolve Intersecting Voids
*/
       if( numInternalVoids )
         {  
          if( dbg ) bcdtmWrite_message(0,0,0,"Resolving %2ld Voids Internal To Island",numInternalVoids) ;
          if( numInternalVoids > 1 ) if( bcdtmSite_resolveVoidsDtmObject(voidDataP) ) goto errexit ;
          if( bcdtmObject_appendDtmObject(dataP,voidDataP)) goto errexit ; 
          bcdtmObject_initialiseDtmObject(voidDataP) ;
         }
/*
**     Free Island Points Memory
*/
       free(islandPtsP) ;
       islandPtsP = NULL ; 
      }
   } 
/*
** Clean Up
*/
 cleanup :
 if( islandPtsP != NULL ) { free(islandPtsP) ; islandPtsP = NULL ; }
 if( voidPtsP   != NULL ) { free(voidPtsP)   ; voidPtsP   = NULL ; }
 if( polyP      != NULL ) bcdtmPolygon_deletePolygonObject(&polyP) ;
 if( voidDataP  != NULL ) bcdtmObject_destroyDtmObject(&voidDataP) ;
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Resolving Voids Within Islands Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Resolving Voids Within Islands Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}

/*==============================================================================*//**
* @memo   Gets The Surrounding Hull For A Set Of Break Lines
* @doc    Gets The Surrounding Hull For A Set Of Break Lines
* @notes  Assumes There Are Only Break Lines In The Data Object
* @param  dataP          ==> Pointer To Data Object      
* @param  hullPtsPP     <==  Pointer To Point Array   
* @param  numHullPtsP   <==  Number Of Points In Point Array 
* @author Rob Cormack 1 April 2004 rob.cormack@bentley.com
* @return DTM_SUCCESS or DTM_ERROR
* @version 
* @see None
*===============================================================================*/
BENTLEYDTM_EXPORT int bcdtmSite_getHullSurroundingBreakLinesDtmObject
(
 BC_DTM_OBJ *dataP,           /*  ==> Pointer To Dtm Object          */
 DPoint3d        **hullPtsPP,      /* <==  Pointer To Point Array          */
 long       *numHullPtsP      /* <==  Number Of Points In Point Array */
)
{
 int        ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 BC_DTM_OBJ *tempP=NULL ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting Hull Surrounding Break Lines") ;
/*
** Initialise
*/
 *numHullPtsP = 0 ;
 if( *hullPtsPP != NULL ) { free(*hullPtsPP) ; *hullPtsPP = NULL ; }
/*
** Test For Valid Data Object
*/
 if( bcdtmObject_testForValidDtmObject(dataP)) goto errexit ;
/*
** Test For Untriangulated DTM
*/
 if( dataP->dtmState != DTMState::Data )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Untriangulated Dtm") ;
    goto errexit ;
   } 
/*
** Make A Local Copy Of The Dtm Object
*/
 if( bcdtmObject_cloneDtmObject(dataP,&tempP)) goto errexit ;
/*
** Triangulate The Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating Dtm Object") ;
 if( bcdtmObject_createTinDtmObject(tempP,1,0.0) ) goto errexit ;
/*
** Modify Tin Hull To Break Lines
*/ 
 if( bcdtmSite_modifyTinHullToBreakLinesDtmObject(tempP) ) goto errexit ;
/*
** Extract Tin Hull
*/
 if( bcdtmList_extractHullDtmObject(tempP,hullPtsPP,numHullPtsP) ) goto errexit ;
/*
** Cleanup
*/
 cleanup :
 if( tempP != NULL ) bcdtmObject_destroyDtmObject(&tempP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Hull Surrounding Break Lines Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting Hull Surrounding Break Lines Error") ;
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
BENTLEYDTM_Private int bcdtmSite_modifyTinHullToBreakLinesDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Modifies Tin Hull To Break Lines 
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  p1,p2,p3=0,p4,clc,closestBreakPoint,numBreakLines,dtmFeature ;
 double pointDistance,closestDistance=0.0 ;
/*
** Null Out Sptr Values
*/
 bcdtmList_reportAndSetToNullNoneNullSptrValuesDtmObject(dtmP,0) ;
/*
** Mark Break Line End Points
*/
 for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
   {
    if( ( clc = nodeAddrP(dtmP,p1)->cPtr ) != dtmP->nullPtr )
      {
       numBreakLines = 0 ;
       while ( clc != dtmP->nullPtr )
         {
          p2  = clistAddrP(dtmP,clc)->pntNum ;
          clc = clistAddrP(dtmP,clc)->nextPtr ;
          if( bcdtmList_testForBreakLineDtmObject(dtmP,p1,p2)) 
            {
             p3 = p2 ;
             ++numBreakLines ;
            }
         }
       if( numBreakLines == 1 ) nodeAddrP(dtmP,p1)->sPtr = p3 ;
      }
   }
/*
** Write Out Break End Points
*/
 if( dbg )
   {
    for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
      {
       if( ( nodeAddrP(dtmP,p1)->sPtr ) != dtmP->nullPnt )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"End Break Point[%6ld] = %10.4lf %10.4lf %10.4lf",p1,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z);
         }
      }  
   }    
/*
** Connect Break Point End To Closest Break Point
*/
 dtmP->incFeatures = 20 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Connecting To Closest Break Point") ;
 for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
   {
    if( ( nodeAddrP(dtmP,p1)->sPtr ) != dtmP->nullPnt )
      { 
       if( dbg ) bcdtmWrite_message(0,0,0,"End Break Point[%6ld] = %10.4lf %10.4lf %10.4lf",p1,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z);
       closestBreakPoint = dtmP->nullPnt ;
       clc = nodeAddrP(dtmP,p1)->cPtr  ;
       while ( clc != dtmP->nullPtr )
         {
          p2  = clistAddrP(dtmP,clc)->pntNum ;
          clc = clistAddrP(dtmP,clc)->nextPtr ;
          if( p2 != nodeAddrP(dtmP,p1)->sPtr  )
            {
             if( bcdtmList_testForPointOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Breakline,p2,&dtmFeature) )
               {
                pointDistance = bcdtmMath_pointDistanceDtmObject(dtmP,p1,p2) ;
                if( closestBreakPoint == dtmP->nullPnt || pointDistance < closestDistance )
                  {
                   closestDistance   = pointDistance ;
                   closestBreakPoint = p2 ;
                  }
               }
            } 
         }
/*
**    Insert Break Line
*/
       if( closestBreakPoint != dtmP->nullPnt )
         {
          if( dbg ) bcdtmWrite_message(0,0,0,"Closest Break Point[%6ld] = %10.4lf %10.4lf %10.4lf",closestBreakPoint,pointAddrP(dtmP,closestBreakPoint)->x,pointAddrP(dtmP,closestBreakPoint)->y,pointAddrP(dtmP,closestBreakPoint)->z) ;
          nodeAddrP(dtmP,p1)->tPtr = closestBreakPoint ;
          if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,DTMFeatureType::Breakline,DTM_NULL_USER_TAG,dtmP->dtmFeatureIndex,p1,1)) goto errexit ;
          ++dtmP->dtmFeatureIndex ;
         }
      }
   }
/*
** Scan Tin Hull And Look For Gaps and Connect To Closest break Point
*/
 p1 = dtmP->hullPoint ;
 do
   {
    p2 = nodeAddrP(dtmP,p1)->hPtr ;
/*
** Only Process If P1-P2 Not A Break Line
*/
    if( ! bcdtmList_testForBreakLineDtmObject(dtmP,p1,p2)) 
      {
/*
**     Only Process If P1 And P2 Are End Points
*/
       if( nodeAddrP(dtmP,p1)->sPtr != dtmP->nullPnt && nodeAddrP(dtmP,p2)->sPtr != dtmP->nullPnt )
         {
          if( ( p3 = bcdtmList_nextAntDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ;
/*
**        Only Process If P3 Is And End Point
*/
          if( nodeAddrP(dtmP,p3)->sPtr != dtmP->nullPnt )      
            {
             closestBreakPoint = dtmP->nullPnt ;
/*
**          Scan Clockwise Looking For Closest Break Point
*/
             if     ( bcdtmList_testForBreakLineDtmObject(dtmP,p3,p2) && ! bcdtmList_testForBreakLineDtmObject(dtmP,p3,p1)) 
               {
                if( ( p4 = bcdtmList_nextClkDtmObject(dtmP,p3,p2)) < 0 ) goto errexit ;
                while ( p4 != nodeAddrP(dtmP,p3)->sPtr )
                  {
                   if( bcdtmList_testForPointOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Breakline,p4,&dtmFeature) )
                     {
                      pointDistance = bcdtmMath_pointDistanceDtmObject(dtmP,p3,p4) ;
                      if( closestBreakPoint == dtmP->nullPnt || pointDistance < closestDistance )
                        {
                         closestDistance   = pointDistance ;
                         closestBreakPoint = p4 ;
                        }
                     }
                   if( ( p4 = bcdtmList_nextClkDtmObject(dtmP,p3,p4)) < 0 ) goto errexit ;
                  }
               }  
/*
**           Scan Anti Clockwise Looking For Closest Break Point
*/
             else if( bcdtmList_testForBreakLineDtmObject(dtmP,p3,p1) && ! bcdtmList_testForBreakLineDtmObject(dtmP,p3,p2)) 
               {
                if( ( p4 = bcdtmList_nextAntDtmObject(dtmP,p3,p1)) < 0 ) goto errexit ;
                while ( p4 != nodeAddrP(dtmP,p3)->sPtr )
                  {
                   if( bcdtmList_testForPointOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Breakline,p4,&dtmFeature) )
                     {
                      pointDistance = bcdtmMath_pointDistanceDtmObject(dtmP,p3,p4) ;
                      if( closestBreakPoint == dtmP->nullPnt || pointDistance < closestDistance )
                        {
                         closestDistance   = pointDistance ;
                         closestBreakPoint = p4 ;
                        }
                     }
                   if( ( p4 = bcdtmList_nextAntDtmObject(dtmP,p3,p4)) < 0 ) goto errexit ;
                  }
               } 
/*
**           Insert Break Line
*/
             if( closestBreakPoint != dtmP->nullPnt )
               {
                if( dbg ) bcdtmWrite_message(0,0,0,"Closest Break Point[%6ld] = %10.4lf %10.4lf %10.4lf",closestBreakPoint,pointAddrP(dtmP,closestBreakPoint)->x,pointAddrP(dtmP,closestBreakPoint)->y,pointAddrP(dtmP,closestBreakPoint)->z) ;
                nodeAddrP(dtmP,p3)->tPtr = closestBreakPoint ;
                if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,DTMFeatureType::Breakline,DTM_NULL_USER_TAG,dtmP->dtmFeatureIndex,p3,1)) goto errexit ;
                ++dtmP->dtmFeatureIndex ;
               }
            }
         }
      } 
    p1 = p2 ;
   } while ( p1 != dtmP->hullPoint ) ;
/*
** Write Tin With The Added Break Lines
*/
 if( dbg ) bcdtmWrite_toFileDtmObject(dtmP,L"afterBreak.tin") ;
/*
** Remove Non Feature Hull Lines
*/
 if( bcdtmList_removeNoneFeatureHullLinesDtmObject(dtmP) ) goto errexit ;
/*
** Clean Tin Object
*/
 if( bcdtmList_cleanDtmObject(dtmP)) goto errexit ;
/*
** Cleanup
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
BENTLEYDTM_EXPORT int bcdtmSite_cleanBreakTopologyDtmObject(BC_DTM_OBJ *dataP,double snapTolerance,DTM_FEATURE **featurePP,long *numFeatureP)
{
 int        ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 BC_DTM_OBJ *dtmP=NULL ; 
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Cleaning Break Topology") ;
/*
** Test For Valid Data Object
*/
 if( bcdtmObject_testForValidDtmObject(dataP)) goto errexit ;
/*
** Test For Untriangulated DTM
*/
 if( dataP->dtmState != DTMState::Data )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Untriangulated Dtm") ;
    goto errexit ;
   } 
/*
** Make A Local Copy Of The Dtm Object
*/
 if( bcdtmObject_cloneDtmObject(dataP,&dtmP)) goto errexit ;
/*
** Triangulate The Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Triangulating Dtm Object") ;
 if( bcdtmObject_createTinDtmObject(dtmP,1,0.0) ) goto errexit ;
/*
** Retriangulate Along Break Lines
*/
 if( bcdtmClean_retriangualteBreakLinesDtmObject(dtmP) ) goto errexit ;
/*
** Clean Break Topology
*/ 
 if( dbg ) bcdtmWrite_message(0,0,0,"Cleaning Tin Break Topology") ;
 if( bcdtmClean_breakTopologyDtmObject(dtmP,snapTolerance)) goto errexit ;
/*
** Write Cleaned Breaks To Feature Array
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Writing Cleaned Tin Breaks To Feature Array") ;
 if( bcdtmSite_writeAllDtmFeatureTypeToDtmFeatureArrayDtmObject(dtmP,DTMFeatureType::Breakline,featurePP,numFeatureP)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
 if( dtmP != NULL ) bcdtmObject_destroyDtmObject(&dtmP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Cleaning Break Topology Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Cleaning Break Topology Error") ;
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
BENTLEYDTM_EXPORT int bcdtmSite_snapToClosestBreakLineDtmObject
(
 BC_DTM_OBJ   *dtmP,                     /* ==> Pointer To Tin Object                    */
 double        xPnt,                      /* ==> x Coordiante Of Point                    */
 double        yPnt,                      /* ==> y Coordiante Of Point                    */
 long          *snapFoundP,               /* <== Snap Found <TRUE,FALSE>                  */
 long          *snapTypeP,                /* <== Snap Type <1=Break Point,2=Break Line>   */
 double        *xSnapP,                   /* <== x Coordiante Of Snap Point               */  
 double        *ySnapP,                   /* <== y Coordiante Of Snap Point               */  
 double        *zSnapP,                   /* <== y Coordiante Of Snap Point               */  
 double        *dSnapP,                   /* <== Distance To Snap Point                   */
 long          *dtmPnt1P,                 /* <== Tin Point Of Break Line End              */ 
 long          *dtmPnt2P,                 /* <== Tin Point Of Break Line End              */ 
 DTM_TIN_POINT_FEATURES **snapFeaturesPP, /* <== Pointer To Dtm Features At Snap Point    */
 long          *numSnapFeaturesP          /* <== Number Of Dtm Features At Snap Point     */
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ; 
 long   dtmPnt1,dtmPnt2,dtmPnt3,onLine,findType,dtmFeature ;
 double x,y,d1,d2,zPnt,snapDistance  ;
 DTM_TIN_POINT_FEATURES *fListP ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Snapping To Closest Break Line") ;
    bcdtmWrite_message(0,0,0,"xPnt = %20.10lf",xPnt) ;
    bcdtmWrite_message(0,0,0,"yPnt = %20.10lf",yPnt) ;
   }
/*
** Initialise
*/
 *snapFoundP = FALSE ;
 *snapTypeP  = 0   ;
 *xSnapP     = 0.0 ;
 *ySnapP     = 0.0 ;
 *zSnapP     = 0.0 ;
 *dSnapP     = 0.0 ;
 *dtmPnt1P   = dtmP->nullPnt ;
 *dtmPnt2P   = dtmP->nullPnt ;
 if( *snapFeaturesPP != NULL ) { free(*snapFeaturesPP) ; *snapFeaturesPP = NULL ; }
 *numSnapFeaturesP = 0 ;
/*
** Validate Tin Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP) != DTM_SUCCESS ) goto errexit ;
/*
** Find Triangle For Point
*/
 if( bcdtmFind_triangleDtmObject(dtmP,xPnt,yPnt,&findType,&dtmPnt1,&dtmPnt2,&dtmPnt3)) goto errexit ;
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"internal findType = %2ld",findType) ;
    if( findType != 0 )
      {
       bcdtmWrite_message(0,0,0,"dtmPnt1 = %8ld ** %12.5lf %12.5lf %10.4lf",dtmPnt1,pointAddrP(dtmP,dtmPnt1)->x,pointAddrP(dtmP,dtmPnt1)->y,pointAddrP(dtmP,dtmPnt1)->z ) ;
       if( dtmPnt2 != dtmP->nullPnt) bcdtmWrite_message(0,0,0,"dtmPnt2 = %8ld ** %12.5lf %12.5lf %10.4lf",dtmPnt2,pointAddrP(dtmP,dtmPnt2)->x,pointAddrP(dtmP,dtmPnt2)->y,pointAddrP(dtmP,dtmPnt2)->z ) ;
       if( dtmPnt3 != dtmP->nullPnt) bcdtmWrite_message(0,0,0,"dtmPnt3 = %8ld ** %12.5lf %12.5lf %10.4lf",dtmPnt3,pointAddrP(dtmP,dtmPnt3)->x,pointAddrP(dtmP,dtmPnt3)->y,pointAddrP(dtmP,dtmPnt3)->z ) ;
      }
   }
/*
**  Test For Closest Point On Break Line
*/
 if( findType == 1 )
   {
    if( bcdtmList_testForPointOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Breakline,dtmPnt1,&dtmFeature))
      {
       snapDistance = bcdtmMath_distance(xPnt,yPnt,pointAddrP(dtmP,dtmPnt1)->x,pointAddrP(dtmP,dtmPnt1)->y) ;
       *snapFoundP = TRUE ;
       *xSnapP = pointAddrP(dtmP,dtmPnt1)->x ;
       *ySnapP = pointAddrP(dtmP,dtmPnt1)->y ;
       *zSnapP = pointAddrP(dtmP,dtmPnt1)->z ;
       *dSnapP = snapDistance ;
       *snapTypeP = 1 ;
       *dtmPnt1P  = dtmPnt1 ;
      }
   }
/*
**  Test For Break Line Triangle Lines
*/
 if( findType == 2 )
   {
/*
**  Test If dtmPnt1-dtmPnt2 Is A Break Line
*/
    if( bcdtmList_testForBreakLineDtmObject(dtmP,dtmPnt1,dtmPnt2))
      {
       snapDistance = bcdtmMath_distanceOfPointFromLine(&onLine,pointAddrP(dtmP,dtmPnt1)->x,pointAddrP(dtmP,dtmPnt1)->y,pointAddrP(dtmP,dtmPnt2)->x,pointAddrP(dtmP,dtmPnt2)->y,xPnt,yPnt,&x,&y) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"snapDistance dtmPnt1-dtmPnt2 = %20.15lf **  xSnap = %12.5lf ySnap = %12.5lf",snapDistance,x,y) ;     
       if( onLine && ( *snapFoundP == FALSE || ( *snapFoundP == TRUE && snapDistance < *dSnapP )))
         {
          bcdtmMath_interpolatePointOnLineDtmObject(dtmP,x,y,&zPnt,dtmPnt1,dtmPnt2) ;
          *snapFoundP = TRUE ;
          *xSnapP = x ;
          *ySnapP = y ;
          *zSnapP = zPnt ;
          *dSnapP = snapDistance ;
          *snapTypeP = 2 ;
          *dtmPnt1P  = dtmPnt1 ;
          *dtmPnt2P  = dtmPnt2 ;
         }
      }
/*
**  Test If dtmPnt2-dtmPnt3 Is A Break Line
*/
    if( bcdtmList_testForBreakLineDtmObject(dtmP,dtmPnt2,dtmPnt3))
      {
       snapDistance = bcdtmMath_distanceOfPointFromLine(&onLine,pointAddrP(dtmP,dtmPnt2)->x,pointAddrP(dtmP,dtmPnt2)->y,pointAddrP(dtmP,dtmPnt3)->x,pointAddrP(dtmP,dtmPnt3)->y,xPnt,yPnt,&x,&y) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"snapDistance dtmPnt2-dtmPnt3 = %20.15lf **  xSnap = %12.5lf ySnap = %12.5lf",snapDistance,x,y) ;     
       if( onLine && ( *snapFoundP == FALSE || ( *snapFoundP == TRUE && snapDistance < *dSnapP )))
         {
          bcdtmMath_interpolatePointOnLineDtmObject(dtmP,x,y,&zPnt,dtmPnt2,dtmPnt3) ;
          *snapFoundP = TRUE ;
          *xSnapP = x ;
          *ySnapP = y ;
          *zSnapP = zPnt ;
          *dSnapP = snapDistance ;
          *snapTypeP = 2 ;
          *dtmPnt1P  = dtmPnt2 ;
          *dtmPnt2P  = dtmPnt3 ;
         }
      }
/*
**  Test If dtmPnt3-dtmPnt1 Is A Break Line
*/
    if( bcdtmList_testForBreakLineDtmObject(dtmP,dtmPnt3,dtmPnt1))
      {
       snapDistance = bcdtmMath_distanceOfPointFromLine(&onLine,pointAddrP(dtmP,dtmPnt3)->x,pointAddrP(dtmP,dtmPnt3)->y,pointAddrP(dtmP,dtmPnt1)->x,pointAddrP(dtmP,dtmPnt1)->y,xPnt,yPnt,&x,&y) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"snapDistance dtmPnt3-dtmPnt1 = %20.15lf **  xSnap = %12.5lf ySnap = %12.5lf",snapDistance,x,y) ;     
       if(  onLine && ( *snapFoundP == FALSE || ( *snapFoundP == TRUE &&  snapDistance < *dSnapP )))
         {
          bcdtmMath_interpolatePointOnLineDtmObject(dtmP,x,y,&zPnt,dtmPnt3,dtmPnt1) ;
          *snapFoundP = TRUE ;
          *xSnapP = x ;
          *ySnapP = y ;
          *zSnapP = zPnt ;
          *dSnapP = snapDistance ;
          *snapTypeP = 2 ;
          *dtmPnt1P  = dtmPnt3 ;
          *dtmPnt2P  = dtmPnt1 ;
         }
      }
   }
/*
** Point External To Tin
*/
 if( findType == 0 )
   { 
/*
**  Find Closest Hull Line
*/
    if( bcdtmFind_findClosestHullLineDtmObject(dtmP,xPnt,yPnt,&zPnt,&findType,&dtmPnt1,&dtmPnt2)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"external findType = %2ld ** dtmPnt1 = %9ld dtmPnt2 = %9ld",findType,dtmPnt1,dtmPnt2) ;
/*
**  Check For Closeness To End Points
*/
    if( findType == 2 )
      {
       d1 = bcdtmMath_distance(xPnt,yPnt,pointAddrP(dtmP,dtmPnt1)->x,pointAddrP(dtmP,dtmPnt1)->y) ;
       d2 = bcdtmMath_distance(xPnt,yPnt,pointAddrP(dtmP,dtmPnt2)->x,pointAddrP(dtmP,dtmPnt2)->y) ;
       if     ( d1 <= d2 && d1 <= dtmP->ppTol )
         {
          findType = 1 ;
          dtmPnt2  = dtmP->nullPnt ;
         }
       else if( d2 <  d1 && d2 <= dtmP->ppTol )
         {
          findType = 1 ;
          dtmPnt1  = dtmPnt2 ;
          dtmPnt2  = dtmP->nullPnt ;
         }
      } 
    if( dbg ) bcdtmWrite_message(0,0,0,"external findType = %2ld ** dtmPnt1 = %9ld dtmPnt2 = %9ld",findType,dtmPnt1,dtmPnt2) ;
/*
**  Point Closest To Hull Point
*/
    if( findType == 1 )
      {
       if( bcdtmList_testForPointOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Breakline,dtmPnt1,&dtmFeature))
         {
          snapDistance = bcdtmMath_distance(xPnt,yPnt,pointAddrP(dtmP,dtmPnt1)->x,pointAddrP(dtmP,dtmPnt1)->y) ;
          *snapFoundP = TRUE ;
          *xSnapP = pointAddrP(dtmP,dtmPnt1)->x ;
          *ySnapP = pointAddrP(dtmP,dtmPnt1)->y ;
          *zSnapP = pointAddrP(dtmP,dtmPnt1)->z ;
          *dSnapP = snapDistance ;
          *snapTypeP = 1 ;
          *dtmPnt1P  = dtmPnt1 ;
         }
      }
/*
**  Point Closest To Hull Line
*/
    if( findType == 2 )
      {
       if( bcdtmList_testForBreakLineDtmObject(dtmP,dtmPnt1,dtmPnt2)) 
         {
          snapDistance = bcdtmMath_distanceOfPointFromLine(&onLine,pointAddrP(dtmP,dtmPnt1)->x,pointAddrP(dtmP,dtmPnt1)->y,pointAddrP(dtmP,dtmPnt2)->x,pointAddrP(dtmP,dtmPnt2)->y,xPnt,yPnt,&x,&y) ;
          bcdtmMath_interpolatePointOnLineDtmObject(dtmP,x,y,&zPnt,dtmPnt1,dtmPnt2) ;
          *snapFoundP = TRUE ;
          *xSnapP = x ;
          *ySnapP = y ;
          *zSnapP = zPnt ;
          *dSnapP = snapDistance ;
          *snapTypeP = 2 ;
          *dtmPnt1P  = dtmPnt1 ;
          *dtmPnt2P  = dtmPnt2 ;
         } 
      }
   }
/*
** Find Closest Orthogonal Break Line To Point
*/
 if( *snapFoundP == FALSE || ( *snapFoundP == TRUE && *dSnapP > dtmP->ppTol ) )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Finding Closest Orthogonal Break Line") ;
    if( bcdtmSite_findClosestOrthogonalBreakLineDtmObject(dtmP,xPnt,yPnt,1,snapFoundP,dtmPnt1P,dtmPnt2P,xSnapP,ySnapP,zSnapP,dSnapP)) goto errexit ;
    if( *snapFoundP )
      {
       *snapTypeP = *snapFoundP ;
       *snapFoundP = TRUE ;
      } 
   }
/*
** Check Distance Of Snap Point To Break Line End Points
*/
 if( *snapFoundP == TRUE && *dtmPnt2P != dtmP->nullPnt )
   {
    d1 = bcdtmMath_distance(xPnt,yPnt,pointAddrP(dtmP,*dtmPnt1P)->x,pointAddrP(dtmP,*dtmPnt1P)->y) ;
    d2 = bcdtmMath_distance(xPnt,yPnt,pointAddrP(dtmP,*dtmPnt2P)->x,pointAddrP(dtmP,*dtmPnt2P)->y) ;
    if( d1 <= d2 && d1 < dtmP->ppTol )
      {
       *dtmPnt2P = dtmP->nullPnt ;
       *xSnapP = pointAddrP(dtmP,dtmPnt1)->x ;
       *ySnapP = pointAddrP(dtmP,dtmPnt1)->y ;
       *zSnapP = pointAddrP(dtmP,dtmPnt1)->z ;
       *dSnapP = d1 ;
       *snapTypeP = 1 ;
      }
    if( d2 <= d1 && d2 < dtmP->ppTol )
      {
       *dtmPnt1P = *dtmPnt2P ;
       *dtmPnt2P = dtmP->nullPnt ;
       *xSnapP = pointAddrP(dtmP,dtmPnt1)->x ;
       *ySnapP = pointAddrP(dtmP,dtmPnt1)->y ;
       *zSnapP = pointAddrP(dtmP,dtmPnt1)->z ;
       *dSnapP = d2 ;
       *snapTypeP = 1 ;
      }
   }
/*
** Get Dtm Features For Snap Point
*/
 if( *snapFoundP == TRUE )
   {
    if( *snapTypeP == 1 ) { if( bcdtmList_getDtmFeaturesForPointDtmObject(dtmP,*dtmPnt1P,snapFeaturesPP,numSnapFeaturesP)) goto errexit ; }
    if( *snapTypeP == 2 ) { if( bcdtmList_getDtmFeaturesForLineDtmObject(dtmP,*dtmPnt1P,*dtmPnt2P,snapFeaturesPP,numSnapFeaturesP)) goto errexit ; } 
   } 
/*
** Write Out Results
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"snapFoundP       = %2ld",*snapFoundP) ;
    bcdtmWrite_message(0,0,0,"snapTypeP        = %2ld",*snapTypeP)  ;
    bcdtmWrite_message(0,0,0,"xSnapP           = %20.10lf",*xSnapP)  ;
    bcdtmWrite_message(0,0,0,"ySnapP           = %20.10lf",*ySnapP)  ;
    bcdtmWrite_message(0,0,0,"zSnapP           = %20.10lf",*zSnapP)  ;
    bcdtmWrite_message(0,0,0,"dSnapP           = %20.15lf",*dSnapP)  ;
    bcdtmWrite_message(0,0,0,"dtmPnt1P         = %9ld",*dtmPnt1P)   ;
    bcdtmWrite_message(0,0,0,"dtmPnt2P         = %9ld",*dtmPnt2P)   ;
    bcdtmWrite_message(0,0,0,"numSnapFeaturesP = %9ld",*numSnapFeaturesP) ;
    for( fListP = *snapFeaturesPP ; fListP < *snapFeaturesPP + *numSnapFeaturesP ; ++fListP )
      {
       bcdtmWrite_message(0,0,0,"snapFeature[%4ld] ** dtmFeature = %8ld ** Type = %4ld ** userTag = %10I64d ** nextPoint = %9ld priorPoint = %9ld",(long)(fListP-*snapFeaturesPP),fListP->dtmFeature,fListP->dtmFeatureType,fListP->userTag,fListP->nextPoint,fListP->priorPoint) ; 
       bcdtmList_writePointsForDtmFeatureDtmObject(dtmP,fListP->dtmFeature) ;
      }
   }

/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Snapping To Closest Break Line Completed") ;
 if( dbg && ret != DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Snapping To Closest Break Line Error") ;
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
BENTLEYDTM_EXPORT int bcdtmSite_findClosestOrthogonalBreakLineForUserTagsDtmObject
(
 BC_DTM_OBJ  *dtmP,                        /* ==> Pointer To Dtm Object                               */
 double       pointX,                       /* ==> Snap Point x Coordinate Value                       */
 double       pointY,                       /* ==> Snap Point y Coordinate Value                       */
 DTMUserTag *userTagsP,                   /* ==> Pointer To User Tag Array                           */
 long         numUserTags,                  /* ==> Number Of User Tags                                 */
 long         *findTypeP,                   /* <== Find Type <0=Not Found,1=Point Snap,2=Line Snap>    */
 long         *brkPnt1P,                    /* <== Tin Point Number For Break Line Segment Start       */
 long         *brkPnt2P,                    /* <== Tin Point Number For Break Line Segment End         */
 double       *brkPntXP,                    /* <== Snap Point x Coordinate Value On Break Line         */ 
 double       *brkPntYP,                    /* <== Snap Point y Coordinate Value On Break Line         */ 
 double       *brkPntZP,                    /* <== Snap Point z Coordinate Value On Break Line         */
 double       *brkDistanceP,                /* <== Snap Distance To Break Line                         */
 DTM_TIN_POINT_FEATURES **brkFeaturesPP,    /* <== Pointer To Dtm Features For Snap Break Line Segment */
 long          *numBrkFeaturesP             /* <== Number Of Dtm Features For Snap Break Line Segment  */
) 
/*
** This Function Finds The Closest User Tag Break Line To pointX,pointY 
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   sPnt,nPnt,dtmFeature,onLineFlag,testBreak,inUserTagList ;
 double x,y,distance,brk1Dist,brk2Dist,xMin,xMax,yMin,yMax ;
 DTMUserTag       *tagP ;
 BC_DTM_FEATURE     *fP ;
 DTM_TIN_POINT_FEATURES *fListP ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Finding Closest Orthogonal User Tag Break Line") ;
    bcdtmWrite_message(0,0,0,"dtmP        = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"pointX      = %12.5lf",pointX) ;
    bcdtmWrite_message(0,0,0,"pointY      = %12.5lf",pointY) ;
    bcdtmWrite_message(0,0,0,"userTagsP   = %p",userTagsP) ;
    bcdtmWrite_message(0,0,0,"numUserTags = %8ld",numUserTags) ;
    if( dbg == 2 )
      {
       if( numUserTags > 0 && userTagsP != NULL )
         {
          for( tagP = userTagsP ; tagP < userTagsP + numUserTags ; ++tagP )
            {
             bcdtmWrite_message(0,0,0,"userTag[%4ld] = %12I64d",(long)(tagP-userTagsP),*tagP) ;
            }
         } 
      }
   }
/*
** Initialise
*/
 *findTypeP = 0 ;
 *brkPnt1P = dtmP->nullPnt ;
 *brkPnt2P = dtmP->nullPnt ;
 *brkPntXP = 0.0 ;
 *brkPntYP = 0.0 ;
 *brkPntZP = 0.0 ;
 *brkDistanceP = 0.0 ;
 if( *brkFeaturesPP != NULL ) { free(*brkFeaturesPP) ; *brkFeaturesPP = NULL ; }
/*
** Scan Break Line Features
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    fP = ftableAddrP(dtmP,dtmFeature) ;
    if( fP->dtmFeatureState == DTMFeatureState::Tin && fP->dtmFeatureType == DTMFeatureType::Breakline )
      {
       sPnt = fP->dtmFeaturePts.firstPoint ;
/*
**     Check If User Tag Is In List
*/
       inUserTagList = TRUE ;
       if( numUserTags > 0 && userTagsP != NULL )
         {
          inUserTagList = FALSE ;
          for( tagP = userTagsP ; tagP < userTagsP + numUserTags  && inUserTagList == FALSE ; ++tagP )
            {
             if( fP->dtmUserTag == *tagP ) inUserTagList = TRUE ;
            } 
         }
/*
**     Scan Dtm Feature For Drape Line Intersect
*/
       if( inUserTagList == TRUE )
         {
          do
            { 
             if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sPnt,&nPnt)) goto errexit ;
             if( nPnt != dtmP->nullPnt )
               {
                testBreak = 1 ;
/*
**              Get Break Line Max And Mins
*/
                if( *findTypeP )
                  {
                   if( pointAddrP(dtmP,sPnt)->x <= pointAddrP(dtmP,nPnt)->x ) { xMin = pointAddrP(dtmP,sPnt)->x ; xMax = pointAddrP(dtmP,nPnt)->x ; }
                   else                                                     { xMin = pointAddrP(dtmP,nPnt)->x ; xMax = pointAddrP(dtmP,sPnt)->x ; }
                   if( pointAddrP(dtmP,sPnt)->y <= pointAddrP(dtmP,nPnt)->y ) { yMin = pointAddrP(dtmP,sPnt)->y ; yMax = pointAddrP(dtmP,nPnt)->y ; }
                   else                                                     { yMin = pointAddrP(dtmP,nPnt)->y ; yMax = pointAddrP(dtmP,sPnt)->y ; }
                   if( pointX < xMin - *brkDistanceP || pointX > xMax + *brkDistanceP || pointY < yMin - *brkDistanceP || pointY > yMax + *brkDistanceP ) testBreak = 0 ;
                  }
/*
**              Test Break Line
*/
                if( testBreak )             
                  {
                   distance = bcdtmMath_distanceOfPointFromLine(&onLineFlag,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y,pointAddrP(dtmP,nPnt)->x,pointAddrP(dtmP,nPnt)->y,pointX,pointY,&x,&y) ;
                   if( onLineFlag )
                     {
                      if( dbg ) bcdtmWrite_message(0,0,0,"onLine ** dtmFeature = %6ld ** sPnt = %8ld nPnt = %8ld  ** distance = %12.6lf",dtmFeature,sPnt,nPnt,distance) ;
/*
**                    Set Closest Break Line
*/
                      if( ! *findTypeP || distance < *brkDistanceP ) 
                        { 
                         brk1Dist = bcdtmMath_distance(pointX,pointY,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y) ;
                         brk2Dist = bcdtmMath_distance(pointX,pointY,pointAddrP(dtmP,nPnt)->x,pointAddrP(dtmP,nPnt)->y) ;
                         if( brk1Dist < brk2Dist && brk1Dist < dtmP->ppTol )
                           {
                            *findTypeP = 1 ;
                            *brkDistanceP = 0.0 ;
                            *brkPntXP  = pointAddrP(dtmP,sPnt)->x  ; 
                            *brkPntYP  = pointAddrP(dtmP,sPnt)->y  ; 
                            *brkPntZP  = pointAddrP(dtmP,sPnt)->z  ; 
                            *brkPnt1P  = sPnt ; 
                            *brkPnt2P  = dtmP->nullPnt ; 
                           }
                         else if( brk2Dist < brk1Dist && brk2Dist < dtmP->ppTol )
                           {
                            *findTypeP = 1 ;
                            *brkDistanceP = 0.0 ;
                            *brkPntXP  = pointAddrP(dtmP,nPnt)->x  ; 
                            *brkPntYP  = pointAddrP(dtmP,nPnt)->y  ; 
                            *brkPntZP  = pointAddrP(dtmP,nPnt)->z  ; 
                            *brkPnt1P  = nPnt ; 
                            *brkPnt2P  = dtmP->nullPnt ; 
                          }
                         else
                           {
                            *brkDistanceP = distance ;
                            *findTypeP = 2 ;
                            *brkPntXP  = x  ; 
                            *brkPntYP  = y  ; 
                            *brkPnt1P  = sPnt ; 
                            *brkPnt2P  = nPnt ; 
                            bcdtmMath_interpolatePointOnLineDtmObject(dtmP,*brkPntXP,*brkPntYP,brkPntZP,sPnt,nPnt) ;
                           }
                         if( dbg )
                           {   
                            bcdtmWrite_message(0,0,0,"Setting Closest Orthogonal Break Point") ;
                            bcdtmWrite_message(0,0,0,"sPnt = %6ld ** %12.4lf %12.4lf %10.4lf",sPnt,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y,pointAddrP(dtmP,sPnt)->z) ;
                            bcdtmWrite_message(0,0,0,"nPnt = %6ld ** %12.4lf %12.4lf %10.4lf",nPnt,pointAddrP(dtmP,nPnt)->x,pointAddrP(dtmP,nPnt)->y,pointAddrP(dtmP,nPnt)->z) ;
                            bcdtmWrite_message(0,0,0,"drapePnt = %12.4lf %12.4lf %10.4lf",x,y,*brkPntZP) ;
                            bcdtmWrite_message(0,0,0,"breakDistanceP = %12.6lf",*brkDistanceP) ;
                           }
                        }
                     }
                  } 
               }
/*
**           Set Next Point
*/
             sPnt = nPnt ;
            } while ( sPnt != fP->dtmFeaturePts.firstPoint && sPnt != dtmP->nullPnt ) ;
         }
      }
   }
/*
** Get Dtm Features For Snap Point
*/
 if( *findTypeP )
   {
    if( *findTypeP == 1 ) { if( bcdtmList_getDtmFeaturesForPointDtmObject(dtmP,*brkPnt1P,brkFeaturesPP,numBrkFeaturesP)) goto errexit ; }
    if( *findTypeP == 2 ) { if( bcdtmList_getDtmFeaturesForLineDtmObject(dtmP,*brkPnt1P,*brkPnt2P,brkFeaturesPP,numBrkFeaturesP)) goto errexit ; } 
   } 
/*
** Write Out Results
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"findTypeP        = %2ld",*findTypeP)     ;
    bcdtmWrite_message(0,0,0,"brkPntXP         = %20.10lf",*brkPntXP)  ;
    bcdtmWrite_message(0,0,0,"brkPntYP         = %20.10lf",*brkPntYP)  ;
    bcdtmWrite_message(0,0,0,"brkPntZP         = %20.10lf",*brkPntZP)  ;
    bcdtmWrite_message(0,0,0,"brkDistanceP     = %20.15lf",*brkDistanceP)    ;
    bcdtmWrite_message(0,0,0,"brkPnt1P         = %9ld",*brkPnt1P)      ;
    bcdtmWrite_message(0,0,0,"brkPnt2P         = %9ld",*brkPnt2P)      ;
    bcdtmWrite_message(0,0,0,"numBrkFeaturesP  = %9ld",*numBrkFeaturesP) ;
    for( fListP = *brkFeaturesPP ; fListP < *brkFeaturesPP + *numBrkFeaturesP ; ++fListP )
      {
       bcdtmWrite_message(0,0,0,"brkFeature[%4ld] ** dtmFeature = %8ld ** Type = %4ld ** userTag = %10I64d ** nextPoint = %9ld priorPoint = %9ld",(long)(fListP-*brkFeaturesPP),fListP->dtmFeature,fListP->dtmFeatureType,fListP->userTag,fListP->nextPoint,fListP->priorPoint) ; 
       bcdtmList_writePointsForDtmFeatureDtmObject(dtmP,fListP->dtmFeature) ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Finding Closest Orthogonal User Tag Break Line Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Finding Closest Orthogonal User Tag Break Line Error") ;
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
BENTLEYDTM_EXPORT int bcdtmSite_projectAtAngleFromPointToBreakLineDtmObject
(
 BC_DTM_OBJ        *dtmP,                     /* ==> Pointer To Tin Object                     */
 double        xPnt,                      /* ==> x Coordiante Of Point                     */
 double        yPnt,                      /* ==> y Coordiante Of Point                     */
 long          brkPnt1,                   /* ==> Tin Point Of Current Break Line End Point */ 
 long          brkPnt2,                   /* ==> Tin Point Of Current Break Line End Point */
 double        projectAngle,              /* ==> Projection Angle                          */ 
 double        deviationAngle,            /* ==> Deviation Angle (+/- about projection angle)  */ 
 long          *breakFoundP,              /* <== Break Found <TRUE,FALSE>                  */
 long          *breakTypeP,               /* <== Break Type <1=Break Point,2=Break Line>   */
 double        *xBreakP,                  /* <== x Coordiante Of Break Point               */  
 double        *yBreakP,                  /* <== y Coordiante Of Break Point               */  
 double        *zBreakP,                  /* <== z Coordiante Of Break Point               */  
 double        *dBreakP,                  /* <== Distance To Break Point                   */
 long          *dtmPnt1P,                 /* <== Tin Point Of Break Line End               */ 
 long          *dtmPnt2P,                 /* <== Tin Point Of Break Line End               */ 
 DTM_TIN_POINT_FEATURES **breakFeaturesPP,/* <== Pointer To Dtm Features At Break Point    */
 long          *numBreakFeaturesP         /* <== Number Of Dtm Features At Break Point     */
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ; 
 long   internalFlag=0 ;
 DTM_TIN_POINT_FEATURES *fListP ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Projecting At Angle From Point To Break Line") ;
    bcdtmWrite_message(0,0,0,"xPnt           = %20.10lf",xPnt)  ;
    bcdtmWrite_message(0,0,0,"yPnt           = %20.10lf",yPnt)  ;
    bcdtmWrite_message(0,0,0,"brkPnt1        = %9ld",brkPnt1)   ;
    bcdtmWrite_message(0,0,0,"brkPnt2        = %9ld",brkPnt2)   ;
    bcdtmWrite_message(0,0,0,"projectAngle   = %12.10lf",projectAngle) ;
    bcdtmWrite_message(0,0,0,"deviationAngle = %12.10lf",deviationAngle) ;
   }
/*
** Initialise
*/
 *breakFoundP = FALSE ;
 *breakTypeP  = 0   ;
 *xBreakP     = 0.0 ;
 *yBreakP     = 0.0 ;
 *zBreakP     = 0.0 ;
 *dBreakP     = 0.0 ;
 *dtmPnt1P   = dtmP->nullPnt ;
 *dtmPnt2P   = dtmP->nullPnt ;
 if( *breakFeaturesPP != NULL ) { free(*breakFeaturesPP) ; *breakFeaturesPP = NULL ; }
 *numBreakFeaturesP = 0 ;
/*
** Validate Tin Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP) != DTM_SUCCESS ) goto errexit ;
/*
** Test For An External Projection
*/
 internalFlag = 1 ;
 if( ( brkPnt2 == dtmP->nullPnt && nodeAddrP(dtmP,brkPnt1)->hPtr != dtmP->nullPnt ) ||
     ( brkPnt2 != dtmP->nullPnt && (nodeAddrP(dtmP,brkPnt1)->hPtr == brkPnt2 || nodeAddrP(dtmP,brkPnt2)->hPtr == brkPnt1 )) )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking For External Project Angle") ;
    if( bcdtmSite_testForAnAngleProjectionExternalToTinHullDtmObject(dtmP,brkPnt1,brkPnt2,projectAngle,&internalFlag)) goto errexit ;
   }
/*
** Only Process If Projection Internal
*/
 if( internalFlag )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Drape Projecting At Angle") ;
    if( bcdtmSite_drapeProjectAtAngleToNextBreakLineDtmObject(dtmP,xPnt,yPnt,projectAngle,deviationAngle,brkPnt1,brkPnt2,breakFoundP,breakTypeP,dtmPnt1P,dtmPnt2P,xBreakP,yBreakP,zBreakP,dBreakP) != DTM_SUCCESS ) goto errexit ;
/*
**  Check For Closer Break Line End Points Within Deviation Angle
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Finding Closest Break Line Point Within Deviation Angle") ;
    if( bcdtmSite_findClosestBreakLineEndPointWithinDeviationAngleDtmObject(dtmP,xPnt,yPnt,projectAngle,deviationAngle,brkPnt1,brkPnt2,breakFoundP,breakTypeP,dtmPnt1P,dtmPnt2P,xBreakP,yBreakP,zBreakP,dBreakP) != DTM_SUCCESS ) goto errexit ;
/*
**  Find Next Closest Break Line At Projection Angle
**  RobC - 11 April 2006  Code Commented Out. Replaced By baove Two Functions
*/
//    if( dbg ) bcdtmWrite_message(0,0,0,"Finding Intersection At Angle") ;
//    if( bcdtmSite_findClosestBreakLineIntersectionForProjectAngleDtmObject(dtmP,xPnt,yPnt,projectAngle,deviationAngle,brkPnt1,brkPnt2,breakFoundP,breakTypeP,dtmPnt1P,dtmPnt2P,xBreakP,yBreakP,zBreakP,dBreakP) != DTM_SUCCESS ) goto errexit ;
/*
**  Get Dtm Features For Break Point
*/
    if( *breakFoundP == TRUE )
      {
       if( *breakTypeP == 1 ) { if( bcdtmList_getDtmFeaturesForPointDtmObject(dtmP,*dtmPnt1P,breakFeaturesPP,numBreakFeaturesP)) goto errexit ; }
       if( *breakTypeP == 2 ) { if( bcdtmList_getDtmFeaturesForLineDtmObject(dtmP,*dtmPnt1P,*dtmPnt2P,breakFeaturesPP,numBreakFeaturesP)) goto errexit ; } 
      }  
   }
/*
**  Write Out Results
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Result Of Projecting At Angle From Point To Break Line") ;
    bcdtmWrite_message(0,0,0,"breakFoundP       = %2ld",*breakFoundP) ;
    bcdtmWrite_message(0,0,0,"breakTypeP        = %2ld",*breakTypeP)  ;
    bcdtmWrite_message(0,0,0,"xBreakP           = %20.10lf",*xBreakP)  ;
    bcdtmWrite_message(0,0,0,"yBreakP           = %20.10lf",*yBreakP)  ;
    bcdtmWrite_message(0,0,0,"zBreakP           = %20.10lf",*zBreakP)  ;
    bcdtmWrite_message(0,0,0,"dBreakP           = %20.15lf",*dBreakP)  ;
    bcdtmWrite_message(0,0,0,"dtmPnt1P          = %9ld",*dtmPnt1P)   ;
    bcdtmWrite_message(0,0,0,"dtmPnt2P          = %9ld",*dtmPnt2P)   ;
    bcdtmWrite_message(0,0,0,"numBreakFeaturesP = %9ld",*numBreakFeaturesP) ;
    for( fListP = *breakFeaturesPP ; fListP < *breakFeaturesPP + *numBreakFeaturesP ; ++fListP )
      {
       bcdtmWrite_message(0,0,0,"breakFeature[%4ld] ** dtmFeature = %8ld ** Type = %4ld ** userTag = %10I64d ** nextPoint = %9ld priorPoint = %9ld",(long)(fListP-*breakFeaturesPP),fListP->dtmFeature,fListP->dtmFeatureType,fListP->userTag,fListP->nextPoint,fListP->priorPoint) ; 
      }
   }

/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Projecting At Angle From Point To Break Line Completed") ;
 if( dbg && ret != DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Projecting At Angle From Point To Break Line Error") ;
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
BENTLEYDTM_EXPORT int bcdtmSite_calculateRangeAnglesForOrthogonalToDtmObject
(
 BC_DTM_OBJ *dtmP,
 double lastX,
 double lastY,
 double pntX,
 double pntY,
 long   dtmPnt1,
 long   dtmPnt2,
 double *rangeStartAngleP,
 double *rangeEndAngleP
)
{
 int ret=DTM_SUCCESS,sdof,dbg=DTM_TRACE_VALUE(0) ;
 long dtmFeature ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Range Angles") ;
/*
** Initialise
*/
 *rangeStartAngleP = 0.0 ;
 *rangeEndAngleP   = 0.0 ;
/*
** Foward From Tin Line
*/
 if( dtmPnt1 != dtmP->nullPnt && dtmPnt2 != dtmP->nullPnt )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Project Angle From Tin Line") ;
    sdof  = bcdtmMath_sideOf(pointAddrP(dtmP,dtmPnt1)->x,pointAddrP(dtmP,dtmPnt1)->y,pointAddrP(dtmP,dtmPnt2)->x,pointAddrP(dtmP,dtmPnt2)->y,lastX,lastY) ;
    if( sdof >= 0 ) 
      {
       *rangeStartAngleP = bcdtmMath_getAngle(pntX,pntY,pointAddrP(dtmP,dtmPnt1)->x,pointAddrP(dtmP,dtmPnt1)->y) + 0.001 ;
       *rangeEndAngleP   = bcdtmMath_getAngle(pntX,pntY,pointAddrP(dtmP,dtmPnt2)->x,pointAddrP(dtmP,dtmPnt2)->y) - 0.001 ;
      }
    else
      {
       *rangeStartAngleP = bcdtmMath_getAngle(pntX,pntY,pointAddrP(dtmP,dtmPnt2)->x,pointAddrP(dtmP,dtmPnt2)->y) + 0.001 ;
       *rangeEndAngleP   = bcdtmMath_getAngle(pntX,pntY,pointAddrP(dtmP,dtmPnt1)->x,pointAddrP(dtmP,dtmPnt1)->y) - 0.001 ;
      }
   }
/*
** Foward From Tin Point
*/
 else if( dtmPnt1 != dtmP->nullPnt && dtmPnt2 == dtmP->nullPnt )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Project Angle From Tin Point") ;
    if( ! bcdtmList_testForPointOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Breakline,dtmPnt1,&dtmFeature))
      {
       bcdtmWrite_message(2,0,0,"Tin Point Not A Break Point") ;
       goto errexit ;
      }
    *rangeStartAngleP = *rangeEndAngleP = bcdtmMath_getAngle(lastX,lastY,pntX,pntY) ;
    *rangeStartAngleP = bcdtmMath_normaliseAngle(*rangeStartAngleP-DTM_PYE/2.0 + 0.001 ) ; 
    *rangeEndAngleP   = bcdtmMath_normaliseAngle(*rangeEndAngleP+DTM_PYE/2.0 - 0.001 ) ; 
   }
/*
**  Clean Up
*/
 cleanup :
/*
**  Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Calculating Range Angles Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Calculating Range Angles Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit:
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmSite_projectOrthogonalToBreakLineFromPointDtmObject
(
 BC_DTM_OBJ        *dtmP,                     /* ==> Pointer To Tin Object                     */
 double        xPnt,                      /* ==> x Coordiante Of Point                     */
 double        yPnt,                      /* ==> y Coordiante Of Point                     */
 long          brkPnt1,                   /* ==> Tin Point Of Current Break Line End Point */ 
 long          brkPnt2,                   /* ==> Tin Point Of Current Break Line End Point */
 double        rangeAngleStart,           /* ==> Start Range Angle                         */ 
 double        rangeAngleEnd,             /* ==> End Range Angle                           */ 
 double        deviationAngle,            /* ==> Deviation Angle (+/- about normal)        */
 long          *breakFoundP,              /* <== Break Found <TRUE,FALSE>                  */
 long          *breakTypeP,               /* <== Break Type <1=Break Point,2=Break Line>   */
 double        *xBreakP,                  /* <== x Coordiante Of Break Point               */  
 double        *yBreakP,                  /* <== y Coordiante Of Break Point               */  
 double        *zBreakP,                  /* <== z Coordiante Of Break Point               */  
 double        *dBreakP,                  /* <== Distance To Break Point                   */
 long          *dtmPnt1P,                 /* <== Tin Point Of Break Line End               */ 
 long          *dtmPnt2P,                 /* <== Tin Point Of Break Line End               */ 
 DTM_TIN_POINT_FEATURES **breakFeaturesPP,/* <== Pointer To Dtm Features At Break Point    */
 long          *numBreakFeaturesP         /* <== Number Of Dtm Features At Break Point     */
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ; 
 long   internalFlag=1 ;
 DTM_TIN_POINT_FEATURES *fListP ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Projecting Orthogonal To Break Line From Point") ;
    bcdtmWrite_message(0,0,0,"xPnt            = %20.10lf",xPnt)  ;
    bcdtmWrite_message(0,0,0,"yPnt            = %20.10lf",yPnt)  ;
    bcdtmWrite_message(0,0,0,"brkPnt1         = %9ld",brkPnt1)   ;
    bcdtmWrite_message(0,0,0,"brkPnt2         = %9ld",brkPnt2)   ;
    bcdtmWrite_message(0,0,0,"rangeAngleStart = %12.10lf",rangeAngleStart) ;
    bcdtmWrite_message(0,0,0,"rangeAngleEnd   = %12.10lf",rangeAngleEnd) ;
    bcdtmWrite_message(0,0,0,"deviationAngle  = %12.10lf",deviationAngle) ;
   }
/*
** Initialise
*/
 *breakFoundP = FALSE ;
 *breakTypeP  = 0   ;
 *xBreakP     = 0.0 ;
 *yBreakP     = 0.0 ;
 *zBreakP     = 0.0 ;
 *dBreakP     = 0.0 ;
 *dtmPnt1P   = dtmP->nullPnt ;
 *dtmPnt2P   = dtmP->nullPnt ;
 if( *breakFeaturesPP != NULL ) { free(*breakFeaturesPP) ; *breakFeaturesPP = NULL ; }
 *numBreakFeaturesP = 0 ;
/*
** Validate Tin Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP) != DTM_SUCCESS ) goto errexit ;
/*
** Test If Projection Goes External To Tin Hull
*/
 if( ( brkPnt2 == dtmP->nullPnt && nodeAddrP(dtmP,brkPnt1)->hPtr != dtmP->nullPnt ) ||
     ( brkPnt2 != dtmP->nullPnt && ( nodeAddrP(dtmP,brkPnt1)->hPtr == brkPnt2 || nodeAddrP(dtmP,brkPnt2)->hPtr == brkPnt1 )) )
   {
    if( bcdtmSite_testForOrthogonalToProjectionExternalToTinHullDtmObject(dtmP,brkPnt1,brkPnt2,rangeAngleStart,rangeAngleEnd,&internalFlag)) goto errexit ;
   }
/*
** Only Process If Projection Is Internal To Tin Hull
*/ 
 if( internalFlag )
   { 
/*
**  Scan For An Initial Orthogonal Intersection
*/
    if( bcdtmSite_drapeAngleScanForOrthogonalBreakLineIntersectionDtmObject(dtmP,xPnt,yPnt,rangeAngleStart,rangeAngleEnd,deviationAngle,brkPnt1,brkPnt2,breakFoundP,breakTypeP,dtmPnt1P,dtmPnt2P,xBreakP,yBreakP,zBreakP,dBreakP) != DTM_SUCCESS ) goto errexit ;
/*
**  Find Next Closest Break Line At Projection Angle
*/
    if( bcdtmSite_findClosestOrthogonalBreakLineIntersectionDtmObject(dtmP,xPnt,yPnt,rangeAngleStart,rangeAngleEnd,deviationAngle,brkPnt1,brkPnt2,breakFoundP,breakTypeP,dtmPnt1P,dtmPnt2P,xBreakP,yBreakP,zBreakP,dBreakP) != DTM_SUCCESS ) goto errexit ;
/*
**  Get Dtm Features For Break Point
*/
    if( *breakFoundP == TRUE )
      {
       if( *breakTypeP == 1 ) { if( bcdtmList_getDtmFeaturesForPointDtmObject(dtmP,*dtmPnt1P,breakFeaturesPP,numBreakFeaturesP)) goto errexit ; }
       if( *breakTypeP == 2 ) { if( bcdtmList_getDtmFeaturesForLineDtmObject(dtmP,*dtmPnt1P,*dtmPnt2P,breakFeaturesPP,numBreakFeaturesP)) goto errexit ; } 
      } 
   }
/*
** Write Out Results
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Result Of Projecting Orthogonal To Break Line From Point") ;
    bcdtmWrite_message(0,0,0,"breakFoundP       = %2ld",*breakFoundP) ;
    bcdtmWrite_message(0,0,0,"breakTypeP        = %2ld",*breakTypeP)  ;
    bcdtmWrite_message(0,0,0,"xBreakP           = %20.10lf",*xBreakP)  ;
    bcdtmWrite_message(0,0,0,"yBreakP           = %20.10lf",*yBreakP)  ;
    bcdtmWrite_message(0,0,0,"zBreakP           = %20.10lf",*zBreakP)  ;
    bcdtmWrite_message(0,0,0,"dBreakP           = %20.15lf",*dBreakP)  ;
    bcdtmWrite_message(0,0,0,"dtmPnt1P          = %9ld",*dtmPnt1P)   ;
    bcdtmWrite_message(0,0,0,"dtmPnt2P          = %9ld",*dtmPnt2P)   ;
    bcdtmWrite_message(0,0,0,"numBreakFeaturesP = %9ld",*numBreakFeaturesP) ;
    for( fListP = *breakFeaturesPP ; fListP < *breakFeaturesPP + *numBreakFeaturesP ; ++fListP )
      {
       bcdtmWrite_message(0,0,0,"breakFeature[%4ld] ** dtmFeature = %8ld ** Type = %4ld ** userTag = %10I64d ** nextPoint = %9ld priorPoint = %9ld",(long)(fListP-*breakFeaturesPP),fListP->dtmFeature,fListP->dtmFeatureType,fListP->userTag,fListP->nextPoint,fListP->priorPoint) ; 
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Projecting Orthogonal To Break Line From Point Completed") ;
 if( dbg && ret != DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Projecting Orthogonal To Break Line From Point Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*==============================================================================*//**
* @memo   Snaps To Closest Break Line Then Drape Orthognal To Break Line
* @doc    Snaps To Closest Break Line Then Drape Orthognal To Break Line
* @param  dtmP                ==> Pointer To Tin object        
* @param  xPnt                ==> Drape Point Start x Coordinate      
* @param  yPnt                ==> Drape Point Start y Coordinate      
* @param  drapeArraysPPP     <== Pointer To Point Arrays Of Drape Points             
* @param  umDrapeArraysP     <== Number Of Drape Arrays              
* @return DTM_SUCCESS or DTM_ERROR
* @author Rob Cormack 1 August 2005 rob.cormack@bentley.com
* @version 
* @see None
*===============================================================================*/
BENTLEYDTM_EXPORT int bcdtmSite_snapToThenOrthogonalToBreakLineDtmObject
(
 BC_DTM_OBJ *dtmP,                  /* ==> Tin Object Pointer                       */
 double xPnt,                        /* ==> Drape Point Start x Coordinate           */
 double yPnt,                        /* ==> Drape Point Start y Coordinate           */
 DTM_POINT_ARRAY  ***drapeArraysPPP, /* <== Pointer To Point Arrays Of Drape Points  */
 long   *numDrapeArraysP             /* <== Number Of Drape Arrays                   */
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   ofs1,ofs2,offset,dtmFeature,drapeFlag,numDrapePts,processDrape,processBreak  ; 
 long   memDrapePts=0,memDrapePtsInc=1000,memDrapeArrays=0,memDrapeArraysInc=100 ;
 long   brkPnt1,brkPnt2,drapePnt1,drapePnt2,listPnt,clPtr ;
 long   nextDrapePnt1,nextDrapePnt2 ;
 double lastDrapePntX,lastDrapePntY,nextDrapePntX,nextDrapePntY,nextDrapePntZ,brkPntX,brkPntY,brkPntZ,drapePntX,drapePntY,drapePntZ ;
 double zPnt,brkDistance,drapeDistance ;
 unsigned char   *lP,*linesP=NULL ; 
 DPoint3d    *drapePtsP=NULL ;
 BC_DTM_OBJ          *dataP=NULL ;
 DTM_POINT_ARRAY  *pointArrayPnt=NULL,**pointArrayPntP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Snapping To And Draping Othogonal To Break Line") ;
    bcdtmWrite_message(0,0,0,"Tin Object      = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"xPnt            = %12.4lf",xPnt) ;
    bcdtmWrite_message(0,0,0,"yPnt            = %12.4lf",yPnt) ;
    bcdtmWrite_message(0,0,0,"drapeArraysPPP  = %p",*drapeArraysPPP) ;
    bcdtmWrite_message(0,0,0,"numDrapeArraysP = %8ld",*numDrapeArraysP) ;
   }
 dbg=DTM_TRACE_VALUE(0) ;
/*
** Initilaise
*/
 *numDrapeArraysP = 0 ;
 if( *drapeArraysPPP != NULL ) 
   {
    bcdtmWrite_message(2,0,0,"Drape Arrays Pointer Is Not NULL") ;
    goto cleanup ; 
   } 
/*
** Validate Tin Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP) != DTM_SUCCESS ) goto errexit ;
/*
** Snap Drape Start Point To Closest Tin Point If In Tolerance And Break Point
*/
 bcdtmFind_closestPointDtmObject(dtmP,xPnt,yPnt,&ofs1) ;
 if( bcdtmList_testForPointOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Breakline,ofs1,&dtmFeature))
   {
    drapeDistance = bcdtmMath_distance(xPnt,yPnt,pointAddrP(dtmP,ofs1)->x,pointAddrP(dtmP,ofs1)->y) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Closest Tin Point %6ld ** Distance = %20.15lf",ofs1,drapeDistance) ;
    if( drapeDistance < dtmP->ppTol ) 
      {
       xPnt = pointAddrP(dtmP,ofs1)->x ;
       yPnt = pointAddrP(dtmP,ofs1)->y ;
      }
   }
/*
** Check Start Point Is On Tin
*/
 if( bcdtmDrape_pointDtmObject(dtmP,xPnt,yPnt,&zPnt,&drapeFlag) != DTM_SUCCESS ) goto errexit ;
 if( ! drapeFlag )
   {
    if( bcdtmFind_findClosestHullLineDtmObject(dtmP,xPnt,yPnt,&zPnt,&drapeFlag,&ofs1,&ofs2)) goto errexit ;
   }
/*
** Find Closest Orthogonal Break Line To Start Drape Point
*/
 if( bcdtmSite_findClosestOrthogonalBreakLineDtmObject(dtmP,xPnt,yPnt,1,&processBreak,&brkPnt1,&brkPnt2,&brkPntX,&brkPntY,&brkPntZ,&brkDistance)) goto errexit ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"processBreak = %2ld",processBreak) ;
    bcdtmWrite_message(0,0,0,"brkPntX      = %15.8lf",brkPntX) ;
    bcdtmWrite_message(0,0,0,"brkPntY      = %15.8lf",brkPntY) ;
    bcdtmWrite_message(0,0,0,"brkPntZ      = %15.8lf",brkPntZ) ;
    bcdtmWrite_message(0,0,0,"brkDistance  = %15.8lf",brkDistance) ;
    bcdtmWrite_message(0,0,0,"brkPnt1      = %9ld",brkPnt1) ;
    bcdtmWrite_message(0,0,0,"brkPnt2      = %9ld",brkPnt2) ;
    if( bcdtmDrape_pointDtmObject(dtmP,brkPntX,brkPntY,&drapePntZ,&drapeFlag) != DTM_SUCCESS ) goto errexit ;
    if( ! drapeFlag )
      {
       bcdtmWrite_message(1,0,0,"Start Break Point External To Tin") ;
       goto errexit ; 
      }
   }
/*
** Only Process If Closest Break Line Found
*/
 if( processBreak )
   { 
/*
**  Write Closest Break Point
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Start Break Point  ** %12.5lf %12.5lf %12.5lf",brkPntX,brkPntY,brkPntZ) ;
/*
**  Allocate And Initialise Memory For Marking Processed Break Line Segments
*/
    linesP = ( unsigned char * ) malloc( (dtmP->cListPtr/8+1) * sizeof(char)) ;
    if( linesP == NULL )
      {
       bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
    for( lP = linesP ; lP < linesP + dtmP->cListPtr/8+1 ; ++lP ) *lP = (char) 0 ;
/*
**  Mark Start Break Line Segment
*/
    if( processBreak == 2 )
      {
       if( brkPnt1 < brkPnt2 ) { ofs1 = brkPnt1 ; ofs2 = brkPnt2 ; }
       else                    { ofs1 = brkPnt2 ; ofs2 = brkPnt1 ; }
       if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,ofs1,ofs2)) goto errexit ;
       bcdtmFlag_setFlag(linesP,offset)  ;
      }
/*
**  Get Next Break Line From Start Break
*/
    while( processBreak )
      {
/*
**     Find Next Closest Break Line
*/
       if( dbg ) bcdtmWrite_message(0,0,0,"***********************************") ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Getting Next Drape From Start Break") ;
       if( bcdtmSite_findNextClosestOrthogonalBreakLineDtmObject(dtmP,linesP,brkPnt2,brkPnt1,brkPntX,brkPntY,0,0.0,0.0,&processBreak,&dtmFeature,&drapePnt1,&drapePnt2,&drapePntX,&drapePntY,&drapePntZ,&drapeDistance)) goto errexit ;
       if( dbg )
         {
          bcdtmWrite_message(0,0,0,"processDrape   = %2ld",processBreak) ;
          bcdtmWrite_message(0,0,0,"drapePntX      = %15.8lf",drapePntX) ;
          bcdtmWrite_message(0,0,0,"drapePntY      = %15.8lf",drapePntY) ;
          bcdtmWrite_message(0,0,0,"drapePntZ      = %15.8lf",drapePntZ) ;
          bcdtmWrite_message(0,0,0,"drapeDistance  = %15.8lf",drapeDistance) ;
          bcdtmWrite_message(0,0,0,"drapePnt1      = %9ld",drapePnt1) ;
          bcdtmWrite_message(0,0,0,"drapePnt2      = %9ld",drapePnt2) ;
         }
/*
**     Break Line Found
*/
       if( processBreak )
         {
/*
**        Mark First Drape Line Segment
*/ 
          if( drapePnt1 < drapePnt2 ) { ofs1 = drapePnt1 ; ofs2 = drapePnt2 ; }
          else                        { ofs1 = drapePnt2 ; ofs2 = drapePnt1 ; }
          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,ofs1,ofs2)) goto errexit ;
          bcdtmFlag_setFlag(linesP,offset)  ;
/*
**        Mark All Break Lines Connected To Break Point
*/
          if( processBreak == 1 )
            {
             clPtr = nodeAddrP(dtmP,drapePnt1)->cPtr;
             while( clPtr != dtmP->nullPtr )
               {
                listPnt = clistAddrP(dtmP,clPtr)->pntNum ;          
                clPtr   = clistAddrP(dtmP,clPtr)->nextPtr ;          
                if( drapePnt1 < listPnt ) { ofs1 = drapePnt1 ; ofs2 = listPnt ; }
                else                      { ofs1 = listPnt ; ofs2 = drapePnt1 ; }
                if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,ofs1,ofs2)) goto errexit ;
                bcdtmFlag_setFlag(linesP,offset)  ;
               }
            }
/*
**        Allocate Initial Memory For Drape Points
*/     
          memDrapePts = memDrapePts + memDrapePtsInc ;
          drapePtsP = ( DPoint3d * ) malloc(memDrapePts * sizeof(DPoint3d)) ;
          if( drapePtsP == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }
/*
**        Store Drape Point Start
*/
          if( dbg == 1 ) bcdtmWrite_message(0,0,0,"**** Storing Start Drape Point ** %12.4lf %12.4lf %12.4lf",xPnt,yPnt,zPnt) ;
          numDrapePts = 0 ; 
          (drapePtsP+numDrapePts)->x = xPnt ;
          (drapePtsP+numDrapePts)->y = yPnt ;
          (drapePtsP+numDrapePts)->z = zPnt ;
          ++numDrapePts   ;
/*
**        Store Break Point
*/
          if( dbg == 1 ) bcdtmWrite_message(0,0,0,"**** Storing Start Break Point ** %12.4lf %12.4lf %12.4lf",brkPntX,brkPntY,brkPntZ) ;
          (drapePtsP+numDrapePts)->x = brkPntX ;
          (drapePtsP+numDrapePts)->y = brkPntY ;
          (drapePtsP+numDrapePts)->z = brkPntZ ;
          ++numDrapePts   ;
          lastDrapePntX = brkPntX ;
          lastDrapePntY = brkPntY ;
/*
**        Store Drape Point
*/
          if( dbg == 1 ) bcdtmWrite_message(0,0,0,"**** Storing First Drape Point  ** %12.4lf %12.4lf %12.4lf",drapePntX,drapePntY,drapePntZ) ;
          (drapePtsP+numDrapePts)->x = drapePntX ;
          (drapePtsP+numDrapePts)->y = drapePntY ;
          (drapePtsP+numDrapePts)->z = drapePntZ ;
          ++numDrapePts ;
/*
**        Loop Until No Normal Intersections Can Be Determined With Break lines
*/
          processDrape = 1 ;
          while( processDrape )
            {
             if( dbg ) bcdtmWrite_message(0,0,0,"Getting Next Drape Point") ;
             if( bcdtmSite_findNextClosestOrthogonalBreakLineDtmObject(dtmP,linesP,drapePnt1,drapePnt2,drapePntX,drapePntY,1,lastDrapePntX,lastDrapePntY,&processDrape,&dtmFeature,&nextDrapePnt1,&nextDrapePnt2,&nextDrapePntX,&nextDrapePntY,&nextDrapePntZ,&drapeDistance)) goto errexit ;
             if( dbg )
               {
                bcdtmWrite_message(0,0,0,"processDrape   = %2ld",processDrape) ;
                bcdtmWrite_message(0,0,0,"nextDrapePnt1  = %9ld",nextDrapePnt1) ;
                bcdtmWrite_message(0,0,0,"nextDrapePnt2  = %9ld",nextDrapePnt2) ;
                bcdtmWrite_message(0,0,0,"nextDrapePntX  = %15.8lf",nextDrapePntX) ;
                bcdtmWrite_message(0,0,0,"nextDrapePntY  = %15.8lf",nextDrapePntY) ;
                bcdtmWrite_message(0,0,0,"nextPrapePntZ  = %15.8lf",nextDrapePntZ) ;
                bcdtmWrite_message(0,0,0,"drapeDistance  = %15.8lf",drapeDistance) ;
               }
/*
**           Find Next Closest Break Line
*/
             if( processDrape )
               {
/*
**              Test For Termination On Tin Hull
*/
                if( nextDrapePnt1 != dtmP->nullPnt && nextDrapePnt2 != dtmP->nullPnt )
                  {
                   if( nodeAddrP(dtmP,nextDrapePnt1)->hPtr == nextDrapePnt2 || nodeAddrP(dtmP,nextDrapePnt2)->hPtr == nextDrapePnt1 ) processDrape = 0 ;
                  }
                else if( nextDrapePnt1 != dtmP->nullPnt && pointAddrP(dtmP,nextDrapePnt1)->x == nextDrapePntX && pointAddrP(dtmP,nextDrapePnt1)->y == nextDrapePntY ) processDrape = 0 ;
                else if( nextDrapePnt2 != dtmP->nullPnt && pointAddrP(dtmP,nextDrapePnt2)->x == nextDrapePntX && pointAddrP(dtmP,nextDrapePnt2)->y == nextDrapePntY ) processDrape = 0 ;
/*
**              Set Last Drape Point
*/
                lastDrapePntX = drapePntX ;
                lastDrapePntY = drapePntY ;
/*
**              Set Current Drape Point
*/
                drapePnt1 = nextDrapePnt1 ;
                drapePnt2 = nextDrapePnt2 ;
                drapePntX = nextDrapePntX ;
                drapePntY = nextDrapePntY ;
                drapePntZ = nextDrapePntZ ;
/*
**              Check And Allocate memory
*/       
                if( numDrapePts == memDrapePts )
                  {
                   memDrapePts = memDrapePts + memDrapePtsInc ;
                   if( drapePtsP == NULL ) drapePtsP = ( DPoint3d * ) malloc(memDrapePts * sizeof(DPoint3d)) ;
                   else                    drapePtsP = ( DPoint3d * ) realloc(drapePtsP,memDrapePts*sizeof(DPoint3d)) ; 
                   if( drapePtsP == NULL )
                     {
                      bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                      goto errexit ;
                     }
                  } 
/*
**              Store Drape Point
*/
                if( dbg == 1 ) bcdtmWrite_message(0,0,0,"**** Storing Next  Drape Point  ** %12.4lf %12.4lf %12.4lf",drapePntX,drapePntY,drapePntZ) ;
                (drapePtsP+numDrapePts)->x = drapePntX ;
                (drapePtsP+numDrapePts)->y = drapePntY ;
                (drapePtsP+numDrapePts)->z = drapePntZ ;
                ++numDrapePts ;
               }
            }
          if( dbg ) bcdtmWrite_message(0,0,0,"===================================") ;
/*
**        Store Drape Points In Drape Array
*/
          if( *numDrapeArraysP == memDrapeArrays )
            {
             memDrapeArrays = memDrapeArrays + memDrapeArraysInc ;
             if( *drapeArraysPPP == NULL ) *drapeArraysPPP = ( DTM_POINT_ARRAY  ** ) malloc(memDrapeArrays * sizeof(DTM_POINT_ARRAY *)) ;
             else                          *drapeArraysPPP = ( DTM_POINT_ARRAY  ** ) realloc(**drapeArraysPPP,memDrapeArrays*sizeof(DTM_POINT_ARRAY *)) ; 
             if( *drapeArraysPPP == NULL )
               {
                bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                goto errexit ;
               } 
            }
/*
**        Allocate Memory For Point Array
*/
          pointArrayPnt = ( DTM_POINT_ARRAY * ) malloc( sizeof(DTM_POINT_ARRAY)) ;
          if( pointArrayPnt == NULL ) 
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }  
/*
**        Populate Point Array
*/
          pointArrayPnt->pointsP   = drapePtsP ;  
          pointArrayPnt->numPoints = numDrapePts ;
/*
**        Store Point Array In Drape Array
*/
          *(*drapeArraysPPP+*numDrapeArraysP) = pointArrayPnt ;
          ++*numDrapeArraysP ;
/*
**        Null Out Memory Arrays
*/
          pointArrayPnt = NULL    ; 
          drapePtsP   = NULL    ; 
          numDrapePts = 0       ;
         }
/*
**     Only Process Closest Drape Line
*/
       processBreak = 0 ;
      } 
   }
/*
** Reallocate Drape Arrays Memory
*/
 if( *numDrapeArraysP > 0 && *numDrapeArraysP < memDrapeArrays )
   {
    *drapeArraysPPP = ( DTM_POINT_ARRAY ** ) realloc(*drapeArraysPPP,*numDrapeArraysP*sizeof(DTM_POINT_ARRAY *)) ; 
   }
/*
** Clean Up
*/
 cleanup :
 if( linesP      != NULL ) free(linesP) ;
 if( drapePtsP   != NULL ) { free(drapePtsP) ; drapePtsP = NULL ; }
 if( dataP       != NULL ) bcdtmObject_destroyDtmObject(&dataP) ;
 if( pointArrayPnt != NULL )
   {
    if( pointArrayPnt->pointsP != NULL ) free(pointArrayPnt->pointsP) ;
    free(pointArrayPnt) ;
   } 
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Snapping To And Draping Othogonal To Break Line Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Snapping To And Draping Othogonal To Break Line Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR  ;
 if( *drapeArraysPPP != NULL )
   {
    for( pointArrayPntP = *drapeArraysPPP ; pointArrayPntP < *drapeArraysPPP + *numDrapeArraysP ; ++pointArrayPntP )
      {
       if( (*pointArrayPntP)->pointsP != NULL ) free((*pointArrayPntP)->pointsP)  ;
      }
    free(*drapeArraysPPP)  ;
    *drapeArraysPPP = NULL ;
   }
 goto cleanup ;
}
/*==============================================================================*//**
* @memo   Snaps To Closest Break Line Then Drapes Orthognal From Break Line
* @doc    Snaps To Closest Break Line Then Drapes Orthognal From Break Line
* @param  dtmP                ==> Pointer To Tin object        
* @param  xPnt                  ==> Drape Point Start x Coordinate      
* @param  yPnt                  ==> Drape Point Start y Coordinate      
* @param  drapeArraysPPP     <== Pointer To Point Arrays Of Drape Points             
* @param  umDrapeArraysP     <== Number Of Drape Arrays              
* @return DTM_SUCCESS or DTM_ERROR
* @author Rob Cormack 25 May 2005 rob.cormack@bentley.com
* @version 
* @see None
*===============================================================================*/
BENTLEYDTM_EXPORT int bcdtmSite_snapToThenOrthogonalFromBreakLineDtmObject
(
 BC_DTM_OBJ *dtmP,                  /* ==> Tin Object Pointer                       */
 double xPnt,                        /* ==> Drape Point Start x Coordinate           */
 double yPnt,                        /* ==> Drape Point Start y Coordinate           */
 DTM_POINT_ARRAY  ***drapeArraysPPP, /* <== Pointer To Point Arrays Of Drape Points  */
 long   *numDrapeArraysP             /* <== Number Of Drape Arrays                   */
)
{
 int    ret=DTM_SUCCESS,sdof1,sdof2,dbg=DTM_TRACE_VALUE(0) ;
 long   loop,dtmFeature,drapeFlag,numDrapePts,processDrape,processBreak  ; 
 long   memDrapePts=0,memDrapePtsInc=1000,memDrapeArrays=0,memDrapeArraysInc=100 ;
 long   ofs1,ofs2,brkPnt1,brkPnt2,drapePnt1,drapePnt2 ;
 double zPnt,endX,endY,lastX,lastY,brkPntX,brkPntY,brkPntZ,drapePntX,drapePntY,drapePntZ ;
 double brkDistance,drapeDistance,radius,drapeAngle,brkAngle1,brkAngle2 ;
 DPoint3d    *drapePtsP=NULL ;
 DTM_POINT_ARRAY  *pointArrayPnt=NULL,**pointArrayPntP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Snapping To Then Draping Othogonal From Break Line") ;
    bcdtmWrite_message(0,0,0,"Tin Object      = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"xPnt            = %12.4lf",xPnt) ;
    bcdtmWrite_message(0,0,0,"yPnt            = %12.4lf",yPnt) ;
    bcdtmWrite_message(0,0,0,"drapeArraysPPP  = %p",*drapeArraysPPP) ;
    bcdtmWrite_message(0,0,0,"numDrapeArraysP = %8ld",*numDrapeArraysP) ;
   }
/*
** Initilaise
*/
 *numDrapeArraysP = 0 ;
 if( *drapeArraysPPP != NULL ) { free(*drapeArraysPPP) ; *drapeArraysPPP = NULL ; } 
/*
** Validate Tin Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP) != DTM_SUCCESS ) goto errexit ;
/*
** Snap Drape Start Point To Closest Tin Point If In Tolerance And Break Point
*/
 bcdtmFind_closestPointDtmObject(dtmP,xPnt,yPnt,&ofs1) ;
 if( bcdtmList_testForPointOnDtmFeatureTypeDtmObject(dtmP,DTMFeatureType::Breakline,ofs1,&dtmFeature))
   {
    drapeDistance = bcdtmMath_distance(xPnt,yPnt,pointAddrP(dtmP,ofs1)->x,pointAddrP(dtmP,ofs1)->y) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Closest Tin Point %6ld ** Distance = %20.15lf",ofs1,drapeDistance) ;
    if( drapeDistance < dtmP->ppTol ) 
      {
       xPnt = pointAddrP(dtmP,ofs1)->x ;
       yPnt = pointAddrP(dtmP,ofs1)->y ;
      }
   }
/*
** Check Start Point Is On Tin
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Start Point Is On Tin") ;
 if( bcdtmDrape_pointDtmObject(dtmP,xPnt,yPnt,&zPnt,&drapeFlag) != DTM_SUCCESS ) goto errexit ;
 if( ! drapeFlag )
   {
    if( bcdtmFind_findClosestHullLineDtmObject(dtmP,xPnt,yPnt,&zPnt,&drapeFlag,&ofs1,&ofs2)) goto errexit ;
   }
/*
** Find Closest Orthogonal Break Line To Start Drape Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Finding Closest Orthogonal Break Line") ;
 if( bcdtmSite_findClosestOrthogonalBreakLineDtmObject(dtmP,xPnt,yPnt,1,&processBreak,&brkPnt1,&brkPnt2,&brkPntX,&brkPntY,&brkPntZ,&brkDistance)) goto errexit ;
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"processBreak = %2ld",processBreak) ;
    bcdtmWrite_message(0,0,0,"brkPntX      = %15.8lf",brkPntX) ;
    bcdtmWrite_message(0,0,0,"brkPntY      = %15.8lf",brkPntY) ;
    bcdtmWrite_message(0,0,0,"brkPntZ      = %15.8lf",brkPntZ) ;
    bcdtmWrite_message(0,0,0,"brkDistance  = %15.8lf",brkDistance) ;
    bcdtmWrite_message(0,0,0,"brkPnt1      = %9ld",brkPnt1) ;
    bcdtmWrite_message(0,0,0,"brkPnt2      = %9ld",brkPnt2) ;
   }
/*
** Only Process If Closest Break Line Found
*/
 if( processBreak )
   { 
/*
**  Write Closest Break Point
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Closest Break Point  ** %12.5lf %12.5lf %12.5lf",brkPntX,brkPntY,brkPntZ) ;
/*
**  Calculate Othogonal From Angles For Break Line
*/
    brkAngle1 = bcdtmMath_getPointAngleDtmObject(dtmP,brkPnt1,brkPnt2) + DTM_PYE / 2.0 ;
    if( brkAngle1 > DTM_2PYE ) brkAngle1 -= DTM_2PYE ;
    brkAngle2 = brkAngle1 + DTM_PYE ;
    if( brkAngle2 > DTM_2PYE ) brkAngle2 -= DTM_2PYE ;
/*
**  Calculate Radius
*/
    radius = sqrt(dtmP->xRange*dtmP->xRange + dtmP->yRange*dtmP->yRange) ;
/*
**  Get Next Break Line From Start Break
*/
    for( loop = 0 ; loop < 2 ; ++loop )
      {
       if( ! loop ) drapeAngle = brkAngle1 ;
       else         drapeAngle = brkAngle2 ;
/*
**     Calculate Radial End Points
*/
       endX = brkPntX + radius * cos(drapeAngle) ; 
       endY = brkPntY + radius * sin(drapeAngle) ; 
/*
**     Find Next Closest Break Line
*/
       if( bcdtmSite_findClosestBreakLineIntersectionDtmObject(dtmP,brkPntX,brkPntY,endX,endY,brkPnt1,brkPnt2,&processBreak,&dtmFeature,&drapePnt1,&drapePnt2,&drapePntX,&drapePntY,&drapePntZ,&drapeDistance)) goto errexit ;
/*
**     Break Line Found
*/
       if( processBreak )
         {
/*
**        Allocate Initial Memory For Drape Points
*/     
          memDrapePts = memDrapePts + memDrapePtsInc ;
          drapePtsP = ( DPoint3d * ) malloc(memDrapePts * sizeof(DPoint3d)) ;
          if( drapePtsP == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }
/*
**        Store Break Point
*/
          numDrapePts = 0 ; 
          (drapePtsP+numDrapePts)->x = brkPntX ;
          (drapePtsP+numDrapePts)->y = brkPntY ;
          (drapePtsP+numDrapePts)->z = brkPntZ ;
          ++numDrapePts   ;
          if( dbg == 1 ) bcdtmWrite_message(0,0,0,"**** Break Point   ** %12.5lf %12.5lf %12.5lf",brkPntX,brkPntY,brkPntZ) ;
/*
**        Set Last Point
*/
          lastX = brkPntX ;
          lastY = brkPntY ;
/*
**        Store Drape Point
*/
          (drapePtsP+numDrapePts)->x = drapePntX ;
          (drapePtsP+numDrapePts)->y = drapePntY ;
          (drapePtsP+numDrapePts)->z = drapePntZ ;
          ++numDrapePts ;
          if( dbg == 1 ) bcdtmWrite_message(0,0,0,"**** First Drape Point   ** %12.5lf %12.5lf %12.5lf",drapePntX,drapePntY,drapePntZ) ;
/*
**        Loop Until No Intersections Can Be Determined With Break lines
*/
          processDrape = 1 ;
          while( processDrape )
            {
/*
**           Calculate Othogonal Angle And Radial End Points
*/
             drapeAngle = bcdtmMath_getPointAngleDtmObject(dtmP,drapePnt1,drapePnt2) + DTM_PYE / 2.0 ;
             if( drapeAngle > DTM_2PYE ) drapeAngle -= DTM_2PYE ;
             endX = drapePntX + radius * cos(drapeAngle) ; 
             endY = drapePntY + radius * sin(drapeAngle) ; 
             sdof1 = bcdtmMath_sideOf(pointAddrP(dtmP,drapePnt1)->x,pointAddrP(dtmP,drapePnt1)->y,pointAddrP(dtmP,drapePnt2)->x,pointAddrP(dtmP,drapePnt2)->y,endX,endY) ;
             sdof2 = bcdtmMath_sideOf(pointAddrP(dtmP,drapePnt1)->x,pointAddrP(dtmP,drapePnt1)->y,pointAddrP(dtmP,drapePnt2)->x,pointAddrP(dtmP,drapePnt2)->y,lastX,lastY) ;
             if( sdof1 == sdof2 )
               {
                drapeAngle = drapeAngle + DTM_PYE ;
                if( drapeAngle > DTM_2PYE ) drapeAngle -= DTM_2PYE ;
                endX = drapePntX + radius * cos(drapeAngle) ; 
                endY = drapePntY + radius * sin(drapeAngle) ; 
               }
/*
**           Set Last Point
*/
             lastX = brkPntX ;
             lastY = brkPntY ;
/*
**           Find Next Closest Break Line
*/
             if( bcdtmSite_findClosestBreakLineIntersectionDtmObject(dtmP,drapePntX,drapePntY,endX,endY,drapePnt1,drapePnt2,&processDrape,&dtmFeature,&drapePnt1,&drapePnt2,&drapePntX,&drapePntY,&drapePntZ,&drapeDistance)) goto errexit ;
             if( processDrape )
               {
/*
**              Write Next Drape Point
*/
                if( dbg == 1 ) bcdtmWrite_message(0,0,0,"**** Next  Drape Point   ** %12.5lf %12.5lf %12.5lf",drapePntX,drapePntY,drapePntZ) ;
/*
**              Check And Allocate memory
*/       
                if( numDrapePts == memDrapePts )
                  {
                   memDrapePts = memDrapePts + memDrapePtsInc ;
                   if( drapePtsP == NULL ) drapePtsP = ( DPoint3d * ) malloc(memDrapePts * sizeof(DPoint3d)) ;
                   else                    drapePtsP = ( DPoint3d * ) realloc(drapePtsP,memDrapePts*sizeof(DPoint3d)) ; 
                   if( drapePtsP == NULL )
                     {
                      bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                      goto errexit ;
                     }
                  } 
/*
**              Store Drape Point
*/
                (drapePtsP+numDrapePts)->x = drapePntX ;
                (drapePtsP+numDrapePts)->y = drapePntY ;
                (drapePtsP+numDrapePts)->z = drapePntZ ;
                ++numDrapePts ;
               }
            }
/*
**        Store Drape Points In Drape Array
*/
          if( *numDrapeArraysP == memDrapeArrays )
            {
             memDrapeArrays = memDrapeArrays + memDrapeArraysInc ;
             if( *drapeArraysPPP == NULL ) *drapeArraysPPP = ( DTM_POINT_ARRAY  ** ) malloc(memDrapeArrays * sizeof(DTM_POINT_ARRAY *)) ;
             else                          *drapeArraysPPP = ( DTM_POINT_ARRAY  ** ) realloc(**drapeArraysPPP,memDrapeArrays*sizeof(DTM_POINT_ARRAY *)) ; 
             if( *drapeArraysPPP == NULL )
               {
                bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                goto errexit ;
               } 
            }
/*
**        Allocate Memory For Point Array
*/
          pointArrayPnt = ( DTM_POINT_ARRAY * ) malloc( sizeof(DTM_POINT_ARRAY)) ;
          if( pointArrayPnt == NULL ) 
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            }  
/*
**        Populate Point Array
*/
          pointArrayPnt->pointsP   = drapePtsP ;  
          pointArrayPnt->numPoints = numDrapePts ;
/*
**        Store Point Array In Drape Array
*/
          *(*drapeArraysPPP+*numDrapeArraysP) = pointArrayPnt ;
          ++*numDrapeArraysP ;
/*
**        Null Out Memory Arrays
*/
          pointArrayPnt = NULL    ; 
          drapePtsP   = NULL    ; 
          numDrapePts = 0       ;
         }
      } 
   }
/*
** Reallocate Drape Arrays Memory
*/
 if( *numDrapeArraysP > 0 && *numDrapeArraysP < memDrapeArrays )
   {
    *drapeArraysPPP = ( DTM_POINT_ARRAY ** ) realloc(*drapeArraysPPP,*numDrapeArraysP*sizeof(DTM_POINT_ARRAY *)) ; 
   }
/*
** Clean Up
*/
 cleanup :
 if( drapePtsP   != NULL ) { free(drapePtsP) ; drapePtsP = NULL ; }
 if( pointArrayPnt != NULL )
   {
    if( pointArrayPnt->pointsP != NULL ) free(pointArrayPnt->pointsP) ;
    free(pointArrayPnt) ;
   } 
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Snapping To Then Draping Othogonal From Break Line Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Snapping To Then Draping Othogonal From Break Line Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR  ;
 if( *drapeArraysPPP != NULL )
   {
    for( pointArrayPntP = *drapeArraysPPP ; pointArrayPntP < *drapeArraysPPP + *numDrapeArraysP ; ++pointArrayPntP )
      {
       if( (*pointArrayPntP)->pointsP != NULL ) free((*pointArrayPntP)->pointsP)  ;
      }
    free(*drapeArraysPPP)  ;
    *drapeArraysPPP = NULL ;
   }
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmSite_snapToClosestBreakLineWithPositiveUserTagDtmObject
(
 BC_DTM_OBJ   *dtmP,                     /* ==> Pointer To Tin Object                    */
 double        xPnt,                      /* ==> x Coordiante Of Point                    */
 double        yPnt,                      /* ==> y Coordiante Of Point                    */
 long          *snapFoundP,               /* <== Snap Found <TRUE,FALSE>                  */
 long          *snapTypeP,                /* <== Snap Type <1=Break Point,2=Break Line>   */
 double        *xSnapP,                   /* <== x Coordiante Of Snap Point               */  
 double        *ySnapP,                   /* <== y Coordiante Of Snap Point               */  
 double        *zSnapP,                   /* <== y Coordiante Of Snap Point               */  
 double        *dSnapP,                   /* <== Distance To Snap Point                   */
 long          *dtmPnt1P,                 /* <== Tin Point Of Break Line End              */ 
 long          *dtmPnt2P,                 /* <== Tin Point Of Break Line End              */ 
 DTM_TIN_POINT_FEATURES **snapFeaturesPP, /* <== Pointer To Dtm Features At Snap Point    */
 long          *numSnapFeaturesP          /* <== Number Of Dtm Features At Snap Point     */
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ; 
 DTM_TIN_POINT_FEATURES *fListP ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Snapping To Closest Break Line With Positive User Tag") ;
    bcdtmWrite_message(0,0,0,"xPnt = %20.10lf",xPnt) ;
    bcdtmWrite_message(0,0,0,"yPnt = %20.10lf",yPnt) ;
   }
/*
** Initialise
*/
 *snapFoundP = FALSE ;
 *snapTypeP  = 0   ;
 *xSnapP     = 0.0 ;
 *ySnapP     = 0.0 ;
 *zSnapP     = 0.0 ;
 *dSnapP     = 0.0 ;
 *dtmPnt1P   = dtmP->nullPnt ;
 *dtmPnt2P   = dtmP->nullPnt ;
 if( *snapFeaturesPP != NULL ) { free(*snapFeaturesPP) ; *snapFeaturesPP = NULL ; }
 *numSnapFeaturesP = 0 ;
/*
** Find Closest Orthogonal Break Line To Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Finding Closest Orthogonal Break Line") ;
 if( bcdtmSite_findClosestOrthogonalBreakLineWithPositiveUserTagDtmObject(dtmP,xPnt,yPnt,snapFoundP,dtmPnt1P,dtmPnt2P,xSnapP,ySnapP,zSnapP,dSnapP)) goto errexit ;
 if( *snapFoundP )
   {
    *snapTypeP = *snapFoundP ;
    *snapFoundP = TRUE ;
   } 
/*
** Get Dtm Features For Snap Point
*/
 if( *snapFoundP == TRUE )
   {
    if( *snapTypeP == 1 ) { if( bcdtmList_getDtmFeaturesForPointDtmObject(dtmP,*dtmPnt1P,snapFeaturesPP,numSnapFeaturesP)) goto errexit ; }
    if( *snapTypeP == 2 ) { if( bcdtmList_getDtmFeaturesForLineDtmObject(dtmP,*dtmPnt1P,*dtmPnt2P,snapFeaturesPP,numSnapFeaturesP)) goto errexit ; } 
   } 
/*
** Write Out Results
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"snapFoundP       = %2ld",*snapFoundP) ;
    bcdtmWrite_message(0,0,0,"snapTypeP        = %2ld",*snapTypeP)  ;
    bcdtmWrite_message(0,0,0,"xSnapP           = %20.10lf",*xSnapP)  ;
    bcdtmWrite_message(0,0,0,"ySnapP           = %20.10lf",*ySnapP)  ;
    bcdtmWrite_message(0,0,0,"zSnapP           = %20.10lf",*zSnapP)  ;
    bcdtmWrite_message(0,0,0,"dSnapP           = %20.15lf",*dSnapP)  ;
    bcdtmWrite_message(0,0,0,"dtmPnt1P         = %9ld",*dtmPnt1P)   ;
    bcdtmWrite_message(0,0,0,"dtmPnt2P         = %9ld",*dtmPnt2P)   ;
    bcdtmWrite_message(0,0,0,"numSnapFeaturesP = %9ld",*numSnapFeaturesP) ;
    for( fListP = *snapFeaturesPP ; fListP < *snapFeaturesPP + *numSnapFeaturesP ; ++fListP )
      {
       bcdtmWrite_message(0,0,0,"snapFeature[%4ld] ** dtmFeature = %8ld ** Type = %4ld ** userTag = %10I64d ** nextPoint = %9ld priorPoint = %9ld",(long)(fListP-*snapFeaturesPP),fListP->dtmFeature,fListP->dtmFeatureType,fListP->userTag,fListP->nextPoint,fListP->priorPoint) ; 
       bcdtmList_writePointsForDtmFeatureDtmObject(dtmP,fListP->dtmFeature) ;
      }
   }

/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Snapping To Closest Break With Positive User Tag Line Completed") ;
 if( dbg && ret != DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Snapping To Closest Break With Positive User Tag Line Error") ;
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
BENTLEYDTM_Public int bcdtmSite_findClosestOrthogonalBreakLineDtmObject(BC_DTM_OBJ *dtmP,double pointX,double pointY,long scanOption,long *findTypeP,long *brkPnt1P,long *brkPnt2P,double *brkPntXP,double *brkPntYP,double *brkPntZP,double *brkDistanceP) 
/*
** This Function Finds The Closest Break Line To pointX,pointY
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   sPnt,nPnt,dtmFeature ;
 long   onLineFlag,testBreak ;
 double x,y,distance,xMin,xMax,yMin,yMax ;
 BC_DTM_FEATURE  *fP ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Finding Closest Orthogonal Break Line") ;
    bcdtmWrite_message(0,0,0,"dtmP        = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"pointX      = %12.5lf",pointX) ;
    bcdtmWrite_message(0,0,0,"pointY      = %12.5lf",pointY) ;
    bcdtmWrite_message(0,0,0,"scanOption  = %2ld",scanOption) ;
   }
/*
** Initialise
*/
 if( scanOption )
   {
    *findTypeP = 0 ;
    *brkPnt1P = dtmP->nullPnt ;
    *brkPnt2P = dtmP->nullPnt ;
    *brkPntXP = 0.0 ;
    *brkPntYP = 0.0 ;
    *brkPntZP = 0.0 ;
    *brkDistanceP = 0.0 ;
   }
/*
** Scan Break Line Features
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    fP = ftableAddrP(dtmP,dtmFeature) ;
    if( fP->dtmFeatureState == DTMFeatureState::Tin && fP->dtmFeatureType == DTMFeatureType::Breakline )
      {
       sPnt = fP->dtmFeaturePts.firstPoint ;
/*
**     Scan Dtm Feature For Drape Line Intersect
*/
       do
         { 
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sPnt,&nPnt)) goto errexit ;
          if( nPnt != dtmP->nullPnt )
            {
             testBreak = 1 ;
/*
**           Get Break Line Max And Mins
*/
             if( *findTypeP )
               {
                if( pointAddrP(dtmP,sPnt)->x <= pointAddrP(dtmP,nPnt)->x ) { xMin = pointAddrP(dtmP,sPnt)->x ; xMax = pointAddrP(dtmP,nPnt)->x ; }
                else                                                     { xMin = pointAddrP(dtmP,nPnt)->x ; xMax = pointAddrP(dtmP,sPnt)->x ; }
                if( pointAddrP(dtmP,sPnt)->y <= pointAddrP(dtmP,nPnt)->y ) { yMin = pointAddrP(dtmP,sPnt)->y ; yMax = pointAddrP(dtmP,nPnt)->y ; }
                else                                                     { yMin = pointAddrP(dtmP,nPnt)->y ; yMax = pointAddrP(dtmP,sPnt)->y ; }
                if( pointX < xMin - *brkDistanceP || pointX > xMax + *brkDistanceP || pointY < yMin - *brkDistanceP || pointY > yMax + *brkDistanceP ) testBreak = 0 ;
               }
/*
**           Test Break Line
*/
             if( testBreak )             
               {
                distance = bcdtmMath_distanceOfPointFromLine(&onLineFlag,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y,pointAddrP(dtmP,nPnt)->x,pointAddrP(dtmP,nPnt)->y,pointX,pointY,&x,&y) ;
                if( onLineFlag )
                  {
                   if( dbg ) bcdtmWrite_message(0,0,0,"onLine ** dtmFeature = %6ld ** sPnt = %8ld nPnt = %8ld  ** distance = %12.6lf",dtmFeature,sPnt,nPnt,distance) ;
/*
**                 Set Closest Break Line
*/
                   if( ! *findTypeP || distance < *brkDistanceP ) 
                     { 
                      *brkDistanceP = distance ;
                      *findTypeP = 2 ;
                      *brkPntXP  = x  ; 
                      *brkPntYP  = y  ; 
                      bcdtmMath_interpolatePointOnLineDtmObject(dtmP,*brkPntXP,*brkPntYP,brkPntZP,sPnt,nPnt) ;
                      if( dbg )
                        {   
                         bcdtmWrite_message(0,0,0,"Setting Closest Orthogonal Break Point") ;
                         bcdtmWrite_message(0,0,0,"sPnt = %6ld ** %12.4lf %12.4lf %10.4lf",sPnt,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y,pointAddrP(dtmP,sPnt)->z) ;
                         bcdtmWrite_message(0,0,0,"nPnt = %6ld ** %12.4lf %12.4lf %10.4lf",nPnt,pointAddrP(dtmP,nPnt)->x,pointAddrP(dtmP,nPnt)->y,pointAddrP(dtmP,nPnt)->z) ;
                         bcdtmWrite_message(0,0,0,"drapePnt = %12.4lf %12.4lf %10.4lf",x,y,*brkPntZP) ;
                         bcdtmWrite_message(0,0,0,"breakDistanceP = %12.6lf",*brkDistanceP) ;
                        }
                      *brkPnt1P  = sPnt ; 
                      *brkPnt2P  = nPnt ; 
                     }
                  }
               } 
            }
/*
**        Set Next Point
*/
          sPnt = nPnt ;
         } while ( sPnt != fP->dtmFeaturePts.firstPoint && sPnt != dtmP->nullPnt ) ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Finding Closest Orthogonal Break Line Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Finding Closest Orthogonal Break Line Error") ;
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
BENTLEYDTM_Public int bcdtmSite_findClosestOrthogonalBreakLineWithPositiveUserTagDtmObject(BC_DTM_OBJ *dtmP,double pointX,double pointY,long *findTypeP,long *brkPnt1P,long *brkPnt2P,double *brkPntXP,double *brkPntYP,double *brkPntZP,double *brkDistanceP) 
/*
** This Function Finds The Closest Break Line To pointX,pointY
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   sPnt,nPnt,dtmFeature ;
 long   onLineFlag,testBreak ;
 double x,y,distance,xMin,xMax,yMin,yMax ;
 BC_DTM_FEATURE  *fP ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Finding Closest Orthogonal Break Line With Positive User Tag") ;
    bcdtmWrite_message(0,0,0,"dtmP        = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"pointX      = %12.5lf",pointX) ;
    bcdtmWrite_message(0,0,0,"pointY      = %12.5lf",pointY) ;
   }
/*
** Initialise
*/
 *findTypeP = 0 ;
 *brkPnt1P = dtmP->nullPnt ;
 *brkPnt2P = dtmP->nullPnt ;
 *brkPntXP = 0.0 ;
 *brkPntYP = 0.0 ;
 *brkPntZP = 0.0 ;
 *brkDistanceP = 0.0 ;
/*
** Scan Break Line Features
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    fP = ftableAddrP(dtmP,dtmFeature) ;
    if( fP->dtmFeatureState == DTMFeatureState::Tin && fP->dtmFeatureType == DTMFeatureType::Breakline && fP->dtmUserTag > 0 )
      {
       sPnt = fP->dtmFeaturePts.firstPoint ;
/*
**     Scan Dtm Feature For Drape Line Intersect
*/
       do
         { 
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sPnt,&nPnt)) goto errexit ;
          if( nPnt != dtmP->nullPnt )
            {
             testBreak = 1 ;
/*
**           Get Break Line Max And Mins
*/
             if( *findTypeP )
               {
                if( pointAddrP(dtmP,sPnt)->x <= pointAddrP(dtmP,nPnt)->x ) { xMin = pointAddrP(dtmP,sPnt)->x ; xMax = pointAddrP(dtmP,nPnt)->x ; }
                else                                                       { xMin = pointAddrP(dtmP,nPnt)->x ; xMax = pointAddrP(dtmP,sPnt)->x ; }
                if( pointAddrP(dtmP,sPnt)->y <= pointAddrP(dtmP,nPnt)->y ) { yMin = pointAddrP(dtmP,sPnt)->y ; yMax = pointAddrP(dtmP,nPnt)->y ; }
                else                                                        { yMin = pointAddrP(dtmP,nPnt)->y ; yMax = pointAddrP(dtmP,sPnt)->y ; }
                if( pointX < xMin - *brkDistanceP || pointX > xMax + *brkDistanceP || pointY < yMin - *brkDistanceP || pointY > yMax + *brkDistanceP ) testBreak = 0 ;
               }
/*
**           Test Break Line
*/
             if( testBreak )             
               {
                distance = bcdtmMath_distanceOfPointFromLine(&onLineFlag,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y,pointAddrP(dtmP,nPnt)->x,pointAddrP(dtmP,nPnt)->y,pointX,pointY,&x,&y) ;
                if( onLineFlag )
                  {
                   if( dbg ) bcdtmWrite_message(0,0,0,"onLine ** dtmFeature = %6ld ** sPnt = %8ld nPnt = %8ld  ** distance = %12.6lf",dtmFeature,sPnt,nPnt,distance) ;
/*
**                 Set Closest Break Line
*/
                   if( ! *findTypeP || distance < *brkDistanceP ) 
                     { 
                      *brkDistanceP = distance ;
                      *findTypeP = 2 ;
                      *brkPntXP  = x  ; 
                      *brkPntYP  = y  ; 
                      bcdtmMath_interpolatePointOnLineDtmObject(dtmP,*brkPntXP,*brkPntYP,brkPntZP,sPnt,nPnt) ;
                      if( dbg )
                        {   
                         bcdtmWrite_message(0,0,0,"Setting Closest Orthogonal Break Point") ;
                         bcdtmWrite_message(0,0,0,"sPnt = %6ld ** %12.4lf %12.4lf %10.4lf",sPnt,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y,pointAddrP(dtmP,sPnt)->z) ;
                         bcdtmWrite_message(0,0,0,"nPnt = %6ld ** %12.4lf %12.4lf %10.4lf",nPnt,pointAddrP(dtmP,nPnt)->x,pointAddrP(dtmP,nPnt)->y,pointAddrP(dtmP,nPnt)->z) ;
                         bcdtmWrite_message(0,0,0,"drapePnt = %12.4lf %12.4lf %10.4lf",x,y,*brkPntZP) ;
                         bcdtmWrite_message(0,0,0,"breakDistanceP = %12.6lf",*brkDistanceP) ;
                        }
                      *brkPnt1P  = sPnt ; 
                      *brkPnt2P  = nPnt ; 
                     }
                  }
               } 
            }
/*
**        Set Next Point
*/
          sPnt = nPnt ;
         } while ( sPnt != fP->dtmFeaturePts.firstPoint && sPnt != dtmP->nullPnt ) ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Finding Closest Orthogonal Break Line With Positive User Tag Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Finding Closest Orthogonal Break Line With Positive User Tag Error") ;
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
BENTLEYDTM_Private int bcdtmSite_testForAnAngleProjectionExternalToTinHullDtmObject
(
 BC_DTM_OBJ *dtmP,
 long   dtmPnt1,
 long   dtmPnt2,
 double projectAngle, 
 long   *internalFlagP
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   clkPnt ;
 double startAngle=0.0,endAngle=0.0 ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Testing For An Angle Projection External To Tin Hull") ;
/*
** Initialise
*/
 *internalFlagP = 1 ;
/*
** Check Tin Point
*/
 if( dtmPnt1 != dtmP->nullPnt && dtmPnt2 == dtmP->nullPnt )
   {
    if( nodeAddrP(dtmP,dtmPnt1)->hPtr != dtmP->nullPnt )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Checking Hull Point For Projection Going External") ;
       if( ( clkPnt = bcdtmList_nextClkDtmObject(dtmP,dtmPnt1,nodeAddrP(dtmP,dtmPnt1)->hPtr)) < 0 ) goto errexit ;
       startAngle = bcdtmMath_getPointAngleDtmObject(dtmP,dtmPnt1,nodeAddrP(dtmP,dtmPnt1)->hPtr) ;
       endAngle   = bcdtmMath_getPointAngleDtmObject(dtmP,dtmPnt1,clkPnt) ;
       if( endAngle     < startAngle ) endAngle      += DTM_2PYE ;
       if( projectAngle < startAngle ) projectAngle  += DTM_2PYE ;
       if( projectAngle < startAngle ) *internalFlagP = 0 ;
       if( projectAngle > endAngle   ) *internalFlagP = 0 ;
      }
   }
/*
** Check Tin Line
*/
 if( dtmPnt1 != dtmP->nullPnt && dtmPnt2 != dtmP->nullPnt )
   {
    if( nodeAddrP(dtmP,dtmPnt1)->hPtr != dtmPnt2 || nodeAddrP(dtmP,dtmPnt2)->hPtr != dtmPnt1 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Checking Hull Line For Projection Going External") ;
       if( nodeAddrP(dtmP,dtmPnt1)->hPtr == dtmPnt2 )
         {
          startAngle = bcdtmMath_getPointAngleDtmObject(dtmP,dtmPnt1,dtmPnt2) ;
          endAngle   = bcdtmMath_getPointAngleDtmObject(dtmP,dtmPnt2,dtmPnt1) ;
         }
       if( nodeAddrP(dtmP,dtmPnt2)->hPtr == dtmPnt1 )
         {
          startAngle = bcdtmMath_getPointAngleDtmObject(dtmP,dtmPnt2,dtmPnt1) ;
          endAngle   = bcdtmMath_getPointAngleDtmObject(dtmP,dtmPnt1,dtmPnt2) ;
         }
       if( endAngle     < startAngle ) endAngle      += DTM_2PYE ;
       if( projectAngle < startAngle ) projectAngle  += DTM_2PYE ;
       if( projectAngle < startAngle ) *internalFlagP = 0 ;
       if( projectAngle > endAngle   ) *internalFlagP = 0 ;
      }
   }
/*
** Write Results
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Internal Projection = %2ld",*internalFlagP ) ;
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Testing For An Orthogonal To External Projection Completed") ;
 if( dbg && ret != DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Testing For An Orthogonal To External Projection Error") ;
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
BENTLEYDTM_Private int bcdtmSite_drapeProjectAtAngleToNextBreakLineDtmObject
(
 BC_DTM_OBJ *dtmP,
 double startX,
 double startY,
 double projectAngle,
 double deviationAngle, 
 long   sBrkPnt1,
 long   sBrkPnt2,
 long   *brkFndP,
 long   *brkTypeP,
 long   *brkPnt1P,
 long   *brkPnt2P,
 double *brkPntXP,
 double *brkPntYP,
 double *brkPntZP,
 double *brkDistanceP
)
/*
** This Function Finds The Closest Break Line To startX,startY
*/
{
 int    ret=DTM_SUCCESS,sdof,dbg=DTM_TRACE_VALUE(0) ;
 long   p1,p2,p3,np1,np2,np3,sBrkPnt3=0,breakPoint=0,processDrape,fndType,drapeType,startDrapeType ;
 double x=0.0,y=0.0,z,xls,yls,zls,xle,yle,lastX,lastY,radius ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Projecting At Angle To Next Break Line") ;
    bcdtmWrite_message(0,0,0,"xPnt           = %20.10lf",startX)  ;
    bcdtmWrite_message(0,0,0,"yPnt           = %20.10lf",startY)  ;
    bcdtmWrite_message(0,0,0,"sBrkPnt1       = %9ld",sBrkPnt1)  ;
    bcdtmWrite_message(0,0,0,"sBrkPnt2       = %9ld",sBrkPnt2)  ;
    bcdtmWrite_message(0,0,0,"projectAngle   = %12.10lf",projectAngle) ;
    bcdtmWrite_message(0,0,0,"deviationAngle = %12.10lf",deviationAngle) ;
   }
/*
** Initialise
*/
 *brkFndP  = FALSE ;
 *brkTypeP = 0 ;
 *brkPnt1P = dtmP->nullPnt ;
 *brkPnt2P = dtmP->nullPnt ;
 *brkPntXP = 0.0 ;
 *brkPntYP = 0.0 ;
 *brkPntZP = 0.0 ;
 *brkDistanceP = 0.0 ;
 projectAngle   = bcdtmMath_normaliseAngle(projectAngle) ;
 deviationAngle = fabs(deviationAngle) ;
/*
** Calculate Length Of Projection Vector
*/
 radius = sqrt((dtmP->xMax-dtmP->xMin)*(dtmP->xMax-dtmP->xMin)+(dtmP->yMax-dtmP->yMin)*(dtmP->yMax-dtmP->yMin)) ;
/*
** Set Direction For Drape
*/
 startDrapeType = 1 ;
 processDrape   = 1 ;
 if( sBrkPnt1 != dtmP->nullPnt && sBrkPnt2 != dtmP->nullPnt ) 
   {
/*
**  Draping From A Line
*/ 
    startDrapeType = 2 ;
    xle = startX + radius * cos(projectAngle)  ; 
    yle = startY + radius * sin(projectAngle)  ; 
/*
**  Get Drape Start Triangle
*/
    sdof = bcdtmMath_sideOf(pointAddrP(dtmP,sBrkPnt1)->x,pointAddrP(dtmP,sBrkPnt1)->y,pointAddrP(dtmP,sBrkPnt2)->x,pointAddrP(dtmP,sBrkPnt2)->y,xle,yle) ;
    if( sdof >= 0  ) { if( ( sBrkPnt3 = bcdtmList_nextAntDtmObject(dtmP,sBrkPnt1,sBrkPnt2))   < 0 ) goto errexit ; }
    else             { if( ( sBrkPnt3 = bcdtmList_nextClkDtmObject(dtmP,sBrkPnt1,sBrkPnt2)) < 0 ) goto errexit ; }
/*
**  Test For Valid Triangle
*/
    if( ! bcdtmList_testLineDtmObject(dtmP,sBrkPnt2,sBrkPnt3) ) processDrape = 0 ;
/*
**  Set Triangle Clockwise
*/
    else if( bcdtmMath_pointSideOfDtmObject(dtmP,sBrkPnt1,sBrkPnt2,sBrkPnt3) > 0 ) { np1 = sBrkPnt1 ; sBrkPnt1 = sBrkPnt2 ; sBrkPnt2 = np1 ; }
   }
/*
** Scan For Break Point Intersection At The Angle Increments
*/ 
 if( processDrape )
   {
/*
**  Initialise Drape Parameters
*/
    p1 = sBrkPnt1 ;
    p2 = sBrkPnt2 ;
    p3 = sBrkPnt3 ;
    drapeType = startDrapeType ;
    xls = startX ;
    yls = startY ;
    xle = startX + radius * cos(projectAngle)  ; 
    yle = startY + radius * sin(projectAngle)  ; 
    processDrape = 1 ;
/*
**  Scan To Break Point
*/
    while ( processDrape )
      {
       lastX = xls ;
       lastY = yls ; 
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"drapeType = %2ld p1 = %8ld p2 = %8ld p3 = %8ld",drapeType,p1,p2,p3) ;
       fndType = bcdtmDrape_getNextPointForDrapeDtmObject(dtmP,xls,yls,xle,yle,&drapeType,p1,p2,p3,&np1,&np2,&np3,&x,&y,&z)  ;
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"fndType = %2ld drapeType = %2ld np1 = %8ld np2 = %8ld np3 = %8ld ** %10.4lf %10.4lf %10.4lf",fndType,drapeType,np1,np2,np3,x,y,z) ;
/*
**     Error Detected
*/
       if( fndType == 4 ) goto errexit ;
/*
**     Next Drape Point Not Found
*/
       if( fndType == 3 ) processDrape = 0 ;
/*
**     Next Drape Point Found
*/ 
       if( fndType == 0 )
         {
          xls = x ;
          yls = y ;
          zls = z ;
          p1 = np1 ; 
          p2 = np2 ;
          p3 = np3 ;
/*
**        Check For Break Point
*/
          breakPoint = 0 ;
          if( drapeType == 1 ) breakPoint = bcdtmList_testForBreakPointDtmObject(dtmP,p1) ;
          if( drapeType == 2 ) breakPoint = bcdtmList_testForBreakLineDtmObject(dtmP,p1,p2) ;
          if( breakPoint )  processDrape = 0 ;
         }
/*
**     Scanned To End Point
*/
       if( fndType == 1 ) processDrape = 0 ;
/*
**     Drape Line Goes External To Tin Hull
*/
       if( fndType == 2 )  processDrape = 0 ;
      }
/*
**  Break Point Found
*/
    if( breakPoint )
      {
       *brkFndP  = TRUE ; 
       *brkTypeP = 1  ;
       if( p2 != dtmP->nullPnt ) *brkTypeP = 2 ;
       *brkPntXP = x  ; 
       *brkPntYP = y  ; 
       if( p2 == dtmP->nullPnt ) *brkPntZP = pointAddrP(dtmP,p1)->z ; 
       else                     bcdtmMath_interpolatePointOnLineDtmObject(dtmP,*brkPntXP,*brkPntYP,brkPntZP,p1,p2) ;
       *brkPnt1P  = p1 ; 
       *brkPnt2P  = p2 ; 
       *brkDistanceP = bcdtmMath_distance(startX,startY,x,y) ;
       if( dbg ) bcdtmWrite_message(0,0,0,"Break Intersection ** p1 = %9ld p2 = %9ld ** dist = %10.4lf ** %12.5lf %12.5lf %10.4lf",p1,p2,*brkDistanceP,x,y,*brkPntZP) ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Projecting At Angle To Next Break Line Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Projecting At Angle To Next Break Line Error") ;
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
BENTLEYDTM_Private int bcdtmSite_findClosestBreakLineEndPointWithinDeviationAngleDtmObject
(
 BC_DTM_OBJ *dtmP,
 double startX,
 double startY,
 double projectAngle,
 double deviationAngle, 
 long   sBrkPnt1,
 long   sBrkPnt2,
 long   *brkFndP,
 long   *brkTypeP,
 long   *brkPnt1P,
 long   *brkPnt2P,
 double *brkPntXP,
 double *brkPntYP,
 double *brkPntZP,
 double *brkDistanceP
)
/*
** This Function Finds The Closest Break Line To startX,startY
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   sPnt,nPnt,dtmFeature,findType ;
 double xMin,yMin,xMax,yMax,brkX,brkY ; 
 double distance,radius,leftX,leftY,rghtX,rghtY,leftAngle,rghtAngle;
 BC_DTM_FEATURE  *fP ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Finding Closest Break Line Point Within Deviation Angle") ;
    bcdtmWrite_message(0,0,0,"xPnt           = %20.10lf",startX)  ;
    bcdtmWrite_message(0,0,0,"yPnt           = %20.10lf",startY)  ;
    bcdtmWrite_message(0,0,0,"sBrkPnt1       = %9ld",sBrkPnt1)  ;
    bcdtmWrite_message(0,0,0,"sBrkPnt2       = %9ld",sBrkPnt2)  ;
    bcdtmWrite_message(0,0,0,"projectAngle   = %12.10lf",projectAngle) ;
    bcdtmWrite_message(0,0,0,"deviationAngle = %12.10lf",deviationAngle) ;
   }
/*
** Only Process For Deviation Angle Greater Than Zero
*/
 if( deviationAngle > 0.0 )
   {
/*
**  Calculate Deviation Angles
*/
    leftAngle = bcdtmMath_normaliseAngle(projectAngle-deviationAngle) ;
    rghtAngle = bcdtmMath_normaliseAngle(projectAngle+deviationAngle) ;
/*
**  Set Break Point Coordinates
*/
    if( *brkFndP == TRUE )
      {
       brkX = *brkPntXP ;
       brkY = *brkPntYP ;
      }
    else
      {
       radius = sqrt((dtmP->xMax-dtmP->xMin)*(dtmP->xMax-dtmP->xMin) + (dtmP->yMax-dtmP->yMin)*(dtmP->yMax-dtmP->yMin)) ;
       brkX   = startX + radius * cos(leftAngle) ;
       brkY   = startY + radius * sin(leftAngle) ;
      } 
/*
**  Calculate Deviation Triangles
*/
    radius  = bcdtmMath_distance(startX,startY,brkX,brkY)   ;
    leftX   = startX + radius * cos(leftAngle) ;
    leftY   = startY + radius * sin(leftAngle) ;
    rghtX   = startX + radius * cos(rghtAngle) ;
    rghtY   = startY + radius * sin(rghtAngle) ;
/*
**  Caculate Bounding Rectangle For Deviation Triangles
*/
    xMin = xMax = startX ;
    yMin = yMax = startY ;
    if( leftX < xMin ) xMin = leftX ;
    if( leftX > xMax ) xMax = leftX ;
    if( rghtX < xMin ) xMin = rghtX ;
    if( rghtX > xMax ) xMax = rghtX ;
    if( brkX  < xMin ) xMin = brkX  ;
    if( brkX  > xMax ) xMax = brkX  ;
    if( leftY < yMin ) yMin = leftY ;
    if( leftY > yMax ) yMax = leftY ;
    if( rghtY < yMin ) yMin = rghtY ;
    if( rghtY > yMax ) yMax = rghtY ;
    if( brkY  < yMin ) yMin = brkY  ;
    if( brkY  > yMax ) yMax = brkY  ;
    if( dbg ) 
      {
       bcdtmWrite_message(0,0,0,"xMin = %12.5lf xMax = %12.5lf",xMin,xMax) ;
       bcdtmWrite_message(0,0,0,"yMin = %12.5lf yMax = %12.5lf",yMin,yMax) ;
      }
/*
**  Scan Tin For Break Points Within Deviation Triangles
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For Break Points Within Deviation Triangles") ;
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       fP = ftableAddrP(dtmP,dtmFeature) ;
       if( fP->dtmFeatureState == DTMFeatureState::Tin && fP->dtmFeatureType == DTMFeatureType::Breakline )
         {
          sPnt = fP->dtmFeaturePts.firstPoint ;
/*
**        Scan To End Point
*/
          do
            { 
             if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sPnt,&nPnt)) goto errexit ;
/*
**           Ignore Feature Point 
*/
             if( sPnt != sBrkPnt1 && sPnt != sBrkPnt2 )
               {
/*
**              Check For Point Internal To Bounding Rectangle
*/
                if( dbg == 2) bcdtmWrite_message(0,0,0,"Checking For Point Within Bounding Rectangle") ;
                if( ( pointAddrP(dtmP,sPnt)->x >= xMin && pointAddrP(dtmP,sPnt)->x <= xMax )  &&
                    ( pointAddrP(dtmP,sPnt)->y >= yMin && pointAddrP(dtmP,sPnt)->y <= yMax )      )
                  {
/*
**                 Write Point
*/
                   if( dbg  ) 
                     {
                      bcdtmWrite_message(0,0,0,"Checking For Point Within Deviation Triangles") ;
                      bcdtmWrite_message(0,0,0,"Point = %8ld hPtr = %9ld ** %12.5lf %12.5lf %12.5lf",sPnt,nodeAddrP(dtmP,sPnt)->hPtr,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y,pointAddrP(dtmP,sPnt)->z) ;
                     } 
/*
**                 Check For Break Point Internal To Deviation Triangles
*/
                   if( ( bcdtmMath_sideOf(startX,startY,brkX,brkY,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y)   <= 0 &&
                         bcdtmMath_sideOf(brkX,brkY,leftX,leftY,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y)     <= 0 &&
                         bcdtmMath_sideOf(leftX,leftY,startX,startY,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y) <= 0    ) ||
                       ( bcdtmMath_sideOf(startX,startY,brkX,brkY,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y)   >= 0 &&
                         bcdtmMath_sideOf(brkX,brkY,rghtX,rghtY,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y)     >= 0 &&
                         bcdtmMath_sideOf(rghtX,rghtY,startX,startY,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y) >= 0    )      )
                     {
                      distance = bcdtmMath_distance(startX,startY,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y) ;
                      if( *brkFndP == FALSE  || distance < *brkDistanceP  )
                        {
/*
**                       Check Line Joining Start Point To Break Point Does Not Intersect Tin Hull  
*/
                         if( dbg ) bcdtmWrite_message(0,0,0,"Break Point[%9ld] ** hPtr = %8ld ** %12.5lf %12.5lf %10.4lf",sPnt,nodeAddrP(dtmP,sPnt)->hPtr,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y,pointAddrP(dtmP,sPnt)->z ) ;
                         if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Intercept With Tin Hull") ;
                         if( bcdtmSite_checkIfDrapeLineIntersectsTinHullDtmObject(dtmP,sBrkPnt1,sBrkPnt2,startX,startY,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y,&findType)) goto errexit ;
                         if( dbg ) bcdtmWrite_message(0,0,0,"Intecept With Tin Hull = %2ld",findType) ;
                         if( findType == 0 ) 
                           { 
                            if( dbg ) bcdtmWrite_message(0,0,0,"**** Setting Closest Intersection With Break Line Vertice") ;
                            *brkFndP  = TRUE ;
                            *brkTypeP = 1 ;
                            *brkPntXP = pointAddrP(dtmP,sPnt)->x  ; 
                            *brkPntYP = pointAddrP(dtmP,sPnt)->y  ; 
                            *brkPntZP = pointAddrP(dtmP,sPnt)->z  ;
                            *brkPnt1P = sPnt ; 
                            *brkPnt2P = dtmP->nullPnt ; 
                            *brkDistanceP = distance ;
                           }
                        }
                     }
                  }
               }                   
/*
**           Set To Next Feature Point
*/
             sPnt = nPnt ;
            } while ( sPnt != fP->dtmFeaturePts.firstPoint && sPnt != dtmP->nullPnt ) ;
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
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Finding Closest Break Line Point Within Deviation Angle Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Finding Closest Break Line Point Within Deviation Angle Error") ;
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
BENTLEYDTM_Private int bcdtmSite_checkIfDrapeLineIntersectsTinHullDtmObject
(
 BC_DTM_OBJ *dtmP,
 long   sBrkPnt1,
 long   sBrkPnt2,
 double startX,
 double startY,
 double endX,
 double endY,
 long  *intersectHullP
)
/*
** This Function Checks If The Drape Line Intersects The Tin Hull
*/
{
 int    ret=DTM_SUCCESS,sdof,dbg=DTM_TRACE_VALUE(0) ;
 long   p1,p2,p3,np1,np2,np3,antPnt,sBrkPnt3=0 ;
 long   processDrape,fndType,drapeType,startDrapeType ;
 double x=0.0,y=0.0,z=0.0,xls,yls,zls,lastX,lastY,radius,vectorAngle,clkAngle,antAngle ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Checking If Drape Line Intersects Tin Hull") ;
    bcdtmWrite_message(0,0,0,"sBrkPnt1  = %9ld",sBrkPnt1)  ;
    bcdtmWrite_message(0,0,0,"sBrkPnt2  = %9ld",sBrkPnt2)  ;
    bcdtmWrite_message(0,0,0,"startX    = %12.5lf",startX)  ;
    bcdtmWrite_message(0,0,0,"startY    = %12.5lf",startY)  ;
    bcdtmWrite_message(0,0,0,"endX      = %12.5lf",endX)  ;
    bcdtmWrite_message(0,0,0,"endY      = %12.5lf",endY)  ;
   }
/*
** Initialise
*/
 *intersectHullP = 0 ;
/*
** Calculate Length Of  Vector
*/
 radius = bcdtmMath_distance(startX,startY,endX,endY) ;
/*
** Set Direction For Drape
*/
 processDrape   = 1 ;
 startDrapeType = 1 ;
/*
** Draping From A Hull Break Point
*/
 if( sBrkPnt1 != dtmP->nullPnt && sBrkPnt2 == dtmP->nullPnt && nodeAddrP(dtmP,sBrkPnt1)->hPtr != dtmP->nullPnt ) 
   {
    if(( antPnt = bcdtmList_nextClkDtmObject(dtmP,sBrkPnt1,nodeAddrP(dtmP,sBrkPnt1)->hPtr)) < 0 ) goto errexit ;
    vectorAngle = bcdtmMath_getAngle(startX,startY,endX,endY) ;
    clkAngle    = bcdtmMath_getPointAngleDtmObject(dtmP,sBrkPnt1,nodeAddrP(dtmP,sBrkPnt1)->hPtr) ;
    antAngle    = bcdtmMath_getPointAngleDtmObject(dtmP,sBrkPnt1,antPnt) ;
    if( clkAngle    < antAngle ) clkAngle    += DTM_2PYE ;
    if( vectorAngle < antAngle ) vectorAngle += DTM_2PYE ;
    if( vectorAngle > antAngle && vectorAngle < clkAngle )
      {
       processDrape = 0 ;
       *intersectHullP = 1 ;
      }
   }
/*
** Draping From A Break Line
*/
 if( sBrkPnt1 != dtmP->nullPnt && sBrkPnt2 != dtmP->nullPnt ) 
   {
/*
**  Draping From A Line
*/ 
    startDrapeType = 2 ;
/*
**  Get Drape Start Triangle
*/
    sdof = bcdtmMath_sideOf(pointAddrP(dtmP,sBrkPnt1)->x,pointAddrP(dtmP,sBrkPnt1)->y,pointAddrP(dtmP,sBrkPnt2)->x,pointAddrP(dtmP,sBrkPnt2)->y,endX,endY) ;
    if( sdof >= 0  ) { if( ( sBrkPnt3 = bcdtmList_nextAntDtmObject(dtmP,sBrkPnt1,sBrkPnt2))   < 0 ) goto errexit ; }
    else             { if( ( sBrkPnt3 = bcdtmList_nextClkDtmObject(dtmP,sBrkPnt1,sBrkPnt2)) < 0 ) goto errexit ; }
/*
**  Test For Valid Triangle
*/
    if( ! bcdtmList_testLineDtmObject(dtmP,sBrkPnt2,sBrkPnt3) ) processDrape = 0 ;
/*
**  Set Triangle Clockwise
*/
    else if( bcdtmMath_pointSideOfDtmObject(dtmP,sBrkPnt1,sBrkPnt2,sBrkPnt3) > 0 ) { np1 = sBrkPnt1 ; sBrkPnt1 = sBrkPnt2 ; sBrkPnt2 = np1 ; }
   }
/*
** Scan For Intersection With Tin Hull
*/ 
 if( processDrape )
   {
/*
**  Initialise Drape Parameters
*/
    p1 = sBrkPnt1 ;
    p2 = sBrkPnt2 ;
    p3 = sBrkPnt3 ;
    drapeType = startDrapeType ;
    xls = startX ;
    yls = startY ;
/*
**  Scan To Break Point
*/
    while ( processDrape )
      {
       lastX = xls ;
       lastY = yls ; 
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"drapeType = %2ld p1 = %8ld p2 = %8ld p3 = %8ld",drapeType,p1,p2,p3) ;
       fndType = bcdtmDrape_getNextPointForDrapeDtmObject(dtmP,xls,yls,endX,endY,&drapeType,p1,p2,p3,&np1,&np2,&np3,&x,&y,&z)  ;
       if( dbg == 1 ) bcdtmWrite_message(0,0,0,"fndType = %2ld drapeType = %2ld np1 = %8ld np2 = %8ld np3 = %8ld ** %10.4lf %10.4lf %10.4lf",fndType,drapeType,np1,np2,np3,x,y,z) ;
/*
**     Error Detected
*/
       if( fndType == 4 ) goto errexit ;
/*
**     Next Drape Point Not Found
*/
       if( fndType == 3 ) processDrape = 0 ;
/*
**     Next Drape Point Found
*/ 
       if( fndType == 0 )
         {
          xls = x ;
          yls = y ;
          zls = z ;
          p1 = np1 ; 
          p2 = np2 ;
          p3 = np3 ;
         }
/*
**     Scanned To End Point
*/
       if( fndType == 1 ) processDrape = 0 ;
/*
**     Drape Line Goes External To Tin Hull
*/
       if( fndType == 2 )  
         {
          processDrape = 0 ;
          *intersectHullP = 0 ;
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
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Projecting At Angle To Next Break Line Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Projecting At Angle To Next Break Line Error") ;
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
BENTLEYDTM_Private int bcdtmSite_findClosestBreakLineIntersectionForProjectAngleDtmObject
(
 BC_DTM_OBJ *dtmP,
 double startX,
 double startY,
 double projectAngle,
 double deviationAngle, 
 long   sBrkPnt1,
 long   sBrkPnt2,
 long   *brkFndP,
 long   *brkTypeP,
 long   *brkPnt1P,
 long   *brkPnt2P,
 double *brkPntXP,
 double *brkPntYP,
 double *brkPntZP,
 double *brkDistanceP
)
/*
** This Function Finds The Closest Break Line To startX,startY
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   sPnt,nPnt,pPnt,firstFound=1,dtmFeature,breakLineEndPnt,checkIntersection ;
 double x,y,distance,endX,endY,length,includedAngle ;
 BC_DTM_FEATURE  *fP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Finding Closest Break Line Intersection At Projection Angle") ;
/*
** Initialise
*/
 *brkFndP  = FALSE ;
 *brkTypeP = 0 ;
 *brkPnt1P = dtmP->nullPnt ;
 *brkPnt2P = dtmP->nullPnt ;
 *brkPntXP = 0.0 ;
 *brkPntYP = 0.0 ;
 *brkPntZP = 0.0 ;
 *brkDistanceP = 0.0 ;
 deviationAngle = fabs(deviationAngle) ;
/*
** Check If Start Point Is Coincident With A Break Line Vertice
*/
 breakLineEndPnt = dtmP->nullPnt ;
 if( sBrkPnt1 != dtmP->nullPnt ) if( startX == pointAddrP(dtmP,sBrkPnt1)->x && startY == pointAddrP(dtmP,sBrkPnt1)->y ) breakLineEndPnt = sBrkPnt1 ;
 if( sBrkPnt2 != dtmP->nullPnt ) if( startX == pointAddrP(dtmP,sBrkPnt2)->x && startY == pointAddrP(dtmP,sBrkPnt2)->y ) breakLineEndPnt = sBrkPnt2 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"breakLineEndPnt = %9ld",breakLineEndPnt ) ;
/*
** Calculate Length Of Projection Vector
*/
 length = sqrt((dtmP->xMax-dtmP->xMin)*(dtmP->xMax-dtmP->xMin)+(dtmP->yMax-dtmP->yMin)*(dtmP->yMax-dtmP->yMin)) ;
 endX = startX + length * cos(projectAngle) ;
 endY = startY + length * sin(projectAngle) ;
/*
** Scan Tin For Break Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For Intersection With Break Line Segments") ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    fP = ftableAddrP(dtmP,dtmFeature) ;
    if( fP->dtmFeatureState == DTMFeatureState::Tin && fP->dtmFeatureType == DTMFeatureType::Breakline )
      {
       sPnt = fP->dtmFeaturePts.firstPoint ;
/*
**     Scan Dtm Feature For Drape Line Intersect
*/
       do
         { 
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sPnt,&nPnt)) goto errexit ;
          if( nPnt != dtmP->nullPnt )
            {
/*
**           Check Points Are Not Equal To Previous Break Line End Point
*/
             if( sPnt != breakLineEndPnt && nPnt != breakLineEndPnt )
               {
/*
**              Check Points Are Not The Same As Previous Break Line Segment
*/
                if( ( sPnt != sBrkPnt1 && nPnt != sBrkPnt2 ) && ( sPnt != sBrkPnt2 && nPnt != sBrkPnt1 ) )
                  {
/*
**                 Check For Intersection
*/
                   if( bcdtmMath_checkIfLinesIntersect(startX,startY,endX,endY,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y,pointAddrP(dtmP,nPnt)->x,pointAddrP(dtmP,nPnt)->y ))
                     {
                      bcdtmMath_normalIntersectCordLines(startX,startY,endX,endY,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y,pointAddrP(dtmP,nPnt)->x,pointAddrP(dtmP,nPnt)->y,&x,&y) ;
                      distance = bcdtmMath_distance(startX,startY,x,y) ;
                      if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Intersection Found ** Distance = %20.15lf **  %12.5lf %12.5lf ** %12.5lf %12.5lf",distance,startX,startY,x,y) ;
/*
**                    Set Closest Break Line
*/
                      if( firstFound || distance < *brkDistanceP ) 
                        {
                         if( dbg ) bcdtmWrite_message(0,0,0,"**** Setting Closest Intersection With Break Line Segment") ;
                         *brkFndP  = TRUE ; 
                         *brkTypeP = 2 ;
                         *brkPntXP = x  ; 
                         *brkPntYP = y  ; 
                         bcdtmMath_interpolatePointOnLineDtmObject(dtmP,*brkPntXP,*brkPntYP,brkPntZP,sPnt,nPnt) ;
                         *brkPnt1P  = sPnt ; 
                         *brkPnt2P  = nPnt ; 
                         *brkDistanceP = distance ;
                         firstFound = 0 ;
                        }
                     }
                  }
               }
            } 
/*
**        Set Next Feature Line
*/
          sPnt = nPnt ;
         } while ( sPnt != fP->dtmFeaturePts.firstPoint && sPnt != dtmP->nullPnt ) ;
      }
   }
/*
** Scan Tin For Possible Range Intersection With Break Line Vertices
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For Range Intersection With Break Line Vertices") ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    fP = ftableAddrP(dtmP,dtmFeature) ;
    if( fP->dtmFeatureState == DTMFeatureState::Tin && fP->dtmFeatureType == DTMFeatureType::Breakline )
      {
       sPnt = fP->dtmFeaturePts.firstPoint ;
/*
**     Scan Feature Points
*/
       do
         { 
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sPnt,&nPnt)) goto errexit ;
/*
**        Ignore Feature Point 
*/
          if( sPnt != sBrkPnt1 && sPnt != sBrkPnt2 && sPnt != *brkPnt1P && sPnt != *brkPnt2P  )
            {
/*
**           Check For Intersection
*/
             checkIntersection = TRUE ; 
             if( *brkFndP == TRUE )
               {
                checkIntersection = FALSE ; 
                if( fabs(pointAddrP(dtmP,sPnt)->x - *brkPntXP) <= *brkDistanceP && 
                    fabs(pointAddrP(dtmP,sPnt)->y - *brkPntYP) <= *brkDistanceP     )
                  {
                   checkIntersection = TRUE ; 
                  }
               }  
/*
**           Only Check If Line End Points Are Witnin Checking Range
*/
             if( checkIntersection == TRUE )
               { 
/*
**              Calculate Included Angle
*/
                includedAngle = bcdtmMath_calculateIncludedAngle(pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y,startX,startY,endX,endY) ;
                if( dbg ) bcdtmWrite_message(0,0,0,"includedAngle = %12.10lf ** deviationAngle = %12.5lf",includedAngle,deviationAngle) ;
/*
**              Calculate Distance Of Start Point To Break Line Vertice
*/
                if( includedAngle <= deviationAngle ) distance = bcdtmMath_distance(startX,startY,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y ) ;
/*
**              Set Closest Break Line
*/
                if( includedAngle <= deviationAngle && ( firstFound || distance < *brkDistanceP ) ) 
                  { 
                   if( dbg ) bcdtmWrite_message(0,0,0,"**** Setting Closest Intersection With Break Line Vertice") ;
                   *brkFndP  = TRUE ;
                   *brkTypeP = 1 ;
                   *brkPntXP = pointAddrP(dtmP,sPnt)->x  ; 
                   *brkPntYP = pointAddrP(dtmP,sPnt)->y  ; 
                   *brkPntZP = pointAddrP(dtmP,sPnt)->z  ;
                   *brkPnt1P = sPnt ; 
                   *brkPnt2P = dtmP->nullPnt ; 
                   *brkDistanceP = distance ;
                   firstFound = 0 ;
                  }
               }
            }
/*
**        Set To Next Feature Point
*/
          pPnt = sPnt ;
          sPnt = nPnt ;
         } while ( sPnt != fP->dtmFeaturePts.firstPoint && sPnt != dtmP->nullPnt ) ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Finding Closest Break Line At Projection Angle Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Finding Closest Break Line At Projection Angle Error") ;
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
BENTLEYDTM_EXPORT int bcdtmSite_writeAllDtmFeatureTypeToDtmFeatureArrayDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureType dtmFeatureType,DTM_FEATURE **featurePP,long *numFeatureP)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   numFeatureType,numPoints,dtmFeature ;
 DPoint3d    *pointsP=NULL ;
 BC_DTM_FEATURE  *featP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Writing DTM Feature Type To Feature Array") ;
/*
** Initialise
*/
 *numFeatureP = 0 ;
 if( *featurePP != NULL )
   {
    free(*featurePP) ;
    *featurePP = NULL ;
   }
/*
** Count Number Of Feature Types In Tin
*/
 numFeatureType = 0 ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures  ; ++dtmFeature )
   {
    featP = ftableAddrP(dtmP,dtmFeature) ;
    if( featP->dtmFeatureState == DTMFeatureState::Tin && featP->dtmFeatureType == dtmFeatureType ) ++numFeatureType ;
   } 
/*
** Allocate Memory
*/
 *featurePP = ( DTM_FEATURE * ) malloc( numFeatureType * sizeof( DTM_FEATURE ) ) ;
 if( *featurePP == NULL )
   {
    bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   } 
/*
** Populate Feature Array
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures  ; ++dtmFeature )
   {
    featP = ftableAddrP(dtmP,dtmFeature) ;
    if( featP->dtmFeatureState == DTMFeatureState::Tin && featP->dtmFeatureType == dtmFeatureType ) 
      {
/*
**     Copy Feature Points To Point Array
*/
       if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&pointsP,&numPoints)) goto errexit ;
/*
**     Store Points In Feature Array
*/
       (*featurePP+*numFeatureP)->dtmFeatureType = featP->dtmFeatureType ;
       (*featurePP+*numFeatureP)->userTag        = featP->dtmUserTag ;
       (*featurePP+*numFeatureP)->userFeatureId  = featP->dtmFeatureId ;
       (*featurePP+*numFeatureP)->pointsP        = pointsP ;
       (*featurePP+*numFeatureP)->numPoints      = numPoints ;
       ++*numFeatureP ;
       pointsP = NULL ;
      }
   } 
/*
** Clean Up
*/
 cleanup :
 if( pointsP != NULL ) free(pointsP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing DTM Feature Type To Feature Array Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Writing DTM Feature Type To Feature Array Error") ;
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
BENTLEYDTM_Private int bcdtmSite_testForOrthogonalToProjectionExternalToTinHullDtmObject
(
 BC_DTM_OBJ *dtmP,
 long   dtmPnt1,
 long   dtmPnt2,
 double rangeAngleStart, 
 double rangeAngleEnd, 
 long   *internalFlagP
)
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   clkPnt ;
 double startAngle=0.0,endAngle=0.0 ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Testing For An Orthogonal To External Projection") ;
/*
** Initialise
*/
 *internalFlagP = 1 ;
/*
** Check Tin Point
*/
 if( dtmPnt1 != dtmP->nullPnt && dtmPnt2 == dtmP->nullPnt )
   {
    if( nodeAddrP(dtmP,dtmPnt1)->hPtr != dtmP->nullPnt )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Checking Hull Point For Projection Going External") ;
       if( ( clkPnt = bcdtmList_nextClkDtmObject(dtmP,dtmPnt1,nodeAddrP(dtmP,dtmPnt1)->hPtr)) < 0 ) goto errexit ;
       startAngle = bcdtmMath_getPointAngleDtmObject(dtmP,dtmPnt1,nodeAddrP(dtmP,dtmPnt1)->hPtr) ;
       endAngle   = bcdtmMath_getPointAngleDtmObject(dtmP,dtmPnt1,clkPnt) ;
       if( endAngle        < startAngle ) endAngle        += DTM_2PYE ;
       if( rangeAngleStart < startAngle ) rangeAngleStart += DTM_2PYE ;
       if( rangeAngleEnd   < startAngle ) rangeAngleEnd   += DTM_2PYE ;
       if( rangeAngleStart > endAngle || rangeAngleEnd > endAngle ) *internalFlagP = 0 ;
       if( rangeAngleStart < startAngle || rangeAngleEnd < startAngle ) *internalFlagP = 0 ;
       if( rangeAngleStart > endAngle   || rangeAngleEnd > endAngle   ) *internalFlagP = 0 ;
      }
   }
/*
** Check Tin Line
*/
 if( dtmPnt1 != dtmP->nullPnt && dtmPnt2 != dtmP->nullPnt )
   {
    if( nodeAddrP(dtmP,dtmPnt1)->hPtr != dtmPnt2 || nodeAddrP(dtmP,dtmPnt2)->hPtr != dtmPnt1 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Checking Hull Line For Projection Going External") ;
       if( nodeAddrP(dtmP,dtmPnt1)->hPtr == dtmPnt2 )
         {
          startAngle = bcdtmMath_getPointAngleDtmObject(dtmP,dtmPnt1,dtmPnt2) ;
          endAngle   = bcdtmMath_getPointAngleDtmObject(dtmP,dtmPnt2,dtmPnt1) ;
         }
       if( nodeAddrP(dtmP,dtmPnt2)->hPtr == dtmPnt1 )
         {
          startAngle = bcdtmMath_getPointAngleDtmObject(dtmP,dtmPnt2,dtmPnt1) ;
          endAngle   = bcdtmMath_getPointAngleDtmObject(dtmP,dtmPnt1,dtmPnt2) ;
         }
       if( endAngle        < startAngle ) endAngle        += DTM_2PYE ;
       if( rangeAngleStart < startAngle ) rangeAngleStart += DTM_2PYE ;
       if( rangeAngleEnd   < startAngle ) rangeAngleEnd   += DTM_2PYE ;
       if( rangeAngleStart < startAngle || rangeAngleEnd < startAngle ) *internalFlagP = 0 ;
       if( rangeAngleStart > endAngle   || rangeAngleEnd > endAngle   ) *internalFlagP = 0 ;
      }
   }
/*
** Write Results
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Internal Projection = %2ld",*internalFlagP ) ;
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Testing For An Orthogonal To External Projection Completed") ;
 if( dbg && ret != DTM_SUCCESS )  bcdtmWrite_message(0,0,0,"Testing For An Orthogonal To External Projection Error") ;
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
BENTLEYDTM_Private int bcdtmSite_drapeAngleScanForOrthogonalBreakLineIntersectionDtmObject
(
 BC_DTM_OBJ *dtmP,
 double startX,
 double startY,
 double rangeAngleStart, 
 double rangeAngleEnd, 
 double deviationAngle, 
 long   sBrkPnt1,
 long   sBrkPnt2,
 long   *brkFndP,
 long   *brkTypeP,
 long   *brkPnt1P,
 long   *brkPnt2P,
 double *brkPntXP,
 double *brkPntYP,
 double *brkPntZP,
 double *brkDistanceP
)
{
 int    ret=DTM_SUCCESS,sdof1,dbg=DTM_TRACE_VALUE(0) ;
 long   n,p1,p2,p3,np1,np2,np3,numAngleInc,sBrkPnt3 ;
 long   drapeType,fndType,breakPoint=0,processDrape,startDrapeType,onLine=0,firstFound=1 ;
 double xls,yls,zls,xle,yle,radius,angle,midAngle,angleInc ;
 double x=0.0,y=0.0,z=0.0,distance=0.0,includedAngle1,includedAngle2,orthoAngle ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Drape Scanning For Orthogonal Break Line Intersection") ;
    bcdtmWrite_message(0,0,0,"startX          = %20.10lf",startX)  ;
    bcdtmWrite_message(0,0,0,"startY          = %20.10lf",startY)  ;
    bcdtmWrite_message(0,0,0,"sbrkPnt1        = %9ld",sBrkPnt1)   ;
    bcdtmWrite_message(0,0,0,"sbrkPnt2        = %9ld",sBrkPnt2)   ;
    bcdtmWrite_message(0,0,0,"rangeAngleStart = %12.10lf",rangeAngleStart) ;
    bcdtmWrite_message(0,0,0,"rangeAngleEnd   = %12.10lf",rangeAngleEnd) ;
    bcdtmWrite_message(0,0,0,"deviationAngle  = %12.10lf",deviationAngle) ;
   }
/*
** Initialise arguments
*/
 *brkFndP  = FALSE ;
 *brkTypeP = 0 ;
 *brkPnt1P = dtmP->nullPnt ;
 *brkPnt2P = dtmP->nullPnt ;
 *brkPntXP = 0.0 ;
 *brkPntYP = 0.0 ;
 *brkPntZP = 0.0 ;
 *brkDistanceP = 0.0 ;
 rangeAngleStart = bcdtmMath_normaliseAngle(rangeAngleStart) ;
 rangeAngleEnd   = bcdtmMath_normaliseAngle(rangeAngleEnd)   ;
 if( rangeAngleEnd <= rangeAngleStart ) rangeAngleEnd += DTM_2PYE ;
 deviationAngle = fabs(deviationAngle) ;
/*
** Initialise Scan Parameters
*/
 if( rangeAngleEnd < rangeAngleStart ) rangeAngleEnd += DTM_2PYE ;
 numAngleInc = 5 ;
 midAngle = bcdtmMath_normaliseAngle((rangeAngleStart+rangeAngleEnd)/ 2.0) ;
 angleInc = (rangeAngleEnd - rangeAngleStart) / (double)(numAngleInc) ;
 radius   = sqrt((dtmP->xMax-dtmP->xMin)*(dtmP->xMax-dtmP->xMin) + (dtmP->yMax-dtmP->yMin)*(dtmP->yMax-dtmP->yMin)) ;
 sBrkPnt3  = dtmP->nullPnt ;
 startDrapeType = 1 ;
/*
** Set Direction For Drape
*/
 if( sBrkPnt1 != dtmP->nullPnt && sBrkPnt2 != dtmP->nullPnt ) 
   {
/*
**  Draping From A Line
*/ 
    startDrapeType = 2 ;
    xle = startX + radius * cos(midAngle)  ; 
    yle = startY + radius * sin(midAngle)  ; 
/*
**  Get Drape Start Triangle
*/
    sdof1 = bcdtmMath_sideOf(pointAddrP(dtmP,sBrkPnt1)->x,pointAddrP(dtmP,sBrkPnt1)->y,pointAddrP(dtmP,sBrkPnt2)->x,pointAddrP(dtmP,sBrkPnt2)->y,xle,yle) ;
    if( sdof1 >= 0  ) { if( ( sBrkPnt3 = bcdtmList_nextAntDtmObject(dtmP,sBrkPnt1,sBrkPnt2))   < 0 ) goto errexit ; }
    else              { if( ( sBrkPnt3 = bcdtmList_nextClkDtmObject(dtmP,sBrkPnt1,sBrkPnt2)) < 0 ) goto errexit ; }
/*
**  Test For Valid Triangle
*/
    if( ! bcdtmList_testLineDtmObject(dtmP,sBrkPnt2,sBrkPnt3) ) processDrape = 0 ;
/*
**  Set Triangle Clockwise
*/
    else if( bcdtmMath_pointSideOfDtmObject(dtmP,sBrkPnt1,sBrkPnt2,sBrkPnt3) > 0 ) { np1 = sBrkPnt1 ; sBrkPnt1 = sBrkPnt2 ; sBrkPnt2 = np1 ; }
   }
/*
** Scan For Break Point Intersection At The Angle Increments
*/ 
 for( n = 0 ; n < numAngleInc + 1 ; ++n )
   {
    if     ( n == 0 )               angle = rangeAngleStart ;
    else if( n == numAngleInc + 1 ) angle = rangeAngleEnd ;
    else                            angle = rangeAngleStart + (double)(n) * angleInc ;
    if( dbg ) bcdtmWrite_message(0,0,0,"**** Scanning To Break Point At Angle %12.10lf",angle) ; 
/*
**  Initialise Drape Parameters
*/
    angle = bcdtmMath_normaliseAngle(angle) ;
    p1 = sBrkPnt1 ;
    p2 = sBrkPnt2 ;
    p3 = sBrkPnt3 ;
    drapeType = startDrapeType ;
    xls = startX ;
    yls = startY ;
    xle = startX + radius * cos(angle)  ; 
    yle = startY + radius * sin(angle)  ; 
    processDrape = 1 ;
/*
**  Scan To Break Point
*/
    while ( processDrape )
      {
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"drapeType = %2ld p1 = %8ld p2 = %8ld p3 = %8ld",drapeType,p1,p2,p3) ;
       fndType = bcdtmDrape_getNextPointForDrapeDtmObject(dtmP,xls,yls,xle,yle,&drapeType,p1,p2,p3,&np1,&np2,&np3,&x,&y,&z)  ;
       if( dbg == 2 ) bcdtmWrite_message(0,0,0,"fndType = %2ld drapeType = %2ld np1 = %8ld np2 = %8ld np3 = %8ld ** %10.4lf %10.4lf %10.4lf",fndType,drapeType,np1,np2,np3,x,y,z) ;
/*
**     Error Detected
*/
       if( fndType == 4 ) goto errexit ;
/*
**     Next Drape Point Not Found
*/
       if( fndType == 3 ) processDrape = 0 ;
/*
**     Next Drape Point Found
*/
       if( fndType == 0 )
         {
          xls = x ;
          yls = y ;
          zls = z ;
          p1 = np1 ; 
          p2 = np2 ;
          p3 = np3 ;
          breakPoint = 0 ;
          if( drapeType == 1 ) breakPoint = bcdtmList_testForBreakPointDtmObject(dtmP,p1) ;
          if( drapeType == 2 ) breakPoint = bcdtmList_testForBreakLineDtmObject(dtmP,p1,p2) ;
          if( breakPoint )  processDrape = 0 ;
         }
/*
**     Scanned To End Point
*/
       if( fndType == 1 ) processDrape = 0 ;
/*
**     Drape Line Goes External To Tin Hull
*/
       if( fndType == 2 )  processDrape = 0 ;
      }
/*
**  If Break Point Found Test For Orthogonal To Break Line
*/
    if( breakPoint )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Break Point Found p1 = %9ld p2 = %9ld ** x = %12.5lf y = %12.5lf z = %12.5lf",p1,p2,x,y,z) ;
/*
**     Break Line Found
*/
       if( p1 != dtmP->nullPnt && p2 != dtmP->nullPnt )
         {
          distance = bcdtmMath_distanceOfPointFromLine(&onLine,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,startX,startY,&x,&y) ;
          if( dbg ) bcdtmWrite_message(0,0,0,"onLine = %2ld distance = %12.5lf ** x = %12.5lf y = %12.5lf",onLine,distance,x,y) ;
/*
**        If No Orthogonal Intersection Check For Near Miss With Break Line End Points
*/ 
          if( ! onLine )
            {
/*
**           Calculate Included Angles
*/
             includedAngle1 = bcdtmMath_calculateIncludedAngle(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,startX,startY,x,y) ;
             includedAngle2 = bcdtmMath_calculateIncludedAngle(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,startX,startY,x,y) ;
             if( dbg ) bcdtmWrite_message(0,0,0,"includeAngle1 = %12.10lf includeAngle2 = %12.10lf",includedAngle1,includedAngle2) ; 
             if( includedAngle1 <= includedAngle2 && includedAngle1 <= deviationAngle )
               {
                onLine = 1 ;
                x = pointAddrP(dtmP,p1)->x ; 
                y = pointAddrP(dtmP,p1)->y ; 
                distance = bcdtmMath_distance(startX,startY,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y) ;
               }
             if( includedAngle2 <  includedAngle1 && includedAngle2 <= deviationAngle )
               {
                onLine = 1 ;
                x = pointAddrP(dtmP,p2)->x ; 
                y = pointAddrP(dtmP,p2)->y ; 
                distance = bcdtmMath_distance(startX,startY,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y) ;
               }
            }
         }
/*
**     Break Point Found
*/
       if( p1 != dtmP->nullPnt && p2 == dtmP->nullPnt )
         {
          includedAngle1 = bcdtmMath_calculateIncludedAngle(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,startX,startY,x,y) ;
          if( dbg ) bcdtmWrite_message(0,0,0,"includeAngle1 = %12.10lf",includedAngle1) ; 
          if( includedAngle1 <= deviationAngle )
            {
             onLine = 1 ;
             x = pointAddrP(dtmP,p1)->x ; 
             y = pointAddrP(dtmP,p1)->y ; 
             distance = bcdtmMath_distance(startX,startY,pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y) ;
            }
         }
/*
**    Orthogonal Intersection Found
*/
      if( onLine )
        {
         if( dbg ) bcdtmWrite_message(0,0,0,"Orthogonal Intersection Found") ;
/*
**       Check Orthogonal Angle Is Within Angle Range
*/
         orthoAngle = bcdtmMath_getAngle(startX,startY,x,y) ;
         if( orthoAngle < rangeAngleStart ) orthoAngle += DTM_2PYE ;
         if( dbg ) bcdtmWrite_message(0,0,0,"startAngle = %12.10lf orthoAngle = %12.10lf endAngle = %12.10lf",rangeAngleStart,orthoAngle,rangeAngleEnd) ;
         if( orthoAngle >= rangeAngleStart && orthoAngle <= rangeAngleEnd )
           {
/*
**          Set Closest Break Line
*/
            if( firstFound || distance < *brkDistanceP ) 
              {
               *brkFndP  = TRUE ; 
               *brkTypeP = 1  ;
               if( p2 != dtmP->nullPnt ) *brkTypeP = 2 ;
               *brkPntXP = x  ; 
               *brkPntYP = y  ; 
               if( p2 == dtmP->nullPnt ) *brkPntZP = pointAddrP(dtmP,p1)->z ; 
               else                     bcdtmMath_interpolatePointOnLineDtmObject(dtmP,*brkPntXP,*brkPntYP,brkPntZP,p1,p2) ;
               *brkPnt1P  = p1 ; 
               *brkPnt2P  = p2 ; 
               *brkDistanceP = distance ;
               firstFound = 0 ;
               if( dbg ) bcdtmWrite_message(0,0,0,"Intersection ** p1 = %9ld p2 = %9ld ** dist = %10.4lf ** %12.5lf %12.5lf %10.4lf",p1,p2,distance,x,y,*brkPntZP) ;
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
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Drape Scanning For Orthogonal Break Line Intersection Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Drape Scanning For Orthogonal Break Line Intersection Error") ;
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
BENTLEYDTM_Private int bcdtmSite_findClosestOrthogonalBreakLineIntersectionDtmObject
(
 BC_DTM_OBJ *dtmP,
 double startX,
 double startY,
 double rangeAngleStart, 
 double rangeAngleEnd, 
 double deviationAngle, 
 long   sBrkPnt1,
 long   sBrkPnt2,
 long   *brkFndP,
 long   *brkTypeP,
 long   *brkPnt1P,
 long   *brkPnt2P,
 double *brkPntXP,
 double *brkPntYP,
 double *brkPntZP,
 double *brkDistanceP
)
/*
** This Function Finds The Closest Break Line To startX,startY
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   sPnt,nPnt,onLine,firstFound=1,dtmFeature,breakLineEndPnt,validProjection ;
 long   checkForIntersection ;
 double x,y,distance,includedAngle1,includedAngle2,orthoAngle ;
 BC_DTM_FEATURE  *fP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Finding Closest Orthogonal Break Line Intersection") ;
/*
** Initialise
*/
/*
 *brkFndP  = FALSE ;
 *brkTypeP = 0 ;
 *brkPnt1P = dtmP->nullPnt ;
 *brkPnt2P = dtmP->nullPnt ;
 *brkPntXP = 0.0 ;
 *brkPntYP = 0.0 ;
 *brkPntZP = 0.0 ;
 *brkDistanceP = 0.0 ;
*/
 rangeAngleStart = bcdtmMath_normaliseAngle(rangeAngleStart) ;
 rangeAngleEnd   = bcdtmMath_normaliseAngle(rangeAngleEnd)   ;
 if( rangeAngleEnd <= rangeAngleStart ) rangeAngleEnd += DTM_2PYE ;
 deviationAngle = fabs(deviationAngle) ;
/*
** Check If Start Point Is Coincident With A Break Line Vertice
*/
 breakLineEndPnt = dtmP->nullPnt ;
 if( sBrkPnt1 != dtmP->nullPnt ) if( startX == pointAddrP(dtmP,sBrkPnt1)->x && startY == pointAddrP(dtmP,sBrkPnt1)->y ) breakLineEndPnt = sBrkPnt1 ;
 if( sBrkPnt2 != dtmP->nullPnt ) if( startX == pointAddrP(dtmP,sBrkPnt2)->x && startY == pointAddrP(dtmP,sBrkPnt2)->y ) breakLineEndPnt = sBrkPnt2 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"breakLineEndPnt = %9ld",breakLineEndPnt ) ;
/*
** Scan Tin For Break Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning For Orthogonal Intersection With Break Line Segments") ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures  ; ++dtmFeature )
   {
    fP = ftableAddrP(dtmP,dtmFeature) ;
    if( fP->dtmFeatureState == DTMFeatureState::Tin && fP->dtmFeatureType == DTMFeatureType::Breakline ) 
      {
       sPnt = fP->dtmFeaturePts.firstPoint ;
/*
**     Scan Dtm Feature For Drape Line Intersect
*/
       do
         { 
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sPnt,&nPnt)) goto errexit ;
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"dtmFeature = %4ld ** sPnt = %9ld nPnt = %6ld",dtmFeature,sPnt,nPnt) ; 
          if( nPnt != dtmP->nullPnt )
            {
/*
**           Check Points Are Not Equal To Previous Break Line End Point
*/
             if( sPnt != breakLineEndPnt && nPnt != breakLineEndPnt )
               {
/*
**              Check Points Are Not The Same As Previous Break Line Segment
*/
                if( ( sPnt != sBrkPnt1 && nPnt != sBrkPnt2 ) && ( sPnt != sBrkPnt2 && nPnt != sBrkPnt1 ) )
                  {
                   checkForIntersection = TRUE ;
/*
**                 Check Line End Points Are Closer Than Current Closest Distance
*/
                   if( *brkFndP == TRUE )
                     {
                      checkForIntersection = FALSE ;
                      if      ( fabs(pointAddrP(dtmP,sPnt)->x - startX ) <= *brkDistanceP ) checkForIntersection = TRUE ;
                      else if ( fabs(pointAddrP(dtmP,nPnt)->x - startX ) <= *brkDistanceP ) checkForIntersection = TRUE ;
                      else if ( fabs(pointAddrP(dtmP,sPnt)->y - startY ) <= *brkDistanceP ) checkForIntersection = TRUE ;
                      else if ( fabs(pointAddrP(dtmP,nPnt)->y - startY ) <= *brkDistanceP ) checkForIntersection = TRUE ;
                     } 
 
/*
**                 Check For Orthogonal Intersection
*/
                   if( checkForIntersection == TRUE )
                     {   
                      distance = bcdtmMath_distanceOfPointFromLine(&onLine,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y,pointAddrP(dtmP,nPnt)->x,pointAddrP(dtmP,nPnt)->y,startX,startY,&x,&y) ;
                      if( dbg == 2 ) bcdtmWrite_message(0,0,0,"onLine = %2ld distance = %12.5lf ** x = %12.5lf y = %12.5lf",onLine,distance,x,y) ;
/*
**                    If No Orthogonal Intersection Check For Near Miss With Break Line End Points
*/ 
                      if( ! onLine )
                        {
/*
**                       Calculate Included Angles
*/
                         includedAngle1 = bcdtmMath_calculateIncludedAngle(pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y,startX,startY,x,y) ;
                         includedAngle2 = bcdtmMath_calculateIncludedAngle(pointAddrP(dtmP,nPnt)->x,pointAddrP(dtmP,nPnt)->y,startX,startY,x,y) ;
                         if( dbg == 2 ) bcdtmWrite_message(0,0,0,"includeAngle1 = %12.10lf includeAngle2 = %12.10lf",includedAngle1,includedAngle2) ; 
                         if( includedAngle1 <= includedAngle2 && includedAngle1 <= deviationAngle )
                           {
                            onLine = 1 ;
                            x = pointAddrP(dtmP,sPnt)->x ; 
                            y = pointAddrP(dtmP,sPnt)->y ; 
                            distance = bcdtmMath_distance(startX,startY,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y) ;
                           }
                         if( includedAngle2 <  includedAngle1 && includedAngle2 <= deviationAngle )
                           {
                            onLine = 1 ;
                            x = pointAddrP(dtmP,nPnt)->x ; 
                            y = pointAddrP(dtmP,nPnt)->y ; 
                            distance = bcdtmMath_distance(startX,startY,pointAddrP(dtmP,nPnt)->x,pointAddrP(dtmP,nPnt)->y) ;
                           }
                        }
/*
**                    Orthogonal Intersection Found
*/
                      if( onLine )
                        {
/*
**                       Check Orthogonal Angle Is Within Angle Range
*/
                         orthoAngle = bcdtmMath_getAngle(startX,startY,x,y) ;
                         if( orthoAngle < rangeAngleStart ) orthoAngle += DTM_2PYE ;
                         if( dbg ) bcdtmWrite_message(0,0,0,"startAngle = %12.10lf projectAngle = %12.10lf endAngle = %12.10lf",rangeAngleStart,orthoAngle,rangeAngleEnd) ;
                         if( orthoAngle >= rangeAngleStart && orthoAngle <= rangeAngleEnd )
                           {
/*
**                          Check Projection Does Not Intersect Any Other Break Lines
*/
                            if( bcdtmSite_checkForValidOrthogonalProjectionDtmObject(dtmP,sBrkPnt1,sBrkPnt2,startX,startY,sPnt,nPnt,x,y,&validProjection)) goto errexit ;
                            if( validProjection )
                              {
/*
**                             Set Closest Break Line
*/
                               if( firstFound || distance < *brkDistanceP ) 
                                 {
                                  *brkFndP  = TRUE ; 
                                  *brkTypeP = 2 ;
                                  *brkPntXP = x  ; 
                                  *brkPntYP = y  ; 
                                  bcdtmMath_interpolatePointOnLineDtmObject(dtmP,*brkPntXP,*brkPntYP,brkPntZP,sPnt,nPnt) ;
                                  *brkPnt1P  = sPnt ; 
                                  *brkPnt2P  = nPnt ; 
                                  *brkDistanceP = distance ;
                                  firstFound = 0 ;
                                 }
                              }
                           }
                        }
                     }
                  }
               }
            } 
/*
**        Set Next Feature Line
*/
          sPnt = nPnt ;
         } while ( sPnt != fP->dtmFeaturePts.firstPoint && sPnt != dtmP->nullPnt ) ;
      }
   }
/*
** Check And Reset Find Type
*/
 if( *brkFndP == TRUE && *brkTypeP == 2 )
   {
    if( *brkPntXP == pointAddrP(dtmP,*brkPnt1P)->x && *brkPntYP == pointAddrP(dtmP,*brkPnt1P)->y )
      {
       *brkTypeP = 1 ;
       *brkPnt2P = dtmP->nullPnt ;
      }
    else if( *brkPntXP == pointAddrP(dtmP,*brkPnt2P)->x && *brkPntYP == pointAddrP(dtmP,*brkPnt2P)->y )
      {
       *brkTypeP = 1 ;
       *brkPnt1P = *brkPnt2P ;
       *brkPnt2P = dtmP->nullPnt ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Finding Closest Orthogonal Break Line Intersection Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Finding Closest Orthogonal Break Line Intersection Error") ;
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
BENTLEYDTM_Private int bcdtmSite_findNextClosestOrthogonalBreakLineDtmObject(BC_DTM_OBJ *dtmP,unsigned char *linesP,long brkPnt1,long brkPnt2,double brkPntX,double brkPntY,long checkReflection,double lastBrkPntX,double lastBrkPntY,long *findTypeP,long *dtmFeatureP,long *nextBrkPnt1P,long *nextBrkPnt2P,double *nextBrkPntXP,double *nextBrkPntYP,double *nextBrkPntZP,double *brkDistanceP)
/*
** This Function Finds The Closest Break Line To brkPntX,brkPntY
*/
{
 int    ret=DTM_SUCCESS,sdof1,sdof2,dbg=DTM_TRACE_VALUE(0) ;
 long   ofs1,ofs2,cbp,fbp,sPnt,nPnt,offset,firstFound=1,dtmFeature ;
 long   onLineFlag,validDrapeLine,processSegment ;
 double x,y,dn,ds,distance ;
 BC_DTM_FEATURE  *fP ;
/*
** Write Entry Message
*/
 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Finding Next Closest Orthogonal Break Line") ;
 if( dbg == 1 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"brkPnt1         = %8ld",brkPnt1) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"brkPnt2         = %8ld",brkPnt2) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"brkPntX         = %12.5lf",brkPntX) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"brkPntY         = %12.5lf",brkPntY) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"checkReflection = %8ld",checkReflection) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"lastBrkPntX     = %12.5lf",lastBrkPntX) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"lastBrkPntY     = %12.5lf",lastBrkPntY) ;
   }
/*
** Initialise
*/
 cbp = dtmP->nullPnt ;
 fbp = dtmP->nullPnt ;
 *findTypeP = 0 ;
 *dtmFeatureP = dtmP->nullPnt ;
 *nextBrkPnt1P = dtmP->nullPnt ;
 *nextBrkPnt2P = dtmP->nullPnt ;
 *nextBrkPntXP = 0.0 ;
 *nextBrkPntYP = 0.0 ;
 *nextBrkPntZP = 0.0 ;
 *brkDistanceP = 0.0 ;
/*
** Scan Break Lines
*/
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures  ; ++dtmFeature )
   {
    fP = ftableAddrP(dtmP,dtmFeature) ;
    if( fP->dtmFeatureState == DTMFeatureState::Tin && fP->dtmFeatureType == DTMFeatureType::Breakline ) 
      {
       sPnt = fP->dtmFeaturePts.firstPoint ;
/*
**     Scan Dtm Feature For Drape Line Intersect
*/
       do
         { 
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sPnt,&nPnt)) goto errexit ;
          if( nPnt != dtmP->nullPnt )
            {
/*
**           Check If Break Line Segment Has Been Previously Processed
*/
             processSegment = 1 ;
             if( linesP != NULL )
               {
                if( sPnt < nPnt ) { ofs1 = sPnt ; ofs2 = nPnt ; }
                else              { ofs1 = nPnt ; ofs2 = sPnt ; }
                if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,ofs1,ofs2)) goto errexit ;
                if( bcdtmFlag_testFlag(linesP,offset) ) processSegment = 0 ;
               }
/*
**           Check Break Line Segment Is Not Current Segment
*/
             if( processSegment )
               {
                if( sPnt == brkPnt1 && nPnt == brkPnt2 ) processSegment = 0 ;
                if( sPnt == brkPnt2 && nPnt == brkPnt1 ) processSegment = 0 ;
               }
/*
**           Determine Normal Distance To Break Line
*/
             if( processSegment )
               {
                distance = bcdtmMath_distanceOfPointFromLine(&onLineFlag,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y,pointAddrP(dtmP,nPnt)->x,pointAddrP(dtmP,nPnt)->y,brkPntX,brkPntY,&x,&y) ;
                if( onLineFlag )
                  {
                   if      ( x == pointAddrP(dtmP,sPnt)->x && y == pointAddrP(dtmP,sPnt)->y ) onLineFlag = 0 ;
                   else if ( x == pointAddrP(dtmP,nPnt)->x && y == pointAddrP(dtmP,nPnt)->y ) onLineFlag = 0 ;
                  }
                if( onLineFlag )
                  {
                   validDrapeLine = 1 ;
                   if( dbg == 1 ) bcdtmWrite_message(0,0,0,"**** Drape Line ** normalDistance = %12.5lf **  %12.5lf %12.5lf ** %12.5lf %12.5lf",distance,brkPntX,brkPntY,x,y) ;
/*
**                 Check For Reflection
*/
                   if( checkReflection )
                     {
                      sdof1 = bcdtmMath_sideOf(pointAddrP(dtmP,brkPnt1)->x,pointAddrP(dtmP,brkPnt1)->y,pointAddrP(dtmP,brkPnt2)->x,pointAddrP(dtmP,brkPnt2)->y,x,y) ;
                      sdof2 = bcdtmMath_sideOf(pointAddrP(dtmP,brkPnt1)->x,pointAddrP(dtmP,brkPnt1)->y,pointAddrP(dtmP,brkPnt2)->x,pointAddrP(dtmP,brkPnt2)->y,lastBrkPntX,lastBrkPntY) ;
                      if( sdof1 == sdof2 ) validDrapeLine = 0 ;
                     }  
/*
**                 Check Drape Line Does Not Intersect Other Break Lines
*/
                   if( validDrapeLine ) 
                     {
                      if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Non Intersecting Drape Line") ;
                      if( bcdtmSite_checkForValidOrthogonalToDrapeLineDtmObject(dtmP,brkPnt1,brkPnt2,brkPntX,brkPntY,sPnt,nPnt,x,y,&validDrapeLine)) goto errexit ;
                     }
/*
**                 Valid Drape Line Found
*/
                   if( validDrapeLine )
                     {
                      if( dbg == 1 ) bcdtmWrite_message(0,0,0,"==== Drape Line ** normalDistance = %12.5lf **  %12.5lf %12.5lf ** %12.5lf %12.5lf",distance,brkPntX,brkPntY,x,y) ;
/*
**                    Set Closest Break Line
*/
                      if( firstFound || distance < *brkDistanceP ) 
                        { 
                         *brkDistanceP = distance ;
                         *findTypeP = 2 ;
                         *nextBrkPntXP  = x  ; 
                         *nextBrkPntYP  = y  ; 
                         bcdtmMath_interpolatePointOnLineDtmObject(dtmP,*nextBrkPntXP,*nextBrkPntYP,nextBrkPntZP,sPnt,nPnt) ;
                         if( dbg )
                           {   
                            bcdtmWrite_message(0,0,0,"Setting Closest Break Point") ;
                            bcdtmWrite_message(0,0,0,"sPnt = %6ld ** %12.4lf %12.4lf %10.4lf",sPnt,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y,pointAddrP(dtmP,sPnt)->z) ;
                            bcdtmWrite_message(0,0,0,"nPnt = %6ld ** %12.4lf %12.4lf %10.4lf",nPnt,pointAddrP(dtmP,nPnt)->x,pointAddrP(dtmP,nPnt)->y,pointAddrP(dtmP,nPnt)->z) ;
                            bcdtmWrite_message(0,0,0,"drapePnt = %12.4lf %12.4lf %10.4lf",x,y,*nextBrkPntZP) ;
                           }
                         *nextBrkPnt1P  = sPnt ; 
                         *nextBrkPnt2P  = nPnt ; 
                         firstFound = 0 ;
                        }
                     }
                  }
               }
            } 
/*
**        Set Next Point
*/
          sPnt = nPnt ;
         } while ( sPnt != fP->dtmFeaturePts.firstPoint && sPnt != dtmP->nullPnt ) ;
      }
   }
/*
** Find Closest Break Point If Orthogonal To Break Line Not Found
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Finding Closest Break Point") ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures  ; ++dtmFeature )
   {
    fP = ftableAddrP(dtmP,dtmFeature) ;
    if( fP->dtmFeatureState == DTMFeatureState::Tin && fP->dtmFeatureType == DTMFeatureType::Breakline ) 
      {
       sPnt = fP->dtmFeaturePts.firstPoint ;
/*
**     Scan Dtm Feature For Drape Line Intersect
*/
       do
         { 
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sPnt,&nPnt)) goto errexit ;
          if( nPnt == dtmP->nullPnt ) if( bcdtmList_getPriorPointForDtmFeatureDtmObject(dtmP,dtmFeature,sPnt,&nPnt)) goto errexit ; 
          if( nPnt != dtmP->nullPnt )
            {
/*
**          Check If Break Line Segment Has Been Previously Processed
*/
             processSegment = 1 ;
             if( linesP != NULL )
               {
                if( sPnt < nPnt ) { ofs1 = sPnt ; ofs2 = nPnt ; }
                else              { ofs1 = nPnt ; ofs2 = sPnt ; }
                if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,ofs1,ofs2)) goto errexit ;
                if( bcdtmFlag_testFlag(linesP,offset) ) processSegment = 0 ;
               }
/*
**           Check Break Line Segment Is Not Current Segment
*/
             if( processSegment )
               {
                if( sPnt == brkPnt1 && nPnt == brkPnt2 ) processSegment = 0 ;
                if( sPnt == brkPnt2 && nPnt == brkPnt1 ) processSegment = 0 ;
               }
/*
**           Determine Normal Distance To Break Line
*/
             if( processSegment )
               {
                distance = bcdtmMath_distanceOfPointFromLine(&onLineFlag,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y,pointAddrP(dtmP,nPnt)->x,pointAddrP(dtmP,nPnt)->y,brkPntX,brkPntY,&x,&y) ;
                if( onLineFlag )
                  {
                   if      ( x == pointAddrP(dtmP,sPnt)->x && y == pointAddrP(dtmP,sPnt)->y ) onLineFlag = 0 ;
                   else if ( x == pointAddrP(dtmP,nPnt)->x && y == pointAddrP(dtmP,nPnt)->y ) onLineFlag = 0 ;
                  }
                if( ! onLineFlag )
                  {
                   validDrapeLine = 1 ;
                   if( dbg == 2 ) bcdtmWrite_message(0,0,0,"**** Drape Line ** normalDistance = %12.5lf **  %12.5lf %12.5lf ** %12.5lf %12.5lf",distance,brkPntX,brkPntY,x,y) ;
/*
**                 Check For Different Point
*/
                   if( checkReflection && distance < 0.000001 ) validDrapeLine = 0 ;   
/*
**                 Check For Reflection
*/
                   if( checkReflection )
                     {
                      sdof1 = bcdtmMath_sideOf(pointAddrP(dtmP,brkPnt1)->x,pointAddrP(dtmP,brkPnt1)->y,pointAddrP(dtmP,brkPnt2)->x,pointAddrP(dtmP,brkPnt2)->y,x,y) ;
                      sdof2 = bcdtmMath_sideOf(pointAddrP(dtmP,brkPnt1)->x,pointAddrP(dtmP,brkPnt1)->y,pointAddrP(dtmP,brkPnt2)->x,pointAddrP(dtmP,brkPnt2)->y,lastBrkPntX,lastBrkPntY) ;
                      if( sdof1 == sdof2 ) validDrapeLine = 0 ;
                     }  
/*
**                 Valid Drape Line Found
*/ 
                   if( validDrapeLine )
                     {
                      if( dbg == 2 ) bcdtmWrite_message(0,0,0,"==== Drape Line ** normalDistance = %12.5lf **  %12.5lf %12.5lf ** %12.5lf %12.5lf",distance,brkPntX,brkPntY,x,y) ;
/*
**                    Set Distance To Line End Point
*/ 
                      ds = bcdtmMath_distance(pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y,x,y) ;                        
                      dn = bcdtmMath_distance(pointAddrP(dtmP,nPnt)->x,pointAddrP(dtmP,nPnt)->y,x,y) ;                        
                      if( ds <= dn ) 
                        {
                         if( bcdtmMath_calculateIncludedAngle(x,y,brkPntX,brkPntY,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y) < 5.0 / 360.0 * DTM_2PYE )
                           {
                            x = pointAddrP(dtmP,sPnt)->x ;
                            y = pointAddrP(dtmP,sPnt)->y ;
                            cbp = sPnt ;
                            fbp = nPnt ;
                           }
                         else validDrapeLine = 0 ; 
                        }
                      else          
                        {
                         if(  bcdtmMath_calculateIncludedAngle(x,y,brkPntX,brkPntY,pointAddrP(dtmP,nPnt)->x,pointAddrP(dtmP,nPnt)->y) < 5.0 / 360.0 * DTM_2PYE ) 
                           {
                            x = pointAddrP(dtmP,nPnt)->x ;
                            y = pointAddrP(dtmP,nPnt)->y ;
                            cbp = nPnt ;
                            fbp = sPnt ;
                           }
                         else validDrapeLine = 0 ; 
                        } 
/*
**                    Check Drape Line Does Not Intersect Other Break Lines
*/
                      if( validDrapeLine ) 
                        {
                         if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Checking For Non Intersecting Drape Line ** %12.4lf %12.4lf ** %12.4lf %12.4lf",brkPntX,brkPntY,x,y) ;
                         if( bcdtmSite_checkForValidOrthogonalToDrapeLineDtmObject(dtmP,brkPnt1,brkPnt2,brkPntX,brkPntY,sPnt,nPnt,x,y,&validDrapeLine)) goto errexit ;
                        }
/*
**                    Set Closest Break Line
*/
                      if( validDrapeLine && ( firstFound || distance < *brkDistanceP ) )
                        { 
                         *brkDistanceP = distance ;
                         *findTypeP = 1 ;
                         *nextBrkPntXP  = x  ; 
                         *nextBrkPntYP  = y  ; 
                         bcdtmMath_interpolatePointOnLineDtmObject(dtmP,*nextBrkPntXP,*nextBrkPntYP,nextBrkPntZP,sPnt,nPnt) ;
                         if( dbg )
                           {   
                            bcdtmWrite_message(0,0,0,"Setting Closest Break Point") ;
                            bcdtmWrite_message(0,0,0,"sPnt = %6ld ** %12.4lf %12.4lf %10.4lf",sPnt,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y,pointAddrP(dtmP,sPnt)->z) ;
                            bcdtmWrite_message(0,0,0,"nPnt = %6ld ** %12.4lf %12.4lf %10.4lf",nPnt,pointAddrP(dtmP,nPnt)->x,pointAddrP(dtmP,nPnt)->y,pointAddrP(dtmP,nPnt)->z) ;
                            bcdtmWrite_message(0,0,0,"drapePnt = %12.4lf %12.4lf %10.4lf",x,y,*nextBrkPntZP) ;
                           }
                         *nextBrkPnt1P  = cbp ; 
                         *nextBrkPnt2P  = fbp ; 
                         firstFound = 0 ;
                        }
                     }
                  }
               }
            }
/*
**        Set Next Point
*/
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sPnt,&nPnt)) goto errexit ;
          sPnt = nPnt ;
         } while ( sPnt != fP->dtmFeaturePts.firstPoint && sPnt != dtmP->nullPnt ) ;
      }
   }
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Finding Closest Break Line Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Finding Closest Break Line Error") ;
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
BENTLEYDTM_Private int bcdtmSite_findClosestBreakLineIntersectionDtmObject(BC_DTM_OBJ *dtmP,double startX,double startY,double endX,double endY,long startPnt,long endPnt,long *findTypeP,long *dtmFeatureP,long *brkPnt1P,long *brkPnt2P,double *brkPntXP,double *brkPntYP,double *brkPntZP,double *brkDistanceP)
/*
** This Function Finds The Closest Break Line To startX,startY
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   sPnt,nPnt,pPnt,firstFound=1,dtmFeature,onLine,breakLineEndPnt,closeFlag ;
 double x,y,distance ;
 BC_DTM_FEATURE  *fP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Finding Closest Break Line Intersection") ;
/*
** Initialise
*/
 *findTypeP = 0 ;
 *dtmFeatureP = dtmP->nullPnt ;
 *brkPnt1P = dtmP->nullPnt ;
 *brkPnt2P = dtmP->nullPnt ;
 *brkPntXP = 0.0 ;
 *brkPntYP = 0.0 ;
 *brkPntZP = 0.0 ;
 *brkDistanceP = 0.0 ;
/*
** Check If Current Drape Point On Break Line End Point
*/
 breakLineEndPnt = dtmP->nullPnt ;
 if( startX == pointAddrP(dtmP,startPnt)->x && startY == pointAddrP(dtmP,startPnt)->y ) breakLineEndPnt = startPnt ;
 if( startX == pointAddrP(dtmP,endPnt)->x   && startY == pointAddrP(dtmP,endPnt)->y   ) breakLineEndPnt = endPnt ;
 if( dbg ) bcdtmWrite_message(0,0,0,"breakLineEndPnt = %9ld",breakLineEndPnt ) ;
/*
** Scan Tin For Break Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Break Line Segments") ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures  ; ++dtmFeature )
   {
    fP = ftableAddrP(dtmP,dtmFeature) ;
    if( fP->dtmFeatureState == DTMFeatureState::Tin && fP->dtmFeatureType == DTMFeatureType::Breakline ) 
      {
       sPnt = fP->dtmFeaturePts.firstPoint ;
/*
**     Scan Dtm Feature For Drape Line Intersect
*/
       do
         { 
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sPnt,&nPnt)) goto errexit ;
          if( nPnt != dtmP->nullPnt )
            {
/*
**           Check Points Are Not Equal To Previous Break Line End Point
*/
             if( sPnt != breakLineEndPnt && nPnt != breakLineEndPnt )
               {
/*
**              Check Points Are Not The Same As Previous Break Line Segment
*/
                if( ( sPnt != startPnt && nPnt != endPnt ) && ( sPnt != endPnt && nPnt != startPnt  ) )
                  {
/*
**                 Check For Intersection
*/
                   if( bcdtmMath_checkIfLinesIntersect(startX,startY,endX,endY,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y,pointAddrP(dtmP,nPnt)->x,pointAddrP(dtmP,nPnt)->y ))
                     {
                      bcdtmMath_normalIntersectCordLines(startX,startY,endX,endY,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y,pointAddrP(dtmP,nPnt)->x,pointAddrP(dtmP,nPnt)->y,&x,&y) ;
                      distance = bcdtmMath_distance(startX,startY,x,y) ;
                      if( dbg == 1 ) bcdtmWrite_message(0,0,0,"==== Drape Line ** normalDistance = %20.15lf **  %12.5lf %12.5lf ** %12.5lf %12.5lf",distance,startX,startY,x,y) ;
                   
/*
**                    Set Closest Break Line
*/
                      if( firstFound || distance < *brkDistanceP ) 
                        { 
                         *brkDistanceP = distance ;
                         *findTypeP = 2 ;
                         *brkPntXP  = x  ; 
                         *brkPntYP  = y  ; 
                         bcdtmMath_interpolatePointOnLineDtmObject(dtmP,*brkPntXP,*brkPntYP,brkPntZP,sPnt,nPnt) ;
                         *brkPnt1P  = sPnt ; 
                         *brkPnt2P  = nPnt ; 
                         firstFound = 0 ;
                        }
                     }
                  }
               }
            } 
/*
**        Set Next Feature Line
*/
          sPnt = nPnt ;
         } while ( sPnt != fP->dtmFeaturePts.firstPoint && sPnt != dtmP->nullPnt ) ;
      }
   }
/*
** Scan Tin For Possible Snap To Break Line Endpoints
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Break Line End Points") ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures  ; ++dtmFeature )
   {
    fP = ftableAddrP(dtmP,dtmFeature) ;
    if( fP->dtmFeatureState == DTMFeatureState::Tin && fP->dtmFeatureType == DTMFeatureType::Breakline ) 
      {
       sPnt = fP->dtmFeaturePts.firstPoint ;
/*
**     Check If Feature Closes
*/
       closeFlag = 0 ;
       do
         {
          if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sPnt,&nPnt)) goto errexit ;
          sPnt = nPnt ;
         } while ( sPnt != dtmP->nullPnt && sPnt != fP->dtmFeaturePts.firstPoint ) ;
       if( sPnt == fP->dtmFeaturePts.firstPoint ) closeFlag = 1 ; 
/*
**     If Feature Does Not Close Scan Dtm Feature For Drape Line Intersect With Feature End Points
*/
       if( ! closeFlag )
         {
          pPnt = dtmP->nullPnt ;
          sPnt = fP->dtmFeaturePts.firstPoint ;
          do
            { 
             if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sPnt,&nPnt)) goto errexit ;
/*
**           Check For sPnt Being An End Point
*/
             if( pPnt == dtmP->nullPnt || nPnt == dtmP->nullPnt )
               {
/*
**              Calculate Orthogonal Distance To Break Line End Point
*/
                distance = bcdtmMath_distanceOfPointFromLine(&onLine,startX,startY,endX,endY,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y,&x,&y) ;
/*
**              RobC 9/Nov/2005 Snap Tolerance To Break Line EndPoint Is Set To 0.01 Units
*/
                if( onLine && distance <= 0.01  )
                  {
/*
**                 Calculate Distance To Start Point
*/
                   distance = bcdtmMath_distance(startX,startY,x,y) ;
                   if( distance > 0.0 )
                     { 
/*
**                    Set Closest Break Line
*/
                      if( firstFound || distance < *brkDistanceP ) 
                        { 
                         *brkDistanceP = distance ;
                         *findTypeP = 2 ;
                         *brkPntXP  = pointAddrP(dtmP,sPnt)->x  ; 
                         *brkPntYP  = pointAddrP(dtmP,sPnt)->y  ; 
                         *brkPntZP  = pointAddrP(dtmP,sPnt)->z  ;
                         *brkPnt1P  = sPnt ; 
                         *brkPnt2P  = nPnt ; 
                         if( nPnt == dtmP->nullPnt ) *brkPnt2P = pPnt ;  
                         firstFound = 0 ;
                        }
                     }
                  }
               }
/*
**           Set Next Feature Line
*/
             pPnt = sPnt ;
             sPnt = nPnt ;
            } while ( sPnt != fP->dtmFeaturePts.firstPoint && sPnt != dtmP->nullPnt ) ;
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
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Finding Closest Break Line Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Finding Closest Break Line Error") ;
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
BENTLEYDTM_Private int bcdtmSite_checkForValidOrthogonalProjectionDtmObject
(
 BC_DTM_OBJ *dtmP,
 long   startPnt1,
 long   startPnt2,
 double startX,
 double startY,
 long   testPnt1,
 long   testPnt2,
 double testX,
 double testY,
 long *validProjectionP
)
/*
** This function checks for a valid projection line. 
** That is :-
** 1. It Does Not Go External To Tin
** 2. It Does Not Intersect Any Break line
**
*/
{
 int    ret=DTM_SUCCESS,sdof1,dbg=DTM_TRACE_VALUE(0) ;
// long   hp1,hp2,sPnt,nPnt,chkForTest,startPnt,testPnt,dtmFeature ;
// double ang1,ang2,ang3,xlmin,xlmax,ylmin,ylmax,xbmin,xbmax,ybmin,ybmax ;
// DTM_FEATURE_TABLE  *fP ;

 long   p1,p2,p3,np1,np2,np3,fndType,drapeType,breakPoint,processDrape ;
 double xi,yi,zi,xls,yls,zls,xle,yle ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Checking For Valid Orthogonal Projection") ;
    bcdtmWrite_message(0,0,0,"startPnt1  = %6ld startPnt2 = %6ld",startPnt1,startPnt2) ;
    if( startPnt1 != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"startPnt1  = %6ld %12.4lf %12.4lf %10.4lf",startPnt1,pointAddrP(dtmP,startPnt1)->x,pointAddrP(dtmP,startPnt1)->y,pointAddrP(dtmP,startPnt1)->z) ;
    if( startPnt2 != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"startPnt2  = %6ld %12.4lf %12.4lf %10.4lf",startPnt2,pointAddrP(dtmP,startPnt2)->x,pointAddrP(dtmP,startPnt2)->y,pointAddrP(dtmP,startPnt2)->z) ;
    bcdtmWrite_message(0,0,0,"startPoint = %12.5lf %12.5lf",startX,startY) ;
    bcdtmWrite_message(0,0,0,"testPnt1   = %6ld testPnt2  = %6ld",testPnt1,testPnt2) ;
    if( testPnt1  != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"testPnt1   = %6ld %12.4lf %12.4lf %10.4lf",testPnt1,pointAddrP(dtmP,testPnt1)->x,pointAddrP(dtmP,testPnt1)->y,pointAddrP(dtmP,testPnt1)->z) ;
    if( testPnt2  != dtmP->nullPnt ) bcdtmWrite_message(0,0,0,"testPnt2   = %6ld %12.4lf %12.4lf %10.4lf",testPnt2,pointAddrP(dtmP,testPnt2)->x,pointAddrP(dtmP,testPnt2)->y,pointAddrP(dtmP,testPnt2)->z) ;
    bcdtmWrite_message(0,0,0,"testPoint  = %12.5lf %12.5lf",testX,testY) ;
   }
/*
** Initialise
*/
 *validProjectionP = 1 ;
 xls = startX ;
 yls = startY ;
 xle = testX  ;
 yle = testY  ;
 p1  = startPnt1 ;
 p2  = startPnt2 ;
 p3  = dtmP->nullPnt ;
 processDrape = 1 ;
 drapeType = 1 ;
/*
** Set Scan Direction
*/
 if( p1 != dtmP->nullPnt && p2 != dtmP->nullPnt ) 
   {
/*
**  Set Drape Starting Type
*/
    drapeType = 2 ;
/*
**  Get Drape Start Triangle
*/
    sdof1 = bcdtmMath_sideOf(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,testX,testY) ;
    if( sdof1 >= 0  ) { if( ( p3 = bcdtmList_nextAntDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ; }
    else              { if( ( p3 = bcdtmList_nextClkDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ; }
/*
**  Test For Valid Triangle
*/
    if( ! bcdtmList_testLineDtmObject(dtmP,p2,p3) )
      {
       *validProjectionP = 0 ;
       processDrape = 0 ;
      }
/*
**  Set Triangle Clockwise
*/
    else if( bcdtmMath_pointSideOfDtmObject(dtmP,p1,p2,p3) > 0 ) { np1 = p1 ; p1 = p2 ; p2 = np1 ; }
   }
/*
**  Scan To End Point
*/
 while ( processDrape )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"drapeType = %2ld p1 = %8ld p2 = %8ld p3 = %8ld",drapeType,p1,p2,p3) ;
    fndType = bcdtmDrape_getNextPointForDrapeDtmObject(dtmP,xls,yls,xle,yle,&drapeType,p1,p2,p3,&np1,&np2,&np3,&xi,&yi,&zi)  ;
    if( dbg ) bcdtmWrite_message(0,0,0,"fndType = %2ld drapeType = %2ld np1 = %8ld np2 = %8ld np3 = %8ld ** %10.4lf %10.4lf %10.4lf",fndType,drapeType,np1,np2,np3,xi,yi,zi) ;
/*
**  Error Detected
*/
    if( fndType == 4 ) goto errexit ;
/*
**  Next Drape Point Not Found
*/
    if( fndType == 3 ) processDrape = 0 ;
/*
**  Check For Reaching Test Line
*/
    if( drapeType == 2 ) 
      {
       if( ( np1 == testPnt1 && np2 == testPnt2 ) || 
           ( np1 == testPnt2 && np2 == testPnt1 )) fndType = 1 ;
      }
/*
**  Next Drape Point Found
*/
    if( fndType == 0 )
      {
       xls = xi ; yls = yi ; zls = zi ;
       p1 = np1 ; p2 = np2 ; p3 = np3 ;
       breakPoint = 0 ;
       if( ( p1 != testPnt1 || p2 != testPnt2 ) && ( p1 != testPnt2 || p2 != testPnt1 ) )
         {  
          if( drapeType == 1 ) breakPoint = bcdtmList_testForBreakPointDtmObject(dtmP,p1) ;
          if( drapeType == 2 ) breakPoint = bcdtmList_testForBreakLineDtmObject(dtmP,p1,p2) ;
         }
       if( breakPoint ) { processDrape = 0 ; *validProjectionP = 0 ; }
      }
/*
**  Scanned To Test Point
*/
    if( fndType == 1 ) processDrape = 0 ;
/*
**  Drape Line Goes External To Tin Hull
*/
    if( fndType == 2 ) { processDrape = 0 ; *validProjectionP = 0 ; }
   }
/*
** Write Results
*/
 if( dbg &&   *validProjectionP) bcdtmWrite_message(0,0,0,"Projection Valid") ;
 if( dbg && ! *validProjectionP) bcdtmWrite_message(0,0,0,"Projection Invalid") ;
/*
** Write Results
*/
 if( dbg &&   *validProjectionP) bcdtmWrite_message(0,0,0,"Projection Valid") ;
 if( dbg && ! *validProjectionP) bcdtmWrite_message(0,0,0,"Projection Invalid") ;
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking For Valid Orthogonal Projection Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking For Valid Orthogonal Projection Error") ;
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
BENTLEYDTM_Private int bcdtmSite_checkForValidOrthogonalToDrapeLineDtmObject(BC_DTM_OBJ *dtmP,long lastPnt1,long lastPnt2,double lastX,double lastY,long testPnt1,long testPnt2,double testX,double testY,long *validDrapeLineP)
/*
** This function checks for a valid drape line. 
** That is :-
** 1. It Does Not Go External To Tin
** 2. It Does Not Intersect Any Break line
**
*/
{
 int    ret=DTM_SUCCESS,sdof1,sdof2,dbg=DTM_TRACE_VALUE(0) ;
 long   p1,p2,hp1,hp2,sPnt,nPnt,chkBreak,dtmFeature ;
 double ang1,ang2,ang3,xlmin,xlmax,ylmin,ylmax,xbmin,xbmax,ybmin,ybmax ;
 BC_DTM_FEATURE  *fP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Checking For Valid Orthogonal Break Line") ;
    bcdtmWrite_message(0,0,0,"lastPnt1 = %6ld lastPnt2 = %6ld ** last = %12.4lf,%12.4lf",lastPnt1,lastPnt2,lastX,lastY) ;
    bcdtmWrite_message(0,0,0,"testPnt1 = %6ld testPnt2 = %6ld ** test = %12.4lf,%12.4lf",testPnt1,testPnt2,testX,testY) ;
    bcdtmWrite_message(0,0,0,"lastPnt1 = %6ld %12.4lf %12.4lf %10.4lf",lastPnt1,pointAddrP(dtmP,lastPnt1)->x,pointAddrP(dtmP,lastPnt1)->y,pointAddrP(dtmP,lastPnt1)->z) ;
    bcdtmWrite_message(0,0,0,"lastPnt2 = %6ld %12.4lf %12.4lf %10.4lf",lastPnt2,pointAddrP(dtmP,lastPnt2)->x,pointAddrP(dtmP,lastPnt2)->y,pointAddrP(dtmP,lastPnt2)->z) ;
    bcdtmWrite_message(0,0,0,"testPnt1 = %6ld %12.4lf %12.4lf %10.4lf",testPnt1,pointAddrP(dtmP,testPnt1)->x,pointAddrP(dtmP,testPnt1)->y,pointAddrP(dtmP,testPnt1)->z) ;
    bcdtmWrite_message(0,0,0,"testPnt2 = %6ld %12.4lf %12.4lf %10.4lf",testPnt2,pointAddrP(dtmP,testPnt2)->x,pointAddrP(dtmP,testPnt2)->y,pointAddrP(dtmP,testPnt2)->z) ;
    if( lastX == pointAddrP(dtmP,lastPnt1)->x && lastY == pointAddrP(dtmP,lastPnt1)->y )
      {
       bcdtmWrite_message(0,0,0,"Last Intersect == Last Point 1") ;
      } 
    else
      {
       bcdtmWrite_message(0,0,0,"Last Intersect != Last Point 1") ;
      }
   }
/*
** Initialise
*/
 *validDrapeLineP = 1 ;
/*
** Check If Drape Line Runs Along Longitudinal Break Line
*/
 p1 = p2 = dtmP->nullPnt ;
 if( lastX == pointAddrP(dtmP,lastPnt1)->x && lastY == pointAddrP(dtmP,lastPnt1)->y ) p1 = lastPnt1 ;
 if( lastX == pointAddrP(dtmP,lastPnt2)->x && lastY == pointAddrP(dtmP,lastPnt2)->y ) p1 = lastPnt2 ;
 if( testX == pointAddrP(dtmP,testPnt1)->x && testY == pointAddrP(dtmP,testPnt1)->y ) p2 = testPnt1 ;
 if( testX == pointAddrP(dtmP,testPnt2)->x && testY == pointAddrP(dtmP,testPnt2)->y ) p2 = testPnt2 ;
 if( p1 != dtmP->nullPnt && p2 != dtmP->nullPnt )
   {
    if( bcdtmList_testForBreakLineDtmObject(dtmP,p1,p2)) *validDrapeLineP = 0 ;
   }
/*
** Test For Drape Line Going External To Tin Hull From Hull Line
*/
 if( *validDrapeLineP )
   {
    if( nodeAddrP(dtmP,lastPnt1)->hPtr == lastPnt2 || nodeAddrP(dtmP,lastPnt2)->hPtr == lastPnt1 )
      {
       if( nodeAddrP(dtmP,lastPnt1)->hPtr == lastPnt2 ) {  hp1 = lastPnt1 ; hp2 = lastPnt2 ; }
       else                                          {  hp1 = lastPnt2 ; hp2 = lastPnt1 ; }
       if( bcdtmMath_sideOf(pointAddrP(dtmP,hp1)->x,pointAddrP(dtmP,hp1)->y,pointAddrP(dtmP,hp2)->x,pointAddrP(dtmP,hp2)->y,testX,testY) <= 0 ) *validDrapeLineP = 0 ;
      }
    else if( nodeAddrP(dtmP,testPnt1)->hPtr == testPnt2 || nodeAddrP(dtmP,testPnt2)->hPtr == testPnt1 )
      {
       if( nodeAddrP(dtmP,testPnt1)->hPtr == testPnt2 ) {  hp1 = testPnt1 ; hp2 = testPnt2 ; }
       else                                          {  hp1 = testPnt2 ; hp2 = testPnt1 ; }
       if( bcdtmMath_sideOf(pointAddrP(dtmP,hp1)->x,pointAddrP(dtmP,hp1)->y,pointAddrP(dtmP,hp2)->x,pointAddrP(dtmP,hp2)->y,lastX,lastY) <= 0 ) *validDrapeLineP = 0 ;
      }
   }
/*
** Test For Drape Line Going External To Tin Hull From Hull Point
*/
 if( *validDrapeLineP )
   {
    if( nodeAddrP(dtmP,lastPnt1)->hPtr != dtmP->nullPnt && lastX == pointAddrP(dtmP,lastPnt1)->x && lastY == pointAddrP(dtmP,lastPnt1)->y )
      {
       hp1 = nodeAddrP(dtmP,lastPnt1)->hPtr ;
       if( ( hp2 = bcdtmList_nextClkDtmObject(dtmP,lastPnt1,hp1)) < 0 ) goto errexit ; 
       ang1 = bcdtmMath_getPointAngleDtmObject(dtmP,lastPnt1,hp2) ;
       ang2 = bcdtmMath_getAngle(pointAddrP(dtmP,lastPnt1)->x,pointAddrP(dtmP,lastPnt1)->y,testX,testY) ;
       ang3 = bcdtmMath_getPointAngleDtmObject(dtmP,lastPnt1,hp1) ;
       if( ang2 < ang1 ) ang2 += DTM_2PYE ;
       if( ang3 < ang1 ) ang3 += DTM_2PYE ;
       if( ang2 >= ang1 && ang2 <= ang3 )  *validDrapeLineP = 0 ;
      }
    else if( nodeAddrP(dtmP,lastPnt2)->hPtr != dtmP->nullPnt && lastX == pointAddrP(dtmP,lastPnt2)->x && lastY == pointAddrP(dtmP,lastPnt2)->y )
      {
       hp1 = nodeAddrP(dtmP,lastPnt2)->hPtr ;
       if( ( hp2 = bcdtmList_nextClkDtmObject(dtmP,lastPnt2,hp1)) < 0 ) goto errexit ; 
       ang1 = bcdtmMath_getPointAngleDtmObject(dtmP,lastPnt2,hp2) ;
       ang2 = bcdtmMath_getAngle(pointAddrP(dtmP,lastPnt2)->x,pointAddrP(dtmP,lastPnt2)->y,testX,testY) ;
       ang3 = bcdtmMath_getPointAngleDtmObject(dtmP,lastPnt2,hp1) ;
       if( ang2 < ang1 ) ang2 += DTM_2PYE ;
       if( ang3 < ang1 ) ang3 += DTM_2PYE ;
       if( ang2 >= ang1 && ang2 <= ang3 )  *validDrapeLineP = 0 ;
      }
   }
/*
** Set Bounding Rectangle For Test Drape Line
*/
 if( *validDrapeLineP )
   {
    if( lastX <= testX ) { xlmin = lastX ; xlmax = testX ; }
    else                 { xlmin = testX ; xlmax = lastX ; }
    if( lastY <= testY ) { ylmin = lastY ; ylmax = testY ; }
    else                 { ylmin = testY ; ylmax = lastY ; }
/*
** Set Last Point
*/
    p1 = lastPnt1 ;
    p2 = lastPnt2 ;
    if( lastX == pointAddrP(dtmP,lastPnt1)->x && lastY == pointAddrP(dtmP,lastPnt1)->y ) { p1 = lastPnt1 ; p2 = dtmP->nullPnt ; }
    if( lastX == pointAddrP(dtmP,lastPnt2)->x && lastY == pointAddrP(dtmP,lastPnt2)->y ) { p1 = lastPnt2 ; p2 = dtmP->nullPnt ; }
    lastPnt1 = p1 ;
    lastPnt2 = p2 ;
    if( dbg ) bcdtmWrite_message(0,0,0,"lastPnt1 = %6ld lastPnt2 = %6ld",lastPnt1,lastPnt2) ;
/*
** Set Test Point
*/
    p1 = testPnt1 ;
    p2 = testPnt2 ;
    if( testX == pointAddrP(dtmP,testPnt1)->x && testY == pointAddrP(dtmP,testPnt1)->y ) { p1 = testPnt1 ; p2 = dtmP->nullPnt ; }
    if( testX == pointAddrP(dtmP,testPnt2)->x && testY == pointAddrP(dtmP,testPnt2)->y ) { p1 = testPnt2 ; p2 = dtmP->nullPnt ; }
    testPnt1 = p1 ;
    testPnt2 = p2 ;
    if( dbg ) bcdtmWrite_message(0,0,0,"testPnt1 = %6ld testPnt2 = %6ld",testPnt1,testPnt2) ;
/*
**  Scan Break Lines
*/
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       fP = ftableAddrP(dtmP,dtmFeature) ;
        if( fP->dtmFeatureState == DTMFeatureState::Tin && fP->dtmFeatureType == DTMFeatureType::Breakline && fP->dtmUserTag > 0 )
         {
          sPnt = fP->dtmFeaturePts.firstPoint ;
/*
**        Scan Dtm Feature For Drape Line Intersect
*/
          do
            { 
             if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sPnt,&nPnt)) goto errexit ;
             if( nPnt != dtmP->nullPnt )
               {
/*
**              Check If Break Line Can Be Tested For Intersection
*/
                chkBreak = 1 ;
                if( lastPnt2 == dtmP->nullPnt && ( lastPnt1 == sPnt || lastPnt1 == nPnt )) chkBreak = 0 ;
                if( testPnt2 == dtmP->nullPnt && ( testPnt1 == sPnt || testPnt1 == nPnt )) chkBreak = 0 ;
                if( ( lastPnt1 == sPnt && lastPnt2 == nPnt ) ||  ( lastPnt1 == nPnt && lastPnt2 == sPnt ) ) chkBreak = 0 ;
                if( ( testPnt1 == sPnt && testPnt2 == nPnt ) ||  ( testPnt1 == nPnt && testPnt2 == sPnt ) ) chkBreak = 0 ;
/*
**              Check Break Line
*/
                if( chkBreak )
                  {
/* 
**                 Set Ranges
*/
                   if( pointAddrP(dtmP,sPnt)->x <= pointAddrP(dtmP,nPnt)->x ) { xbmin = pointAddrP(dtmP,sPnt)->x ; xbmax = pointAddrP(dtmP,nPnt)->x ; }
                   else                                               { xbmin = pointAddrP(dtmP,nPnt)->x ; xbmax = pointAddrP(dtmP,sPnt)->x ; }
                   if( pointAddrP(dtmP,sPnt)->y <= pointAddrP(dtmP,nPnt)->y ) { ybmin = pointAddrP(dtmP,sPnt)->y ; ybmax = pointAddrP(dtmP,nPnt)->y ; }
                   else                                               { ybmin = pointAddrP(dtmP,nPnt)->y ; ybmax = pointAddrP(dtmP,sPnt)->y ; }
/*
**                 Check For Overlap Of Bounding Rectangles
*/
                   if( xbmax >= xlmin && xbmin <= xlmax && ybmax >= ylmin && ybmin <= ylmax )
                     {
/*
**                    Check For Intersection
*/ 
                      sdof1 = bcdtmMath_sideOf(lastX,lastY,testX,testY,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y) ;
                      sdof2 = bcdtmMath_sideOf(lastX,lastY,testX,testY,pointAddrP(dtmP,nPnt)->x,pointAddrP(dtmP,nPnt)->y) ;
                      if( sdof1 != sdof2 )
                        {
                         sdof1 = bcdtmMath_sideOf(pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y,pointAddrP(dtmP,nPnt)->x,pointAddrP(dtmP,nPnt)->y,lastX,lastY) ;
                         sdof2 = bcdtmMath_sideOf(pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y,pointAddrP(dtmP,nPnt)->x,pointAddrP(dtmP,nPnt)->y,testX,testY) ;
/*
**                       Intersection Found
*/
                         if( sdof1 != sdof2 ) 
                           {
                            *validDrapeLineP = 0 ;
                            if( dbg )
                              {
                               bcdtmWrite_message(0,0,0,"Intersect Found With sPnt = %6ld nPnt = %6ld",sPnt,nPnt) ;
                               bcdtmWrite_message(0,0,0,"sPnt = %6ld %12.4lf %12.4lf %10.4lf",sPnt,pointAddrP(dtmP,sPnt)->x,pointAddrP(dtmP,sPnt)->y,pointAddrP(dtmP,sPnt)->z) ;
                               bcdtmWrite_message(0,0,0,"nPnt = %6ld %12.4lf %12.4lf %10.4lf",nPnt,pointAddrP(dtmP,nPnt)->x,pointAddrP(dtmP,nPnt)->y,pointAddrP(dtmP,nPnt)->z) ;
                              }
                           }
                        } 
                     }
                  }
               } 
/*
**           Set Next Point
*/
             sPnt = nPnt ;
            } while ( sPnt != fP->dtmFeaturePts.firstPoint && sPnt != dtmP->nullPnt ) ;
         }
      }
   }
/*
** Write Results
*/
 if( dbg &&   *validDrapeLineP) bcdtmWrite_message(0,0,0,"Valid Drape Line") ;
 if( dbg && ! *validDrapeLineP) bcdtmWrite_message(0,0,0,"Invalid Drape Line") ;
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking For Valid Drape Line Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking For Valid Drape Line Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR  ;
 goto cleanup ;
}
