/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmEditDraw.cpp $
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
BENTLEYDTM_EXPORT int bcdtmEdit_drawPointFeaturesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long point,double contourInterval,void *userP )
/*
** This Function draws All The Lines Connected To A Point
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   p2, p3, clc;
 bool voidFlag;
 double x,y,z,SlopeD,SlopeP,Aspect,Height ;
/*
** Write Entry Mode
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Drawing Point Features ** Point = %8ld drawMode = %8ld",point,drawMode) ;
/*
** Initialise
*/
 if( point >= 0 && point < dtmP->numPoints )
   {
    if( ( clc = nodeAddrP(dtmP,point)->cPtr ) != dtmP->nullPtr )
      {
       if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,point,clistAddrP(dtmP,clc)->pntNum)) < 0 ) goto errexit ;
       while ( clc != dtmP->nullPtr )
         {
          p3  = clistAddrP(dtmP,clc)->pntNum ;
          clc = clistAddrP(dtmP,clc)->nextPtr ;
          if( nodeAddrP(dtmP,p3)->hPtr != point )
            {
             if( bcdtmList_testForVoidTriangleDtmObject(dtmP,point,p2,p3,voidFlag) ) goto errexit ;
             if( ! voidFlag )
               {
/*
**             Draw Triangle
*/
                if( dbg == 2 ) bcdtmWrite_message(0,0,0,"**** Drawing Triangle") ;
                if( drawMode != 2 && drawMode != 3 )
                  {
                   if( bcdtmEdit_drawTriangleDtmObject(dtmP,loadFunctionP,drawMode,point,p2,p3,userP ) ) goto errexit ;
                  }
                else
                  {
                   if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,point)->x,pointAddrP(dtmP,point)->y,pointAddrP(dtmP,point)->z)) goto errexit ;
                   if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,p3)->x,pointAddrP(dtmP,p3)->y,pointAddrP(dtmP,p3)->z)) goto errexit ;
                   if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Triangle,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
                  }
/*
**              Draw Contours
*/
                if( dbg == 2 ) bcdtmWrite_message(0,0,0,"**** Drawing Contours") ;
                if( bcdtmEdit_drawContoursForTriangleDtmObject(dtmP,loadFunctionP,drawMode,point,p2,p3,contourInterval,userP)) goto errexit ;
/*
**              Draw Flow Arrow
*/
                if( dbg == 2 ) bcdtmWrite_message(0,0,0,"**** Drawing Flow Arrow") ;
                x = ( pointAddrP(dtmP,point)->x + pointAddrP(dtmP,p2)->x + pointAddrP(dtmP,p3)->x ) / 3.0 ;
                y = ( pointAddrP(dtmP,point)->y + pointAddrP(dtmP,p2)->y + pointAddrP(dtmP,p3)->y ) / 3.0 ;
                z = ( pointAddrP(dtmP,point)->z + pointAddrP(dtmP,p2)->z + pointAddrP(dtmP,p3)->z ) / 3.0 ;
                bcdtmMath_getTriangleAttributesDtmObject(dtmP,point,p2,p3,&SlopeD,&SlopeP,&Aspect,&Height) ;
                if( bcdtmLoad_storePointInCache(x,y,z)) goto errexit ;
                if( bcdtmLoad_storePointInCache(SlopeD,SlopeP,Aspect)   ) goto errexit ;
                if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Theme,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
               }
/*
**           Draw DTM Features For Line p2-p3
*/
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"**** Drawing Triangle Base") ;
             if( bcdtmList_testForVoidLineDtmObject(dtmP,p2,p3,voidFlag) ) goto errexit ;
             if( ! voidFlag ) if( bcdtmEdit_drawDtmFeaturesForLineDtmObject(dtmP,loadFunctionP,drawMode,p2,p3,userP) ) goto errexit ;
            }
/*
**        Draw DTM Features For Line point-p2
*/
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"**** Drawing Triangle Edge") ;
          if( bcdtmList_testForVoidLineDtmObject(dtmP,point,p2,voidFlag) ) goto errexit ;
          if( ! voidFlag ) if( bcdtmEdit_drawDtmFeaturesForLineDtmObject(dtmP,loadFunctionP,drawMode,point,p2,userP) ) goto errexit ;
/*
**        Reset For Next Triangle Point
*/
          p2 = p3 ;
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
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Drawing Point Features ** Point = %8ld drawMode = %8ld Completed",point,drawMode) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Drawing Point Features ** Point = %8ld drawMode = %8ld Error",point,drawMode) ;
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
BENTLEYDTM_EXPORT int bcdtmEdit_drawTriangleDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long P1,long P2,long P3,void *userP)
/*
** This Function Draws A Triangle
*/
{
 if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z)) return(DTM_ERROR) ;
 if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z)) return(DTM_ERROR) ;
 if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,pointAddrP(dtmP,P3)->z)) return(DTM_ERROR) ;
 if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z)) return(DTM_ERROR) ;
 if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Triangle,drawMode,dtmP->nullFeatureId,userP) ) return(DTM_ERROR) ;
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
BENTLEYDTM_Private int bcdtmEdit_drawDtmFeaturesForLineDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long P1,long P2,void *userP )
/*
** This Function Draws Dtm Features For A Line
** Rob Cormack  July 2003
*/
{
 int   ret=DTM_SUCCESS ;
 long  numLineFeatures, featureType;
 DTMFeatureType dtmFeatureType;
 bvector<DTMTinPointFeatures> lineFeatures;
/*
** Get List Of DTM Features For Line
*/
 if( bcdtmList_getDtmFeaturesForLineDtmObject(dtmP,P1,P2, lineFeatures) ) goto errexit ;
 numLineFeatures = (long)lineFeatures.size();
/*
** Draw Hull Line
*/
 if(nodeAddrP(dtmP,P1)->hPtr == P2 || nodeAddrP(dtmP,P2)->hPtr == P1 )
   {
    if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z)) goto errexit ;
    if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z)) goto errexit ;
    if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Hull,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
   }
/*
** Scan Features And Draw DTM Features
*/
 if( numLineFeatures > 0 )
   {
    for( featureType = 0 ; featureType < 4 ; ++featureType )
      {
      switch (featureType)
          {
              case 0: dtmFeatureType = DTMFeatureType::Breakline; break;
              case 1: dtmFeatureType = DTMFeatureType::ContourLine; break;
              case 2: dtmFeatureType = DTMFeatureType::Void; break;
              case 3: dtmFeatureType = DTMFeatureType::Island; break;
              default:
                  dtmFeatureType = DTMFeatureType::None;
          }
       for(auto& plf : lineFeatures)
         {
          if( plf.dtmFeatureType == dtmFeatureType )
            {
             if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z)) goto errexit ;
             if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z)) goto errexit ;
             if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,dtmFeatureType,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
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
 return(0) ;
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
BENTLEYDTM_EXPORT int bcdtmEdit_drawTriangleFeaturesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long P1,long P2,long P3,double contourInterval,void *userP )
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   clc;
 bool voidFlag;
 double x,y,z,SlopeD,SlopeP,Aspect,Height ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Drawing Triangle Features") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP   = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"drawMode        = %8ld",drawMode) ;
    bcdtmWrite_message(0,0,0,"P1              = %8ld",P1) ;
    bcdtmWrite_message(0,0,0,"P2              = %8ld",P3) ;
    bcdtmWrite_message(0,0,0,"P2              = %8ld",P3) ;
    bcdtmWrite_message(0,0,0,"contourInterval = %8.3lf",contourInterval) ;
    bcdtmWrite_message(0,0,0,"userP           = %8ld",userP) ;
   }
/*
** Validate Triangle
*/
 if( nodeAddrP(dtmP,P1)->cPtr == dtmP->nullPtr ) goto cleanup ;
 if( nodeAddrP(dtmP,P2)->cPtr == dtmP->nullPtr ) goto cleanup ;
 if( nodeAddrP(dtmP,P3)->cPtr == dtmP->nullPtr ) goto cleanup ;
 if( ! bcdtmList_testLineDtmObject(dtmP,P1,P2))  goto cleanup ;
 if( ! bcdtmList_testLineDtmObject(dtmP,P2,P3))  goto cleanup ;
 if( ! bcdtmList_testLineDtmObject(dtmP,P3,P1))  goto cleanup ;
/*
** Test For Void Triangle
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Testing For Void Triangle") ;
 if( bcdtmList_testForVoidTriangleDtmObject(dtmP,P1,P2,P3,voidFlag)) goto errexit ;
 if( voidFlag )
   {
    if( bcdtmList_testForVoidHullLineDtmObject(dtmP,P1,P3) || bcdtmList_testForVoidHullLineDtmObject(dtmP,P1,P3))
      {
       if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z)) goto errexit ;
       if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,pointAddrP(dtmP,P3)->z)) goto errexit ;
       if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Void,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
      }
    if( bcdtmList_testForVoidHullLineDtmObject(dtmP,P3,P2) || bcdtmList_testForVoidHullLineDtmObject(dtmP,P2,P3))
      {
       if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,pointAddrP(dtmP,P3)->z)) goto errexit ;
       if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z)) goto errexit ;
       if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Void,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
      }
    if( bcdtmList_testForVoidHullLineDtmObject(dtmP,P2,P1) || bcdtmList_testForVoidHullLineDtmObject(dtmP,P1,P2))
      {
       if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z)) goto errexit ;
       if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z)) goto errexit ;
       if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Void,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
      }
    goto cleanup ;
   }

/*
** Draw Triangle Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Drawing Triangle Lines") ;
 if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z)) goto errexit ;
 if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z)) goto errexit ;
 if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,pointAddrP(dtmP,P3)->z)) goto errexit ;
 if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z)) goto errexit ;
 if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Triangle,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
/*
** Draw Hull Lines For Triangle
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Drawing Hull Lines") ;
 if(nodeAddrP(dtmP,P1)->hPtr == P2 || nodeAddrP(dtmP,P2)->hPtr == P1 )
   {
    if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z)) goto errexit ;
    if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z)) goto errexit ;
    if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Hull,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
   }
 if(nodeAddrP(dtmP,P2)->hPtr == P3 || nodeAddrP(dtmP,P3)->hPtr == P2 )
   {
    if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,pointAddrP(dtmP,P3)->z)) goto errexit ;
    if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z)) goto errexit ;
    if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Hull,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
   }
 if(nodeAddrP(dtmP,P1)->hPtr == P3 || nodeAddrP(dtmP,P3)->hPtr == P1 )
   {
    if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z)) goto errexit ;
    if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,pointAddrP(dtmP,P3)->z)) goto errexit ;
    if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Hull,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
   }
/*
** Draw DTM Features For Line
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Drawing Triangle Line Features") ;
 if( ( clc = nodeAddrP(dtmP,P1)->fPtr ) != dtmP->nullPtr )
   {
    while ( clc != dtmP->nullPtr )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeaturePts.firstPoint != dtmP->nullPnt && flistAddrP(dtmP,clc)->nextPnt == P2 )
         {
          if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z)) goto errexit ;
          if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z)) goto errexit ;
          if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
         }
       clc  = flistAddrP(dtmP,clc)->nextPtr ;
      }
   }
 if( ( clc = nodeAddrP(dtmP,P2)->fPtr ) != dtmP->nullPtr )
   {
    while ( clc != dtmP->nullPtr )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeaturePts.firstPoint != dtmP->nullPnt && flistAddrP(dtmP,clc)->nextPnt == P1 )
         {
          if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z)) goto errexit ;
          if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z)) goto errexit ;
          if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
         }
       clc  = flistAddrP(dtmP,clc)->nextPtr ;
      }
   }

 if( ( clc = nodeAddrP(dtmP,P2)->fPtr ) != dtmP->nullPtr )
   {
    while ( clc != dtmP->nullPtr )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeaturePts.firstPoint != dtmP->nullPnt && flistAddrP(dtmP,clc)->nextPnt == P3 )
         {
          if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,pointAddrP(dtmP,P3)->z)) goto errexit ;
          if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z)) goto errexit ;
          if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
         }
       clc  = flistAddrP(dtmP,clc)->nextPtr ;
      }
   }
 if( ( clc = nodeAddrP(dtmP,P3)->fPtr ) != dtmP->nullPtr )
   {
    while ( clc != dtmP->nullPtr )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeaturePts.firstPoint != dtmP->nullPnt && flistAddrP(dtmP,clc)->nextPnt == P1 )
         {
          if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,pointAddrP(dtmP,P3)->z)) goto errexit ;
          if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z)) goto errexit ;
          if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
         }
       clc  = flistAddrP(dtmP,clc)->nextPtr ;
      }
   }

 if( ( clc = nodeAddrP(dtmP,P3)->fPtr ) != dtmP->nullPtr )
   {
    while ( clc != dtmP->nullPtr )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeaturePts.firstPoint != dtmP->nullPnt && flistAddrP(dtmP,clc)->nextPnt == P1 )
         {
          if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,pointAddrP(dtmP,P3)->z)) goto errexit ;
          if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z)) goto errexit ;
          if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
         }
       clc  = flistAddrP(dtmP,clc)->nextPtr ;
      }
   }
 if( ( clc = nodeAddrP(dtmP,P1)->fPtr ) != dtmP->nullPtr )
   {
    while ( clc != dtmP->nullPtr )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeaturePts.firstPoint != dtmP->nullPnt && flistAddrP(dtmP,clc)->nextPnt == P3 )
         {
          if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,pointAddrP(dtmP,P3)->z)) goto errexit ;
          if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z)) goto errexit ;
          if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
         }
       clc  = flistAddrP(dtmP,clc)->nextPtr ;
      }
   }
/*
** Draw Contours For Triangle
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Drawing Triangle Contours") ;
 if( bcdtmEdit_drawContoursForTriangleDtmObject(dtmP,loadFunctionP,drawMode,P1,P2,P3,contourInterval,userP)) goto errexit ;
/*
** Draw Flow Arrows For Triangle
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Drawing Triangle Attributes") ;
 x = ( pointAddrP(dtmP,P1)->x + pointAddrP(dtmP,P2)->x + pointAddrP(dtmP,P3)->x ) / 3.0 ;
 y = ( pointAddrP(dtmP,P1)->y + pointAddrP(dtmP,P2)->y + pointAddrP(dtmP,P3)->y ) / 3.0 ;
 z = ( pointAddrP(dtmP,P1)->z + pointAddrP(dtmP,P2)->z + pointAddrP(dtmP,P3)->z ) / 3.0 ;
 bcdtmMath_getTriangleAttributesDtmObject(dtmP,P1,P2,P3,&SlopeD,&SlopeP,&Aspect,&Height) ;
 if( bcdtmLoad_storePointInCache(x,y,z)) goto errexit ;
 if( bcdtmLoad_storePointInCache(SlopeD,SlopeP,Aspect)   ) goto errexit ;
 if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Theme,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Drawing Triangle Features Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Drawing Triangle Features Error") ;
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
BENTLEYDTM_Public int bcdtmEdit_drawContoursForTriangleDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long P1,long P2,long P3,double contourInterval,void *userP)
/*
** This Function Updates the Contours For A Triangle
**
*/
{
 int    ret=DTM_SUCCESS ;
 long   cpts, level;
 bool voidTriangle;
 double dx,dy,dz,zc,zz ;
 double Zcon,Zmin,Zmax ;
 DPoint3d    trgPts[3] ;
/*
** If Void Triangle Return
*/
 if( bcdtmList_testForVoidTriangleDtmObject(dtmP,P1,P2,P3,voidTriangle) ) goto errexit ;
 if( voidTriangle ) goto cleanup ;
/*
** Update All Contour Lines For Triangle
*/
 if( contourInterval == 0.0 ) contourInterval = 10.0 ;
 Zmin = Zmax = pointAddrP(dtmP,P1)->z ;
 if( pointAddrP(dtmP,P2)->z < Zmin ) Zmin = pointAddrP(dtmP,P2)->z ;
 if( pointAddrP(dtmP,P2)->z > Zmax ) Zmax = pointAddrP(dtmP,P2)->z ;
 if( pointAddrP(dtmP,P3)->z < Zmin ) Zmin = pointAddrP(dtmP,P3)->z ;
 if( pointAddrP(dtmP,P3)->z > Zmax ) Zmax = pointAddrP(dtmP,P3)->z ;
 level = (long)( Zmin / contourInterval ) ;
 Zmin  = level * contourInterval ;
 level = (long)( Zmax / contourInterval ) + 1 ;
 Zmax  = level * contourInterval ;
 for( Zcon = Zmin ; Zcon <= Zmax ; Zcon += contourInterval  )
   {
    cpts = 0 ;
    zc = Zcon  ;
    if( zc != pointAddrP(dtmP,P1)->z && zc != pointAddrP(dtmP,P2)->z && zc != pointAddrP(dtmP,P3)->z )
      {
       if( ( zc >= pointAddrP(dtmP,P1)->z && zc <= pointAddrP(dtmP,P2)->z )  ||
           ( zc <= pointAddrP(dtmP,P1)->z && zc >= pointAddrP(dtmP,P2)->z )     )
         {
          zz = zc - pointAddrP(dtmP,P1)->z ;
          dz = pointAddrP(dtmP,P2)->z - pointAddrP(dtmP,P1)->z ;
          dx = pointAddrP(dtmP,P2)->x - pointAddrP(dtmP,P1)->x ;
          dy = pointAddrP(dtmP,P2)->y - pointAddrP(dtmP,P1)->y ;
          trgPts[cpts].x = pointAddrP(dtmP,P1)->x + dx * zz / dz ;
          trgPts[cpts].y = pointAddrP(dtmP,P1)->y + dy * zz / dz ;
          trgPts[cpts].z = zc ;
          ++cpts ;
         }
       if( ( zc >= pointAddrP(dtmP,P1)->z && zc <= pointAddrP(dtmP,P3)->z )  ||
           ( zc <= pointAddrP(dtmP,P1)->z && zc >= pointAddrP(dtmP,P3)->z )     )
         {
          zz = zc - pointAddrP(dtmP,P1)->z ;
          dz = pointAddrP(dtmP,P3)->z - pointAddrP(dtmP,P1)->z ;
          dx = pointAddrP(dtmP,P3)->x - pointAddrP(dtmP,P1)->x ;
          dy = pointAddrP(dtmP,P3)->y - pointAddrP(dtmP,P1)->y ;
          trgPts[cpts].x = pointAddrP(dtmP,P1)->x +  dx * zz / dz ;
          trgPts[cpts].y = pointAddrP(dtmP,P1)->y +  dy * zz / dz ;
          trgPts[cpts].z = zc ;
          ++cpts ;
         }
       if( ( zc >= pointAddrP(dtmP,P2)->z && zc <= pointAddrP(dtmP,P3)->z )  ||
           ( zc <= pointAddrP(dtmP,P2)->z && zc >= pointAddrP(dtmP,P3)->z )     )
         {
          zz = zc - pointAddrP(dtmP,P2)->z ;
          dz = pointAddrP(dtmP,P3)->z - pointAddrP(dtmP,P2)->z ;
          dx = pointAddrP(dtmP,P3)->x - pointAddrP(dtmP,P2)->x ;
          dy = pointAddrP(dtmP,P3)->y - pointAddrP(dtmP,P2)->y ;
          trgPts[cpts].x = pointAddrP(dtmP,P2)->x +  dx * zz / dz ;
          trgPts[cpts].y = pointAddrP(dtmP,P2)->y +  dy * zz / dz ;
          trgPts[cpts].z = zc ;
          ++cpts ;
         }
      }
    else if ( zc == pointAddrP(dtmP,P1)->z && zc == pointAddrP(dtmP,P2)->z && zc != pointAddrP(dtmP,P3)->z)
      {
       trgPts[0].x = pointAddrP(dtmP,P1)->x ;
       trgPts[0].y = pointAddrP(dtmP,P1)->y ;
       trgPts[0].z = zc ;
       trgPts[1].x = pointAddrP(dtmP,P2)->x ;
       trgPts[1].y = pointAddrP(dtmP,P2)->y ;
       trgPts[1].z = zc ;
       cpts = 2 ;
      }
    else if ( zc == pointAddrP(dtmP,P1)->z && zc == pointAddrP(dtmP,P3)->z && zc != pointAddrP(dtmP,P2)->z)
      {
       trgPts[0].x = pointAddrP(dtmP,P1)->x ;
       trgPts[0].y = pointAddrP(dtmP,P1)->y ;
       trgPts[0].z = zc ;
       trgPts[1].x = pointAddrP(dtmP,P3)->x ;
       trgPts[1].y = pointAddrP(dtmP,P3)->y ;
       trgPts[1].z = zc ;
       cpts = 2 ;
      }
    else if ( zc == pointAddrP(dtmP,P2)->z && zc == pointAddrP(dtmP,P3)->z && zc != pointAddrP(dtmP,P1)->z)
      {
       trgPts[0].x = pointAddrP(dtmP,P2)->x ;
       trgPts[0].y = pointAddrP(dtmP,P2)->y ;
       trgPts[0].z = zc ;
       trgPts[1].x = pointAddrP(dtmP,P3)->x ;
       trgPts[1].y = pointAddrP(dtmP,P3)->y ;
       trgPts[1].z = zc ;
       cpts = 2 ;
      }
    else if ( zc == pointAddrP(dtmP,P1)->z && zc != pointAddrP(dtmP,P2)->z && zc != pointAddrP(dtmP,P3)->z)
      {
       trgPts[0].x = pointAddrP(dtmP,P1)->x ;
       trgPts[0].y = pointAddrP(dtmP,P1)->y ;
       trgPts[0].z = zc ;
       cpts = 1 ;
       if( ( zc >= pointAddrP(dtmP,P2)->z && zc <= pointAddrP(dtmP,P3)->z )  ||
           ( zc <= pointAddrP(dtmP,P2)->z && zc >= pointAddrP(dtmP,P3)->z )     )
         {
          zz = zc - pointAddrP(dtmP,P2)->z ;
          dz = pointAddrP(dtmP,P3)->z - pointAddrP(dtmP,P2)->z ;
          dx = pointAddrP(dtmP,P3)->x - pointAddrP(dtmP,P2)->x ;
          dy = pointAddrP(dtmP,P3)->y - pointAddrP(dtmP,P2)->y ;
          trgPts[cpts].x = pointAddrP(dtmP,P2)->x +  dx * zz / dz ;
          trgPts[cpts].y = pointAddrP(dtmP,P2)->y +  dy * zz / dz ;
          trgPts[cpts].z = zc ;
          ++cpts ;
         }
      }
    else if ( zc == pointAddrP(dtmP,P2)->z && zc != pointAddrP(dtmP,P1)->z && zc != pointAddrP(dtmP,P3)->z)
      {
       trgPts[0].x = pointAddrP(dtmP,P2)->x ;
       trgPts[0].y = pointAddrP(dtmP,P2)->y ;
       trgPts[0].z = zc ;
       cpts = 1 ;
       if( ( zc >= pointAddrP(dtmP,P1)->z && zc <= pointAddrP(dtmP,P3)->z )  ||
           ( zc <= pointAddrP(dtmP,P1)->z && zc >= pointAddrP(dtmP,P3)->z )     )
         {
          zz = zc - pointAddrP(dtmP,P1)->z ;
          dz = pointAddrP(dtmP,P3)->z - pointAddrP(dtmP,P1)->z ;
          dx = pointAddrP(dtmP,P3)->x - pointAddrP(dtmP,P1)->x ;
          dy = pointAddrP(dtmP,P3)->y - pointAddrP(dtmP,P1)->y ;
          trgPts[cpts].x = pointAddrP(dtmP,P1)->x +  dx * zz / dz ;
          trgPts[cpts].y = pointAddrP(dtmP,P1)->y +  dy * zz / dz ;
          trgPts[cpts].z = zc ;
          ++cpts ;
         }
      }
    else if ( zc == pointAddrP(dtmP,P3)->z && zc != pointAddrP(dtmP,P1)->z && zc != pointAddrP(dtmP,P2)->z)
      {
       trgPts[0].x = pointAddrP(dtmP,P3)->x ;
       trgPts[0].y = pointAddrP(dtmP,P3)->y ;
       trgPts[0].z = zc ;
       cpts = 1 ;
       if( ( zc >= pointAddrP(dtmP,P1)->z && zc <= pointAddrP(dtmP,P2)->z )  ||
           ( zc <= pointAddrP(dtmP,P1)->z && zc >= pointAddrP(dtmP,P2)->z )     )
         {
          zz = zc - pointAddrP(dtmP,P1)->z ;
          dz = pointAddrP(dtmP,P2)->z - pointAddrP(dtmP,P1)->z ;
          dx = pointAddrP(dtmP,P2)->x - pointAddrP(dtmP,P1)->x ;
          dy = pointAddrP(dtmP,P2)->y - pointAddrP(dtmP,P1)->y ;
          trgPts[cpts].x = pointAddrP(dtmP,P1)->x + dx * zz / dz ;
          trgPts[cpts].y = pointAddrP(dtmP,P1)->y + dy * zz / dz ;
          trgPts[cpts].z = zc ;
          ++cpts ;
         }
      }
   if( cpts == 2 )
     {
      if( bcdtmLoad_storePointInCache(trgPts[0].x,trgPts[0].y,trgPts[0].z)) goto errexit ;
      if( bcdtmLoad_storePointInCache(trgPts[1].x,trgPts[1].y,trgPts[1].z)) goto errexit ;
      if( bcdtmLoad_storePointInCache(trgPts[1].x,trgPts[1].y,trgPts[1].z)) goto errexit ;
      if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Contour,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
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
BENTLEYDTM_EXPORT int bcdtmEdit_drawTriangleLinesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long P1,long P2,long P3, void *userP)
{
 int ret=DTM_SUCCESS ;
/*
** Draw Triangle Lines
*/
 if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z)) goto errexit ;
 if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z)) goto errexit ;
 if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Triangle,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
 if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z)) goto errexit ;
 if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,pointAddrP(dtmP,P3)->z)) goto errexit ;
 if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Triangle,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
 if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,pointAddrP(dtmP,P3)->z)) goto errexit ;
 if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P1)->x,pointAddrP(dtmP,P1)->y,pointAddrP(dtmP,P1)->z)) goto errexit ;
 if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Triangle,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
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
BENTLEYDTM_EXPORT int bcdtmEdit_drawTriangleBaseLinesForPointDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long P1, void *userP)
/*
** This Function Draws The Base Lines Of Triangles With Vertev P1
*/
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long clc, clf, P2, P3;
 bool voidLine;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Drawing Triangle Base Lines For Point") ;
    bcdtmWrite_message(0,0,0,"dtmP          = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"loadFunctionP = %p",loadFunctionP) ;
    bcdtmWrite_message(0,0,0,"drawMode      = %8ld",drawMode) ;
    bcdtmWrite_message(0,0,0,"P1            = %8ld",P1) ;
    bcdtmWrite_message(0,0,0,"userP         = %p",userP) ;
   }
/*
** Scan Around Point Draw Lines
*/
 if( (clc = nodeAddrP(dtmP,P1)->cPtr ) == dtmP->nullPnt ) goto cleanup ;
 if(( P2 = bcdtmList_nextAntDtmObject(dtmP,P1,clistAddrP(dtmP,clc)->pntNum)) < 0 ) goto errexit ;
 while( clc != dtmP->nullPtr )
   {
    P3  = clistAddrP(dtmP,clc)->pntNum ;
    clc = clistAddrP(dtmP,clc)->nextPtr ;
    if( bcdtmList_testForVoidLineDtmObject(dtmP,P2,P3,voidLine)) goto errexit ;
    if( ! voidLine )
      {
/*
**     Draw Triangle Lines
*/
       if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z)) goto errexit ;
       if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,pointAddrP(dtmP,P3)->z)) goto errexit ;
       if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Triangle,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
/*
**     Draw Hull Lines
*/
       if( nodeAddrP(dtmP,P3)->hPtr == P2 )
         {
          if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z)) goto errexit ;
          if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,pointAddrP(dtmP,P3)->z)) goto errexit ;
          if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Hull,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
         }
/*
**     Draw Dtm Features
*/
       if( ( clf = nodeAddrP(dtmP,P2)->fPtr ) != dtmP->nullPtr )
         {
          while ( clf != dtmP->nullPtr )
            {
             if( ftableAddrP(dtmP,flistAddrP(dtmP,clf)->dtmFeature)->dtmFeaturePts.firstPoint != dtmP->nullPnt && flistAddrP(dtmP,clf)->nextPnt == P3 )
               {
                if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,pointAddrP(dtmP,P3)->z)) goto errexit ;
                if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z)) goto errexit ;
                if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,ftableAddrP(dtmP,flistAddrP(dtmP,clf)->dtmFeature)->dtmFeatureType,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
               }
             clf  = flistAddrP(dtmP,clf)->nextPtr ;
            }
         }

       if( ( clf = nodeAddrP(dtmP,P3)->fPtr ) != dtmP->nullPtr )
         {
          while ( clf != dtmP->nullPtr )
            {
             if( ftableAddrP(dtmP,flistAddrP(dtmP,clf)->dtmFeature)->dtmFeaturePts.firstPoint != dtmP->nullPnt && flistAddrP(dtmP,clf)->nextPnt == P2 )
               {
                if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P3)->x,pointAddrP(dtmP,P3)->y,pointAddrP(dtmP,P3)->z)) goto errexit ;
                if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,P2)->x,pointAddrP(dtmP,P2)->y,pointAddrP(dtmP,P2)->z)) goto errexit ;
                if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,ftableAddrP(dtmP,flistAddrP(dtmP,clf)->dtmFeature)->dtmFeatureType,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
               }
             clf  = flistAddrP(dtmP,clf)->nextPtr ;
            }
         }
      }
    P2 = P3 ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Drawing Triangle Base Lines For Point Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Drawing Triangle Base Lines For Point Error") ;
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
BENTLEYDTM_EXPORT int bcdtmEdit_drawLineFeaturesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long P1,long P2,double contourInterval,void *userP)
/*
** This Function draws All The Lines Connected To A Line
*/
{
 int    ret=DTM_SUCCESS,dbg=0 ;
 long   ap, cp;
 bool voidFeature;
 double x,y,z,SlopeD,SlopeP,Aspect,Height ;
/*
** Write Status Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Drawing Line Features ** drawMode = %2ld P1 = %8ld P2 = %8ld",drawMode,P1,P2) ;
/*
** Test For Valid Tin Line
*/
 if( P1 >= 0 && P1 < dtmP->numPoints && P2 >= 0 && P2 < dtmP->numPoints )
   {
    if(nodeAddrP(dtmP,P1)->cPtr != dtmP->nullPtr && nodeAddrP(dtmP,P2)->cPtr != dtmP->nullPtr )
      {
       if( bcdtmList_testLineDtmObject(dtmP,P1,P2) )
         {
/*
**       Get Next Points
*/
          if( ( ap = bcdtmList_nextAntDtmObject(dtmP,P1,P2)) < 0 ) goto errexit ;
          if( ( cp = bcdtmList_nextClkDtmObject(dtmP,P1,P2)) < 0 ) goto errexit ;
          if( ! bcdtmList_testLineDtmObject(dtmP,ap,P2)) ap = dtmP->nullPnt ;
          if( ! bcdtmList_testLineDtmObject(dtmP,cp,P2)) cp = dtmP->nullPnt ;
/*
**       Draw Triangle And Contour Features For Line
*/
          if( ap != dtmP->nullPnt )
            {
             if( bcdtmList_testForVoidTriangleDtmObject(dtmP,P1,ap,P2,voidFeature)) goto errexit ;
             if( ! voidFeature )
               {
                if( bcdtmEdit_drawTriangleDtmObject(dtmP,loadFunctionP,drawMode,P1,ap,P2,userP) ) goto errexit ;
                if( bcdtmEdit_drawContoursForTriangleDtmObject(dtmP,loadFunctionP,drawMode,P1,ap,P2,contourInterval,userP)) goto errexit ;
               }
            }
          if( cp != dtmP->nullPnt )
            {
             if( bcdtmList_testForVoidTriangleDtmObject(dtmP,P1,P2,cp,voidFeature)) goto errexit ;
             if( ! voidFeature )
               {
                if( bcdtmEdit_drawTriangleDtmObject(dtmP,loadFunctionP,drawMode,P1,P2,cp,userP) ) goto errexit ;
                if( bcdtmEdit_drawContoursForTriangleDtmObject(dtmP,loadFunctionP,drawMode,P1,P2,cp,contourInterval,userP)) goto errexit ;
               }
            }
/*
**        Draw Dtm Features For Line
*/
          if( bcdtmEdit_drawDtmFeaturesForLineDtmObject(dtmP,loadFunctionP,drawMode,P1,P2,userP) ) goto errexit ;
          if( ap != dtmP->nullPnt )
            {
             if( bcdtmEdit_drawDtmFeaturesForLineDtmObject(dtmP,loadFunctionP,drawMode,P1,ap,userP) ) goto errexit ;
             if( bcdtmEdit_drawDtmFeaturesForLineDtmObject(dtmP,loadFunctionP,drawMode,ap,P2,userP) ) goto errexit ;
            }
          if( cp != dtmP->nullPnt )
            {
             if( bcdtmEdit_drawDtmFeaturesForLineDtmObject(dtmP,loadFunctionP,drawMode,P1,cp,userP) ) goto errexit ;
             if( bcdtmEdit_drawDtmFeaturesForLineDtmObject(dtmP,loadFunctionP,drawMode,cp,P2,userP) ) goto errexit ;
            }
/*
**        Draw Flow Arrows For Line
*/
          if( ap != dtmP->nullPnt )
            {
             if( bcdtmList_testForVoidTriangleDtmObject(dtmP,P1,ap,P2,voidFeature)) goto errexit ;
             if( ! voidFeature )
               {
                x = ( pointAddrP(dtmP,P1)->x + pointAddrP(dtmP,ap)->x + pointAddrP(dtmP,P2)->x ) / 3.0 ;
                y = ( pointAddrP(dtmP,P1)->y + pointAddrP(dtmP,ap)->y + pointAddrP(dtmP,P2)->y ) / 3.0 ;
                z = ( pointAddrP(dtmP,P1)->z + pointAddrP(dtmP,ap)->z + pointAddrP(dtmP,P2)->z ) / 3.0 ;
                bcdtmMath_getTriangleAttributesDtmObject(dtmP,P1,ap,P2,&SlopeD,&SlopeP,&Aspect,&Height) ;
                if( bcdtmLoad_storePointInCache(x,y,z)) goto errexit ;
                if( bcdtmLoad_storePointInCache(SlopeD,SlopeP,Aspect)   ) goto errexit ;
                if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Theme,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
               }
            }
          if( cp != dtmP->nullPnt )
            {
             if( bcdtmList_testForVoidTriangleDtmObject(dtmP,P1,P2,cp,voidFeature)) goto errexit ;
             if( ! voidFeature )
               {
                x = ( pointAddrP(dtmP,P1)->x + pointAddrP(dtmP,P2)->x + pointAddrP(dtmP,cp)->x ) / 3.0 ;
                y = ( pointAddrP(dtmP,P1)->y + pointAddrP(dtmP,P2)->y + pointAddrP(dtmP,cp)->y ) / 3.0 ;
                z = ( pointAddrP(dtmP,P1)->z + pointAddrP(dtmP,P2)->z + pointAddrP(dtmP,cp)->z ) / 3.0 ;
                bcdtmMath_getTriangleAttributesDtmObject(dtmP,P1,P2,cp,&SlopeD,&SlopeP,&Aspect,&Height) ;
                if( bcdtmLoad_storePointInCache(x,y,z)) goto errexit ;
                if( bcdtmLoad_storePointInCache(SlopeD,SlopeP,Aspect)   ) goto errexit ;
                if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Theme,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
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
BENTLEYDTM_EXPORT int bcdtmEdit_drawPolygonFeaturesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,DPoint3d *polyPtsP,long numPolyPts,double contourInterval,void *userP) 
/*
** This Function Draws All The DTM Features Within A Polygon
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   point ;
 double Xmin,Ymin,Xmax,Ymax ;
 DPoint3d    *p3dP ;
 DPoint3d *pointP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Drawing Polygon Features ** drawMode = %2ld",drawMode) ;
/*
** Get Bounding Rectangle For Polygon
*/
 Xmin = Xmax = polyPtsP->x ;
 Ymin = Ymax = polyPtsP->y ;
 for ( p3dP = polyPtsP + 1 ; p3dP < polyPtsP + numPolyPts ; ++p3dP )
   {
    if( p3dP->x < Xmin ) Xmin = p3dP->x ;
    if( p3dP->x > Xmax ) Xmax = p3dP->x ;
    if( p3dP->y < Ymin ) Ymin = p3dP->y ;
    if( p3dP->y > Ymax ) Ymax = p3dP->y ;
   }
/*
** Scan Points And Draw Points In Tin Object
*/
 for( point = 0 ; point < dtmP->numPoints ; ++point )
   {
    if( nodeAddrP(dtmP,point)->cPtr != dtmP->nullPtr )
      {
       pointP = pointAddrP(dtmP,point) ;
       if( pointP->x >= Xmin && pointP->x <= Xmax && pointP->y >= Ymin && pointP->y <= Ymax )
         {
          if( bcdtmClip_pointInPointArrayPolygon(polyPtsP,numPolyPts,pointP->x,pointP->y))
            {
             if( bcdtmEdit_drawPointFeaturesDtmObject(dtmP,loadFunctionP,drawMode,point,contourInterval,userP)) goto errexit ;
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
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Drawing Polygon Features Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Drawing Polygon Features Error") ;
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
BENTLEYDTM_EXPORT int bcdtmEdit_drawTptrFeaturesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long startPoint,long updateOption,double contourInterval,void *userP)
/*
** This Function draws All Features Internal To A Tptr Polygon
** The Tptr Polygon Must Be AntiClockwise
*/
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long   sp, p2, p3, np, clc, feat;
 bool  voidResult;
 DTMFeatureType type;
 double x,y,z,SlopeD,SlopeP,Aspect,Height ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Drawing Tptr Features ** updateOption = %2ld startPoint = %8ld",updateOption,startPoint) ;
/*
** Test Update Flag
*/
 if( updateOption == 2 ) goto cleanup ;
 if( startPoint == dtmP->nullPnt ) goto cleanup ;
/*
** Write Tptr List
*/
 if( dbg ) bcdtmList_writeTptrListDtmObject(dtmP,startPoint) ;
/*
** Draw Triangle Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Drawing Triangle Lines") ;
 sp = startPoint ;
 do
   {
    p2  = nodeAddrP(dtmP,sp)->tPtr ;
    do
      {
       if( bcdtmList_testForVoidLineDtmObject(dtmP,sp,p2,voidResult) ) goto errexit ;
       if( ! voidResult )
         {
          if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z)) goto errexit ;
          if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z)) goto errexit ;
          if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Triangle,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
         }
       if(( p2 = bcdtmList_nextAntDtmObject(dtmP,sp,p2)) < 0 ) goto errexit ;
      } while ( p2 != nodeAddrP(dtmP,sp)->tPtr ) ;
    sp = nodeAddrP(dtmP,sp)->tPtr  ;
   } while ( sp != startPoint && nodeAddrP(dtmP,sp)->tPtr != dtmP->nullPnt ) ;
/*
** Draw Triangle Lines For Last Tptr List Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Drawing Triangle Lines For Last List Point") ;
 if( sp != startPoint )
   {
    sp  = p2 ;
    clc = nodeAddrP(dtmP,sp)->cPtr ;
    while ( clc != dtmP->nullPtr )
      {
       p2  = clistAddrP(dtmP,clc)->pntNum ;
       clc = clistAddrP(dtmP,clc)->nextPtr ;
       if( bcdtmList_testForVoidLineDtmObject(dtmP,sp,p2,voidResult) ) goto errexit ;
       if( ! voidResult )
         {
          if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z)) goto errexit ;
          if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z)) goto errexit ;
          if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Triangle,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
         }
      }
   }
/*
** Draw Hull Lines
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Drawing Hull Lines") ;
 sp = startPoint ;
 do
   {
    p2  = nodeAddrP(dtmP,sp)->tPtr ;
    if(nodeAddrP(dtmP,sp)->hPtr == p2 || nodeAddrP(dtmP,p2)->hPtr == sp )
      {
       if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z)) goto errexit ;
       if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z)) goto errexit ;
       if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Hull,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
      }
    sp = p2 ;
   } while ( sp != startPoint && nodeAddrP(dtmP,sp)->tPtr != dtmP->nullPnt  ) ;
/*
** Draw DTM Features
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Drawing Dtm Features") ;
 sp = startPoint ;
 do
   {
    p2  = nodeAddrP(dtmP,sp)->tPtr ;

    if( ( clc = nodeAddrP(dtmP,sp)->fPtr ) != dtmP->nullPtr )
      {
       while ( clc != dtmP->nullPtr )
         {
          feat = flistAddrP(dtmP,clc)->dtmFeature ;
          type = ftableAddrP(dtmP,feat)->dtmFeatureType ;
          if( ftableAddrP(dtmP,feat)->dtmFeaturePts.firstPoint != dtmP->nullPnt && flistAddrP(dtmP,clc)->nextPnt == p2 )
            {
             if( bcdtmList_testForVoidLineDtmObject(dtmP,sp,p2,voidResult) ) goto errexit ;
             if( ! voidResult )
               {
                if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z)) goto errexit ;
                if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z)) goto errexit ;
                if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,type,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
               }
            }
          clc  = flistAddrP(dtmP,clc)->nextPtr ;
         }
      }

    if( ( clc = nodeAddrP(dtmP,p2)->fPtr ) != dtmP->nullPtr )
      {
       while ( clc != dtmP->nullPtr )
         {
          feat = flistAddrP(dtmP,clc)->dtmFeature ;
          type = ftableAddrP(dtmP,feat)->dtmFeatureType ;
          if( ftableAddrP(dtmP,feat)->dtmFeaturePts.firstPoint != dtmP->nullPnt && flistAddrP(dtmP,clc)->nextPnt == sp )
            {
             if( bcdtmList_testForVoidLineDtmObject(dtmP,sp,p2,voidResult) ) goto errexit ;
             if( ! voidResult )
               {
                if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z)) goto errexit ;
                if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z)) goto errexit ;
                if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,type,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
               }
            }
          clc  = flistAddrP(dtmP,clc)->nextPtr ;
         }
      }

    if( ( clc = nodeAddrP(dtmP,sp)->fPtr ) != dtmP->nullPtr )
      {
       while ( clc != dtmP->nullPtr )
         {
          feat = flistAddrP(dtmP,clc)->dtmFeature ;
          type = ftableAddrP(dtmP,feat)->dtmFeatureType ;
          np   = flistAddrP(dtmP,clc)->nextPnt ;
          if( np != dtmP->nullPnt )
            {
             if( ftableAddrP(dtmP,feat)->dtmFeaturePts.firstPoint != dtmP->nullPnt && nodeAddrP(dtmP,np)->tPtr != dtmP->nullPnt  )
               {
                if( bcdtmList_testForVoidLineDtmObject(dtmP,sp,np,voidResult) ) goto errexit ;
                if( ! voidResult )
                  {
                   if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z)) goto errexit ;
                   if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,np)->x,pointAddrP(dtmP,np)->y,pointAddrP(dtmP,np)->z)) goto errexit ;
                   if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,type,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
                  }
               }
            }
          clc  = flistAddrP(dtmP,clc)->nextPtr ;
         }
      }

    sp = p2 ;
   } while ( sp != startPoint  && nodeAddrP(dtmP,sp)->tPtr != dtmP->nullPnt ) ;
/*
** Draw Contours
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Drawing Contours") ;
 sp = startPoint ;
 do
   {
    p2  = nodeAddrP(dtmP,sp)->tPtr ;
    if( ( p3 = bcdtmList_nextClkDtmObject(dtmP,sp,p2)) < 0 ) goto errexit ;
    do
      {
       if( bcdtmList_testForVoidTriangleDtmObject(dtmP,sp,p2,p3,voidResult)) goto errexit ;
       if( ! voidResult ) if( bcdtmEdit_drawContoursForTriangleDtmObject(dtmP,loadFunctionP,drawMode,sp,p2,p3,contourInterval,userP)) goto errexit ;
       p2 = p3 ;
       if( ( p3 = bcdtmList_nextClkDtmObject(dtmP,sp,p2)) < 0 ) goto errexit ;
      }  while ( p2 != nodeAddrP(dtmP,sp)->tPtr ) ;
    sp  = nodeAddrP(dtmP,sp)->tPtr ;
   } while ( sp != startPoint  && nodeAddrP(dtmP,sp)->tPtr != dtmP->nullPnt ) ;
/*
** Draw Contour Lines For Last Tptr List Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Drawing Contours For Last Point") ;
 if( sp != startPoint )
   {
    sp  = p2 ;
    clc = nodeAddrP(dtmP,sp)->cPtr ;
    if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,sp,clistAddrP(dtmP,clc)->pntNum)) < 0 ) goto errexit ;
    while ( clc != dtmP->nullPtr )
      {
       p3  = clistAddrP(dtmP,clc)->pntNum ;
       clc = clistAddrP(dtmP,clc)->nextPtr ;
       if( bcdtmList_testForVoidTriangleDtmObject(dtmP,sp,p2,p3,voidResult)) goto errexit ;
       if( ! voidResult ) if( bcdtmEdit_drawContoursForTriangleDtmObject(dtmP,loadFunctionP,drawMode,sp,p2,p3,contourInterval,userP)) goto errexit ;
       p2 = p3 ;
      }
   }
/*
** Draw Flow Arrows For Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Drawing Flow Arrows") ;
 sp = startPoint ;
 do
   {
    p2  = nodeAddrP(dtmP,sp)->tPtr ;
    if( ( p3 = bcdtmList_nextClkDtmObject(dtmP,sp,p2)) < 0 ) goto errexit ;
    while( nodeAddrP(dtmP,sp)->tPtr != p3 )
      {
       if( bcdtmList_testForVoidTriangleDtmObject(dtmP,sp,p2,p3,voidResult)) goto errexit ;
       if( ! voidResult )
         {
          x = ( pointAddrP(dtmP,sp)->x + pointAddrP(dtmP,p2)->x + pointAddrP(dtmP,p3)->x ) / 3.0 ;
          y = ( pointAddrP(dtmP,sp)->y + pointAddrP(dtmP,p2)->y + pointAddrP(dtmP,p3)->y ) / 3.0 ;
          z = ( pointAddrP(dtmP,sp)->z + pointAddrP(dtmP,p2)->z + pointAddrP(dtmP,p3)->z ) / 3.0 ;
          bcdtmMath_getTriangleAttributesDtmObject(dtmP,sp,p2,p3,&SlopeD,&SlopeP,&Aspect,&Height) ;
          if( bcdtmLoad_storePointInCache(x,y,z)) goto errexit ;
          if( bcdtmLoad_storePointInCache(SlopeD,SlopeP,Aspect)   ) goto errexit ;
          if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Theme,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
             }
       p2 = p3 ;
       if( ( p3 = bcdtmList_nextClkDtmObject(dtmP,sp,p2)) < 0 ) goto errexit ;
      }
    sp  = nodeAddrP(dtmP,sp)->tPtr ;
   } while ( sp != startPoint  && nodeAddrP(dtmP,sp)->tPtr != dtmP->nullPnt ) ;
/*
** Draw Flow Arrows For Last Tptr List Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Drawing Flow Arrows For Last Point") ;
 if( sp != startPoint )
   {
    sp  = p2 ;
    clc = nodeAddrP(dtmP,sp)->cPtr ;
    if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,sp,clistAddrP(dtmP,clc)->pntNum)) < 0 ) goto errexit ;
    while ( clc != dtmP->nullPtr )
      {
       p3  = clistAddrP(dtmP,clc)->pntNum ;
       clc = clistAddrP(dtmP,clc)->nextPtr ;
       if( bcdtmList_testForVoidTriangleDtmObject(dtmP,sp,p2,p3,voidResult)) goto errexit ;
       if( ! voidResult )
         {
          x = ( pointAddrP(dtmP,sp)->x + pointAddrP(dtmP,p2)->x + pointAddrP(dtmP,p3)->x ) / 3.0 ;
          y = ( pointAddrP(dtmP,sp)->y + pointAddrP(dtmP,p2)->y + pointAddrP(dtmP,p3)->y ) / 3.0 ;
          z = ( pointAddrP(dtmP,sp)->z + pointAddrP(dtmP,p2)->z + pointAddrP(dtmP,p3)->z ) / 3.0 ;
          bcdtmMath_getTriangleAttributesDtmObject(dtmP,sp,p2,p3,&SlopeD,&SlopeP,&Aspect,&Height) ;
          if( bcdtmLoad_storePointInCache(x,y,z)) goto errexit ;
          if( bcdtmLoad_storePointInCache(SlopeD,SlopeP,Aspect)   ) goto errexit ;
          if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Theme,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
             }
       p2 = p3 ;
      }
   }
/*
** Null Out Tptr Array
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Nulling Tptr List") ;
 sp = startPoint ;
 do
   {
    p2 = nodeAddrP(dtmP,sp)->tPtr ;
    nodeAddrP(dtmP,sp)->tPtr = dtmP->nullPnt ;
    sp = p2 ;
   } while ( sp != startPoint  && nodeAddrP(dtmP,sp)->tPtr != dtmP->nullPnt ) ;
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Drawing Tptr Features Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Drawing Tptr Features Error") ;
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
BENTLEYDTM_Public int bcdtmEdit_drawTptrLineFeaturesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long startPoint,void *userP)
/*
** This Function draws All Features Internal To A Tptr Polygon
** Tptr Polygon Must Be AntiClockwise
*/
{
 int    ret=DTM_SUCCESS ;
 long   sp, p2, clc, feat;
 bool  voidResult;
 DTMFeatureType type;
/*
** Test Update Flag
*/
 if( startPoint == dtmP->nullPnt ) return(0) ;
/*
** Draw Triangle Lines
*/
 sp = startPoint ;
 do
   {
    p2  = nodeAddrP(dtmP,sp)->tPtr ;
    if( bcdtmList_testForVoidLineDtmObject(dtmP,sp,p2,voidResult) ) goto errexit ;
    if( ! voidResult )
      {
       if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z)) goto errexit ;
       if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z)) goto errexit ;
       if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Triangle,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
      }
    sp = p2 ;
   } while ( sp != startPoint && sp != dtmP->nullPnt ) ;
/*
** Draw Hull Lines
*/
 sp = startPoint ;
 do
   {
    p2  = nodeAddrP(dtmP,sp)->tPtr ;
    if(nodeAddrP(dtmP,sp)->hPtr == p2 || nodeAddrP(dtmP,p2)->hPtr == sp )
      {
       if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z)) goto errexit ;
       if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z)) goto errexit ;
       if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Hull,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
      }
    sp = p2 ;
   } while ( sp != startPoint && nodeAddrP(dtmP,sp)->tPtr != dtmP->nullPnt  ) ;
/*
** Draw DTM Features
*/
 sp = startPoint ;
 do
   {
    p2  = nodeAddrP(dtmP,sp)->tPtr ;

    if( ( clc = nodeAddrP(dtmP,sp)->fPtr ) != dtmP->nullPtr )
      {
       while ( clc != dtmP->nullPtr )
         {
          feat = flistAddrP(dtmP,clc)->dtmFeature ;
          type = ftableAddrP(dtmP,feat)->dtmFeatureType ;
          if( ftableAddrP(dtmP,feat)->dtmFeaturePts.firstPoint != dtmP->nullPnt && flistAddrP(dtmP,clc)->nextPnt == p2 )
            {
             if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z)) goto errexit ;
             if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z)) goto errexit ;
             if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,type,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
            }
          clc  = flistAddrP(dtmP,clc)->nextPtr ;
         }
      }

    if( ( clc = nodeAddrP(dtmP,p2)->fPtr ) != dtmP->nullPtr )
      {
       while ( clc != dtmP->nullPtr )
         {
          feat = flistAddrP(dtmP,clc)->dtmFeature ;
          type = ftableAddrP(dtmP,feat)->dtmFeatureType ;
          if( ftableAddrP(dtmP,feat)->dtmFeaturePts.firstPoint != dtmP->nullPnt && flistAddrP(dtmP,clc)->nextPnt == sp )
            {
             if( bcdtmList_testForVoidLineDtmObject(dtmP,sp,p2,voidResult) ) goto errexit ;
             if( ! voidResult )
               {
                if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z)) goto errexit ;
                if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z)) goto errexit ;
                if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,type,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
               }
            }
          clc  = flistAddrP(dtmP,clc)->nextPtr ;
         }
      }

    sp = p2 ;
   } while ( sp != startPoint  && nodeAddrP(dtmP,sp)->tPtr != dtmP->nullPnt ) ;
/*
** Null Out Tptr Array
*/
 sp = startPoint ;
 do
   {
    p2 = nodeAddrP(dtmP,sp)->tPtr ;
    nodeAddrP(dtmP,sp)->tPtr = dtmP->nullPnt ;
    sp = p2 ;
   } while ( sp != startPoint  && nodeAddrP(dtmP,sp)->tPtr != dtmP->nullPnt ) ;
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
BENTLEYDTM_EXPORT int bcdtmEdit_drawInternalTptrFeaturesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long startPoint,double contourInterval,void *userP)
/*
** This Function Moves The z Values within A Tpyt Polygon
*/
{
 int    ret=DTM_SUCCESS ;
 long   sp,clc,cp,np,process,point ;
 DTMDirection Direction;
 double Area ;
 DTM_TIN_NODE  *nodeP ;
/*
** Check Direction Of Tptr Polygon And If Clockwise
** Set Direction Anti Clockwise
*/
 if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPoint,&Area,&Direction) ) goto errexit ;
 if( Direction == DTMDirection::Clockwise ) if( bcdtmList_reverseTptrPolygonDtmObject(dtmP,startPoint)) goto errexit ;
/*
** Mark Points Immediately Internal To Tptr Polygon
*/
 sp = startPoint ;
 do
   {
    np = nodeAddrP(dtmP,sp)->tPtr ;
    if( ( cp = bcdtmList_nextAntDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
    while ( nodeAddrP(dtmP,cp)->tPtr != sp )
      {
       if( nodeAddrP(dtmP,cp)->tPtr == dtmP->nullPnt ) nodeAddrP(dtmP,cp)->tPtr = -dtmP->nullPnt ;
       if( ( cp = bcdtmList_nextAntDtmObject(dtmP,sp,cp)) < 0 ) goto errexit ;
      }
    sp = np ;
   } while ( sp != startPoint ) ;
/*
** Mark All Points Connecting to Marked Points
*/
 process = 1 ;
 while( process )
   {
    process = 0 ;
    for( sp = 0 ; sp < dtmP->numPoints ; ++sp )
      {
       if( nodeAddrP(dtmP,sp)->tPtr == -dtmP->nullPnt )
             {
              clc = nodeAddrP(dtmP,sp)->cPtr ;
              while ( clc != dtmP->nullPtr )
                {
                 cp  = clistAddrP(dtmP,clc)->pntNum ;
                 clc = clistAddrP(dtmP,clc)->nextPtr ;
                 if( nodeAddrP(dtmP,cp)->tPtr == dtmP->nullPnt )
                   {
                        nodeAddrP(dtmP,cp)->tPtr = -dtmP->nullPnt ;
                        process = 1 ;
                   }
                }
              nodeAddrP(dtmP,sp)->tPtr = -dtmP->nullPnt + 10 ;
             }
      }
   }
/*
** Display Point Features On Tptr Hull
*/
 sp = startPoint ;
 do
   {
    if( bcdtmEdit_drawPointFeaturesDtmObject(dtmP,loadFunctionP,drawMode,sp,contourInterval,userP)) goto errexit ;
    sp = nodeAddrP(dtmP,sp)->tPtr ;
   } while ( sp != startPoint ) ;
/*
** Scan Internal Points And Display Point Features
*/
 for( point = 0 ; point < dtmP->numPoints ; ++point )
   {
    nodeP = nodeAddrP(dtmP,point) ;
    if( nodeP->cPtr != dtmP->nullPtr && nodeP->tPtr < 0  )
      {
       if( bcdtmEdit_drawPointFeaturesDtmObject(dtmP,loadFunctionP,drawMode,point,contourInterval,userP)) goto errexit ;
       nodeP->tPtr = dtmP->nullPnt ;
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
BENTLEYDTM_EXPORT int bcdtmEdit_drawExternalTptrFeaturesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long startPoint,long updateOption,double contourInterval,void *userP)
/*
** This Function draws All Features Internal To A Tptr Polygon
** Tptr Polygon Must Be AntiClockwise
*/
{
 int    ret=DTM_SUCCESS ;
 long   sp, p2, p3, clc, feat;
 bool  voidResult;
 DTMFeatureType type;
 double x,y,z,SlopeD,SlopeP,Aspect,Height ;
/*
** Test Update Flag
*/
 if( updateOption == 2 ) return(0) ;
/*
** Draw Triangle Lines
*/
 sp = startPoint ;
 do
   {
    p2  = nodeAddrP(dtmP,sp)->tPtr ;
    do
      {
       if( bcdtmList_testForVoidLineDtmObject(dtmP,sp,p2,voidResult) ) goto errexit ;
       if( ! voidResult )
         {
          if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z)) goto errexit ;
          if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z)) goto errexit ;
          if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Triangle,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
         }
       if(( p2 = bcdtmList_nextAntDtmObject(dtmP,sp,p2)) < 0 ) goto errexit ;
      } while ( p2 != nodeAddrP(dtmP,sp)->tPtr ) ;
    sp = nodeAddrP(dtmP,sp)->tPtr  ;
   } while ( sp != startPoint && nodeAddrP(dtmP,sp)->tPtr != dtmP->nullPnt ) ;
/*
** Draw Triangle Lines For Last Tptr List Point
*/
 if( sp != startPoint )
   {
    sp  = p2 ;
    clc = nodeAddrP(dtmP,sp)->cPtr ;
    while ( clc != dtmP->nullPtr )
      {
       p2  = clistAddrP(dtmP,clc)->pntNum ;
       clc = clistAddrP(dtmP,clc)->nextPtr ;
       if( bcdtmList_testForVoidLineDtmObject(dtmP,sp,p2,voidResult) ) goto errexit ;
       if( ! voidResult )
         {
          if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z)) goto errexit ;
          if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z)) goto errexit ;
          if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Triangle,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
         }
      }
   }
/*
** Draw Hull Lines
*/
 sp = startPoint ;
 do
   {
    p2  = nodeAddrP(dtmP,sp)->tPtr ;
    if(nodeAddrP(dtmP,sp)->hPtr == p2 || nodeAddrP(dtmP,p2)->hPtr == sp )
      {
       if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z)) goto errexit ;
       if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z)) goto errexit ;
       if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Hull,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
      }
    sp = p2 ;
   } while ( sp != startPoint && nodeAddrP(dtmP,sp)->tPtr != dtmP->nullPnt  ) ;
/*
** Draw DTM Features
*/
 sp = startPoint ;
 do
   {
    p2  = nodeAddrP(dtmP,sp)->tPtr ;

    if( ( clc = nodeAddrP(dtmP,sp)->fPtr ) != dtmP->nullPtr )
      {
       while ( clc != dtmP->nullPtr )
         {
          feat = flistAddrP(dtmP,clc)->dtmFeature ;
          type = ftableAddrP(dtmP,feat)->dtmFeatureType ;
          if( ftableAddrP(dtmP,feat)->dtmFeaturePts.firstPoint != dtmP->nullPnt && flistAddrP(dtmP,clc)->nextPnt == p2 )
            {
             if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z)) goto errexit ;
             if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z)) goto errexit ;
             if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,type,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
            }
          clc  = flistAddrP(dtmP,clc)->nextPtr ;
         }
      }

    if( ( clc = nodeAddrP(dtmP,p2)->fPtr ) != dtmP->nullPtr )
      {
       while ( clc != dtmP->nullPtr )
         {
          feat = flistAddrP(dtmP,clc)->dtmFeature ;
          type = ftableAddrP(dtmP,feat)->dtmFeatureType ;
          if( ftableAddrP(dtmP,feat)->dtmFeaturePts.firstPoint != dtmP->nullPnt && flistAddrP(dtmP,clc)->nextPnt == sp )
            {
             if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z)) goto errexit ;
             if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,p2)->x,pointAddrP(dtmP,p2)->y,pointAddrP(dtmP,p2)->z)) goto errexit ;
             if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,type,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
            }
          clc  = flistAddrP(dtmP,clc)->nextPtr ;
         }
      }
    sp = p2 ;
   } while ( sp != startPoint  && nodeAddrP(dtmP,sp)->tPtr != dtmP->nullPnt ) ;
/*
** Draw Contours
*/
 sp = startPoint ;
 do
   {
    p2  = nodeAddrP(dtmP,sp)->tPtr ;
    if( ( p3 = bcdtmList_nextClkDtmObject(dtmP,sp,p2)) < 0 ) goto errexit ;
    do
      {
       if( bcdtmList_testForVoidTriangleDtmObject(dtmP,sp,p2,p3,voidResult)) goto errexit ;
       if( ! voidResult ) if( bcdtmEdit_drawContoursForTriangleDtmObject(dtmP,loadFunctionP,drawMode,sp,p2,p3,contourInterval,userP)) goto errexit ;
       p2 = p3 ;
       if( ( p3 = bcdtmList_nextClkDtmObject(dtmP,sp,p2)) < 0 ) goto errexit ;
      }  while ( p2 != nodeAddrP(dtmP,sp)->tPtr ) ;
    sp  = nodeAddrP(dtmP,sp)->tPtr ;
   } while ( sp != startPoint  && nodeAddrP(dtmP,sp)->tPtr != dtmP->nullPnt ) ;
/*
** Draw Contour Lines For Last Tptr List Point
*/
 if( sp != startPoint )
   {
    sp  = p2 ;
    clc = nodeAddrP(dtmP,sp)->cPtr ;
    if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,sp,clistAddrP(dtmP,clc)->pntNum)) < 0 ) goto errexit ;
    while ( clc != dtmP->nullPtr )
      {
       p3  = clistAddrP(dtmP,clc)->pntNum ;
       clc = clistAddrP(dtmP,clc)->nextPtr ;
       if( bcdtmList_testForVoidTriangleDtmObject(dtmP,sp,p2,p3,voidResult)) goto errexit ;
       if( ! voidResult ) if( bcdtmEdit_drawContoursForTriangleDtmObject(dtmP,loadFunctionP,drawMode,sp,p2,p3,contourInterval,userP)) goto errexit ;
       p2 = p3 ;
      }
   }
/*
** Draw Flow Arrows For Point
*/
 sp = startPoint ;
 do
   {
    p2  = nodeAddrP(dtmP,sp)->tPtr ;
    if( ( p3 = bcdtmList_nextClkDtmObject(dtmP,sp,p2)) < 0 ) goto errexit ;
    while( nodeAddrP(dtmP,sp)->tPtr != p3 )
      {
       if( bcdtmList_testForVoidTriangleDtmObject(dtmP,sp,p2,p3,voidResult)) goto errexit ;
       if( ! voidResult )
         {
          x = ( pointAddrP(dtmP,sp)->x + pointAddrP(dtmP,p2)->x + pointAddrP(dtmP,p3)->x ) / 3.0 ;
          y = ( pointAddrP(dtmP,sp)->y + pointAddrP(dtmP,p2)->y + pointAddrP(dtmP,p3)->y ) / 3.0 ;
          z = ( pointAddrP(dtmP,sp)->z + pointAddrP(dtmP,p2)->z + pointAddrP(dtmP,p3)->z ) / 3.0 ;
          bcdtmMath_getTriangleAttributesDtmObject(dtmP,sp,p2,p3,&SlopeD,&SlopeP,&Aspect,&Height) ;
          if( bcdtmLoad_storePointInCache(x,y,z)) goto errexit ;
          if( bcdtmLoad_storePointInCache(SlopeD,SlopeP,Aspect)   ) goto errexit ;
          if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Theme,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
             }
       p2 = p3 ;
       if( ( p3 = bcdtmList_nextClkDtmObject(dtmP,sp,p2)) < 0 ) goto errexit ;
      }
    sp  = nodeAddrP(dtmP,sp)->tPtr ;
   } while ( sp != startPoint  && nodeAddrP(dtmP,sp)->tPtr != dtmP->nullPnt ) ;
/*
** Draw Flow Arrows For Last Tptr List Point
*/
 if( sp != startPoint )
   {
    sp  = p2 ;
    clc = nodeAddrP(dtmP,sp)->cPtr ;
    if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,sp,clistAddrP(dtmP,clc)->pntNum)) < 0 ) goto errexit ;
    while ( clc != dtmP->nullPtr )
      {
       p3  = clistAddrP(dtmP,clc)->pntNum ;
       clc = clistAddrP(dtmP,clc)->nextPtr ;
       if( bcdtmList_testForVoidTriangleDtmObject(dtmP,sp,p2,p3,voidResult)) goto errexit ;
       if( ! voidResult )
         {
          x = ( pointAddrP(dtmP,sp)->x + pointAddrP(dtmP,p2)->x + pointAddrP(dtmP,p3)->x ) / 3.0 ;
          y = ( pointAddrP(dtmP,sp)->y + pointAddrP(dtmP,p2)->y + pointAddrP(dtmP,p3)->y ) / 3.0 ;
          z = ( pointAddrP(dtmP,sp)->z + pointAddrP(dtmP,p2)->z + pointAddrP(dtmP,p3)->z ) / 3.0 ;
          bcdtmMath_getTriangleAttributesDtmObject(dtmP,sp,p2,p3,&SlopeD,&SlopeP,&Aspect,&Height) ;
          if( bcdtmLoad_storePointInCache(x,y,z)) goto errexit ;
          if( bcdtmLoad_storePointInCache(SlopeD,SlopeP,Aspect)   ) goto errexit ;
          if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Theme,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
             }
       p2 = p3 ;
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
BENTLEYDTM_EXPORT int bcdtmEdit_drawDtmFeatureDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long dtmFeature,void *userP) 
/*
** This Function draws All dtmFeatures Internal To A Tptr Polygon
** Tptr Polygon Must Be AntiClockwise
*/
{
 int    ret=DTM_SUCCESS ;
 long   sp, np, spnt;
 DTMFeatureType dtmFeatureType;
/*
** Test dtmFeature Range
*/
 if( dtmFeature < 0 || dtmFeature >= dtmP->numFeatures  ) goto cleanup ;
 if( ( sp = spnt = ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ) == dtmP->nullPnt ) goto cleanup ;
/*
** Initialise
*/
 dtmFeatureType = ftableAddrP(dtmP,dtmFeature)->dtmFeatureType ;
/*
** Draw dtmFeature Lines
*/
 bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sp,&np) ;
 while ( np != dtmP->nullPnt )
   {
    if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z)) goto errexit ;
    if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,np)->x,pointAddrP(dtmP,np)->y,pointAddrP(dtmP,np)->z)) goto errexit ;
    if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Triangle,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
    if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,sp)->x,pointAddrP(dtmP,sp)->y,pointAddrP(dtmP,sp)->z)) goto errexit ;
    if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,np)->x,pointAddrP(dtmP,np)->y,pointAddrP(dtmP,np)->z)) goto errexit ;
    if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,dtmFeatureType,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
    sp = np  ;
    bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sp,&np) ;
    if( sp == spnt ) np = dtmP->nullPnt ;
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
BENTLEYDTM_EXPORT int bcdtmEdit_drawPointPerimeterDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long Point,void *userP )
/*
** This Function Draws Arounnd Perimeter Of A Point
*/
{
 int  ret=DTM_SUCCESS ;
 long clc,pp,np ;
/*
** Initialise
*/
 if( Point < 0 || Point >=dtmP->numPoints ) goto cleanup ;
 if( nodeAddrP(dtmP,Point)->cPtr == dtmP->nullPtr ) goto cleanup ;
/*
** Scan Around Point
*/
 clc = nodeAddrP(dtmP,Point)->cPtr ;
 np  = clistAddrP(dtmP,clc)->pntNum ;
 if(( pp = bcdtmList_nextAntDtmObject(dtmP,Point,np)) < 0 ) goto errexit ;
 while ( clc != dtmP->nullPtr )
   {
    np  = clistAddrP(dtmP,clc)->pntNum ;
    clc = clistAddrP(dtmP,clc)->nextPtr ;
    if( bcdtmList_testLineDtmObject(dtmP,pp,np))
      {
       if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,pp)->x,pointAddrP(dtmP,pp)->y,pointAddrP(dtmP,pp)->z)) goto errexit ;
       if( bcdtmLoad_storePointInCache(pointAddrP(dtmP,np)->x,pointAddrP(dtmP,np)->y,pointAddrP(dtmP,np)->z)) goto errexit ;
       if( bcdtmLoad_callUserBrowseFunctionWithCachePoints(loadFunctionP,DTMFeatureType::Triangle,drawMode,dtmP->nullFeatureId,userP) ) goto errexit ;
      }
    pp  = np ;
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
BENTLEYDTM_EXPORT int bcdtmEdit_drawDeletedFeaturesDtmObject(BC_DTM_OBJ *dtmP,DTMFeatureCallback loadFunctionP,long drawMode,long *deletedFeaturesP,long numDeletedFeatures,double contourInterval,void *userP)
/*
** Draw dtmFeatures For Deleted Void
*/
{
 int  ret=DTM_SUCCESS ;
 long n,sp,np,mp,lp,pp,spnt ;
/*
** Draw dtmFeatures For First Deleted dtmFeature ** Previously A Void
*/
 sp = spnt = ftableAddrP(dtmP,*deletedFeaturesP)->dtmFeaturePts.firstPoint ;
 bcdtmList_getPriorPointForDtmFeatureDtmObject(dtmP,*deletedFeaturesP,sp,&pp) ;
 do
   {
    bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,*deletedFeaturesP,sp,&np) ;
    if((lp = bcdtmList_nextAntDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
    mp = np ;
    while ( lp != pp )
      {
       bcdtmEdit_drawTriangleFeaturesDtmObject(dtmP,loadFunctionP,drawMode,sp,lp,mp,contourInterval,userP) ;
       mp = lp ;
       if((lp = bcdtmList_nextAntDtmObject(dtmP,sp,lp)) < 0 ) goto errexit ;
      }
    pp = sp ;
    sp = np ;
   } while ( sp != spnt ) ;
/*
** Draw dtmFeatures For Subsequent Deleted dtmFeatures ** Previously Islands
*/
 for ( n = 1 ; n < numDeletedFeatures ; ++n )
   {
    sp = spnt = ftableAddrP(dtmP,*(deletedFeaturesP+n))->dtmFeaturePts.firstPoint ;
    bcdtmList_getPriorPointForDtmFeatureDtmObject(dtmP,*(deletedFeaturesP+n),sp,&pp) ;
    do
      {
       bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,*(deletedFeaturesP+n),sp,&np) ;
       if((lp = bcdtmList_nextClkDtmObject(dtmP,sp,np)) < 0 ) goto errexit ;
       mp = np ;
       while( lp != pp )
         {
          bcdtmEdit_drawTriangleFeaturesDtmObject(dtmP,loadFunctionP,drawMode,sp,mp,lp,contourInterval,userP) ;
          mp = lp ;
          if((lp = bcdtmList_nextClkDtmObject(dtmP,sp,lp)) < 0 ) goto errexit ;
         }
       pp = sp ;
       sp = np ;
      } while ( sp != spnt ) ;
   }
/*
** Remove dtmFeatures
*/
 for( n = 0 ; n < numDeletedFeatures ; ++n )
   {
    if( bcdtmInsert_removeDtmFeatureFromDtmObject(dtmP,*(deletedFeaturesP+n))) goto errexit ;
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
