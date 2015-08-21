/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmSort.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
#include "bcdtminlines.h" 
thread_local static unsigned long randomSeed ;
thread_local static unsigned long dtmRandomSeed=0 ;
thread_local static long  lastStartPoint = -1;
thread_local static long  numTiles = 0;
thread_local static long  tileStartPointArray[100000];
thread_local static long  numTilePoints[100000];
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public unsigned long randomNumber(unsigned long size)
/*
** This Function Generates A Random Number Between 0 and size
*/
{
 randomSeed = (randomSeed * 1366l + 150889l) % 714025l;
// return ( randomSeed / (714025l / size + 1)) ;
 return(size >> 1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private unsigned long dtmRandomNumber(unsigned long size)
/*
** This Function Generates A Random Number Between 0 and size
*/
{
// dtmRandomSeed = (dtmRandomSeed * 1366l + 150889l) % 714025l;
// return ( dtmRandomSeed / (714025l / size + 1)) ;
 return(size >> 1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public void bcdtmSort_taggedPointsIntoTilesForTriangulationDtmObject(BC_DTM_OBJ *dtmP,long *tagP,long startPoint,long numPoints) 
{
 long divider ;
/*
** Divide Number Of Points By 2
*/
 divider = numPoints >> 1 ;
/*
** Create Two Data Sets
*/
 if( numPoints - divider >= 2) 
   {
    if( divider >= 2) 
      {
       bcdtmSort_mergeSortTaggedPointsIntoTilesDtmObject(dtmP,tagP,startPoint,divider,DTM_Y_AXIS) ;
      }
    bcdtmSort_mergeSortTaggedPointsIntoTilesDtmObject(dtmP,tagP,startPoint+divider,numPoints-divider,DTM_Y_AXIS) ;
   }
/*
** Job Completed
*/
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private void bcdtmSort_mergeSortTaggedPointsIntoTilesDtmObject(BC_DTM_OBJ *dtmP,long *tagP,long startPoint,long numPoints,int axis)
/* 
**
** Sorts points into Clusters
** Sorts by x-coordinate if axis == 0  
** Sorts by y-coordinate if axis == 1    
** Tiles containing only two or three vertices must always be sorted by the x-coordinate
**                                           
*/
{
 long divider ;
/*
** Divide Number Of Points By 2
*/
 divider = numPoints >> 1 ;
/* 
** Subsets of two or three vertices must be sorted by x-coordinate. 
*/
 if( numPoints <= 3 ) axis = 0;
/*
** Cluster with a horizontal or vertical cut 
*/
 bcdtmSort_mergeSortTaggedPointsInTileDtmObject(dtmP,tagP,startPoint,numPoints,divider,axis) ;
/*
** Recursively Cluster The Points
*/
 if( numPoints-divider >= 2 ) 
   {
    if( divider >= 2) 
      {
       bcdtmSort_mergeSortTaggedPointsIntoTilesDtmObject(dtmP,tagP,startPoint,divider,1-axis);
      }
    bcdtmSort_mergeSortTaggedPointsIntoTilesDtmObject(dtmP,tagP,startPoint+divider,numPoints-divider,1-axis);
   }
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
#ifdef NEW
BENTLEYDTM_Private void bcdtmSort_mergeSortTaggedPointsInTileDtmObject(BC_DTM_OBJ *dtmP,long *tagP,long startPoint,long numPoints,long median,long axis)
/*
**  Sorts an array of points so that the first `median' points
**  occur lexicographically before the remaining points
**
**  Uses the x-coordinate as the primary key if axis == 0;
**  Uses the y-coordinate as the primary key if axis == 1
*/
{
 long    left,right,pivot;
 bool swap ;
 double  pivot1=0.0,pivot2=0.0,X,Y;
 DTM_TIN_POINT *p1P,*p2P,temp;
 long    tag ;
/*
** Two Points
*/
 if( numPoints == 2)
   {
    p1P = pointAddrP(dtmP,startPoint) ;
    p2P = pointAddrP(dtmP,startPoint+1) ;
    swap = false ;
    if( axis == DTM_X_AXIS && ( p1P->x > p2P->x || ( p1P->x == p2P->x && p1P->y > p2P->y ))) swap = true ;
    else if( axis == DTM_Y_AXIS && ( p1P->y > p2P->y || ( p1P->y == p2P->y && p1P->x > p2P->x ))) swap = true;
    if( swap == true )
      {
       temp = *p1P ;
       *p1P = *p2P ;
       *p2P = temp ;
       tag   = *(tagP+startPoint) ;
       *(tagP+startPoint) = *(tagP+startPoint+1) ;
       *(tagP+startPoint+1) = tag ;
      }
   }
 else
   {
/*
** Choose a random point to split the point array.
*/
    pivot  = (long) dtmRandomNumber(numPoints);
    p1P = pointAddrP(dtmP,startPoint+pivot) ;
    if( axis == DTM_X_AXIS ) { pivot1 = p1P->x ; pivot2 = p1P->y ; }
    else if( axis == DTM_Y_AXIS ) { pivot1 = p1P->y ; pivot2 = p1P->x ; }
/*
**  Split the point array.
*/
    left = -1;
    right = numPoints;
    while (left < right)
          {
/*
**     Search for a point whose coordinate is too large for the left.
*/
       do
         {
          ++left ;
           p1P = pointAddrP(dtmP,startPoint+left) ;
           X = p1P->x ;
           Y = p1P->y ;
           if( axis == DTM_X_AXIS)
               {
               if (!( X < pivot1 || ( X == pivot1 && Y < pivot2 ))) break;
               }
           else if (!( Y < pivot1 || ( Y == pivot1 && X < pivot2 ))) break;
         } while (left <= right) ;
/*
**     Search for a point whose coordinate is too small for the right.
*/
       do
        {
        --right ;
        p1P = pointAddrP(dtmP,startPoint+right) ;
        X = p1P->x ;
        Y = p1P->y ;
        if( axis == DTM_X_AXIS)
               {
               if (!(X > pivot1 || ( X == pivot1 && Y > pivot2 ))) break ;
               }
        else if (!( Y > pivot1 || ( Y == pivot1 && X > pivot2 ))) break ;
        } while( left <= right) ;
/*
**     Swap the left and right points
*/
       if( left < right )
             {
          p1P = pointAddrP(dtmP,startPoint+left) ;
          p2P = pointAddrP(dtmP,startPoint+right) ;
          temp = *p1P ;
          *p1P = *p2P ;
          *p2P = temp ;
          tag  = *(tagP+startPoint+left) ;
          *(tagP+startPoint+left)  = *(tagP+startPoint+right) ;
          *(tagP+startPoint+right) = tag ;
         }
      }
/*
** Recursively sort the left subset.
*/
    if( left > median)
      {
       bcdtmSort_mergeSortTaggedPointsInTileDtmObject(dtmP,tagP,startPoint,left,median,axis);
      }
/*
** Recursively sort the right subset.
*/
    if( right < median - 1 )
      {
       bcdtmSort_mergeSortTaggedPointsInTileDtmObject(dtmP,tagP,startPoint+right+1,numPoints-right-1,median-right-1,axis);
      }
   }
}
#else
BENTLEYDTM_Private void bcdtmSort_mergeSortTaggedPointsInTileDtmObject(BC_DTM_OBJ *dtmP,long *tagP,long startPoint,long numPoints,long median,long axis)
/*
**  Sorts an array of points so that the first `median' points
**  occur lexicographically before the remaining points  
**
**  Uses the x-coordinate as the primary key if axis == 0; 
**  Uses the y-coordinate as the primary key if axis == 1
*/
{
 long    left,right,pivot;
 bool swap,scan ;
 double  pivot1=0.0,pivot2=0.0,x,y;
 DTM_TIN_POINT *p1P,*p2P,temp;
 long    tag ; 
/*
** Two Points
*/
 if( numPoints == 2) 
   {
    p1P = pointAddrP(dtmP,startPoint) ;
    p2P = pointAddrP(dtmP,startPoint+1) ;
    swap = false ;
    if( axis == DTM_X_AXIS && ( p1P->x > p2P->x || ( p1P->x == p2P->x && p1P->y > p2P->y ))) swap = true ;
    if( axis == DTM_Y_AXIS && ( p1P->y > p2P->y || ( p1P->y == p2P->y && p1P->x > p2P->x ))) swap = true ;
    if( swap == true )
      {
       temp = *p1P ;
       *p1P = *p2P ;
       *p2P = temp ;
       tag   = *(tagP+startPoint) ;
       *(tagP+startPoint) = *(tagP+startPoint+1) ;
       *(tagP+startPoint+1) = tag ;
      }
   }
 else
   {
/* 
** Choose a random point to split the point array. 
*/
    pivot  = (long) dtmRandomNumber(numPoints);
    p1P = pointAddrP(dtmP,startPoint+pivot) ;
    if( axis == DTM_X_AXIS ) { pivot1 = p1P->x ; pivot2 = p1P->y ; }
    if( axis == DTM_Y_AXIS ) { pivot1 = p1P->y ; pivot2 = p1P->x ; }
/* 
**  Split the point array. 
*/
    left = -1;
    right = numPoints;
    while (left < right) 
      {
/* 
**     Search for a point whose coordinate is too large for the left. 
*/
       do 
         {
          ++left ;
           scan = false ;
           p1P = pointAddrP(dtmP,startPoint+left) ;
           x = p1P->x ; 
           y = p1P->y ; 
           if( axis == DTM_X_AXIS && ( x < pivot1 || ( x == pivot1 && y < pivot2 ))) scan = true ;
           if( axis == DTM_Y_AXIS && ( y < pivot1 || ( y == pivot1 && x < pivot2 ))) scan = true ;
         } while( left <= right && scan == true ) ;
/* 
**     Search for a point whose coordinate is too small for the right. 
*/
       do 
         {
          --right ;
           scan = false ;
           p1P = pointAddrP(dtmP,startPoint+right) ;
           x = p1P->x ; 
           y = p1P->y ; 
           if( axis == DTM_X_AXIS && ( x > pivot1 || ( x == pivot1 && y > pivot2 ))) scan = true ;
           if( axis == DTM_Y_AXIS && ( y > pivot1 || ( y == pivot1 && x > pivot2 ))) scan = true ;
         } while( left <= right &&  scan == true ) ;
/* 
**     Swap the left and right points 
*/
       if( left < right ) 
         {
          p1P = pointAddrP(dtmP,startPoint+left) ;
          p2P = pointAddrP(dtmP,startPoint+right) ;
          temp = *p1P ;
          *p1P = *p2P ;
          *p2P = temp ;
          tag  = *(tagP+startPoint+left) ;
          *(tagP+startPoint+left)  = *(tagP+startPoint+right) ;
          *(tagP+startPoint+right) = tag ;
         }
      }
/* 
** Recursively sort the left subset. 
*/
    if( left > median) 
      {
       bcdtmSort_mergeSortTaggedPointsInTileDtmObject(dtmP,tagP,startPoint,left,median,axis);
      }
/* 
** Recursively sort the right subset. 
*/
    if( right < median - 1 ) 
      {
       bcdtmSort_mergeSortTaggedPointsInTileDtmObject(dtmP,tagP,startPoint+right+1,numPoints-right-1,median-right-1,axis);
      }
   }
}
#endif
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmSort_pointsIntoTilesDtmObject
( 
 BC_DTM_OBJ *dtmP,              /* ==> Pointer To DTM Object      */ 
 long startPoint,               /* ==> Start Point In DTM Object  */
 long numPoints,                /* ==> Number Of Points To Tile   */ 
 long maxTilePts,               /* ==> Maximum Points Per Tile    */
 long **tileOffsetPP,           /* <== Tile offsets In DTM Object */
 long **numTilePtsPP,           /* <== Number Of Points In Tile   */
 long *numTilesP                /* <== Number Of Tiles            */
) 
{
 int  ret=DTM_SUCCESS ;
 long n,divider ;
/*
** Initialise
*/
 *numTilesP = 0 ;
 if( *tileOffsetPP != NULL ) { free(*tileOffsetPP) ; *tileOffsetPP = NULL ; }
 if( *numTilePtsPP != NULL ) { free(*numTilePtsPP) ; *numTilePtsPP = NULL ; }
/*
** Initialise Global Variable
*/
 numTiles = 0 ;
 lastStartPoint=-1 ;
/*
** Divide Number Of Points By 2
*/
 divider = numPoints >> 1 ;
/*
** Create Two Data Sets
*/
 if( numPoints - divider >= 2) 
   {
    if( divider >= 2) 
      {
       bcdtmSort_mergeSortPointsIntoTilesDtmObject(dtmP,maxTilePts,startPoint,divider,DTM_Y_AXIS) ;
      }
    bcdtmSort_mergeSortPointsIntoTilesDtmObject(dtmP,maxTilePts,startPoint+divider,numPoints-divider,DTM_Y_AXIS) ;
   }
/*
** Set Return Values
*/
 *numTilesP = numTiles ;
 *tileOffsetPP = ( long * ) malloc( *numTilesP * sizeof(long)) ;
 *numTilePtsPP = ( long * ) malloc( *numTilesP * sizeof(long)) ;
 if( *tileOffsetPP == NULL || *numTilePtsPP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
 for( n = 0 ; n < numTiles ; ++n )
   {
    *(*tileOffsetPP+n) = tileStartPointArray[n] ;
    *(*numTilePtsPP+n) = numTilePoints[n] ;
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
 if( *tileOffsetPP != NULL ) { free(*tileOffsetPP) ; *tileOffsetPP = NULL ; }
 if( *numTilePtsPP != NULL ) { free(*numTilePtsPP) ; *numTilePtsPP = NULL ; }
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private void bcdtmSort_mergeSortPointsIntoTilesDtmObject(BC_DTM_OBJ *dtmP,long maxTilePts,long startPoint,long numPoints,int axis)
/* 
**
** Sorts points into Clusters
** Sorts by x-coordinate if axis == 0  
** Sorts by y-coordinate if axis == 1    
** Tiles containing only two or three vertices must always be sorted by the x-coordinate
**                                           
*/
{
 long divider ;
// static lastStartPoint=-1 ;
/*
** Divide Number Of Points By 2
*/
 divider = numPoints >> 1 ;
// bcdtmWrite_message(0,0,0,"00 startPoint = %8ld numPoints = %8ld",startPoint,numPoints) ;
/* 
** Subsets of two or three vertices must be sorted by x-coordinate. 
*/
 if( numPoints <= 3 )  axis = 0;
/*
** Cluster with a horizontal or vertical cut 
*/
 bcdtmSort_tilePointsDtmObject(dtmP,startPoint,numPoints,divider,axis) ;
/*
** Recursively Cluster The Points
*/
 if( numPoints-divider > maxTilePts / 2 ) 
   {
    if( divider >= 2) 
      {
       bcdtmSort_mergeSortPointsIntoTilesDtmObject(dtmP,maxTilePts,startPoint,divider,1-axis);
      }
    bcdtmSort_mergeSortPointsIntoTilesDtmObject(dtmP,maxTilePts,startPoint+divider,numPoints-divider,1-axis);
   }
/*
** Set Tiles
*/
 if( startPoint > lastStartPoint )
   {
    tileStartPointArray[numTiles] = startPoint ;
    numTilePoints[numTiles]       = numPoints ;
    ++numTiles ;  
    lastStartPoint = startPoint ;
   }
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private void bcdtmSort_tilePointsDtmObject(BC_DTM_OBJ *dtmP,long startPoint,long numPoints,long median,long axis)
/*
**  Sorts an array of points so that the first `median' points
**  occur lexicographically before the remaining points  
**
**  Uses the x-coordinate as the primary key if axis == 0; 
**  Uses the y-coordinate as the primary key if axis == 1
*/
{
 long    left,right,pivot,swap,scan ;
 double  pivot1=0.0,pivot2=0.0,x,y;
 DTM_TIN_POINT *p1P,*p2P,temp;
/*
** Two Points
*/
 if( numPoints == 2 ) 
   {
    p1P = pointAddrP(dtmP,startPoint) ;
    p2P = pointAddrP(dtmP,startPoint+1) ;
    swap = FALSE ;
    if( axis == DTM_X_AXIS && ( p1P->x > p2P->x || ( p1P->x == p2P->x && p1P->y > p2P->y ))) swap = TRUE ;
    if( axis == DTM_Y_AXIS && ( p1P->y > p2P->y || ( p1P->y == p2P->y && p1P->x > p2P->x ))) swap = TRUE ;
    if( swap == TRUE )
      {
       temp = *p1P ;
       *p1P = *p2P ;
       *p2P = temp ;
      }
   }
 else
   {
/* 
** Choose a random point to split the point array. 
*/
    pivot  = (long) dtmRandomNumber(numPoints);
    p1P = pointAddrP(dtmP,startPoint+pivot) ;
    if( axis == DTM_X_AXIS ) { pivot1 = p1P->x ; pivot2 = p1P->y ; }
    if( axis == DTM_Y_AXIS ) { pivot1 = p1P->y ; pivot2 = p1P->x ; }
/* 
**  Split the point array. 
*/
    left = -1;
    right = numPoints;
    while (left < right) 
      {
/* 
**     Search for a point whose coordinate is too large for the left. 
*/
       do 
         {
          ++left ;
           scan = FALSE ;
           p1P = pointAddrP(dtmP,startPoint+left) ;
           x = p1P->x ; 
           y = p1P->y ; 
           if( axis == DTM_X_AXIS && ( x < pivot1 || ( x == pivot1 && y < pivot2 ))) scan = TRUE ;
           if( axis == DTM_Y_AXIS && ( y < pivot1 || ( y == pivot1 && x < pivot2 ))) scan = TRUE ;
         } while( left <= right && scan == TRUE ) ;
/* 
**     Search for a point whose coordinate is too small for the right. 
*/
       do 
         {
          --right ;
           scan = FALSE ;
           p1P = pointAddrP(dtmP,startPoint+right) ;
           x = p1P->x ; 
           y = p1P->y ; 
           if( axis == DTM_X_AXIS && ( x > pivot1 || ( x == pivot1 && y > pivot2 ))) scan = TRUE ;
           if( axis == DTM_Y_AXIS && ( y > pivot1 || ( y == pivot1 && x > pivot2 ))) scan = TRUE ;
         } while( left <= right &&  scan == TRUE ) ;
/* 
**     Swap the left and right points 
*/
       if( left < right ) 
         {
          p1P = pointAddrP(dtmP,startPoint+left) ;
          p2P = pointAddrP(dtmP,startPoint+right) ;
          temp = *p1P ;
          *p1P = *p2P ;
          *p2P = temp ;
         }
      }
/* 
** Recursively sort the left subset. 
*/
    if( left > median) 
      {
       bcdtmSort_tilePointsDtmObject(dtmP,startPoint,left,median,axis);
      }
/* 
** Recursively sort the right subset. 
*/
    if( right < median - 1 ) 
      {
       bcdtmSort_tilePointsDtmObject(dtmP,startPoint+right+1,numPoints-right-1,median-right-1,axis);
      }
   }
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmSort_taggedPointsIntoTilesDtmObject
( 
 BC_DTM_OBJ *dtmP,              /* ==> Pointer To DTM Object      */ 
 long *tagP,                    /* ==> Pointer To User Tags       */ 
 long startPoint,               /* ==> Start Point In DTM Object  */
 long numPoints,                /* ==> Number Of Points To Tile   */ 
 long maxTilePts,               /* ==> Maximum Points Per Tile    */
 long **tileOffsetPP,           /* <== Tile offsets In DTM Object */
 long **numTilePtsPP,           /* <== Number Of Points In Tile   */
 long *numTilesP                /* <== Number Of Tiles            */
) 
{
 int  ret=DTM_SUCCESS ;
 long n,divider ;
/*
** Initialise
*/
 *numTilesP = 0 ;
 if( *tileOffsetPP != NULL ) { free(*tileOffsetPP) ; *tileOffsetPP = NULL ; }
 if( *numTilePtsPP != NULL ) { free(*numTilePtsPP) ; *numTilePtsPP = NULL ; }
/*
** Initialise Global Variable
*/
 numTiles = 0 ;
 lastStartPoint=-1 ;
/*
** Divide Number Of Points By 2
*/
 divider = numPoints >> 1 ;
/*
** Create Two Data Sets
*/
 if( numPoints - divider > maxTilePts / 2) 
   {
    if( divider >= 2) 
      {
       bcdtmSort_mergeSortTileTaggedPointsDtmObject(dtmP,tagP,maxTilePts,startPoint,divider,DTM_Y_AXIS) ;
      }
    bcdtmSort_mergeSortTileTaggedPointsDtmObject(dtmP,tagP,maxTilePts,startPoint+divider,numPoints-divider,DTM_Y_AXIS) ;
   }
/*
** Set Return Values
*/
 *numTilesP = numTiles ;
 *tileOffsetPP = ( long * ) malloc( *numTilesP * sizeof(long)) ;
 *numTilePtsPP = ( long * ) malloc( *numTilesP * sizeof(long)) ;
 if( *tileOffsetPP == NULL || *numTilePtsPP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
 for( n = 0 ; n < numTiles ; ++n )
   {
    *(*tileOffsetPP+n) = tileStartPointArray[n] ;
    *(*numTilePtsPP+n) = numTilePoints[n] ;
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
 if( *tileOffsetPP != NULL ) { free(*tileOffsetPP) ; *tileOffsetPP = NULL ; }
 if( *numTilePtsPP != NULL ) { free(*numTilePtsPP) ; *numTilePtsPP = NULL ; }
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private void bcdtmSort_mergeSortTileTaggedPointsDtmObject(BC_DTM_OBJ *dtmP,long *tagP,long maxTilePts,long startPoint,long numPoints,int axis)
/* 
**
** Sorts points into Clusters
** Sorts by x-coordinate if axis == 0  
** Sorts by y-coordinate if axis == 1    
** Tiles containing only two or three vertices must always be sorted by the x-coordinate
**                                           
*/
{
 long divider ;
/*
** Divide Number Of Points By 2
*/
 divider = numPoints >> 1 ;
// bcdtmWrite_message(0,0,0,"00 startPoint = %8ld numPoints = %8ld",startPoint,numPoints) ;
/* 
** Subsets of two or three vertices must be sorted by x-coordinate. 
*/
 if( numPoints <= 3 )  axis = 0;
/*
** Cluster with a horizontal or vertical cut 
*/
 bcdtmSort_tileTaggedPointsDtmObject(dtmP,tagP,startPoint,numPoints,divider,axis) ;
/*
** Recursively Cluster The Points
*/
 if( numPoints-divider > maxTilePts / 2 ) 
   {
    if( divider >= 2) 
      {
       bcdtmSort_mergeSortTileTaggedPointsDtmObject(dtmP,tagP,maxTilePts,startPoint,divider,1-axis);
      }
    bcdtmSort_mergeSortTileTaggedPointsDtmObject(dtmP,tagP,maxTilePts,startPoint+divider,numPoints-divider,1-axis);
   }
/*
** Set Tiles
*/
 if( startPoint > lastStartPoint )
   {
    tileStartPointArray[numTiles] = startPoint ;
    numTilePoints[numTiles]       = numPoints ;
    ++numTiles ;  
    lastStartPoint = startPoint ;
   }
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private void bcdtmSort_tileTaggedPointsDtmObject(BC_DTM_OBJ *dtmP,long *tagP,long startPoint,long numPoints,long median,long axis)
/*
**  Sorts an array of points so that the first `median' points
**  occur lexicographically before the remaining points  
**
**  Uses the x-coordinate as the primary key if axis == 0; 
**  Uses the y-coordinate as the primary key if axis == 1
*/
{
 long    left,right,pivot,swap,scan ;
 double  pivot1=0.0,pivot2=0.0,x,y;
 DTM_TIN_POINT *p1P,*p2P,temp;
/*
** Two Points
*/
 if( numPoints == 2 ) 
   {
    p1P = pointAddrP(dtmP,startPoint) ;
    p2P = pointAddrP(dtmP,startPoint+1) ;
    swap = FALSE ;
    if( axis == DTM_X_AXIS && ( p1P->x > p2P->x || ( p1P->x == p2P->x && p1P->y > p2P->y ))) swap = TRUE ;
    if( axis == DTM_Y_AXIS && ( p1P->y > p2P->y || ( p1P->y == p2P->y && p1P->x > p2P->x ))) swap = TRUE ;
    if( swap == TRUE )
      {
       temp = *p1P ;
       *p1P = *p2P ;
       *p2P = temp ;
       swap = *(tagP+startPoint) ;
       *(tagP+startPoint) = *(tagP+startPoint+1) ;
       *(tagP+startPoint+1) = swap ;
      }
   }
 else
   {
/* 
** Choose a random point to split the point array. 
*/
    pivot  = (long) dtmRandomNumber(numPoints);
    p1P = pointAddrP(dtmP,startPoint+pivot) ;
    if( axis == DTM_X_AXIS ) { pivot1 = p1P->x ; pivot2 = p1P->y ; }
    if( axis == DTM_Y_AXIS ) { pivot1 = p1P->y ; pivot2 = p1P->x ; }
/* 
**  Split the point array. 
*/
    left = -1;
    right = numPoints;
    while (left < right) 
      {
/* 
**     Search for a point whose coordinate is too large for the left. 
*/
       do 
         {
          ++left ;
           scan = FALSE ;
           p1P = pointAddrP(dtmP,startPoint+left) ;
           x = p1P->x ; 
           y = p1P->y ; 
           if( axis == DTM_X_AXIS && ( x < pivot1 || ( x == pivot1 && y < pivot2 ))) scan = TRUE ;
           if( axis == DTM_Y_AXIS && ( y < pivot1 || ( y == pivot1 && x < pivot2 ))) scan = TRUE ;
         } while( left <= right && scan == TRUE ) ;
/* 
**     Search for a point whose coordinate is too small for the right. 
*/
       do 
         {
          --right ;
           scan = FALSE ;
           p1P = pointAddrP(dtmP,startPoint+right) ;
           x = p1P->x ; 
           y = p1P->y ; 
           if( axis == DTM_X_AXIS && ( x > pivot1 || ( x == pivot1 && y > pivot2 ))) scan = TRUE ;
           if( axis == DTM_Y_AXIS && ( y > pivot1 || ( y == pivot1 && x > pivot2 ))) scan = TRUE ;
         } while( left <= right &&  scan == TRUE ) ;
/* 
**     Swap the left and right points 
*/
       if( left < right ) 
         {
          p1P = pointAddrP(dtmP,startPoint+left) ;
          p2P = pointAddrP(dtmP,startPoint+right) ;
          temp = *p1P ;
          *p1P = *p2P ;
          *p2P = temp ;
          swap = *(tagP+startPoint) ;
          *(tagP+startPoint) = *(tagP+startPoint+1) ;
          *(tagP+startPoint+1) = swap ;
         }
      }
/* 
** Recursively sort the left subset. 
*/
    if( left > median) 
      {
       bcdtmSort_tileTaggedPointsDtmObject(dtmP,tagP,startPoint,left,median,axis);
      }
/* 
** Recursively sort the right subset. 
*/
    if( right < median - 1 ) 
      {
       bcdtmSort_tileTaggedPointsDtmObject(dtmP,tagP,startPoint+right+1,numPoints-right-1,median-right-1,axis);
      }
   }
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmSort_qsortPointArrayXaxisCompareFunction(const DPoint3d *p3d1P,const DPoint3d *p3d2P)
/*
** Compare Function For Qsort Of Point Array
*/
{
 if     ( p3d1P->x < p3d2P->x ) return(-1) ;
 else if( p3d1P->x > p3d2P->x ) return( 1) ;
 else if( p3d1P->y < p3d2P->y ) return(-1) ;
 else if( p3d1P->y > p3d2P->y ) return( 1) ;
 else if( p3d1P->z < p3d2P->z ) return(-1) ;
 else if( p3d1P->z > p3d2P->z ) return( 1) ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmSort_qsortPointArrayYaxisCompareFunction(const DPoint3d *p3d1P,const DPoint3d *p3d2P)
/*
** Compare Function For Qsort Of Point Array
*/
{
 if     ( p3d1P->y < p3d2P->y ) return(-1) ;
 else if( p3d1P->y > p3d2P->y ) return( 1) ;
 else if( p3d1P->y < p3d2P->y ) return(-1) ;
 else if( p3d1P->y > p3d2P->y ) return( 1) ;
 else if( p3d1P->z < p3d2P->z ) return(-1) ;
 else if( p3d1P->z > p3d2P->z ) return( 1) ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public void bcdtmSort_qsortPointArray(DPoint3d *ptsP,long numPts,long axis) 
{
 if( axis == DTM_X_AXIS ) qsortCPP(ptsP,numPts,sizeof(DPoint3d),bcdtmSort_qsortPointArrayXaxisCompareFunction) ;
 if( axis == DTM_Y_AXIS ) qsortCPP(ptsP,numPts,sizeof(DPoint3d),bcdtmSort_qsortPointArrayYaxisCompareFunction) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmSort_removeDuplicatesPointArray(DPoint3d **ptsPP,long *numPtsP,long *numDupPtsP) 
{
 DPoint3d *pts1P,*pts2P ;
/*
** Initialise
*/
 *numDupPtsP = 0 ;
/*
** Scan Point Array And Copy Over Duplicates
*/
 pts1P = *ptsPP ;
 for( pts2P = *ptsPP + 1 ; pts2P < *ptsPP + *numPtsP ; ++pts2P )
   {
    if( pts2P->x != pts1P->x || pts2P->y != pts1P->y )
      {
       ++pts1P ;
       if( pts1P != pts2P ) *pts1P = *pts2P ;
      } 
    else ++*numDupPtsP ;
   }
/*
** Resize Points Array
*/
 if( (long)(pts1P-*ptsPP) + 1 < *numPtsP )
   {
    *numPtsP = (long)(pts1P-*ptsPP) + 1 ;
    *ptsPP = ( DPoint3d * ) realloc( *ptsPP,*numPtsP*sizeof(DPoint3d)) ;
   }
/*
** Return
*/
 return(DTM_SUCCESS) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmSort_removeDuplicatesWithinTolerancePointArray(DPoint3d **ptsPP,long *numPtsP,double ppTol,long *numDupPtsP) 
{
 int    ret=DTM_SUCCESS ;
 long   ofs,numDupFlag  ;
 double dx,dy,ppTolSq   ;
 unsigned char   *dupFlagP=NULL ;
 DPoint3d    *pts1P,*pts2P ;
/*
** Initialise
*/
 *numDupPtsP = 0 ;
 if( ppTol < 0.0 ) ppTol = 0.0 ;
 ppTolSq = ppTol * ppTol ;
/*
** Allocate Memory For Marking Duplicate Points 
*/
 numDupFlag = *numPtsP / 8 + 1 ;
 dupFlagP = ( unsigned char * ) malloc ( numDupFlag * sizeof(char)) ;
 if( dupFlagP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Set Memory To Zero
*/
 memset(dupFlagP,0,numDupFlag) ;
/*
** Scan Point Array And Mark Duplicates Within Tolerance
*/
 pts1P = *ptsPP ;
 for( ofs = 0 , pts1P = *ptsPP ; pts1P < *ptsPP + *numPtsP ; ++ofs , ++pts1P )
   {
    if( ! bcdtmFlag_testFlag(dupFlagP,ofs) )
      {
       pts2P = pts1P + 1 ;
       while( pts2P < *ptsPP + *numPtsP && pts2P->x - pts1P->x < ppTol )
         {
          if( fabs(pts2P->y - pts1P->y) < ppTol )
            {
             dx = pts2P->x - pts1P->x ;
             dy = pts2P->y - pts1P->y ;
             if( ( dx*dx + dy*dy ) < ppTolSq )
               {
                bcdtmFlag_setFlag(dupFlagP,(long)(pts2P-*ptsPP)) ;
                ++*numDupPtsP ;
               }
            } 
          ++pts2P ; 
         }
      }
   }
/*
**  Remove Duplicate Points If Detected
*/
 if( *numDupPtsP > 0 )
   {
/*
**  Copy Over Duplicates
*/
    for( ofs = 0 , pts1P = pts2P = *ptsPP ; pts2P < *ptsPP + *numPtsP ; ++ofs , ++pts2P )
      {
       if( ! bcdtmFlag_testFlag(dupFlagP,ofs) )
         {
          if( pts1P != pts2P ) *pts1P = *pts2P ;
          ++pts1P ;  
         } 
      } 
/*
**  Resize Points Array
*/
    *numPtsP = *numPtsP - *numDupPtsP ;
    *ptsPP = ( DPoint3d * ) realloc( *ptsPP,*numPtsP*sizeof(DPoint3d)) ;
   }
/*
** Clean Up
*/
 cleanup :
 if( dupFlagP != NULL ) free(dupFlagP) ;
/*
** Return
*/
 return(DTM_SUCCESS) ;
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
BENTLEYDTM_Public void bcdtmSort_clustersPointArray(DPoint3d *ptsP,long numPts) 
{
 long divider ;
/*
** Divide Number Of Points By 2
*/
 divider = numPts >> 1 ;
/*
** Create Two Data Sets
*/
 if( numPts - divider >= 2) 
   {
    if( divider >= 2) 
      {
       bcdtmSort_mergeSortClustersPointArray(ptsP,divider,DTM_Y_AXIS) ;
      }
    bcdtmSort_mergeSortClustersPointArray(ptsP+divider,numPts-divider,DTM_Y_AXIS) ;
   }
/*
** Job Completed
*/
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private void bcdtmSort_mergeSortClustersPointArray(DPoint3d *ptsP,long numPts,int axis)
/* 
**  Sorts points into Clusters
**  Sorts by x-coordinate if axis == 0; 
**  Sorts by y-coordinate if axis == 1.   
**  Tiles containing only two or three vertices must always be sorted by the x-coordinate.                                           
*/
{
 long divider ;
/*
** Divide Number Of Points By 2
*/
  divider = numPts >> 1 ;
/* 
** Subsets of two or three vertices must be sorted by x-coordinate. 
*/
  if( numPts <= 3 )  axis = 0;
/*
** Cluster with a horizontal or vertical cut 
*/
  bcdtmSort_pointArrayCluster(ptsP,numPts,divider,axis) ;
/*
** Recursively Cluster The Points
*/
  if( numPts-divider >= 2 ) 
    {
     if( divider >= 2) 
       {
        bcdtmSort_mergeSortClustersPointArray(ptsP,divider,1-axis);
       }
     bcdtmSort_mergeSortClustersPointArray(ptsP+divider,numPts-divider,1-axis);
   }
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private void bcdtmSort_pointArrayCluster(DPoint3d *ptsP,long numPts,long median,long axis)
/*
**  Sorts an array of points so that the first `median' points
**  occur lexicographically before the remaining points  
**
**  Uses the x-coordinate as the primary key if axis == 0; 
**  Uses the y-coordinate as the primary key if axis == 1
*/
{
 long    left,right,pivot,swap,scan ;
 double  pivot1=0.0,pivot2=0.0,x,y;
 DPoint3d     *p1P,*p2P,temp;
/*
** Two Points
*/
 if (numPts == 2) 
  {
   p1P = ptsP ;
   p2P = ptsP + 1 ;
   swap = FALSE ;
   if( axis == DTM_X_AXIS && ( p1P->x > p2P->x || ( p1P->x == p2P->x && p1P->y > p2P->y ))) swap = TRUE ;
   if( axis == DTM_Y_AXIS && ( p1P->y > p2P->y || ( p1P->y == p2P->y && p1P->x > p2P->x ))) swap = TRUE ;
   if( swap == TRUE )
     {
      temp = *p1P ;
      *p1P = *p2P ;
      *p2P = temp ;
     }
   }
 else
   {
/* 
** Choose a random point to split the point array. 
*/
    pivot  = (long) randomNumber(numPts);
    if( axis == DTM_X_AXIS ) { pivot1 = (ptsP+pivot)->x ; pivot2 = (ptsP+pivot)->y ; }
    if( axis == DTM_Y_AXIS ) { pivot1 = (ptsP+pivot)->y ; pivot2 = (ptsP+pivot)->x ; }
/* 
**  Split the point array. 
*/
    left = -1;
    right = numPts;
    while (left < right) 
      {
/* 
**  Search for a point whose coordinate is too large for the left. 
*/
       do 
         {
          ++left ;
          scan = FALSE ;
          x = (ptsP+left)->x ; 
          y = (ptsP+left)->y ; 
          if( axis == DTM_X_AXIS && ( x < pivot1 || ( x == pivot1 && y < pivot2 ))) scan = TRUE ;
          if( axis == DTM_Y_AXIS && ( y < pivot1 || ( y == pivot1 && x < pivot2 ))) scan = TRUE ;
         } while( left <= right && scan == TRUE ) ;
/* 
**  Search for a point whose coordinate is too small for the right. 
*/
       do 
         {
          --right ;
          scan = FALSE ;
          x = (ptsP+right)->x ; 
          y = (ptsP+right)->y ; 
          if( axis == DTM_X_AXIS && ( x > pivot1 || ( x == pivot1 && y > pivot2 ))) scan = TRUE ;
          if( axis == DTM_Y_AXIS && ( y > pivot1 || ( y == pivot1 && x > pivot2 ))) scan = TRUE ;
         } while( left <= right &&  scan == TRUE ) ;
/* 
** Swap the left and right points 
*/
       if( left < right ) 
         {
          p1P = ptsP + left ;
          p2P = ptsP + right ;
          temp = *p1P ;
          *p1P = *p2P ;
          *p2P = temp ;
         }
      }
/* 
** Recursively sort the left subset. 
*/
    if( left > median) 
      {
       bcdtmSort_pointArrayCluster(ptsP,left,median,axis);
      }
/* 
** Recursively sort the right subset. 
*/
    if( right < median - 1 ) 
      {
       bcdtmSort_pointArrayCluster(ptsP+right+1,numPts-right-1,median-right-1,axis);
      }
   }
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmSort_tilePointArray(DPoint3d *ptsP,long numPts,double tileLength,DTM_POINT_ARRAY_TILE **tilesPP,long *numTilesP) 
{
 int    ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0),cdbg=DTM_CHECK_VALUE(0) ;
 long   memTile,memTileIni,memTileInc,numTilePts,saveNumPts ;
 DPoint3d    *p3dP,*p3dLowP,*p3dHighP ;
 DPoint3d    *tileP,*tileLowP,*tileHighP ;
 double xLow,yLow ;
 DTM_POINT_ARRAY_TILE *patP ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Tiling Points Array") ;
/*
** Initialise
*/
 saveNumPts = numPts ;
 *numTilesP = 0 ;
 if( *tilesPP != NULL )
   {
    free(*tilesPP) ;
    *tilesPP = NULL ; 
   }
/*
**  Sort Point Array On x Axis
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Sorting x Axis") ;
 bcdtmSort_qsortPointArray(ptsP,numPts,DTM_X_AXIS) ;
/*
** Initialise Tile Points Memory Variables
*/
 memTile = 0 ;
 memTileIni = (long)( ((ptsP+numPts-1)->x - ptsP->x ) / tileLength + 2 ) ;
 memTileIni = memTileIni * memTileIni ;
 memTileInc = memTileIni / 2 ;
 if( dbg ) bcdtmWrite_message(0,0,0,"memTileIni = %8ld memTileInc = %8ld",memTileIni,memTileInc) ;
/*
** Set Limits
*/
 p3dLowP = ptsP ;
 p3dHighP = ptsP + numPts - 1 ;
/*
**  Scan x Axis And Sort y Axis Into Tiles
*/
 while ( p3dLowP <= p3dHighP )
   {
    p3dP = p3dLowP ;
    xLow = p3dP->x ;
    while ( p3dP <= p3dHighP && (p3dP->x - xLow) <= tileLength ) ++p3dP ;
    --p3dP ;
    numPts = (long) (p3dP-p3dLowP) + 1 ;
    if( dbg ) bcdtmWrite_message(0,0,0,"offset = %8ld numPts = %8ld ** Sorting y Axis",(long)(p3dLowP-ptsP),numPts) ; 
    if( numPts > 0 ) 
      {
/*
**     Sort y Axis
*/
       bcdtmSort_qsortPointArray(p3dLowP,numPts,DTM_Y_AXIS) ;
/*
**     Tile y Axis
*/
       tileLowP  = p3dLowP ; 
       tileHighP = p3dP    ;
       while ( tileLowP <= tileHighP )
         {
/*
**        Scan From Tile Start To Tile End
*/
          yLow = tileLowP->y ; 
          tileP = tileLowP ;
          while ( tileP <= tileHighP && (tileP->y - yLow) <= tileLength ) ++tileP ; 
          --tileP ;
/*
**        Check Tile Pointer Memory
*/
          if( *numTilesP == memTile )
            {
             if( memTile == 0 ) memTile = memTileIni ;
             else               memTile = memTile + memTileInc ; 
             if( *tilesPP == NULL ) *tilesPP = ( DTM_POINT_ARRAY_TILE * ) malloc(memTile * sizeof(DTM_POINT_ARRAY_TILE)) ;
             else                   *tilesPP = ( DTM_POINT_ARRAY_TILE * ) realloc(*tilesPP,memTile * sizeof(DTM_POINT_ARRAY_TILE)) ;
             if( *tilesPP == NULL )
               {
                bcdtmWrite_message(0,0,0,"Memory Allocation Failure") ;
                goto errexit ;
               }
            }
/*
**        Store Tile Points
*/
          (*tilesPP+*numTilesP)->firstPntP = tileLowP ;
          (*tilesPP+*numTilesP)->lastPntP  = tileP  ;
          ++*numTilesP ; 
/*
**        Set Tile Low Pointer
*/
          tileLowP = tileP + 1 ;
         }
      }
    p3dLowP = p3dP + 1 ;
   }
/*
** Resize Tile Points Array
*/
 if( *tilesPP != NULL && *numTilesP < memTile )
   {
    *tilesPP = ( DTM_POINT_ARRAY_TILE * ) realloc(*tilesPP,*numTilesP*sizeof(DTM_POINT_ARRAY_TILE)) ;
   }
/*
**  Check Total Number Of Points In Tiles
*/
 if( cdbg )
   {
    numTilePts = 0 ;
    for( patP = *tilesPP ; patP < *tilesPP + *numTilesP ; ++patP )
      {
       numTilePts = numTilePts +(long)(patP->lastPntP-patP->firstPntP) + 1 ;
      }
    if( numTilePts != saveNumPts )
      {
       bcdtmWrite_message(0,0,0,"numTilePts = %8ld numPts = %8ld",numTilePts,saveNumPts) ; 
       bcdtmWrite_message(2,0,0,"Total Number Of Tile Points Is Not Equal To The Number Of Points") ;
      }
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
BENTLEYDTM_Public void bcdtmSort_clustersPointArrayWithTag(DPoint3d *ptsP,long *tagP,long numPts) 
{
 long divider ;
/*
** Divide Number Of Points By 2
*/
 divider = numPts >> 1 ;
/*
** Create Two Data Sets
*/
 if( numPts - divider >= 2) 
   {
    if( divider >= 2) 
      {
       bcdtmSort_mergeSortClustersPointArrayWithTag(ptsP,tagP,divider,DTM_Y_AXIS) ;
      }
    bcdtmSort_mergeSortClustersPointArrayWithTag(ptsP+divider,tagP+divider,numPts-divider,DTM_Y_AXIS) ;
   }
/*
** Job Completed
*/
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private void bcdtmSort_mergeSortClustersPointArrayWithTag(DPoint3d *ptsP,long *tagP,long numPts,int axis)
/* 
**
** Sorts points into Clusters
** Sorts by x-coordinate if axis == 0  
** Sorts by y-coordinate if axis == 1    
** Tiles containing only two or three vertices must always be sorted by the x-coordinate
**                                           
*/
{
 long divider ;
/*
** Divide Number Of Points By 2
*/
 divider = numPts >> 1 ;
/* 
** Subsets of two or three vertices must be sorted by x-coordinate. 
*/
 if( numPts <= 3 )  axis = 0;
/*
** Cluster with a horizontal or vertical cut 
*/
 bcdtmSort_pointArrayClusterWithTag(ptsP,tagP,numPts,divider,axis) ;
/*
** Recursively Cluster The Points
*/
 if( numPts-divider >= 2 ) 
   {
    if( divider >= 2) 
      {
       bcdtmSort_mergeSortClustersPointArrayWithTag(ptsP,tagP,divider,1-axis);
      }
    bcdtmSort_mergeSortClustersPointArrayWithTag(ptsP+divider,tagP+divider,numPts-divider,1-axis);
   }
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private void bcdtmSort_pointArrayClusterWithTag(DPoint3d *ptsP,long *tagP,long numPts,long median,long axis)
/*
**  Sorts an array of points so that the first `median' points
**  occur lexicographically before the remaining points  
**
**  Uses the x-coordinate as the primary key if axis == 0; 
**  Uses the y-coordinate as the primary key if axis == 1
*/
{
 long    left,right,pivot,swap,scan ;
 double  pivot1=0.0,pivot2=0.0,x,y;
 DPoint3d     *p1P,*p2P,temp;
 long    tag ; 
/*
** Two Points
*/
 if( numPts == 2) 
   {
    p1P = ptsP ;
    p2P = ptsP + 1 ;
    swap = FALSE ;
    if( axis == DTM_X_AXIS && ( p1P->x > p2P->x || ( p1P->x == p2P->x && p1P->y > p2P->y ))) swap = TRUE ;
    if( axis == DTM_Y_AXIS && ( p1P->y > p2P->y || ( p1P->y == p2P->y && p1P->x > p2P->x ))) swap = TRUE ;
    if( swap == TRUE )
      {
       temp = *p1P ;
       *p1P = *p2P ;
       *p2P = temp ;
       tag   = *tagP ;
       *tagP = *(tagP+1) ;
       *(tagP+1) = tag ;
      }
   }
 else
   {
/* 
** Choose a random point to split the point array. 
*/
    pivot  = (long) randomNumber(numPts);
    if( axis == DTM_X_AXIS ) { pivot1 = (ptsP+pivot)->x ; pivot2 = (ptsP+pivot)->y ; }
    if( axis == DTM_Y_AXIS ) { pivot1 = (ptsP+pivot)->y ; pivot2 = (ptsP+pivot)->x ; }
/* 
**  Split the point array. 
*/
    left = -1;
    right = numPts;
    while (left < right) 
       {
/* 
**  Search for a point whose coordinate is too large for the left. 
*/
       do 
         {
          ++left ;
           scan = FALSE ;
          x = (ptsP+left)->x ; 
           y = (ptsP+left)->y ; 
           if( axis == DTM_X_AXIS && ( x < pivot1 || ( x == pivot1 && y < pivot2 ))) scan = TRUE ;
           if( axis == DTM_Y_AXIS && ( y < pivot1 || ( y == pivot1 && x < pivot2 ))) scan = TRUE ;
         } while( left <= right && scan == TRUE ) ;
/* 
**  Search for a point whose coordinate is too small for the right. 
*/
       do 
          {
          --right ;
           scan = FALSE ;
          x = (ptsP+right)->x ; 
           y = (ptsP+right)->y ; 
           if( axis == DTM_X_AXIS && ( x > pivot1 || ( x == pivot1 && y > pivot2 ))) scan = TRUE ;
           if( axis == DTM_Y_AXIS && ( y > pivot1 || ( y == pivot1 && x > pivot2 ))) scan = TRUE ;
         } while( left <= right &&  scan == TRUE ) ;
/* 
** Swap the left and right points 
*/
       if( left < right ) 
          {
          p1P = ptsP + left ;
          p2P = ptsP + right ;
          temp = *p1P ;
          *p1P = *p2P ;
          *p2P = temp ;
          tag           = *(tagP+left) ;
          *(tagP+left)  = *(tagP+right) ;
          *(tagP+right) = tag ;
         }
      }
/* 
** Recursively sort the left subset. 
*/
    if( left > median) 
      {
       bcdtmSort_pointArrayClusterWithTag(ptsP,tagP,left,median,axis);
      }
/* 
** Recursively sort the right subset. 
*/
    if( right < median - 1 ) 
      {
       bcdtmSort_pointArrayClusterWithTag(ptsP+right+1,tagP+right+1,numPts-right-1,median-right-1,axis);
      }
   }
}
