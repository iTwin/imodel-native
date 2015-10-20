/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmCheck.cpp $
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
BENTLEYDTM_EXPORT int bcdtmCheck_trianglesDtmObject
(
 BC_DTM_OBJ *dtmP
)
/*
** This Function Checks The Tin For 
** Triangle Topology And Precision Errors
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ; 
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Triangles ** dtmP = %p",dtmP) ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check For Tin Error State
*/
  if( dtmP->dtmState == DTMState::TinError ) 
    {
     bcdtmWrite_message(0,0,0,"DTM Object %p Tin Structure Is Invalid",dtmP) ;
     goto errexit ;
    }
/*
** Only Check Tin Component Of Dtm Object
*/
  if( dtmP->dtmState == DTMState::Tin )
    {
/*
**   Check Topology
*/
     if( dbg ) bcdtmWrite_message(0,0,0,"**** Checking Tin Topology") ;
     if( bcdtmCheck_topologyDtmObject(dtmP,1))
       { 
        bcdtmWrite_message(1,0,0,"**** Tin Topology Invalid") ; 
        goto errexit ; 
       }
     if( dbg ) bcdtmWrite_message(0,0,0,"**** Tin Topology Valid") ;
/*
**   Check Precision
*/
     if( dbg ) bcdtmWrite_message(0,0,0,"**** Checking Tin Precision") ;
     if( bcdtmCheck_precisionDtmObject(dtmP,0) ) 
       { 
        bcdtmWrite_message(1,0,0,"**** Tin Precision Invalid") ;
        goto errexit ;
       }
     if( dbg ) bcdtmWrite_message(0,0,0,"**** Tin Precision Valid") ;
    } 
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Triangles Completed",dtmP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Triangles Error",dtmP) ;
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
BENTLEYDTM_EXPORT int bcdtmCheck_tinComponentDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Checks The Integrity Of A Dtm Object
**
*/
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ; 
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Check DTM Object %p Tin Component",dtmP) ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check For Tin Error State
*/
  if( dtmP->dtmState == DTMState::TinError ) 
    {
     bcdtmWrite_message(0,0,0,"DTM Object %p Tin Structure Is Invalid",dtmP) ;
     goto errexit ;
    }
/*
** Only Check Tin Component Of Dtm Object
*/
  if( dtmP->dtmState == DTMState::Tin )
    {
/*
**   Check Topology
*/
     if( dbg ) bcdtmWrite_message(0,0,0,"Checking Tin Topology") ;
     if( bcdtmCheck_topologyDtmObject(dtmP,1))
       { 
        bcdtmWrite_message(1,0,0,"Tin Topology Invalid") ; goto errexit ; 
       }
     if( dbg ) bcdtmWrite_message(0,0,0,"Tin Topology Valid") ;
/*
**   Check Precision
*/
     if( dbg ) bcdtmWrite_message(0,0,0,"Checking Tin Precision") ;
     if( bcdtmCheck_precisionDtmObject(dtmP,0) ) 
       { 
        bcdtmWrite_message(1,0,0,"Tin Precision Invalid") ;
        goto errexit ;
       }
     if( dbg ) bcdtmWrite_message(0,0,0,"Tin Precision Valid") ;
/*
**   Check Topology DTM Features
*/
     if( dbg ) bcdtmWrite_message(0,0,0,"Checking Tin Feature Topology") ;
     if( bcdtmCheck_topologyDtmFeaturesDtmObject(dtmP,dbg))
       {
        bcdtmWrite_message(1,0,0,"Tin Feature Topolgy Invalid") ;
        goto errexit ;
       }
     if( dbg ) bcdtmWrite_message(0,0,0,"Tin Feature Topology Valid") ;
/*
**   Check Sort Order Dtm Object
*/
     if( dbg ) bcdtmWrite_message(0,0,0,"Checking Dtm Sort Order") ;
     if( bcdtmCheck_sortOrderDtmObject(dtmP,0) ) 
       { 
        bcdtmWrite_message(1,0,0,"Dtm Sort Order Invalid") ;
        goto errexit ; 
       }
     if( dbg ) bcdtmWrite_message(0,0,0,"Dtm Sort Order Valid") ;
/*
**   Check For Intersecting Hull Lines
*/
/*
     if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Intersecting Hull Lines") ;
     if( bcdtmCheck_forIntersectingTinHullLinesDtmObject(dtmP,0) )
       {
        bcdtmWrite_message(1,0,0,"Intersecting Hull Lines") ;
        goto errexit ; 
    } 
     if( dbg ) bcdtmWrite_message(0,0,0,"No Intersecting Hull Lines") ;
*/     
     
    } 
/*
** Clean Up
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Check DTM Object %p Tin Component Completed",dtmP) ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Check DTM Object %p Tin Component Error",dtmP) ;
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
 goto cleanup  ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmCheck_topologyDtmObject(BC_DTM_OBJ *dtmP,long writeError)
/*
**
** This Function Checks The Topology Of the DTMFeatureState::Tin
** Return Values  = 0  No Error
**                > 0  Number Of Errors
**
*/ 
{

 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  p1,p2,p3,p4,clPtr,errcnt=0 ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Only Check Tin Component Of Dtm Object
*/
  if( dtmP->dtmState == DTMState::Tin )
    {
/*
**   Process Circular List for Each Data Point
*/
     for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
       {
        clPtr = nodeAddrP(dtmP,p1)->cPtr ; 
        if( clPtr == dtmP->nullPtr )
          {
           if( dbg && writeError ) bcdtmWrite_message(0,0,0,"Warning ** No Circular List For Point %6ld",p1) ;
          }
        else if( clPtr < 0 || clPtr >= dtmP->cListPtr )
          {
           if( writeError ) bcdtmWrite_message(0,0,0,"Invalid Circular List Pointer Point %6ld",p1) ;
           ++errcnt  ;
          }
        else
          {
           p2 = clistAddrP(dtmP,clPtr)->pntNum ;
           if( (p2 = bcdtmList_nextAntDtmObject(dtmP,p1,p2)) < 0 )
             {
              if( writeError ) bcdtmWrite_message(0,0,0,"Circular List Corrupted Counter Clockwise ** %8ld %8ld",p1,p2) ;
              ++errcnt ;
             }
           else
             {
              while( clPtr != dtmP->nullPtr )
                {
                 p3 = clistAddrP(dtmP,clPtr)->pntNum ;
                 if( ( p4 = bcdtmList_nextClkDtmObject(dtmP,p3,p1)) < 0 )
                   {
                    if( writeError ) bcdtmWrite_message(0,0,0,"Circular List Corrupted Clockwise ** %8ld %8ld",p1,p2) ;
                    ++errcnt ;
                   }
                 else if( nodeAddrP(dtmP,p3)->hPtr != p1 && p4 != p2 )
                   {
                    if( writeError ) bcdtmWrite_message(0,0,0,"Circular List Corrupted ** %8ld %8ld %8ld",p1,p2,p3) ;
                    ++errcnt  ;
                   }
                 p2 = p3 ;
                 clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
                }
             }
          }
       }
    }
/*
** Print Number Of Errors
*/
 if( writeError && errcnt > 0 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Dtm Topology Errors = %6d",errcnt) ;
   }
/*
** Write Error Message
*/
 if( errcnt ) 
   {
    if( writeError ) bcdtmWrite_message(1,0,0,"Dtm Topology Invalid") ;
    goto errexit ;
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
BENTLEYDTM_Public int bcdtmCheck_precisionDtmObject(BC_DTM_OBJ *dtmP,long writeError)
/*
** This Functions Checks The Precision Of A DTM Object
*/
{
 int  ret=DTM_SUCCESS ; 
 long p1,p2,p3,clPtr,errcnt=0 ;
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Only Check Tin Component Of Dtm Object
*/
  if( dtmP->dtmState == DTMState::Tin )
    {
/*
**   Scan All Points
*/
    for ( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )
      {
       if( ( clPtr = nodeAddrP(dtmP,+p1)->cPtr ) != dtmP->nullPtr )
         {
          if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clPtr)->pntNum)) < 0 ) goto errexit  ;
          while ( clPtr != dtmP->nullPtr )
            {
             p3  = clistAddrP(dtmP,clPtr)->pntNum ;
             clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
             if( p2 > p1 && p3 > p1 && nodeAddrP(dtmP,+p1)->hPtr != p2 )
               {
                if( bcdtmMath_allPointSideOfDtmObject(dtmP,p1,p2,p3) >= 0 )
                  {
                   if( writeError )
                     {
                      bcdtmWrite_message(0,0,0,"Precision Error ** Point = %8ld %8ld %8ld",p1,p2,p3) ;
                      bcdtmList_writeCircularListForPointDtmObject(dtmP,p1) ; 
                      bcdtmList_writeCircularListForPointDtmObject(dtmP,p2) ; 
                      bcdtmList_writeCircularListForPointDtmObject(dtmP,p3) ; 
                     }
                   ++errcnt ;
                  }
               }  
             p2 = p3 ;
            }
         }
      }
   }
/*
** Print Error Statistics
*/
 if( writeError && errcnt > 0 )  bcdtmWrite_message(0,0,0,"Number Of Dtm Precision Errors = %8ld",errcnt) ;
/*
** Write Error Message
*/
 if( errcnt )
   {
    if( writeError) bcdtmWrite_message(1,0,0,"Tin Precision Invalid") ;
    goto errexit ;
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
BENTLEYDTM_Public int bcdtmCheck_topologyDtmFeaturesDtmObject(BC_DTM_OBJ *dtmP,long writeError)
/*
** This Function Checks The Connectivity Of DTM features in the DTMFeatureState::Tin
**
** Return Values  = 0 No System Errors
**                = 1 System Errors
**                = 2 Topology Errors
*/
{
 int ret=DTM_SUCCESS,err ;
 long feature,numError=0 ;
/*
**  Check Connectivity Of DTM Features
*/
 for( feature = 0  ; feature < dtmP->numFeatures ; ++feature )
   {
    if(( err = bcdtmList_checkConnectivityOfDtmFeatureDtmObject(dtmP,feature,1)) == 1 ) goto errexit ;
    if( err == 2 ) 
      {
       if( writeError ) bcdtmWrite_message(2,0,0,"Topology Error In Feature %6ld ",feature) ;
       ret = 2 ;
       ++numError ;
      }
   }
/*
** Write Message
*/
 if( writeError && numError ) bcdtmWrite_message(0,0,0,"Number Of Feature Topology Errors = %6ld",numError) ;
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
BENTLEYDTM_Public int bcdtmCheck_sortOrderDtmObject(BC_DTM_OBJ *dtmP, long writeError )
/*
** This Function Checks The Sort Order Of A Dtm Object
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  point ;
 DPoint3d *p1P,*p2P ;
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Only Check Tin Component Of Dtm Object
*/
 if( dtmP->dtmState >= DTMState::PointsSorted )
   {
/*
**   Write Points
*/
     if( dbg ) bcdtmWrite_message(0,0,0,"dtmP->numPoints = %8ld dtmP->numSortedPoints = %8ld",dtmP->numPoints,dtmP->numSortedPoints) ;
     if( dbg == 2 ) 
       {
        for( point = 0 ; point < dtmP->numPoints ; ++point )
          {
           p1P = pointAddrP(dtmP,point) ;
           bcdtmWrite_message(0,0,0,"Point = %9ld ** %15.5lf %15.8lf %10.4lf",point,p1P->x,p1P->y,p1P->z) ; 
          } 
       }
/*
**   Scan Points And Check Sort Order
*/
     p1P = dtmP->pointsPP[0] ;
     for( point = 1 ; point < dtmP->numSortedPoints && ret == DTM_SUCCESS ; ++point )
       {
        p2P = pointAddrP(dtmP,point) ;
        if( p1P->x > p2P->x || ( p1P->x == p2P->x && p1P->y > p2P->y )) 
          { 
           ret = DTM_ERROR ;
           if( writeError )
             {
              bcdtmWrite_message(0,0,0,"Points Out Of Sort Order") ;
              bcdtmWrite_message(0,0,0,"Point = %9ld ** %15.5lf %15.8lf %10.4lf",point-1,p1P->x,p1P->y,p1P->z) ; 
              bcdtmWrite_message(0,0,0,"Point = %9ld ** %15.5lf %15.8lf %10.4lf",point,p2P->x,p2P->y,p2P->z) ; 
             }
          }
        p1P = p2P ;
       }
    }
/*
** Write Error Message
*/
 if( ret != DTM_SUCCESS ) bcdtmWrite_message(1,0,0,"Dtm Sort Order Invalid") ; 
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
BENTLEYDTM_Public int bcdtmCheck_dtmFeatureEndPointsDtmObject(BC_DTM_OBJ *dtmP,long reportErrors)
/*
** This Function Checks The Point Dtm Feature List To Ensure
** That Points Marked As Dtm Feature Points Are The Correct End Point
** For The Dtm Feature
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  pnt,firstPnt,nextPnt=0,lastPnt,listPnt,listPtr,listPtr1,dtmFeature ;
/*
** Write Entry Message
*/
 if( reportErrors) dbg=DTM_TRACE_VALUE(0) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Dtm Feature End Points") ;
/*
** Check For Valid Tin Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Scan Tin Points For Feature End Points
*/
 for( pnt = 0 ; pnt < dtmP->numPoints ; ++pnt )
   {
    if( nodeAddrP(dtmP,pnt)->cPtr != dtmP->nullPtr )
      {
       listPtr = nodeAddrP(dtmP,pnt)->fPtr ;
       while ( listPtr != dtmP->nullPtr )
         {
          listPnt    = flistAddrP(dtmP,listPtr)->nextPnt ;
          dtmFeature = flistAddrP(dtmP,listPtr)->dtmFeature ; 
          listPtr    = flistAddrP(dtmP,listPtr)->nextPtr ;
/*
**        Check For Feature End Point
*/
          if( listPnt == dtmP->nullPnt )
            {
/*
**           Scan To Last Point Of Feature
*/
             lastPnt  =  dtmP->nullPnt ;
             firstPnt =  ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ;
             listPtr1 =  nodeAddrP(dtmP,firstPnt)->fPtr ;
             while ( listPtr1 != dtmP->nullPtr )
               {
                while( listPtr1 != dtmP->nullPtr && flistAddrP(dtmP,listPtr1)->dtmFeature != dtmFeature ) listPtr1 = flistAddrP(dtmP,listPtr1)->nextPtr ;
                if( listPtr1 != dtmP->nullPtr )
                  {
                   nextPnt = flistAddrP(dtmP,listPtr1)->nextPnt ;
                   if( nextPnt != dtmP->nullPnt )
                     {
                      lastPnt  = nextPnt ;
                      listPtr1 = nodeAddrP(dtmP,nextPnt)->fPtr ;
                     }
                  }
                if ( nextPnt == dtmP->nullPnt || nextPnt == firstPnt ) listPtr1 = dtmP->nullPtr ;
               }
/*
**           Check For Closed Dtm Feature
*/
             if(  nextPnt == firstPnt ) lastPnt = dtmP->nullPnt ;
/*
**           Check Last Point Corresponds To End Point Of Feature
*/
             if( lastPnt != pnt )
               {
                if( ret == DTM_SUCCESS )
                  {
                   bcdtmWrite_message(1,0,0,"Dtm Feature End Point Error") ;
                   ret = DTM_ERROR ;
                  }
                if( reportErrors )
                  {
                   bcdtmWrite_message(0,0,0,"Point %6ld Is Not A End Point Of Dtm Feature %6ld",pnt,dtmFeature) ;
                  }
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
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Dtm Feature End Points Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Dtm Feature End Points Error") ;
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
BENTLEYDTM_Public int bcdtmCheck_forKnotInDtmFeatureDtmObject(BC_DTM_OBJ *dtmP,long dtmFeature )
/*
** This Function Checks The Sort Order Of A Tin Object
*/
{
 int ret=DTM_SUCCESS ;
 long  sp,np,firstPoint ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check Feature Range
*/
 if( dtmFeature < 0 || dtmFeature >= dtmP->numFeatures ) 
   {
    bcdtmWrite_message(2,0,0,"Feature Range Error") ;
    goto errexit ;
   }
/*
** Set Feature Address
*/
 dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
/*
** Only Check If Feature In Tin 
*/
 if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
   {
/*
**  Null  Sptr Values
*/
    bcdtmList_nullSptrValuesDtmObject(dtmP) ;
/*
**  Scan Feature Points And Check For Knot In Feature
*/
    sp = firstPoint = dtmFeatureP->dtmFeaturePts.firstPoint ;
    do
      {
       if( bcdtmList_getNextPointForDtmFeatureDtmObject(dtmP,dtmFeature,sp,&np) ) goto errexit ;
       if( np != dtmP->nullPnt && np != firstPoint  )
         {
          if( nodeAddrP(dtmP,np)->sPtr != dtmP->nullPnt )
            { 
             bcdtmWrite_message(1,0,0,"Knot Detected In dtmFeature %6ld At Point %6ld",dtmFeature,np) ;
             goto errexit ;
            }
         } 
       nodeAddrP(dtmP,sp)->sPtr = np ;
       sp = np ; 
      } while ( sp != dtmP->nullPnt && sp != firstPoint )  ;
   } 
/*
** Clean Up
*/
 cleanup :
/*
** Return
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
BENTLEYDTM_Public int bcdtmCheck_tinFeatureListsDtmObject(BC_DTM_OBJ *dtmP,long reportErrors)
/*
** This Function Checks The Dtm Tin Feature Lists To Ensure :-
**
** 1. Points With A Feature List Exist In The Feature Point List
** 2. The Feature Point List Has A Corresponding Point Feature List Entry
**
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  pnt,firstPnt,nextPnt=0,flistPtr,flistPtr1,dtmFeature,pntFound ;
 long  numFeatures=0,numFeaturePts=0,numPtsWithFeatures=0,numPntFeatures=0,numKnots=0,numNextPnt=0 ;
 BC_DTM_FEATURE *dtmFeatureP = NULL; 
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Dtm Feature Lists") ;
/*
** Check For Valid Tin Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
**  Check If DTM Is In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"DTM Object %p Not Triangulated",dtmP) ;
    goto errexit ;
   }
/*
** Scan Tin Points For Features And Check Tin Point Is On Feature
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Tin Points Are On Associated Feature") ;
 for( pnt = 0 ; pnt < dtmP->numPoints ; ++pnt )
   {
/*
**  Scan Feature List For Point
*/ 
    if( nodeAddrP(dtmP,pnt)->fPtr != dtmP->nullPtr )
      {
       ++numPtsWithFeatures ;
       flistPtr = nodeAddrP(dtmP,pnt)->fPtr ;
       while ( flistPtr != dtmP->nullPtr )
         {
          nextPnt    = flistAddrP(dtmP,flistPtr)->nextPnt ;
          dtmFeature = flistAddrP(dtmP,flistPtr)->dtmFeature ; 
          flistPtr    = flistAddrP(dtmP,flistPtr)->nextPtr ;
          ++numPntFeatures ;
/*
**        Scan To Feature Point
*/
          pntFound = FALSE ;
          firstPnt = ftableAddrP(dtmP,dtmFeature)->dtmFeaturePts.firstPoint ;
          flistPtr1 = nodeAddrP(dtmP,firstPnt)->fPtr ;
          if( firstPnt == pnt ) pntFound = TRUE ;
          while ( flistPtr1 != dtmP->nullPtr && pntFound == FALSE )
            {
             while( flistPtr1 != dtmP->nullPtr && flistAddrP(dtmP,flistPtr1)->dtmFeature != dtmFeature ) flistPtr1 = flistAddrP(dtmP,flistPtr1)->nextPtr ;
             if( flistPtr1 != dtmP->nullPtr )
               {
                nextPnt  = flistAddrP(dtmP,flistPtr1)->nextPnt ;
                flistPtr1 = nodeAddrP(dtmP,nextPnt)->fPtr ;
                if( nextPnt == pnt ) pntFound = TRUE ;
                if ( nextPnt == dtmP->nullPnt || nextPnt == firstPnt ) flistPtr1 = dtmP->nullPtr ;
               }
            } 
         
/*
**        Check Point Found
*/
          if( pntFound == FALSE )
            {
             if( reportErrors ) bcdtmWrite_message(2,0,0,"Point %6ld Is Not In Dtm Feature %6ld",pnt,dtmFeature) ;
             ret = DTM_ERROR ;
            } 
         }
      }
   }
/*
** Scan Feature Points And Check Tin Point Has A Corresponding Feature Entry
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Dtm Features Points Have An Associated Point Feature List") ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && ( pnt = dtmFeatureP->dtmFeaturePts.firstPoint ) != dtmP->nullPnt )
      {
       ++numFeatures ; 
       do
         {
          pntFound = FALSE ;
          nextPnt  = dtmP->nullPnt ;
          flistPtr1 = nodeAddrP(dtmP,pnt)->fPtr ;
          while( flistPtr1 != dtmP->nullPtr && flistAddrP(dtmP,flistPtr1)->dtmFeature != dtmFeature ) flistPtr1 = flistAddrP(dtmP,flistPtr1)->nextPtr ;
          if( flistPtr1 != dtmP->nullPtr )
            {
             pntFound = TRUE ;
             nextPnt  = flistAddrP(dtmP,flistPtr1)->nextPnt ;
             ++numFeaturePts ;
            }
/*
**        Check Point Has Feature List For Feature
*/
          if( pntFound == FALSE )
            {
             if( reportErrors ) bcdtmWrite_message(2,0,0,"Dtm Feature %6ld Has No Feature List Entry For Point %6ld",dtmFeature,pnt) ;
             ret = DTM_ERROR ;
            }
/*
**        Set Next Point
*/
          pnt = nextPnt ;
         } while ( pnt != dtmP->nullPnt && pnt != dtmFeatureP->dtmFeaturePts.firstPoint ) ;  
      }
   }
/*
** Check For Knots In DTM Features
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Knots In DTM Features") ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && ( pnt = dtmFeatureP->dtmFeaturePts.firstPoint ) != dtmP->nullPnt )
      {
       if( bcdtmCheck_forKnotInDtmFeatureDtmObject(dtmP,dtmFeature) ) 
         {
          ++numKnots ;
          if( reportErrors ) bcdtmWrite_message(2,0,0,"Knot Detected In Dtm Feature = %6ld",dtmFeature) ;
          ret = DTM_ERROR ;
         }
      }
   }
/*
** Check For Next Point Same As Current Point
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Next Point Errors") ;
 for( pnt = 0 ; pnt < dtmP->numPoints ; ++pnt )
   {
    if( nodeAddrP(dtmP,pnt)->fPtr != dtmP->nullPtr )
      {
       ++numPtsWithFeatures ;
       flistPtr = nodeAddrP(dtmP,pnt)->fPtr ;
       while ( flistPtr != dtmP->nullPtr )
         {
          nextPnt    = flistAddrP(dtmP,flistPtr)->nextPnt ;
          dtmFeature = flistAddrP(dtmP,flistPtr)->dtmFeature ; 
          flistPtr    = flistAddrP(dtmP,flistPtr)->nextPtr ;
          if( nextPnt == pnt ) 
            {
             if( reportErrors ) bcdtmWrite_message(2,0,0,"pnt = %9ld ** nextPnt = %9ld dtmFeature = %6ld",pnt,nextPnt,dtmFeature) ;
             ret = DTM_ERROR ;
             ++numNextPnt ; 
            }
         }
      }
   }
/*
** Write Statistics
*/
 if( reportErrors )
   {
    bcdtmWrite_message(0,0,0,"Number Of Dtm Features           = %9ld",numFeatures) ;
    bcdtmWrite_message(0,0,0,"Number Of Dtm Features Points    = %9ld",numFeaturePts) ;
    bcdtmWrite_message(0,0,0,"Number Of Points With Features   = %9ld",numPtsWithFeatures) ;
    bcdtmWrite_message(0,0,0,"Number Of Point Feature Lists    = %9ld",numPntFeatures) ;
    bcdtmWrite_message(0,0,0,"Number Of Feature Knots Detected = %9ld",numKnots) ;
    bcdtmWrite_message(0,0,0,"Number Of Next Point Errors      = %9ld",numNextPnt) ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Dtm Feature Lists Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Dtm Feature Lists Error") ;
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
BENTLEYDTM_Public int bcdtmCheck_forPointOffsetIndexRangeErrorsDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Checks The Point Indexes Of A Dtm Object For Range Errors
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  n,dtmFeature ;
 char  dtmFeatureTypeName[50] ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Only Check If Features Present
*/
 if( dtmP->numFeatures > 0 )
   {
    for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
      {
       dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
       if( dtmFeatureP->dtmFeatureState == DTMFeatureState::OffsetsArray )
         {
         long* offsetP = bcdtmMemory_getPointerOffset(dtmP,dtmFeatureP->dtmFeaturePts.offsetPI);
         
          for( n = 0 ; n < dtmFeatureP->numDtmFeaturePts ; ++n )
            {
             if( offsetP[n] < 0 || offsetP[n] >= dtmP->numPoints )
               {
                if( dbg ) 
                  {
                   bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName) ;
                   bcdtmWrite_message(0,0,0,"dtmFeature = %6ld Type = %s Point = %4ld Index = %8ld",dtmFeature,dtmFeatureTypeName,n,offsetP[n]) ;
                  }
                bcdtmWrite_message(2,0,0,"Point Index Range Error") ;
                ret = DTM_ERROR ;
               }
            }
         }
      }
/*
**  Check For Error
*/
    if( ret != DTM_SUCCESS ) goto errexit ;
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
BENTLEYDTM_Public int bcdtmCheck_forDuplicatePointOffsetPointersDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Checks The Point Indexes Of A Dtm Object For Range Errors
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  dtmFeature1,dtmFeature2 ;
 BC_DTM_FEATURE *dtmFeature1P,*dtmFeature2P ;
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Only Check If Features Present
*/
 if( dtmP->numFeatures > 0 )
   {
    for( dtmFeature1 = 0 ; dtmFeature1 < dtmP->numFeatures ; ++dtmFeature1 )
      {
       dtmFeature1P = ftableAddrP(dtmP,dtmFeature1) ;
       if( dtmFeature1P->dtmFeatureState == DTMFeatureState::OffsetsArray )
         {
          for( dtmFeature2 = dtmFeature1+1 ; dtmFeature2 < dtmP->numFeatures ; ++dtmFeature2 )
            {
             dtmFeature2P = ftableAddrP(dtmP,dtmFeature2) ;
             if( dtmFeature2P->dtmFeatureState == DTMFeatureState::OffsetsArray )
               {
                if( bcdtmMemory_getPointerOffset(dtmP,dtmFeature2P->dtmFeaturePts.offsetPI) == bcdtmMemory_getPointerOffset(dtmP,dtmFeature1P->dtmFeaturePts.offsetPI) )
                  {
                   if( dbg )
                     {
                      bcdtmWrite_message(0,0,0,"dtmFeature1 = %4ld offset1P = %p ** dtmFeature2 = %4ld offset2P = %p",dtmFeature1,dtmFeature1P->dtmFeaturePts.offsetPI,dtmFeature2,dtmFeature2P->dtmFeaturePts.offsetPI) ;
                     }
                   bcdtmWrite_message(2,0,0,"Duplicate Point Offset Index Memory Pointers") ;
                   ret = DTM_ERROR ;
                  }
               }  
            } 
         }
      }
/*
**  Check For Error
*/
    if( ret != DTM_SUCCESS ) goto errexit ;
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
BENTLEYDTM_Public int bcdtmCheck_hullFeatureStateDtmObject(BC_DTM_OBJ *dtmP)
/*
** This Function Checks The HULL Feature State For A Triangulated DTM
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  dtmFeature ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Only Check For Triangulated DTM
*/
 if( dtmP->dtmState == DTMState::Tin )
   { 
/*
**  Only Check If Features Present
*/
    if( dtmP->numFeatures > 0 )
      {
       for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
          if( dtmFeatureP->dtmFeatureType == DTMFeatureType::Hull )
            {
             if( dtmFeatureP->dtmFeatureState != DTMFeatureState::Tin )
               {
                if( dbg )
                  {
                   bcdtmWrite_message(0,0,0,"dtmFeature = %4ld ** Type = DTMFeatureType::Hull ** dtmFeature State = %2ld offsetP = %p",dtmFeature,dtmFeatureP->dtmFeatureState,dtmFeatureP->dtmFeaturePts.offsetPI) ;
                  }
                bcdtmWrite_message(1,0,0,"Error In Hull Feature State") ;
                goto errexit ;
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
BENTLEYDTM_Public int bcdtmCheck_forTinFeatureErrorsDtmObject(BC_DTM_OBJ *dtmP,long messageFlag,long *numTinErrorsP)
/*
** This Function Checks The HULL Feature State For A Triangulated DTM
*/
{
 int   ret=DTM_SUCCESS ;
 long  dtmFeature ;
 char  dtmFeatureTypeName[50] ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Initialise
*/
 *numTinErrorsP = 0 ;
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Only Check For Triangulated DTM
*/
 if( dtmP->dtmState == DTMState::Tin )
   { 
/*
**  Only Check If Features Present
*/
    if( dtmP->numFeatures > 0 )
      {
       for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
          if( dtmFeatureP->dtmFeatureState == DTMFeatureState::TinError )
            { 
             ++*numTinErrorsP ;
             if( messageFlag )
               {
                bcdtmData_getDtmFeatureTypeNameFromDtmFeatureType(dtmFeatureP->dtmFeatureType,dtmFeatureTypeName);
                bcdtmWrite_message(0,0,0,"Tin Feature Error ** Feature Type = %30s ** numFeaturePts = %6ld",dtmFeatureTypeName,dtmFeatureP->numDtmFeaturePts) ;
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
BENTLEYDTM_Public int bcdtmCheck_forIntersectingTinHullLinesDtmObject(BC_DTM_OBJ *dtmP,long messageFlag)
/*
** This Function Checks For Intersecting dtmP Hull Lines
*/
{
 int    dbg=DTM_TRACE_VALUE(0),err,sd1,sd2 ;
 long   hp1,hp2,lp1,lp2,hullPoint=0 ;
 double xhn,xhm,yhn,yhm,xln,yln,xlm,ylm ;
/*
** Write Out Hull Points Develeopment Only
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking For Intersecting Hull Lines") ;
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Hull Points And Coordinates") ;
    bcdtmWrite_message(0,0,0,"===========================") ;
    hp1 = dtmP->hullPoint ;
    do
      {
       bcdtmWrite_message(0,0,0,"HullPoint[%8ld] = %6ld ** %15.8lf %15.8lf %10.4lf",hullPoint,hp1,pointAddrP(dtmP,hp1)->x,pointAddrP(dtmP,hp1)->y,pointAddrP(dtmP,hp1)->z) ;
       hp1 = nodeAddrP(dtmP,hp1)->hPtr ;
       ++hullPoint ;
      } while ( hp1 != dtmP->hullPoint ) ;
    bcdtmWrite_message(0,0,0,"HullPoint[%8ld] = %6ld ** %15.8lf %15.8lf %10.4lf",hullPoint,hp1,pointAddrP(dtmP,hp1)->x,pointAddrP(dtmP,hp1)->y,pointAddrP(dtmP,hp1)->z) ;
   }
/*
** Scan Hull And Check For Intersecting dtmP Hull Lines
*/
 err=0 ;
 hp1 = dtmP->hullPoint ;
 do
   {
    hp2 = nodeAddrP(dtmP,hp1)->hPtr ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Hull Line %8ld %8ld",hp1,hp2) ;
    if( pointAddrP(dtmP,hp1)->x <= pointAddrP(dtmP,hp2)->x ) { xhn = pointAddrP(dtmP,hp1)->x ; xhm = pointAddrP(dtmP,hp2)->x ; }
    else                                                     { xhm = pointAddrP(dtmP,hp1)->x ; xhn = pointAddrP(dtmP,hp2)->x ; }
    if( pointAddrP(dtmP,hp1)->y <= pointAddrP(dtmP,hp2)->y ) { yhn = pointAddrP(dtmP,hp1)->y ; yhm = pointAddrP(dtmP,hp2)->y ; }
    else                                                     { yhm = pointAddrP(dtmP,hp1)->y ; yhn = pointAddrP(dtmP,hp2)->y ; }
    xhn = xhn - 0.001 ; yhn = yhn - 0.001 ;
    xhm = xhm + 0.001 ; yhm = yhm + 0.001 ; 
/*
** Scan All Hull Lines Looking For Intersection
*/
    lp1 = dtmP->hullPoint ;
    do
      {
       lp2 = nodeAddrP(dtmP,lp1)->hPtr ;
       if( lp1 != hp1 && lp1 != hp2 && lp2 != hp1 && lp2 != hp2 )
         {
          if( pointAddrP(dtmP,lp1)->x <= pointAddrP(dtmP,lp2)->x ) { xln = pointAddrP(dtmP,lp1)->x ; xlm = pointAddrP(dtmP,lp2)->x ; }
          else                                                     { xlm = pointAddrP(dtmP,lp1)->x ; xln = pointAddrP(dtmP,lp2)->x ; }
          if( pointAddrP(dtmP,lp1)->y <= pointAddrP(dtmP,lp2)->y ) { yln = pointAddrP(dtmP,lp1)->y ; ylm = pointAddrP(dtmP,lp2)->y ; }
          else                                                     { ylm = pointAddrP(dtmP,lp1)->y ; yln = pointAddrP(dtmP,lp2)->y ; }
          if( xln <= xhm && xlm >= xhn && yln <= yhm  && ylm >= yhn  )
            { 
             sd1 = bcdtmMath_sideOf(pointAddrP(dtmP,hp1)->x,pointAddrP(dtmP,hp1)->y,pointAddrP(dtmP,hp2)->x,pointAddrP(dtmP,hp2)->y,pointAddrP(dtmP,lp1)->x,pointAddrP(dtmP,lp1)->y) ;
             sd2 = bcdtmMath_sideOf(pointAddrP(dtmP,hp1)->x,pointAddrP(dtmP,hp1)->y,pointAddrP(dtmP,hp2)->x,pointAddrP(dtmP,hp2)->y,pointAddrP(dtmP,lp2)->x,pointAddrP(dtmP,lp2)->y) ;
             if( sd1 != sd2 && sd1 != 0 && sd2 != 0 )
               {
                sd1 = bcdtmMath_sideOf(pointAddrP(dtmP,lp1)->x,pointAddrP(dtmP,lp1)->y,pointAddrP(dtmP,lp2)->x,pointAddrP(dtmP,lp2)->y,pointAddrP(dtmP,hp1)->x,pointAddrP(dtmP,hp1)->y) ;
                sd2 = bcdtmMath_sideOf(pointAddrP(dtmP,lp1)->x,pointAddrP(dtmP,lp1)->y,pointAddrP(dtmP,lp2)->x,pointAddrP(dtmP,lp2)->y,pointAddrP(dtmP,hp2)->x,pointAddrP(dtmP,hp2)->y) ;
                if( sd1 != sd2 && sd1 != 0 && sd2 != 0 )
                  {
                   ++err ;
                   if( messageFlag )
                     {
                      bcdtmWrite_message(0,0,0,"Hp1 = %6ld  Hp2 = %6ld",hp1,hp2) ;
                      bcdtmWrite_message(0,0,0,"Lp1 = %6ld  Lp2 = %6ld",lp1,lp2) ;
                      bcdtmWrite_message(0,0,0,"Intersected Hull Line ** %10.4lf %10.4lf ** %10.4lf %10.4lf",pointAddrP(dtmP,hp1)->x,pointAddrP(dtmP,hp1)->y,pointAddrP(dtmP,hp2)->x,pointAddrP(dtmP,hp2)->y) ;
                      bcdtmWrite_message(0,0,0,"                      ** %10.4lf %10.4lf ** %10.4lf %10.4lf",pointAddrP(dtmP,lp1)->x,pointAddrP(dtmP,lp1)->y,pointAddrP(dtmP,lp2)->x,pointAddrP(dtmP,lp2)->y) ;
                      bcdtmWrite_message(0,0,0,"") ;
                     } 
                  }
               }
            }
         }
       lp1 = lp2 ; 
      } while ( lp1 != dtmP->hullPoint ) ;
/*
** Set For Next Hull Line
*/
    hp1 = hp2 ; 
   } while ( hp1 != dtmP->hullPoint ) ;
/*
** Job Completed
*/
 if( dbg && ! err ) bcdtmWrite_message(0,0,0,"Checking For Intersecting Hull Lines Completed") ;
 if( dbg &&   err ) bcdtmWrite_message(0,0,0,"Checking For Intersecting Hull Lines Error") ;
 return(err) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmCheck_zeroSlopePolygonsDtmObject
(
 BC_DTM_OBJ *dtmP
)
/*
** This Function Checks Zero Slope Polygons In A Triangulated DTM
*/
{
 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  ap,cp,pnt,nextPnt,dtmFeature,startPoint,hullLine ;
 DTMDirection direction;
 long  numZeroSlopePolygons,numClockWisePolygons,numAntClockWisePolygons,numErrors=0 ;
 double apz,cpz,area,elevation ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Check For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Only Check For Triangulated DTM
*/
 if( dtmP->dtmState == DTMState::Tin )
   { 
/*
**  Only Check If Features Present
*/
    if( dtmP->numFeatures > 0 )
      {
      
//     Count Number Of Zero Slope Polygons
      
       numZeroSlopePolygons = 0 ;
       numClockWisePolygons = 0 ;
       numAntClockWisePolygons = 0 ;
       for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
          if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeatureType == DTMFeatureType::ZeroSlopePolygon )
            { 
             ++numZeroSlopePolygons ;
             if( dtmFeatureP->internalToDtmFeature ) ++numClockWisePolygons ;
             else                                    ++numAntClockWisePolygons ;
            } 
         }
         
//     Log Counts
         
       if( dbg )
         {
          bcdtmWrite_message(0,0,0,"Number Of Zero Slope Polygons         = %8ld",numZeroSlopePolygons) ;
          bcdtmWrite_message(0,0,0,"Number Of Clock Wise Polygons         = %8ld",numClockWisePolygons) ;
          bcdtmWrite_message(0,0,0,"Number Of Counter Clock Wise Polygons = %8ld",numAntClockWisePolygons) ;
         }
         
//     Check Edges Of Zero Slope Polygons

       numErrors = 0 ; 
       for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
         {
          dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
          if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin && dtmFeatureP->dtmFeatureType == DTMFeatureType::ZeroSlopePolygon )
            {
             if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,dtmFeature,&startPoint)) goto errexit ; 
             if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPoint,&area,&direction)) goto errexit ;
             if( direction == DTMDirection::Clockwise && dtmFeatureP->internalToDtmFeature != 1 )
               {
                bcdtmWrite_message(1,0,0,"Zero Slope Polygon Direction Not Set Correctly To Clockwise") ;
                ++numErrors ;
               }
             else if( direction == DTMDirection::AntiClockwise && dtmFeatureP->internalToDtmFeature != 0 )
               {
                bcdtmWrite_message(1,0,0,"Zero Slope Polygon Direction Not Set Correctly To Counter Clockwise ** dtmFeatureP->internalToDtmFeature = %2ld",dtmFeatureP->internalToDtmFeature) ;
                ++numErrors ;
               }
               
//          Check Elevation 

            pnt = startPoint ;
            elevation = pointAddrP(dtmP,startPoint)->z ;
            do
              {
               if( pointAddrP(dtmP,pnt)->z != elevation )
                 {
                  bcdtmWrite_message(1,0,0,"Zero Slope Polygon Elevation Error ** Elevation = %10.4lf pnt->z = %10.4lf",elevation,pointAddrP(dtmP,pnt)->z) ;
                  ++numErrors ;
                 }
               pnt = nodeAddrP(dtmP,pnt)->tPtr ;   
              } while( pnt != startPoint ) ;

//          Check For Zero Edge Errors              

            pnt = startPoint ;
            do
              {
               nextPnt = nodeAddrP(dtmP,pnt)->tPtr ;
               if(( ap = bcdtmList_nextAntDtmObject(dtmP,pnt,nextPnt)) < 0 ) goto errexit ;
               if(( cp = bcdtmList_nextClkDtmObject(dtmP,pnt,nextPnt)) < 0 ) goto errexit ;
               if( ! bcdtmList_testLineDtmObject(dtmP,ap,nextPnt)) ap = dtmP->nullPnt ;
               if( ! bcdtmList_testLineDtmObject(dtmP,cp,nextPnt)) cp = dtmP->nullPnt ;
               apz = cpz = dtmP->zMin - 2.0 *( dtmP->zMax - dtmP->zMin ) ;
               if( ap != dtmP->nullPnt ) apz = pointAddrP(dtmP,ap)->z ;
               if( cp != dtmP->nullPnt ) cpz = pointAddrP(dtmP,cp)->z ;
               if( ap != dtmP->nullPnt && cp != dtmP->nullPnt )
                 {
                  if( bcdtmList_checkForLineOnHullLineDtmObject(dtmP,pnt,nextPnt,&hullLine)) goto errexit ;
                  if( apz == cpz && ! hullLine )
                    {
                     bcdtmWrite_message(1,0,0,"Zero Slope Polygon Zero Edge 1 Error") ;
                     ++numErrors ;
                    }
                 }
                 
               if( ap != dtmP->nullPnt && cp == dtmP->nullPnt )
                 {
                  if( direction == DTMDirection::Clockwise && apz != elevation )
                    {
                     bcdtmWrite_message(1,0,0,"Zero Slope Polygon Zero Edge 2 Error") ;
                     ++numErrors ;
                    }
                  else if (direction == DTMDirection::AntiClockwise && apz != elevation)
                    {
                     bcdtmWrite_message(1,0,0,"Zero Slope Polygon Zero Edge 3 Error") ;
                     ++numErrors ;
                    }
                 }
                 
               if( ap == dtmP->nullPnt && cp != dtmP->nullPnt )
                 {
                  if( direction == DTMDirection::Clockwise && cpz == elevation )
                    {
                     bcdtmWrite_message(1,0,0,"Zero Slope Polygon Zero Edge 4 Error") ;
                     ++numErrors ;
                    }
                  else if (direction == DTMDirection::AntiClockwise && cpz == elevation)
                    {
                     bcdtmWrite_message(1,0,0,"Zero Slope Polygon Zero Edge 5 Error") ;
                     ++numErrors ;
                    }
                 }
                 
               pnt = nextPnt ;   
              } while( pnt != startPoint ) ;

//          Null Tptr List

            if( bcdtmList_nullTptrListDtmObject(dtmP,startPoint)) goto errexit ;
               
            }
         } 
       if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Errors = %8ld",numErrors) ;    
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
BENTLEYDTM_Public int bcdtmCheck_topologyRangeDtmObject(BC_DTM_OBJ *dtmP,long firstPoint,long lastPoint,long writeError)
/*
**
** This Function Checks The Topology Of the DTMFeatureState::Tin
** Return Values  = 0  No Error
**                > 0  Number Of Errors
**
*/ 
{

 int   ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long  p1,p2,p3,p4,clPtr,errcnt=0 ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Topology Range Dtm Object") ;
    bcdtmWrite_message(0,0,0,"dtmP       = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"firstPoint = %8ld",firstPoint) ;
    bcdtmWrite_message(0,0,0,"lastPoint  = %8ld",lastPoint) ;
    bcdtmWrite_message(0,0,0,"writeError = %8ld",writeError) ;
   } 
/*
** Test For Valid Dtm Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Only Check Tin Component Of Dtm Object
*/
  if( dtmP->dtmState == DTMState::Tin )
    {
/*
**   Process Circular List for Each Data Point
*/
     for( p1 = firstPoint ; p1 < lastPoint ; ++p1 )
       {
        clPtr = nodeAddrP(dtmP,p1)->cPtr ; 
        if( clPtr == dtmP->nullPtr )
          {
           if( dbg && writeError ) bcdtmWrite_message(0,0,0,"Warning ** No Circular List For Point %6ld",p1) ;
          }
        else if( clPtr < 0 || clPtr >= dtmP->cListPtr )
          {
           if( writeError ) bcdtmWrite_message(0,0,0,"Invalid Circular List Pointer Point %6ld",p1) ;
           ++errcnt  ;
          }
        else
          {
           p2 = clistAddrP(dtmP,clPtr)->pntNum ;
           if( (p2 = bcdtmList_nextAntDtmObject(dtmP,p1,p2)) < 0 )
             {
              if( writeError ) bcdtmWrite_message(0,0,0,"Circular List Corrupted Counter Clockwise ** %8ld %8ld",p1,p2) ;
              ++errcnt ;
             }
           else
             {
              while( clPtr != dtmP->nullPtr )
                {
                 p3 = clistAddrP(dtmP,clPtr)->pntNum ;
                 if( ( p4 = bcdtmList_nextClkDtmObject(dtmP,p3,p1)) < 0 )
                   {
                    if( writeError ) bcdtmWrite_message(0,0,0,"Circular List Corrupted Clockwise ** %8ld %8ld",p1,p2) ;
                    ++errcnt ;
                   }
                 else if( nodeAddrP(dtmP,p3)->hPtr != p1 && p4 != p2 )
                   {
                    if( writeError ) bcdtmWrite_message(0,0,0,"Circular List Corrupted ** %8ld %8ld %8ld",p1,p2,p3) ;
                    ++errcnt  ;
                   }
                 p2 = p3 ;
                 clPtr = clistAddrP(dtmP,clPtr)->nextPtr ;
                }
             }
          }
       }
    }
/*
** Print Number Of Errors
*/
 if( writeError && errcnt > 0 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Dtm Topology Errors = %6d",errcnt) ;
   }
/*
** Write Error Message
*/
 if( errcnt ) 
   {
    if( writeError ) bcdtmWrite_message(1,0,0,"Dtm Topology Invalid") ;
    goto errexit ;
   }  
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Topology Range Dtm Object Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Checking Topology Range Dtm Object Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
