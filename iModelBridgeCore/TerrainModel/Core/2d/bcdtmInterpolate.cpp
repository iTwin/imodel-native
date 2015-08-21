/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmInterpolate.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
#include "bcdtminlines.h" 
//#pragma optimize( "p", on )
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmInterpolate_dtmFeatureTypeDtmObject
(
 BC_DTM_OBJ  *dtmP,                       // ==> Contains Linear DTM Features To Be Interpolated
 DTMFeatureType dtmFeatureType,                    // ==> Dtm Feature Type To Be Interpolated
 double snapTolerance,                    // ==> Snap tolerance for interpolation purposes
 BC_DTM_OBJ   *spotsP,                    // ==> 3D Points To Interpolate From
 BC_DTM_OBJ   *intDtmP,                   // ==> DTM to Store Interpolated Features
 long  *numDtmFeaturesP,                  // <== Number Of Dtm Features
 long  *numDtmFeaturesInterpolatedP       // <== Number Of Dtm Features Interpolated
)
/*
** This Method Interpolates Linear Features From 3D Points
*/
{
 int       ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long      dtmFeature,numFeaturePts,interpolateResult,numDuplicates ;
 long      numDtmFeatures,numDtmFeaturesInterpolated ;
 DPoint3d       *featurePtsP=NULL ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Initilaise Variables
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Interpolate DTM Feature Type") ;
    bcdtmWrite_message(0,0,0,"dtmP            = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"dtmFeatureType  = %8ld",dtmFeatureType) ;
    bcdtmWrite_message(0,0,0,"snapTolerance   = %8.3lf",snapTolerance) ;
    bcdtmWrite_message(0,0,0,"spotsP          = %p",spotsP) ;
    bcdtmWrite_message(0,0,0,"intDtmP         = %p",intDtmP) ;
   }
/*
** Validate Parameters
*/
 *numDtmFeaturesP = 0 ;
 *numDtmFeaturesInterpolatedP = 0 ;
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit  ;
 if( bcdtmObject_testForValidDtmObject(spotsP)) goto errexit  ;
 if( bcdtmObject_testForValidDtmObject(intDtmP)) goto errexit  ;
 if( dtmP->dtmState != DTMState::Data || spotsP->dtmState != DTMState::Data )
   {
    bcdtmWrite_message(1,0,0,"Method Requires Untriangulated DTM") ;
    goto errexit ;
   }
/*
** Sort Spots And Remove Duplicate Points
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Sorting Spots")  ;
 if( bcdtmObject_sortDtmObject(spotsP)) goto errexit ;
 spotsP->numSortedPoints = spotsP->numPoints ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Duplicates DTM Object") ;
 if( bcdtmObject_removeDuplicatesDtmObject(spotsP,&numDuplicates)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Duplicates = %8ld",numDuplicates) ;
 spotsP->numSortedPoints = spotsP->numPoints ;
 spotsP->dtmState = DTMState::Data ;
/*
**  Scan And Interpolate Dtm Features
*/
 numDtmFeatures = 0 ;
 numDtmFeaturesInterpolated = 0 ;
 for( dtmFeature = 0 ; dtmFeature < dtmP->numFeatures ; ++dtmFeature )
   {
    dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
    if( dbg ) bcdtmWrite_message(0,0,0,"dtmFeature = %8ld ** Type = %2ld State = %2ld",dtmFeature,dtmFeatureP->dtmFeatureType,dtmFeatureP->dtmFeatureState) ; 
    if( dtmFeatureP->dtmFeatureType == dtmFeatureType && dtmFeatureP->dtmFeatureState == DTMFeatureState::Data )
      {
       ++numDtmFeatures ;
       if( bcdtmList_copyDtmFeaturePointsToPointArrayDtmObject(dtmP,dtmFeature,&featurePtsP,&numFeaturePts )) goto errexit ;
       if( numFeaturePts > 1 )
         {
          if( bcdtmInterpolate_linearFeatureDtmObject(spotsP,&featurePtsP,&numFeaturePts,snapTolerance,&interpolateResult)) goto errexit ;
          if( interpolateResult )
            {
             ++numDtmFeaturesInterpolated ;
             if( bcdtmObject_storeDtmFeatureInDtmObject(intDtmP,dtmFeatureType,dtmFeatureP->dtmUserTag,1,&dtmFeatureP->dtmUserTag,featurePtsP,numFeaturePts)) goto errexit ;
            } 
         }
      }
   }
/*
** Set Return Values
*/
 *numDtmFeaturesP = numDtmFeatures ;
 *numDtmFeaturesInterpolatedP = numDtmFeaturesInterpolated ;
/*
** Clean Up
*/
 cleanup :
 if( featurePtsP != NULL ) { free(featurePtsP) ; featurePtsP = NULL ; }
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Interpolate DTM Feature Type Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Interpolate DTM Feature Type Error") ;
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
BENTLEYDTM_Private int bcdtmInterpolate_intPointCompareFunction(const void *void1P, const void *void2P)
{
 struct  IntPoint { long intFlag , pointCode ; double len,x,y,z ; } *int1P,*int2P ;
 int1P = ( struct  IntPoint * ) void1P ;
 int2P = ( struct  IntPoint * ) void2P ;
 if( int1P->len < int2P->len ) return(-1) ;
 if( int1P->len > int2P->len ) return(1) ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmInterpolate_linearFeatureDtmObject
(
 BC_DTM_OBJ *spotsDtmP,
 DPoint3d    **pointsPP,
 long   *numPointsP,
 double snapTolerance,
 long   *interpolationResultP
 )
/*
** This Function Interpolates Spots Onto A Linear Feature
*/
{
 int     ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0);
 long    p,ps,pe,pnt1,pnt2,onLine,intFlag ;
 long    numIntPoints=0,memIntPoints=100,memIntPointsInc=100 ;
 long    numAllPoints=0,memAllPoints=0,pointsInterpolated ;
 double  xs,ys,xe,ye,ds,de,dm,xMin,xMax,yMin,yMax,xn,yn ;
 double  deltaZ,segmentLength,interpolateLength ;
 DPoint3d     *p3dP ;
 DTM_TIN_POINT *pntP ;
 struct  IntPoint { long intFlag , pointCode ; double len,x,y,z ; } *intP,*int1P,*int2P,*int3P,*intLowP,*intHighP,*intPointsP=NULL,*allPointsP=NULL ;
/*
** Write Entry Message
*/ 
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Interpolate Linear Feature") ;
    bcdtmWrite_message(0,0,0,"spotsDtmP             = %p",spotsDtmP) ;
    bcdtmWrite_message(0,0,0,"*pointsPP             = %p",*pointsPP) ;
    bcdtmWrite_message(0,0,0,"*numPointsP           = %8ld",*numPointsP) ;
    bcdtmWrite_message(0,0,0,"snapTolerance         = %8.3lf",snapTolerance) ;
    bcdtmWrite_message(0,0,0,"*interpolationResultP = %8ld",*interpolationResultP) ;
   }
/*
** Write Out Linear Feature Points
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Linear Feature Points = %8ld",*numPointsP) ;
    for( p3dP = *pointsPP ; p3dP < *pointsPP + *numPointsP ; ++p3dP )
      {
       bcdtmWrite_message(0,0,0,"Point[%5ld] = %12.5lf %12.5lf",(long)(p3dP-*pointsPP),p3dP->x,p3dP->y) ;
      } 
   } 
/*
** Initialise
*/
 *interpolationResultP = 0 ;
/*
** Allocate Memory For Interpolation Points
*/
 intPointsP = ( struct IntPoint * ) malloc( memIntPoints * sizeof(struct IntPoint)) ;
 if( intPointsP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   } 
/*
** Process Each Line Segment At a Time
*/
 numIntPoints = 0 ;
 for( p3dP = *pointsPP ; p3dP < *pointsPP + *numPointsP - 1 ; ++p3dP )
   {
    if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Processing Segment %4ld",(long)(p3dP-*pointsPP)) ;
    xs = p3dP->x     ; ys = p3dP->y ;
    xe = (p3dP+1)->x ; ye = (p3dP+1)->y ;
    if( xs != xe || ys != ye  )
      {
       if( xs <= xe ) { xMin = xs ; xMax = xe ; }
       else           { xMin = xe ; xMax = xs ; }
       if( ys <= ye ) { yMin = ys ; yMax = ye ; }
       else           { yMin = ye ; yMax = ys ; }
       xMin -= snapTolerance ; xMax += snapTolerance ;
       yMin -= snapTolerance ; yMax += snapTolerance ;
/*
**     Get Closest Spots To Line End Points
*/
       bcdtmInterpolate_findClosestPointDtmObject(spotsDtmP,xs,ys,&ps) ;
       bcdtmInterpolate_findClosestPointDtmObject(spotsDtmP,xe,ye,&pe) ;
/*
**     Get Distance Of Closest Points
*/
       ds = bcdtmInterpolate_getDistanceFromPointDtmObject(spotsDtmP,ps,xs,ys) ;
       de = bcdtmInterpolate_getDistanceFromPointDtmObject(spotsDtmP,pe,xe,ye) ;
/*
**     Write Out Closest Points
*/
       if( dbg == 2 )
         {
          bcdtmWrite_message(0,0,0,"xs = %12.5lf ys = %12.5lf ** ps = %8ld psx = %12.5lf psy = %12.5lf psz = %10.4lf ** ds = %10.4lf",xs,ys,ps,pointAddrP(spotsDtmP,ps)->x,pointAddrP(spotsDtmP,ps)->y,pointAddrP(spotsDtmP,ps)->z,ds) ;
          bcdtmWrite_message(0,0,0,"xe = %12.5lf ye = %12.5lf ** pe = %8ld pex = %12.5lf pey = %12.5lf pez = %10.4lf ** de = %10.4lf",xe,ye,pe,pointAddrP(spotsDtmP,pe)->x,pointAddrP(spotsDtmP,pe)->y,pointAddrP(spotsDtmP,pe)->z,de) ;
         }
/*
**    Get Points For Scan Of Minimum Bounding Rectangle
*/
       pnt1 = ps ; 
       pnt2 = pe ;
       if( dbg )
         {
          if( pnt1 < 0 || pnt1 >= spotsDtmP->numPoints ) bcdtmWrite_message(0,0,0," pnt1 = %6ld Line Range Value ** spotsDtmP->numPts = %6ld",pnt1,spotsDtmP->numPoints ) ;
          if( pnt2 < 0 || pnt2 >= spotsDtmP->numPoints ) bcdtmWrite_message(0,0,0," pnt2 = %6ld Line Range Value ** spotsDtmP->numPts = %6ld",pnt2,spotsDtmP->numPoints ) ;
         }  
       if( xs < xe || ( xs == xe && ys < ye ) ) 
         { 
          while ( pnt1 > 0                        &&  ( xs - pointAddrP(spotsDtmP,pnt1)->x ) < snapTolerance ) --pnt1 ;
          while ( pnt2 < spotsDtmP->numPoints - 1 &&  ( pointAddrP(spotsDtmP,pnt2)->x - xe ) < snapTolerance ) ++pnt2 ;
         }
       else
         { 
          while ( pnt1 < spotsDtmP->numPoints - 1 &&  ( pointAddrP(spotsDtmP,pnt1)->x - xe ) < snapTolerance ) ++pnt1 ;
          while ( pnt2 > 0                        &&  ( xs - pointAddrP(spotsDtmP,pnt2)->x ) < snapTolerance ) --pnt2 ;
         }
       if( dbg )
         {
          if( pnt1 < 0 || pnt1 >= spotsDtmP->numPoints ) bcdtmWrite_message(0,0,0,"pnt1 = %6ld Range Value ** spotsDtmP->numPts = %6ld",pnt1,spotsDtmP->numPoints ) ;
          if( pnt2 < 0 || pnt2 >= spotsDtmP->numPoints ) bcdtmWrite_message(0,0,0,"pnt2 = %6ld Range Value ** spotsDtmP->numPts = %6ld",pnt2,spotsDtmP->numPoints ) ;
         }  
/*
**     Check Memory
*/
       if( numIntPoints >= memIntPoints )
         {
          memIntPoints = memIntPoints + memIntPointsInc ;
          intPointsP = ( struct IntPoint * ) realloc( intPointsP , memIntPoints * sizeof(struct IntPoint)) ;
          if( intPointsP == NULL )
            {
             bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
             goto errexit ;
            } 
         } 
/*
**     Store Point Start Point
*/
       if( ds <= snapTolerance ) intFlag = 1 ; 
       else                      intFlag = 0 ;
       (intPointsP+numIntPoints)->intFlag   = intFlag ;
       (intPointsP+numIntPoints)->pointCode = 0  ;
       (intPointsP+numIntPoints)->len = 0.0 ;
       (intPointsP+numIntPoints)->x = xs ;
       (intPointsP+numIntPoints)->y = ys ;
       (intPointsP+numIntPoints)->z = pointAddrP(spotsDtmP,ps)->z ;
       ++numIntPoints ;
/*
**     Scan To End Of Line Segment
*/
       if( pnt1 <= pnt2 )
         {
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Scanning Forwards") ;
          for( p = pnt1 ; p <= pnt2  ; ++p )
            {
             if( dbg == 2 ) bcdtmWrite_message(0,0,0,"p = %6ld",p) ; 
             if( ( p != ps || ds > snapTolerance ) && ( p != pe || de > snapTolerance ) )
               {
                pntP = pointAddrP(spotsDtmP,p) ;
                if( pntP->y >= yMin && pntP->y <= yMax && pntP->x >= xMin && pntP->x <= xMax    )
                  {
                   if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Distance From Line = %10.4lf", bcdtmMath_distanceOfPointFromLine(&onLine,xs,ys,xe,ye,pntP->x,pntP->y,&xn,&yn)) ;
                   if( ( dm = bcdtmMath_distanceOfPointFromLine(&onLine,xs,ys,xe,ye,pntP->x,pntP->y,&xn,&yn)) <= snapTolerance )
                     {
                      if( onLine ) 
                        {
                         if( dbg == 2 ) bcdtmWrite_message(0,0,0,"onLine = %2ld ** p = %8ld %12.5lf %12.5lf %10.4lf ** dm = %10.4lf xn = %12.5lf yn = %12.5lf len = %10.4lf",onLine,p,pntP->x,pntP->y,pntP->z,dm,xn,yn,bcdtmMath_distance(xs,ys,xn,yn)) ;
/*
**                       Check Memory
*/
                         if( numIntPoints >= memIntPoints )
                           {
                            memIntPoints = memIntPoints + memIntPointsInc ;
                            intPointsP = ( struct IntPoint * ) realloc( intPointsP , memIntPoints * sizeof(struct IntPoint)) ;
                            if( intPointsP == NULL )
                              {
                               bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                               goto errexit ;
                              } 
                           } 
/*
**                       Store Point
*/
                         (intPointsP+numIntPoints)->intFlag   = 1 ;
                         (intPointsP+numIntPoints)->pointCode = 1 ;
                         (intPointsP+numIntPoints)->len = bcdtmMath_distance(xs,ys,xn,yn) ;
                         (intPointsP+numIntPoints)->x = xn ;
                         (intPointsP+numIntPoints)->y = yn ;
                         (intPointsP+numIntPoints)->z = pntP->z ;
                         ++numIntPoints ;
                        }
                     } 
                  }
               }
            }
         }
       else
         {
          if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Scanning Backwards") ;
          for( p = pnt1 ; p >= pnt2 ; --p )
            {
             if( ( p != ps || ds > snapTolerance ) && ( p != pe || de > snapTolerance ) )
               {
                pntP = pointAddrP(spotsDtmP,p) ;
                if( dbg == 2 ) bcdtmWrite_message(0,0,0,"Distance From Line = %10.4lf", bcdtmMath_distanceOfPointFromLine(&onLine,xs,ys,xe,ye,pntP->x,pntP->y,&xn,&yn)) ;
                if( pntP->y >= yMin && pntP->y <= yMax && pntP->x >= xMin && pntP->x <= xMax    )
                  {
                   if( ( dm = bcdtmMath_distanceOfPointFromLine(&onLine,xs,ys,xe,ye,pntP->x,pntP->y,&xn,&yn)) < snapTolerance )
                     {
                      if( onLine )
                        { 
                         if( dbg == 2 ) bcdtmWrite_message(0,0,0,"onLine = %2ld ** p = %8ld %12.5lf %12.5lf %10.4lf ** dm = %10.4lf xn = %12.5lf yn = %12.5lf len = %10.4lf",onLine,p,pntP->x,pntP->y,pntP->z,dm,xn,yn,bcdtmMath_distance(xs,ys,xn,yn)) ;
/*
**                       Check Memory
*/
                         if( numIntPoints >= memIntPoints )
                           {
                            memIntPoints = memIntPoints + memIntPointsInc ;
                            intPointsP = ( struct IntPoint * ) realloc( intPointsP , memIntPoints * sizeof(struct IntPoint)) ;
                            if( intPointsP == NULL )
                              {
                               bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                               goto errexit ;
                              } 
                           } 
/*
**                       Store Point
*/
                         (intPointsP+numIntPoints)->intFlag   = 1 ;
                         (intPointsP+numIntPoints)->pointCode = 1 ;
                         (intPointsP+numIntPoints)->len = bcdtmMath_distance(xs,ys,xn,yn) ;
                         (intPointsP+numIntPoints)->x = xn ;
                         (intPointsP+numIntPoints)->y = yn ;
                         (intPointsP+numIntPoints)->z = pointAddrP(spotsDtmP,p)->z ;
                         ++numIntPoints ;
                        }
                     }
                  }
               }
            }
         }
/*
**     Store Last Point
*/
       if( p3dP+1 == *pointsPP + *numPointsP - 1 )
         {
/*
**        Check Memory
*/
          if( numIntPoints >= memIntPoints )
            {
             memIntPoints = memIntPoints + memIntPointsInc ;
             intPointsP = ( struct IntPoint * ) realloc( intPointsP , memIntPoints * sizeof(struct IntPoint)) ;
             if( intPointsP == NULL )
               {
                bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
                goto errexit ;
               } 
            } 
/*
**        Store Point
*/
          if( de <= snapTolerance ) intFlag = 1 ;
          else                      intFlag = 0 ;
          (intPointsP+numIntPoints)->intFlag = intFlag ;
          (intPointsP+numIntPoints)->pointCode = 0 ;
          (intPointsP+numIntPoints)->len = bcdtmMath_distance(xs,ys,xe,ye) ;
          (intPointsP+numIntPoints)->x = xe ;
          (intPointsP+numIntPoints)->y = ye ;
          (intPointsP+numIntPoints)->z = pointAddrP(spotsDtmP,pe)->z ;
          ++numIntPoints ;
         }
/*
**     Sort Points On Distance From Start of Segment
*/
       qsortCPP(intPointsP,numIntPoints,sizeof(struct IntPoint),bcdtmInterpolate_intPointCompareFunction) ; 
/*
**     Copy Interpolation Points
*/
       memAllPoints = memAllPoints + numIntPoints ;
       if( allPointsP == NULL ) allPointsP = (struct IntPoint *) malloc( memAllPoints * sizeof(struct IntPoint)) ;
       else                     allPointsP = (struct IntPoint *) realloc( allPointsP, memAllPoints * sizeof(struct IntPoint)) ;
       if( allPointsP == NULL )
         {
          bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
          goto errexit ;
         } 
       for( intP = intPointsP ; intP < intPointsP + numIntPoints ; ++intP )
         {
          *(allPointsP+numAllPoints) = *intP ;
          ++numAllPoints ;
         } 
       numIntPoints = 0 ;
      }
   }
/*
** Write Out Interpolated Points
*/
 if( dbg == 1 )
   {
    bcdtmWrite_message(0,0,0,"Number Of Interpolated Points = %8ld",numAllPoints) ;
    for( intP = allPointsP ; intP < allPointsP + numAllPoints ; ++intP )
      {
       bcdtmWrite_message(0,0,0,"IntPoint[%8ld] intFlag = %2ld pointCode = %2ld len = %10.4lf ** %12.5lf %12.5lf %10.4lf",(long)(intP-allPointsP),intP->intFlag,intP->pointCode,intP->len,intP->x,intP->y,intP->z) ;
      } 
   }
/*
** Determine If Any Points Were Interpolated Onto The DTM Feature
*/
 pointsInterpolated = 0 ;
 for( intP = allPointsP ; intP < allPointsP + numAllPoints ; ++intP )
   {
    if( intP->intFlag ) ++pointsInterpolated ;
   } 
/*
** Only Interpolate If There Are more Than Two Points Interpolated
*/
  if( pointsInterpolated > 2 )
    { 
     *interpolationResultP = 1 ;
/*
**   Determine Start And Stop Of Interpolation
*/
     intLowP = allPointsP ;
     intHighP = allPointsP + numAllPoints - 1 ;
     while( intLowP  < allPointsP + numAllPoints && intLowP->intFlag == 0 ) ++intLowP ;
     while( intHighP > allPointsP && intHighP->intFlag == 0 ) --intHighP ;
     if( dbg == 1 )
       {
        bcdtmWrite_message(0,0,0,"IntLowPoint  [%8ld] intFlag = %2ld pointCode = %2ld len = %10.4lf ** %12.5lf %12.5lf %10.4lf",(long)(intLowP-allPointsP),intLowP->intFlag,intLowP->pointCode,intLowP->len,intLowP->x,intLowP->y,intLowP->z) ;
        bcdtmWrite_message(0,0,0,"IntHighPpoint[%8ld] intFlag = %2ld pointCode = %2ld len = %10.4lf ** %12.5lf %12.5lf %10.4lf",(long)(intHighP-allPointsP),intHighP->intFlag,intHighP->pointCode,intHighP->len,intHighP->x,intHighP->y,intHighP->z) ;
       } 
/*
**  Interpolate Between Interpolated points
*/
     for( intP = intLowP ; intP <= intHighP ; ++intP )
       {
        if( intP->intFlag == 0 )
          {
           int1P = intP - 1 ;
           int2P = intP ;
           while( int2P <= intHighP && int2P->intFlag == 0 ) ++int2P ;
/*
**         Calculate Length Of Segment Between int1P and int2P
*/
           segmentLength = 0.0 ;
           for( int3P = int1P + 1 ; int3P <= int2P ; ++int3P )
             {
              segmentLength = segmentLength + bcdtmMath_distance((int3P-1)->x,(int3P-1)->y,int3P->x,int3P->y) ;
             } 
           if( dbg == 1 ) bcdtmWrite_message(0,0,0,"Segment Length = %10.4lf",segmentLength) ;
/*
**         Interpolate Points
*/
           interpolateLength = 0.0 ;
           deltaZ = int2P->z - int1P->z ;  
           for( int3P = int1P + 1 ; int3P < int2P ; ++int3P )
            {
             interpolateLength = interpolateLength + bcdtmMath_distance((int3P-1)->x,(int3P-1)->y,int3P->x,int3P->y) ;
             int3P->z =  interpolateLength / segmentLength * deltaZ + int1P->z ;
             int3P->intFlag = 2 ;
            }
/*
**        Reset Iterator
*/
          intP = int2P ; 
         } 
      }    
/*
** Write Out Interpolated Points
*/
    if( dbg == 1 )
      {
       bcdtmWrite_message(0,0,0,"Number Of Interpolated Points = %8ld",numAllPoints) ;
       for( intP = allPointsP ; intP < allPointsP + numAllPoints ; ++intP )
         {
          bcdtmWrite_message(0,0,0,"IntPoint[%8ld] intFlag = %2ld pointCode = %2ld len = %10.4lf ** %12.5lf %12.5lf %10.4lf",(long)(intP-allPointsP),intP->intFlag,intP->pointCode,intP->len,intP->x,intP->y,intP->z) ;
         } 
      }
/*
**  Allocate memory For Interpolated Points
*/
    if( *pointsPP != NULL ) { free(*pointsPP) ; *pointsPP = NULL ; }
    *pointsPP = ( DPoint3d *) malloc(((long)(intHighP-intLowP)+1)*sizeof(DPoint3d)) ;
    if( *pointsPP == NULL )
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  Copy Interpolated Points
*/ 
    *numPointsP = 0 ;
    p3dP = *pointsPP ;
    for( intP = allPointsP ; intP < allPointsP + numAllPoints ; ++intP )
      {
       if( intP->intFlag )
         {
          p3dP->x = intP->x ;
          p3dP->y = intP->y ;
          p3dP->z = intP->z ;
          ++p3dP ;
          ++*numPointsP ;
         }
      }
/*
**  Write Out Interpolated Feature
*/
    if( dbg == 1 )
      {
       bcdtmWrite_message(0,0,0,"Number Of Interpolated Points = %8ld",*numPointsP) ;
       for( p3dP = *pointsPP ; p3dP < *pointsPP + *numPointsP ; ++p3dP )
         {
          bcdtmWrite_message(0,0,0,"Point[%4ld] ** %12.5lf %12.5lf %12.5lf",(long)(p3dP-*pointsPP),p3dP->x,p3dP->y,p3dP->z ) ;
         }
      }
   }
/*
** Clean Up
*/
 cleanup :
 if( intPointsP != NULL ) { free(intPointsP) ; intPointsP = NULL ; }
 if( allPointsP != NULL ) { free(allPointsP) ; allPointsP = NULL ; }
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Interpolate Linear Feature Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Interpolate Linear Feature Error") ;
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
BENTLEYDTM_Private int bcdtmInterpolate_findClosestPointDtmObject(BC_DTM_OBJ *dtmP,double x,double y,long *cPointP)
/*
** This routine finds the closest Dtm point to p(x,y)
*/
{
 long    dbg=DTM_TRACE_VALUE(0) ;
 long    cpnt,spnt,process ;
 double  dn = 0.0,dd ;
 DTM_TIN_POINT *cpntP ;
/*
** Write Entry Message
*/
 if( dbg ) 
   {
    bcdtmWrite_message(0,0,0,"Finding Closest Point") ;
    bcdtmWrite_message(0,0,0,"dtmP                  = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"x                     = %12.5lf",x) ; 
    bcdtmWrite_message(0,0,0,"y                     = %12.5lf",y) ; 
    bcdtmWrite_message(0,0,0,"dtmP->dtmState        = %8ld",dtmP->dtmState) ;
    bcdtmWrite_message(0,0,0,"dtmP->numPoints       = %8ld",dtmP->numPoints) ;
    bcdtmWrite_message(0,0,0,"dtmP->numSortedPoints = %8ld",dtmP->numSortedPoints) ;
   }
/*
** Initialise
*/
 *cPointP = dtmP->nullPnt ;
/*
**  Binary Scan x-Axis and find first x point value equal x or less than x
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Binary Scanning DTM Object") ;
 bcdtmFind_binaryScanDtmObject(dtmP,x,&cpnt) ;
 if( dbg ) bcdtmWrite_message(0,0,0,"cPnt = %8ld",cpnt) ;
 spnt = *cPointP = cpnt ;
 dn = bcdtmMath_distance(pointAddrP(dtmP,cpnt)->x,pointAddrP(dtmP,cpnt)->y,x,y) ;
 if( dn == 0 )  goto cleanup ;
/*
** Scan Back Until x - x point value  > dn
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Backwards") ;
 process = 1 ;
 for( cpnt = spnt - 1 ; cpnt >= 0 && process  ; --cpnt )
   {
    cpntP = pointAddrP(dtmP,cpnt) ;
    if( x - cpntP->x >= dn ) process = 0 ;
    else
      {
       if( fabs(y-cpntP->y ) < dn  )
         {
          dd = bcdtmMath_distance(cpntP->x,cpntP->y,x,y) ;
          if( dd < dn ) 
            {
             dn = dd ;
             *cPointP = cpnt ;
            }
         }
      }
   }
/*
**  Scan Forwards Until x point value - x > dn
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Forwards") ;
 if( dn > 0.0 ) process = 1 ;
 for( cpnt = spnt + 1 ; cpnt < dtmP->numSortedPoints && process ; ++cpnt )
   {
    cpntP = pointAddrP(dtmP,cpnt) ;
    if( cpntP->x - x >= dn ) process = 0 ;
    else 
      {
       if( fabs(y-cpntP->y) < dn )
         {
          dd = bcdtmMath_distance(cpntP->x,cpntP->y,x,y) ;
          if( dd < dn ) 
            { 
             dn = dd ;
             *cPointP = cpnt ;
            }
         }
      }
   }
/*
**  Scan Inserted 
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Scanning Inserted") ;
 if( dtmP->numSortedPoints != dtmP->numPoints && dn > 0.0 )
   {
    for( cpnt = dtmP->numSortedPoints ; cpnt < dtmP->numPoints ; ++cpnt )
      {
       cpntP = pointAddrP(dtmP,cpnt) ;
       if( fabs(x-cpntP->x) <= dn && fabs(y-cpntP->y) <= dn )
         {
          dd = bcdtmMath_distance(cpntP->x,cpntP->y,x,y) ;
          if( dd < dn ) { dn = dd ; *cPointP = cpnt ; }
         }
      }
   }
/*
** Job Completed
*/
 cleanup :
 if( dbg ) bcdtmWrite_message(0,0,0,"Finding Closest Point Completed ** cp = %8ld ** dn = %15.10lf",*cPointP,dn) ;
 if( dn == 0.0  ) return(1) ; 
 else             return(2) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private double bcdtmInterpolate_getDistanceFromPointDtmObject(BC_DTM_OBJ *dtmP,long point,double x,double y)
{
 double  dx,dy ;
 DTM_TIN_POINT *pointP ;
 pointP = pointAddrP(dtmP,point) ;
 dx = x - pointP->x ;
 dy = y - pointP->y ;
 return( sqrt(dx*dx+dy*dy)) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|   int bcdtmInterpolate_rotateAndTranslatePoint()                     |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmInterpolate_rotateAndTranslatePoint(double Px,double Py,double Tx,double Ty,double RotAngle, double *Nx, double *Ny)
/*
** This Function Rotates And Translates A Point
*/
{
 double nx,ny ;
/*
** Rotate Point
*/
 nx = Px * cos( RotAngle ) - Py * sin ( RotAngle ) ;
 ny = Px * sin( RotAngle ) + Py * cos ( RotAngle ) ;
/*
** Translate point
*/
 *Nx = nx + Tx ;
 *Ny = ny + Ty ;
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|   int bcdtmInterpolate_getPointOnArc()                               |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmInterpolate_getPointOnArc(double *Px,double *Py,double Paxis,double Saxis,double Angle)
/*
** This Function Calculates the Coordinates of a Point On an Arc
** The Centre of The Ellipse is at the origon and the rotation angle of
** the major axis is zero
*/
{
 double r,r1,r2,r1s,r2s,rs ;
/*
** Initialise Variables
*/
 r1 = Paxis / 2.0   ;
 r2 = Saxis / 2.0   ;
 r1s = r1 * r1 ;
 r2s = r2 * r2 ;
 rs  = r1s*r2s / ( r1s * sin(Angle) * sin(Angle) + r2s * cos(Angle) * cos(Angle) ) ;
 r   = sqrt(rs) ;
 *Px = r * cos(Angle) ;
 *Py = r * sin(Angle) ;
/*
**  Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmInterpolate_translateAndRotatePoint(double Px,double Py,double Tx,double Ty,double RotAngle, double *Nx, double *Ny)
/*
** This Function Translates and Rotates A Point
*/
{
 double nx,ny ;
/*
** Translate point
*/
 nx = Px + Tx ;
 ny = Py + Ty ;
/*
** Rotate Point
*/
 *Nx = nx * cos( RotAngle ) - ny * sin ( RotAngle ) ;
 *Ny = nx * sin( RotAngle ) + ny * cos ( RotAngle ) ;
/*
** Job Completed
*/
 return(0) ;
}
