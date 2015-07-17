/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmTheme.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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
BENTLEYDTM_EXPORT int  bcdtmTheme_loadTriangleAttributesDtmObject
(
 BC_DTM_OBJ *dtmP,               /* ==> Pointer To Dtm Object                        */
 DTMFeatureCallback loadFunctionP,  /* ==> Pointer To Load Function                     */
 long       useFence,            /* ==> Load Feature Within Fence <TRUE,FALSE>       */
 DPoint3d        *fencePtsP,          /* ==> Pointer To Fence Points                      */
 long       numFencePts,         /* ==> Number Of Fence Points                       */
 void       *userP               /* ==> User Pointer Passed Back To User             */ 
)
{
 int      ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long     p1,p2,p3,clPtr,voidTriangle,voidsInDtm=FALSE,fndType;
 DTMFenceOption loadFlag;
 long     trgPnt1,trgPnt2,trgPnt3 ;
 double   height=0.0,slopePercent=0.0,slopeDegrees=0.0,aspect=0.0 ;
 double   xTrgMin,yTrgMin,xTrgMax,yTrgMax ;
 DPoint3d      trgPts[4] ;
 BC_DTM_OBJ    *clipDtmP=NULL ;
 DTM_TIN_POINT *pnt1P,*pnt2P,*pnt3P ;
 DTM_TIN_NODE  *nodeP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Loading Triangle Attributes From Dtm Object") ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
/*
** Check If DTM Is In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Create Clipping Tin
*/
 if( useFence == TRUE )
   {
    if     ( fencePtsP == NULL || numFencePts < 4 ) useFence = FALSE ;
    else if( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y ) useFence = FALSE ;
    else if( bcdtmClip_buildClippingTinFromFencePointsDtmObject(&clipDtmP,fencePtsP,numFencePts) ) goto errexit ;
   }
/*
**  Check For Voids In Dtm
*/
 bcdtmList_testForVoidsInDtmObject(dtmP,&voidsInDtm) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"voidsInDtm = %2ld",voidsInDtm) ;
/*
** Scan All Tin Points
*/
 for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
   {
    clPtr = nodeAddrP(dtmP,p1)->cPtr ;
    if( clPtr != dtmP->nullPtr )
      {
       pnt1P = pointAddrP(dtmP,p1) ;
       nodeP = nodeAddrP(dtmP,p1)  ;
       if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
       pnt2P = pointAddrP(dtmP,p2) ;
       while( clPtr != dtmP->nullPtr )
         {
          p3    = clistAddrP(dtmP,clPtr)->pntNum ;
          clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
          pnt3P = pointAddrP(dtmP,p3) ;
          if( p2 > p1 && p3 > p1 ) 
            {
             if( nodeP->hPtr != p2 ) 
               {
/*
**              Set Bounding Rectangle For Triangle
*/
                loadFlag = DTMFenceOption::Inside ;
                if( useFence == TRUE )
                  { 
                   xTrgMin = xTrgMax = pnt1P->x ;
                   yTrgMin = yTrgMax = pnt1P->y ;
                   if( xTrgMin > pnt2P->x ) xTrgMin = pnt2P->x ;
                   if( xTrgMin > pnt3P->x ) xTrgMin = pnt3P->x ;
                   if( xTrgMax < pnt2P->x ) xTrgMax = pnt2P->x ;
                   if( xTrgMax < pnt3P->x ) xTrgMax = pnt3P->x ;
                   if( yTrgMin > pnt2P->y ) yTrgMin = pnt2P->y ;
                   if( yTrgMin > pnt3P->y ) yTrgMin = pnt3P->y ;
                   if( yTrgMax < pnt2P->y ) yTrgMax = pnt2P->y ;
                   if( yTrgMax < pnt3P->y ) yTrgMax = pnt3P->y ;
                   if (xTrgMax < clipDtmP->xMin || xTrgMin > clipDtmP->xMax) loadFlag = DTMFenceOption::None;
	               if( yTrgMax < clipDtmP->yMin || yTrgMin > clipDtmP->yMax ) loadFlag = DTMFenceOption::None;
                  }
/*
**              Check For Void Triangle
*/
                if (loadFlag == DTMFenceOption::Inside && voidsInDtm == TRUE)
                  { 
                   if( bcdtmList_testForVoidTriangleDtmObject(dtmP,p1,p2,p3,&voidTriangle)) goto errexit ;
                   if( voidTriangle) loadFlag = DTMFenceOption::None ;
                  }
/*
**              Check For OverLap With Fence
*/
                if (loadFlag == DTMFenceOption::Inside && useFence == TRUE)
                  {
                   if( bcdtmFind_triangleDtmObject(clipDtmP,pnt1P->x,pnt1P->y,&fndType,&trgPnt1,&trgPnt2,&trgPnt3)) goto errexit ;
                   if( ! fndType )
                     {
                      if( bcdtmFind_triangleDtmObject(clipDtmP,pnt2P->x,pnt2P->y,&fndType,&trgPnt1,&trgPnt2,&trgPnt3)) goto errexit ;
                      if( ! fndType )
                        { 
                         if( bcdtmFind_triangleDtmObject(clipDtmP,pnt3P->x,pnt3P->y,&fndType,&trgPnt1,&trgPnt2,&trgPnt3)) goto errexit ;
                         if( ! fndType )
                           { 
                            trgPts[0].x = pnt1P->x ; trgPts[0].y = pnt1P->x ; 
                            trgPts[1].x = pnt2P->x ; trgPts[1].y = pnt3P->z ; 
                            trgPts[2].x = pnt3P->x ; trgPts[2].y = pnt3P->y ; 
                            trgPts[3].x = pnt1P->x ; trgPts[3].y = pnt1P->x ; 
                            if( bcdtmLoad_testForOverlapWithTinHullDtmObject(clipDtmP,trgPts,4,&loadFlag)) goto errexit ; 
                           }
                        }
                     } 
                  }
/*
**              Load Triangle Attributes
*/
                if( loadFlag == DTMFenceOption::Inside)
                  {
                   trgPts[0].x = ( pnt1P->x + pnt2P->x + pnt3P->x ) / 3.0 ;
                   trgPts[0].y = ( pnt1P->y + pnt2P->y + pnt3P->y ) / 3.0 ;
                   trgPts[0].z = ( pnt1P->z + pnt2P->z + pnt3P->z ) / 3.0 ;
                   bcdtmMath_getTriangleAttributesDtmObject(dtmP,p1,p2,p3,&slopeDegrees,&slopePercent,&aspect,&height) ;
                   trgPts[1].x = slopeDegrees ;
                   trgPts[1].y = slopePercent ;
                   trgPts[1].z = aspect ;
                   if( bcdtmLoad_callUserLoadFunction(loadFunctionP,DTMFeatureType::Theme,0,dtmP->nullFeatureId,trgPts,2,userP)) goto errexit ;
                  }
               }
            } 
/*
**        Set For Next Triangle
*/
          p2    = p3 ;
          pnt2P = pnt3P ;
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( clipDtmP != NULL ) bcdtmObject_destroyDtmObject(&clipDtmP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Triangle Attributes From Dtm Object Completed")  ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Triangle Attributes From Dtm Object Error")  ;
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
BENTLEYDTM_EXPORT int  bcdtmTheme_loadThemeFromDtmFile
(
 WCharCP dtmFileP,           /* ==> Pointer To Dtm File                          */
 long   polyOption,          /* ==> Polygonise Themes < TRUE,FALSE >             */ 
 long   themeOption,         /* ==> Theme Option                                 */
 DRange1d *themeRangesP,       /* ==> Pointer To Theme Ranges                      */
 long   numThemeRanges,      /* ==> Number Of Theme Ranges                       */
 DTMFeatureCallback loadFunctionP,  /* ==> Pointer To Load Function                     */
 long   useFence,            /* ==> Load Feature Within Fence <TRUE,FALSE>       */
 DPoint3d    *fencePtsP,          /* ==> Pointer To Fence Points                      */
 long   numFencePts,         /* ==> Number Of Fence Points                       */
 void   *userP               /* ==> User Pointer Passed Back To User             */ 
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 BC_DTM_OBJ *dtmP=NULL ;
 DRange1d *themeP ;
/*
** Write Entry Message
*/
 if( dbg )
   { 
    bcdtmWrite_message(0,0,0,"Loading Theme From Dtm File") ;
    bcdtmWrite_message(0,0,0,"dtmFileP        = %s",dtmFileP) ;
    bcdtmWrite_message(0,0,0,"polyOption      = %8ld",polyOption) ;
    bcdtmWrite_message(0,0,0,"themeOption     = %8ld",themeOption) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP   = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"useFence        = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fencePtsP       = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts     = %8ld",numFencePts) ;
    bcdtmWrite_message(0,0,0,"userP           = %p",userP) ;
    bcdtmWrite_message(0,0,0,"themeRangesP    = %p",themeRangesP) ;
    bcdtmWrite_message(0,0,0,"numThemeRanges  = %8ld",numThemeRanges) ;
    for( themeP = themeRangesP ; themeP < themeRangesP + numThemeRanges ; ++themeP )
       {
        bcdtmWrite_message(0,0,0,"Range[%4ld] >= %10.4lf < %10.4lf",(long)(themeP-themeRangesP),themeP->low,themeP->high) ;
       }
   }
/*
** Test If Requested Dtm Is Current Dtm
*/
 if( bcdtmUtility_testForAndSetCurrentDtmObject(&dtmP,dtmFileP)) goto errexit ;
/*
** Determine And Load Theme
*/
 if( bcdtmTheme_loadThemeFromDtmObject(dtmP,polyOption,themeOption,themeRangesP,numThemeRanges,loadFunctionP,useFence,fencePtsP,numFencePts,userP)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Theme From Dtm File Completed")  ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Theme From Dtm File Error")  ;
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
BENTLEYDTM_EXPORT int bcdtmTheme_loadThemeFromDtmObject
(
 BC_DTM_OBJ *dtmP,               /* ==> Pointer To Dtm Object                        */
 long       polyOption,          /* ==> Polygonise Themes < TRUE,FALSE >             */ 
 long       themeOption,         /* ==> Theme Option                                 */
 DRange1d     *themeRangesP,       /* ==> Pointer To Theme Ranges                      */
 long       numThemeRanges,      /* ==> Number Of Theme Ranges                       */
 DTMFeatureCallback loadFunctionP,  /* ==> Pointer To Load Function                     */
 long       useFence,            /* ==> Load Feature Within Fence <TRUE,FALSE>       */
 DPoint3d        *fencePtsP,          /* ==> Pointer To Fence Points                      */
 long       numFencePts,         /* ==> Number Of Fence Points                       */
 void       *userP               /* ==> User Pointer Passed Back To User             */ 
)
/*
** This Function Loads A Theme From A Dtm Object
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0)   ;
 BC_DTM_OBJ *cloneDtmP=NULL ;
 DRange1d *themeP ;
/*
** Write Entry Message
*/
 if( dbg )
   { 
    bcdtmWrite_message(0,0,0,"Loading Theme From Dtm Object") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"polyOption      = %8ld",polyOption) ;
    bcdtmWrite_message(0,0,0,"themeOption     = %8ld",themeOption) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP   = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"useFence        = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fencePtsP       = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts     = %8ld",numFencePts) ;
    bcdtmWrite_message(0,0,0,"userP           = %p",userP) ;
    bcdtmWrite_message(0,0,0,"themeRangesP    = %p",themeRangesP) ;
    bcdtmWrite_message(0,0,0,"numThemeRanges  = %8ld",numThemeRanges) ;
    for( themeP = themeRangesP ; themeP < themeRangesP + numThemeRanges ; ++themeP )
       {
        bcdtmWrite_message(0,0,0,"Range[%4ld] >= %10.4lf < %10.4lf",(long)(themeP-themeRangesP),themeP->low,themeP->high) ;
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
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Create Clipping Tin
*/
 if     ( fencePtsP == NULL || numFencePts < 4 ) useFence = FALSE ;
 else if( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y ) useFence = FALSE ;
 if( useFence == TRUE )
   {
   if( bcdtmClip_cloneAndClipToPolygonDtmObject(dtmP,&cloneDtmP,fencePtsP,numFencePts,DTMClipOption::External)) goto errexit ;
   }
 else  cloneDtmP = dtmP ;
/*
** Load Theme Triangle
*/
 if( polyOption == FALSE ) if( bcdtmTheme_loadTriangleThemesFromDtmObject(cloneDtmP,themeOption,themeRangesP,numThemeRanges,loadFunctionP,userP)) goto errexit ; 
/*
** Load Dtm Polygon Themes
*/
 if( polyOption != FALSE  ) if( bcdtmTheme_loadPolygonThemesFromDtmObject(cloneDtmP,themeOption,themeRangesP,numThemeRanges,loadFunctionP,userP)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
 if( cloneDtmP != NULL && cloneDtmP != dtmP ) bcdtmObject_destroyDtmObject(&cloneDtmP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Theme From Dtm Object Completed")  ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Theme From Dtm Object Error")  ;
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
BENTLEYDTM_Private int bcdtmTheme_loadTriangleThemesFromDtmObject
(
 BC_DTM_OBJ *dtmP,
 long       themeOption,
 DRange1d     *themeRangesP,
 long       numThemeRanges,
 DTMFeatureCallback loadFunctionP,
 void       *userP
)
/*
**  Load Each Triangle Theme
*/
{
 int      ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long     p1,p2,p3,clPtr,loadFlag,voidTriangle,themeIndex,voidsInDtm ;
 double   height=0.0,slopePercent=0.0,slopeDegrees=0.0,aspect=0.0 ;
 DPoint3d      trgPts[4] ;
 DRange1d   *themeP ;
 DTM_TIN_NODE  *nodeP ;
 DTM_TIN_POINT *pnt1P,*pnt2P,*pnt3P ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Loading Triangle Theme From Dtm Object") ;
/*
**  Check For Voids In Dtm
*/
 bcdtmList_testForVoidsInDtmObject(dtmP,&voidsInDtm) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"voidsInDtm = %2ld",voidsInDtm) ;
/*
** Scan All Tin Points
*/
 for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
   {
    clPtr = nodeAddrP(dtmP,p1)->cPtr ;
    if( clPtr != dtmP->nullPtr )
      {
       pnt1P = pointAddrP(dtmP,p1) ;
       nodeP = nodeAddrP(dtmP,p1)  ;
       if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
       pnt2P = pointAddrP(dtmP,p2) ;
       while( clPtr != dtmP->nullPtr )
         {
          p3    = clistAddrP(dtmP,clPtr)->pntNum ;
          clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
          pnt3P = pointAddrP(dtmP,p3) ;
          if( p2 > p1 && p3 > p1 ) 
            {
             if( nodeP->hPtr != p2 ) 
               {
                loadFlag = TRUE ;
/*
**              Check For Void Triangle
*/
                if( loadFlag == TRUE && voidsInDtm == TRUE ) 
                  { 
                   if( bcdtmList_testForVoidTriangleDtmObject(dtmP,p1,p2,p3,&voidTriangle)) goto errexit ;
                   if( voidTriangle) loadFlag = FALSE ;
                  }
/*
**              Load Triangle Theme
*/
                if( loadFlag == TRUE ) 
                  {
                   bcdtmMath_getTriangleAttributesDtmObject(dtmP,p1,p2,p3,&slopeDegrees,&slopePercent,&aspect,&height) ;
                   themeIndex = DTM_NULL_PNT ;
                   for( themeP = themeRangesP ; themeP < themeRangesP + numThemeRanges && themeIndex == DTM_NULL_PNT ; ++themeP )
                     {
                      switch ( themeOption )
                        {
                         case  1 :
                            if( height >= themeP->low && height < themeP->high ) themeIndex = (long) ( themeP-themeRangesP) ;
                         break   ; 

                         case  2 :
                            if( slopePercent >= themeP->low && slopePercent < themeP->high ) themeIndex = (long) ( themeP-themeRangesP) ;
                         break   ; 

                         case  3 :
                            if( slopeDegrees >= themeP->low && slopeDegrees < themeP->high ) themeIndex = (long) ( themeP-themeRangesP) ;
                         break   ; 

                         case  4 :
                           if( aspect >= themeP->low && aspect < themeP->high ) themeIndex = (long) ( themeP-themeRangesP) ;
                         break   ; 
                        }
                     } 
/*
**                 Load Theme
*/
                   if( themeIndex != DTM_NULL_PNT )
                     { 
                      trgPts[0].x = pnt1P->x ; trgPts[0].y = pnt1P->y ; trgPts[0].z = pnt1P->z ; 
                      trgPts[1].x = pnt2P->x ; trgPts[1].y = pnt2P->y ; trgPts[1].z = pnt2P->z ;  
                      trgPts[2].x = pnt3P->x ; trgPts[2].y = pnt3P->y ; trgPts[2].z = pnt3P->z ;  
                      trgPts[3].x = pnt1P->x ; trgPts[3].y = pnt1P->y ; trgPts[3].z = pnt1P->z ; 
                      if( bcdtmLoad_callUserLoadFunction(loadFunctionP,DTMFeatureType::Theme,themeIndex,dtmP->nullFeatureId,trgPts,4,userP)) goto errexit ;
                     } 
                  }
               }
            } 
/*
**        Reset For Next Triangle
*/
          p2 = p3 ;
          pnt2P = pnt3P ;
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
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Triangle Theme From Dtm Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Triangle Theme From Dtm Object Error") ;
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
BENTLEYDTM_EXPORT int  bcdtmTheme_loadPolygonThemesFromDtmObjectOld
(
 BC_DTM_OBJ *dtmP,
 long       themeOption,
 DRange1d     *themeRangesP,
 long       numThemeRanges,
 DTMFeatureCallback loadFunctionP,
 void       *userP
)
{
/*
**  Load Polygonised Triangle Themes
*/
 int      ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0) ;
 char     charValue,nullValue=(char)-1/*255*/,*charP,*themeLineP=NULL ;
 long     p1,p2,p3,lp,clPtr,offset,ofs1,ofs2,loadFlag,voidTriangle,themeIndex,voidsInDtm ;
 long     numFeaturePts=0,memFeaturePts=0,memFeaturePtsInc=10000,startTime ;
 double   height=0.0,slopePercent=0.0,slopeDegrees=0.0,aspect=0.0 ;
 double   x,y,sx,sy,area ;
 DPoint3d      *featurePtsP=NULL ;
 DRange1d   *themeP ;
 DTM_TIN_NODE  *nodeP ;
 DTM_TIN_POINT *pnt1P,*pnt2P,*pnt3P ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Loading Polygonised Triangle Themes From Dtm Object") ;
/*
**  Check For Voids In Dtm
*/
 bcdtmList_testForVoidsInDtmObject(dtmP,&voidsInDtm) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"voidsInDtm = %2ld",voidsInDtm) ;
/*
** Allocate Memory
*/
 themeLineP = ( char * ) malloc ( dtmP->cListPtr * sizeof(char)) ;
 if( themeLineP == NULL ) 
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
 for( charP = themeLineP ; charP < themeLineP + dtmP->cListPtr ; ++charP ) *charP = nullValue ;
/*
** Scan All Tin Points
*/
 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Assigning Triangle Themes") ;
 for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
   {
    clPtr = nodeAddrP(dtmP,p1)->cPtr ;
    if( clPtr != dtmP->nullPtr )
      {
       pnt1P = pointAddrP(dtmP,p1) ;
       nodeP = nodeAddrP(dtmP,p1)  ;
       if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
       pnt2P = pointAddrP(dtmP,p2) ;
       while( clPtr != dtmP->nullPtr )
         {
          p3    = clistAddrP(dtmP,clPtr)->pntNum ;
          clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
          pnt3P = pointAddrP(dtmP,p3) ;
          if( p2 > p1 && p3 > p1 ) 
            {
             if( nodeP->hPtr != p2 ) 
               {
                loadFlag = TRUE ;
/*
**              Check For Void Triangle
*/
                if( voidsInDtm == TRUE ) 
                  { 
                   if( bcdtmList_testForVoidTriangleDtmObject(dtmP,p1,p2,p3,&voidTriangle)) goto errexit ;
                   if( voidTriangle) loadFlag = FALSE ;
                  }
/*
**              Load Triangle Theme
*/
                if( loadFlag == TRUE ) 
                  {
                   bcdtmMath_getTriangleAttributesDtmObject(dtmP,p1,p2,p3,&slopeDegrees,&slopePercent,&aspect,&height) ;
                   themeIndex = DTM_NULL_PNT ;
                   for( themeP = themeRangesP ; themeP < themeRangesP + numThemeRanges && themeIndex == DTM_NULL_PNT ; ++themeP )
                     {
                      switch ( themeOption )
                        {
                         case  1 :
                            if( height >= themeP->low && height < themeP->high ) themeIndex = (long) ( themeP-themeRangesP) ;
                         break   ; 

                         case  2 :
                            if( slopePercent >= themeP->low && slopePercent < themeP->high ) themeIndex = (long) ( themeP-themeRangesP) ;
                         break   ; 

                         case  3 :
                            if( slopeDegrees >= themeP->low && slopeDegrees < themeP->high ) themeIndex = (long) ( themeP-themeRangesP) ;
                         break   ; 

                         case  4 :
                           if( aspect >= themeP->low && aspect < themeP->high ) themeIndex = (long) ( themeP-themeRangesP) ;
                         break   ; 
                        } ;
                     } 
/*
**                 Load Theme
*/
                   if( themeIndex != DTM_NULL_PNT )
                     { 
			          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,p1,p2) ) goto errexit ;
			          *(themeLineP+offset) = ( char )themeIndex ;
			          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,p2,p3) ) goto errexit ;
			          *(themeLineP+offset) = ( char )themeIndex ; ;
			          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,p3,p1) ) goto errexit ;
			          *(themeLineP+offset) = ( char )themeIndex ;
                     }
                  }
               }
            } 
          p2 = p3 ;
         }
      }
   }
/*
** Write Assignment Time
*/
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Assign Triangle Themes = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Extract Polygons On the Dtm Hull
*/
 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Loading Theme Polygons On Tin Hull") ;
 p1 = dtmP->hullPoint ;
 do 
   {
    p2 = nodeAddrP(dtmP,p1)->hPtr ;
    bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs1,p2,p1) ;
    if( ( charValue = *(themeLineP+ofs1)) != nullValue )
      {
       p3 = p2 ;
       p2 = p1 ;
       if( bcdtmLoad_storeFeaturePoint(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z,&featurePtsP,&numFeaturePts,&memFeaturePts,memFeaturePtsInc)) goto errexit ;
       if( bcdtmLoad_storeFeaturePoint(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z,&featurePtsP,&numFeaturePts,&memFeaturePts,memFeaturePtsInc)) goto errexit ;
/*
**    Scan Until Back To First Point
*/
       do
         {
          if( ( p3 = bcdtmList_nextAntDtmObject(dtmP,p2,p3)) < 0  )  goto errexit ;
          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs1,p2,p3) )  goto errexit ;
          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs2,p3,p2) )  goto errexit ;
          while( *(themeLineP+ofs1) == charValue && *(themeLineP+ofs2) == charValue )
            {
             if( ( p3 = bcdtmList_nextAntDtmObject(dtmP,p2,p3)) < 0  ) goto errexit ;
             if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs1,p2,p3) ) goto errexit ;
             if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs2,p3,p2) ) goto errexit ;
            }
          *(themeLineP+ofs1) = nullValue ;
          if( bcdtmLoad_storeFeaturePoint(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z,&featurePtsP,&numFeaturePts,&memFeaturePts,memFeaturePtsInc)) goto errexit ;
          lp = p2 ; p2 = p3 ; p3 = lp ;
         } while ( p2 != p1 ) ;
       if( bcdtmLoad_storeFeaturePoint(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z,&featurePtsP,&numFeaturePts,&memFeaturePts,memFeaturePtsInc)) goto errexit ;
/*
**     Load Theme Polygon
*/
       if( bcdtmLoad_callUserLoadFunction(loadFunctionP,DTMFeatureType::Theme,charValue,dtmP->nullFeatureId,featurePtsP,numFeaturePts,userP)) goto errexit ;
       numFeaturePts = 0 ;
      }
    p1 = nodeAddrP(dtmP,p1)->hPtr  ;
   } while( p1 != dtmP->hullPoint ) ;
/*
** Write Times
*/
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Load Theme Polygons On Tin Hull = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Extract Internal Polygons
*/
 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Loading Internal Theme Polygons") ;
 for( p1=0 ; p1 < dtmP->numPoints ; ++p1 )
   {
    nodeP = nodeAddrP(dtmP,p1) ;
    if( nodeP->hPtr == dtmP->nullPnt )
      { 
       clPtr = nodeAddrP(dtmP,p1)->cPtr ;
       while ( clPtr != dtmP->nullPtr )
         {
          p2 = clistAddrP(dtmP,clPtr)->pntNum ;
          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs1,p1,p2) ) goto errexit ;
          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs2,p2,p1) ) goto errexit ;
          if( *(themeLineP+ofs1) != *(themeLineP+ofs2)  && *(themeLineP+ofs1) != nullValue  && bcdtmList_testLineDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum) && ( nodeAddrP(dtmP,p1)->hPtr != p2 || nodeAddrP(dtmP,p2)->hPtr != p1 ) )
            {         
	         charValue = *(themeLineP+ofs1) ;
	         p3 = p1 ;
/*
**           Get Polygon Direction
*/
             area = 0.0 ; 
             sx = pointAddrP(dtmP,p1)->x ;
             sy = pointAddrP(dtmP,p1)->y ;
             x  = pointAddrP(dtmP,p2)->x - sx ;
             y  = pointAddrP(dtmP,p2)->y - sy  ;
             area = area + ( x * y ) / 2.0 + x * sy ;
             sx = pointAddrP(dtmP,p2)->x ;
             sy = pointAddrP(dtmP,p2)->y ;
             do
               {
                if( ( p3 = bcdtmList_nextAntDtmObject(dtmP,p2,p3)) < 0  )  goto errexit ;
                if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs1,p2,p3) )  goto errexit ;
                if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs2,p3,p2) )  goto errexit ;
                while( *(themeLineP+ofs1) == charValue && *(themeLineP+ofs2) == charValue )
                  {
                   if( ( p3 = bcdtmList_nextAntDtmObject(dtmP,p2,p3)) < 0  ) goto errexit ;
                   if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs1,p2,p3) ) goto errexit ;
                   if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs2,p3,p2) ) goto errexit ;
                  }
                x = pointAddrP(dtmP,p3)->x - sx ; 
                y = pointAddrP(dtmP,p3)->y - sy  ;
                area = area + ( x * y ) / 2.0 + x * sy ;
                sx = pointAddrP(dtmP,p3)->x ; sy = pointAddrP(dtmP,p3)->y ;
	            lp = p2 ; p2 = p3 ; p3 = lp ;
	           } while ( p2 != p1 ) ;
/*
**           If Polygon Is Clockwise Write Polygon
*/
             if( area > 0.0 ) 
               {
                p3 = p1 ;
                p2 = clistAddrP(dtmP,clPtr)->pntNum ;
                if( bcdtmLoad_storeFeaturePoint(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z,&featurePtsP,&numFeaturePts,&memFeaturePts,memFeaturePtsInc)) goto errexit ;
                if( bcdtmLoad_storeFeaturePoint(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z,&featurePtsP,&numFeaturePts,&memFeaturePts,memFeaturePtsInc)) goto errexit ;
/*
**              Scan Until Back To First Point
*/
                do
	              {
                   if( ( p3 = bcdtmList_nextAntDtmObject(dtmP,p2,p3)) < 0  )  goto errexit ;
                   if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs1,p2,p3) )  goto errexit ;
                   if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs2,p3,p2) )  goto errexit ;
                   while( *(themeLineP+ofs1) == charValue && *(themeLineP+ofs2) == charValue )
                     {
                      if( ( p3 = bcdtmList_nextAntDtmObject(dtmP,p2,p3)) < 0  ) goto errexit ;
                      if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs1,p2,p3) ) goto errexit ;
                      if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs2,p3,p2) ) goto errexit ;
                     }
	               *(themeLineP+ofs1)  = nullValue ;
                   if( bcdtmLoad_storeFeaturePoint(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z,&featurePtsP,&numFeaturePts,&memFeaturePts,memFeaturePtsInc)) goto errexit ;
	               lp = p2 ; p2 = p3 ; p3 = lp ;
	              } while ( p2 != p1 ) ;
                if( bcdtmLoad_storeFeaturePoint(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z,&featurePtsP,&numFeaturePts,&memFeaturePts,memFeaturePtsInc)) goto errexit ;
/*
**              Load Theme Polygon
*/
                if( bcdtmLoad_callUserLoadFunction(loadFunctionP,DTMFeatureType::Theme,charValue,dtmP->nullFeatureId,featurePtsP,numFeaturePts,userP)) goto errexit ;
                numFeaturePts = 0 ;
               }
	        }
          clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
         }
      } 
   }
/*
** Write Times
*/
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Load Internal Theme Polygons = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Clean Up
*/
 cleanup :
 if( themeLineP  != NULL ) free(themeLineP)  ;
 if( featurePtsP != NULL ) free(featurePtsP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Polygonised Triangle Themes From Dtm Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Polygonised Triangle Themes From Dtm Object Error") ;
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
BENTLEYDTM_EXPORT int  bcdtmTheme_loadPolygonThemesFromDtmObject
(
 BC_DTM_OBJ *dtmP,
 long       themeOption,
 DRange1d     *themeRangesP,
 long       numThemeRanges,
 DTMFeatureCallback loadFunctionP,
 void       *userP
)
{
/*
**  Load Polygonised Triangle Themes
*/
 int      ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 char     charValue,nullValue=(char)-1/*255*/,*charP,*themeLineP=NULL ;
 long     p1,p2,p3,lp,clPtr,offset,ofs1,ofs2,loadFlag,voidTriangle,themeIndex,voidsInDtm ;
 long     numFeaturePts=0,memFeaturePts=0,memFeaturePtsInc=10000,startTime;
 DTMDirection direction;
 double   height=0.0,slopePercent=0.0,slopeDegrees=0.0,aspect=0.0 ;
 double   area ;
 DPoint3d      *featurePtsP=NULL ;
 DRange1d   *themeP ;
 DTM_TIN_NODE  *nodeP ;
 DTM_TIN_POINT *pnt1P,*pnt2P,*pnt3P ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Loading Polygonised Triangle Themes From Dtm Object") ;
/*
**  Check For Voids In Dtm
*/
 bcdtmList_testForVoidsInDtmObject(dtmP,&voidsInDtm) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"voidsInDtm = %2ld",voidsInDtm) ;
/*
** Allocate Memory
*/
 themeLineP = ( char * ) malloc ( dtmP->cListPtr * sizeof(char)) ;
 if( themeLineP == NULL ) 
   { 
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
 for( charP = themeLineP ; charP < themeLineP + dtmP->cListPtr ; ++charP ) *charP = nullValue ;
/*
** Scan All Tin Points
*/
 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Assigning Triangle Themes") ;
 for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
   {
    clPtr = nodeAddrP(dtmP,p1)->cPtr ;
    if( clPtr != dtmP->nullPtr )
      {
       pnt1P = pointAddrP(dtmP,p1) ;
       nodeP = nodeAddrP(dtmP,p1)  ;
       if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
       pnt2P = pointAddrP(dtmP,p2) ;
       while( clPtr != dtmP->nullPtr )
         {
          p3    = clistAddrP(dtmP,clPtr)->pntNum ;
          clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
          pnt3P = pointAddrP(dtmP,p3) ;
          if( p2 > p1 && p3 > p1 ) 
            {
             if( nodeP->hPtr != p2 ) 
               {
                loadFlag = TRUE ;
/*
**              Check For Void Triangle
*/
                if( voidsInDtm == TRUE ) 
                  { 
                   if( bcdtmList_testForVoidTriangleDtmObject(dtmP,p1,p2,p3,&voidTriangle)) goto errexit ;
                   if( voidTriangle) loadFlag = FALSE ;
                  }
/*
**              Load Triangle Theme
*/
                if( loadFlag == TRUE ) 
                  {
                   bcdtmMath_getTriangleAttributesDtmObject(dtmP,p1,p2,p3,&slopeDegrees,&slopePercent,&aspect,&height) ;
                   themeIndex = DTM_NULL_PNT ;
                   for( themeP = themeRangesP ; themeP < themeRangesP + numThemeRanges && themeIndex == DTM_NULL_PNT ; ++themeP )
                     {
                      switch ( themeOption )
                        {
                         case  1 :
                            if( height >= themeP->low && height < themeP->high ) themeIndex = (long) ( themeP-themeRangesP) ;
                         break   ; 

                         case  2 :
                            if( slopePercent >= themeP->low && slopePercent < themeP->high ) themeIndex = (long) ( themeP-themeRangesP) ;
                         break   ; 

                         case  3 :
                            if( slopeDegrees >= themeP->low && slopeDegrees < themeP->high ) themeIndex = (long) ( themeP-themeRangesP) ;
                         break   ; 

                         case  4 :
                           if( aspect >= themeP->low && aspect < themeP->high ) themeIndex = (long) ( themeP-themeRangesP) ;
                         break   ; 
                        } ;
                     } 
/*
**                 Load Theme
*/
                   if( themeIndex != DTM_NULL_PNT )
                     { 
			          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,p1,p2) ) goto errexit ;
			          *(themeLineP+offset) = ( char )themeIndex ;
			          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,p2,p3) ) goto errexit ;
			          *(themeLineP+offset) = ( char )themeIndex ; ;
			          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&offset,p3,p1) ) goto errexit ;
			          *(themeLineP+offset) = ( char )themeIndex ;
                     }
                  }
               }
            } 
          p2 = p3 ;
         }
      }
   }
/*
** Write Assignment Time
*/
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Assign Triangle Themes = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Extract Polygons On the Dtm Hull
*/
 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Loading Theme Polygons On Tin Hull") ;
 p1 = dtmP->hullPoint ;
 do 
   {
    p2 = nodeAddrP(dtmP,p1)->hPtr ;
    bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs1,p2,p1) ;
    if( ( charValue = *(themeLineP+ofs1)) != nullValue )
      {
       p3 = p2 ;
       p2 = p1 ;
       if( bcdtmLoad_storeFeaturePoint(pointAddrP(dtmP,p1)->x,pointAddrP(dtmP,p1)->y,pointAddrP(dtmP,p1)->z,&featurePtsP,&numFeaturePts,&memFeaturePts,memFeaturePtsInc)) goto errexit ;
       if( bcdtmLoad_storeFeaturePoint(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z,&featurePtsP,&numFeaturePts,&memFeaturePts,memFeaturePtsInc)) goto errexit ;
/*
**    Scan Until Back To First Point
*/
       do
         {
          if( ( p3 = bcdtmList_nextAntDtmObject(dtmP,p2,p3)) < 0  )  goto errexit ;
          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs1,p2,p3) )  goto errexit ;
          if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs2,p3,p2) )  goto errexit ;
          while( *(themeLineP+ofs1) == charValue && *(themeLineP+ofs2) == charValue )
            {
             if( ( p3 = bcdtmList_nextAntDtmObject(dtmP,p2,p3)) < 0  ) goto errexit ;
             if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs1,p2,p3) ) goto errexit ;
             if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs2,p3,p2) ) goto errexit ;
            }
          *(themeLineP+ofs1) = nullValue ;
          if( bcdtmLoad_storeFeaturePoint(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z,&featurePtsP,&numFeaturePts,&memFeaturePts,memFeaturePtsInc)) goto errexit ;
          lp = p2 ; p2 = p3 ; p3 = lp ;
         } while ( p2 != p1 ) ;
       if( bcdtmLoad_storeFeaturePoint(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z,&featurePtsP,&numFeaturePts,&memFeaturePts,memFeaturePtsInc)) goto errexit ;
/*
**     Load Theme Polygon
*/
       if( bcdtmLoad_callUserLoadFunction(loadFunctionP,DTMFeatureType::Theme,charValue,dtmP->nullFeatureId,featurePtsP,numFeaturePts,userP)) goto errexit ;
       numFeaturePts = 0 ;
      }
    p1 = nodeAddrP(dtmP,p1)->hPtr  ;
   } while( p1 != dtmP->hullPoint ) ;
/*
** Write Times
*/
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Load Theme Polygons On Tin Hull = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Extract Internal Polygons
*/
 startTime = bcdtmClock() ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Loading Internal Theme Polygons") ;
 for( p1=0 ; p1 < dtmP->numPoints ; ++p1 )
   {
    nodeP = nodeAddrP(dtmP,p1) ;
    if( ! bcdtmFlag_testVoidBitPCWD(&nodeP->PCWD))
      {    
       if( nodeP->hPtr == dtmP->nullPnt )
         { 
          clPtr =  nodeP->cPtr ;
          while ( clPtr != dtmP->nullPtr )
            {
             p2 = clistAddrP(dtmP,clPtr)->pntNum ;
             if( nodeAddrP(dtmP,p1)->tPtr == dtmP->nullPnt )
               { 
                if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs1,p1,p2) ) goto errexit ;
                if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs2,p2,p1) ) goto errexit ;
                if( *(themeLineP+ofs1) != *(themeLineP+ofs2)  && *(themeLineP+ofs1) != nullValue  )
                  {         
	               charValue = *(themeLineP+ofs1) ;
/*
**                 Trace Theme
*/
	               p3 = p1 ;
                   do
                     {
                      if( ( lp = bcdtmList_nextAntDtmObject(dtmP,p2,p1)) < 0  )  goto errexit ;
                      if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs1,p2,lp) )  goto errexit ;
                      if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs2,lp,p2) )  goto errexit ;
                      while( *(themeLineP+ofs1) == charValue && *(themeLineP+ofs2) == charValue )
                        {
                         if( ( lp = bcdtmList_nextAntDtmObject(dtmP,p2,lp)) < 0  ) goto errexit ;
                         if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs1,p2,lp) ) goto errexit ;
                         if( bcdtmTheme_getLineOffsetDtmObject(dtmP,&ofs2,lp,p2) ) goto errexit ;
                        }
                      nodeAddrP(dtmP,p1)->tPtr = p2 ;
	                  p1 = p2 ;
                      p2 = lp ;
	                 } while ( p1 != p3 ) ;
/*
**                 Check Connectivity Of Tptr Polygon
*/
                   if( cdbg )
                     {
                      if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,p1,0))
                        {
                         bcdtmWrite_message(2,0,0,"Connectivity Error In Tptr Polygon") ;
                         goto errexit ;
                        }  
                     }
/*
**                 Calculate Area Of Theme
*/
                   if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,p1,&area,&direction)) goto errexit ;
//                   if( dbg ) bcdtmWrite_message(0,0,0,"Area = %12.5lf Direction = %2ld",area,direction) ;
/*
**                 If Direction Is Clockwise Write Theme
*/
                   if (direction == DTMDirection::Clockwise)
                     {
                      p2 = p1 ;
                      do
	                    {
                         if( bcdtmLoad_storeFeaturePoint(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z,&featurePtsP,&numFeaturePts,&memFeaturePts,memFeaturePtsInc)) goto errexit ;
	                     p2 = nodeAddrP(dtmP,p2)->tPtr ;
	                    } while ( p2 != p1 ) ;
                      if( bcdtmLoad_storeFeaturePoint(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z,&featurePtsP,&numFeaturePts,&memFeaturePts,memFeaturePtsInc)) goto errexit ;
/*
**                    Load Theme Polygon
*/
                      if( bcdtmLoad_callUserLoadFunction(loadFunctionP,DTMFeatureType::Theme,charValue,dtmP->nullFeatureId,featurePtsP,numFeaturePts,userP)) goto errexit ;
                      numFeaturePts = 0 ;
                     }
/*
**                 Null Tptr List
*/
                   else if( bcdtmList_nullTptrListDtmObject(dtmP,p1)) goto errexit ;
	              }
               }
             clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
            }
         } 
      }
   }
/*
** Write Times
*/
 if( tdbg ) bcdtmWrite_message(0,0,0,"Time To Load Internal Theme Polygons = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Clean Up
*/
 cleanup :
 if( themeLineP  != NULL ) free(themeLineP)  ;
 if( featurePtsP != NULL ) free(featurePtsP) ;
 bcdtmList_nullTptrValuesDtmObject(dtmP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Polygonised Triangle Themes From Dtm Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Polygonised Triangle Themes From Dtm Object Error") ;
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
BENTLEYDTM_Public int bcdtmTheme_getLineOffsetDtmObject(BC_DTM_OBJ *dtmP,long *offsetP,long pnt1,long pnt2 )
/*
** This Function Gets The Offset In The Circular List
*/
{
 int  ret=DTM_SUCCESS ;
 long clPtr ;
/*
** Initialise Variables
*/
 *offsetP = dtmP->nullPnt ;
/*
** Check Point Ranges
*/
 if( pnt1 < 0 || pnt1 >= dtmP->numPoints || pnt2 < 0 || pnt2 >= dtmP->numPoints )
   { 
    bcdtmWrite_message(2,0,0,"Tin Point Range Error") ; 
    goto errexit ; 
   }
/*
** Check Point Has Not Been Deleted
*/
 if( nodeAddrP(dtmP,pnt1)->cPtr == dtmP->nullPtr )
   { 
    bcdtmWrite_message(2,0,0,"No Circular List For Point") ;
    goto errexit ; 
   }
/*
** Scan List
*/
 clPtr = nodeAddrP(dtmP,pnt1)->cPtr ;
 while ( clPtr != dtmP->nullPtr && *offsetP == dtmP->nullPnt )
   {
    if( clistAddrP(dtmP,clPtr)->pntNum == pnt2 ) *offsetP = clPtr ; 
    clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
   }
/*
** Error Exit If Offset Not Found
*/
 if( *offsetP == dtmP->nullPnt ) goto errexit ;
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
BENTLEYDTM_EXPORT int  bcdtmTheme_loadThemeFromLatticeFile
(
 WCharCP latticeFileP,      /* ==> Pointer To Dtm File                          */
 long   polyOption,          /* ==> Polygonise Themes < TRUE,FALSE >             */ 
 long   themeOption,         /* ==> Theme Option                                 */
 DRange1d *themeRangesP,       /* ==> Pointer To Theme Ranges                      */
 long   numThemeRanges,      /* ==> Number Of Theme Ranges                       */
 DTMFeatureCallback loadFunctionP,  /* ==> Pointer To Load Function                     */
 long   useFence,            /* ==> Load Feature Within Fence <TRUE,FALSE>       */
 DPoint3d    *fencePtsP,          /* ==> Pointer To Fence Points                      */
 long   numFencePts,         /* ==> Number Of Fence Points                       */
 void   *userP               /* ==> User Pointer Passed Back To User             */ 
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 DTM_LAT_OBJ *latticeP=NULL ;
 DRange1d *themeP ;
/*
** Write Entry Message
*/
 if( dbg )
   { 
    bcdtmWrite_message(0,0,0,"Loading Theme From Lattice File") ;
    bcdtmWrite_message(0,0,0,"latticeFileP    = %s",latticeFileP) ;
    bcdtmWrite_message(0,0,0,"polyOption      = %8ld",polyOption) ;
    bcdtmWrite_message(0,0,0,"themeOption     = %8ld",themeOption) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP   = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"useFence        = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fencePtsP       = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts     = %8ld",numFencePts) ;
    bcdtmWrite_message(0,0,0,"userP           = %p",userP) ;
    bcdtmWrite_message(0,0,0,"themeRangesP    = %8ld",themeRangesP) ;
    bcdtmWrite_message(0,0,0,"numThemeRanges  = %8ld",numThemeRanges) ;
    for( themeP = themeRangesP ; themeP < themeRangesP + numThemeRanges ; ++themeP )
       {
        bcdtmWrite_message(0,0,0,"Range[%4ld] >= %10.4lf < %10.4lf",(long)(themeP-themeRangesP),themeP->low,themeP->high) ;
       }
   }
/*
** Test If Requested Dtm Is Current Tin
*/
 if( bcdtmUtl_testForAndSetCurrentLatticeObject(&latticeP,latticeFileP)) goto errexit ;
/*
** Determine And Load Theme
*/
 if( bcdtmTheme_loadThemeFromLatticeObject(latticeP,polyOption,themeOption,themeRangesP,numThemeRanges,loadFunctionP,useFence,fencePtsP,numFencePts,userP)) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Theme From Lattice File Completed")  ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Theme From Lattice File Error")  ;
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
BENTLEYDTM_EXPORT int bcdtmTheme_loadThemeFromLatticeObject
(
 DTM_LAT_OBJ *latticeP,           /* ==> Pointer To Lattice Object                    */
 long        polyOption,          /* ==> Polygonise Themes < TRUE,FALSE >             */ 
 long        themeOption,         /* ==> Theme Option                                 */
 DRange1d      *themeRangesP,       /* ==> Pointer To Theme Ranges                      */
 long        numThemeRanges,      /* ==> Number Of Theme Ranges                       */
 DTMFeatureCallback loadFunctionP,  /* ==> Pointer To Load Function                     */
 long        useFence,            /* ==> Load Feature Within Fence <TRUE,FALSE>       */
 DPoint3d         *fencePtsP,          /* ==> Pointer To Fence Points                      */
 long        numFencePts,         /* ==> Number Of Fence Points                       */
 void        *userP               /* ==> User Pointer Passed Back To User             */ 
)
/*
** This Function Loads A Theme From A Lattice Object
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),tdbg=DTM_TIME_VALUE(0)  ;
 long startTime ;
 BC_DTM_OBJ *clipDtmP=NULL ;
 DRange1d *themeP ;
/*
** Write Entry Message
*/
 if( dbg )
   { 
    bcdtmWrite_message(0,0,0,"Loading Theme From Lattice Object") ;
    bcdtmWrite_message(0,0,0,"latticeP        = %p",latticeP) ;
    bcdtmWrite_message(0,0,0,"polyOption      = %8ld",polyOption) ;
    bcdtmWrite_message(0,0,0,"themeOption     = %8ld",themeOption) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP   = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"useFence        = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fencePtsP       = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts     = %8ld",numFencePts) ;
    bcdtmWrite_message(0,0,0,"userP           = %p",userP) ;
    bcdtmWrite_message(0,0,0,"themeRangesP    = %8ld",themeRangesP) ;
    bcdtmWrite_message(0,0,0,"numThemeRanges  = %8ld",numThemeRanges) ;
    for( themeP = themeRangesP ; themeP < themeRangesP + numThemeRanges ; ++themeP )
       {
        bcdtmWrite_message(0,0,0,"Range[%4ld] >= %10.4lf < %10.4lf",(long)(themeP-themeRangesP),themeP->low,themeP->high) ;
       }
   }
/*
** Test For Valid Lattice Object
*/
 if( bcdtmObject_testForValidLatticeObject(latticeP)) goto errexit ;
/*
** Create Clipping Tin
*/
 if( useFence == TRUE )
   {
    if     ( fencePtsP == NULL || numFencePts < 4 ) useFence = FALSE ;
    else if( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y ) useFence = FALSE ;
    else if( bcdtmClip_buildClippingTinFromFencePointsDtmObject(&clipDtmP,fencePtsP,numFencePts) ) goto errexit ;
   }
/*
** Load Lattice Cell Themes
*/
 if( polyOption == FALSE ) if( bcdtmTheme_loadCellThemesFromLatticeObject(latticeP,themeOption,themeRangesP,numThemeRanges,loadFunctionP,useFence,clipDtmP,userP) ) goto errexit ;
/*
** Load Lattice Polygon Themes
*/
 startTime = bcdtmClock() ;
 if( polyOption != FALSE  ) if(  bcdtmTheme_loadPolygonThemesFromLatticeObject(latticeP,themeOption,themeRangesP,numThemeRanges,loadFunctionP,useFence,clipDtmP,userP) ) goto errexit ;
 if( tdbg ) bcdtmWrite_message(0,0,0,"** Time To Determine Polygonise And Load Lattice Theme  = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
/*
** Clean Up
*/
 cleanup :
 if( clipDtmP != NULL ) free(clipDtmP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Theme From Dtm Object Completed")  ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Theme From Dtm Object Error")  ;
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
BENTLEYDTM_Private int  bcdtmTheme_loadCellThemesFromLatticeObject
(
 DTM_LAT_OBJ *latticeP,           /* ==> Pointer To Lattice Object                    */
 long        themeOption,         /* ==> Theme Option                                 */
 DRange1d      *themeRangesP,       /* ==> Pointer To Theme Ranges                      */
 long        numThemeRanges,      /* ==> Number Of Theme Ranges                       */
 DTMFeatureCallback loadFunctionP,  /* ==> Pointer To Load Function                     */
 long        useFence,            /* ==> Load Feature Within Fence <TRUE,FALSE>       */
 BC_DTM_OBJ  *clipDtmP,           /* ==> Pointer To DTM Object Clipping Tin           */
 void        *userP               /* ==> User Pointer Passed Back To User             */ 
)
/*
** This Function Load Cell Lattice Themes
*/
{
 int      ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long     i,j,themeIndex ;
 DTMFenceOption loadFlag;
 long     fndType,trgPnt1,trgPnt2,trgPnt3 ;
 float    z1,z2,z3,z4 ;
 double   height=0.0,slopePercent=0.0,slopeDegrees=0.0,aspect=0.0 ;
 double   x1,y1,x2,y2,xLatMin,yLatMin,xLatMax,yLatMax,zLatMin,zLatMax ;
 DPoint3d      latPts[5] ;
 DRange1d   *themeP ;
/*
** Write Entry Message
*/
 if( dbg )
   { 
    bcdtmWrite_message(0,0,0,"Loading Lattice Cell Themes From Lattice Object") ;
    bcdtmWrite_message(0,0,0,"latticeP        = %p",latticeP) ;
    bcdtmWrite_message(0,0,0,"themeOption     = %8ld",themeOption) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP   = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"useFence        = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"clipDtmP        = %p",clipDtmP) ; 
    bcdtmWrite_message(0,0,0,"userP           = %p",userP) ;
    bcdtmWrite_message(0,0,0,"themeRangesP    = %8ld",themeRangesP) ;
    bcdtmWrite_message(0,0,0,"numThemeRanges  = %8ld",numThemeRanges) ;
    for( themeP = themeRangesP ; themeP < themeRangesP + numThemeRanges ; ++themeP )
       {
        bcdtmWrite_message(0,0,0,"Range[%4ld] >= %10.3lf < %10.4lf",(long)(themeP-themeRangesP),themeP->low,themeP->high) ;
       }
   }
/*
** Scan Lattice Cells
*/
 for( i = 0 ; i < latticeP->NYL - 1 ; ++i )
   {
    for( j = 0 ; j < latticeP->NXL - 1 ; ++j )
      {
       z1 = *(latticeP->LAT + j*latticeP->NYL + i)     ; z2 = *(latticeP->LAT + j*(latticeP->NYL) + i + 1) ;
       z3 = *(latticeP->LAT + (j+1)*latticeP->NYL + i) ; z4 = *(latticeP->LAT + (j+1)*latticeP->NYL + i + 1) ;
       if( z1 != latticeP->NULLVAL && z2 != latticeP->NULLVAL && z3 != latticeP->NULLVAL && z4 != latticeP->NULLVAL )
         {
          x1 = latticeP->DX * i + latticeP->LXMIN ; 
          x2 = x1 + latticeP->DX ;
          y1 = latticeP->DY * j + latticeP->LYMIN;
          y2 = y1 + latticeP->DY ;
          latPts[0].x = x1 ; latPts[0].y = y1 ; latPts[0].z = z1 ;
          latPts[1].x = x2 ; latPts[1].y = y1 ; latPts[1].z = z2 ;
	      latPts[2].x = x2 ; latPts[2].y = y2 ; latPts[2].z = z4 ;
	      latPts[3].x = x1 ; latPts[3].y = y2 ; latPts[3].z = z3 ;
	      latPts[4].x = x1 ; latPts[4].y = y1 ; latPts[4].z = z1 ;
/*
**        Set Load Flag
*/
          loadFlag = DTMFenceOption::Inside ;
/*
**        Check Fence
*/
          if( useFence == TRUE )
            {         
	         bcdtmMath_getStringMaxMinP3D(1,latPts,4,&xLatMin,&xLatMax,&yLatMin,&yLatMax,&zLatMin,&zLatMax) ;
	         if     ( xLatMax < clipDtmP->xMin || xLatMin > clipDtmP->xMax ) loadFlag = DTMFenceOption::None ;
             else if (yLatMax < clipDtmP->yMin || yLatMin > clipDtmP->xMax) loadFlag = DTMFenceOption::None;
             else
               {
                if( bcdtmFind_triangleDtmObject(clipDtmP,x1,y1,&fndType,&trgPnt1,&trgPnt2,&trgPnt3)) goto errexit ;
                if( ! fndType )
                  {
                   if( bcdtmFind_triangleDtmObject(clipDtmP,x1,y2,&fndType,&trgPnt1,&trgPnt2,&trgPnt3)) goto errexit ;
                   if( ! fndType )
                     { 
                      if( bcdtmFind_triangleDtmObject(clipDtmP,x2,y1,&fndType,&trgPnt1,&trgPnt2,&trgPnt3)) goto errexit ;
                      if( ! fndType )
                        { 
                         if( bcdtmFind_triangleDtmObject(clipDtmP,x2,y2,&fndType,&trgPnt1,&trgPnt2,&trgPnt3)) goto errexit ;
                         if( ! fndType )
                           { 
                            if( bcdtmLoad_testForOverlapWithTinHullDtmObject(clipDtmP,latPts,5,&loadFlag)) goto errexit ; 
                           }
                        }
                     }
                  } 
               }
            } 
/*
**        Get Theme Index
*/
          if( loadFlag == DTMFenceOption::Inside)
            {
	         bcdtmMath_getLatticeAttributes(latPts,&slopeDegrees,&slopePercent,&aspect,&height) ;
             themeIndex = DTM_NULL_PNT ;
             for( themeP = themeRangesP ; themeP < themeRangesP + numThemeRanges && themeIndex == DTM_NULL_PNT ; ++themeP )
               {
                switch ( themeOption )
                  {
                   case  1 :
                     if( height >= themeP->low && height < themeP->high ) themeIndex = (long) ( themeP-themeRangesP) ;
                   break   ; 

                   case  2 :
                     if( slopePercent >= themeP->low && slopePercent < themeP->high ) themeIndex = (long) ( themeP-themeRangesP) ;
                   break   ; 

                   case  3 :
                     if( slopeDegrees >= themeP->low && slopeDegrees < themeP->high ) themeIndex = (long) ( themeP-themeRangesP) ;
                   break   ; 

                   case  4 :
                     if( aspect >= themeP->low && aspect < themeP->high ) themeIndex = (long) ( themeP-themeRangesP) ;
                   break   ; 
                  }
               } 
/*
**           Load Theme
*/
             if( themeIndex != DTM_NULL_PNT )
               { 
                if( bcdtmLoad_callUserLoadFunction(loadFunctionP,DTMFeatureType::Theme,themeIndex,DTM_NULL_FEATURE_ID,latPts,5,userP)) goto errexit ;
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
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Lattice Cell Themes From Lattice Object Completed")  ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Lattice Cell Themes From Lattice Object Error")  ;
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
BENTLEYDTM_Private int bcdtmTheme_loadPolygonThemesFromLatticeObject
(
 DTM_LAT_OBJ *latticeP,           /* ==> Pointer To Lattice Object                    */
 long        themeOption,         /* ==> Theme Option                                 */
 DRange1d      *themeRangesP,       /* ==> Pointer To Theme Ranges                      */
 long        numThemeRanges,      /* ==> Number Of Theme Ranges                       */
 DTMFeatureCallback loadFunctionP,  /* ==> Pointer To Load Function                     */
 long        useFence,            /* ==> Load Feature Within Fence <TRUE,FALSE>       */
 BC_DTM_OBJ  *clipDtmP,           /* ==> Pointer To DTM Object Clipping Tin           */
 void        *userP               /* ==> User Pointer Passed Back To User             */ 
)
/*
** This Function Load Polygon Themes From A Lattice Object
*/
{
 int      ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long     i,j,ofs,nxl,nyl,ofs1,ofs2,process,themeIndex ;
 DTMFenceOption loadFlag;
 long     i1=0,i2,j1=0,j2,dirn=0,si=0,sj=0 ;
 long     fndType,trgPnt1,trgPnt2,trgPnt3 ;
 long     numFeaturePts=0,memFeaturePts=0,memFeaturePtsInc=10000 ;
 double   height=0.0,slopePercent=0.0,slopeDegrees=0.0,aspect=0.0 ;
 double   x1,y1,x2,y2,z1,z2,z3,z4,sx,sy,xLatMin,yLatMin,xLatMax,yLatMax,zLatMin,zLatMax ;
 DPoint3d      latPts[5],*featurePtsP=NULL ;
 char     startValue=0,nullValue=(char)-1/*255*/,*charP,*latThemeP=NULL ;
 DRange1d   *themeP ;
/*
** Write Entry Message
*/
 if( dbg )
   { 
    bcdtmWrite_message(0,0,0,"Loading Lattice Polygon Themes From Lattice Object") ;
    bcdtmWrite_message(0,0,0,"latticeP        = %p",latticeP) ;
    bcdtmWrite_message(0,0,0,"themeOption     = %8ld",themeOption) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP   = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"useFence        = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"clipDtmP        = %p",clipDtmP) ; 
    bcdtmWrite_message(0,0,0,"userP           = %p",userP) ;
    bcdtmWrite_message(0,0,0,"themeRangesP    = %8ld",themeRangesP) ;
    bcdtmWrite_message(0,0,0,"numThemeRanges  = %8ld",numThemeRanges) ;
    for( themeP = themeRangesP ; themeP < themeRangesP + numThemeRanges ; ++themeP )
       {
        bcdtmWrite_message(0,0,0,"Range[%4ld] >= %10.4lf < %10.4lf",(long)(themeP-themeRangesP),themeP->low,themeP->high) ;
       }
   }
/*
** Initialise
*/
 nxl = latticeP->NXL - 1 ;
 nyl = latticeP->NYL - 1 ;  
/*
** Allocate Memory For latThemeP Themes
*/
 latThemeP = ( char * ) malloc ( nxl*nyl*sizeof(char)) ;
 if( latThemeP == NULL ) { bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ; goto errexit ; }
 for( charP = latThemeP ; charP < latThemeP + nxl*nyl ; ++charP ) *charP = nullValue ;
/*
** Scan Lattice
*/
/*
** Scan Lattice Cells
*/
 for( i = 0 ; i < latticeP->NYL - 1 ; ++i )
   {
    for( j = 0 ; j < latticeP->NXL - 1 ; ++j )
      {
       z1 = *(latticeP->LAT + j*latticeP->NYL + i)     ; z2 = *(latticeP->LAT + j*(latticeP->NYL) + i + 1) ;
       z3 = *(latticeP->LAT + (j+1)*latticeP->NYL + i) ; z4 = *(latticeP->LAT + (j+1)*latticeP->NYL + i + 1) ;
       if( z1 != latticeP->NULLVAL && z2 != latticeP->NULLVAL && z3 != latticeP->NULLVAL && z4 != latticeP->NULLVAL )
         {
          x1 = latticeP->DX * i + latticeP->LXMIN ; 
          x2 = x1 + latticeP->DX ;
          y1 = latticeP->DY * j + latticeP->LYMIN;
          y2 = y1 + latticeP->DY ;
          latPts[0].x = x1 ; latPts[0].y = y1 ; latPts[0].z = z1 ;
          latPts[1].x = x2 ; latPts[1].y = y1 ; latPts[1].z = z2 ;
	      latPts[2].x = x2 ; latPts[2].y = y2 ; latPts[2].z = z4 ;
	      latPts[3].x = x1 ; latPts[3].y = y2 ; latPts[3].z = z3 ;
	      latPts[4].x = x1 ; latPts[4].y = y1 ; latPts[4].z = z1 ;
/*
**        Set Load Flag
*/
          loadFlag = DTMFenceOption::Inside;
/*
**        Check Fence
*/
          if( useFence == TRUE )
            {         
	         bcdtmMath_getStringMaxMinP3D(1,latPts,4,&xLatMin,&xLatMax,&yLatMin,&yLatMax,&zLatMin,&zLatMax) ;
             if (xLatMax < clipDtmP->xMin || xLatMin > clipDtmP->xMax) loadFlag = DTMFenceOption::None;
             else if (yLatMax < clipDtmP->yMin || yLatMin > clipDtmP->xMax) loadFlag = DTMFenceOption::None;
             else
               {
                if( bcdtmFind_triangleDtmObject(clipDtmP,x1,y1,&fndType,&trgPnt1,&trgPnt2,&trgPnt3)) goto errexit ;
                if( ! fndType )
                  {
                   if( bcdtmFind_triangleDtmObject(clipDtmP,x1,y2,&fndType,&trgPnt1,&trgPnt2,&trgPnt3)) goto errexit ;
                   if( ! fndType )
                     { 
                      if( bcdtmFind_triangleDtmObject(clipDtmP,x2,y1,&fndType,&trgPnt1,&trgPnt2,&trgPnt3)) goto errexit ;
                      if( ! fndType )
                        { 
                         if( bcdtmFind_triangleDtmObject(clipDtmP,x2,y2,&fndType,&trgPnt1,&trgPnt2,&trgPnt3)) goto errexit ;
                         if( ! fndType )
                           { 
                            if( bcdtmLoad_testForOverlapWithTinHullDtmObject(clipDtmP,latPts,5,&loadFlag)) goto errexit ; 
                           }
                        }
                     }
                  } 
               }
            } 
/*
**        Get Theme Index
*/
          if( loadFlag == DTMFenceOption::Inside )
            {
	         bcdtmMath_getLatticeAttributes(latPts,&slopeDegrees,&slopePercent,&aspect,&height) ;
             themeIndex = DTM_NULL_PNT ;
             for( themeP = themeRangesP ; themeP < themeRangesP + numThemeRanges && themeIndex == DTM_NULL_PNT ; ++themeP )
               {
                switch ( themeOption )
                  {
                   case  1 :
                     if( height >= themeP->low && height < themeP->high ) themeIndex = (long) ( themeP-themeRangesP) ;
                   break   ; 

                   case  2 :
                     if( slopePercent >= themeP->low && slopePercent < themeP->high ) themeIndex = (long) ( themeP-themeRangesP) ;
                   break   ; 

                   case  3 :
                     if( slopeDegrees >= themeP->low && slopeDegrees < themeP->high ) themeIndex = (long) ( themeP-themeRangesP) ;
                   break   ; 

                   case  4 :
                     if( aspect >= themeP->low && aspect < themeP->high ) themeIndex = (long) ( themeP-themeRangesP) ;
                   break   ; 
                  }
               } 
/*
**           Set Theme Index
*/
             if( themeIndex != DTM_NULL_PNT )
               { 
                ofs = j * (latticeP->NYL-1) + i ;
                *(latThemeP+ofs) = (char) themeIndex ;
               } 
            }
         }
      }
   }
/*
** Extract Lattice Theme Polygons
*/
 for( j = 0 ; j < nxl ; ++j )
   {
    for( i = 0 ; i < nyl - 1 ; ++i )
      {
       ofs1 = j * nyl + i  ; ofs2 = ofs1 + 1 ;
       if( *(latThemeP+ofs1) != *(latThemeP+ofs2) )
         {
	      if( *(latThemeP+ofs2) != nullValue )
	        {
	         dirn = 1 ;
	         i1 = i + 1 ; j1 = j ; 
	         si = i1    ; sj = j1 ;
	         startValue = *(latThemeP+ofs2) ; 
	        }

	      else if( *(latThemeP+ofs1) != nullValue )
	        {
	         dirn = 3 ;
	         i1 = i     ; j1 = j ; 
	         si = i1    ; sj = j1 ;
	         startValue = *(latThemeP+ofs1) ; 
	        }

	      bcdtmTheme_getLatticeLineCoordinatesLatticeObject(latticeP,i1,j1,dirn,&x1,&y1,&z1,&x2,&y2,&z2) ;
          if( bcdtmLoad_storeFeaturePoint(x1,y1,z1,&featurePtsP,&numFeaturePts,&memFeaturePts,memFeaturePtsInc)) goto errexit ;
          if( bcdtmLoad_storeFeaturePoint(x2,y2,z2,&featurePtsP,&numFeaturePts,&memFeaturePts,memFeaturePtsInc)) goto errexit ;
          sx = x1 ; sy = y1 ;
	      
          do
	        {
	         process = 1 ;
	         while ( process )
	           {
	            process = 0 ;
	            if( bcdtmTheme_getNextCellClk(i1,j1,&dirn,nxl,nyl,&i2,&j2) )
	              {
	               ofs2 = j2 * nyl + i2 ;
	               if( startValue == *(latThemeP+ofs2) )
	                 {
		              --dirn ; if( dirn == 0 ) dirn = 4 ;
		              --dirn ; if( dirn == 0 ) dirn = 4 ;
		              i1 = i2 ; j1 = j2 ;
		              process = 1 ;
		             }
	              }
	           } 
	         bcdtmTheme_getLatticeLineCoordinatesLatticeObject(latticeP,i1,j1,dirn,&x1,&y1,&z1,&x2,&y2,&z2) ;
             if( bcdtmLoad_storeFeaturePoint(x2,y2,z2,&featurePtsP,&numFeaturePts,&memFeaturePts,memFeaturePtsInc)) goto errexit ;
	        } while ( sx != x2 || sy != y2 ) ;
/*
**        Load Theme Polygon
*/
          if( bcdtmLoad_callUserLoadFunction(loadFunctionP,DTMFeatureType::Theme,startValue,DTM_NULL_FEATURE_ID,featurePtsP,numFeaturePts,userP)) goto errexit ;
          numFeaturePts = 0 ;
/*
**        Null Theme Polygon
*/
          bcdtmTheme_nullThemePolygonLatticeObject(latticeP,latThemeP,si,sj,startValue,nullValue,nxl,nyl) ;
	     }
      }
   }       
/*
** Clean Up
*/
 cleanup :
 if( latThemeP   != NULL ) free(latThemeP) ;
 if( featurePtsP != NULL ) free(featurePtsP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Lattice Polygon Themes From Lattice Object Completed")  ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Lattice Polygon Themes From Lattice Object Error")  ;
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
BENTLEYDTM_Private int bcdtmTheme_getNextCellClk(long I1,long J1,long *Dirn,long Nxl,long Nyl,long *I2,long *J2)
/*
** These Function Gets The Next Cell In A Clockwise Direction
*/
{
 int  ret=DTM_SUCCESS ;
/*
** Increment Direction
*/
 ++*Dirn ;
 if( *Dirn > 4 ) *Dirn = 1 ;
/*
** Get Next Cell Depending On Direction
*/
 switch( *Dirn )
   {
    case 1 : /* Get Left Cell */
      *I2 = I1 ; *J2 = J1 ;
      if( I1 > 0 ) { --*I2 ; goto errexit ; }
      else         return(0) ;
    break  ;   

    case 2 : /* Get Bottom Cell */
      *I2 = I1 ; *J2 = J1 ;
      if( J1 > 0 ) { --*J2 ; goto errexit ; }
      else         return(0) ;
    break  ;   

     case 3 : /* Get Right Cell */
      *I2 = I1 ; *J2 = J1 ;
      if( I1 < Nyl - 1 ) { ++*I2 ; goto errexit ; }
      else         return(0) ;
    break  ;   

     case 4 : /* Get Top Cell */
      *I2 = I1 ; *J2 = J1 ;
      if( J1 < Nxl - 1 ) { ++*J2 ; goto errexit ; }
      else         return(0) ;
    break  ;   
   } ;
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
BENTLEYDTM_Private int bcdtmTheme_getLatticeLineCoordinatesLatticeObject(DTM_LAT_OBJ *latticeP,long I,long J,long Dirn,double *X1,double *Y1,double *Z1,double *X2,double *Y2,double *Z2)
/*
** These Function Gets The Next Cell In A Clockwise Direction
*/
{
 long ofs1,ofs2 ;
/*
** Get Next Cell Depending On Direction
*/
 switch( Dirn )
   {
    case 1 : /* Get Left Line */
    *X1 = *X2 = latticeP->LXMIN + I * latticeP->DX ;
    *Y1 = latticeP->LYMIN + (J+1) * latticeP->DY   ;
    *Y2 = latticeP->LYMIN + J * latticeP->DY ; 
    ofs1 = (J+1) * latticeP->NYL + I ;
    ofs2 =  J    * latticeP->NYL + I ;
    *Z1 = *(latticeP->LAT+ofs1)  ;
    *Z2 = *(latticeP->LAT+ofs2)   ;
    break  ;   

    case 2 : /* Bottom Line */
    *Y1 = *Y2 = latticeP->LYMIN + J * latticeP->DY ;
    *X1 = latticeP->LXMIN + I * latticeP->DX   ;
    *X2 = latticeP->LXMIN + (I+1) * latticeP->DX ; 
    ofs1 =  J * latticeP->NYL + I ;
    ofs2 =  J * latticeP->NYL + I + 1 ;
    *Z1 = *(latticeP->LAT+ofs1)   ;
    *Z2 = *(latticeP->LAT+ofs2)   ;
    break  ;   

    case 3 : /* Right Line */
    *X1 = *X2 = latticeP->LXMIN + (I+1) * latticeP->DX ;
    *Y1 = latticeP->LYMIN + J * latticeP->DY   ;
    *Y2 = latticeP->LYMIN + (J+1) * latticeP->DY ; 
    ofs1 =  J     * latticeP->NYL + I + 1 ;
    ofs2 =  (J+1) * latticeP->NYL + I + 1 ;
    *Z1 = *(latticeP->LAT+ofs1)   ;
    *Z2 = *(latticeP->LAT+ofs2)   ;
    break  ;   

    case 4 : /* Top Line */
    *Y1 = *Y2 = latticeP->LYMIN + (J+1) * latticeP->DY ;
    *X1 = latticeP->LXMIN + (I+1) * latticeP->DX   ;
    *X2 = latticeP->LXMIN +  I * latticeP->DX ; 
    ofs1 =  (J+1) * latticeP->NYL + I + 1 ;
    ofs2 =  (J+1) * latticeP->NYL + I     ;
    *Z1 = *(latticeP->LAT+ofs1)   ;
    *Z2 = *(latticeP->LAT+ofs2)   ;
    break  ;   
   } ;
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
BENTLEYDTM_Public int bcdtmTheme_nullThemePolygonLatticeObject(DTM_LAT_OBJ *latticeP,char *latThemeP ,long I,long J,char Cv,char Nv,long Nxl,long Nyl )
/*
** This Function Fills A Theme Polygon With Null Values
*/
{
 long i,j,si,sj,ni,nj,ofs,imin,imax,jmin,jmax,dirn,cont,process ;
 char markValue=(char)-2/*254*/ ;
/*
**  Null Current Cell
*/
 if( Cv == Nv ) return(0) ;
/*
** Scan To Left Edge Of Polygon
*/
 i = I ;
 process = 1 ;
 while( process )
   {
    if(i-1 >= 0 )
     {
      ofs = J * latticeP->NYL + i - 1 ;
      if( *(latThemeP+ofs) == Cv ) i = i - 1 ;
      else                       process = 0 ; 
     }
    else process = 0 ;
   } 
 si = i ; 
 sj = j = J ;
 imin = imax = si ; 
 jmin = jmax = sj ;
/*
** Mark Cells On Internal Edge Of Polygon
*/
 dirn = 1 ;
 ofs  = j * Nyl + i ;
 *(latThemeP+ofs) = markValue ;
 process = 1 ;
 while ( process )
   {
    if( bcdtmTheme_getNextCellClk(i,j,&dirn,Nxl,Nyl,&ni,&nj) )
      {
       ofs = nj * Nyl + ni ;
       if( *(latThemeP+ofs) == Cv || *(latThemeP+ofs) == markValue )
         {
          *(latThemeP+ofs) = markValue ;
          --dirn ; if( dirn == 0 ) dirn = 4 ;
          --dirn ; if( dirn == 0 ) dirn = 4 ;
          i = ni ; j = nj ;
         }
      }
    if( i < imin ) imin = i ; if( i > imax ) imax = i ;
    if( j < jmin ) jmin = j ; if( j > jmax ) jmax = j ;
    if( i == si && j == sj && dirn == 1 ) process = 0 ;
   } 
/*
**
**  Set Cells Left To Right Between Mark Values To Mark Values
**
*/
 for ( j = jmin ; j <= jmax ; ++j )
   {
    i = imin ;
    process = 1 ;
    while ( process ) 
      {
/*
**  Scan To Mark
*/
       cont = 1 ;
       while ( i <= imax && cont )
         {
          ofs = j * Nyl + i ;
          if( *(latThemeP+ofs) == markValue ) cont = 0 ;
          else                       ++i ;
         } 
/*
** Scan And Set Mark Values
*/
       cont = 1 ;               
       while ( i <= imax && cont )
         {
          ofs = j * Nyl + i ;
          if( *(latThemeP+ofs) == markValue || *(latThemeP+ofs) == Cv )
            { *(latThemeP+ofs) = markValue ; ++i ; } 
          else cont = 0 ;
         } 
/*
** Test For End Of Row
*/
       if( i > imax ) process = 0 ;
      }
   }
/*
**
**  Set Cells Right To Left Between Mark Values To Mark Values
**
*/
 for ( j = jmin ; j <= jmax ; ++j )
   {
    i = imax ;
    process = 1 ;
    while ( process ) 
      {
/*
**  Scan To Mark
*/
       cont = 1 ;
       while ( i >= imin && cont )
         {
          ofs = j * Nyl + i ;
          if( *(latThemeP+ofs) == markValue ) cont = 0 ;
          else                       --i ;
         } 
/*
** Scan And Set Mark Values
*/
       cont = 1 ;               
       while ( i >= imin && cont )
         {
          ofs = j * Nyl + i ;
          if( *(latThemeP+ofs) == markValue || *(latThemeP+ofs) == Cv )
            { *(latThemeP+ofs) = markValue ; --i ; } 
          else cont = 0 ;
         } 
/*
** Test For End Of Row
*/
       if( i < imin ) process = 0 ;
      }
   }
/*
**
** Scan And Set All Marked Values To Null
**
*/
 for ( j = jmin ; j <= jmax ; ++j )
   {
    for ( i = imin ; i <= imax ; ++i )
      {
       ofs = j * Nyl + i ;
       if( *(latThemeP+ofs) == markValue ) *(latThemeP+ofs) = Nv ;
      }
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
BENTLEYDTM_EXPORT int bcdtmTheme_loadHillShadeThemeFromDtmObject
(
 BC_DTM_OBJ *dtmP,               /* ==> Pointer To Dtm Object                        */
 long       greyScaleRange,      /* ==> Grey Scale Range                             */ 
 double     altitudeDegrees,     /* ==> Altitude In Degrees                          */
 double     azimuthDegrees,      /* ==> Azimuth In Degrees                           */
 long       useFence,            /* ==> Load Feature Within Fence <TRUE,FALSE>       */
 DPoint3d        *fencePtsP,          /* ==> Pointer To Fence Points                      */
 long       numFencePts,         /* ==> Number Of Fence Points                       */
 DTMFeatureCallback loadFunctionP,  /* ==> Pointer To Load Function                     */
 void       *userP               /* ==> User Pointer Passed Back To User             */ 
)
/*
** This Function Loads A Hill Shade Theme From A Dtm Object
** Standard Setting For Azimuth  = 315 degrees ( North West )
** Standard Setting For Altitude =  45 degrees 
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0)   ;
 BC_DTM_OBJ *cloneDtmP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   { 
    bcdtmWrite_message(0,0,0,"Loading Hill Shade Theme From Dtm Object") ;
    bcdtmWrite_message(0,0,0,"dtmP             = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"greyScaleRange   = %8ld",greyScaleRange) ;
    bcdtmWrite_message(0,0,0,"altitudeDegrees  = %8ld",altitudeDegrees) ;
    bcdtmWrite_message(0,0,0,"azimuthDegrees   = %p",azimuthDegrees) ;
    bcdtmWrite_message(0,0,0,"useFence        = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fencePtsP       = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts     = %8ld",numFencePts) ;
    bcdtmWrite_message(0,0,0,"userP           = %p",userP) ;
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
    bcdtmWrite_message(2,0,0,"Method Requires Triangulated DTM") ;
    goto errexit ;
   }
/*
** Create Clipping Tin
*/
 if     ( fencePtsP == NULL || numFencePts < 4 ) useFence = FALSE ;
 else if( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y ) useFence = FALSE ;
 if( useFence == TRUE )
   {
    if( bcdtmClip_cloneAndClipToPolygonDtmObject(dtmP,&cloneDtmP,fencePtsP,numFencePts,DTMClipOption::External)) goto errexit ;
   }
 else  cloneDtmP = dtmP ;
/*
** Load Hill Shade For Triangles
*/
 if( bcdtmTheme_loadTriangleHillShadeThemeFromDtmObject(cloneDtmP,greyScaleRange,altitudeDegrees,azimuthDegrees,loadFunctionP,userP)) goto errexit ; 
/*
** Clean Up
*/
 cleanup :
 if( cloneDtmP != NULL && cloneDtmP != dtmP ) bcdtmObject_destroyDtmObject(&cloneDtmP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Hill Shade Theme From Dtm Object Completed")  ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Hill Shade Theme From Dtm ObjectError")  ;
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
BENTLEYDTM_Private int bcdtmTheme_loadTriangleHillShadeThemeFromDtmObject
(
 BC_DTM_OBJ *dtmP,
 long       greyScaleRange,
 double     altitudeDegrees,
 double     azimuthDegrees,
 DTMFeatureCallback loadFunctionP,
 void       *userP
)
/*
**  Load Each Triangle Theme
*/
{
 int      ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long     p1,p2,p3,clPtr,loadFlag,voidTriangle,voidsInDtm,greyScale ;
 double   height=0.0,slopePercent=0.0,slopeDegrees=0.0,aspect=0.0,reflectance=0.0 ;
 double   azRad,altRad,slope ;
 DPoint3d      trgPts[4] ;
 DTM_TIN_NODE  *nodeP ;
 DTM_TIN_POINT *pnt1P,*pnt2P,*pnt3P ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Loading Triangle Hill Shade Theme") ;
/*
** Convert Azimuth And Altitude To Degrees
*/
 azRad  = azimuthDegrees  * DTM_PYE / 180.0 ;
 altRad = altitudeDegrees * DTM_PYE / 180.0 ;
/*
**  Check For Voids In Dtm
*/
 bcdtmList_testForVoidsInDtmObject(dtmP,&voidsInDtm) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"voidsInDtm = %2ld",voidsInDtm) ;
/*
** Scan All Tin Points
*/
 for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
   {
    clPtr = nodeAddrP(dtmP,p1)->cPtr ;
    if( clPtr != dtmP->nullPtr )
      {
       pnt1P = pointAddrP(dtmP,p1) ;
       nodeP = nodeAddrP(dtmP,p1)  ;
       if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit ;
       pnt2P = pointAddrP(dtmP,p2) ;
       while( clPtr != dtmP->nullPtr )
         {
          p3    = clistAddrP(dtmP,clPtr)->pntNum ;
          clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
          pnt3P = pointAddrP(dtmP,p3) ;
          if( p2 > p1 && p3 > p1 ) 
            {
             if( nodeP->hPtr != p2 ) 
               {
                loadFlag = TRUE ;
/*
**              Check For Void Triangle
*/
                if( loadFlag == TRUE && voidsInDtm == TRUE ) 
                  { 
                   if( bcdtmList_testForVoidTriangleDtmObject(dtmP,p1,p2,p3,&voidTriangle)) goto errexit ;
                   if( voidTriangle) loadFlag = FALSE ;
                  }
/*
**              Load Triangle Hill Shade Theme
*/
                if( loadFlag == TRUE ) 
                  {
                   bcdtmMath_getTriangleAttributesDtmObject(dtmP,p1,p2,p3,&slopeDegrees,&slopePercent,&aspect,&height) ;
                   slope = slopePercent / 100.0 ;
                   aspect = aspect * DTM_PYE / 180.0 ;
                   reflectance = sin(altRad)*sin(slope) + cos(altRad)*cos(slope)*cos(-azRad - aspect - DTM_PYE / 2.0 ) ;
                   if( reflectance < 0.0 ) reflectance = 0.0 ;
                   greyScale = (long)(( double) greyScaleRange * reflectance ) ;
                   trgPts[0].x = pnt1P->x ; trgPts[0].y = pnt1P->y ; trgPts[0].z = pnt1P->z ; 
                   trgPts[1].x = pnt2P->x ; trgPts[1].y = pnt2P->y ; trgPts[1].z = pnt2P->z ;  
                   trgPts[2].x = pnt3P->x ; trgPts[2].y = pnt3P->y ; trgPts[2].z = pnt3P->z ;  
                   trgPts[3].x = pnt1P->x ; trgPts[3].y = pnt1P->y ; trgPts[3].z = pnt1P->z ; 
                   if( bcdtmLoad_callUserLoadFunction(loadFunctionP,DTMFeatureType::Theme,greyScale,dtmP->nullFeatureId,trgPts,4,userP)) goto errexit ;
                  }
               }
            } 
/*
**        Reset For Next Triangle
*/
          p2 = p3 ;
          pnt2P = pnt3P ;
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
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Triangle Theme From Dtm Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Triangle Theme From Dtm Object Error") ;
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
BENTLEYDTM_EXPORT int bcdtmTheme_loadHillShadeThemeFromLatticeObject
(
 DTM_LAT_OBJ *latticeP,          /* ==> Pointer To Lattice Object                    */
 long       greyScaleRange,      /* ==> Grey Scale Range                             */ 
 double     altitudeDegrees,     /* ==> Altitude In Degrees                          */
 double     azimuthDegrees,      /* ==> Azimuth In Degrees                           */
 long       useFence,            /* ==> Load Feature Within Fence <TRUE,FALSE>       */
 DPoint3d        *fencePtsP,          /* ==> Pointer To Fence Points                      */
 long       numFencePts,         /* ==> Number Of Fence Points                       */
 DTMFeatureCallback loadFunctionP,  /* ==> Pointer To Load Function                     */
 void       *userP               /* ==> User Pointer Passed Back To User             */ 
)
{
/*
** This Function Loads A Hill Shade Theme From A Lattice Object
** Standard Setting For Azimuth  = 315 degrees ( North West )
** Standard Setting For Altitude =  45 degrees 
*/
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0)  ;
 BC_DTM_OBJ *clipDtmP=NULL ;
/*
** Write Entry Message
*/
 if( dbg )
   { 
    bcdtmWrite_message(0,0,0,"Loading Hill Shade Theme From Lattice") ;
    bcdtmWrite_message(0,0,0,"latticeP         = %p",latticeP) ;
    bcdtmWrite_message(0,0,0,"greyScaleRange   = %8ld",greyScaleRange) ;
    bcdtmWrite_message(0,0,0,"altitudeDegrees  = %8ld",altitudeDegrees) ;
    bcdtmWrite_message(0,0,0,"azimuthDegrees   = %p",azimuthDegrees) ;
    bcdtmWrite_message(0,0,0,"useFence         = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"fencePtsP        = %p",fencePtsP) ;
    bcdtmWrite_message(0,0,0,"numFencePts      = %8ld",numFencePts) ;
    bcdtmWrite_message(0,0,0,"userP            = %p",userP) ;
   }
/*
** Test For Valid Lattice Object
*/
 if( bcdtmObject_testForValidLatticeObject(latticeP)) goto errexit ;
/*
** Create Clipping Tin
*/
 if( useFence == TRUE )
   {
    if     ( fencePtsP == NULL || numFencePts < 4 ) useFence = FALSE ;
    else if( fencePtsP->x != (fencePtsP+numFencePts-1)->x || fencePtsP->y != (fencePtsP+numFencePts-1)->y ) useFence = FALSE ;
    else if( bcdtmClip_buildClippingTinFromFencePointsDtmObject(&clipDtmP,fencePtsP,numFencePts) ) goto errexit ;
   }
/*
** Load Lattice Hill Shade Cell Theme
*/
 if( bcdtmTheme_loadCellHillShadeFromLatticeObject(latticeP,greyScaleRange,altitudeDegrees,azimuthDegrees,loadFunctionP,useFence,clipDtmP,userP) ) goto errexit ;
/*
** Clean Up
*/
 cleanup :
 if( clipDtmP != NULL ) free(clipDtmP) ;
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Hill Shade Theme From Lattice Completed")  ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Hill Shade Theme From Lattice Error")  ;
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
BENTLEYDTM_Private int  bcdtmTheme_loadCellHillShadeFromLatticeObject
(
 DTM_LAT_OBJ *latticeP,           /* ==> Pointer To Lattice Object                    */
 long        greyScaleRange,      /* ==> Grey Scale Range                             */ 
 double      altitudeDegrees,     /* ==> Altitude In Degrees                          */
 double      azimuthDegrees,      /* ==> Azimuth In Degrees                           */
 DTMFeatureCallback loadFunctionP,  /* ==> Pointer To Load Function                     */
 long        useFence,            /* ==> Load Feature Within Fence <TRUE,FALSE>       */
 BC_DTM_OBJ  *clipDtmP,           /* ==> Pointer To DTM Object Clipping Tin           */
 void        *userP               /* ==> User Pointer Passed Back To User             */ 
)
/*
** This Function Load Cell Lattice Hill Shade Themes
*/
{
 int      ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long     i,j,greyScale ;
 DTMFenceOption loadFlag;
 long     fndType,trgPnt1,trgPnt2,trgPnt3 ;
 float    z1,z2,z3,z4 ;
 double   height=0.0,slopePercent=0.0,slopeDegrees=0.0,aspect=0.0 ;
 double   x1,y1,x2,y2,xLatMin,yLatMin,xLatMax,yLatMax,zLatMin,zLatMax ;
 double   azRad,altRad,slope,reflectance ;
 DPoint3d      latPts[5] ;
/*
** Write Entry Message
*/
 if( dbg )
   { 
    bcdtmWrite_message(0,0,0,"Loading Lattice Cell Hill Shade Theme") ;
    bcdtmWrite_message(0,0,0,"latticeP         = %p",latticeP) ;
    bcdtmWrite_message(0,0,0,"greyScaleRange   = %8ld",greyScaleRange) ;
    bcdtmWrite_message(0,0,0,"altitudeDegrees  = %8ld",altitudeDegrees) ;
    bcdtmWrite_message(0,0,0,"azimuthDegrees   = %p",azimuthDegrees) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP    = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"useFence         = %8ld",useFence) ;
    bcdtmWrite_message(0,0,0,"clipDtmP         = %p",clipDtmP) ; 
    bcdtmWrite_message(0,0,0,"userP            = %p",userP) ;
   }
/*
** Convert Azimuth And Altitude To Degrees
*/
 azRad  = azimuthDegrees  * DTM_PYE / 180.0 ;
 altRad = altitudeDegrees * DTM_PYE / 180.0 ;
/*
** Scan Lattice Cells
*/
 for( i = 0 ; i < latticeP->NYL - 1 ; ++i )
   {
    for( j = 0 ; j < latticeP->NXL - 1 ; ++j )
      {
       z1 = *(latticeP->LAT + j*latticeP->NYL + i)     ; z2 = *(latticeP->LAT + j*(latticeP->NYL) + i + 1) ;
       z3 = *(latticeP->LAT + (j+1)*latticeP->NYL + i) ; z4 = *(latticeP->LAT + (j+1)*latticeP->NYL + i + 1) ;
       if( z1 != latticeP->NULLVAL && z2 != latticeP->NULLVAL && z3 != latticeP->NULLVAL && z4 != latticeP->NULLVAL )
         {
          x1 = latticeP->DX * i + latticeP->LXMIN ; 
          x2 = x1 + latticeP->DX ;
          y1 = latticeP->DY * j + latticeP->LYMIN;
          y2 = y1 + latticeP->DY ;
          latPts[0].x = x1 ; latPts[0].y = y1 ; latPts[0].z = z1 ;
          latPts[1].x = x2 ; latPts[1].y = y1 ; latPts[1].z = z2 ;
	      latPts[2].x = x2 ; latPts[2].y = y2 ; latPts[2].z = z4 ;
	      latPts[3].x = x1 ; latPts[3].y = y2 ; latPts[3].z = z3 ;
	      latPts[4].x = x1 ; latPts[4].y = y1 ; latPts[4].z = z1 ;
/*
**        Set Load Flag
*/
          loadFlag = DTMFenceOption::Inside;
/*
**        Check Fence
*/
          if( useFence == TRUE )
            {         
	         bcdtmMath_getStringMaxMinP3D(1,latPts,4,&xLatMin,&xLatMax,&yLatMin,&yLatMax,&zLatMin,&zLatMax) ;
             if (xLatMax < clipDtmP->xMin || xLatMin > clipDtmP->xMax) loadFlag = DTMFenceOption::None;
             else if (yLatMax < clipDtmP->yMin || yLatMin > clipDtmP->xMax) loadFlag = DTMFenceOption::None;
             else
               {
                if( bcdtmFind_triangleDtmObject(clipDtmP,x1,y1,&fndType,&trgPnt1,&trgPnt2,&trgPnt3)) goto errexit ;
                if( ! fndType )
                  {
                   if( bcdtmFind_triangleDtmObject(clipDtmP,x1,y2,&fndType,&trgPnt1,&trgPnt2,&trgPnt3)) goto errexit ;
                   if( ! fndType )
                     { 
                      if( bcdtmFind_triangleDtmObject(clipDtmP,x2,y1,&fndType,&trgPnt1,&trgPnt2,&trgPnt3)) goto errexit ;
                      if( ! fndType )
                        { 
                         if( bcdtmFind_triangleDtmObject(clipDtmP,x2,y2,&fndType,&trgPnt1,&trgPnt2,&trgPnt3)) goto errexit ;
                         if( ! fndType )
                           { 
                            if( bcdtmLoad_testForOverlapWithTinHullDtmObject(clipDtmP,latPts,5,&loadFlag)) goto errexit ; 
                           }
                        }
                     }
                  } 
               }
            } 
/*
**        Load Cell Hill Shade Value
*/
          if( loadFlag == DTMFenceOption::Inside)
            {
	         bcdtmMath_getLatticeAttributes(latPts,&slopeDegrees,&slopePercent,&aspect,&height) ;
             slope = slopePercent / 100.0 ;
             aspect = aspect * DTM_PYE / 180.0 ;
             reflectance = sin(altRad)*sin(slope) + cos(altRad)*cos(slope)*cos(-azRad - aspect - DTM_PYE / 2.0 ) ;
             if( reflectance < 0.0 ) reflectance = 0.0 ;
             greyScale = (long)(( double) greyScaleRange * reflectance ) ;
             if( bcdtmLoad_callUserLoadFunction(loadFunctionP,DTMFeatureType::Theme,greyScale,DTM_NULL_FEATURE_ID,latPts,5,userP)) goto errexit ;
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
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Lattice Cell Hill Shade Theme Completed")  ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading Lattice Cell Hill Shade Theme Error")  ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
   
