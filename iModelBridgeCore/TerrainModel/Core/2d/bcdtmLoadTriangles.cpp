/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmLoadTriangles.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
#include "bcdtminlines.h"
/*==============================================================================*//**
* @memo   Load Triangles From A DTM File
* @doc    Load Triangles From A DTM File
* @notes  This function loads Triangles From A DTM File
* @author Rob Cormack March 2007  rob.cormack@bentley.con
* @param  dtmFile,              ==> Dtm File Name
* @param  *loadFunctionP        ==> Pointer To Load Function
* @param  useFence,             ==> Load Feature Within Fence < TRUE,FALSE >
* @param  fenceType,            ==> Fence Type
* @param  fenceOption           ==> Fence Option <INSIDE(1),OVERLAP(2)>
* @param  fencePts,             ==> DPoint3d Array Of Fence Points
* @param  numFencePts           ==> Number Of Fence Points
* @param  userP                 ==> User Pointer Passed Back To User
* @return DTM_SUCCESS or DTM_ERROR
* @version
* @see None
*===============================================================================*/
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmInterruptLoad_trianglesFromDtmFile
(
 WCharCP dtmFileP,        /* ==> Dtm File                                       */
 DTMFeatureCallback loadFunctionP,   /* ==> Pointer To Load Function                       */
 long useFence,            /* ==> Load Feature Within Fence <TRUE,FALSE>         */
 DTMFenceType fenceType,           /* ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape>   */
 DTMFenceOption fenceOption,         /* ==> Fence Option <INSIDE(1),OVERLAP(2)>            */
 DPoint3d  *fencePtsP,          /* ==> DPoint3d Array Of Fence Points                      */
 long numFencePts,         /* ==> Number Of Fence Points                         */
 void *userP               /* ==> User Pointer Passed Back To User               */
)
/*
** This Function Loads All Occurrences Of A DTM Feature Type From A Dtm File
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 BC_DTM_OBJ *dtmP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Interrupy Loading Triangles From Dtm File") ;
    bcdtmWrite_message(0,0,0,"dtmFile           = %s",dtmFileP) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP     = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"useFence          = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fenceOption       = %8ld",fenceOption) ;
    bcdtmWrite_message(0,0,0,"fencePtsP         = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts       = %8ld",numFencePts) ;
    bcdtmWrite_message(0,0,0,"userP             = %p",userP) ;
   }
/*
** Test If Requested Dtm Is Current Dtm
*/
 if( bcdtmUtility_testForAndSetCurrentDtmObject(&dtmP,dtmFileP)) goto errexit ;
/*
** Load Dtm Feature From Dtm Object
*/
 if( bcdtmInterruptLoad_trianglesFromDtmObject(dtmP,loadFunctionP,useFence,fenceType,fenceOption,fencePtsP,numFencePts,userP)) goto errexit ;
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
/*==============================================================================*//**
* @memo   Load Triangles From A DTM Dtm Object
* @doc    Load Triangles From A DTM Dtm Object
* @author Rob Cormack March 2007  rob.cormack@bentley.con
* @param  dtmP                  ==> Pointer To DTM object
* @param  *loadFunctionP        ==> Pointer To Load Function
* @param  useFence              ==> Load Feature Within Fence < TRUE,FALSE >
* @param  fenceType,            ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape>
* @param  fenceOption           ==> Fence Option <INSIDE(1),OVERLAP(2)>
* @param  fencePts              ==> DPoint3d Array Of Fence Points
* @param  numFencePts           ==> Number Of Fence Points
* @param  userP                 ==> User Pointer Passed Back To User
* @return DTM_SUCCESS or DTM_ERROR
* @version
* @see None
*===============================================================================*/
BENTLEYDTM_EXPORT int bcdtmInterruptLoad_trianglesFromDtmObject
(
 BC_DTM_OBJ  *dtmP,           /* ==> Pointer To DTM Dtm object                    */
 DTMFeatureCallback loadFunctionP,   /* ==> Pointer To Load Function                     */
 long    useFence,            /* ==> Load Feature Within Fence <TRUE,FALSE>       */
 DTMFenceType    fenceType,           /* ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape> */
 DTMFenceOption    fenceOption,         /* ==> Fence Option <INSIDE(1),OVERLAP(2)>          */
 DPoint3d     *fencePtsP,          /* ==> DPoint3d Array Of Fence Points                    */
 long    numFencePts,         /* ==> Number Of Fence Points                       */
 void    *userP               /* ==> User Pointer Passed Back To User             */
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Interrupt Loading Triangles From Dtm Object") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP     = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"useFence          = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fenceType         = %8ld",fenceType) ;
    bcdtmWrite_message(0,0,0,"fenceOption       = %8ld",fenceOption) ;
    bcdtmWrite_message(0,0,0,"fencePtsP         = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts       = %8ld",numFencePts) ;
    bcdtmWrite_message(0,0,0,"userP             = %p",userP) ;
   }
/*
** Validate Fence
*/
 if( useFence == TRUE && ( fencePtsP == NULL || numFencePts <= 2 ) ) useFence = FALSE ;
 if( useFence == TRUE && ( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )) useFence = FALSE ;
 if( useFence )
   {
    if( fenceType   != DTMFenceType::Block && fenceType   != DTMFenceType::Shape ) fenceType = DTMFenceType::Block ;
    if( fenceOption != DTMFenceOption::Inside      && fenceOption != DTMFenceOption::Outside && fenceOption != DTMFenceOption::Overlap ) fenceOption = DTMFenceOption::Overlap ;
   }
/*
** Test For Valid Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Valid Dtm Object") ;
 if( bcdtmObject_testForValidDtmObject(dtmP) ) goto errexit ;
/*
** Check If DTM Is In Tin State
*/
  if( dtmP->dtmState != DTMState::Tin )
    {
     bcdtmWrite_message(2,0,0,"DTM Object %p Not Triangulated",dtmP) ;
     goto errexit ;
    }
/*
** Load Triangles
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Loading Triangles") ;
 if( bcdtmInterruptLoad_trianglesDtmObject(dtmP,loadFunctionP,useFence,fenceType,fenceOption,fencePtsP,numFencePts,userP) ) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Interrupt Loading Triangles From Dtm Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Interrupt Loading Triangles From Dtm Object Error") ;
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
BENTLEYDTM_Private int bcdtmInterruptLoad_trianglesDtmObject
(
 BC_DTM_OBJ *dtmP,
 DTMFeatureCallback loadFunctionP,
 long useFence,
 DTMFenceType fenceType,
 DTMFenceOption fenceOption,
 DPoint3d *fencePtsP,
 long numFencePts,
 void *userP
)
/*
** This Function Loads Triangles From A Dtm Object
*/
{
 int             ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long            p1,p2,p3,clPtr,numTriangles;
 bool voidFlag, voidsInDtm = false, loadFence = false;
 long            startPnt,lastPnt,startTime,loadTime,leftMostPnt ;
 DPoint3d             trgPts[4] ;
 DTM_TIN_NODE    *nodeP,*node1P,*node2P,*node3P ;
 BC_DTM_OBJ      *clipDtmP=NULL  ;
 DPoint3d   *p1P,*p2P,*p3P  ;
 DTM_CIR_LIST    *clistP ;
 static long     pointMarkOffset=0;
/*
**    Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Loading Triangles From DTM Object %p",dtmP) ;
 loadTime = bcdtmClock() ;
/*
** Build Clipping Dtm For Fence Operations
*/
 startTime = bcdtmClock() ;
 if( useFence == TRUE )
   {
    if( bcdtmClip_buildClippingTinFromFencePointsDtmObject(&clipDtmP,fencePtsP,numFencePts) ) goto errexit ;
    if( fenceType == DTMFenceType::Block && dtmP->xMin >= clipDtmP->xMin && dtmP->xMax <= clipDtmP->xMax &&  dtmP->yMin >= clipDtmP->yMin && dtmP->yMax <= clipDtmP->yMax ) useFence = FALSE ;
   }
 if( tdbg ) bcdtmWrite_message(0,0,0,"** Clip Time = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Check For Voids In Dtm
*/
 startTime = bcdtmClock() ;
 bcdtmList_testForVoidsInDtmObject(dtmP,voidsInDtm) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"voidsInDtm = %2ld",voidsInDtm) ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"** Void Time = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Initialise
*/
 startPnt = 0 ;
 lastPnt  = dtmP->numPoints - 1 ;
 leftMostPnt  = 0 ;
 numTriangles = 0 ;
/*
** Get Start And End Points
*/
 if( useFence == TRUE )
   {
/*
**  Test If Load Should Be Terminated
*/
    if( bcdtmLoad_callUserLoadFunction(loadFunctionP,DTMFeatureType::CheckStop,dtmP->nullUserTag,dtmP->nullFeatureId,trgPts,4,userP)) goto errexit ;
/*
**  Find Points Immediately Before And After Fence
*/
    if( useFence == TRUE )
      {
       startTime = bcdtmClock() ;
       bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMin,&startPnt) ;
       while( startPnt > 0 && pointAddrP(dtmP,startPnt)->x >= clipDtmP->xMin ) --startPnt ;
       if( pointAddrP(dtmP,startPnt)->x < clipDtmP->xMin ) ++startPnt ;
       bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMax,&lastPnt) ;
       while( lastPnt < dtmP->numPoints - 1 && pointAddrP(dtmP,lastPnt)->x <= clipDtmP->xMin ) ++lastPnt ;
       if( tdbg )
         {
          bcdtmWrite_message(0,0,0,"startPnt = %8ld startPnt->x = %12.5lf",startPnt,pointAddrP(dtmP,startPnt)->x) ;
          bcdtmWrite_message(0,0,0,"lastPnt  = %8ld lastPnt->x  = %12.5lf",lastPnt,pointAddrP(dtmP,lastPnt)->x) ;
          bcdtmWrite_message(0,0,0,"clipDtmP->xMin  = %12.5lf clipDtmP->xMax  = %12.5lf",clipDtmP->xMin,clipDtmP->xMax) ;
          bcdtmWrite_message(0,0,0,"lastPnt  = %8ld lastPnt->x  = %12.5lf",lastPnt,pointAddrP(dtmP,lastPnt)->x) ;
          bcdtmWrite_message(0,0,0,"** Index Time 00 = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
         }
/*
**     Test If Load Should Be Terminated
*/
       if( bcdtmLoad_callUserLoadFunction(loadFunctionP,DTMFeatureType::CheckStop,dtmP->nullUserTag,dtmP->nullFeatureId,trgPts,4,userP)) goto errexit ;
      }
/*
**     Test If Load Should Be Terminated
*/
    if( bcdtmLoad_callUserLoadFunction(loadFunctionP,DTMFeatureType::CheckStop,dtmP->nullUserTag,dtmP->nullFeatureId,trgPts,4,userP)) goto errexit ;
   }
/*
** Scan And Load Triangles Within Fence
*/
 if( useFence == TRUE )
   {
    startTime = bcdtmClock() ;
    for( p1 = startPnt ; p1 <= lastPnt ; ++p1 )
      {
       node1P = nodeAddrP(dtmP,p1) ;
       p1P = pointAddrP(dtmP,p1) ;
       if( ( clPtr = node1P->cPtr ) != dtmP->nullPtr )
         {
          trgPts[0].x = p1P->x ;
          trgPts[0].y = p1P->y ;
          trgPts[0].z = p1P->z ;
          trgPts[3].x = p1P->x ;
          trgPts[3].y = p1P->y ;
          trgPts[3].z = p1P->z ;
          if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
          node2P = nodeAddrP(dtmP,p2) ;
          while ( clPtr != dtmP->nullPtr )
            {
             clistP = clistAddrP(dtmP,clPtr) ;
             clPtr  = clistP->nextPtr ;
             p3     = clistP->pntNum ;
             node3P = nodeAddrP(dtmP,p3) ;
             if( node1P->hPtr != p2 )
               {
                if( p2 > p1 && p3 > p1 || ( p2 < startPnt || p3 < startPnt) )
                  {
                   voidFlag = false ;
                   if( voidsInDtm ) { if( bcdtmList_testForVoidTriangleDtmObject(dtmP,p1,p2,p3,voidFlag)) goto errexit ; }
                   if( voidFlag == false)
                     {
/*
**                    Set Point Addresses
*/
                      p2P = pointAddrP(dtmP,p2) ;
                      p3P = pointAddrP(dtmP,p3) ;
/*
**                    Set Point Coordinates
*/
                      trgPts[1].x = p2P->x ;
                      trgPts[1].y = p2P->y ;
                      trgPts[1].z = p2P->z ;
                      trgPts[2].x = p3P->x ;
                      trgPts[2].y = p3P->y ;
                      trgPts[2].z = p3P->z ;
                      ++numTriangles ;
/*
**                    Check If A Triangle Vertex Is Within Fence
*/
                      loadFence = false ;
                      if       ( p1P->x >= clipDtmP->xMin && p1P->x <= clipDtmP->xMax && p1P->y >= clipDtmP->yMin && p1P->y <= clipDtmP->yMax) loadFence = true ;
                      else  if ( p2P->x >= clipDtmP->xMin && p2P->x <= clipDtmP->xMax && p2P->y >= clipDtmP->yMin && p2P->y <= clipDtmP->yMax) loadFence = true ;
                      else  if ( p3P->x >= clipDtmP->xMin && p3P->x <= clipDtmP->xMax && p3P->y >= clipDtmP->yMin && p3P->y <= clipDtmP->yMax) loadFence = true ;
/*
**                    Load Triangle
*/
                      if( loadFence == true )
                        {
                         if( bcdtmLoad_callUserLoadFunction(loadFunctionP,DTMFeatureType::Triangle,dtmP->nullUserTag,dtmP->nullFeatureId,trgPts,4,userP)) goto errexit ;
                        }
                     }
                  }
               }
/*
**           Set For Next Triangle
*/
             p2     = p3 ;
             node2P = node3P ;
            }
         }
/*
**     Test For Scan Point Beyond Fence
*/
       if( p1P->x >= clipDtmP->xMax ) p1 = dtmP->numPoints ;
      }
/*
**  Write Load Times
*/
    if( tdbg )
      {
       bcdtmWrite_message(0,0,0,"** Index Time 02 = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
       bcdtmWrite_message(0,0,0,"Number Of Triangles Loaded = %8ld",numTriangles) ;
      }
   }
/*
**  Scan And Load All Tin Triangles
*/
 else
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Triangles") ;
    startTime = bcdtmClock() ;
    for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
      {
       nodeP = nodeAddrP(dtmP,p1) ;
       if( ( clPtr = nodeP->cPtr) != dtmP->nullPtr )
         {
/*
**        Set Triangle Point 1 Coordinates
*/
          p1P = pointAddrP(dtmP,p1) ;
          trgPts[0].x = p1P->x ;
          trgPts[0].y = p1P->y ;
          trgPts[0].z = p1P->z ;
          trgPts[3].x = p1P->x ;
          trgPts[3].y = p1P->y ;
          trgPts[3].z = p1P->z ;
/*
**        Get Anti Clockwise Point To First Point In Circular List
*/
          if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
          p2P = pointAddrP(dtmP,p2) ;
/*
**        Scan Circular List
*/
          while( clPtr != dtmP->nullPtr )
            {
             clistP = clistAddrP(dtmP,clPtr) ;
             p3     = clistP->pntNum ;
             p3P    = pointAddrP(dtmP,p3) ;
             clPtr  = clistP->nextPtr ;
/*
**           Check For Valid Triangle
*/
             if( p2 > p1 && p3 > p1 && nodeP->hPtr != p2 )
               {
/*
**              Check For Void Triangle
*/
                voidFlag = false ;
                if( voidsInDtm ) { if( bcdtmList_testForVoidTriangleDtmObject(dtmP,p1,p2,p3,voidFlag)) goto errexit ; }
                if( voidFlag == false)
                  {
/*
**                 Set Remaining Triangle Coordinates
*/
                   trgPts[1].x = p2P->x ;
                   trgPts[1].y = p2P->y ;
                   trgPts[1].z = p2P->z ;
                   trgPts[2].x = p3P->x ;
                   trgPts[2].y = p3P->y ;
                   trgPts[2].z = p3P->z ;
/*
**                 Load Triangle
*/
                   if( bcdtmLoad_callUserLoadFunction(loadFunctionP,DTMFeatureType::Triangle,dtmP->nullUserTag,dtmP->nullFeatureId,trgPts,4,userP)) goto errexit ;
                  }
               }
/*
**           Set For Next Triangle
*/
             p2  = p3 ;
             p2P = p3P ;
            }
         }
      }
/*
**  Write Load Times
*/
    if( tdbg )
      {
       bcdtmWrite_message(0,0,0,"** Index Time 02 = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
       bcdtmWrite_message(0,0,0,"Number Of Triangles Loaded = %8ld",numTriangles) ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 ++pointMarkOffset ;
 if( clipDtmP != NULL ) bcdtmObject_destroyDtmObject(&clipDtmP) ;
/*
** Job Completed
*/
 if( tdbg ) bcdtmWrite_message(0,0,0,"** Load Time = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),loadTime)) ;
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Triangles From DTM Object %p Completed",dtmP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Triangles From DTM Object %p Error",dtmP) ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*==============================================================================*//**
* @memo   Load Triangles From A DTM Dtm Object
* @doc    Load Triangles From A DTM Dtm Object
* @author Rob Cormack March 2007  rob.cormack@bentley.con
* @param  dtmP                  ==> Pointer To DTM object
* @param  *loadFunctionP        ==> Pointer To Load Function
* @param  useFence              ==> Load Feature Within Fence < TRUE,FALSE >
* @param  fenceType,            ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape>
* @param  fenceOption           ==> Fence Option <INSIDE(1),OVERLAP(2)>
* @param  fencePts              ==> DPoint3d Array Of Fence Points
* @param  numFencePts           ==> Number Of Fence Points
* @param  userP                 ==> User Pointer Passed Back To User
* @return DTM_SUCCESS or DTM_ERROR
* @version
* @see None
*===============================================================================*/

typedef int ( __stdcall *CSharpDtmGenericCallBack)(DTMFeatureType dtmFeatureType,Int64 dtmUserTag , Int64 dtmFeatureId, DPoint3d *featurePtsP , long numFeaturePts,void* userP ) ;
typedef int (__stdcall *CSharpDtmTinMeshCallBack)(DTMFeatureType dtmFeatureType, long numTriangles, long numMeshPts, DPoint3d *MeshPtsP, long numMeshFaces, long *meshFacesP, void* userP);

BENTLEYDTM_EXPORT int bcdtmInterruptLoadCSharp_trianglesDtmObject
(
 BC_DTM_OBJ *dtmP,
 CSharpDtmGenericCallBack loadFunctionP,
 long useFence,
 DTMFenceType fenceType,
 DTMFenceOption fenceOption,
 DPoint3d *fencePtsP,
 long numFencePts,
 void *userP
) ;

BENTLEYDTM_EXPORT int bcdtmInterruptLoadCSharp_trianglesFromDtmObject
(
 BC_DTM_OBJ  *dtmP,           /* ==> Pointer To DTM Dtm object                    */
 CSharpDtmGenericCallBack loadFunctionP,   /* ==> Pointer To Load Function                     */
 long    useFence,            /* ==> Load Feature Within Fence <TRUE,FALSE>       */
 DTMFenceType fenceType,           /* ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape> */
 DTMFenceOption fenceOption,         /* ==> Fence Option <INSIDE(1),OVERLAP(2)>          */
 DPoint3d     *fencePtsP,          /* ==> DPoint3d Array Of Fence Points                    */
 long    numFencePts,         /* ==> Number Of Fence Points                       */
 void    *userP               /* ==> User Pointer Passed Back To User             */
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Interrupt Loading Triangles From Dtm Object") ;
    bcdtmWrite_message(0,0,0,"dtmP              = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP     = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"useFence          = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fenceType         = %8ld",fenceType) ;
    bcdtmWrite_message(0,0,0,"fenceOption       = %8ld",fenceOption) ;
    bcdtmWrite_message(0,0,0,"fencePtsP         = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts       = %8ld",numFencePts) ;
    bcdtmWrite_message(0,0,0,"userP             = %p",userP) ;
   }
/*
** Validate Fence
*/
 if( useFence == TRUE && ( fencePtsP == NULL || numFencePts <= 2 ) ) useFence = FALSE ;
 if( useFence == TRUE && ( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )) useFence = FALSE ;
 if( useFence )
   {
    if( fenceType   != DTMFenceType::Block && fenceType   != DTMFenceType::Shape ) fenceType = DTMFenceType::Block ;
    if( fenceOption != DTMFenceOption::Inside      && fenceOption != DTMFenceOption::Outside && fenceOption != DTMFenceOption::Overlap ) fenceOption = DTMFenceOption::Overlap ;
   }
/*
** Test For Valid Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Valid Dtm Object") ;
 if( bcdtmObject_testForValidDtmObject(dtmP) ) goto errexit ;
/*
** Check If DTM Is In Tin State
*/
  if( dtmP->dtmState != DTMState::Tin )
    {
     bcdtmWrite_message(2,0,0,"DTM Object %p Not Triangulated",dtmP) ;
     goto errexit ;
    }
/*
** Load Triangles
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Loading Triangles") ;
 if( bcdtmInterruptLoadCSharp_trianglesDtmObject(dtmP,loadFunctionP,useFence,fenceType,fenceOption,fencePtsP,numFencePts,userP) ) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Interrupt Loading Triangles From Dtm Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Interrupt Loading Triangles From Dtm Object Error") ;
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
BENTLEYDTM_Public int bcdtmInterruptLoadCSharp_trianglesDtmObject
(
 BC_DTM_OBJ *dtmP,
 CSharpDtmGenericCallBack loadFunctionP,
 long useFence,
 DTMFenceType fenceType,
 DTMFenceOption fenceOption,
 DPoint3d *fencePtsP,
 long numFencePts,
 void *userP
)
/*
** This Function Loads Triangles From A Dtm Object
*/
{
 int             ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long            p1,p2,p3,clPtr,numTriangles;
 long            startPnt,lastPnt,startTime,loadTime,leftMostPnt ;
 bool voidsInDtm = false, loadFence = false, voidFlag;
 DPoint3d             trgPts[4] ;
 DTM_TIN_NODE    *nodeP,*node1P,*node2P,*node3P ;
 BC_DTM_OBJ      *clipDtmP=NULL  ;
 DPoint3d   *p1P,*p2P,*p3P  ;
 DTM_CIR_LIST    *clistP ;
 static long     pointMarkOffset=0;
/*
**    Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Loading Triangles From DTM Object %p",dtmP) ;
 loadTime = bcdtmClock() ;
/*
** Build Clipping Dtm For Fence Operations
*/
 startTime = bcdtmClock() ;
 if( useFence == TRUE )
   {
    if( bcdtmClip_buildClippingTinFromFencePointsDtmObject(&clipDtmP,fencePtsP,numFencePts) ) goto errexit ;
    if( fenceType == DTMFenceType::Block && dtmP->xMin >= clipDtmP->xMin && dtmP->xMax <= clipDtmP->xMax &&  dtmP->yMin >= clipDtmP->yMin && dtmP->yMax <= clipDtmP->yMax ) useFence = FALSE ;
   }
 if( tdbg ) bcdtmWrite_message(0,0,0,"** Clip Time = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Check For Voids In Dtm
*/
 startTime = bcdtmClock() ;
 bcdtmList_testForVoidsInDtmObject(dtmP,voidsInDtm) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"voidsInDtm = %2ld",voidsInDtm) ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"** Void Time = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Initialise
*/
 startPnt = 0 ;
 lastPnt  = dtmP->numPoints - 1 ;
 leftMostPnt  = 0 ;
 numTriangles = 0 ;
/*
** Get Start And End Points
*/
 if( useFence == TRUE )
   {
/*
**  Test If Load Should Be Terminated
*/
    if( loadFunctionP(DTMFeatureType::CheckStop,dtmP->nullUserTag,dtmP->nullFeatureId,trgPts,4,userP)) goto errexit ;
/*
**  Find Points Immediately Before And After Fence
*/
    if( useFence == TRUE )
      {
       startTime = bcdtmClock() ;
       bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMin,&startPnt) ;
       while( startPnt > 0 && pointAddrP(dtmP,startPnt)->x >= clipDtmP->xMin ) --startPnt ;
       if( pointAddrP(dtmP,startPnt)->x < clipDtmP->xMin ) ++startPnt ;
       bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMax,&lastPnt) ;
       while( lastPnt < dtmP->numPoints  - 1 && pointAddrP(dtmP,lastPnt)->x <= clipDtmP->xMin ) ++lastPnt ;
       if( tdbg )
         {
          bcdtmWrite_message(0,0,0,"startPnt = %8ld startPnt->x = %12.5lf",startPnt,pointAddrP(dtmP,startPnt)->x) ;
          bcdtmWrite_message(0,0,0,"lastPnt  = %8ld lastPnt->x  = %12.5lf",lastPnt,pointAddrP(dtmP,lastPnt)->x) ;
          bcdtmWrite_message(0,0,0,"clipDtmP->xMin  = %12.5lf clipDtmP->xMax  = %12.5lf",clipDtmP->xMin,clipDtmP->xMax) ;
          bcdtmWrite_message(0,0,0,"lastPnt  = %8ld lastPnt->x  = %12.5lf",lastPnt,pointAddrP(dtmP,lastPnt)->x) ;
          bcdtmWrite_message(0,0,0,"** Index Time 00 = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
         }
/*
**     Test If Load Should Be Terminated
*/
       if( loadFunctionP(DTMFeatureType::CheckStop,dtmP->nullUserTag,dtmP->nullFeatureId,trgPts,4,userP)) goto errexit ;
      }
/*
**     Test If Load Should Be Terminated
*/
    if( loadFunctionP(DTMFeatureType::CheckStop,dtmP->nullUserTag,dtmP->nullFeatureId,trgPts,4,userP)) goto errexit ;
   }
/*
** Scan And Load Triangles Within Fence
*/
 if( useFence == TRUE )
   {
    startTime = bcdtmClock() ;
    for( p1 = startPnt ; p1 <= lastPnt ; ++p1 )
      {
       node1P = nodeAddrP(dtmP,p1) ;
       p1P = pointAddrP(dtmP,p1) ;
       if( ( clPtr = node1P->cPtr ) != dtmP->nullPtr )
         {
          trgPts[0].x = p1P->x ;
          trgPts[0].y = p1P->y ;
          trgPts[0].z = p1P->z ;
          trgPts[3].x = p1P->x ;
          trgPts[3].y = p1P->y ;
          trgPts[3].z = p1P->z ;
          if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
          node2P = nodeAddrP(dtmP,p2) ;
          while ( clPtr != dtmP->nullPtr )
            {
             clistP = clistAddrP(dtmP,clPtr) ;
             clPtr  = clistP->nextPtr ;
             p3     = clistP->pntNum ;
             node3P = nodeAddrP(dtmP,p3) ;
             if( node1P->hPtr != p2 )
               {
                if( p2 > p1 && p3 > p1 || ( p2 < startPnt || p3 < startPnt) )
                  {
                   voidFlag = false ;
                   if( voidsInDtm ) { if( bcdtmList_testForVoidTriangleDtmObject(dtmP,p1,p2,p3,voidFlag)) goto errexit ; }
                   if( voidFlag == false)
                     {
/*
**                    Set Point Addresses
*/
                      p2P = pointAddrP(dtmP,p2) ;
                      p3P = pointAddrP(dtmP,p3) ;
/*
**                    Set Point Coordinates
*/
                      trgPts[1].x = p2P->x ;
                      trgPts[1].y = p2P->y ;
                      trgPts[1].z = p2P->z ;
                      trgPts[2].x = p3P->x ;
                      trgPts[2].y = p3P->y ;
                      trgPts[2].z = p3P->z ;
                      ++numTriangles ;
/*
**                    Check If A Triangle Vertex Is Within Fence
*/
                      loadFence = FALSE ;
                      if       ( p1P->x >= clipDtmP->xMin && p1P->x <= clipDtmP->xMax && p1P->y >= clipDtmP->yMin && p1P->y <= clipDtmP->yMax) loadFence = TRUE ;
                      else  if ( p2P->x >= clipDtmP->xMin && p2P->x <= clipDtmP->xMax && p2P->y >= clipDtmP->yMin && p2P->y <= clipDtmP->yMax) loadFence = TRUE ;
                      else  if ( p3P->x >= clipDtmP->xMin && p3P->x <= clipDtmP->xMax && p3P->y >= clipDtmP->yMin && p3P->y <= clipDtmP->yMax) loadFence = TRUE ;
/*
**                    Load Triangle
*/
                      if( loadFence == TRUE )
                        {
                         if( loadFunctionP(DTMFeatureType::Triangle,dtmP->nullUserTag,dtmP->nullFeatureId,trgPts,4,userP)) goto errexit ;
                         ++numTriangles ;
                        }
                     }
                  }
               }
/*
**           Set For Next Triangle
*/
             p2     = p3 ;
             node2P = node3P ;
            }
         }
/*
**     Test For Scan Point Beyond Fence
*/
       if( p1P->x >= clipDtmP->xMax ) p1 = dtmP->numPoints ;
      }
   }
/*
**  Scan And Load All Tin Triangles
*/
 else
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Scanning All Triangles") ;
    startTime = bcdtmClock() ;
    for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
      {
       nodeP = nodeAddrP(dtmP,p1) ;
       if( ( clPtr = nodeP->cPtr) != dtmP->nullPtr )
         {
/*
**        Set Triangle Point 1 Coordinates
*/
          p1P = pointAddrP(dtmP,p1) ;
          trgPts[0].x = p1P->x ;
          trgPts[0].y = p1P->y ;
          trgPts[0].z = p1P->z ;
          trgPts[3].x = p1P->x ;
          trgPts[3].y = p1P->y ;
          trgPts[3].z = p1P->z ;
/*
**        Get Anti Clockwise Point To First Point In Circular List
*/
          if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
          p2P = pointAddrP(dtmP,p2) ;
/*
**        Scan Circular List
*/
          while( clPtr != dtmP->nullPtr )
            {
             clistP = clistAddrP(dtmP,clPtr) ;
             p3     = clistP->pntNum ;
             p3P    = pointAddrP(dtmP,p3) ;
             clPtr  = clistP->nextPtr ;
/*
**           Check For Valid Triangle
*/
             if( p2 > p1 && p3 > p1 && nodeP->hPtr != p2 )
               {
/*
**              Check For Void Triangle
*/
                voidFlag = false ;
                if( voidsInDtm ) { if( bcdtmList_testForVoidTriangleDtmObject(dtmP,p1,p2,p3,voidFlag)) goto errexit ; }
                if( voidFlag == false)
                  {
/*
**                 Set Remaining Triangle Coordinates
*/
                   trgPts[1].x = p2P->x ;
                   trgPts[1].y = p2P->y ;
                   trgPts[1].z = p2P->z ;
                   trgPts[2].x = p3P->x ;
                   trgPts[2].y = p3P->y ;
                   trgPts[2].z = p3P->z ;
/*
**                 Load Triangle
*/
                   if( loadFunctionP(DTMFeatureType::Triangle,dtmP->nullUserTag,dtmP->nullFeatureId,trgPts,4,userP)) goto errexit ;
                   ++numTriangles ;
                  }
               }
/*
**           Set For Next Triangle
*/
             p2  = p3 ;
             p2P = p3P ;
            }
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
 ++pointMarkOffset ;
 if( clipDtmP != NULL ) bcdtmObject_destroyDtmObject(&clipDtmP) ;
/*
** Job Completed
*/
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Load %8ld Triangles = %8.3lf Seconds",numTriangles,bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Triangles From DTM Object %p Completed",dtmP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Triangles From DTM Object %p Error",dtmP) ;
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
BENTLEYDTM_Private int bcdtmInterruptLoad_trianglesDtmObjectOld
(
 BC_DTM_OBJ *dtmP,
 DTMFeatureCallback loadFunctionP,
 long useFence,
 DTMFenceType fenceType,
 DTMFenceOption fenceOption,
 DPoint3d *fencePtsP,
 long numFencePts,
 void *userP
)
/*
** This Function Loads Triangles From A Dtm Object
*/
{
 int             ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long            n,p1,p2,p3,clPtr,numTriangles,numClipArrays,clipResult;
 long            startPnt,lastPnt,startTime,loadTime,leftMostPnt,numMarked,pointMark ;
 bool voidFlag, voidsInDtm = false;
 long            findType,trgPnt1,trgPnt2,trgPnt3 ;
 DPoint3d             trgPts[4] ;
 DTM_TIN_NODE    *nodeP,*node1P,*node2P,*node3P ;
 BC_DTM_OBJ      *clipDtmP=NULL  ;
 DPoint3d   *p1P,*p2P,*p3P,*pntP  ;
 DTM_POINT_ARRAY **clipArraysPP=NULL ;
 DTM_CIR_LIST    *clistP ;
 static long     pointMarkOffset=0;
/*
**    Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Loading Triangles From DTM Object %p",dtmP) ;
 loadTime = bcdtmClock() ;
/*
** Build Clipping Dtm For Fence Operations
*/
 startTime = bcdtmClock() ;
 if( useFence == TRUE )
   {
    if( bcdtmClip_buildClippingTinFromFencePointsDtmObject(&clipDtmP,fencePtsP,numFencePts) ) goto errexit ;
    if( fenceType == DTMFenceType::Block && dtmP->xMin >= clipDtmP->xMin && dtmP->xMax <= clipDtmP->xMax &&  dtmP->yMin >= clipDtmP->yMin && dtmP->yMax <= clipDtmP->yMax ) useFence = FALSE ;
   }
 if( tdbg ) bcdtmWrite_message(0,0,0,"** Clip Time = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Check For Voids In Dtm
*/
 startTime = bcdtmClock() ;
 bcdtmList_testForVoidsInDtmObject(dtmP,voidsInDtm) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"voidsInDtm = %2ld",voidsInDtm) ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"** Void Time = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Initialise
*/
 startPnt = 0 ;
 lastPnt  = dtmP->numPoints ;
 leftMostPnt  = 0 ;
 numTriangles = 0 ;
/*
** Get Start And End Points
*/
 if( useFence == TRUE )
   {
/*
**  Mark Points Immediately External To Fence
*/
    pointMark = dtmP->numPoints * 2 + pointMarkOffset;
    startTime = bcdtmClock() ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points External To Fence") ;
    if( bcdtmLoad_markTinPointsExternalToFenceDtmObject(dtmP,clipDtmP,pointMark,&leftMostPnt,&numMarked)) goto errexit ;
    if( tdbg )
      {
       bcdtmWrite_message(0,0,0,"leftMostPnt = %8ld leftMostPnt->x = %12.5lf",leftMostPnt,pointAddrP(dtmP,leftMostPnt)->x) ;
       bcdtmWrite_message(0,0,0,"** Time To Mark %6ld Tin Points External To Fence = %8.3lf Seconds",numMarked,bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
      }
    if( numMarked == 0 ) useFence = FALSE ;
/*
**  Test If Load Should Be Terminated
*/
    if( bcdtmLoad_callUserLoadFunction(loadFunctionP,DTMFeatureType::CheckStop,dtmP->nullUserTag,dtmP->nullFeatureId,trgPts,4,userP)) goto errexit ;
/*
**  Find Points Immediately Before And After Fence
*/
    if( useFence == TRUE )
      {
       startTime = bcdtmClock() ;
       bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMin,&startPnt) ;
       while( startPnt > 0 && pointAddrP(dtmP,startPnt)->x >= clipDtmP->xMin ) --startPnt ;
       if( pointAddrP(dtmP,startPnt)->x < clipDtmP->xMin ) ++startPnt ;
       bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMax,&lastPnt) ;
       while( lastPnt < dtmP->numPoints  - 1 && pointAddrP(dtmP,lastPnt)->x <= clipDtmP->xMin ) ++lastPnt ;
       if( tdbg )
         {
          bcdtmWrite_message(0,0,0,"startPnt = %8ld startPnt->x = %12.5lf",startPnt,pointAddrP(dtmP,startPnt)->x) ;
          bcdtmWrite_message(0,0,0,"lastPnt  = %8ld lastPnt->x  = %12.5lf",lastPnt,pointAddrP(dtmP,lastPnt)->x) ;
          bcdtmWrite_message(0,0,0,"clipDtmP->xMin  = %12.5lf clipDtmP->xMax  = %12.5lf",clipDtmP->xMin,clipDtmP->xMax) ;
          bcdtmWrite_message(0,0,0,"lastPnt  = %8ld lastPnt->x  = %12.5lf",lastPnt,pointAddrP(dtmP,lastPnt)->x) ;
          bcdtmWrite_message(0,0,0,"** Index Time 00 = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
         }
/*
**     Test If Load Should Be Terminated
*/
       if( bcdtmLoad_callUserLoadFunction(loadFunctionP,DTMFeatureType::CheckStop,dtmP->nullUserTag,dtmP->nullFeatureId,trgPts,4,userP)) goto errexit ;
/*
**     Mark Points Within Fence Block
*/
       startTime = bcdtmClock() ;
       if( fenceType == DTMFenceType::Block )
         {
          for( p1 = startPnt ; p1 <= lastPnt ; ++p1 )
            {
             pntP = pointAddrP(dtmP,p1) ;
             if( pntP->y >= clipDtmP->yMin && pntP->y <= clipDtmP->yMax ) nodeAddrP(dtmP,p1)->sPtr = pointMark ;
            }
         }
/*
**     Mark Points Within Fence Shape
*/
       if( fenceType == DTMFenceType::Shape )
         {
          for( p1 = startPnt ; p1 <= lastPnt ; ++p1 )
            {
             pntP = pointAddrP(dtmP,p1) ;
             findType = 0 ;
             if( pntP->x >= clipDtmP->xMin && pntP->x <= clipDtmP->xMax && pntP->y >= clipDtmP->yMin && pntP->y <= clipDtmP->yMax )
               {
                if( bcdtmFind_triangleDtmObject(clipDtmP,pntP->x,pntP->y,&findType,&trgPnt1,&trgPnt2,&trgPnt3)) goto errexit ;
               }
             if( findType  ) nodeAddrP(dtmP,p1)->sPtr = pointMark ;
            }
         }
       if( tdbg ) bcdtmWrite_message(0,0,0,"** Index Time 01 = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
      }
/*
**     Test If Load Should Be Terminated
*/
    if( bcdtmLoad_callUserLoadFunction(loadFunctionP,DTMFeatureType::CheckStop,dtmP->nullUserTag,dtmP->nullFeatureId,trgPts,4,userP)) goto errexit ;
   }
/*
** Scan And Load Triangles Within Fence
*/
 if( useFence == TRUE )
   {
    startTime = bcdtmClock() ;
    for( p1 = leftMostPnt ; p1 <= lastPnt ; ++p1 )
      {
       node1P = nodeAddrP(dtmP,p1) ;
       if( node1P->sPtr == pointMark && ( clPtr = node1P->cPtr) != dtmP->nullPtr )
         {
          p1P = pointAddrP(dtmP,p1) ;
          trgPts[0].x = p1P->x ;
          trgPts[0].y = p1P->y ;
          trgPts[0].z = p1P->z ;
          trgPts[3].x = p1P->x ;
          trgPts[3].y = p1P->y ;
          trgPts[3].z = p1P->z ;
          if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
          node2P = nodeAddrP(dtmP,p2) ;
          while ( clPtr != dtmP->nullPtr )
            {
             clistP = clistAddrP(dtmP,clPtr) ;
             clPtr  = clistP->nextPtr ;
             p3     = clistP->pntNum ;
             node3P = nodeAddrP(dtmP,p3) ;
             if( node1P->hPtr != p2 )
               {
                if( p2 > p1 && p3 > p1 && node2P->sPtr == pointMark && node3P->sPtr == pointMark )
                  {
                   voidFlag = FALSE ;
                   if( voidsInDtm ) { if( bcdtmList_testForVoidTriangleDtmObject(dtmP,p1,p2,p3,voidFlag)) goto errexit ; }
                   if( voidFlag == FALSE )
                     {
/*
**                    Set Point Addresses
*/
                      p2P = pointAddrP(dtmP,p2) ;
                      p3P = pointAddrP(dtmP,p3) ;
/*
**                    Set Point Coordinates
*/
                      trgPts[1].x = p2P->x ;
                      trgPts[1].y = p2P->y ;
                      trgPts[1].z = p2P->z ;
                      trgPts[2].x = p3P->x ;
                      trgPts[2].y = p3P->y ;
                      trgPts[2].z = p3P->z ;
                      ++numTriangles ;
/*
**                    Load Triangle
*/
                      if( fenceOption == DTMFenceOption::Overlap )
                        {
                         if( bcdtmLoad_callUserLoadFunction(loadFunctionP,DTMFeatureType::Triangle,dtmP->nullUserTag,dtmP->nullFeatureId,trgPts,4,userP)) goto errexit ;
                        }
                      if( fenceOption == DTMFenceOption::Inside )
                        {
                         if( bcdtmClip_featurePointArrayToTinHullDtmObject(clipDtmP,fenceOption,trgPts,4,&clipResult,&clipArraysPP,&numClipArrays)) goto errexit ;
                         if( clipResult == 1 ) if( bcdtmLoad_callUserLoadFunction(loadFunctionP,DTMFeatureType::Triangle,dtmP->nullUserTag,dtmP->nullFeatureId,trgPts,4,userP)) goto errexit ;
                         if( clipResult == 2 )
                           {
                            for( n = 0 ; n < numClipArrays ; ++n )
                              {
                               if( bcdtmLoad_callUserLoadFunction(loadFunctionP,DTMFeatureType::Triangle,dtmP->nullUserTag,dtmP->nullFeatureId,clipArraysPP[n]->pointsP,clipArraysPP[n]->numPoints,userP)) goto errexit ;
                              }
                            bcdtmMem_freePointerArrayToPointArrayMemory(&clipArraysPP,numClipArrays) ;
                           }
                        }
                     }
                  }
               }
/*
**           Set For Next Triangle
*/
             p2     = p3 ;
             node2P = node3P ;
            }
         }
/*
**     Test For Scan Point Beyond Fence
*/
       if( p1P->x >= clipDtmP->xMax ) p1 = dtmP->numPoints ;
      }
/*
**  Write Load Times
*/
    if( tdbg )
      {
       bcdtmWrite_message(0,0,0,"** Index Time 02 = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
       bcdtmWrite_message(0,0,0,"Number Of Triangles Loaded = %8ld",numTriangles) ;
      }
   }
/*
**  Scan And Load All Tin Triangles
*/
 else
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Triangles") ;
    startTime = bcdtmClock() ;
    for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
      {
       nodeP = nodeAddrP(dtmP,p1) ;
       if( ( clPtr = nodeP->cPtr) != dtmP->nullPtr )
         {
/*
**        Set Triangle Point 1 Coordinates
*/
          p1P = pointAddrP(dtmP,p1) ;
          trgPts[0].x = p1P->x ;
          trgPts[0].y = p1P->y ;
          trgPts[0].z = p1P->z ;
          trgPts[3].x = p1P->x ;
          trgPts[3].y = p1P->y ;
          trgPts[3].z = p1P->z ;
/*
**        Get Anti Clockwise Point To First Point In Circular List
*/
          if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
          p2P = pointAddrP(dtmP,p2) ;
/*
**        Scan Circular List
*/
          while( clPtr != dtmP->nullPtr )
            {
             clistP = clistAddrP(dtmP,clPtr) ;
             p3     = clistP->pntNum ;
             p3P    = pointAddrP(dtmP,p3) ;
             clPtr  = clistP->nextPtr ;
/*
**           Check For Valid Triangle
*/
             if( p2 > p1 && p3 > p1 && nodeP->hPtr != p2 )
               {
/*
**              Check For Void Triangle
*/
                voidFlag = FALSE ;
                if( voidsInDtm ) { if( bcdtmList_testForVoidTriangleDtmObject(dtmP,p1,p2,p3,voidFlag)) goto errexit ; }
                if( voidFlag == FALSE )
                  {
/*
**                 Set Remaining Triangle Coordinates
*/
                   trgPts[1].x = p2P->x ;
                   trgPts[1].y = p2P->y ;
                   trgPts[1].z = p2P->z ;
                   trgPts[2].x = p3P->x ;
                   trgPts[2].y = p3P->y ;
                   trgPts[2].z = p3P->z ;
/*
**                 Load Triangle
*/
                   if( bcdtmLoad_callUserLoadFunction(loadFunctionP,DTMFeatureType::Triangle,dtmP->nullUserTag,dtmP->nullFeatureId,trgPts,4,userP)) goto errexit ;
                  }
               }
/*
**           Set For Next Triangle
*/
             p2  = p3 ;
             p2P = p3P ;
            }
         }
      }
/*
**  Write Load Times
*/
    if( tdbg )
      {
       bcdtmWrite_message(0,0,0,"** Index Time 02 = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
       bcdtmWrite_message(0,0,0,"Number Of Triangles Loaded = %8ld",numTriangles) ;
      }
   }
/*
** Clean Up
*/
 cleanup :
 ++pointMarkOffset ;
 if( clipDtmP != NULL ) bcdtmObject_destroyDtmObject(&clipDtmP) ;
/*
** Job Completed
*/
 if( tdbg ) bcdtmWrite_message(0,0,0,"** Load Time = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),loadTime)) ;
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Triangles From DTM Object %p Completed",dtmP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Triangles From DTM Object %p Error",dtmP) ;
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
BENTLEYDTM_Public int bcdtmLoad_markTinPointsExternalToFenceDtmObject
(
 BC_DTM_OBJ *dtmP,
 BC_DTM_OBJ *fenceDtmP,
 long pointMark ,
 long *leftMostPntP,
 long *numMarkedP
)
/*
** This Function Marks Tin Points External To A Fence
*/
{
 int             ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long            pnt1,pnt2,numExternal=0,startTime ;
 DPoint3d   *pnt1P,*pnt2P  ;
/*
**    Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Tin Points External To Fence") ;
/*
** Initialise
*/
 *numMarkedP = 0 ;
 *leftMostPntP = dtmP->nullPnt ;
 startTime = bcdtmClock() ;
/*
** Scan Fence DTM Hull And Mark Points
*/
 pnt1  = fenceDtmP->hullPoint ;
 pnt1P = pointAddrP(fenceDtmP,pnt1) ;
 do
   {
    pnt2  = nodeAddrP(fenceDtmP,pnt1)->hPtr ;
    pnt2P = pointAddrP(fenceDtmP,pnt2) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Fence Line ** %12.5lf %12.5lf ** %12.5lf %12.5lf",pnt1P->x,pnt1P->y,pnt2P->x,pnt2P->y) ;
    if( bcdtmLoad_markIntersectingTinLinePointsDtmObject(dtmP,pnt1P->x,pnt1P->y,pnt2P->x,pnt2P->y,pointMark,leftMostPntP,numMarkedP)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"leftMostPnt = %9ld numMarked = %9ld",*leftMostPntP,*numMarkedP) ;
    pnt1  = pnt2  ;
    pnt1P = pnt2P ;
   } while ( pnt1 != fenceDtmP->hullPoint )  ;
/*
** Log Left And Right Most Points And Their Coordinates
*/
 if( dbg == 1 && *leftMostPntP != dtmP->nullPnt )
   {
    ++numExternal ;
    bcdtmWrite_message(0,0,0,"leftMostPnt  = %9ld ** %12.5lf %12.5lf %10.4lf",*leftMostPntP,pointAddrP(dtmP,*leftMostPntP)->x,pointAddrP(dtmP,*leftMostPntP)->y,pointAddrP(dtmP,*leftMostPntP)->z ) ;
    pnt2 = dtmP->nullPnt ;
    for( pnt1 = *leftMostPntP + 1 ; pnt1 < dtmP->numPoints ; ++pnt1 )
      {
       if( nodeAddrP(dtmP,pnt1)->sPtr == pointMark )
         {
          ++numExternal ;
          pnt2 = pnt1 ;
         }
      }
     bcdtmWrite_message(0,0,0,"rightMostPnt = %9ld ** %12.5lf %12.5lf %10.4lf",pnt2,pointAddrP(dtmP,pnt2)->x,pointAddrP(dtmP,pnt2)->y,pointAddrP(dtmP,pnt2)->z ) ;
     bcdtmWrite_message(0,0,0,"Number Of Immediately External Points = %8ld",numExternal ) ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Tin Points External To Fence Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Tin Points External To Fence Completed") ;
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
BENTLEYDTM_Private int bcdtmLoad_markIntersectingTinLinePointsDtmObject
(
 BC_DTM_OBJ *dtmP,
 double  pnt1X,
 double  pnt1Y,
 double  pnt2X,
 double  pnt2Y,
 long pointMark ,
 long *leftMostPointP,
 long *numMarkedP
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   p1,p2,p3,np1,np2,np3,fndType,drapeType,processDrape ;
 double xls,yls,zls,xle,yle,xi,yi,zi ;
/*
**    Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Intersecting Tin Line Points") ;
 xls = pnt1X ; yls = pnt1Y ;
 xle = pnt2X ; yle = pnt2Y ;
/*
**  Write Out Line To Be Drapped
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Drape Line = %10.4lf %10.4lf ** %10.4lf %10.4lf",xls,yls,xle,yle) ;
/*
**  Find Triangle Containing Drape Start Point
*/
 processDrape = 1 ;
 if( bcdtmFind_triangleDtmObject(dtmP,xls,yls,&fndType,&p1,&p2,&p3)) goto errexit ;
 if( fndType == 0 )
   {
    p1 = p2 = dtmP->nullPnt ;
    if( bcdtmDrape_findClosestLineInterceptWithHullDtmObject(dtmP,xls,yls,xle,yle,&fndType,&p1,&p2,&xls,&yls,&zls) ) goto errexit ;
    if( fndType == 0 )  processDrape = 0 ;
    else
      {
       if( fndType == 2 )
         {
          p3 = p1 ;
          p1 = p2 ;
          p2 = p3 ;
          if( ( p3 = bcdtmList_nextClkDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ;
         }
      }
   } else if( fndType == 2 ) fndType = 3 ;
/*
**  Plot Start of Profile Line on Surface
*/
 if( processDrape )
   {
    drapeType = fndType ;
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Drape Start Point ** drapeType = %2ld p1 = %8ld p2 = %8ld p3 = %8ld ** %10.4lf %10.4lf",drapeType,p1,p2,p3,xls,yls) ;
    if( drapeType == 1 )
      {
       p2 = p3 = dtmP->nullPnt ;
       nodeAddrP(dtmP,p1)->sPtr = pointMark ;
       ++*numMarkedP ;
       if( *leftMostPointP == dtmP->nullPnt || p1 < *leftMostPointP ) *leftMostPointP = p1 ;
      }
    else
      {
       nodeAddrP(dtmP,p1)->sPtr = pointMark ;
       nodeAddrP(dtmP,p2)->sPtr = pointMark ;
       if( *leftMostPointP == dtmP->nullPnt || p1 < *leftMostPointP ) *leftMostPointP = p1 ;
       if( p2 < *leftMostPointP ) *leftMostPointP = p2 ;
       ++*numMarkedP ;
       ++*numMarkedP ;
      }
   }
/*
**  Scan To Drape Line End
*/
 while ( processDrape )
   {
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"drapeType = %2ld p1 = %8ld p2 = %8ld p3 = %8ld",drapeType,p1,p2,p3) ;
    fndType = bcdtmDrape_getNextPointForDrapeDtmObject(dtmP,xls,yls,xle,yle,&drapeType,p1,p2,p3,&np1,&np2,&np3,&xi,&yi,&zi)  ;
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"fndType = %2ld drapeType = %2ld np1 = %8ld np2 = %8ld np3 = %8ld ** %10.4lf %10.4lf %10.4lf",fndType,drapeType,np1,np2,np3,xi,yi,zi) ;
/*
**  Error Detected
*/
    if( fndType == 4 ) goto errexit ;
/*
**  Next Drape Point Not Found
*/
    if( fndType == 3 ) processDrape = 0 ;
/*
**  Next Drape Point Found
*/
    if( fndType == 0 || fndType == 1 )
      {
       xls = xi ;
       yls = yi ;
       zls = zi ;
       p1 = np1 ;
       p2 = np2 ;
       p3 = np3 ;
       if( fndType == 1 ) processDrape = 0 ;
       else
         {
          if( drapeType == 1 )
            {
             nodeAddrP(dtmP,p1)->sPtr = pointMark ;
             if( p1 < *leftMostPointP ) *leftMostPointP = p1 ;
             ++*numMarkedP ;
            }
          if( drapeType == 2 )
            {
             nodeAddrP(dtmP,p1)->sPtr = pointMark ;
             nodeAddrP(dtmP,p2)->sPtr = pointMark ;
             if( p1 < *leftMostPointP ) *leftMostPointP = p1 ;
             if( p2 < *leftMostPointP ) *leftMostPointP = p2 ;
             ++*numMarkedP ;
             ++*numMarkedP ;
            }
         }
      }
/*
**  Drape Line Goes External To Tin Hull
*/
    if( fndType == 2 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Line Goes External To Tin Hull") ;
       if( bcdtmDrape_findClosestLineInterceptWithHullDtmObject(dtmP,xls,yls,xle,yle,&drapeType,&p1,&p2,&xi,&yi,&zls) ) goto errexit ;
       if( dbg ) bcdtmWrite_message(0,0,0,"drapeType = %2ld ** p1 = %9ld p2 = %9ld",drapeType,p1,p2) ;
/*
**     No Further Intersections Of Drape Line With Tin Hull
*/
       if( drapeType == 0 ) processDrape = 0 ;
/*
**     Drape Line Coincident With Tin Hull
*/
       if( drapeType == 3 )
         {
          drapeType = 1 ;
          xls = xi ;
          yls = yi ;
          p2 = dtmP->nullPnt ;
          p3 = dtmP->nullPnt ;
          nodeAddrP(dtmP,p1)->sPtr = pointMark ;
          if( p1 < *leftMostPointP ) *leftMostPointP = p1 ;
          ++*numMarkedP ;
         }
/*
**     Drape Line Crosses Gulf In Tin Hull
*/
       else if( drapeType != 0 )
         {
/*
**        Drape Point At Hull Intersection
*/
          xls =  xi ;
          yls = yi  ;
          if( drapeType == 1 )
            {
             nodeAddrP(dtmP,p1)->sPtr = pointMark ;
             if( p1 < *leftMostPointP ) *leftMostPointP = p1 ;
             ++*numMarkedP ;
            }
          if( drapeType == 2 )
            {
             p3 = p1 ; p1 = p2 ; p2 = p3 ;
             if( ( p3 = bcdtmList_nextClkDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ;
             nodeAddrP(dtmP,p1)->sPtr = pointMark ;
             nodeAddrP(dtmP,p2)->sPtr = pointMark ;
             if( p1 < *leftMostPointP ) *leftMostPointP = p1 ;
             if( p2 < *leftMostPointP ) *leftMostPointP = p2 ;
             ++*numMarkedP ;
             ++*numMarkedP ;
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
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Intersecting Tin Line Points Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Intersecting Tin Line Points Error") ;
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
BENTLEYDTM_Private int bcdtmLoad_pointMarksInternalToFenceDtmObject
(
 BC_DTM_OBJ *dtmP,
 double  pnt1X,
 double  pnt1Y,
 double  pnt2X,
 double  pnt2Y,
 long *leftMostPointP,
 long *numMarkedP
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   p1,p2,p3,np1,np2,np3,fndType,drapeType,processDrape ;
 long   lastMarkPoint,firstMarkPoint ;
 double xls,yls,zls,xle,yle,xi,yi,zi ;
/*
**    Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Internal To Fence") ;
/*
** Initialise
*/
 firstMarkPoint = dtmP->nullPnt ;
 lastMarkPoint  = dtmP->nullPnt ;
 xls = pnt1X ; yls = pnt1Y ;
 xle = pnt2X ; yle = pnt2X ;
/*
**  Write Out Line To Be Drapped
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Drape Line = %10.4lf %10.4lf ** %10.4lf %10.4lf",xls,yls,xle,yle) ;
/*
**  Find Triangle Containing Drape Start Point
*/
 processDrape = 1 ;
 if( bcdtmFind_triangleDtmObject(dtmP,xls,yls,&fndType,&p1,&p2,&p3)) goto errexit ;
/*
** Find Type ( fndTypeP ) Return Values
**
**   0 - Data Point Outside Data Set Area
**   1 - Point on Triangle Vertex p1
**   2 - Triangle Found vertices are p1,p2,p3
*/
 if( fndType == 0 )
   {
    p1 = p2 = dtmP->nullPnt ;
    if( bcdtmDrape_findClosestLineInterceptWithHullDtmObject(dtmP,xls,yls,xle,yle,&fndType,&p1,&p2,&xls,&yls,&zls) ) goto errexit ;
/*
**
** fndTypeP == 0  No Intercept
**          == 1  Intesects With Hull Point pnt1P
**          == 2  Intesects With Hull Line  pnt1P-pnt2P
**          == 3  Pulled Onto Next Hull Point
**
*/
   if( fndType == 0 )  processDrape = 0 ;
    else
      {
       if( fndType == 2 )
         {
          p3 = p1 ;
          p1 = p2 ;
          p2 = p3 ;
          if( ( p3 = bcdtmList_nextClkDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ;
         }
      }
   } else if( fndType == 2 ) fndType = 3 ;
/*
**  Plot Start of Profile Line on Surface
*/
 if( processDrape )
   {
    drapeType = fndType ;
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Drape Start Point ** drapeType = %2ld p1 = %8ld p2 = %8ld p3 = %8ld ** %10.4lf %10.4lf",drapeType,p1,p2,p3,xls,yls) ;
    if( drapeType == 1 )
      {
       p2 = p3 = dtmP->nullPnt ;
       nodeAddrP(dtmP,p1)->sPtr = 1 ;
       ++*numMarkedP ;
       if( *leftMostPointP == dtmP->nullPnt || p1 < *leftMostPointP ) *leftMostPointP = p1 ;
      }
    else
      {
       nodeAddrP(dtmP,p1)->sPtr = 1 ;
       nodeAddrP(dtmP,p2)->sPtr = 1 ;
       if( *leftMostPointP == dtmP->nullPnt || p1 < *leftMostPointP ) *leftMostPointP = p1 ;
       if( p2 < *leftMostPointP ) *leftMostPointP = p2 ;
       ++*numMarkedP ;
       ++*numMarkedP ;
      }
   }
/*
**  Scan To Drape Line End
*/
 while ( processDrape )
   {
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"drapeType = %2ld p1 = %8ld p2 = %8ld p3 = %8ld",drapeType,p1,p2,p3) ;
    fndType = bcdtmDrape_getNextPointForDrapeDtmObject(dtmP,xls,yls,xle,yle,&drapeType,p1,p2,p3,&np1,&np2,&np3,&xi,&yi,&zi)  ;
/*
** Return Values
**
**   == 0   Next Point Found
**   == 1   End Point In Triangle
**   == 2   Drape Goes Outside Hull
**   == 3   Error No Intercept Found
**   == 4   System Error Detected - Terminate Processing
*/
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"fndType = %2ld drapeType = %2ld np1 = %8ld np2 = %8ld np3 = %8ld ** %10.4lf %10.4lf %10.4lf",fndType,drapeType,np1,np2,np3,xi,yi,zi) ;
/*
**  Error Detected
*/
    if( fndType == 4 ) goto errexit ;
/*
**  Next Drape Point Not Found
*/
    if( fndType == 3 ) processDrape = 0 ;
/*
**  Next Drape Point Found
*/
    if( fndType == 0 || fndType == 1 )
      {
       xls = xi ;
       yls = yi ;
       zls = zi ;
       p1 = np1 ;
       p2 = np2 ;
       p3 = np3 ;
       if( fndType == 1 ) processDrape = 0 ;
       else
         {
          if( drapeType == 1 )
            {
             nodeAddrP(dtmP,p1)->sPtr = 1 ;
             if( p1 < *leftMostPointP ) *leftMostPointP = p1 ;
             ++*numMarkedP ;
            }
          if( drapeType == 2 )
            {
             nodeAddrP(dtmP,p1)->sPtr = 1 ;
             nodeAddrP(dtmP,p2)->sPtr = 1 ;
             if( p1 < *leftMostPointP ) *leftMostPointP = p1 ;
             if( p2 < *leftMostPointP ) *leftMostPointP = p2 ;
             ++*numMarkedP ;
             ++*numMarkedP ;
            }
         }
      }
/*
**  Drape Line Goes External To Tin Hull
*/
    if( fndType == 2 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Line Goes External To Tin Hull") ;
       if( bcdtmDrape_findClosestLineInterceptWithHullDtmObject(dtmP,xls,yls,xle,yle,&drapeType,&p1,&p2,&xi,&yi,&zls) ) goto errexit ;
       if( dbg ) bcdtmWrite_message(0,0,0,"drapeType = %2ld ** p1 = %9ld p2 = %9ld",drapeType,p1,p2) ;
/*
**     No Further Intersections Of Drape Line With Tin Hull
*/
       if( drapeType == 0 ) processDrape = 0 ;
/*
**     Drape Line Coincident With Tin Hull
*/
       if( drapeType == 3 )
         {
          drapeType = 1 ;
          xls = xi ;
          yls = yi ;
          p2 = dtmP->nullPnt ;
          p3 = dtmP->nullPnt ;
          nodeAddrP(dtmP,p1)->sPtr = 1 ;
          if( p1 < *leftMostPointP ) *leftMostPointP = p1 ;
          ++*numMarkedP ;
         }
/*
**     Drape Line Crosses Gulf In Tin Hull
*/
       else if( drapeType != 0 )
         {
/*
**        Drape Point At Hull Intersection
*/
          xls =  xi ;
          yls = yi  ;
          if( drapeType == 1 )
            {
             nodeAddrP(dtmP,p1)->sPtr = 1 ;
             if( p1 < *leftMostPointP ) *leftMostPointP = p1 ;
             ++*numMarkedP ;
            }
          if( drapeType == 2 )
            {
             p3 = p1 ; p1 = p2 ; p2 = p3 ;
             if( ( p3 = bcdtmList_nextClkDtmObject(dtmP,p1,p2)) < 0 ) goto errexit ;
             nodeAddrP(dtmP,p1)->sPtr = 1 ;
             nodeAddrP(dtmP,p2)->sPtr = 1 ;
             if( p1 < *leftMostPointP ) *leftMostPointP = p1 ;
             if( p2 < *leftMostPointP ) *leftMostPointP = p2 ;
             ++*numMarkedP ;
             ++*numMarkedP ;
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
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Points Internal To Fence") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Marking Points Internal To Fence") ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*==============================================================================*//**
* @memo   Loads The Dtm As A Mesh Suitable For Storing As A Microstation Poly Face Mesh Element
* @doc    Loads The Dtm As A Mesh Suitable For Storing As A Microstation Poly Face Mesh Element
* @notes  The mesh points are stored in meshPtsPP
* @notes  The mesh faces  are stored in meshFacesP
* @notes  The meshPtsPP and meshFacesP arrays can be placed directly in the mdl function
* @notes  to create a poly face mesh. Example :-
* @notes  mdlMesh_newPolyface(&meshP,NULL,meshFacesP,3,numMeshFacesP,meshPtsPP,numMeshPtsP)
* @param  dtmP                  ==> Pointer To Dtm object
* @param  maxTriangles          ==> Maximum Number Of Triangles To Be Returned Per Call
* @param  *loadFunctionP        ==> Pointer To Load Function
* @param  useFence              ==> Load Feature Within Fence < TRUE,FALSE >
* @param  fenceType,            ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape>
* @param  fenceOption           ==> Fence Option <INSIDE(1),OVERLAP(2)>
* @param  fencePts              ==> DPoint3d Array Of Fence Points
* @param  numFencePts           ==> Number Of Fence Points
* @param  userP                 ==> User Pointer Passed Back To User
* @return DTM_SUCCESS or DTM_ERROR
* @author Rob Cormack - March 2008 - rob.cormack@bentley.com
* @version
* @see None
*===============================================================================*/
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmInterruptLoad_triangleMeshFromDtmObject
(
 BC_DTM_OBJ *dtmP,         /* ==> Pointer To Dtm Object                          */
 long maxTriangles,        /* ==> Maximum Number Of Triangles To Load            */
 DTMTriangleMeshCallback loadFunctionP,   /* ==> Pointer To Load Function                       */
 long useFence,            /* ==> Load Feature Within Fence <TRUE,FALSE>         */
 DTMFenceType fenceType,           /* ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape>   */
 DTMFenceOption fenceOption,         /* ==> Fence Option <INSIDE(1),OVERLAP(2)>            */
 DPoint3d  *fencePtsP,          /* ==> DPoint3d Array Of Fence Points                      */
 long numFencePts,         /* ==> Number Of Fence Points                         */
 void *userP               /* ==> User Pointer Passed Back To User               */
)
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long  pnt1, pnt2, pnt3, clPtr, numTriangles;
 bool  voidTriangle, voidsInDtm;
 long  node,startPnt,lastPnt,numTrianglesLoaded,startTime ;
 long  fndType,tinPnt1,tinPnt2,tinPnt3,numMeshPts,minTptrPnt,maxTptrPnt ;
 long  *faceP,*meshFacesP=NULL ;
 DPoint3d   *p3dP,*meshPtsP=NULL ;
 BC_DTM_OBJ *clipDtmP=NULL ;
 DTM_CIR_LIST  *clistP ;
 DPoint3d *pntP ;
 DTM_TIN_NODE  *nodeP,*node1P,*node2P,*node3P ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    startTime = bcdtmClock() ;
    bcdtmWrite_message(0,0,0,"Interrupt Loading Triangle Mesh") ;
    bcdtmWrite_message(0,0,0,"Dtm Object       = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"maxTriangles     = %8ld",maxTriangles) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP    = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"useFence         = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fenceType        = %8ld",fenceType) ;
    bcdtmWrite_message(0,0,0,"fenceOption      = %8ld",fenceOption) ;
    bcdtmWrite_message(0,0,0,"fencePtsP        = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts      = %8ld",numFencePts) ;
    bcdtmWrite_message(0,0,0,"userP            = %p",userP) ;
    if( useFence && numFencePts > 2 )
      {
       for( p3dP = fencePtsP ; p3dP < fencePtsP + numFencePts ; ++p3dP )
         {
          bcdtmWrite_message(0,0,0,"Fence Point[%4ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-fencePtsP),p3dP->x,p3dP->y,p3dP->z) ;
         }
       bcdtmWrite_message(0,0,0,"xMin = %12.5lf xMax = %12.5lf xRange = %12.5lf",dtmP->xMin,dtmP->xMax,dtmP->xRange) ;
       bcdtmWrite_message(0,0,0,"yMin = %12.5lf yMax = %12.5lf yRange = %12.5lf",dtmP->yMin,dtmP->yMax,dtmP->yRange) ;
       bcdtmWrite_message(0,0,0,"zMin = %12.5lf zMax = %12.5lf zRange = %12.5lf",dtmP->zMin,dtmP->zMax,dtmP->zRange) ;
      }
   }
/*
** Validate Fence
*/
 if( useFence == TRUE && ( fencePtsP == NULL || numFencePts <= 2 ) ) useFence = FALSE ;
 if( useFence == TRUE && ( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )) useFence = FALSE ;
 if( useFence )
   {
    if( fenceType   != DTMFenceType::Block && fenceType   != DTMFenceType::Shape ) fenceType = DTMFenceType::Block ;
    if( fenceOption != DTMFenceOption::Inside      && fenceOption != DTMFenceOption::Outside && fenceOption != DTMFenceOption::Overlap ) fenceOption = DTMFenceOption::Overlap ;
   }
/*
** Validate Mesh Size
*/
 if( maxTriangles <= 0 ) maxTriangles = 50000 ;
 if( maxTriangles > dtmP->numTriangles ) maxTriangles = dtmP->numTriangles ;
/*
** Test For Valid Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Valid Dtm Object") ;
 if( bcdtmObject_testForValidDtmObject(dtmP) ) goto errexit ;
/*
**  Check If DTM Is In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Check For Voids In The Triangulation
*/
 voidsInDtm = false ;
 bcdtmList_testForVoidsInDtmObject(dtmP,voidsInDtm) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"voidsInDtm = %2ld",voidsInDtm) ;
/*
**  Build Clipping Dtm For Fence
*/
 if( useFence == TRUE )
   {
    if( bcdtmClip_buildClippingTinFromFencePointsDtmObject(&clipDtmP,fencePtsP,numFencePts)) goto errexit ;
    if( fenceType == DTMFenceType::Block && dtmP->xMin >= clipDtmP->xMin && dtmP->xMax <= clipDtmP->xMax &&  dtmP->yMin >= clipDtmP->yMin && dtmP->yMax <= clipDtmP->yMax ) useFence = FALSE ;
    if( dbg ) bcdtmWrite_message(0,0,0,"useFence = %2ld",useFence) ;
   }
/*
** Initialise
*/
 startPnt = 0 ;
 lastPnt  = dtmP->numPoints - 1 ;
 numTriangles = 0 ;
 numTrianglesLoaded = 0 ;
// bcdtmList_nullSptrValuesDtmObject(dtmP) ;
// bcdtmList_nullTptrValuesDtmObject(dtmP) ;
/*
**  Find Points Immediately Before And After Fence
*/
 if( useFence == TRUE )
   {
    startTime = bcdtmClock() ;
    bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMin,&startPnt) ;
    while( startPnt > 0 && pointAddrP(dtmP,startPnt)->x >= clipDtmP->xMin ) --startPnt ;
    if( pointAddrP(dtmP,startPnt)->x < clipDtmP->xMin ) ++startPnt ;
    bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMax,&lastPnt) ;
    while( lastPnt < dtmP->numPoints - 1  && pointAddrP(dtmP,lastPnt)->x <= clipDtmP->xMin ) ++lastPnt ;
    if( tdbg )
      {
       bcdtmWrite_message(0,0,0,"startPnt = %8ld startPnt->x = %12.5lf",startPnt,pointAddrP(dtmP,startPnt)->x) ;
       bcdtmWrite_message(0,0,0,"lastPnt  = %8ld lastPnt->x  = %12.5lf",lastPnt,pointAddrP(dtmP,lastPnt)->x) ;
       bcdtmWrite_message(0,0,0,"clipDtmP->xMin  = %12.5lf clipDtmP->xMax  = %12.5lf",clipDtmP->xMin,clipDtmP->xMax) ;
       bcdtmWrite_message(0,0,0,"lastPnt  = %8ld lastPnt->x  = %12.5lf",lastPnt,pointAddrP(dtmP,lastPnt)->x) ;
       bcdtmWrite_message(0,0,0,"** Index Time 00 = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
      }
    if( startPnt == 0 && lastPnt == dtmP->numPoints ) useFence = 0 ;
/*
**  Mark Tin Points Within Block Fence
*/
    if( useFence == TRUE )
      {
       if( fenceType == DTMFenceType::Block )
         {
          for( pnt1 = startPnt ; pnt1 <= lastPnt ; ++pnt1 )
            {
             pntP = pointAddrP(dtmP,pnt1) ;
             if( pntP->x >= clipDtmP->xMin && pntP->x <= clipDtmP->xMax &&
                 pntP->y >= clipDtmP->yMin && pntP->y <= clipDtmP->yMax     ) nodeAddrP(dtmP,pnt1)->sPtr = 1 ;
            }
         }
/*
**    Mark Tin Points Within Shape Fence
*/
       if( fenceType == DTMFenceType::Shape )
         {
          for( pnt1 = startPnt ; pnt1 <= lastPnt ; ++pnt1 )
            {
             pntP = pointAddrP(dtmP,pnt1) ;
             if( pntP->x >= clipDtmP->xMin && pntP->x <= clipDtmP->xMax &&
                 pntP->y >= clipDtmP->yMin && pntP->y <= clipDtmP->yMax     )
               {
                if( bcdtmFind_triangleDtmObject(dtmP,pntP->x,pntP->y,&fndType,&tinPnt1,&tinPnt2,&tinPnt3)) goto errexit ;
                if( fndType ) nodeAddrP(dtmP,pnt1)->sPtr = 1 ;
               }
            }
         }
      }
   }
/*
** Write Start Scan Point And Last Scan Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"startPnt = %8ld lastPnt = %8ld",startPnt,lastPnt) ;
/*
** Allocate Memory For Mesh Faces
*/
 meshFacesP = (long * ) malloc( maxTriangles * 3 * sizeof(long)) ;
 if( meshFacesP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
 faceP = meshFacesP ;
/*
** Scan DTM And Accumulate Triangle Mesh
*/
 minTptrPnt = dtmP->numPoints ;
 maxTptrPnt = -1 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning DTM Triangles") ;
 for( pnt1 = startPnt ; pnt1 <= lastPnt ; ++pnt1 )
   {
    node1P = nodeAddrP(dtmP,pnt1) ;
    if( useFence == FALSE || ( useFence == TRUE && node1P->sPtr == 1 ))
      {
       if( ( clPtr = node1P->cPtr ) != dtmP->nullPtr )
         {
          if( ( pnt2 = bcdtmList_nextAntDtmObject(dtmP,pnt1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
          node2P = nodeAddrP(dtmP,pnt2) ;
          while( clPtr != dtmP->nullPtr )
            {
             clistP = clistAddrP(dtmP,clPtr) ;
             pnt3   = clistP->pntNum ;
             clPtr  = clistP->nextPtr ;
             node3P = nodeAddrP(dtmP,pnt3) ;
             if( node1P->hPtr != pnt2 )
               {
                if( ( pnt2 > pnt1 && pnt3 > pnt1 ) && ( node2P->sPtr == dtmP->nullPnt || node3P->sPtr == dtmP->nullPnt )  )
                  {
/*
**                 Test For Void Triangle
*/
                   voidTriangle = FALSE ;
                   if( voidsInDtm == TRUE ) if( bcdtmList_testForVoidTriangleDtmObject(dtmP,pnt1,pnt2,pnt3,voidTriangle)) goto errexit ;
/*
**                 Process If None Void Triangle
*/
                   if( voidTriangle == FALSE )
                     {
                      *faceP = pnt1  ; ++faceP ;
                      *faceP = pnt2  ; ++faceP ;
                      *faceP = pnt3  ; ++faceP ;
                      ++numTriangles ;
/*
**                    Check For Maximum Load Triangles
*/
                      if( numTriangles == maxTriangles )
                        {
                         if( dbg ) bcdtmWrite_message(0,0,0,"Creating Triangle Mesh ** numTriangles = %8ld",numTriangles) ;
/*
**                       Mark Mesh Points
*/
                         minTptrPnt = dtmP->numPoints ;
                         maxTptrPnt = -1 ;
                         for( faceP = meshFacesP ; faceP < meshFacesP + numTriangles * 3 ; ++faceP )
                           {
                            if( *faceP < minTptrPnt ) minTptrPnt = *faceP ;
                            if( *faceP > maxTptrPnt ) maxTptrPnt = *faceP ;
                            nodeAddrP(dtmP,*faceP)->tPtr = 1 ;
                           }
                         if( dbg ) bcdtmWrite_message(0,0,0,"minTptrPoint = %8ld maxTptrPoint = %8ld",minTptrPnt,maxTptrPnt) ;
/*
**                       Count Number Of Mesh Points
*/
                         numMeshPts = 0 ;
                         for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
                           {
                            nodeP = nodeAddrP(dtmP,node) ;
                            if( nodeP->tPtr == 1 )
                              {
                               ++numMeshPts ;
                               nodeP->tPtr = numMeshPts ;
                              }
                           }
/*
**                       Allocate Memory For Mesh Points
*/
                         if( dbg ) bcdtmWrite_message(0,0,0,"Populating Mesh Points ** numMeshPts = %8ld",numMeshPts) ;
                         meshPtsP = ( DPoint3d * ) malloc( numMeshPts * sizeof(DPoint3d) ) ;
                         if( meshPtsP == NULL )
                           {
                            bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                            goto errexit ;
                           }
/*
**                       Populate Mesh Points Array
*/
                         p3dP = meshPtsP ;
                         for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
                           {
                            nodeP = nodeAddrP(dtmP,node) ;
                            if( nodeP->tPtr > 0 && nodeP->tPtr < dtmP->nullPnt )
                              {
                               pntP    = pointAddrP(dtmP,node) ;
                               p3dP->x = pntP->x ;
                               p3dP->y = pntP->y ;
                               p3dP->z = pntP->z ;
                               ++p3dP ;
                              }
                           }
/*
**                       Reset Point Indexes In Mesh Faces
*/
                         if( dbg ) bcdtmWrite_message(0,0,0,"Resting Point Indexes In Mesh Faces") ;
                         for( faceP = meshFacesP ; faceP < meshFacesP + numTriangles * 3 ; ++faceP )
                           {
                            *faceP = nodeAddrP(dtmP,*faceP)->tPtr ;
                           }
/*
**                       Null Tptr Values
*/
                         for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
                           {
                            nodeAddrP(dtmP,node)->tPtr = dtmP->nullPnt ;
                           }
/*
**                       Call Browse Function
*/
                         if( dbg ) bcdtmWrite_message(0,0,0,"Calling Load Function dtmP->numtriangles = %8ld ** numTriangles = %8ld numMeshPts = %8ld numMeshFaces = %8ld",dtmP->numTriangles,numTriangles,numMeshPts,maxTriangles*3) ;
                         if( loadFunctionP(DTMFeatureType::TriangleMesh,numTriangles,numMeshPts,meshPtsP,numTriangles*3,meshFacesP,userP)) goto errexit ;
                         faceP = meshFacesP ;
                         if( meshPtsP   != NULL ) { free(meshPtsP)   ; meshPtsP   = NULL ; }
                         numTrianglesLoaded = numTrianglesLoaded + numTriangles ;
                         numTriangles = 0 ;
                         minTptrPnt = dtmP->numPoints ;
                         maxTptrPnt = -1 ;
                        }
                     }
                  }
               }
/*
**           Continue Scan For Triangles
*/
             pnt2 = pnt3 ;
             node2P = node3P ;
            }
         }
      }
   }
/*
** Check For Unloaded Triangles
*/
 if( numTriangles > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Creating Triangle Mesh For Unloaded Triangles ** numTriangles = %8ld",numTriangles) ;
/*
**  Mark Mesh Points
*/
    minTptrPnt = dtmP->numPoints ;
    maxTptrPnt = -1 ;
    for( faceP = meshFacesP ; faceP < meshFacesP + numTriangles * 3 ; ++faceP )
      {
       if( *faceP < minTptrPnt ) minTptrPnt = *faceP ;
       if( *faceP > maxTptrPnt ) maxTptrPnt = *faceP ;
       nodeAddrP(dtmP,*faceP)->tPtr = 1 ;
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"minTptrPoint = %8ld maxTptrPoint = %8ld",minTptrPnt,maxTptrPnt) ;
/*
**  Count Number Of Mesh Points
*/
    numMeshPts = 0 ;
    for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
      {
       nodeP = nodeAddrP(dtmP,node) ;
       if( nodeP->tPtr == 1 )
         {
          ++numMeshPts ;
          nodeP->tPtr = numMeshPts ;
         }
      }
/*
**  Allocate Memory For Mesh Points
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Mesh Points Memory ** numMeshPts = %8ld",numMeshPts) ;
    meshPtsP = ( DPoint3d * ) malloc( numMeshPts * sizeof(DPoint3d) ) ;
    if( meshPtsP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  Populate Mesh Points Array
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Polulating Mesh Points Memory ** numMeshPts = %8ld",numMeshPts) ;
    p3dP = meshPtsP ;
    for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
      {
       nodeP = nodeAddrP(dtmP,node) ;
       if( nodeP->tPtr > 0 && nodeP->tPtr < dtmP->nullPnt )
         {
          pntP = pointAddrP(dtmP,node) ;
          p3dP->x = pntP->x ;
          p3dP->y = pntP->y ;
          p3dP->z = pntP->z ;
          ++p3dP ;
         }
      }
/*
**  Reset Point Indexes In Mesh Faces
*/
    for( faceP = meshFacesP ; faceP < meshFacesP + numTriangles * 3 ; ++faceP )
      {
       *faceP = nodeAddrP(dtmP,*faceP)->tPtr ;
      }
/*
**  Null Tptr Values
*/
    for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
      {
       nodeAddrP(dtmP,node)->tPtr = dtmP->nullPnt ;
      }
/*
**  Call Browse Function
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"** Calling Load Function dtmP->numtriangles = %8ld ** numTriangles = %8ld numMeshPts = %8ld numMeshFaces = %8ld",dtmP->numTriangles,numTriangles,numMeshPts,numTriangles * 3) ;
    if( loadFunctionP(DTMFeatureType::TriangleMesh,numTriangles,numMeshPts,meshPtsP,numTriangles*3,meshFacesP,userP)) goto errexit ;
    faceP = meshFacesP ;
    if( meshPtsP   != NULL ) { free(meshPtsP)   ; meshPtsP   = NULL ; }
    numTrianglesLoaded = numTrianglesLoaded + numTriangles ;
    numTriangles = 0 ;
    minTptrPnt = dtmP->numPoints ;
    maxTptrPnt = -1 ;
   }
/*
** Clean Up
*/
 cleanup :
 if ( useFence == TRUE )
   {
    for( node = startPnt ; node <= lastPnt ; ++node )
      {
       nodeAddrP(dtmP,node)->tPtr = dtmP->nullPnt ;
       nodeAddrP(dtmP,node)->sPtr = dtmP->nullPnt ;
      }
   }
 if( meshPtsP   != NULL ) { free(meshPtsP)   ; meshPtsP   = NULL ; }
 if( meshFacesP != NULL ) { free(meshFacesP) ; meshFacesP = NULL ; }
 if( clipDtmP   != NULL ) bcdtmObject_destroyDtmObject(&clipDtmP) ;
/*
** Job Completed
*/
 if( tdbg )
   {
    bcdtmWrite_message(0,0,0,"Number Of Triangles Loaded  = %10ld ** dtmP->numTriangles = %10ld",numTrianglesLoaded,dtmP->numTriangles) ;
    bcdtmWrite_message(0,0,0,"Time To Load Triangle Mesh  = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime) ) ;
   }
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Interrupt Loading Triangle Mesh Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Interrupt Loading Triangle Mesh Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR  ;
 goto cleanup ;
}
/*==============================================================================*//**
* @memo   Loads The Dtm As A Mesh Suitable For Storing As A Microstation Poly Face Mesh Element
* @doc    Loads The Dtm As A Mesh Suitable For Storing As A Microstation Poly Face Mesh Element
* @notes  The mesh points are stored in meshPtsPP
* @notes  The mesh faces  are stored in meshFacesP
* @notes  The meshPtsPP and meshFacesP arrays can be placed directly in the mdl function
* @notes  to create a poly face mesh. Example :-
* @notes  mdlMesh_newPolyface(&meshP,NULL,meshFacesP,3,numMeshFacesP,meshPtsPP,numMeshPtsP)
* @param  dtmP                  ==> Pointer To Dtm object
* @param  maxTriangles          ==> Maximum Number Of Triangles To Be Returned Per Call
* @param  *loadFunctionP        ==> Pointer To Load Function
* @param  useFence              ==> Load Feature Within Fence < TRUE,FALSE >
* @param  fenceType,            ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape>
* @param  fenceOption           ==> Fence Option <INSIDE(1),OVERLAP(2)>
* @param  fencePts              ==> DPoint3d Array Of Fence Points
* @param  numFencePts           ==> Number Of Fence Points
* @param  userP                 ==> User Pointer Passed Back To User
* @return DTM_SUCCESS or DTM_ERROR
* @author Rob Cormack - March 2008 - rob.cormack@bentley.com
* @version
* @see None
*===============================================================================*/
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmInterruptLoadCSharp_triangleMeshFromDtmObject
(
 BC_DTM_OBJ *dtmP,                         /* ==> Pointer To Dtm Object                          */
 long maxTriangles,                        /* ==> Maximum Number Of Triangles To Load            */
 CSharpDtmTinMeshCallBack loadFunctionP,   /* ==> Pointer To Load Function                       */
 long useFence,                            /* ==> Load Feature Within Fence <TRUE,FALSE>         */
 DTMFenceType fenceType,                           /* ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape>   */
 DTMFenceOption fenceOption,                         /* ==> Fence Option <INSIDE(1),OVERLAP(2)>            */
 DPoint3d  *fencePtsP,                          /* ==> DPoint3d Array Of Fence Points                      */
 long numFencePts,                         /* ==> Number Of Fence Points                         */
 void *userP                               /* ==> User Pointer Passed Back To User               */
)
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long  pnt1, pnt2, pnt3, clPtr, numTriangles;
 bool  voidTriangle, voidsInDtm;
 long  node,startPnt,lastPnt,numTrianglesLoaded,startTime ;
 long  fndType,tinPnt1,tinPnt2,tinPnt3,numMeshPts,minTptrPnt,maxTptrPnt ;
 long  *faceP,*meshFacesP=NULL ;
 DPoint3d   *p3dP,*meshPtsP=NULL ;
 BC_DTM_OBJ *clipDtmP=NULL ;
 DTM_CIR_LIST  *clistP ;
 DPoint3d *pntP ;
 DTM_TIN_NODE  *nodeP,*node1P,*node2P,*node3P ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    startTime = bcdtmClock() ;
    bcdtmWrite_message(0,0,0,"Interrupt Loading Triangle Mesh") ;
    bcdtmWrite_message(0,0,0,"Dtm Object       = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"maxTriangles     = %8ld",maxTriangles) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP    = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"useFence         = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fenceType        = %8ld",fenceType) ;
    bcdtmWrite_message(0,0,0,"fenceOption      = %8ld",fenceOption) ;
    bcdtmWrite_message(0,0,0,"fencePtsP        = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts      = %8ld",numFencePts) ;
    bcdtmWrite_message(0,0,0,"userP            = %p",userP) ;
   if( useFence && numFencePts > 2 )
      {
       for( p3dP = fencePtsP ; p3dP < fencePtsP + numFencePts ; ++p3dP )
         {
          bcdtmWrite_message(0,0,0,"Fence Point[%4ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-fencePtsP),p3dP->x,p3dP->y,p3dP->z) ;
         }
       bcdtmWrite_message(0,0,0,"xMin = %12.5lf xMax = %12.5lf xRange = %12.5lf",dtmP->xMin,dtmP->xMax,dtmP->xRange) ;
       bcdtmWrite_message(0,0,0,"yMin = %12.5lf yMax = %12.5lf yRange = %12.5lf",dtmP->yMin,dtmP->yMax,dtmP->yRange) ;
       bcdtmWrite_message(0,0,0,"zMin = %12.5lf zMax = %12.5lf zRange = %12.5lf",dtmP->zMin,dtmP->zMax,dtmP->zRange) ;
      }
   }
/*
** Validate Fence
*/
 if( useFence == TRUE && ( fencePtsP == NULL || numFencePts <= 2 ) ) useFence = FALSE ;
 if( useFence == TRUE && ( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )) useFence = FALSE ;
 if( useFence )
   {
    if( fenceType   != DTMFenceType::Block && fenceType   != DTMFenceType::Shape ) fenceType = DTMFenceType::Block ;
    if( fenceOption != DTMFenceOption::Inside      && fenceOption != DTMFenceOption::Outside && fenceOption != DTMFenceOption::Overlap ) fenceOption = DTMFenceOption::Overlap ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"After useFence = %2ld ** fenceType = %2ld fenceOption = %2ld",useFence,fenceType,fenceOption) ;
/*
** Validate Mesh Size
*/
 if( maxTriangles <= 0 ) maxTriangles = 50000 ;
 if( maxTriangles > dtmP->numTriangles ) maxTriangles = dtmP->numTriangles ;
/*
** Test For Valid Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Valid Dtm Object") ;
 if( bcdtmObject_testForValidDtmObject(dtmP) ) goto errexit ;
/*
**  Check If DTM Is In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Check For Voids In The Triangulation
*/
 voidsInDtm = false ;
 bcdtmList_testForVoidsInDtmObject(dtmP,voidsInDtm) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"voidsInDtm = %2ld",voidsInDtm) ;
/*
**  Build Clipping Dtm For Fence
*/
 if( useFence == TRUE )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Building Clipping DTM For Fence") ;
    if( bcdtmClip_buildClippingTinFromFencePointsDtmObject(&clipDtmP,fencePtsP,numFencePts)) goto errexit ;
    if( fenceType == DTMFenceType::Block && dtmP->xMin >= clipDtmP->xMin && dtmP->xMax <= clipDtmP->xMax &&  dtmP->yMin >= clipDtmP->yMin && dtmP->yMax <= clipDtmP->yMax ) useFence = FALSE ;
    if( dbg ) bcdtmWrite_message(0,0,0,"useFence = %2ld",useFence) ;
   }
/*
** Initialise
*/
 startPnt = 0 ;
 lastPnt  = dtmP->numPoints - 1 ;
 numTriangles = 0 ;
 numTrianglesLoaded = 0 ;
 bcdtmList_nullSptrValuesDtmObject(dtmP) ;
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
/*
**  Find Points Immediately Before And After Fence
*/
 if( useFence == TRUE )
   {
    startTime = bcdtmClock() ;
    bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMin,&startPnt) ;
    while( startPnt > 0 && pointAddrP(dtmP,startPnt)->x >= clipDtmP->xMin ) --startPnt ;
    if( pointAddrP(dtmP,startPnt)->x < clipDtmP->xMin ) ++startPnt ;
    bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMax,&lastPnt) ;
    while( lastPnt < dtmP->numPoints - 1  && pointAddrP(dtmP,lastPnt)->x <= clipDtmP->xMin ) ++lastPnt ;
    if( tdbg )
      {
       bcdtmWrite_message(0,0,0,"startPnt = %8ld startPnt->x = %12.5lf",startPnt,pointAddrP(dtmP,startPnt)->x) ;
       bcdtmWrite_message(0,0,0,"lastPnt  = %8ld lastPnt->x  = %12.5lf",lastPnt,pointAddrP(dtmP,lastPnt)->x) ;
       bcdtmWrite_message(0,0,0,"clipDtmP->xMin  = %12.5lf clipDtmP->xMax  = %12.5lf",clipDtmP->xMin,clipDtmP->xMax) ;
       bcdtmWrite_message(0,0,0,"lastPnt  = %8ld lastPnt->x  = %12.5lf",lastPnt,pointAddrP(dtmP,lastPnt)->x) ;
       bcdtmWrite_message(0,0,0,"** Index Time 00 = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
      }
/*
**  Mark Tin Points Within Block Fence
*/
    if( fenceType == DTMFenceType::Block )
      {
       for( pnt1 = startPnt ; pnt1 <= lastPnt ; ++pnt1 )
         {
          pntP = pointAddrP(dtmP,pnt1) ;
          if( pntP->x >= clipDtmP->xMin && pntP->x <= clipDtmP->xMax &&
              pntP->y >= clipDtmP->yMin && pntP->y <= clipDtmP->yMax     ) nodeAddrP(dtmP,pnt1)->sPtr = 1 ;
         }
      }
/*
**  Mark Tin Points Within Shape Fence
*/
    if( fenceType == DTMFenceType::Shape )
      {
       for( pnt1 = startPnt ; pnt1 <= lastPnt ; ++pnt1 )
         {
          pntP = pointAddrP(dtmP,pnt1) ;
          if( pntP->x >= clipDtmP->xMin && pntP->x <= clipDtmP->xMax &&
              pntP->y >= clipDtmP->yMin && pntP->y <= clipDtmP->yMax     )
            {
             if( bcdtmFind_triangleDtmObject(dtmP,pntP->x,pntP->y,&fndType,&tinPnt1,&tinPnt2,&tinPnt3)) goto errexit ;
             if( fndType ) nodeAddrP(dtmP,pnt1)->sPtr = 1 ;
            }
         }
      }
   }
/*
** Write Start Scan Point And Last Scan Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"startPnt = %8ld lastPnt = %8ld",startPnt,lastPnt) ;
/*
** Allocate Memory For Mesh Faces
*/
 meshFacesP   = (long * ) malloc( maxTriangles * 3 * sizeof(long)) ;
 if( meshFacesP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Scan DTM And Accumulate Triangle Mesh
*/
 numTriangles = 0 ;
 faceP = meshFacesP ;
 maxTptrPnt = -1 ;
 minTptrPnt = dtmP->numPoints ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning DTM Triangles") ;
 for( pnt1 = startPnt ; pnt1 <= lastPnt ; ++pnt1 )
   {
    node1P = nodeAddrP(dtmP,pnt1) ;
    if( useFence == FALSE || ( useFence == TRUE && node1P->sPtr == 1 ))
      {
       if( ( clPtr = node1P->cPtr ) != dtmP->nullPtr )
         {
          if( ( pnt2 = bcdtmList_nextAntDtmObject(dtmP,pnt1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
          node2P = nodeAddrP(dtmP,pnt2) ;
          while( clPtr != dtmP->nullPtr )
            {
             clistP = clistAddrP(dtmP,clPtr) ;
             pnt3   = clistP->pntNum ;
             clPtr  = clistP->nextPtr ;
             node3P = nodeAddrP(dtmP,pnt3) ;
             if( node1P->hPtr != pnt2 )
               {
//                if( ( pnt1 < pnt2 && pnt1 < pnt3 ) && ( useFence == FALSE || ( useFence == TRUE && ( node2P->sPtr == 1 || node3P->sPtr == 1 ))))
                if( ( pnt1 < pnt2 && pnt1 < pnt3 ) )
                  {
/*
**                 Test For Void Triangle
*/
                   voidTriangle = false ;
                   if( voidsInDtm == true ) if( bcdtmList_testForVoidTriangleDtmObject(dtmP,pnt1,pnt2,pnt3,voidTriangle)) goto errexit ;
/*
**                 Process If None Void Triangle
*/
                   if( voidTriangle == false )
                     {
                      *faceP = pnt1  ; ++faceP ;
                      *faceP = pnt2  ; ++faceP ;
                      *faceP = pnt3  ; ++faceP ;
                      ++numTriangles ;
/*
**                    Check For Maximum Load Triangles
*/
                      if( numTriangles == maxTriangles )
                        {
                         if( dbg ) bcdtmWrite_message(0,0,0,"Creating Triangle Mesh ** numTriangles = %8ld",numTriangles) ;
/*
**                       Mark Mesh Points
*/
                         minTptrPnt = dtmP->numPoints ;
                         maxTptrPnt = -1 ;
                         for( faceP = meshFacesP ; faceP < meshFacesP + numTriangles * 3 ; ++faceP )
                           {
                            if( *faceP < minTptrPnt ) minTptrPnt = *faceP ;
                            if( *faceP > maxTptrPnt ) maxTptrPnt = *faceP ;
                            nodeAddrP(dtmP,*faceP)->tPtr = 1 ;
                           }
                         if( dbg ) bcdtmWrite_message(0,0,0,"minTptrPoint = %8ld maxTptrPoint = %8ld",minTptrPnt,maxTptrPnt) ;
/*
**                       Count Number Of Mesh Points
*/
                         numMeshPts = 0 ;
                         for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
                           {
                            nodeP = nodeAddrP(dtmP,node) ;
                            if( nodeP->tPtr == 1 )
                              {
                               ++numMeshPts ;
                               nodeP->tPtr = numMeshPts ;
                              }
                           }
/*
**                       Allocate Memory For Mesh Points
*/
                         if( dbg ) bcdtmWrite_message(0,0,0,"Populating Mesh Points ** numMeshPts = %8ld",numMeshPts) ;
                         meshPtsP = ( DPoint3d * ) malloc( numMeshPts * sizeof(DPoint3d) ) ;
                         if( meshPtsP == NULL )
                           {
                            bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                            goto errexit ;
                           }
/*
**                       Populate Mesh Points Array
*/
                         p3dP = meshPtsP ;
                         for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
                           {
                            nodeP = nodeAddrP(dtmP,node) ;
                            if( nodeP->tPtr > 0 && nodeP->tPtr < dtmP->nullPnt )
                              {
                               pntP    = pointAddrP(dtmP,node) ;
                               p3dP->x = pntP->x ;
                               p3dP->y = pntP->y ;
                               p3dP->z = pntP->z ;
                               ++p3dP ;
                              }
                           }
/*
**                       Reset Point Indexes In Mesh Faces
*/
                         if( dbg ) bcdtmWrite_message(0,0,0,"Resting Point Indexes In Mesh Faces") ;
                         for( faceP = meshFacesP ; faceP < meshFacesP + numTriangles * 3 ; ++faceP )
                           {
                            *faceP = nodeAddrP(dtmP,*faceP)->tPtr ;
                           }
/*
**                       Null Tptr Values
*/
                         for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
                           {
                            nodeAddrP(dtmP,node)->tPtr = dtmP->nullPnt ;
                           }
/*
**                       Call Browse Function
*/
                         if( dbg ) bcdtmWrite_message(0,0,0,"Calling Load Function dtmP->numtriangles = %8ld ** numTriangles = %8ld numMeshPts = %8ld numMeshFaces = %8ld",dtmP->numTriangles,numTriangles,numMeshPts,maxTriangles*3) ;
                         if( loadFunctionP(DTMFeatureType::TriangleMesh,numTriangles,numMeshPts,meshPtsP,numTriangles*3,meshFacesP,userP)) goto errexit ;
                         faceP = meshFacesP ;
                         if( meshPtsP   != NULL ) { free(meshPtsP)   ; meshPtsP   = NULL ; }
                         numTrianglesLoaded = numTrianglesLoaded + numTriangles ;
                         numTriangles = 0 ;
                         minTptrPnt = dtmP->numPoints ;
                         maxTptrPnt = -1 ;
                        }
                     }
                  }
               }
/*
**           Continue Scan For Triangles
*/
             pnt2 = pnt3 ;
             node2P = node3P ;
            }
         }
      }
   }
/*
** Check For Unloaded Triangles
*/
 if( numTriangles > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"** Creating Triangle Mesh For Unloaded Triangles ** numTriangles = %8ld",numTriangles) ;
/*
**  Mark Mesh Points
*/
    minTptrPnt = dtmP->numPoints ;
    maxTptrPnt = -1 ;
    for( faceP = meshFacesP ; faceP < meshFacesP + numTriangles * 3 ; ++faceP )
      {
       if( *faceP < minTptrPnt ) minTptrPnt = *faceP ;
       if( *faceP > maxTptrPnt ) maxTptrPnt = *faceP ;
       nodeAddrP(dtmP,*faceP)->tPtr = 1 ;
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"minTptrPoint = %8ld maxTptrPoint = %8ld",minTptrPnt,maxTptrPnt) ;
/*
**  Count Number Of Mesh Points
*/
    numMeshPts = 0 ;
    for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
      {
       nodeP = nodeAddrP(dtmP,node) ;
       if( nodeP->tPtr == 1 )
         {
          ++numMeshPts ;
          nodeP->tPtr = numMeshPts ;
         }
      }
/*
**  Allocate Memory For Mesh Points
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Allocating Mesh Points Memory ** numMeshPts = %8ld",numMeshPts) ;
    meshPtsP = ( DPoint3d * ) malloc( numMeshPts * sizeof(DPoint3d) ) ;
    if( meshPtsP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  Populate Mesh Points Array
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Polulating Mesh Points Memory ** numMeshPts = %8ld",numMeshPts) ;
    p3dP = meshPtsP ;
    for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
      {
       nodeP = nodeAddrP(dtmP,node) ;
       if( nodeP->tPtr > 0 && nodeP->tPtr < dtmP->nullPnt )
         {
          pntP = pointAddrP(dtmP,node) ;
          p3dP->x = pntP->x ;
          p3dP->y = pntP->y ;
          p3dP->z = pntP->z ;
          ++p3dP ;
         }
      }
/*
**  Reset Point Indexes In Mesh Faces
*/
    for( faceP = meshFacesP ; faceP < meshFacesP + numTriangles * 3 ; ++faceP )
      {
       *faceP = nodeAddrP(dtmP,*faceP)->tPtr ;
       if( *faceP > numMeshPts )
         {
          bcdtmWrite_message(2,0,0,"Invalid Index For Mesh Face **  face Index = %8ld numMeshPts = %8ld",*faceP,numMeshPts) ;
          goto errexit ;
         }
      }
/*
**  Null Tptr Values
*/
    for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
      {
       nodeAddrP(dtmP,node)->tPtr = dtmP->nullPnt ;
      }
/*
**  Call Browse Function
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"** Calling Load Function dtmP->numtriangles = %8ld ** numTriangles = %8ld numMeshPts = %8ld numMeshFaces = %8ld",dtmP->numTriangles,numTriangles,numMeshPts,numTriangles * 3) ;
    if( loadFunctionP(DTMFeatureType::TriangleMesh,numTriangles,numMeshPts,meshPtsP,numTriangles*3,meshFacesP,userP)) goto errexit ;
    faceP = meshFacesP ;
    if( meshPtsP   != NULL ) { free(meshPtsP)   ; meshPtsP   = NULL ; }
    numTrianglesLoaded = numTrianglesLoaded + numTriangles ;
    numTriangles = 0 ;
    minTptrPnt = dtmP->numPoints ;
    maxTptrPnt = -1 ;
   }
/*
** Clean Up
*/
 cleanup :
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
 bcdtmList_nullSptrValuesDtmObject(dtmP) ;
 if( meshPtsP   != NULL ) { free(meshPtsP)   ; meshPtsP   = NULL ; }
 if( meshFacesP != NULL ) { free(meshFacesP) ; meshFacesP = NULL ; }
 if( clipDtmP   != NULL ) bcdtmObject_destroyDtmObject(&clipDtmP) ;
/*
** Job Completed
*/
 if( dbg  ) bcdtmWrite_message(0,0,0,"Number Of Triangles Loaded  = %10ld ** dtmP->numTriangles = %10ld",numTrianglesLoaded,dtmP->numTriangles) ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Load Triangle Mesh  = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime) ) ;
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Interrupt Loading Triangle Mesh Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Interrupt Loading Triangle Mesh Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR  ;
 goto cleanup ;
}
/*==============================================================================*//**
* @memo   Loads The Dtm As A Mesh For Rendering Or Shading Purposes
* @doc    Loads The Dtm As A Mesh For Rendering Or Shading Purposes
* @notes  The mesh points are stored in meshPtsPP
* @notes  The mesh faces  are stored in meshFacesP
* @notes  The meshPtsPP and meshFacesP arrays can be placed directly in the mdl function
* @notes  to create a poly face mesh. Example :-
* @notes  mdlMesh_newPolyface(&meshP,NULL,meshFacesP,3,numMeshFacesP,meshPtsPP,numMeshPtsP)
* @param  dtmP                  ==> Pointer To Dtm object
* @param  maxTriangles          ==> Maximum Number Of Triangles To Be Returned Per Call
* @param  *loadFunctionP        ==> Pointer To Load Function
* @param  useFence              ==> Load Feature Within Fence < TRUE,FALSE >
* @param  fenceType,            ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape>
* @param  fenceOption           ==> Fence Option <INSIDE(1),OVERLAP(2)>
* @param  fencePts              ==> DPoint3d Array Of Fence Points
* @param  numFencePts           ==> Number Of Fence Points
* @param  userP                 ==> User Pointer Passed Back To User
* @return DTM_SUCCESS or DTM_ERROR
* @author Rob Cormack - March 2008 - rob.cormack@bentley.com
* @version
* @see None
*===============================================================================*/
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmInterruptLoad_triangleShadeMeshFromDtmObjectOld
(
 BC_DTM_OBJ *dtmP,         /* ==> Pointer To Dtm Object                              */
 long maxTriangles,        /* ==> Maximum Number Of Triangles To Load Per Call       */
 long vectorOption,        /* ==> Vector Option <1=Surface Derivatives,2=Averaged Triangle Surface Normals> */
 double zAxisFactor,       /* ==> Factor To Exaggerate The z Axis default value 1.0  */
 int (*loadFunctionP)(DTMFeatureType, long, long, DPoint3d*, DPoint3d*, long, long*, void*),   /* ==> Pointer To Load Function                           */
 long useFence,            /* ==> Load Feature Within Fence <TRUE,FALSE>             */
 DTMFenceType fenceType,           /* ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape>       */
 DTMFenceOption fenceOption,         /* ==> Fence Option <INSIDE(1),OVERLAP(2)>                */
 DPoint3d  *fencePtsP,          /* ==> DPoint3d Array Of Fence Points                          */
 long numFencePts,         /* ==> Number Of Fence Points                             */
 void *userP               /* ==> User Pointer Passed Back To User                   */
)
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long  pnt1, pnt2, pnt3, clPtr, numTriangles;
 bool  voidTriangle, voidsInDtm;
 long  node,startPnt,lastPnt,numTrianglesLoaded,startTime ;
 long  fndType,tinPnt1,tinPnt2,tinPnt3,numMeshPts,minTptrPnt,maxTptrPnt ;
 long  *faceP,*meshFacesP=NULL ;
 DPoint3d   *p3dP,*vectP,*meshPtsP=NULL,normalVector,*meshVectorsP=NULL ;
 long meshVectorsSize = 0, meshPtsSize = 0;
 double dz ;
 BC_DTM_OBJ    *clipDtmP=NULL ;
 DTM_CIR_LIST  *clistP ;
 DPoint3d *pntP ;
 DTM_TIN_NODE  *nodeP,*node1P,*node2P,*node3P ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    startTime = bcdtmClock() ;
    bcdtmWrite_message(0,0,0,"Interrupt Loading Triangle Shade Mesh") ;
    bcdtmWrite_message(0,0,0,"Dtm Object     = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"maxTriangles   = %8ld",maxTriangles) ;
    bcdtmWrite_message(0,0,0,"vectorOption   = %8ld",vectorOption) ;
    bcdtmWrite_message(0,0,0,"zAxisFactor    = %8ld",zAxisFactor) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP  = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"useFence       = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fenceType      = %8ld",fenceType) ;
    bcdtmWrite_message(0,0,0,"fenceOption    = %8ld",fenceOption) ;
    bcdtmWrite_message(0,0,0,"fencePtsP      = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts    = %8ld",numFencePts) ;
    bcdtmWrite_message(0,0,0,"userP          = %p",userP) ;
   }
/*
** Validate Fence
*/
 if( useFence == TRUE && ( fencePtsP == NULL || numFencePts <= 2 ) ) useFence = FALSE ;
 if( useFence == TRUE && ( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )) useFence = FALSE ;
 if( useFence )
   {
    if( fenceType   != DTMFenceType::Block && fenceType   != DTMFenceType::Shape ) fenceType = DTMFenceType::Block ;
    if( fenceOption != DTMFenceOption::Inside      && fenceOption != DTMFenceOption::Outside && fenceOption != DTMFenceOption::Overlap ) fenceOption = DTMFenceOption::Overlap ;
   }
/*
** Validate Normal Vector Option
*/
 if( vectorOption < 1 || vectorOption > 2 ) vectorOption = 2 ;
/*
** Validate z Axis Factor
*/
 if( zAxisFactor <= 0.0 ) zAxisFactor = 1.0 ;
/*
** Test For Valid Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Valid Dtm Object") ;
 if( bcdtmObject_testForValidDtmObject(dtmP) ) goto errexit ;
/*
**  Check If DTM Is In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Validate Mesh Size
*/
 if( maxTriangles <= 0 ) maxTriangles = 50000 ;
 if( maxTriangles > dtmP->numTriangles ) maxTriangles = dtmP->numTriangles ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Reset maxTriangles To = %8ld",maxTriangles) ;
/*
** Check For Voids In The Triangulation
*/
 voidsInDtm = false;
 bcdtmList_testForVoidsInDtmObject(dtmP,voidsInDtm) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"voidsInDtm = %2ld",voidsInDtm) ;
/*
**  Build Clipping Dtm For Fence
*/
 if( useFence == TRUE )
   {
    if( bcdtmClip_buildClippingTinFromFencePointsDtmObject(&clipDtmP,fencePtsP,numFencePts)) goto errexit ;
    if( fabs( dtmP->xMin - clipDtmP->xMin ) < dtmP->ppTol ) clipDtmP->xMin = dtmP->xMin ;
    if( fabs( dtmP->xMax - clipDtmP->xMax ) < dtmP->ppTol ) clipDtmP->xMax = dtmP->xMax ;
    if( fabs( dtmP->yMin - clipDtmP->yMin ) < dtmP->ppTol ) clipDtmP->yMin = dtmP->yMin ;
    if( fabs( dtmP->yMax - clipDtmP->yMax ) < dtmP->ppTol ) clipDtmP->yMax = dtmP->yMax ;
    if( fenceType == DTMFenceType::Block && dtmP->xMin >= clipDtmP->xMin && dtmP->xMax <= clipDtmP->xMax &&  dtmP->yMin >= clipDtmP->yMin && dtmP->yMax <= clipDtmP->yMax ) useFence = FALSE ;
    if( dbg ) bcdtmWrite_message(0,0,0,"useFence = %2ld ** fenceType = %2ld",useFence,fenceType) ;
  }
/*
** Initialise
*/
 startPnt = 0 ;
 lastPnt  = dtmP->numPoints - 1 ;
 numTriangles = 0 ;
 numTrianglesLoaded = 0 ;
/*
** Write Start Scan Point And Last Scan Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"startPnt = %8ld lastPnt = %8ld",startPnt,lastPnt) ;
/*
**  Find Points Immediately Before And After Fence
*/
 if( useFence == TRUE )
   {
    startTime = bcdtmClock() ;
    bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMin,&startPnt) ;
    while( startPnt > 0 && pointAddrP(dtmP,startPnt)->x >= clipDtmP->xMin ) --startPnt ;
    if( pointAddrP(dtmP,startPnt)->x < clipDtmP->xMin ) ++startPnt ;
    bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMax,&lastPnt) ;
    while( lastPnt < dtmP->numPoints - 1  && pointAddrP(dtmP,lastPnt)->x <= clipDtmP->xMax ) ++lastPnt ;
    if( tdbg )
      {
       bcdtmWrite_message(0,0,0,"startPnt = %8ld startPnt->x = %12.5lf",startPnt,pointAddrP(dtmP,startPnt)->x) ;
       bcdtmWrite_message(0,0,0,"lastPnt  = %8ld lastPnt->x  = %12.5lf",lastPnt,pointAddrP(dtmP,lastPnt)->x) ;
       bcdtmWrite_message(0,0,0,"clipDtmP->xMin  = %12.5lf clipDtmP->xMax  = %12.5lf",clipDtmP->xMin,clipDtmP->xMax) ;
       bcdtmWrite_message(0,0,0,"lastPnt  = %8ld lastPnt->x  = %12.5lf",lastPnt,pointAddrP(dtmP,lastPnt)->x) ;
       bcdtmWrite_message(0,0,0,"** Index Time 00 = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
      }
/*
**  Mark Tin Points Within Block Fence
*/
    if( fenceType == DTMFenceType::Block )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Within Fence Block") ;
       for( pnt1 = startPnt ; pnt1 <= lastPnt ; ++pnt1 )
         {
          pntP = pointAddrP(dtmP,pnt1) ;
          if( pntP->x >= clipDtmP->xMin && pntP->x <= clipDtmP->xMax &&
              pntP->y >= clipDtmP->yMin && pntP->y <= clipDtmP->yMax     ) nodeAddrP(dtmP,pnt1)->sPtr = 1 ;
         }
      }
/*
**  Mark Tin Points Within Shape Fence
*/
    if( fenceType == DTMFenceType::Shape )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Within Fence Shape") ;
       for( pnt1 = startPnt ; pnt1 <= lastPnt ; ++pnt1 )
         {
          pntP = pointAddrP(dtmP,pnt1) ;
          if( pntP->x >= clipDtmP->xMin && pntP->x <= clipDtmP->xMax &&
              pntP->y >= clipDtmP->yMin && pntP->y <= clipDtmP->yMax     )
            {
             if( bcdtmFind_triangleDtmObject(dtmP,pntP->x,pntP->y,&fndType,&tinPnt1,&tinPnt2,&tinPnt3)) goto errexit ;
             if( fndType ) nodeAddrP(dtmP,pnt1)->sPtr = 1 ;
            }
         }
      }
/*
**  Write Start Scan Point And Last Scan Point
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"startPnt = %8ld lastPnt = %8ld",startPnt,lastPnt) ;
   }
/*
** Allocate Memory For Mesh Faces
*/
 meshFacesP   = (long * ) malloc( maxTriangles * 3 * sizeof(long)) ;
 if( meshFacesP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
 faceP = meshFacesP ;
/*
** Scan DTM And Accumulate Triangle Mesh
*/
 minTptrPnt = dtmP->numPoints ;
 maxTptrPnt = -1 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning DTM Triangles") ;
 for( pnt1 = startPnt ; pnt1 <= lastPnt ; ++pnt1 )
   {
    node1P = nodeAddrP(dtmP,pnt1) ;
    if( useFence == FALSE || ( useFence == TRUE && node1P->sPtr == 1 ))
      {
       if( ( clPtr = node1P->cPtr ) != dtmP->nullPtr )
         {
          if( ( pnt2 = bcdtmList_nextAntDtmObject(dtmP,pnt1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
          node2P = nodeAddrP(dtmP,pnt2) ;
          while( clPtr != dtmP->nullPtr )
            {
             clistP = clistAddrP(dtmP,clPtr) ;
             pnt3   = clistP->pntNum ;
             clPtr  = clistP->nextPtr ;
             node3P = nodeAddrP(dtmP,pnt3) ;
             if( node1P->hPtr != pnt2 )
               {
                if( pnt2 > pnt1 && pnt3 > pnt1  || ( useFence == TRUE && (  node2P->sPtr != 1 || pnt2 > pnt1) && (node3P->sPtr != 1 || pnt3 > pnt1) ) )
                  {
/*
**                 Test For Void Triangle
*/
                   voidTriangle = false ;

                   if(voidsInDtm == true ) if( bcdtmList_testForVoidTriangleDtmObject(dtmP,pnt1,pnt2,pnt3,voidTriangle)) goto errexit ;
/*
**                 Process If None Void Triangle
*/
                   if( voidTriangle == false )
                     {
                      *faceP = pnt3  ; ++faceP ;
                      *faceP = pnt2  ; ++faceP ;
                      *faceP = pnt1  ; ++faceP ;
                      ++numTriangles ;
/*
**                    Check For Maximum Load Triangles
*/
                      if( numTriangles == maxTriangles )
                        {
                         if( dbg ) bcdtmWrite_message(0,0,0,"Creating Triangle Mesh For Max Traingles ** numTriangles = %8ld",numTriangles) ;
/*
**                       Mark Mesh Points
*/
                         minTptrPnt = dtmP->numPoints ;
                         maxTptrPnt = -1 ;
                         for( faceP = meshFacesP ; faceP < meshFacesP + numTriangles * 3 ; ++faceP )
                           {
                            if( *faceP < minTptrPnt ) minTptrPnt = *faceP ;
                            if( *faceP > maxTptrPnt ) maxTptrPnt = *faceP ;
                            nodeAddrP(dtmP,*faceP)->tPtr = 1 ;
                           }
                         if( dbg ) bcdtmWrite_message(0,0,0,"minTptrPoint = %8ld maxTptrPoint = %8ld",minTptrPnt,maxTptrPnt) ;
/*
**                       Count Number Of Mesh Points
*/
                         numMeshPts = 0 ;
                         for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
                           {
                            nodeP = nodeAddrP(dtmP,node) ;
                            if( nodeP->tPtr == 1 )
                              {
                               ++numMeshPts ;
                               nodeP->tPtr = numMeshPts ;
                              }
                           }
/*
**                       Allocate Memory For Mesh Points
*/
                         if( dbg ) bcdtmWrite_message(0,0,0,"Populating Mesh Points ** numMeshPts = %8ld",numMeshPts) ;
                         if(!meshPtsP || numMeshPts > meshPtsSize )
                             {
                             if(meshPtsP) free(meshPtsP);
                             meshPtsP = ( DPoint3d * ) malloc( numMeshPts * sizeof(DPoint3d) ) ;
                             if( meshPtsP == NULL )
                               {
                                bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                                goto errexit ;
                               }
                             meshPtsSize = numMeshPts;
                             }
/*
**                       Allocate Memory For Mesh Vectors
*/
                         if(!meshVectorsP || numMeshPts > meshVectorsSize)
                             {
                             if(meshVectorsP) free(meshVectorsP);
                             meshVectorsP = ( DPoint3d * ) malloc( numMeshPts * sizeof(DPoint3d) ) ;
                             if( meshVectorsP == NULL )
                               {
                                bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                                goto errexit ;
                               }
                             meshVectorsSize = numMeshPts;
                             }
/*
**                      Exaggerate z Axis
*/
                        if( zAxisFactor != 1.0 )
                          {
                           if( dbg ) bcdtmWrite_message(0,0,0,"Exaggerating z Axis") ;
                           for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
                             {
                              nodeP = nodeAddrP(dtmP,node) ;
                              if( nodeP->tPtr > 0 && nodeP->tPtr < dtmP->nullPnt  )
                                {
                                 pntP    = pointAddrP(dtmP,node) ;
                                 dz = ( pntP->z - dtmP->zMin ) * zAxisFactor ;
                                 pntP->z = dtmP->zMin + dz ;
                                }
                             }
                          }

/*
**                       Populate Mesh Points Array And Mesh Vectors Array
*/
                         p3dP  = meshPtsP ;
                         vectP = meshVectorsP ;
                         for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
                           {
                            nodeP = nodeAddrP(dtmP,node) ;
                            if( nodeP->tPtr > 0 && nodeP->tPtr < dtmP->nullPnt  )
                              {
                               pntP    = pointAddrP(dtmP,node) ;
                               p3dP->x = pntP->x ;
                               p3dP->y = pntP->y ;
                               p3dP->z = pntP->z ;
                               ++p3dP ;
                               if( bcdtmLoad_calculateNormalVectorForTriangleVertexDtmObject(dtmP,node,2,zAxisFactor,&normalVector)) goto errexit ;
                               *vectP  = normalVector ;
                               ++vectP ;
                              }
                           }
/*
**                      De Exaggerate z Axis
*/
                        if( zAxisFactor != 1.0 )
                          {
                           if( dbg ) bcdtmWrite_message(0,0,0,"DE-Exaggerating z Axis") ;
                           for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
                             {
                              nodeP = nodeAddrP(dtmP,node) ;
                              if( nodeP->tPtr > 0 && nodeP->tPtr < dtmP->nullPnt  )
                                {
                                 pntP    = pointAddrP(dtmP,node) ;
                                 dz = ( pntP->z - dtmP->zMin ) / zAxisFactor ;
                                 pntP->z = dtmP->zMin + dz ;
                                }
                             }
                          }
/*
**                       Reset Point Indexes In Mesh Faces
*/
                         for( faceP = meshFacesP ; faceP < meshFacesP + numTriangles * 3 ; ++faceP )
                           {
                            *faceP = nodeAddrP(dtmP,*faceP)->tPtr ;
                           }
/*
**                       Null Tptr Values
*/
                         for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
                           {
                            nodeAddrP(dtmP,node)->tPtr = dtmP->nullPnt ;
                           }
/*
**                       Call Browse Function
*/
                         if( loadFunctionP != NULL )if( loadFunctionP(DTMFeatureType::TriangleShadeMesh,numTriangles,numMeshPts,meshPtsP,meshVectorsP,numTriangles*3,meshFacesP,userP)) goto errexit ;
                         faceP = meshFacesP ;
                         //if( meshPtsP     != NULL ) { free(meshPtsP)     ; meshPtsP       = NULL ; }
                         //if( meshVectorsP != NULL ) { free(meshVectorsP) ; meshVectorsP   = NULL ; }
                         numTrianglesLoaded = numTrianglesLoaded + numTriangles ;
                         numTriangles = 0 ;
                         minTptrPnt = dtmP->numPoints ;
                         maxTptrPnt = -1 ;
                        }
                     }
                  }
               }
/*
**           Continue Scan For Triangles
*/
             pnt2 = pnt3 ;
             node2P = node3P ;
            }
         }
      }
   }
/*
** Check For Unloaded Triangles
*/
 if( numTriangles > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Creating Triangle Mesh For Unloaded Triangles ** numTriangles = %8ld",numTriangles) ;
/*
**  Mark Mesh Points
*/
    minTptrPnt = dtmP->numPoints ;
    maxTptrPnt = -1 ;
    for( faceP = meshFacesP ; faceP < meshFacesP + numTriangles * 3 ; ++faceP )
      {
       if( *faceP < minTptrPnt ) minTptrPnt = *faceP ;
       if( *faceP > maxTptrPnt ) maxTptrPnt = *faceP ;
       nodeAddrP(dtmP,*faceP)->tPtr = 1 ;
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"minTptrPoint = %8ld maxTptrPoint = %8ld",minTptrPnt,maxTptrPnt) ;
/*
**  Count Number Of Mesh Points
*/
    numMeshPts = 0 ;
    for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
      {
       nodeP = nodeAddrP(dtmP,node) ;
       if( nodeP->tPtr == 1 )
         {
          ++numMeshPts ;
          nodeP->tPtr = numMeshPts ;
         }
      }
/*
**  Allocate Memory For Mesh Points
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Populating Mesh Points ** numMeshPts = %8ld",numMeshPts) ;

    if(!meshPtsP || numMeshPts > meshPtsSize)
        {
        if(meshPtsP) free(meshPtsP);
        meshPtsP = ( DPoint3d * ) malloc( numMeshPts * sizeof(DPoint3d) ) ;
        if( meshPtsP == NULL )
          {
           bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
           goto errexit ;
          }
        meshPtsSize = numMeshPts;
        }
/*
**  Allocate Memory For Mesh Vectors
*/
    if(!meshVectorsP || numMeshPts > meshVectorsSize)
        {
        if(meshVectorsP) free(meshVectorsP);
        meshVectorsP = ( DPoint3d * ) malloc( numMeshPts * sizeof(DPoint3d) ) ;
        if( meshVectorsP == NULL )
          {
           bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
           goto errexit ;
          }
        meshVectorsSize = numMeshPts;
        }
/*
**   Exaggerate z Axis
*/
    if( zAxisFactor != 1.0 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Exaggerating z Axis") ;
       for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
         {
          nodeP = nodeAddrP(dtmP,node) ;
          if( nodeP->tPtr > 0 && nodeP->tPtr < dtmP->nullPnt )
            {
             pntP    = pointAddrP(dtmP,node) ;
             dz = ( pntP->z - dtmP->zMin ) * zAxisFactor ;
             pntP->z = dtmP->zMin + dz ;
            }
         }
     }
/*
**  Populate Mesh Points and Mesh Vector Arrayies
*/
    p3dP = meshPtsP ;
    vectP = meshVectorsP ;
    for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
      {
       nodeP = nodeAddrP(dtmP,node) ;
       if( nodeP->tPtr > 0 && nodeP->tPtr < dtmP->nullPnt )
         {
          pntP = pointAddrP(dtmP,node) ;
          p3dP->x = pntP->x ;
          p3dP->y = pntP->y ;
          p3dP->z = pntP->z ;
          ++p3dP ;
          if( bcdtmLoad_calculateNormalVectorForTriangleVertexDtmObject(dtmP,node,2,zAxisFactor,&normalVector)) goto errexit ;
          *vectP  = normalVector ;
          ++vectP ;
         }
      }
/*
**  Reset Point Indexes In Mesh Faces
*/
    for( faceP = meshFacesP ; faceP < meshFacesP + numTriangles * 3 ; ++faceP )
      {
       *faceP = nodeAddrP(dtmP,*faceP)->tPtr ;
      }
/*
**  De Exaggerate z Axis
*/
    if( zAxisFactor != 1.0 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"De-Exaggerating z Axis") ;
       for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
         {
          nodeP = nodeAddrP(dtmP,node) ;
          if( nodeP->tPtr > 0 && nodeP->tPtr < dtmP->nullPnt )
            {
             pntP    = pointAddrP(dtmP,node) ;
             dz = ( pntP->z - dtmP->zMin ) / zAxisFactor ;
             pntP->z = dtmP->zMin + dz ;
            }
         }
      }
/*
**  Null Tptr Values
*/
    for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
      {
       nodeAddrP(dtmP,node)->tPtr = dtmP->nullPnt ;
      }
/*
**  Call Browse Function
*/
    if( loadFunctionP != NULL ) if( loadFunctionP(DTMFeatureType::TriangleShadeMesh,numTriangles,numMeshPts,meshPtsP,meshVectorsP,numTriangles*3,meshFacesP, userP)) goto errexit ;
    faceP = meshFacesP ;
    if( meshPtsP     != NULL ) { free(meshPtsP)     ; meshPtsP       = NULL ; }
    if( meshVectorsP != NULL ) { free(meshVectorsP) ; meshVectorsP   = NULL ; }
    numTrianglesLoaded = numTrianglesLoaded + numTriangles ;
    numTriangles = 0 ;
    minTptrPnt = dtmP->numPoints ;
    maxTptrPnt = -1 ;
   }
/*
** Clean Up
*/
 cleanup :
 if( meshPtsP       != NULL ) { free(meshPtsP)       ; meshPtsP       = NULL ; }
 if( meshFacesP     != NULL ) { free(meshFacesP)     ; meshFacesP     = NULL ; }
 if( meshVectorsP   != NULL ) { free(meshVectorsP)   ; meshVectorsP   = NULL ; }
 if( clipDtmP       != NULL ) bcdtmObject_destroyDtmObject(&clipDtmP) ;
 if( useFence && dtmP->dtmState == DTMState::Tin ) for( node = startPnt ; node <= lastPnt ; ++node ) nodeAddrP(dtmP,node)->sPtr = dtmP->nullPnt ;
/*
** Job Completed
*/
 if( dbg  ) bcdtmWrite_message(0,0,0,"Number Of Triangles Loaded  = %10ld ** dtmP->numTriangles = %10ld",numTrianglesLoaded,dtmP->numTriangles) ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Load Triangle Mesh  = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime) ) ;
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Interrupt Loading Triangle Shade Mesh Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Interrupt Loading Triangle Shade Mesh Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( dtmP->dtmState == DTMState::Tin )
   {
    bcdtmList_nullTptrValuesDtmObject(dtmP) ;
    bcdtmList_nullSptrValuesDtmObject(dtmP) ;
   }
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR  ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmInterruptLoad_triangleShadeMeshFromDtmObject
(
 BC_DTM_OBJ *dtmP,         /* ==> Pointer To Dtm Object                              */
 long maxTriangles,        /* ==> Maximum Number Of Triangles To Load Per Call       */
 long vectorOption,        /* ==> Vector Option <1=Surface Derivatives,2=Averaged Triangle Surface Normals> */
 double zAxisFactor,       /* ==> Factor To Exaggerate The z Axis default value 1.0  */
 DTMTriangleShadeMeshCallback loadFunctionP,   /* ==> Pointer To Load Function                           */
 long useFence,            /* ==> Load Feature Within Fence <TRUE,FALSE>             */
 DTMFenceType fenceType,           /* ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape>       */
 DTMFenceOption fenceOption,         /* ==> Fence Option <INSIDE(1),OVERLAP(2)>                */
 DPoint3d  *fencePtsP,          /* ==> DPoint3d Array Of Fence Points                          */
 long numFencePts,         /* ==> Number Of Fence Points                             */
 void *userP               /* ==> User Pointer Passed Back To User                   */
)
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long  pnt1, pnt2, pnt3, clPtr, numTriangles;
 bool  voidTriangle, voidsInDtm;
 long  node,startPnt,lastPnt,numTrianglesLoaded,startTime ;
 long  fndType,tinPnt1,tinPnt2,tinPnt3,numMeshPts,minTptrPnt,maxTptrPnt ;
 long  *faceP,*meshFacesP=NULL ;
 long  pointMark,pointMarkOffset=0,leftMostPnt,numMarked=0 ;
 DPoint3d   *p3dP,*vectP,*meshPtsP=NULL,normalVector,*meshVectorsP=NULL ;
 long meshVectorsSize = 0, meshPtsSize = 0;
 double dz ;
 BC_DTM_OBJ    *clipDtmP=NULL ;
 DTM_CIR_LIST  *clistP ;
 DPoint3d *pntP ;
 DTM_TIN_NODE  *nodeP,*node1P,*node2P,*node3P ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    startTime = bcdtmClock() ;
    bcdtmWrite_message(0,0,0,"Interrupt Loading Triangle Shade Mesh") ;
    bcdtmWrite_message(0,0,0,"Dtm Object     = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"maxTriangles   = %8ld",maxTriangles) ;
    bcdtmWrite_message(0,0,0,"vectorOption   = %8ld",vectorOption) ;
    bcdtmWrite_message(0,0,0,"zAxisFactor    = %8ld",zAxisFactor) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP  = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"useFence       = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fenceType      = %8ld",fenceType) ;
    bcdtmWrite_message(0,0,0,"fenceOption    = %8ld",fenceOption) ;
    bcdtmWrite_message(0,0,0,"fencePtsP      = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts    = %8ld",numFencePts) ;
    bcdtmWrite_message(0,0,0,"userP          = %p",userP) ;
    if( fencePtsP != NULL && numFencePts > 0 )
      {
       for( p3dP = fencePtsP ; p3dP < fencePtsP + numFencePts ; ++p3dP )
         {
          bcdtmWrite_message(0,0,0,"Fence Point[%8ld] = %12.5lf %12.5lf %10.4lf",(long)(p3dP-fencePtsP),p3dP->x,p3dP->y,p3dP->z) ;
         }
      }
   }
/*
** Validate Fence
*/
 if( useFence == TRUE && ( fencePtsP == NULL || numFencePts <= 2 ) ) useFence = FALSE ;
 if( useFence == TRUE && ( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )) useFence = FALSE ;
 if( useFence )
   {
    if( fenceType   != DTMFenceType::Block && fenceType   != DTMFenceType::Shape ) fenceType = DTMFenceType::Block ;
    if( fenceOption != DTMFenceOption::Inside      && fenceOption != DTMFenceOption::Outside && fenceOption != DTMFenceOption::Overlap ) fenceOption = DTMFenceOption::Overlap ;
   }
 if( dbg ) bcdtmWrite_message(0,0,0,"useFence = %2ld",useFence) ;
/*
** Validate Normal Vector Option
*/
 if( vectorOption < 1 || vectorOption > 2 ) vectorOption = 2 ;
/*
** Validate z Axis Factor
*/
 if( zAxisFactor <= 0.0 ) zAxisFactor = 1.0 ;
/*
** Test For Valid Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Valid Dtm Object") ;
 if( bcdtmObject_testForValidDtmObject(dtmP) ) goto errexit ;
/*
**  Check If DTM Is In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Validate Mesh Size
*/
 if( maxTriangles <= 0 ) maxTriangles = 50000 ;
 if( maxTriangles > dtmP->numTriangles ) maxTriangles = dtmP->numTriangles ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Reset maxTriangles To = %8ld",maxTriangles) ;
/*
** Check For Voids In The Triangulation
*/
 voidsInDtm = false ;
 bcdtmList_testForVoidsInDtmObject(dtmP,voidsInDtm) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"voidsInDtm = %2ld",voidsInDtm) ;
/*
**  Build Clipping Dtm For Fence
*/
 if( useFence == TRUE )
   {
    if( bcdtmClip_buildClippingTinFromFencePointsDtmObject(&clipDtmP,fencePtsP,numFencePts)) goto errexit ;
    if( fabs( dtmP->xMin - clipDtmP->xMin ) < dtmP->ppTol ) clipDtmP->xMin = dtmP->xMin ;
    if( fabs( dtmP->xMax - clipDtmP->xMax ) < dtmP->ppTol ) clipDtmP->xMax = dtmP->xMax ;
    if( fabs( dtmP->yMin - clipDtmP->yMin ) < dtmP->ppTol ) clipDtmP->yMin = dtmP->yMin ;
    if( fabs( dtmP->yMax - clipDtmP->yMax ) < dtmP->ppTol ) clipDtmP->yMax = dtmP->yMax ;
    if( fenceType == DTMFenceType::Block && dtmP->xMin >= clipDtmP->xMin && dtmP->xMax <= clipDtmP->xMax &&  dtmP->yMin >= clipDtmP->yMin && dtmP->yMax <= clipDtmP->yMax ) useFence = FALSE ;
    if( dbg ) bcdtmWrite_message(0,0,0,"useFence = %2ld ** fenceType = %2ld",useFence,fenceType) ;
  }
/*
** Initialise
*/
 leftMostPnt = startPnt = 0 ;
 lastPnt  = dtmP->numPoints - 1 ;
 numTriangles = 0 ;
 numTrianglesLoaded = 0 ;
/*
** Write Start Scan Point And Last Scan Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"startPnt = %8ld lastPnt = %8ld",startPnt,lastPnt) ;
/*
**  Find Points Immediately Before And After Fence
*/
 if( useFence == TRUE )
   {

//  Mark Points Immediately External To Fence

    pointMark = dtmP->numPoints * 2 + pointMarkOffset;
    startTime = bcdtmClock() ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points External To Fence") ;
    if( bcdtmLoad_markTinPointsExternalToFenceDtmObject(dtmP,clipDtmP,pointMark,&leftMostPnt,&numMarked)) goto errexit ;
    if( tdbg )
      {
       bcdtmWrite_message(0,0,0,"leftMostPnt = %8ld leftMostPnt->x = %12.5lf",leftMostPnt,pointAddrP(dtmP,leftMostPnt)->x) ;
       bcdtmWrite_message(0,0,0,"** Time To Mark %6ld Tin Points External To Fence = %8.3lf Seconds",numMarked,bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
      }
//  if( numMarked == 0 ) useFence = FALSE ;

//  Mark Points Internal To Fence

    startTime = bcdtmClock() ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Internal To Fence") ;
    bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMin,&startPnt) ;
    while( startPnt > 0 && pointAddrP(dtmP,startPnt)->x >= clipDtmP->xMin ) --startPnt ;
    if( pointAddrP(dtmP,startPnt)->x < clipDtmP->xMin ) ++startPnt ;
    bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMax,&lastPnt) ;
    while( lastPnt < dtmP->numPoints - 1  && pointAddrP(dtmP,lastPnt)->x <= clipDtmP->xMax ) ++lastPnt ;
    if( tdbg )
      {
       bcdtmWrite_message(0,0,0,"startPnt = %8ld startPnt->x = %12.5lf",startPnt,pointAddrP(dtmP,startPnt)->x) ;
       bcdtmWrite_message(0,0,0,"lastPnt  = %8ld lastPnt->x  = %12.5lf",lastPnt,pointAddrP(dtmP,lastPnt)->x) ;
       bcdtmWrite_message(0,0,0,"clipDtmP->xMin  = %12.5lf clipDtmP->xMax  = %12.5lf",clipDtmP->xMin,clipDtmP->xMax) ;
       bcdtmWrite_message(0,0,0,"lastPnt  = %8ld lastPnt->x  = %12.5lf",lastPnt,pointAddrP(dtmP,lastPnt)->x) ;
       bcdtmWrite_message(0,0,0,"** Index Time 00 = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
      }
/*
**  Mark Tin Points Within Block Fence
*/
    if( fenceType == DTMFenceType::Block )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Within Fence Block") ;
       for( pnt1 = startPnt ; pnt1 <= lastPnt ; ++pnt1 )
         {
          pntP = pointAddrP(dtmP,pnt1) ;
          if( pntP->x >= clipDtmP->xMin && pntP->x <= clipDtmP->xMax &&
              pntP->y >= clipDtmP->yMin && pntP->y <= clipDtmP->yMax     ) nodeAddrP(dtmP,pnt1)->sPtr = pointMark ;
         }
      }
/*
**  Mark Tin Points Within Shape Fence
*/
    if( fenceType == DTMFenceType::Shape )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Within Fence Shape") ;
       for( pnt1 = startPnt ; pnt1 <= lastPnt ; ++pnt1 )
         {
          pntP = pointAddrP(dtmP,pnt1) ;
          if( pntP->x >= clipDtmP->xMin && pntP->x <= clipDtmP->xMax &&
              pntP->y >= clipDtmP->yMin && pntP->y <= clipDtmP->yMax     )
            {
             if( bcdtmFind_triangleDtmObject(dtmP,pntP->x,pntP->y,&fndType,&tinPnt1,&tinPnt2,&tinPnt3)) goto errexit ;
             if( fndType ) nodeAddrP(dtmP,pnt1)->sPtr = pointMark ;
            }
         }
      }
/*
**  Write Start Scan Point And Last Scan Point
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"startPnt = %8ld lastPnt = %8ld",startPnt,lastPnt) ;
   }
/*
** Allocate Memory For Mesh Faces
*/
 meshFacesP   = (long * ) malloc( maxTriangles * 3 * sizeof(long)) ;
 if( meshFacesP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
 faceP = meshFacesP ;
/*
** Log Marked Points
*/
 if( dbg )
   {
    for( pnt1 = 0 ; pnt1 < dtmP->numPoints ; ++pnt1 )
      {
       if( nodeAddrP(dtmP,pnt1)->sPtr == pointMark )
         {
          bcdtmWrite_message(0,0,0,"Marked Point[%8ld] ** %12.5lf %12.5lf %10.4lf",pnt1,pointAddrP(dtmP,pnt1)->x,pointAddrP(dtmP,pnt1)->y,pointAddrP(dtmP,pnt1)->z) ;
         }
      }
   }
/*
** Scan DTM And Accumulate Triangle Mesh
*/
 minTptrPnt = dtmP->numPoints ;
 maxTptrPnt = -1 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning DTM Triangles") ;
// for( pnt1 = startPnt ; pnt1 <= lastPnt ; ++pnt1 )
 for( pnt1 = leftMostPnt ; pnt1 <= lastPnt ; ++pnt1 )
   {
    node1P = nodeAddrP(dtmP,pnt1) ;
    if( useFence == FALSE || ( useFence == TRUE && node1P->sPtr == pointMark ))
      {
//bcdtmWrite_message(0,0,0,"Loading Triangles For Point %8ld",pnt1) ;
       if( ( clPtr = node1P->cPtr ) != dtmP->nullPtr )
         {
          if( ( pnt2 = bcdtmList_nextAntDtmObject(dtmP,pnt1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
          node2P = nodeAddrP(dtmP,pnt2) ;
          while( clPtr != dtmP->nullPtr )
            {
             clistP = clistAddrP(dtmP,clPtr) ;
             pnt3   = clistP->pntNum ;
             clPtr  = clistP->nextPtr ;
             node3P = nodeAddrP(dtmP,pnt3) ;
             if( node1P->hPtr != pnt2 )
               {
//                if( pnt2 > pnt1 && pnt3 > pnt1  || ( useFence == TRUE && (  node2P->sPtr != pointMark || pnt2 > pnt1) && (node3P->sPtr != pointMark || pnt3 > pnt1) ) )
                if( ( useFence == FALSE && pnt2 > pnt1 && pnt3 > pnt1 ) || ( useFence == TRUE &&  node2P->sPtr == pointMark && pnt2 > pnt1 && node3P->sPtr == pointMark && pnt3 > pnt1 ) )
                  {
// bcdtmWrite_message(0,0,0,"Triangle %8ld %8ld %8ld",pnt1,pnt2,pnt3) ;
/*
**                 Test For Void Triangle
*/
                   voidTriangle = false ;
                   if(voidsInDtm == true ) if( bcdtmList_testForVoidTriangleDtmObject(dtmP,pnt1,pnt2,pnt3,voidTriangle)) goto errexit ;
/*
**                 Process If None Void Triangle
*/
                   if( voidTriangle == false)
                     {
                      *faceP = pnt3  ; ++faceP ;
                      *faceP = pnt2  ; ++faceP ;
                      *faceP = pnt1  ; ++faceP ;
                      ++numTriangles ;
/*
**                    Check For Maximum Load Triangles
*/
                      if( numTriangles == maxTriangles )
                        {
                         if( dbg ) bcdtmWrite_message(0,0,0,"Creating Triangle Mesh For Max Traingles ** numTriangles = %8ld",numTriangles) ;
/*
**                       Mark Mesh Points
*/
                         minTptrPnt = dtmP->numPoints ;
                         maxTptrPnt = -1 ;
                         for( faceP = meshFacesP ; faceP < meshFacesP + numTriangles * 3 ; ++faceP )
                           {
                            if( *faceP < minTptrPnt ) minTptrPnt = *faceP ;
                            if( *faceP > maxTptrPnt ) maxTptrPnt = *faceP ;
                            nodeAddrP(dtmP,*faceP)->tPtr = 1 ;
                           }
                         if( dbg ) bcdtmWrite_message(0,0,0,"minTptrPoint = %8ld maxTptrPoint = %8ld",minTptrPnt,maxTptrPnt) ;
/*
**                       Count Number Of Mesh Points
*/
                         numMeshPts = 0 ;
                         for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
                           {
                            nodeP = nodeAddrP(dtmP,node) ;
                            if( nodeP->tPtr == 1 )
                              {
                               ++numMeshPts ;
                               nodeP->tPtr = numMeshPts ;
                              }
                           }
/*
**                       Allocate Memory For Mesh Points
*/
                         if( dbg ) bcdtmWrite_message(0,0,0,"Populating Mesh Points ** numMeshPts = %8ld",numMeshPts) ;
                         if(!meshPtsP || numMeshPts > meshPtsSize )
                             {
                             if(meshPtsP) free(meshPtsP);
                             meshPtsP = ( DPoint3d * ) malloc( numMeshPts * sizeof(DPoint3d) ) ;
                             if( meshPtsP == NULL )
                               {
                                bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                                goto errexit ;
                               }
                             meshPtsSize = numMeshPts;
                             }
/*
**                       Allocate Memory For Mesh Vectors
*/
                         if(!meshVectorsP || numMeshPts > meshVectorsSize)
                             {
                             if(meshVectorsP) free(meshVectorsP);
                             meshVectorsP = ( DPoint3d * ) malloc( numMeshPts * sizeof(DPoint3d) ) ;
                             if( meshVectorsP == NULL )
                               {
                                bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                                goto errexit ;
                               }
                             meshVectorsSize = numMeshPts;
                             }
/*
**                      Exaggerate z Axis
*/
                        if( zAxisFactor != 1.0 )
                          {
                           if( dbg ) bcdtmWrite_message(0,0,0,"Exaggerating z Axis") ;
                           for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
                             {
                              nodeP = nodeAddrP(dtmP,node) ;
                              if( nodeP->tPtr > 0 && nodeP->tPtr < dtmP->nullPnt  )
                                {
                                 pntP    = pointAddrP(dtmP,node) ;
                                 dz = ( pntP->z - dtmP->zMin ) * zAxisFactor ;
                                 pntP->z = dtmP->zMin + dz ;
                                }
                             }
                          }

/*
**                       Populate Mesh Points Array And Mesh Vectors Array
*/
                         p3dP  = meshPtsP ;
                         vectP = meshVectorsP ;
                         for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
                           {
                            nodeP = nodeAddrP(dtmP,node) ;
                            if( nodeP->tPtr > 0 && nodeP->tPtr < dtmP->nullPnt  )
                              {
                               pntP    = pointAddrP(dtmP,node) ;
                               p3dP->x = pntP->x ;
                               p3dP->y = pntP->y ;
                               p3dP->z = pntP->z ;
                               ++p3dP ;
                               if( bcdtmLoad_calculateNormalVectorForTriangleVertexDtmObject(dtmP,node,2,zAxisFactor,&normalVector)) goto errexit ;
                               *vectP  = normalVector ;
                               ++vectP ;
                              }
                           }
/*
**                      De Exaggerate z Axis
*/
                        if( zAxisFactor != 1.0 )
                          {
                           if( dbg ) bcdtmWrite_message(0,0,0,"DE-Exaggerating z Axis") ;
                           for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
                             {
                              nodeP = nodeAddrP(dtmP,node) ;
                              if( nodeP->tPtr > 0 && nodeP->tPtr < dtmP->nullPnt  )
                                {
                                 pntP    = pointAddrP(dtmP,node) ;
                                 dz = ( pntP->z - dtmP->zMin ) / zAxisFactor ;
                                 pntP->z = dtmP->zMin + dz ;
                                }
                             }
                          }
/*
**                       Reset Point Indexes In Mesh Faces
*/
                         for( faceP = meshFacesP ; faceP < meshFacesP + numTriangles * 3 ; ++faceP )
                           {
                            *faceP = nodeAddrP(dtmP,*faceP)->tPtr ;
                           }
/*
**                       Null Tptr Values
*/
                         for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
                           {
                            nodeAddrP(dtmP,node)->tPtr = dtmP->nullPnt ;
                           }
/*
**                       Call Browse Function
*/
                         if( loadFunctionP != NULL )if( loadFunctionP(DTMFeatureType::TriangleShadeMesh,numTriangles,numMeshPts,meshPtsP,meshVectorsP,numTriangles*3,meshFacesP,userP)) goto errexit ;
                         faceP = meshFacesP ;
                         //if( meshPtsP     != NULL ) { free(meshPtsP)     ; meshPtsP       = NULL ; }
                         //if( meshVectorsP != NULL ) { free(meshVectorsP) ; meshVectorsP   = NULL ; }
                         numTrianglesLoaded = numTrianglesLoaded + numTriangles ;
                         numTriangles = 0 ;
                         minTptrPnt = dtmP->numPoints ;
                         maxTptrPnt = -1 ;
                        }
                     }
                  }
               }
/*
**           Continue Scan For Triangles
*/
             pnt2 = pnt3 ;
             node2P = node3P ;
            }
         }
      }
   }
/*
** Check For Unloaded Triangles
*/
 if( numTriangles > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Creating Triangle Mesh For Unloaded Triangles ** numTriangles = %8ld",numTriangles) ;
/*
**  Mark Mesh Points
*/
    minTptrPnt = dtmP->numPoints ;
    maxTptrPnt = -1 ;
    for( faceP = meshFacesP ; faceP < meshFacesP + numTriangles * 3 ; ++faceP )
      {
       if( *faceP < minTptrPnt ) minTptrPnt = *faceP ;
       if( *faceP > maxTptrPnt ) maxTptrPnt = *faceP ;
       nodeAddrP(dtmP,*faceP)->tPtr = 1 ;
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"minTptrPoint = %8ld maxTptrPoint = %8ld",minTptrPnt,maxTptrPnt) ;
/*
**  Count Number Of Mesh Points
*/
    numMeshPts = 0 ;
    for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
      {
       nodeP = nodeAddrP(dtmP,node) ;
       if( nodeP->tPtr == 1 )
         {
          ++numMeshPts ;
          nodeP->tPtr = numMeshPts ;
         }
      }
/*
**  Allocate Memory For Mesh Points
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Populating Mesh Points ** numMeshPts = %8ld",numMeshPts) ;

    if(!meshPtsP || numMeshPts > meshPtsSize)
        {
        if(meshPtsP) free(meshPtsP);
        meshPtsP = ( DPoint3d * ) malloc( numMeshPts * sizeof(DPoint3d) ) ;
        if( meshPtsP == NULL )
          {
           bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
           goto errexit ;
          }
        meshPtsSize = numMeshPts;
        }
/*
**  Allocate Memory For Mesh Vectors
*/
    if(!meshVectorsP || numMeshPts > meshVectorsSize)
        {
        if(meshVectorsP) free(meshVectorsP);
        meshVectorsP = ( DPoint3d * ) malloc( numMeshPts * sizeof(DPoint3d) ) ;
        if( meshVectorsP == NULL )
          {
           bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
           goto errexit ;
          }
        meshVectorsSize = numMeshPts;
        }
/*
**   Exaggerate z Axis
*/
    if( zAxisFactor != 1.0 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Exaggerating z Axis") ;
       for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
         {
          nodeP = nodeAddrP(dtmP,node) ;
          if( nodeP->tPtr > 0 && nodeP->tPtr < dtmP->nullPnt )
            {
             pntP    = pointAddrP(dtmP,node) ;
             dz = ( pntP->z - dtmP->zMin ) * zAxisFactor ;
             pntP->z = dtmP->zMin + dz ;
            }
         }
     }
/*
**  Populate Mesh Points and Mesh Vector Arrayies
*/
    p3dP = meshPtsP ;
    vectP = meshVectorsP ;
    for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
      {
       nodeP = nodeAddrP(dtmP,node) ;
       if( nodeP->tPtr > 0 && nodeP->tPtr < dtmP->nullPnt )
         {
          pntP = pointAddrP(dtmP,node) ;
          p3dP->x = pntP->x ;
          p3dP->y = pntP->y ;
          p3dP->z = pntP->z ;
          ++p3dP ;
          if( bcdtmLoad_calculateNormalVectorForTriangleVertexDtmObject(dtmP,node,2,zAxisFactor,&normalVector)) goto errexit ;
          *vectP  = normalVector ;
          ++vectP ;
         }
      }
/*
**  Reset Point Indexes In Mesh Faces
*/
    for( faceP = meshFacesP ; faceP < meshFacesP + numTriangles * 3 ; ++faceP )
      {
       *faceP = nodeAddrP(dtmP,*faceP)->tPtr ;
      }
/*
**  De Exaggerate z Axis
*/
    if( zAxisFactor != 1.0 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"De-Exaggerating z Axis") ;
       for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
         {
          nodeP = nodeAddrP(dtmP,node) ;
          if( nodeP->tPtr > 0 && nodeP->tPtr < dtmP->nullPnt )
            {
             pntP    = pointAddrP(dtmP,node) ;
             dz = ( pntP->z - dtmP->zMin ) / zAxisFactor ;
             pntP->z = dtmP->zMin + dz ;
            }
         }
      }
/*
**  Null Tptr Values
*/
    for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
      {
       nodeAddrP(dtmP,node)->tPtr = dtmP->nullPnt ;
      }
/*
**  Call Browse Function
*/
    if( loadFunctionP != NULL ) if( loadFunctionP(DTMFeatureType::TriangleShadeMesh,numTriangles,numMeshPts,meshPtsP,meshVectorsP,numTriangles*3,meshFacesP, userP)) goto errexit ;
    faceP = meshFacesP ;
    if( meshPtsP     != NULL ) { free(meshPtsP)     ; meshPtsP       = NULL ; }
    if( meshVectorsP != NULL ) { free(meshVectorsP) ; meshVectorsP   = NULL ; }
    numTrianglesLoaded = numTrianglesLoaded + numTriangles ;
    numTriangles = 0 ;
    minTptrPnt = dtmP->numPoints ;
    maxTptrPnt = -1 ;
   }
/*
** Clean Up
*/
 cleanup :
 if( meshPtsP       != NULL ) { free(meshPtsP)       ; meshPtsP       = NULL ; }
 if( meshFacesP     != NULL ) { free(meshFacesP)     ; meshFacesP     = NULL ; }
 if( meshVectorsP   != NULL ) { free(meshVectorsP)   ; meshVectorsP   = NULL ; }
 if( clipDtmP       != NULL ) bcdtmObject_destroyDtmObject(&clipDtmP) ;
 if( useFence && dtmP->dtmState == DTMState::Tin ) for( node = startPnt ; node <= lastPnt ; ++node ) nodeAddrP(dtmP,node)->sPtr = dtmP->nullPnt ;
/*
** Job Completed
*/
 if( dbg  ) bcdtmWrite_message(0,0,0,"Number Of Triangles Loaded  = %10ld ** dtmP->numTriangles = %10ld",numTrianglesLoaded,dtmP->numTriangles) ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Load Triangle Mesh  = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime) ) ;
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Interrupt Loading Triangle Shade Mesh Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Interrupt Loading Triangle Shade Mesh Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( dtmP->dtmState == DTMState::Tin )
   {
    bcdtmList_nullTptrValuesDtmObject(dtmP) ;
    bcdtmList_nullSptrValuesDtmObject(dtmP) ;
   }
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR  ;
 goto cleanup ;
}


/*==============================================================================*//**
* @memo   Loads The Dtm As A Mesh For Rendering Or Shading Purposes For QvCache
* @doc    Loads The Dtm As A Mesh For Rendering Or Shading Purposes For QvCache
* @notes  The mesh points are stored in meshPtsPP
* @notes  The mesh faces  are stored in meshFacesP
* @notes  The meshPtsPP and meshFacesP arrays can be placed directly in the mdl function
* @notes  to create a poly face mesh. Example :-
* @notes  mdlMesh_newPolyface(&meshP,NULL,meshFacesP,3,numMeshFacesP,meshPtsPP,numMeshPtsP)
* @param  dtmP                  ==> Pointer To Dtm object
* @param  maxTriangles          ==> Maximum Number Of Triangles To Be Returned Per Call
* @param  *loadFunctionP        ==> Pointer To Load Function
* @param  useFence              ==> Load Feature Within Fence < TRUE,FALSE >
* @param  fenceType,            ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape>
* @param  fenceOption           ==> Fence Option <INSIDE(1),OVERLAP(2)>
* @param  fencePts              ==> DPoint3d Array Of Fence Points
* @param  numFencePts           ==> Number Of Fence Points
* @param  userP                 ==> User Pointer Passed Back To User
* @return DTM_SUCCESS or DTM_ERROR
* @author Rob Cormack - March 2008 - rob.cormack@bentley.com
* @version
* @see None
*===============================================================================*/
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmInterruptLoad_triangleShadeMeshForQVCacheFromDtmObject
(
 BC_DTM_OBJ *dtmP,         /* ==> Pointer To Dtm Object                              */
 long maxTriangles,        /* ==> Maximum Number Of Triangles To Load Per Call       */
 long vectorOption,        /* ==> Vector Option <1=Surface Derivatives,2=Averaged Triangle Surface Normals> */
 double zAxisFactor,       /* ==> Factor To Exaggerate The z Axis default value 1.0  */
 DTMTriangleShadeMeshCallback loadFunctionP,   /* ==> Pointer To Load Function                           */
 long useFence,            /* ==> Load Feature Within Fence <TRUE,FALSE>             */
 DTMFenceType fenceType,           /* ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape>       */
 DTMFenceOption fenceOption,         /* ==> Fence Option <INSIDE(1),OVERLAP(2)>                */
 DPoint3d  *fencePtsP,          /* ==> DPoint3d Array Of Fence Points                          */
 long numFencePts,         /* ==> Number Of Fence Points                             */
 void *userP               /* ==> User Pointer Passed Back To User                   */
)
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long  pnt1, pnt2, pnt3, clPtr, numTriangles;
 bool  voidTriangle, voidsInDtm;
 long  node,startPnt,lastPnt,numTrianglesLoaded,startTime ;
 long  numMeshPts,minTptrPnt,maxTptrPnt ;
 long  *faceP,*meshFacesP=NULL ;
 DPoint3d   *p3dP,*vectP,*meshPtsP=NULL,normalVector,*meshVectorsP=NULL ;
 long  meshVectorsSize = 0, meshPtsSize = 0;
 double dz ;
 BC_DTM_OBJ    *clipDtmP=NULL ;
 DTM_CIR_LIST  *clistP ;
 DPoint3d *pntP ;
 DTM_TIN_NODE  *nodeP,*node1P,*node2P,*node3P ;

/*
** Write Entry Message
*/
 if( dbg )
   {
    startTime = bcdtmClock() ;
    bcdtmWrite_message(0,0,0,"Interrupt Loading Triangle Shade Mesh") ;
    bcdtmWrite_message(0,0,0,"Dtm Object     = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"maxTriangles   = %8ld",maxTriangles) ;
    bcdtmWrite_message(0,0,0,"vectorOption   = %8ld",vectorOption) ;
    bcdtmWrite_message(0,0,0,"zAxisFactor    = %8ld",zAxisFactor) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP  = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"useFence       = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fenceType      = %8ld",fenceType) ;
    bcdtmWrite_message(0,0,0,"fenceOption    = %8ld",fenceOption) ;
    bcdtmWrite_message(0,0,0,"fencePtsP      = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts    = %8ld",numFencePts) ;
    bcdtmWrite_message(0,0,0,"userP          = %p",userP) ;
   }
/*
** Validate Fence
*/
 if( useFence == TRUE && ( fencePtsP == NULL || numFencePts <= 2 ) ) useFence = FALSE ;
 if( useFence == TRUE && ( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )) useFence = FALSE ;
 if( useFence )
   {
    if( fenceType   != DTMFenceType::Block && fenceType   != DTMFenceType::Shape ) fenceType = DTMFenceType::Block ;
    if( fenceOption != DTMFenceOption::Inside      && fenceOption != DTMFenceOption::Outside && fenceOption != DTMFenceOption::Overlap ) fenceOption = DTMFenceOption::Overlap ;
   }
/*
** Validate Mesh Size
*/
 if( maxTriangles <= 0 ) maxTriangles = 50000 ;
 if( maxTriangles > dtmP->numTriangles ) maxTriangles = dtmP->numTriangles ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Reset maxTriangles To = %8ld",maxTriangles) ;
/*
** Validate Normal Vector Option
*/
 if( vectorOption < 1 || vectorOption > 2 ) vectorOption = 2 ;
/*
** Validate z Axis Factor
*/
 if( zAxisFactor <= 0.0 ) zAxisFactor = 1.0 ;
/*
** Test For Valid Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Valid Dtm Object") ;
 if( bcdtmObject_testForValidDtmObject(dtmP) ) goto errexit ;
/*
**  Check If DTM Is In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Check For Voids In The Triangulation
*/
 voidsInDtm = false ;
 bcdtmList_testForVoidsInDtmObject(dtmP,voidsInDtm) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"voidsInDtm = %2ld",voidsInDtm) ;
/*
**  Build Clipping Dtm For Fence
*/
 if( useFence == TRUE )
   {
    if(fenceType != DTMFenceType::Block)
       {
        bcdtmWrite_message(2,0,0,"DTM Fence Block is the only supported fence option.") ;
        goto errexit ;
       }
    if( bcdtmClip_buildClippingTinFromFencePointsDtmObject(&clipDtmP,fencePtsP,numFencePts)) goto errexit ;
    if( fabs( dtmP->xMin - clipDtmP->xMin ) < dtmP->ppTol ) clipDtmP->xMin = dtmP->xMin ;
    if( fabs( dtmP->xMax - clipDtmP->xMax ) < dtmP->ppTol ) clipDtmP->xMax = dtmP->xMax ;
    if( fabs( dtmP->yMin - clipDtmP->yMin ) < dtmP->ppTol ) clipDtmP->yMin = dtmP->yMin ;
    if( fabs( dtmP->yMax - clipDtmP->yMax ) < dtmP->ppTol ) clipDtmP->yMax = dtmP->yMax ;
    if( fenceType == DTMFenceType::Block && dtmP->xMin >= clipDtmP->xMin && dtmP->xMax <= clipDtmP->xMax &&  dtmP->yMin >= clipDtmP->yMin && dtmP->yMax <= clipDtmP->yMax ) useFence = FALSE ;
    if( dbg ) bcdtmWrite_message(0,0,0,"useFence = %2ld ** fenceType = %2ld",useFence,fenceType) ;
  }
/*
** Initialise
*/
 startPnt = 0 ;
 lastPnt  = dtmP->numPoints - 1 ;
 numTriangles = 0 ;
 numTrianglesLoaded = 0 ;
/*
** Write Start Scan Point And Last Scan Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"startPnt = %8ld lastPnt = %8ld",startPnt,lastPnt) ;
/*
**  Find Points Immediately Before And After Fence
*/
 if( useFence == TRUE )
   {
    startTime = bcdtmClock() ;
    bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMin,&startPnt) ;
    while( startPnt > 0 && pointAddrP(dtmP,startPnt)->x >= clipDtmP->xMin ) --startPnt ;
    if( pointAddrP(dtmP,startPnt)->x < clipDtmP->xMin ) ++startPnt ;
    bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMax,&lastPnt) ;
    while( lastPnt < dtmP->numPoints - 1  && pointAddrP(dtmP,lastPnt)->x <= clipDtmP->xMax ) ++lastPnt ;
    if( tdbg )
      {
       bcdtmWrite_message(0,0,0,"startPnt = %8ld startPnt->x = %12.5lf",startPnt,pointAddrP(dtmP,startPnt)->x) ;
       bcdtmWrite_message(0,0,0,"lastPnt  = %8ld lastPnt->x  = %12.5lf",lastPnt,pointAddrP(dtmP,lastPnt)->x) ;
       bcdtmWrite_message(0,0,0,"clipDtmP->xMin  = %12.5lf clipDtmP->xMax  = %12.5lf",clipDtmP->xMin,clipDtmP->xMax) ;
       bcdtmWrite_message(0,0,0,"lastPnt  = %8ld lastPnt->x  = %12.5lf",lastPnt,pointAddrP(dtmP,lastPnt)->x) ;
       bcdtmWrite_message(0,0,0,"** Index Time 00 = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
      }
/*
**  Mark Tin Points Within Block Fence
*/
    if( fenceType == DTMFenceType::Block )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Within Fence Block") ;
       for( pnt1 = startPnt ; pnt1 <= lastPnt ; ++pnt1 )
         {
          pntP = pointAddrP(dtmP,pnt1) ;
          if (pntP->x > clipDtmP->xMax || pntP->y > clipDtmP->yMax)
              nodeAddrP(dtmP,pnt1)->sPtr = 2;
          else if( pntP->x >= clipDtmP->xMin && pntP->y >= clipDtmP->yMin ) nodeAddrP(dtmP,pnt1)->sPtr = 1 ;
         }
      }
/*
**  Write Start Scan Point And Last Scan Point
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"startPnt = %8ld lastPnt = %8ld",startPnt,lastPnt) ;
   }
/*
** Allocate Memory For Mesh Faces
*/
 meshFacesP   = (long * ) malloc( maxTriangles * 3 * sizeof(long)) ;
 if( meshFacesP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
 faceP = meshFacesP ;
/*
** Scan DTM And Accumulate Triangle Mesh
*/
 minTptrPnt = dtmP->numPoints ;
 maxTptrPnt = -1 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning DTM Triangles") ;
 for( pnt1 = startPnt ; pnt1 <= lastPnt ; ++pnt1 )
   {
    node1P = nodeAddrP(dtmP,pnt1) ;
    if( useFence == FALSE || ( useFence == TRUE && node1P->sPtr == 1 ))
      {
       if( ( clPtr = node1P->cPtr ) != dtmP->nullPtr )
         {
          if( ( pnt2 = bcdtmList_nextAntDtmObject(dtmP,pnt1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
          node2P = nodeAddrP(dtmP,pnt2) ;
          while( clPtr != dtmP->nullPtr )
            {
             clistP = clistAddrP(dtmP,clPtr) ;
             pnt3   = clistP->pntNum ;
             clPtr  = clistP->nextPtr ;
             node3P = nodeAddrP(dtmP,pnt3) ;
             if( node1P->hPtr != pnt2 )
               {
//                if( pnt2 > pnt1 && pnt3 > pnt1  || ( useFence == TRUE && ( node2P->sPtr != 1 || node3P->sPtr != 1 )))
                if( pnt2 > pnt1 && pnt3 > pnt1  || ( useFence == TRUE && (  node2P->sPtr != 1 || pnt2 > pnt1) && (node3P->sPtr != 1 || pnt3 > pnt1)))
                  {
/*
**                 Test For Void Triangle
*/
                   voidTriangle = false ;

                   //if( useFence == TRUE)
                   //    {
                   //    if(node2P->sPtr == 1 && pnt2 < pnt1)
                   //        voidTriangle = TRUE;
                   //    if(node3P->sPtr == 1 && pnt3 < pnt1)
                   //        voidTriangle = TRUE;
                   //    }

                   if( node2P->sPtr == 2 || node3P->sPtr == 2 || pnt2 > lastPnt || pnt3 > lastPnt)
                       {
                       voidTriangle = true;
                       }

                  if(!voidTriangle && voidsInDtm == true ) if( bcdtmList_testForVoidTriangleDtmObject(dtmP,pnt1,pnt2,pnt3,voidTriangle)) goto errexit ;
/*
**                 Process If None Void Triangle
*/
                   if( voidTriangle == false )
                     {
                      *faceP = pnt3  ; ++faceP ;
                      *faceP = pnt2  ; ++faceP ;
                      *faceP = pnt1  ; ++faceP ;
                      ++numTriangles ;
/*
**                    Check For Maximum Load Triangles
*/
                      if( numTriangles == maxTriangles )
                        {
                         if( dbg ) bcdtmWrite_message(0,0,0,"Creating Triangle Mesh For Max Traingles ** numTriangles = %8ld",numTriangles) ;
/*
**                       Mark Mesh Points
*/
                         minTptrPnt = dtmP->numPoints ;
                         maxTptrPnt = -1 ;
                         for( faceP = meshFacesP ; faceP < meshFacesP + numTriangles * 3 ; ++faceP )
                           {
                            if( *faceP < minTptrPnt ) minTptrPnt = *faceP ;
                            if( *faceP > maxTptrPnt ) maxTptrPnt = *faceP ;
                            nodeAddrP(dtmP,*faceP)->tPtr = 1 ;
                           }
                         if( dbg ) bcdtmWrite_message(0,0,0,"minTptrPoint = %8ld maxTptrPoint = %8ld",minTptrPnt,maxTptrPnt) ;
/*
**                       Count Number Of Mesh Points
*/
                         numMeshPts = 0 ;
                         for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
                           {
                            nodeP = nodeAddrP(dtmP,node) ;
                            if( nodeP->tPtr == 1 )
                              {
                               ++numMeshPts ;
                               nodeP->tPtr = numMeshPts ;
                              }
/*
                            else if (nodeP->tPtr != 0 && nodeP->tPtr != dtmP->nullPnt)
                                {
                                nodeP->tPtr = dtmP->nullPnt;
                                }
*/
                           }
/*
**                       Allocate Memory For Mesh Points
*/
                         if( dbg ) bcdtmWrite_message(0,0,0,"Populating Mesh Points ** numMeshPts = %8ld",numMeshPts) ;
                         if(!meshPtsP || numMeshPts > meshPtsSize )
                             {
                             if(meshPtsP) free(meshPtsP);
                             meshPtsP = ( DPoint3d * ) malloc( numMeshPts * sizeof(DPoint3d) ) ;
                             if( meshPtsP == NULL )
                               {
                                bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                                goto errexit ;
                               }
                             meshPtsSize = numMeshPts;
                             }
/*
**                       Allocate Memory For Mesh Vectors
*/
                         if(!meshVectorsP || numMeshPts > meshVectorsSize)
                             {
                             if(meshVectorsP) free(meshVectorsP);
                             meshVectorsP = ( DPoint3d * ) malloc( numMeshPts * sizeof(DPoint3d) ) ;
                             if( meshVectorsP == NULL )
                               {
                                bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                                goto errexit ;
                               }
                             meshVectorsSize = numMeshPts;
                             }
/*
**                      Exaggerate z Axis
*/
                        if( zAxisFactor != 1.0 )
                          {
                           if( dbg ) bcdtmWrite_message(0,0,0,"Exaggerating z Axis") ;
                           for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
                             {
                              nodeP = nodeAddrP(dtmP,node) ;
                              if( nodeP->tPtr > 0 && nodeP->tPtr < dtmP->nullPnt  )
                                {
                                 pntP    = pointAddrP(dtmP,node) ;
                                 dz = ( pntP->z - dtmP->zMin ) * zAxisFactor ;
                                 pntP->z = dtmP->zMin + dz ;
                                }
                             }
                          }

/*
**                       Populate Mesh Points Array And Mesh Vectors Array
*/
                         p3dP  = meshPtsP ;
                         vectP = meshVectorsP ;
                         for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
                           {
                            nodeP = nodeAddrP(dtmP,node) ;
                            if( nodeP->tPtr > 0 && nodeP->tPtr < dtmP->nullPnt  )
                              {
                               pntP    = pointAddrP(dtmP,node) ;
                               p3dP->x = pntP->x ;
                               p3dP->y = pntP->y ;
                               p3dP->z = pntP->z ;
                               ++p3dP ;
                               if( bcdtmLoad_calculateNormalVectorForTriangleVertexDtmObject(dtmP,node,2,zAxisFactor,&normalVector)) goto errexit ;
                               *vectP  = normalVector ;
                               ++vectP ;
                              }
                           }
/*
**                      De Exaggerate z Axis
*/
                        if( zAxisFactor != 1.0 )
                          {
                           if( dbg ) bcdtmWrite_message(0,0,0,"DE-Exaggerating z Axis") ;
                           for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
                             {
                              nodeP = nodeAddrP(dtmP,node) ;
                              if( nodeP->tPtr > 0 && nodeP->tPtr < dtmP->nullPnt  )
                                {
                                 pntP    = pointAddrP(dtmP,node) ;
                                 dz = ( pntP->z - dtmP->zMin ) / zAxisFactor ;
                                 pntP->z = dtmP->zMin + dz ;
                                }
                             }
                          }
/*
**                       Reset Point Indexes In Mesh Faces
*/
                         for( faceP = meshFacesP ; faceP < meshFacesP + numTriangles * 3 ; ++faceP )
                           {
                            *faceP = nodeAddrP(dtmP,*faceP)->tPtr ;
                           }
/*
**                       Null Tptr Values
*/
                         for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
                           {
                            nodeAddrP(dtmP,node)->tPtr = dtmP->nullPnt ;
                           }
/*
**                       Call Browse Function
*/
                         if( loadFunctionP != NULL )if( loadFunctionP(DTMFeatureType::TriangleShadeMesh,numTriangles,numMeshPts,meshPtsP,meshVectorsP,numTriangles*3,meshFacesP,userP)) goto errexit ;
                         faceP = meshFacesP ;
                         //if( meshPtsP     != NULL ) { free(meshPtsP)     ; meshPtsP       = NULL ; }
                         //if( meshVectorsP != NULL ) { free(meshVectorsP) ; meshVectorsP   = NULL ; }
                         numTrianglesLoaded = numTrianglesLoaded + numTriangles ;
                         numTriangles = 0 ;
                         minTptrPnt = dtmP->numPoints ;
                         maxTptrPnt = -1 ;
                        }
                     }
                  }
               }
/*
**           Continue Scan For Triangles
*/
             pnt2 = pnt3 ;
             node2P = node3P ;
            }
         }
      }
   }
/*
** Check For Unloaded Triangles
*/
 if( numTriangles > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Creating Triangle Mesh For Unloaded Triangles ** numTriangles = %8ld",numTriangles) ;
/*
**  Mark Mesh Points
*/
    minTptrPnt = dtmP->numPoints ;
    maxTptrPnt = -1 ;
    for( faceP = meshFacesP ; faceP < meshFacesP + numTriangles * 3 ; ++faceP )
      {
       if( *faceP < minTptrPnt ) minTptrPnt = *faceP ;
       if( *faceP > maxTptrPnt ) maxTptrPnt = *faceP ;
       nodeAddrP(dtmP,*faceP)->tPtr = 1 ;
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"minTptrPoint = %8ld maxTptrPoint = %8ld",minTptrPnt,maxTptrPnt) ;
/*
**  Count Number Of Mesh Points
*/
    numMeshPts = 0 ;
    for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
      {
       nodeP = nodeAddrP(dtmP,node) ;
       if( nodeP->tPtr == 1 )
         {
          ++numMeshPts ;
          nodeP->tPtr = numMeshPts ;
         }
      }
/*
**  Allocate Memory For Mesh Points
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Populating Mesh Points ** numMeshPts = %8ld",numMeshPts) ;

    if(!meshPtsP || numMeshPts > meshPtsSize)
        {
        if(meshPtsP) free(meshPtsP);
        meshPtsP = ( DPoint3d * ) malloc( numMeshPts * sizeof(DPoint3d) ) ;
        if( meshPtsP == NULL )
          {
           bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
           goto errexit ;
          }
        meshPtsSize = numMeshPts;
        }
/*
**  Allocate Memory For Mesh Vectors
*/
    if(!meshVectorsP || numMeshPts > meshVectorsSize)
        {
        if(meshVectorsP) free(meshVectorsP);
        meshVectorsP = ( DPoint3d * ) malloc( numMeshPts * sizeof(DPoint3d) ) ;
        if( meshVectorsP == NULL )
          {
           bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
           goto errexit ;
          }
        meshVectorsSize = numMeshPts;
        }
/*
**   Exaggerate z Axis
*/
    if( zAxisFactor != 1.0 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Exaggerating z Axis") ;
       for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
         {
          nodeP = nodeAddrP(dtmP,node) ;
          if( nodeP->tPtr > 0 && nodeP->tPtr < dtmP->nullPnt )
            {
             pntP    = pointAddrP(dtmP,node) ;
             dz = ( pntP->z - dtmP->zMin ) * zAxisFactor ;
             pntP->z = dtmP->zMin + dz ;
            }
         }
     }
/*
**  Populate Mesh Points and Mesh Vector Arrayies
*/
    p3dP = meshPtsP ;
    vectP = meshVectorsP ;
    for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
      {
       nodeP = nodeAddrP(dtmP,node) ;
       if( nodeP->tPtr > 0 && nodeP->tPtr < dtmP->nullPnt )
         {
          pntP = pointAddrP(dtmP,node) ;
          p3dP->x = pntP->x ;
          p3dP->y = pntP->y ;
          p3dP->z = pntP->z ;
          ++p3dP ;
          if( bcdtmLoad_calculateNormalVectorForTriangleVertexDtmObject(dtmP,node,2,zAxisFactor,&normalVector)) goto errexit ;
          *vectP  = normalVector ;
          ++vectP ;
         }
      }
/*
**  Reset Point Indexes In Mesh Faces
*/
    for( faceP = meshFacesP ; faceP < meshFacesP + numTriangles * 3 ; ++faceP )
      {
       *faceP = nodeAddrP(dtmP,*faceP)->tPtr ;
      }
/*
**  De Exaggerate z Axis
*/
    if( zAxisFactor != 1.0 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"De-Exaggerating z Axis") ;
       for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
         {
          nodeP = nodeAddrP(dtmP,node) ;
          if( nodeP->tPtr > 0 && nodeP->tPtr < dtmP->nullPnt )
            {
             pntP    = pointAddrP(dtmP,node) ;
             dz = ( pntP->z - dtmP->zMin ) / zAxisFactor ;
             pntP->z = dtmP->zMin + dz ;
            }
         }
      }
/*
**  Null Tptr Values
*/
    for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
      {
       nodeAddrP(dtmP,node)->tPtr = dtmP->nullPnt ;
      }
/*
**  Call Browse Function
*/
    if( loadFunctionP != NULL ) if( loadFunctionP(DTMFeatureType::TriangleShadeMesh,numTriangles,numMeshPts,meshPtsP,meshVectorsP,numTriangles*3,meshFacesP, userP)) goto errexit ;
    faceP = meshFacesP ;
    if( meshPtsP     != NULL ) { free(meshPtsP)     ; meshPtsP       = NULL ; }
    if( meshVectorsP != NULL ) { free(meshVectorsP) ; meshVectorsP   = NULL ; }
    numTrianglesLoaded = numTrianglesLoaded + numTriangles ;
    numTriangles = 0 ;
    minTptrPnt = dtmP->numPoints ;
    maxTptrPnt = -1 ;
   }
/*
** Clean Up
*/
 cleanup :
 if( dtmP->dtmState == DTMState::Tin ) bcdtmList_nullTptrValuesDtmObject(dtmP) ;
 if( meshPtsP       != NULL ) { free(meshPtsP)       ; meshPtsP       = NULL ; }
 if( meshFacesP     != NULL ) { free(meshFacesP)     ; meshFacesP     = NULL ; }
 if( meshVectorsP   != NULL ) { free(meshVectorsP)   ; meshVectorsP   = NULL ; }
 if( clipDtmP       != NULL ) bcdtmObject_destroyDtmObject(&clipDtmP) ;
 if( useFence && dtmP->dtmState == DTMState::Tin ) for( node = startPnt ; node <= lastPnt ; ++node ) nodeAddrP(dtmP,node)->sPtr = dtmP->nullPnt ;
/*
** Job Completed
*/
 if( dbg  ) bcdtmWrite_message(0,0,0,"Number Of Triangles Loaded  = %10ld ** dtmP->numTriangles = %10ld",numTrianglesLoaded,dtmP->numTriangles) ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Load Triangle Mesh  = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime) ) ;
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Interrupt Loading Triangle Shade Mesh Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Interrupt Loading Triangle Sahde Mesh Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( dtmP->dtmState == DTMState::Tin )
   {
    bcdtmList_nullTptrValuesDtmObject(dtmP) ;
    bcdtmList_nullSptrValuesDtmObject(dtmP) ;
   }
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR  ;
 goto cleanup ;
}



/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmLoad_calculateNormalVectorForTriangleVertexDtmObject
(
 BC_DTM_OBJ *dtmP,                    /* ==> Pointer To Dtm Object      */
 long       point,                    /* ==> Tin Point To Calculate Normal Vector For */
 long       vectorOption,             /* ==> VectorOption <1=Surface Derivatives,2=Averaged Triangle Surface Normals> */
 double     zAxisFactor,              /* ==> Factor To Exaggerate The z Coordinate  */
 DPoint3d   *normalVectorP            /* <== Pointer To Normal Vector  */
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long p2,p3,clPtr ;
 DPoint3d  avgVector,trgVector ;
 double dz ;
 DTM_CIR_LIST *clistP ;
 DPoint3d *pntP ;
 DTM_TIN_NODE *node2P,*node3P ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Calculating Normal Vector For Triangle Vertex") ;
/*
** Initialise
*/
 normalVectorP->x = 0.0 ;
 normalVectorP->y = 0.0 ;
 normalVectorP->z = 1.0 ;
/*
** Check For Valid Dtm Object
*/
// if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check DTM In Tin State
*/
// if( dtmP->dtmState != DTMState::Tin )
//   {
//    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
//    goto errexit ;
//   }
/*
**  Check For Point Range Error
*/
 if( point < 0 || point >= dtmP->numPoints )
   {
    bcdtmWrite_message(1,0,0,"Point Range Error") ;
    goto errexit ;
   }
/*
**  Scan Triangles At Vertex
*/
  clPtr = nodeAddrP(dtmP,point)->cPtr ;
  if( clPtr != dtmP->nullPtr )
    {
     avgVector.x = 0.0 ;
     avgVector.y = 0.0 ;
     avgVector.z = 0.0 ;
     if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,point,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
     node2P = nodeAddrP(dtmP,p2) ;
     while( clPtr != dtmP->nullPtr )
       {
        clistP = clistAddrP(dtmP,clPtr) ;
        p3 = clistP->pntNum ;
        clPtr = clistP->nextPtr ;
        node3P = nodeAddrP(dtmP,p3) ;
        if( nodeAddrP(dtmP,p3)->hPtr != point )
          {
/*
**         Check If z Coordinate Has To Be Exaggerated
*/
           if( zAxisFactor != 1.0 )
             {
              if( node2P->tPtr == dtmP->nullPnt )
                {
                 pntP    = pointAddrP(dtmP,p2) ;
                 dz = ( pntP->z - dtmP->zMin ) * zAxisFactor ;
                 pntP->z = dtmP->zMin + dz ;
                }
              if( node3P->tPtr == dtmP->nullPnt )
                {
                 pntP    = pointAddrP(dtmP,p3) ;
                 dz = ( pntP->z - dtmP->zMin ) * zAxisFactor ;
                 pntP->z = dtmP->zMin + dz ;
                }
             }
/*
**         Calculate Normal For Triangle Plane
*/
           bcdtmMath_calculateNormalVectorToPlaneDtmObject(dtmP,point,p3,p2,&trgVector) ;
           avgVector.x = avgVector.x + trgVector.x  ;
           avgVector.y = avgVector.y + trgVector.y ;
           avgVector.z = avgVector.z + trgVector.z ;
/*
**         Check If z Coordinate Has To Be De Exaggerated
*/
           if( zAxisFactor != 1.0 )
             {
              if( node2P->tPtr == dtmP->nullPnt )
                {
                 pntP    = pointAddrP(dtmP,p2) ;
                 dz = ( pntP->z - dtmP->zMin ) / zAxisFactor ;
                 pntP->z = dtmP->zMin + dz ;
                }
              if( node3P->tPtr == dtmP->nullPnt )
                {
                 pntP    = pointAddrP(dtmP,p3) ;
                 dz = ( pntP->z - dtmP->zMin ) / zAxisFactor ;
                 pntP->z = dtmP->zMin + dz ;
                }
             }
          }
        p2 = p3 ;
        node2P = node3P ;
       }
/*
**   Set Return Vector
*/
     *normalVectorP = avgVector;
    }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Calculating Normal Vector For Triangle Vertex Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Calculating Normal Vector For Triangle Vertex Error") ;
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
BENTLEYDTM_EXPORT int bcdtmInterruptLoad_triangleHillShadeMeshFromDtmObject
(
 BC_DTM_OBJ *dtmP,           /* ==> Pointer To Dtm Object                              */
 long    maxTriangles,       /* ==> Maximum Number Of Triangles To Load Per Call       */
 long   greyScaleRange,      /* ==> Grea Scale Range                                   */
 double altitudeDegrees,     /* ==> Altitude Setting In Degrees                        */
 double azimuthDegrees,      /* ==> Azimuth Setting In Degrees                         */
 double zAxisFactor,         /* ==> Factor To Exaggerate The z Axis default value 1.0  */
 DTMTriangleHillShadeMeshCallback loadFunctionP,  /* ==> Pointer To Load Function                           */
 long   useFence,            /* ==> Load Feature Within Fence <TRUE,FALSE>             */
 DTMFenceType fenceType,           /* ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape>       */
 DTMFenceOption fenceOption,         /* ==> Fence Option <INSIDE(1),OVERLAP(2)>                */
 DPoint3d    *fencePtsP,          /* ==> DPoint3d Array Of Fence Points                          */
 long   numFencePts,         /* ==> Number Of Fence Points                             */
 void   *userP               /* ==> User Pointer Passed Back To User                   */
)
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 long  p1, p2, p3, pnt1, pnt2, pnt3, clPtr, numTriangles;
 bool voidTriangle, voidsInDtm;
 long  node,startPnt,lastPnt,numTrianglesLoaded,startTime ;
 long  fndType,tinPnt1,tinPnt2,tinPnt3,numMeshPts,minTptrPnt,maxTptrPnt ;
 long  *faceP,*meshFacesP=NULL ;
 long  *reflP,*meshReflectanceP=NULL;
 double slope,aspect,height,slopeDegrees,slopePercent,reflectance,azRad,altRad ;
 DPoint3d   *p3dP,*meshPtsP=NULL ;
 double dz ;
 BC_DTM_OBJ    *clipDtmP=NULL ;
 DTM_CIR_LIST  *clistP ;
 DPoint3d *pntP ;
 DTM_TIN_NODE  *nodeP,*node1P,*node2P,*node3P ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    startTime = bcdtmClock() ;
    bcdtmWrite_message(0,0,0,"Interrupt Loading Triangle Hill Shade Mesh") ;
    bcdtmWrite_message(0,0,0,"Dtm Object      = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"maxTriangles    = %8ld",maxTriangles) ;
    bcdtmWrite_message(0,0,0,"greyScaleRange  = %8ld",greyScaleRange) ;
    bcdtmWrite_message(0,0,0,"altitudeDegrees = %8.3lf",altitudeDegrees) ;
    bcdtmWrite_message(0,0,0,"azimuthDegrees  = %8.3lf",azimuthDegrees) ;
    bcdtmWrite_message(0,0,0,"zAxisFactor     = %8.3lf",zAxisFactor) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP   = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"useFence        = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fenceType       = %8ld",fenceType) ;
    bcdtmWrite_message(0,0,0,"fenceOption     = %8ld",fenceOption) ;
    bcdtmWrite_message(0,0,0,"fencePtsP       = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts     = %8ld",numFencePts) ;
    bcdtmWrite_message(0,0,0,"userP           = %p",userP) ;
   }
/*
** Validate Fence
*/
 if( useFence == TRUE && ( fencePtsP == NULL || numFencePts <= 2 ) ) useFence = FALSE ;
 if( useFence == TRUE && ( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )) useFence = FALSE ;
 if( useFence )
   {
    if( fenceType   != DTMFenceType::Block && fenceType   != DTMFenceType::Shape ) fenceType = DTMFenceType::Block ;
    if( fenceOption != DTMFenceOption::Inside      && fenceOption != DTMFenceOption::Outside && fenceOption != DTMFenceOption::Overlap ) fenceOption = DTMFenceOption::Overlap ;
   }
/*
** Validate Mesh Size
*/
 if( maxTriangles <= 0 ) maxTriangles = 50000 ;
 if( maxTriangles > dtmP->numTriangles ) maxTriangles = dtmP->numTriangles ;
 if( dbg ) bcdtmWrite_message(0,0,0,"maxTriangles = %8ld",maxTriangles) ;
/*
** Validate Normal Vector Option
*/
/*
** Validate z Axis Factor
*/
 if( zAxisFactor <= 0.0 ) zAxisFactor = 1.0 ;
/*
** Test For Valid Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Valid Dtm Object") ;
 if( bcdtmObject_testForValidDtmObject(dtmP) ) goto errexit ;
/*
**  Check If DTM Is In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Check For Voids In The Triangulation
*/
 voidsInDtm = false ;
 bcdtmList_testForVoidsInDtmObject(dtmP,voidsInDtm) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"voidsInDtm = %2ld",voidsInDtm) ;
/*
**  Build Clipping Dtm For Fence
*/
 if( useFence == TRUE )
   {
    if( bcdtmClip_buildClippingTinFromFencePointsDtmObject(&clipDtmP,fencePtsP,numFencePts)) goto errexit ;
    if( fenceType == DTMFenceType::Block && dtmP->xMin >= clipDtmP->xMin && dtmP->xMax <= clipDtmP->xMax &&  dtmP->yMin >= clipDtmP->yMin && dtmP->yMax <= clipDtmP->yMax ) useFence = FALSE ;
    if( dbg ) bcdtmWrite_message(0,0,0,"useFence = %2ld ** fenceType = %2ld",useFence,fenceType) ;
  }
/*
** Initialise
*/
 startPnt = 0 ;
 lastPnt  = dtmP->numPoints - 1 ;
 numTriangles = 0 ;
 numTrianglesLoaded = 0 ;

// bcdtmList_nullSptrValuesDtmObject(dtmP) ;
// bcdtmList_nullTptrValuesDtmObject(dtmP) ;
/*
** Convert Azimuth And Altitude To Degrees
*/
 azRad  = azimuthDegrees  * DTM_PYE / 180.0 ;
 altRad = altitudeDegrees * DTM_PYE / 180.0 ;
/*
** Write Start Scan Point And Last Scan Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"startPnt = %8ld lastPnt = %8ld",startPnt,lastPnt) ;
/*
**  Find Points Immediately Before And After Fence
*/
 if( useFence == TRUE )
   {
    startTime = bcdtmClock() ;
    bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMin,&startPnt) ;
    while( startPnt > 0 && pointAddrP(dtmP,startPnt)->x >= clipDtmP->xMin ) --startPnt ;
    if( pointAddrP(dtmP,startPnt)->x < clipDtmP->xMin ) ++startPnt ;
    bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMax,&lastPnt) ;
    while( lastPnt < dtmP->numPoints - 1  && pointAddrP(dtmP,lastPnt)->x <= clipDtmP->xMin ) ++lastPnt ;
    if( tdbg )
      {
       bcdtmWrite_message(0,0,0,"startPnt = %8ld startPnt->x = %12.5lf",startPnt,pointAddrP(dtmP,startPnt)->x) ;
       bcdtmWrite_message(0,0,0,"lastPnt  = %8ld lastPnt->x  = %12.5lf",lastPnt,pointAddrP(dtmP,lastPnt)->x) ;
       bcdtmWrite_message(0,0,0,"clipDtmP->xMin  = %12.5lf clipDtmP->xMax  = %12.5lf",clipDtmP->xMin,clipDtmP->xMax) ;
       bcdtmWrite_message(0,0,0,"lastPnt  = %8ld lastPnt->x  = %12.5lf",lastPnt,pointAddrP(dtmP,lastPnt)->x) ;
       bcdtmWrite_message(0,0,0,"** Index Time 00 = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
      }
/*
**  Mark Tin Points Within Block Fence
*/
    if( fenceType == DTMFenceType::Block )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Within Fence Block") ;
       for( pnt1 = startPnt ; pnt1 <= lastPnt ; ++pnt1 )
         {
          pntP = pointAddrP(dtmP,pnt1) ;
          if( pntP->x >= clipDtmP->xMin && pntP->x <= clipDtmP->xMax &&
              pntP->y >= clipDtmP->yMin && pntP->y <= clipDtmP->yMax     ) nodeAddrP(dtmP,pnt1)->sPtr = 1 ;
         }
      }
/*
**  Mark Tin Points Within Shape Fence
*/
    if( fenceType == DTMFenceType::Shape )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Within Fence Shape") ;
       for( pnt1 = startPnt ; pnt1 <= lastPnt ; ++pnt1 )
         {
          pntP = pointAddrP(dtmP,pnt1) ;
          if( pntP->x >= clipDtmP->xMin && pntP->x <= clipDtmP->xMax &&
              pntP->y >= clipDtmP->yMin && pntP->y <= clipDtmP->yMax     )
            {
             if( bcdtmFind_triangleDtmObject(dtmP,pntP->x,pntP->y,&fndType,&tinPnt1,&tinPnt2,&tinPnt3)) goto errexit ;
             if( fndType ) nodeAddrP(dtmP,pnt1)->sPtr = 1 ;
            }
         }
      }
/*
**  Write Start Scan Point And Last Scan Point
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"startPnt = %8ld lastPnt = %8ld",startPnt,lastPnt) ;
   }
/*
** Allocate Memory For Mesh Faces
*/
 meshFacesP   = (long * ) malloc( maxTriangles * 3 * sizeof(long)) ;
 if( meshFacesP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
 faceP = meshFacesP ;
/*
** Allocate Memory For Mesh Face Reflectance
*/
 meshReflectanceP = (long * ) malloc( maxTriangles * sizeof(long)) ;
 if( meshReflectanceP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
 reflP = meshReflectanceP ;
/*
** Scan DTM And Accumulate Triangle Mesh
*/
 minTptrPnt = dtmP->numPoints ;
 maxTptrPnt = -1 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning DTM Triangles") ;
 for( pnt1 = startPnt ; pnt1 <= lastPnt ; ++pnt1 )
   {
    node1P = nodeAddrP(dtmP,pnt1) ;
    if( useFence == FALSE || ( useFence == TRUE && node1P->sPtr == 1 ))
      {
       if( ( clPtr = node1P->cPtr ) != dtmP->nullPtr )
         {
          if( ( pnt2 = bcdtmList_nextAntDtmObject(dtmP,pnt1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
          node2P = nodeAddrP(dtmP,pnt2) ;
          while( clPtr != dtmP->nullPtr )
            {
             clistP = clistAddrP(dtmP,clPtr) ;
             pnt3   = clistP->pntNum ;
             clPtr  = clistP->nextPtr ;
             node3P = nodeAddrP(dtmP,pnt3) ;
             if( node1P->hPtr != pnt2 )
               {
                if( pnt2 > pnt1 && pnt3 > pnt1  || ( useFence == TRUE && ( node2P->sPtr != 1 || node3P->sPtr != 1 )))
                  {
/*
**                 Test For Void Triangle
*/
                   voidTriangle = false ;
                   if( voidsInDtm == true ) if( bcdtmList_testForVoidTriangleDtmObject(dtmP,pnt1,pnt2,pnt3,voidTriangle)) goto errexit ;
/*
**                 Process If None Void Triangle
*/
                   if( voidTriangle == false )
                     {
                      *faceP = pnt3  ; ++faceP ;
                      *faceP = pnt2  ; ++faceP ;
                      *faceP = pnt1  ; ++faceP ;
                      ++numTriangles ;
/*
**                    Check For Maximum Load Triangles
*/
                      if( numTriangles == maxTriangles )
                        {
                         if( dbg ) bcdtmWrite_message(0,0,0,"Creating Triangle Mesh For Max Traingles ** numTriangles = %8ld",numTriangles) ;
/*
**                       Mark Mesh Points
*/
                         minTptrPnt = dtmP->numPoints ;
                         maxTptrPnt = -1 ;
                         for( faceP = meshFacesP ; faceP < meshFacesP + numTriangles * 3 ; ++faceP )
                           {
                            if( *faceP < minTptrPnt ) minTptrPnt = *faceP ;
                            if( *faceP > maxTptrPnt ) maxTptrPnt = *faceP ;
                            nodeAddrP(dtmP,*faceP)->tPtr = 1 ;
                           }
                         if( dbg ) bcdtmWrite_message(0,0,0,"minTptrPoint = %8ld maxTptrPoint = %8ld",minTptrPnt,maxTptrPnt) ;
/*
**                       Count Number Of Mesh Points
*/
                         numMeshPts = 0 ;
                         for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
                           {
                            nodeP = nodeAddrP(dtmP,node) ;
                            if( nodeP->tPtr == 1 )
                              {
                               ++numMeshPts ;
                               nodeP->tPtr = numMeshPts ;
                              }
                           }
/*
**                       Allocate Memory For Mesh Points
*/
                         if( dbg ) bcdtmWrite_message(0,0,0,"Populating Mesh Points ** numMeshPts = %8ld",numMeshPts) ;
                         meshPtsP = ( DPoint3d * ) malloc( numMeshPts * sizeof(DPoint3d) ) ;
                         if( meshPtsP == NULL )
                           {
                            bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                            goto errexit ;
                           }
/*
**                      Exaggerate z Axis
*/
                        if( zAxisFactor != 1.0 )
                          {
                           if( dbg ) bcdtmWrite_message(0,0,0,"Exaggerating z Axis") ;
                           for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
                             {
                              nodeP = nodeAddrP(dtmP,node) ;
                              if( nodeP->tPtr > 0 && nodeP->tPtr < dtmP->nullPnt  )
                                {
                                 pntP    = pointAddrP(dtmP,node) ;
                                 dz = ( pntP->z - dtmP->zMin ) * zAxisFactor ;
                                 pntP->z = dtmP->zMin + dz ;
                                }
                             }
                          }

/*
**                       Populate Mesh Points Array
*/
                         if( dbg ) bcdtmWrite_message(0,0,0,"Populating Mesh Points Array") ;
                         p3dP  = meshPtsP ;
                         for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
                           {
                            nodeP = nodeAddrP(dtmP,node) ;
                            if( nodeP->tPtr > 0 && nodeP->tPtr < dtmP->nullPnt  )
                              {
                               pntP    = pointAddrP(dtmP,node) ;
                               p3dP->x = pntP->x ;
                               p3dP->y = pntP->y ;
                               p3dP->z = pntP->z ;
                               ++p3dP ;
                              }
                           }
/*
**                      Populate Reflectance Array
*/
                        if( dbg ) bcdtmWrite_message(0,0,0,"Populating Reflectance Array") ;
                        reflP = meshReflectanceP ;
                        for( faceP = meshFacesP ; faceP < meshFacesP + numTriangles * 3 ; faceP = faceP + 3 )
                          {
                           p1 = *faceP ;
                           p2 = *(faceP+1) ;
                           p3 = *(faceP+2) ;
                           bcdtmMath_getTriangleAttributesDtmObject(dtmP,p1,p2,p3,&slopeDegrees,&slopePercent,&aspect,&height) ;
                           slope = slopePercent / 100.0 ;
                           aspect = aspect * DTM_PYE / 180.0 ;
                           reflectance = sin(altRad)*sin(slope) + cos(altRad)*cos(slope)*cos(-azRad - aspect - DTM_PYE / 2.0 ) ;
                           if( reflectance < 0.0 ) reflectance = 0.0 ;
                           *reflP = ( long)( ( double ) greyScaleRange * reflectance )  ;
                           ++reflP ;
                          }

/*
**                      De Exaggerate z Axis
*/
                        if( zAxisFactor != 1.0 )
                          {
                           if( dbg ) bcdtmWrite_message(0,0,0,"De Exaggerating z Axis") ;
                           for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
                             {
                              nodeP = nodeAddrP(dtmP,node) ;
                              if( nodeP->tPtr > 0 && nodeP->tPtr < dtmP->nullPnt  )
                                {
                                 pntP    = pointAddrP(dtmP,node) ;
                                 dz = ( pntP->z - dtmP->zMin ) / zAxisFactor ;
                                 pntP->z = dtmP->zMin + dz ;
                                }
                             }
                          }
/*
**                       Reset Point Indexes In Mesh Faces
*/
                         for( faceP = meshFacesP ; faceP < meshFacesP + numTriangles * 3 ; ++faceP )
                           {
                            *faceP = nodeAddrP(dtmP,*faceP)->tPtr ;
                           }
/*
**                       Null Tptr Values
*/
                         for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
                           {
                            nodeAddrP(dtmP,node)->tPtr = dtmP->nullPnt ;
                           }
/*
**                       Call Browse Function
*/
                         if( dbg ) bcdtmWrite_message(0,0,0,"Calling Triangle Hill Shade Mesh Browsing Function") ;
                         if( loadFunctionP(DTMFeatureType::TriangleHillShadeMesh,numTriangles,numMeshPts,meshPtsP,meshReflectanceP,numTriangles*3,meshFacesP,userP)) goto errexit ;
                         faceP = meshFacesP ;
                         if( meshPtsP != NULL ) { free(meshPtsP); meshPtsP = NULL ; }
                         numTrianglesLoaded = numTrianglesLoaded + numTriangles ;
                         numTriangles = 0 ;
                         minTptrPnt = dtmP->numPoints ;
                         maxTptrPnt = -1 ;
                        }
                     }
                  }
               }
/*
**           Continue Scan For Triangles
*/
             pnt2 = pnt3 ;
             node2P = node3P ;
            }
         }
      }
   }
/*
** Check For Unloaded Triangles
*/
 if( numTriangles > 0 )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Creating Triangle Mesh For Unloaded Triangles ** numTriangles = %8ld",numTriangles) ;
/*
**  Mark Mesh Points
*/
    minTptrPnt = dtmP->numPoints ;
    maxTptrPnt = -1 ;
    for( faceP = meshFacesP ; faceP < meshFacesP + numTriangles * 3 ; ++faceP )
      {
       if( *faceP < minTptrPnt ) minTptrPnt = *faceP ;
       if( *faceP > maxTptrPnt ) maxTptrPnt = *faceP ;
       nodeAddrP(dtmP,*faceP)->tPtr = 1 ;
      }
    if( dbg ) bcdtmWrite_message(0,0,0,"minTptrPoint = %8ld maxTptrPoint = %8ld",minTptrPnt,maxTptrPnt) ;
/*
**  Count Number Of Mesh Points
*/
    numMeshPts = 0 ;
    for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
      {
       nodeP = nodeAddrP(dtmP,node) ;
       if( nodeP->tPtr == 1 )
         {
          ++numMeshPts ;
          nodeP->tPtr = numMeshPts ;
         }
      }
/*
**  Allocate Memory For Mesh Points
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Populating Mesh Points ** numMeshPts = %8ld",numMeshPts) ;
    meshPtsP = ( DPoint3d * ) malloc( numMeshPts * sizeof(DPoint3d) ) ;
    if( meshPtsP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**   Exaggerate z Axis
*/
    if( zAxisFactor != 1.0 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Exaggerating z Axis") ;
       for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
         {
          nodeP = nodeAddrP(dtmP,node) ;
          if( nodeP->tPtr > 0 && nodeP->tPtr < dtmP->nullPnt )
            {
             pntP    = pointAddrP(dtmP,node) ;
             dz = ( pntP->z - dtmP->zMin ) * zAxisFactor ;
             pntP->z = dtmP->zMin + dz ;
            }
         }
     }
/*
**  Populate Mesh Points
*/
    p3dP = meshPtsP ;
    for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
      {
       nodeP = nodeAddrP(dtmP,node) ;
       if( nodeP->tPtr > 0 && nodeP->tPtr < dtmP->nullPnt )
         {
          pntP = pointAddrP(dtmP,node) ;
          p3dP->x = pntP->x ;
          p3dP->y = pntP->y ;
          p3dP->z = pntP->z ;
          ++p3dP ;
         }
      }
/*
**  Populate Reflectance Array
*/
    reflP = meshReflectanceP ;
    for( faceP = meshFacesP ; faceP < meshFacesP + numTriangles * 3 ; faceP = faceP + 3  )
      {
       p1 = *faceP ;
       p2 = *(faceP+1) ;
       p3 = *(faceP+2) ;
       bcdtmMath_getTriangleAttributesDtmObject(dtmP,p1,p2,p3,&slopeDegrees,&slopePercent,&aspect,&height) ;
       slope = slopePercent / 100.0 ;
       aspect = aspect * DTM_PYE / 180.0 ;
       reflectance = sin(altRad)*sin(slope) + cos(altRad)*cos(slope)*cos(-azRad - aspect - DTM_PYE / 2.0 ) ;
       if( reflectance < 0.0 ) reflectance = 0.0 ;
       *reflP = ( long)( ( double ) greyScaleRange * reflectance )  ;
       ++reflP ;
      }
/*
**  Reset Point Indexes In Mesh Faces
*/
    for( faceP = meshFacesP ; faceP < meshFacesP + numTriangles * 3 ; ++faceP )
      {
       *faceP = nodeAddrP(dtmP,*faceP)->tPtr ;
      }
/*
**  De Exaggerate z Axis
*/
    if( zAxisFactor != 1.0 )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Exaggerating z Axis") ;
       for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
         {
          nodeP = nodeAddrP(dtmP,node) ;
          if( nodeP->tPtr > 0 && nodeP->tPtr < dtmP->nullPnt )
            {
             pntP    = pointAddrP(dtmP,node) ;
             dz = ( pntP->z - dtmP->zMin ) / zAxisFactor ;
             pntP->z = dtmP->zMin + dz ;
            }
         }
      }
/*
**  Null Tptr Values
*/
    for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
      {
       nodeAddrP(dtmP,node)->tPtr = dtmP->nullPnt ;
      }
/*
**  Call Browse Function
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"Calling Triange Hill Shade Mesh Browsing Function") ;
    if( loadFunctionP(DTMFeatureType::TriangleHillShadeMesh,numTriangles,numMeshPts,meshPtsP,meshReflectanceP,numTriangles*3,meshFacesP, userP)) goto errexit ;
    faceP = meshFacesP ;
    if( meshPtsP  != NULL ) { free(meshPtsP)     ; meshPtsP       = NULL ; }
    numTrianglesLoaded = numTrianglesLoaded + numTriangles ;
    numTriangles = 0 ;
    minTptrPnt = dtmP->numPoints ;
    maxTptrPnt = -1 ;
   }
/*
** Clean Up
*/
 cleanup :
 if( meshPtsP         != NULL ) { free(meshPtsP)         ; meshPtsP         = NULL ; }
 if( meshFacesP       != NULL ) { free(meshFacesP)       ; meshFacesP       = NULL ; }
 if( meshReflectanceP != NULL ) { free(meshReflectanceP) ; meshReflectanceP = NULL ; }
 if( clipDtmP         != NULL ) bcdtmObject_destroyDtmObject(&clipDtmP) ;
 if( useFence ) for( node = startPnt ; node <= lastPnt ; ++node ) nodeAddrP(dtmP,node)->sPtr = dtmP->nullPnt ;

/*
** Job Completed
*/
 if( dbg  ) bcdtmWrite_message(0,0,0,"Number Of Triangles Loaded  = %10ld ** dtmP->numTriangles = %10ld",numTrianglesLoaded,dtmP->numTriangles) ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Load Triangle Mesh  = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime) ) ;
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Interrupt Loading Triangle Hill Shade Mesh Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Interrupt Loading Triangle Hill Sahde Mesh Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
 bcdtmList_nullSptrValuesDtmObject(dtmP) ;
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR  ;
 goto cleanup ;
}



/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
static inline void bcdtmRange_addPointToRange (BC_DTM_OBJ *dtmP, long p, DRange3d& range)
    {
    DPoint3d* pt = pointAddrP (dtmP, p);
    range.extend ( pt);
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmRange_triangleShadeMeshForQVCacheFromDtmObject
(
 BC_DTM_OBJ *dtmP,         /* ==> Pointer To Dtm Object                              */
 bool useFence,            /* ==> Load Feature Within Fence <TRUE,FALSE>             */
 DTMFenceType fenceType,           /* ==> Fence Type <DTMFenceType::Block,DTMFenceType::Shape>       */
 DTMFenceOption fenceOption,         /* ==> Fence Option <INSIDE(1),OVERLAP(2)>                */
 DPoint3d *fencePtsP,      /* ==> DPoint3d Array Of Fence Points                     */
 long numFencePts,         /* ==> Number Of Fence Points                             */
 DRange3d& range
)
{
 int   ret=DTM_SUCCESS,dbg=0,tdbg=0 ;
 long  pnt1,pnt2,pnt3,clPtr,numTriangles;
 long  node,startPnt,lastPnt,numTrianglesLoaded,startTime ;
 long minTptrPnt,maxTptrPnt ;
 BC_DTM_OBJ    *clipDtmP=NULL ;
 DTM_CIR_LIST  *clistP ;
 DPoint3d *pntP ;
 DTM_TIN_NODE  *node1P,*node2P,*node3P ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    startTime = bcdtmClock() ;
    bcdtmWrite_message(0,0,0,"Interrupt Loading Triangle Shade Mesh") ;
    bcdtmWrite_message(0,0,0,"Dtm Object     = %p",dtmP) ;
//    bcdtmWrite_message(0,0,0,"zAxisFactor    = %8ld",zAxisFactor) ;
    bcdtmWrite_message(0,0,0,"useFence       = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fenceType      = %8ld",fenceType) ;
    bcdtmWrite_message(0,0,0,"fenceOption    = %8ld",fenceOption) ;
    bcdtmWrite_message(0,0,0,"fencePtsP      = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts    = %8ld",numFencePts) ;
   }
/*
** Initialise
*/
 startPnt = 0 ;
 lastPnt  = dtmP->numPoints - 1 ;
 numTriangles = 0 ;
 numTrianglesLoaded = 0 ;
 range.init();
/*
** Validate Fence
*/
 if( useFence == TRUE && ( fencePtsP == NULL || numFencePts <= 2 ) ) useFence = FALSE ;
 if( useFence == TRUE && ( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y )) useFence = FALSE ;
 if( useFence )
   {
    if( fenceType   != DTMFenceType::Block && fenceType   != DTMFenceType::Shape ) fenceType = DTMFenceType::Block ;
    if( fenceOption != DTMFenceOption::Inside      && fenceOption != DTMFenceOption::Outside && fenceOption != DTMFenceOption::Overlap ) fenceOption = DTMFenceOption::Overlap ;
   }
/*
** Test For Valid Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Valid Dtm Object") ;
 if( bcdtmObject_testForValidDtmObject(dtmP) ) goto errexit ;
/*
**  Check If DTM Is In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
**  Build Clipping Dtm For Fence
*/
 if( useFence == TRUE )
   {
    if(fenceType != DTMFenceType::Block)
       {
        bcdtmWrite_message(2,0,0,"DTM Fence Block is the only supported fence option.") ;
        goto errexit ;
       }
    if( bcdtmClip_buildClippingTinFromFencePointsDtmObject(&clipDtmP,fencePtsP,numFencePts)) goto errexit ;
    if( fabs( dtmP->xMin - clipDtmP->xMin ) < dtmP->ppTol ) clipDtmP->xMin = dtmP->xMin ;
    if( fabs( dtmP->xMax - clipDtmP->xMax ) < dtmP->ppTol ) clipDtmP->xMax = dtmP->xMax ;
    if( fabs( dtmP->yMin - clipDtmP->yMin ) < dtmP->ppTol ) clipDtmP->yMin = dtmP->yMin ;
    if( fabs( dtmP->yMax - clipDtmP->yMax ) < dtmP->ppTol ) clipDtmP->yMax = dtmP->yMax ;
    if( fenceType == DTMFenceType::Block && dtmP->xMin >= clipDtmP->xMin && dtmP->xMax <= clipDtmP->xMax &&  dtmP->yMin >= clipDtmP->yMin && dtmP->yMax <= clipDtmP->yMax ) useFence = FALSE ;
    if( dbg ) bcdtmWrite_message(0,0,0,"useFence = %2ld ** fenceType = %2ld",useFence,fenceType) ;
  }
/*
** Write Start Scan Point And Last Scan Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"startPnt = %8ld lastPnt = %8ld",startPnt,lastPnt) ;
/*
**  Find Points Immediately Before And After Fence
*/
 if( useFence == TRUE )
   {
    startTime = bcdtmClock() ;
    bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMin,&startPnt) ;
    while( startPnt > 0 && pointAddrP(dtmP,startPnt)->x >= clipDtmP->xMin ) --startPnt ;
    if( pointAddrP(dtmP,startPnt)->x < clipDtmP->xMin ) ++startPnt ;
    bcdtmFind_binaryScanDtmObject(dtmP,clipDtmP->xMax,&lastPnt) ;
    while( lastPnt < dtmP->numPoints - 1  && pointAddrP(dtmP,lastPnt)->x <= clipDtmP->xMax ) ++lastPnt ;
    if( tdbg )
      {
       bcdtmWrite_message(0,0,0,"startPnt = %8ld startPnt->x = %12.5lf",startPnt,pointAddrP(dtmP,startPnt)->x) ;
       bcdtmWrite_message(0,0,0,"lastPnt  = %8ld lastPnt->x  = %12.5lf",lastPnt,pointAddrP(dtmP,lastPnt)->x) ;
       bcdtmWrite_message(0,0,0,"clipDtmP->xMin  = %12.5lf clipDtmP->xMax  = %12.5lf",clipDtmP->xMin,clipDtmP->xMax) ;
       bcdtmWrite_message(0,0,0,"lastPnt  = %8ld lastPnt->x  = %12.5lf",lastPnt,pointAddrP(dtmP,lastPnt)->x) ;
       bcdtmWrite_message(0,0,0,"** Index Time 00 = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
      }
/*
**  Mark Tin Points Within Block Fence
*/
    if( fenceType == DTMFenceType::Block )
      {
       if( dbg ) bcdtmWrite_message(0,0,0,"Marking Points Within Fence Block") ;
       for( pnt1 = startPnt ; pnt1 <= lastPnt ; ++pnt1 )
         {
          pntP = pointAddrP(dtmP,pnt1) ;
          if(pntP->x > clipDtmP->xMax || pntP->y > clipDtmP->yMax)
              nodeAddrP(dtmP,pnt1)->sPtr = 2;
          else if( pntP->x >= clipDtmP->xMin && pntP->y >= clipDtmP->yMin ) nodeAddrP(dtmP,pnt1)->sPtr = 1 ;
         }
      }
/*
**  Write Start Scan Point And Last Scan Point
*/
    if( dbg ) bcdtmWrite_message(0,0,0,"startPnt = %8ld lastPnt = %8ld",startPnt,lastPnt) ;
   }
/*
** Scan DTM And Accumulate Triangle Mesh
*/
 minTptrPnt = dtmP->numPoints ;
 maxTptrPnt = -1 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning DTM Triangles") ;
 for( pnt1 = startPnt ; pnt1 <= lastPnt ; ++pnt1 )
   {
    node1P = nodeAddrP(dtmP,pnt1) ;
    if( useFence == FALSE || ( useFence == TRUE && node1P->sPtr == 1 ))
      {
       if( ( clPtr = node1P->cPtr ) != dtmP->nullPtr )
         {
          if( ( pnt2 = bcdtmList_nextAntDtmObject(dtmP,pnt1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
          node2P = nodeAddrP(dtmP,pnt2) ;
          while( clPtr != dtmP->nullPtr )
            {
             clistP = clistAddrP(dtmP,clPtr) ;
             pnt3   = clistP->pntNum ;
             clPtr  = clistP->nextPtr ;
             node3P = nodeAddrP(dtmP,pnt3) ;
             if( node1P->hPtr != pnt2 )
               {
                if( pnt2 > pnt1 && pnt3 > pnt1  || ( useFence == TRUE && (  node2P->sPtr != 1 || pnt2 > pnt1) && (node3P->sPtr != 1 || pnt3 > pnt1)))
                  {
                  if (node1P->tPtr == dtmP->nullPnt)
                    {
                    node1P->tPtr = pnt1;
                    if (minTptrPnt > pnt1)
                        minTptrPnt = pnt1;
                    if (maxTptrPnt < pnt1)
                        maxTptrPnt = pnt1;
                    bcdtmRange_addPointToRange (dtmP, pnt1, range);
                    }
                  if (node2P->tPtr == dtmP->nullPnt)
                    {
                    node2P->tPtr = pnt2;
                    if (minTptrPnt > pnt2)
                        minTptrPnt = pnt2;
                    if (maxTptrPnt < pnt2)
                        maxTptrPnt = pnt2;
                    bcdtmRange_addPointToRange (dtmP, pnt2, range);
                    }
                  if (node3P->tPtr == dtmP->nullPnt)
                    {
                    node3P->tPtr = pnt3;
                    if (minTptrPnt > pnt3)
                        minTptrPnt = pnt3;
                    if (maxTptrPnt < pnt3)
                        maxTptrPnt = pnt3;
                    bcdtmRange_addPointToRange (dtmP, pnt3, range);
                    }
                  }
               }
/*
**           Continue Scan For Triangles
*/
             pnt2 = pnt3 ;
             node2P = node3P ;
            }
         }
      }
   }
/*
**  Null Tptr Values
*/
    for( node = minTptrPnt ; node <= maxTptrPnt ; ++node )
       nodeAddrP(dtmP,node)->tPtr = dtmP->nullPnt ;
  if (range.IsEmpty())
      {
      ret = DTM_ERROR;
      goto cleanup2;
      }
/*
** Clean Up
*/
 cleanup :
 if( ret != DTM_SUCCESS && dtmP->dtmState == DTMState::Tin ) bcdtmList_nullTptrValuesDtmObject(dtmP) ;
 cleanup2:
 if( clipDtmP       != NULL ) bcdtmObject_destroyDtmObject(&clipDtmP) ;
 if( useFence && dtmP->dtmState == DTMState::Tin ) for( node = startPnt ; node <= lastPnt ; ++node ) nodeAddrP(dtmP,node)->sPtr = dtmP->nullPnt ;
/*
** Job Completed
*/
 if( dbg  ) bcdtmWrite_message(0,0,0,"Number Of Triangles Loaded  = %10ld ** dtmP->numTriangles = %10ld",numTrianglesLoaded,dtmP->numTriangles) ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Load Triangle Mesh  = %7.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime) ) ;
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Interrupt Loading Triangle Shade Mesh Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Interrupt Loading Triangle Sahde Mesh Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( dtmP->dtmState == DTMState::Tin )
   {
    bcdtmList_nullTptrValuesDtmObject(dtmP) ;
    bcdtmList_nullSptrValuesDtmObject(dtmP) ;
   }
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR  ;
 goto cleanup ;
}



